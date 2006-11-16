//---------------------------------------------------------------------------
//
//  File        :   ExportNetCDF.cpp
//  Description :   Export data from an ImageData into a NetCDF file
//  Project     :   ?
//  Author      :   Enrico Zini (for ARPA SIM Emilia Romagna)
//  Source      :   derived from SAFH5CT2NetCDF.cpp by Le Duc, as modified by
//                  Francesca Di Giuseppe and from XRIT2Grib.cpp by Graziano
//                  Giuliani (Lamma Regione Toscana)
//  RCS ID      :   $Id$
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

#include <config.h>

#include <msat/ExportNetCDF.h>

#include <netcdfcpp.h>

#include <sstream>
#include <stdexcept>

// For MSG_channel_name
#include <hrit/MSG_HRIT.h>

#include "NetCDFUtils.h"

#define INSTITUTION "ARPA-SIM"
#define TYPE "Processed Products"

using namespace std;

namespace msat {

//
// Creates NetCDF product
//
void ExportNetCDF(const Image& img, const std::string& fileName)
{
  // Get the channel name
  string channelstring = MSG_channel_name((t_enum_MSG_spacecraft)img.spacecraftIDToHRIT(img.spacecraft_id), img.channel_id);

  // Change sensitive characters into underscores
  for (string::iterator i = channelstring.begin();
	i != channelstring.end(); ++i)
    if (*i == ' ' || *i == '.' || *i == ',')
      *i = '_';

  // Build up output NetCDF file name and open it
  NcFile ncf(fileName.c_str(), NcFile::Replace);
  if (!ncf.is_valid())
    throw std::runtime_error("Creating a NcFile structure for file " + fileName);

  // Fill arrays on creation
  ncf.set_fill(NcFile::Fill);

  // Add Global Attributes
  //cerr << "global." << endl;

  ncfAddAttr(ncf, "Satellite", MSG_spacecraft_name((t_enum_MSG_spacecraft)Image::spacecraftIDToHRIT(img.spacecraft_id)).c_str());
  char reftime[64];
  sprintf(reftime, "%04d-%02d-%02d %02d:%02d:00 UTC",
           img.year, img.month, img.day, img.hour, img.minute);
  ncfAddAttr(ncf, "Antenna", "Fixed");
  ncfAddAttr(ncf, "Receiver", "HIMET");
  ncfAddAttr(ncf, "Time", reftime);
  ncfAddAttr(ncf, "Area_Name", "SpaceView");
  char projname[16];
  sprintf(projname, "GEOS(%3.1f)", 0);
  //sprintf(projname, "GEOS(%3.1f)", pds.sublon);
  ncfAddAttr(ncf, "Projection", projname);
  ncfAddAttr(ncf, "Columns", 3712);
  //if (! ncf.add_att("Columns", pds.npix ) ) return false;
  ncfAddAttr(ncf, "Lines", 3712);
  //if (! ncf.add_att("Lines", pds.nlin ) ) return false;
  ncfAddAttr(ncf, "AreaStartPix", img.x0 );
  //if (! ncf.add_att("AreaStartPix", (int) m.grid.sp.X0 ) ) return false;
  ncfAddAttr(ncf, "AreaStartLin", img.y0 );
  //if (! ncf.add_att("AreaStartLin", (int) m.grid.sp.Y0 ) ) return false;
  ncfAddAttr(ncf, "SampleX", 1.0 );
  ncfAddAttr(ncf, "SampleY", 1.0 );
  ncfAddAttr(ncf, "Column_Scale_Factor", img.column_factor);
  ncfAddAttr(ncf, "Line_Scale_Factor", img.line_factor);
  ncfAddAttr(ncf, "Column_Offset", img.column_offset);
  ncfAddAttr(ncf, "Line_Offset", img.line_offset);
  ncfAddAttr(ncf, "Orbit_Radius", 6610684);
  //if (! ncf.add_att("Orbit_Radius", pds.sh) ) return false;
  ncfAddAttr(ncf, "Longitude", 0);
  //if (! ncf.add_att("Longitude", pds.sublon) ) return false;
  ncfAddAttr(ncf, "NortPolar", 1);
  ncfAddAttr(ncf, "NorthSouth", 0);
  ncfAddAttr(ncf, "title", img.historyPlusEvent("Exported to NetCDF").c_str());
  ncfAddAttr(ncf, "Institution", INSTITUTION);
  ncfAddAttr(ncf, "Type", TYPE);
  ncfAddAttr(ncf, "Version", PACKAGE_VERSION);
  ncfAddAttr(ncf, "Conventions", "COARDS");
  ncfAddAttr(ncf, "history", "Created from SAF HDF5 data");

  // Dimensions
  //cerr << "dimensions." << endl;

  NcDim *tdim = ncf.add_dim("time");
  if (!tdim->is_valid()) throw std::runtime_error("adding time dimension failed");
  NcDim *ldim = ncf.add_dim("line", img.data->lines);
  if (!ldim->is_valid()) throw std::runtime_error("adding line dimension failed");
  NcDim *cdim = ncf.add_dim("column", img.data->columns);
  if (!cdim->is_valid()) throw std::runtime_error("adding column dimension failed");

  // Add Calibration values
  //cerr << "calibration. cs: " << channelstring << endl;

  NcVar *tvar = ncf.add_var("time", ncDouble, tdim);
  if (!tvar->is_valid()) throw std::runtime_error("adding time variable failed");
  tvar->add_att("long_name", "Time");
  tvar->add_att("units", "seconds since 2000-01-01 00:00:00 UTC");
  double atime = (double) img.forecastSeconds2000();
  if (!tvar->put(&atime, 1)) throw std::runtime_error("setting time variable failed");

  NcVar *ivar = ncf.add_var(channelstring.c_str(), ncFloat, tdim, ldim, cdim);
  if (!ivar->is_valid()) throw std::runtime_error("adding " + channelstring + " variable failed");
	ncfAddAttr(*ivar, "add_offset", 0.0);
  ncfAddAttr(*ivar, "scale_factor", 1.0);
  ncfAddAttr(*ivar, "chnum", img.channel_id);

  float *pixels = img.data->allScaled();
  if (img.channel_id > 3 && img.channel_id < 12)
    ivar->add_att("units", "K");
  else
    ivar->add_att("units", "mW m^-2 sr^-1 (cm^-1)^-1");

  // Write output values
  //cerr << "output." << endl;
  if (!ivar->put(pixels, 1, img.data->lines, img.data->columns))
		throw std::runtime_error("writing image values failed");

  // Close NetCDF output
  (void) ncf.close( );

  delete [ ] pixels;

//  cout << "Wrote file " << NcName << "." << endl;
}


class NetCDFExporter : public ImageConsumer
{
public:
	virtual void processImage(auto_ptr<Image> img)
	{
		ExportNetCDF(*img, img->defaultFilename() + ".nc");
	}
};

std::auto_ptr<ImageConsumer> createNetCDFExporter()
{
	return std::auto_ptr<ImageConsumer>(new NetCDFExporter());
}

}

// vim:set ts=2 sw=2:
