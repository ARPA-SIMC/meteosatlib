//---------------------------------------------------------------------------
//
//  File        :   NetCDF2GRIB.cpp
//  Description :   Convert MSG HRIT data from NetCDF format to GRIB format
//  Project     :   Lamma 2004
//  Author      :   Graziano Giuliani (Lamma Regione Toscana)
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

#include <conv/Image.h>

// GRIB Interface

#include <GRIB.h>

#include <sstream>
#include <iostream>
#include <stdexcept>

#include <conv/Image.tcc>

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
	// Read image data
	auto_ptr< ImageDataWithPixels<float> > res(new ImageDataWithPixels<float>(m.grid.nx, m.grid.ny));
  memcpy(res->pixels, m.field.vals, m.grid.nxny * sizeof(float));

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
			img->column_offset = (int)m.grid.sp.X0;	// probably need some (x-1)*2
			img->line_offset = (int)m.grid.sp.Y0;		// probably need some (x-1)*2
			img->sublon = m.grid.sp.lop;
			img->column_factor = Image::columnFactorFromSeviriDX((int)round(m.grid.sp.dx * 1000));
			img->line_factor = Image::lineFactorFromSeviriDY((int)round(m.grid.sp.dy * 1000));
			break;
		default:
		{
			std::stringstream str;
			str << "GRIB projection " << m.grid.type << " is not supported";
			throw std::runtime_error(str.str());
		}
	}

	switch (m.level.type)
	{
		case GRIB_LEVEL_SATELLITE_METEOSAT8:
			img->channel_id = (int)m.level.lv1 << 8 + (int)m.level.lv2;
			break;
		default:
		{
			std::stringstream str;
			str << "GRIB level " << m.level.type << " is not supported";
			throw std::runtime_error(str.str());
		}
	}

	if (m.get_pds_size() >= 12)
	{
		unsigned char* pds = m.get_pds_values(0, 12);
		switch (pds[0])
		{
			case 3:
				std::cerr << "pds type 3 does not convey satellite identifier information: using default of 55." << endl;
				img->spacecraft_id = 55;
				break;
			case 24:
				img->spacecraft_id = (pds[9] << 8) + pds[10];
				break;
			default:
				std::cerr << "unsupported pds type " << pds[0] << ": using default satellite identifier 55." << endl;
				img->spacecraft_id = 55;
				break;
		}
		delete[] pds;
	}

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
