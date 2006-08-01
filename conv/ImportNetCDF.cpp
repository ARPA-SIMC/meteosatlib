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

#include <conv/ImageData.h>

// HRI format interface

#include <hrit/MSG_HRIT.h>

// GRIB Interface

#include <GRIB.h>

#include <sstream>
#include <stdexcept>

#define TITLE "Observation File from MSG-SEVIRI"
#define INSTITUTION "HIMET"
#define TYPE "Obs file"
#define HIMET_VERSION 0.0

#define PATH_SEPARATOR "/"
// For windows use #define PATH_SEPARATOR "\"

using namespace std;

void usage(char *pname);
char *chname(char *chdesc, int len);
bool GribProduct(char *ncfname);

struct NetCDFImage
{
	virtual void acquire(const NcVar& var) = 0;
};

template<typename Item>
struct NetCDFImageData : public ImageDataWithPixels<Item>, public NetCDFImage
{
	virtual void acquire(const NcVar& var)
	{
		if (var.num_dims() != 3)
		{
			stringstream msg;
			msg << "Number of dimensions for " << var.name() << " should be 3 but is " << var.num_dims() << "instead";
			throw std::runtime_error(msg.str());
		}
		int tsize = var.get_dim(0)->size();
		if (tsize != 1)
		{
			stringstream msg;
			msg << "Size of the time dimension for " << var.name() << " should be 1 but is " << tsize << "instead";
			throw std::runtime_error(msg.str());
		}

		this->lines = var.get_dim(1)->size();
		this->columns = var.get_dim(2)->size();

		if (this->pixels)
			delete[] this->pixels;
		this->pixels = new Item[this->columns * this->lines];

		if (!var.get(this->pixels, 1, this->lines, this->columns))
			throw std::runtime_error("reading image pixels failed");
	}
};


#if 0
std::auto_ptr<ImageData> ImportNetCDF(const std::string& filename)
{

  if (! ncf.add_att("Satellite",
            MSG_spacecraft_name((t_enum_MSG_spacecraft) pds.spc).c_str()))
    return false;
  sprintf(reftime, "%04d-%02d-%02d %02d:%02d:00 UTC",
           m.gtime.year, m.gtime.month, m.gtime.day,
	   m.gtime.hour, m.gtime.minute);
  if (! ncf.add_att("Antenna", "Fixed") ) return false;
  if (! ncf.add_att("Receiver", "HIMET") ) return false;
  if (! ncf.add_att("Area_Name", "SpaceView" ) ) return false;
  sprintf(projname, "GEOS(%3.1f)", pds.sublon);
  if (! ncf.add_att("Projection", projname) ) return false;

  if (! ncf.add_att("NortPolar", 1) ) return false;
  if (! ncf.add_att("NorthSouth", 0) ) return false;
  if (! ncf.add_att("title", TITLE) ) return false;
  if (! ncf.add_att("Institution", INSTITUTION) ) return false;
  if (! ncf.add_att("Type", TYPE) ) return false;
  if (! ncf.add_att("Version", HIMET_VERSION) ) return false;
  if (! ncf.add_att("Conventions", "COARDS") ) return false;
  if (! ncf.add_att("history", "Created from raw data") ) return false;
#endif

#if 0
  char *channelstring = strdup(MSG_channel_name((t_enum_MSG_spacecraft) pds.spc,
                               pds.chn).c_str( ));
  char *channel = chname(channelstring, strlen(channelstring) + 1);

  // Build up output NetCDF file name and open it
  sprintf( NcName, "%s_%4d%02d%02d_%02d%02d.nc", channel,
           m.gtime.year, m.gtime.month, m.gtime.day,
	   m.gtime.hour, m.gtime.minute);
  NcFile ncf ( NcName , NcFile::Replace );
  if (! ncf.is_valid()) return false;

  // Fill arrays on creation
  ncf.set_fill(NcFile::Fill);

  // Add Global Attributes
  if (! ncf.add_att("Satellite",
            MSG_spacecraft_name((t_enum_MSG_spacecraft) pds.spc).c_str()))
    return false;
  sprintf(reftime, "%04d-%02d-%02d %02d:%02d:00 UTC",
           m.gtime.year, m.gtime.month, m.gtime.day,
	   m.gtime.hour, m.gtime.minute);
  if (! ncf.add_att("Antenna", "Fixed") ) return false;
  if (! ncf.add_att("Receiver", "HIMET") ) return false;
  if (! ncf.add_att("Time", reftime) ) return false;
  if (! ncf.add_att("Area_Name", "SpaceView" ) ) return false;
  sprintf(projname, "GEOS(%3.1f)", pds.sublon);
  if (! ncf.add_att("Projection", projname) ) return false;
  if (! ncf.add_att("Columns", pds.npix ) ) return false;
  if (! ncf.add_att("Lines", pds.nlin ) ) return false;
  if (! ncf.add_att("AreaStartPix", (int) m.grid.sp.X0 ) ) return false;
  if (! ncf.add_att("AreaStartLin", (int) m.grid.sp.Y0 ) ) return false;
  if (! ncf.add_att("SampleX", 1.0 ) ) return false;
  if (! ncf.add_att("SampleY", 1.0 ) ) return false;
  if (! ncf.add_att("Column_Scale_Factor", pds.cfac) ) return false;
  if (! ncf.add_att("Line_Scale_Factor", pds.lfac) ) return false;
  if (! ncf.add_att("Column_Offset", pds.coff) ) return false;
  if (! ncf.add_att("Line_Offset", pds.loff) ) return false;
  if (! ncf.add_att("Orbit_Radius", pds.sh) ) return false;
  if (! ncf.add_att("Longitude", pds.sublon) ) return false;
  if (! ncf.add_att("NortPolar", 1) ) return false;
  if (! ncf.add_att("NorthSouth", 0) ) return false;
  if (! ncf.add_att("title", TITLE) ) return false;
  if (! ncf.add_att("Institution", INSTITUTION) ) return false;
  if (! ncf.add_att("Type", TYPE) ) return false;
  if (! ncf.add_att("Version", HIMET_VERSION) ) return false;
  if (! ncf.add_att("Conventions", "COARDS") ) return false;
  if (! ncf.add_att("history", "Created from raw data") ) return false;

  // Dimensions

  tdim = ncf.add_dim("time");
  if (!tdim->is_valid()) return false;
  ldim = ncf.add_dim("line", nlin);
  if (!ldim->is_valid()) return false;
  cdim = ncf.add_dim("column", npix);
  if (!cdim->is_valid()) return false;

  // Add Calibration values

  tvar = ncf.add_var("time", ncDouble, tdim);
  if (!tvar->is_valid()) return false;
  tvar->add_att("long_name", "Time");
  tvar->add_att("units", "seconds since 2000-01-01 00:00:00 UTC");
  double atime = (double) m.gtime.ForecastSeconds2000( );
  if (!tvar->put(&atime, 1)) return false;

  ivar = ncf.add_var(channel, ncFloat, tdim, ldim, cdim);
  if (!ivar->is_valid()) return false;
    if (!ivar->add_att("add_offset", 1.0)) return false;
  if (!ivar->add_att("scale_factor", 1.0)) return false;
  if (!ivar->add_att("chnum", pds.chn)) return false;
  if (pds.chn > 3 && pds.chn < 12)
  {
    ivar->add_att("units", "K");
    for (int i = 0; i < (int) total_size; i ++)
      if (pixels[i] > 0) pixels[i] += 145.0;
  }
  else
    ivar->add_att("units", "mW m^-2 sr^-1 (cm^-1)^-1");

  // Write output values
  if (!ivar->put(pixels, 1, nlin, npix)) return false;

  // Close NetCDF output
  (void) ncf.close( );

  delete [ ] pixels;

  return( true );
}
#endif

bool isNetCDF(const std::string& filename)
{
	return filename.substr(filename.size() - 3) == ".nc";
}

class NetCDFImageImporter : public ImageImporter
{
	std::string filename;
  NcFile ncf;

	void readHeader(ImageData& img)
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

		// FIXME: come la parso questa??
		stmp = GET(string, "Satellite");

		if (GETDEF(int, "Columns", 3712) != 3712)
			cerr << "Columns should have been 3712 but is " << GETDEF(int, "Columns", 3712) << " instead: ignoring it" << endl;
		if (GETDEF(int, "Lines", 3712) != 3712)
			cerr << "Lines should have been 3712 but is " << GETDEF(int, "Lines", 3712) << " instead: ignoring it" << endl;

		img.column_offset = 1 + 3712/2 - GET(int, "AreaStartPix");
		img.line_offset = 1 + 3712/2 - GET(int, "AreaStartLin");

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

		stmp = GET(string, "Time");
		if (sscanf(stmp.c_str(), "%04d-%02d-%02d %02d:%02d:00 UTC",
					&img.year, &img.month, &img.day, &img.hour, &img.minute) != 5)
			throw std::runtime_error("could not parse Time attribute " + stmp);
#undef GET
#undef GETDEF
	}

	void readData(const NcVar& var, ImageData& img)
	{
		NcAtt* a;

		img.offset = ((a = var.get_att("add_offset")) != NULL ? a->as_float(0) : 0);
		img.slope = ((a = var.get_att("scale_factor")) != NULL ? a->as_float(0) : 1);
		img.channel_id = ((a = var.get_att("chnum")) != NULL ? a->as_int(0) :
			throw std::runtime_error(string("could not find the 'chnum' attribute in image ") + var.name()));

		dynamic_cast<NetCDFImage*>(&img)->acquire(var);
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
		auto_ptr<ImageData> data;
		for (int i = 0; i < ncf.num_vars(); ++i)
		{
			NcVar* var = ncf.get_var(i);
			if (string(var->name()) == "time")
				continue;
			switch (var->type())
			{
				case ncByte: data.reset(new NetCDFImageData<unsigned char>()); break;
				case ncChar: data.reset(new NetCDFImageData<char>()); break;
				case ncShort: data.reset(new NetCDFImageData<int16_t>()); break;
				case ncLong: data.reset(new NetCDFImageData<int32_t>()); break;
				case ncFloat: data.reset(new NetCDFImageData<float>()); break;
				case ncDouble: data.reset(new NetCDFImageData<double>()); break;
			}
			readHeader(*data);
			readData(*var, *data);
			output.processImage(*data);
		}
	}
};

std::auto_ptr<ImageImporter> createNetCDFImporter(const std::string& filename)
{
	return std::auto_ptr<ImageImporter>(new NetCDFImageImporter(filename));
}

// vim:set ts=2 sw=2:
