//---------------------------------------------------------------------------
//
//  File        :   ExportRepNetCDF.cpp
//  Description :   Export data from an ImageData into a Met_Reproj-style NetCDF file
//  Author      :   Enrico Zini (for ARPA SIM Emilia Romagna)
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

#include <msat/ExportRepNetCDF.h>
#include <msat/proj/const.h>
#include <msat/proj/Latlon.h>

#include <netcdfcpp.h>

#include <sstream>
#include <stdexcept>

#include "NetCDFUtils.h"

#define INSTITUTION "ARPA-SIM"
#define TYPE "Processed Products"

using namespace std;

namespace msat {

//
// Creates Met_Reproj-style NetCDF product
//
void ExportRepNetCDF(const Image& img, const std::string& fileName)
{
	// Check that Image has a Latlon projection, otherwise we cannot encode it
	if (dynamic_cast<proj::Latlon*>(img.proj.get()) == 0)
		throw std::runtime_error("image data are not in a regular latitude-longitude grid");

  // Build up output NetCDF file name and open it
  NcFile ncf(fileName.c_str(), NcFile::Replace);
  if (!ncf.is_valid())
    throw std::runtime_error("Creating a NcFile structure for file " + fileName);

  // Fill arrays on creation
  ncf.set_fill(NcFile::Fill);


	// Interpolate and fetch the value, makes it possible to then compute z_range
	// and also X and Y dimensions

	int xsize = img.data->columns;
	int ysize = img.data->lines;

  // Uncropped image goes from (90,-180) to (-90, 180)
	// Image goes from (90,-180)+(x0/column_offset, y0/line_offset)
	// Image goes to (90,-180)+((x0+columns)/column_offset, (y0+lines)/line_offset)
	// Steps are 1/column_offset and 1/line_offset

	float data[xsize * ysize];
	float lats[ysize];
	float lons[xsize];
	for (int y = 0; y < ysize; ++y)
	{
		for (int x = 0; x < xsize; ++x)
		{
			double lat, lon;
			img.pixelsToCoords(x, y, lat, lon);
			float datum = img.data->scaled(x, y);
			if (datum == img.data->missingValue)
				data[y * xsize + x] = FP_NAN;
			else
				data[y * xsize + x] = datum;

			if (y == 0) lons[x] = lon;
			if (x == 0) lats[y] = lat;
		}
	}

	string name = Image::spacecraftName(img.spacecraft_id);
  ncfAddAttr(ncf, "Satellite", name.c_str());
	//ncf.add_att("Antenna", ncf.get_att("Antenna")->as_string(0));
  ncfAddAttr(ncf, "Antenna", "Fixed");
	//ncf.add_att("Receiver", ncf.get_att("Receiver")->as_string(0));
  ncfAddAttr(ncf, "Receiver", "HIMET");

  char reftime[64];
  sprintf(reftime, "%04d-%02d-%02d %02d:%02d:00 UTC",
           img.year, img.month, img.day, img.hour, img.minute);
  ncfAddAttr(ncf, "Time", reftime);

  ncfAddAttr(ncf, "Institution", INSTITUTION);
  ncfAddAttr(ncf, "Version", PACKAGE_VERSION);
  ncfAddAttr(ncf, "Conventions", "COARDS");
  ncfAddAttr(ncf, "history", img.historyPlusEvent("Exported to Met_Reproj-style NetCDF").c_str());
  ncfAddAttr(ncf, "title", img.historyPlusEvent("Exported to Met_Reproj-style NetCDF").c_str());

  NcDim *tdim = ncf.add_dim("time");
  if (!tdim->is_valid()) throw std::runtime_error("adding time dimension failed");
  NcDim *londim = ncf.add_dim("longitude", img.data->columns);
  if (!londim->is_valid()) throw std::runtime_error("adding longitude dimension failed");
  NcDim *latdim = ncf.add_dim("latitude", img.data->lines);
  if (!latdim->is_valid()) throw std::runtime_error("adding latitude dimension failed");


  NcVar *var = ncf.add_var("time", ncDouble, tdim);
  if (!var->is_valid()) throw std::runtime_error("adding lon variable failed");
  ncfAddAttr(*var, "long_name", "Time");
	ncfAddAttr(*var, "units", "seconds since 2000-01-01 00:00:00 UTC");
  double atime = (double)img.forecastSeconds2000();
	if (!var->put(&atime, 1))
		throw std::runtime_error("writing time values failed");

  var = ncf.add_var("longitude", ncFloat, londim);
  if (!var->is_valid()) throw std::runtime_error("adding lon variable failed");
	ncfAddAttr(*var, "long_name", "longitude");
	ncfAddAttr(*var, "units", "degrees_east");
	ncfAddAttr(*var, "modulo", 360.0);
  ncfAddAttr(*var, "topology", "circular");
  if (!var->put(lons, xsize))
		throw std::runtime_error("writing longitude values failed");

  var = ncf.add_var("latitude", ncFloat, latdim);
  if (!var->is_valid()) throw std::runtime_error("adding lat variable failed");
	ncfAddAttr(*var, "long_name", "latitude");
	ncfAddAttr(*var, "units", "degrees_north");
  if (!var->put(lats, ysize))
		throw std::runtime_error("writing latitude values failed");

  var = ncf.add_var(img.shortName.c_str(), ncFloat, tdim, latdim, londim);
  if (!var->is_valid()) throw std::runtime_error("adding " + img.shortName + " variable failed");
	ncfAddAttr(*var, "axis", "TYX");
	ncfAddAttr(*var, "long_name", img.shortName.c_str());
	ncfAddAttr(*var, "units", img.unit.c_str());
	ncfAddAttr(*var, "add_offset", 0.0);
  ncfAddAttr(*var, "scale_factor", 1.0);
  ncfAddAttr(*var, "chnum", img.channel_id);
	//ncfAddAttr(*var, "_FillValue", (float)FP_NAN);
  if (!var->put(data, 1, ysize, xsize))
		throw std::runtime_error("writing image values failed");
	
  // Close NetCDF output
  (void) ncf.close( );

//  cout << "Wrote file " << NcName << "." << endl;
}

struct RepNetCDFExporter : public ImageConsumer
{
	RepNetCDFExporter() {}
	virtual void processImage(auto_ptr<Image> img)
	{
		ExportRepNetCDF(*img, img->defaultFilename + "_rep.nc");
	}
};

std::auto_ptr<ImageConsumer> createRepNetCDFExporter()
{
	return std::auto_ptr<ImageConsumer>(new RepNetCDFExporter());
}

}

// vim:set ts=2 sw=2:
