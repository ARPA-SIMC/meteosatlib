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

struct NetCDFImageData : public ImageData
{
public:
  float* pixels;

	// FIXME: this amount of bpp is meaningless, since we read floats
  NetCDFImageData() : pixels(0) { bpp = 16; }
  ~NetCDFImageData()
  {
    if (pixels)
      delete[] pixels;
  }

  virtual int unscaled(int column, int line) const
  {
      return (int)pixels[line * columns + column];
  }
};


std::auto_ptr<ImageData> ImportNetCDF(const std::string& filename)
{
	auto_ptr<ImageData> data(new NetCDFImageData());

  NcFile ncf(filename.c_str(), NcFile::ReadOnly);
  if (! ncf.is_valid())
		throw std::runtime_error("Failed to open file " + filename);
	NcAtt* a;

	int tmp;
	float ftmp;
	std::string stmp;

#define GET(var, type, name) \
	if ((a = ncf.get_att(name))) \
		var = a->as_##type(0); \
	else \
		throw std::runtime_error(#type " value " name " not found in file " + filename)

	GET(data->columns, int, "Columns");
	GET(data->lines, int, "Lines");
	GET(tmp, int, "AreaStartPix");
  data->column_offset = 1 + 3712/2 - tmp;
	GET(tmp, int, "AreaStartLin");
	data->line_offset = 1 + 3712/2 - tmp;

	GET(ftmp, float, "SampleX");
	if (ftmp != 1.0)
	{
    stringstream msg;
    msg << "SampleX should have been 1.0 but is " << ftmp << " instead.";
    throw std::runtime_error(msg.str());
	}

	GET(ftmp, float, "SampleY");
	if (ftmp != 1.0)
	{
    stringstream msg;
    msg << "SampleY should have been 1.0 but is " << ftmp << " instead.";
    throw std::runtime_error(msg.str());
	}

	GET(data->column_factor, int, "Column_Scale_Factor");
	GET(data->line_factor, int, "Line_Scale_Factor");
	GET(data->column_offset, int, "Column_Offset");
	GET(data->line_offset, int, "Line_Offset");

	GET(ftmp, float, "Orbit_Radius");
	if (ftmp != 1.0)
    cerr << "Orbit_Radius should have been 6610684 " << ftmp << " instead: ignoring it." << endl;

	GET(ftmp, float, "Longitude");
	if (ftmp != 0)
	{
    stringstream msg;
    msg << "Longitude should have been 0 but is " << ftmp << " instead.";
    throw std::runtime_error(msg.str());
	}


	GET(stmp, string, "Time");
	if (sscanf(stmp.c_str(), "%04d-%02d-%02d %02d:%02d:00 UTC",
				&data->year, &data->month, &data->day, &data->hour, &data->minute) != 5)
		throw std::runtime_error("could not parse Time attribute " + stmp);
		
	//NcVar* ivar = ncf.get_var

	// TODO from here
	cout << ncf.num_vars() << " variables found in the file." << endl;
	cout << "Names:" << endl;
	for (int i = 0; i < ncf.num_vars(); ++i)
		cout << ncf.get_var(i)->name() << endl;

	// Missing: channel_id, spacecraft_id, 

	// TODO: unknown: slope, offset, bpp
	// TODO: for now I use hardcoded placeholders
	data->slope = 1;
	data->offset = 0;

	return data;

#if 0
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
#endif
}

bool isNetCDF(const std::string& filename)
{
	return filename.substr(filename.size() - 3) == ".nc";
}

class NetCDFImageImporter : public ImageImporter
{
	std::string filename;

public:
	NetCDFImageImporter(const std::string& filename) : filename(filename) {}

	virtual void read(ImageConsumer& output)
	{
		std::auto_ptr<ImageData> img = ImportNetCDF(filename);
		output.processImage(*img);
	}
};

std::auto_ptr<ImageImporter> createNetCDFImporter(const std::string& filename)
{
	return std::auto_ptr<ImageImporter>(new NetCDFImageImporter(filename));
}

// vim:set ts=2 sw=2:
