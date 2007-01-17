//---------------------------------------------------------------------------
//
//  File        :   ImportGRIB.cpp
//  Description :   Read a msag::Image from a GRIB message
//  Project     :   Lamma 2004
//  Author      :   Enrico Zini (ARPA SIM Emilia Romagna)
//  Based on work by: Graziano Giuliani (Lamma Regione Toscana)
//  Source      :   n/a
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//  
//---------------------------------------------------------------------------
#include <cstdio>
#include <cmath>
#include <cstdlib>

#include "config.h"

#include <msat/Image.h>
#include "proj/const.h"
#include "proj/Geos.h"

// GRIB Interface

#include <grib/GRIB.h>

#include <sstream>
#include <iostream>
#include <stdexcept>

#include <msat/Image.tcc>
#include <msat/Progress.h>

#define TITLE "Observation File from MSG-SEVIRI"
#define INSTITUTION "HIMET"
#define TYPE "Obs file"
#define HIMET_VERSION 0.0

#define PATH_SEPARATOR "/"
// For windows use #define PATH_SEPARATOR "\"

using namespace std;

namespace msat {

bool isGrib(const std::string& filename)
{
	if (access(filename.c_str(), F_OK) != 0)
		return false;
	return filename.substr(filename.size() - 4) == ".grb";
}

auto_ptr<Image> importGrib(GRIB_MESSAGE& m)
{
	ProgressTask p("Reading GRIB file");

	// Read image data
	auto_ptr< ImageDataWithPixels<float> > res(new ImageDataWithPixelsPrescaled<float>(m.grid.nx, m.grid.ny));
  memcpy(res->pixels, m.field.vals, m.grid.nxny * sizeof(float));

	// Handle missing values
	res->missing = m.field.undef_high;
	res->missingValue = res->missing;
	for (int i = 0; i < m.grid.nxny; ++i)
		if (res->pixels[i] >= m.field.undef_low && res->pixels[i] <= m.field.undef_high)
			res->pixels[i] = res->missing;

	res->slope = exp10(-m.field.decimalscale);
	res->offset = -m.field.refvalue*res->slope;
	res->bpp = m.field.numbits;
	res->scalesToInt = m.field.binscale == 0;

	auto_ptr< Image > img(new Image());
	img->setData(res.release());

	img->year = m.gtime.year;
	img->month = m.gtime.month;
	img->day = m.gtime.day;
	img->hour = m.gtime.hour;
	img->minute = m.gtime.minute;

	switch (m.grid.type)
	{
		case GRIB_GRID_SPACEVIEW:
			img->x0 = (int)m.grid.sp.X0;	// probably need some (x-1)*2
			img->y0 = (int)m.grid.sp.Y0;		// probably need some (x-1)*2
			img->column_offset = (int)m.grid.sp.Xp;	// probably need some (x-1)*2
			img->line_offset = (int)m.grid.sp.Yp;		// probably need some (x-1)*2
			img->proj.reset(new proj::Geos(m.grid.sp.lop, ORBIT_RADIUS));
			img->column_res = Image::columnResFromSeviriDX(m.grid.sp.dx);
			img->line_res = Image::lineResFromSeviriDY(m.grid.sp.dy);
			break;
		default:
		{
			std::stringstream str;
			str << "GRIB projection " << m.grid.type << " is not supported";
			throw std::runtime_error(str.str());
		}
	}

#if 0
	switch (m.level.type)
	{
		case GRIB_LEVEL_SATELLITE_METEOSAT8:
			img->channel_id = (int)m.level.lv1 << 8 + (int)m.level.lv2;
			cerr << "CHID " << m.level.lv1 << "--" << m.level.lv2 << endl;
			break;
		default:
		{
			std::stringstream str;
			str << "GRIB level " << m.level.type << " is not supported";
			throw std::runtime_error(str.str());
		}
	}
#endif

	if (m.get_pds_size() >= 12)
	{
		unsigned char* pds = m.get_pds_values(0, 12);
		img->spacecraft_id = (int)pds[9];
		img->channel_id = ((int)pds[10] << 8) + (int)pds[11];
		delete[] pds;
	} else {
		std::stringstream str;
		str << "GRIB PDS is too short (" << m.get_pds_size() << " where at least 12 are needed)";
		throw std::runtime_error(str.str());
	}

#if 0
	img->spacecraft_id = 55;
	if (m.get_pds_size() >= 52)
	{
		// ECMWF local GRIB extensions, see:
		// http://www.ecmwf.int/publications/manuals/libraries/gribex/localGRIBUsage.html
		unsigned char* pds = m.get_pds_values(0, 52);
		if (pds[4] != 200)
		{
				std::cerr << "pds extensions found but from an unsupported originating centre (" << (int)pds[5] << "): using default satellite identifier 55." << endl;
		} else {
			switch (pds[40])
			{
				case 3:
					std::cerr << "pds type 3 does not convey satellite identifier information: using default of 55.  TODO: does displgrib get the default as well?" << endl;
					break;
				case 24:
					img->spacecraft_id = (pds[49] << 8) + pds[50];
					break;
				default:
					std::cerr << "unsupported pds type " << (int)pds[0] << ": using default satellite identifier 55." << endl;
					break;
			}
		}
		delete[] pds;
	} else
			std::cerr << "no local PDS extentions: using default satellite identifier 55." << endl;
#endif

  return img;
}


class GribImageImporter : public ImageImporter
{
	std::string filename;

public:
	GribImageImporter(const std::string& filename)
		: filename(filename) {}

	virtual void read(ImageConsumer& output)
	{
		GRIB_FILE gf;
		GRIB_MESSAGE m;
	 
		if (gf.OpenRead(filename) != 0)
			throw std::runtime_error("cannot open Grib file " + filename);

		int count = 0;
		while (gf.ReadMessage(m) == 0)
		{
			auto_ptr<Image> img = importGrib(m);
			img->setQualityFromPathname(filename);
			img->addToHistory("Imported from GRIB " + img->defaultFilename());
			cropIfNeeded(*img);
			output.processImage(img);
			++count;
		}
		if (count == 0)
			throw std::runtime_error("cannot read any Grib messages from file " + filename);
	}
};

std::auto_ptr<ImageImporter> createGribImporter(const std::string& filename)
{
	return std::auto_ptr<ImageImporter>(new GribImageImporter(filename));
}

}

// vim:set ts=2 sw=2:
