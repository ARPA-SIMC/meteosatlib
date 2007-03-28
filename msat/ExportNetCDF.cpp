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
#include <msat/proj/const.h>

#include <netcdfcpp.h>

#include <sstream>
#include <stdexcept>

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
  // Build up output NetCDF file name and open it
  NcFile ncf(fileName.c_str(), NcFile::Replace);
  if (!ncf.is_valid())
    throw std::runtime_error("Creating a NcFile structure for file " + fileName);

  // Fill arrays on creation
  ncf.set_fill(NcFile::Fill);

  // Add Global Attributes
  //cerr << "global." << endl;

  ncfAddAttr(ncf, "Satellite", Image::spacecraftName(Image::spacecraftIDToHRIT(img.spacecraft_id)).c_str());
  char reftime[64];
  sprintf(reftime, "%04d-%02d-%02d %02d:%02d:00 UTC",
           img.year, img.month, img.day, img.hour, img.minute);
  ncfAddAttr(ncf, "Antenna", "Fixed");
  ncfAddAttr(ncf, "Receiver", "HIMET");
  ncfAddAttr(ncf, "Time", reftime);
  ncfAddAttr(ncf, "Area_Name", "SpaceView");
  char projname[16];
  sprintf(projname, "GEOS(%3.1f)", 0.0);
  //sprintf(projname, "GEOS(%3.1f)", pds.sublon);
  ncfAddAttr(ncf, "Projection", projname);
  ncfAddAttr(ncf, "Columns", 3712);
  //if (! ncf.add_att("Columns", pds.npix ) ) return false;
  ncfAddAttr(ncf, "Lines", 3712);
  //if (! ncf.add_att("Lines", pds.nlin ) ) return false;
  ncfAddAttr(ncf, "AreaStartPix", img.x0 + 1);
  //if (! ncf.add_att("AreaStartPix", (int) m.grid.sp.X0 ) ) return false;
  ncfAddAttr(ncf, "AreaStartLin", img.y0 + 1);
  //if (! ncf.add_att("AreaStartLin", (int) m.grid.sp.Y0 ) ) return false;
  ncfAddAttr(ncf, "SampleX", 1.0 );
  ncfAddAttr(ncf, "SampleY", 1.0 );
  ncfAddAttr(ncf, "Column_Scale_Factor", img.column_res * exp2(16));
  ncfAddAttr(ncf, "Line_Scale_Factor", img.line_res * exp2(16));
  ncfAddAttr(ncf, "Column_Offset", img.column_offset);
  ncfAddAttr(ncf, "Line_Offset", img.line_offset);
  ncfAddAttr(ncf, "Orbit_Radius", ORBIT_RADIUS);
  //if (! ncf.add_att("Orbit_Radius", pds.sh) ) return false;
  ncfAddAttr(ncf, "Longitude", 0);
  //if (! ncf.add_att("Longitude", pds.sublon) ) return false;
  ncfAddAttr(ncf, "NortPolar", 1);
  ncfAddAttr(ncf, "NorthSouth", img.column_res >= 0 ? 1 : 0);
  ncfAddAttr(ncf, "title", img.historyPlusEvent("Exported to NetCDF").c_str());
  ncfAddAttr(ncf, "Institution", INSTITUTION);
  ncfAddAttr(ncf, "Type", TYPE);
  ncfAddAttr(ncf, "Version", PACKAGE_VERSION);
  ncfAddAttr(ncf, "Conventions", "COARDS");
  ncfAddAttr(ncf, "history", img.history.c_str());

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

  NcVar *ivar = ncf.add_var(img.shortName.c_str(), ncFloat, tdim, ldim, cdim);
  if (!ivar->is_valid()) throw std::runtime_error("adding " + img.shortName + " variable failed");
	ncfAddAttr(*ivar, "add_offset", 0.0);
  ncfAddAttr(*ivar, "scale_factor", 1.0);
  ncfAddAttr(*ivar, "chnum", img.channel_id);
	ncfAddAttr(*ivar, "units", img.unit.c_str());

  // Write output values
  //cerr << "output." << endl;
  float *pixels = img.data->allScaled();
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
		ExportNetCDF(*img, img->defaultFilename + ".nc");
	}
};

std::auto_ptr<ImageConsumer> createNetCDFExporter()
{
	return std::auto_ptr<ImageConsumer>(new NetCDFExporter());
}

}

// vim:set ts=2 sw=2:
