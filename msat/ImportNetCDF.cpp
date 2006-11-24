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

// Unidata NetCDF

#include <netcdfcpp.h>

#include <msat/Image.h>
#include "proj/const.h"
#include "proj/Geos.h"

#include <sstream>
#include <stdexcept>

#include <msat/Image.tcc>
#include <msat/Progress.h>

#include "NetCDFUtils.h"

#define PATH_SEPARATOR "/"
// For windows use #define PATH_SEPARATOR "\"

using namespace std;

// char *chname(char *chdesc, int len);
// bool GribProduct(char *ncfname);

namespace msat {

bool isNetCDF(const std::string& filename)
{
	if (access(filename.c_str(), F_OK) != 0)
		return false;
	if (filename.substr(filename.size() - 3) != ".nc")
		return false;
	try {
		NcFile ncf(filename.c_str(), NcFile::ReadOnly);
		if (! ncf.is_valid())
			return false;
		return ncf.get_att("Column_Scale_Factor") != NULL;
	} catch (...) {
		return false;
	}
}

class NetCDFImageImporter : public ImageImporter
{
	std::string filename;
  NcFile ncf;

	void readHeader(Image& img)
	{
		NcAtt* a;
		float ftmp;
		std::string stmp;

#define GET(type, name) \
		((a = ncf.get_att(name)) != NULL ? a->as_##type(0) : \
			throw std::runtime_error(#type " value " name " not found in file " + filename))
#define GETDEF(type, name, def) \
		((a = ncf.get_att(name)) != NULL ? a->as_##type(0) : def)

		img.history = GET(string, "title");

		stmp = GET(string, "Satellite");
		if (stmp == "MSG1")
			img.spacecraft_id = 55;
		else
			throw std::runtime_error("finding the satellite ID for satellite " + stmp + " is still unimplemented");

		if (GETDEF(int, "Columns", 3712) != 3712)
			cerr << "Columns should have been 3712 but is " << GETDEF(int, "Columns", 3712) << " instead: ignoring it" << endl;
		if (GETDEF(int, "Lines", 3712) != 3712)
			cerr << "Lines should have been 3712 but is " << GETDEF(int, "Lines", 3712) << " instead: ignoring it" << endl;

//		img.column_offset = 1 + 3712/2 - GET(int, "AreaStartPix");
//		img.line_offset = 1 + 3712/2 - GET(int, "AreaStartLin");

		ftmp = GETDEF(float, "SampleX", 1.0);
		if (ftmp != 1.0)
		{
			stringstream msg;
			msg << "SampleX should have been 1.0 but is " << ftmp << " instead.";
			throw std::runtime_error(msg.str());
		}

		ftmp = GETDEF(float, "SampleY", 1.0);
		if (ftmp != 1.0)
		{
			stringstream msg;
			msg << "SampleY should have been 1.0 but is " << ftmp << " instead.";
			throw std::runtime_error(msg.str());
		}

		img.column_factor = GET(int, "Column_Scale_Factor");
		img.line_factor = GET(int, "Line_Scale_Factor");
		img.column_offset = GET(int, "Column_Offset");
		img.line_offset = GET(int, "Line_Offset");
		img.x0 = GET(int, "AreaStartPix");
		img.y0 = GET(int, "AreaStartLin");

		ftmp = GETDEF(float, "Orbit_Radius", 6610684);
		if (ftmp != 6610684)
		{
			cerr << "Orbit_Radius should have been 6610684 but is " << ftmp << " instead: ignoring it." << endl;
			ftmp = 6610684;
		}

		ftmp = GETDEF(float, "Longitude", 0);
		if (ftmp != 0)
		{
			stringstream msg;
			msg << "Longitude should have been 0 but is " << ftmp << " instead.";
			throw std::runtime_error(msg.str());
		}
		img.proj.reset(new proj::Geos(ftmp, ORBIT_RADIUS));

		stmp = GET(string, "Time");
		if (sscanf(stmp.c_str(), "%04d-%02d-%02d %02d:%02d:00 UTC",
					&img.year, &img.month, &img.day, &img.hour, &img.minute) != 5)
			throw std::runtime_error("could not parse Time attribute " + stmp);
#undef GET
#undef GETDEF
	}

	template<typename Sample>
	void readData(const NcVar& var, Image& img)
	{
		NcAtt* a;

		img.setData(acquireImage<Sample>(var));

		img.data->offset = ((a = var.get_att("add_offset")) != NULL ? a->as_float(0) : 0);
		img.data->slope = ((a = var.get_att("scale_factor")) != NULL ? a->as_float(0) : 1);
		img.channel_id = ((a = var.get_att("chnum")) != NULL ? a->as_int(0) :
			throw std::runtime_error(string("could not find the 'chnum' attribute in image ") + var.name()));
	}

public:
	NetCDFImageImporter(const std::string& filename)
		: filename(filename), ncf(filename.c_str(), NcFile::ReadOnly)
	{
		if (! ncf.is_valid())
			throw std::runtime_error("Failed to open file " + filename);
	}

	virtual void read(ImageConsumer& output)
	{
		ProgressTask p("Reading NetCDF file " + filename);
		for (int i = 0; i < ncf.num_vars(); ++i)
		{
			auto_ptr<Image> img(new Image);
			readHeader(*img);
			img->setQualityFromPathname(filename);

			NcVar* var = ncf.get_var(i);
			if (string(var->name()) == "time")
				continue;
			ProgressTask p1(string("Reading NetCDF variable ") + var->name());

			switch (var->type())
			{
				case ncNoType: throw std::runtime_error("The NetCDF data has values with unknown type");
				case ncByte:   readData<ncbyte>(*var, *img); img->data->scalesToInt = true;  break;
				case ncChar:   readData<char>(*var, *img);   img->data->scalesToInt = true;  break;
				case ncShort:  readData<short>(*var, *img);  img->data->scalesToInt = true;  break;
				case ncInt:    readData<int>(*var, *img);    img->data->scalesToInt = true;  break;
				case ncFloat:  readData<float>(*var, *img);  img->data->scalesToInt = false; break;
				case ncDouble: readData<double>(*var, *img); img->data->scalesToInt = false; break;
			}
			img->addToHistory("Imported from NetCDF " + img->defaultFilename());
			computeBPP(*img->data);
			cropIfNeeded(*img);
			output.processImage(img);
		}
	}
};

std::auto_ptr<ImageImporter> createNetCDFImporter(const std::string& filename)
{
	return std::auto_ptr<ImageImporter>(new NetCDFImageImporter(filename));
}

}

// vim:set ts=2 sw=2:
