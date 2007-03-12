//---------------------------------------------------------------------------
//
//  File        :   ImportNetCDF24.cpp
//  Description :   Import satellite images from the NetCDF24 format
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
#include "ImportNetCDF24.h"

#include <cstdio>
#include <cmath>
#include <cstdlib>

#include "config.h"

// Unidata NetCDF

#include <netcdfcpp.h>

#include <msat/Image.h>
#include <msat/ImportUtils.h>
#include "proj/const.h"
#include "proj/Geos.h"
#include <grib/GRIB.h>

#include <sstream>
#include <stdexcept>

#include <msat/Image.tcc>
#include <msat/Progress.h>

#include "NetCDFUtils.h"

#define TITLE "Observation File from MSG-SEVIRI"
#define INSTITUTION "HIMET"
#define TYPE "Obs file"
#define HIMET_VERSION 0.0

#define PATH_SEPARATOR "/"
// For windows use #define PATH_SEPARATOR "\"

using namespace std;

// char *chname(char *chdesc, int len);
// bool GribProduct(char *ncfname);

namespace msat {

bool isNetCDF24(const std::string& filename)
{
	if (access(filename.c_str(), F_OK) != 0)
		return false;
	if (filename.substr(filename.size() - 3) != ".nc")
		return false;
	try {
		NcFile ncf(filename.c_str(), NcFile::ReadOnly);
		if (! ncf.is_valid())
			return false;
		return ncf.get_att("GribEditionNumber") != NULL;
	} catch (...) {
		return false;
	}
}

class NetCDF24ImageImporter : public ImageImporter
{
	std::string filename;
  NcFile ncf;

	void readHeader(Image& img)
	{
		NcAtt* a;
		std::string stmp;

#define GET(type, name) \
		((a = ncf.get_att(name)) != NULL ? a->as_##type(0) : \
			throw std::runtime_error(#type " value " name " not found in file " + filename))
#define GETDEF(type, name, def) \
		((a = ncf.get_att(name)) != NULL ? a->as_##type(0) : def)
#define PGET(type, name) \
		((a = proj->get_att(name)) != NULL ? a->as_##type(0) : \
			throw std::runtime_error(#type " value Projection/" name " not found in file " + filename))

		if (GET(int, "GribEditionNumber") != 1)
		{
			stringstream str;
			str << "Message has unsupported edition number " << GET(int, "GribEditionNumber")
				  << " (only 1 is supported)";
			throw std::runtime_error(str.str());
		}

		if (GET(int, "LevelType") != GRIB_LEVEL_SATELLITE_METEOSAT8)
			throw std::runtime_error("Only Meteosat 8 data is currently imported");

		img.channel_id = GET(int, "L1");
		img.year = GET(int, "Year");
		img.month =	GET(int, "Month");
		img.day =	GET(int, "Day");
		img.hour = GET(int, "Hour");
		img.minute = GET(int, "Minute");
		img.spacecraft_id = GET(int, "SatelliteID");

		NcVar* proj = ncf.get_var("Projection");
		img.proj.reset(new proj::Geos(PGET(float, "Lop"), ORBIT_RADIUS));
		img.column_res = Image::columnResFromSeviriDX(PGET(int, "DX"));
		img.line_res = Image::lineResFromSeviriDY(PGET(int, "DY"));
		img.column_offset =	PGET(int, "Xp");
		img.line_offset =	PGET(int, "Yp");
		img.x0 =	PGET(int, "X0");
		img.y0 =	PGET(int, "Y0");

#undef GET
#undef GETDEF
#undef PGET
	}

	template<typename Sample>
	void readData(const NcVar& var, Image& img)
	{
		NcAtt* a;

		img.setData(acquireImage<Sample>(var));

		img.data->offset = ((a = var.get_att("add_offset")) != NULL ? a->as_float(0) : 0);
		img.data->slope = ((a = var.get_att("scale_factor")) != NULL ? a->as_float(0) : 1);
		img.channel_id = ((a = var.get_att("channel")) != NULL ? a->as_int(0) :
			throw std::runtime_error(string("could not find the 'channel' attribute in image ") + var.name()));
		img.unit = ((a = var.get_att("units")) != NULL ? a->as_string(0) : Image::channelUnit(img.spacecraft_id, img.channel_id));
	}

public:
	NetCDF24ImageImporter(const std::string& filename)
		: filename(filename), ncf(filename.c_str(), NcFile::ReadOnly)
	{
		if (! ncf.is_valid())
			throw std::runtime_error("Failed to open file " + filename);
	}

	virtual void read(ImageConsumer& output)
	{
		ProgressTask p("Reading NetCDF24 file " + filename);
		for (int i = 0; i < ncf.num_vars(); ++i)
		{
			auto_ptr<Image> img(new Image);
			readHeader(*img);
			img->setQualityFromPathname(filename);

			NcVar* var = ncf.get_var(i);
			if (string(var->name()) == "Projection")
				continue;
			if (string(var->name()) == "time")
				continue;

			ProgressTask p1(string("Reading NetCDF24 variable ") + var->name());

			switch (var->type())
			{
				case ncByte:   readData<ncbyte>(*var, *img); img->data->scalesToInt = true;  break;
				case ncChar:   readData<char>(*var, *img);   img->data->scalesToInt = true;  break;
				case ncShort:  readData<short>(*var, *img);  img->data->scalesToInt = true;  break;
				case ncInt:    readData<int>(*var, *img);    img->data->scalesToInt = true;  break;
				case ncFloat:  readData<float>(*var, *img);  img->data->scalesToInt = false; break;
				case ncDouble: readData<double>(*var, *img); img->data->scalesToInt = false; break;
			}
			img->defaultFilename = util::satelliteSingleImageFilename(*img);
			img->shortName = util::satelliteSingleImageShortName(*img);
			img->addToHistory("Imported from NetCDF24 " + img->defaultFilename + " variable " + var->name());
			computeBPP(*img->data);
			cropIfNeeded(*img);
			output.processImage(img);
		}
	}
};

std::auto_ptr<ImageImporter> createNetCDF24Importer(const std::string& filename)
{
	return std::auto_ptr<ImageImporter>(new NetCDF24ImageImporter(filename));
}

}

// vim:set ts=2 sw=2:
