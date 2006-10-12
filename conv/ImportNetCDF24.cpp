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

#include <conv/Image.h>
#include <grib/GRIB.h>

#include <sstream>
#include <stdexcept>

#include <conv/Image.tcc>

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

template<typename Sample>
static ImageData* acquireImage(const NcVar& var)
{
	if (var.num_dims() != 3)
	{
		stringstream msg;
		msg << "Number of dimensions for " << var.name() << " should be 3 but is " << var.num_dims() << " instead";
		throw std::runtime_error(msg.str());
	}

	int tsize = var.get_dim(0)->size();
	if (tsize != 1)
	{
		stringstream msg;
		msg << "Size of the time dimension for " << var.name() << " should be 1 but is " << tsize << " instead";
		throw std::runtime_error(msg.str());
	}

	auto_ptr< ImageDataWithPixels<Sample> > res(new ImageDataWithPixels<Sample>(var.get_dim(2)->size(), var.get_dim(1)->size()));

	if (!var.get(res->pixels, 1, res->lines, res->columns))
		throw std::runtime_error("reading image pixels failed");

	return res.release();
}

class NetCDF24ImageImporter : public ImageImporter
{
	std::string filename;
  NcFile ncf;

	void readHeader(Image& img)
	{
		NcAtt* a;
		int tmp;
		float ftmp;
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

		img.sublon = PGET(float, "Lop");
		img.column_factor = Image::columnFactorFromSeviriDX(PGET(int, "DX"));
		img.line_factor = Image::lineFactorFromSeviriDY(PGET(int, "DY"));
		img.column_offset =	PGET(int, "X0");
		img.line_offset =	PGET(int, "Y0");

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
		for (int i = 0; i < ncf.num_vars(); ++i)
		{
			auto_ptr<Image> img(new Image);
			readHeader(*img);

			NcVar* var = ncf.get_var(i);
			if (string(var->name()) == "Projection")
				continue;
			if (string(var->name()) == "time")
				continue;

			switch (var->type())
			{
				case ncByte: readData<unsigned char>(*var, *img); img->data->scalesToInt = true; break;
				case ncChar: readData<char>(*var, *img); img->data->scalesToInt = true; break;
				case ncShort: readData<int16_t>(*var, *img); img->data->scalesToInt = true; break;
				case ncLong: readData<int32_t>(*var, *img); img->data->scalesToInt = true; break;
				case ncFloat: readData<float>(*var, *img); img->data->scalesToInt = false; break;
				case ncDouble: readData<double>(*var, *img); img->data->scalesToInt = false; break;
			}
			if (shouldCrop())
				img->crop(cropX, cropY, cropWidth, cropHeight);
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
