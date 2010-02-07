//---------------------------------------------------------------------------
//
//  File        :   ExportGDTNetCDF.cpp
//  Description :   Export data from an ImageData into a NetCDF file usign GDT conventions
//  Author      :   Enrico Zini (for ARPA SIM Emilia Romagna)
//  GDT link    :   http://www-pcmdi.llnl.gov/drach/GDT_convention.html
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

#include <msat/ExportGDTNetCDF.h>
#include <msat/facts.h>
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
// Creates GDT NetCDF product
//
void ExportGDTNetCDF(/*const GDTExportOptions& opts,*/ const Image& img, const std::string& fileName)
{
	// Check that Image has a Latlon projection, otherwise we cannot encode it
	// with GDT
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

  NcDim *londim = ncf.add_dim("lon", xsize);
  if (!londim->is_valid()) throw std::runtime_error("adding lon dimension failed");
  NcDim *latdim = ncf.add_dim("lat", ysize);
  if (!latdim->is_valid()) throw std::runtime_error("adding lat dimension failed");

  NcVar *var = ncf.add_var("lon", ncFloat, londim);
  if (!var->is_valid()) throw std::runtime_error("adding lon variable failed");
	ncfAddAttr(*var, "long_name", "longitude");
	ncfAddAttr(*var, "units", "degrees_east");
	ncfAddAttr(*var, "modulo", 360.0);
  ncfAddAttr(*var, "topology", "circular");
  if (!var->put(lons, xsize))
		throw std::runtime_error("writing longitude values failed");

  var = ncf.add_var("lat", ncFloat, latdim);
  if (!var->is_valid()) throw std::runtime_error("adding lat variable failed");
	ncfAddAttr(*var, "long_name", "latitude");
	ncfAddAttr(*var, "units", "degrees_north");
  if (!var->put(lats, ysize))
		throw std::runtime_error("writing latitude values failed");

  var = ncf.add_var(img.shortName.c_str(), ncFloat, latdim, londim);
  if (!var->is_valid()) throw std::runtime_error("adding " + img.shortName + " variable failed");
	ncfAddAttr(*var, "axis", "YX");
	ncfAddAttr(*var, "long_name", img.shortName.c_str());
	ncfAddAttr(*var, "units", img.unit.c_str());
	ncfAddAttr(*var, "add_offset", 0.0);
  ncfAddAttr(*var, "scale_factor", 1.0);
  ncfAddAttr(*var, "chnum", img.channel_id);
  if (!var->put(data, ysize, xsize))
		throw std::runtime_error("writing image values failed");
	
#if 0	
	// Interpolate and fetch the value, makes it possible to then compute z_range
	// and also X and Y dimensions

	float data[opts.xsize() * opts.ysize()];
	float lats[opts.ysize()];
	float lons[opts.xsize()];
	for (int y = 0; y < opts.ysize(); ++y)
	{
		for (int x = 0; x < opts.xsize(); ++x)
		{
			int xpix, ypix;
			img.coordsToPixels(opts.lat(y), opts.lon(x), xpix, ypix);
			float datum = img.data->scaled(xpix, ypix);
			if (datum == img.data->missingValue)
				data[y * opts.xsize() + x] = FP_NAN;
			else
				data[y * opts.xsize() + x] = datum;
		}
		lats[y] = opts.lat(y);
	}
	for (int x = 0; x < opts.xsize(); ++x)
		lons[x] = opts.lon(x);

  NcDim *londim = ncf.add_dim("lon", opts.xsize());
  if (!londim->is_valid()) throw std::runtime_error("adding lon dimension failed");
  NcDim *latdim = ncf.add_dim("lat", opts.ysize());
  if (!latdim->is_valid()) throw std::runtime_error("adding lat dimension failed");

  NcVar *var = ncf.add_var("lon", ncFloat, londim);
  if (!var->is_valid()) throw std::runtime_error("adding lon variable failed");
	ncfAddAttr(*var, "long_name", "longitude");
	ncfAddAttr(*var, "units", "degrees_east");
	ncfAddAttr(*var, "modulo", 360.0);
  ncfAddAttr(*var, "topology", "circular");
  if (!var->put(lons, opts.xsize()))
		throw std::runtime_error("writing longitude values failed");

  var = ncf.add_var("lat", ncFloat, latdim);
  if (!var->is_valid()) throw std::runtime_error("adding lat variable failed");
	ncfAddAttr(*var, "long_name", "latitude");
	ncfAddAttr(*var, "units", "degrees_north");
  if (!var->put(lats, opts.ysize()))
		throw std::runtime_error("writing latitude values failed");

  var = ncf.add_var(channelstring.c_str(), ncFloat, latdim, londim);
  if (!var->is_valid()) throw std::runtime_error("adding " + channelstring + " variable failed");
	ncfAddAttr(*var, "axis", "YX");
	ncfAddAttr(*var, "long_name", channelstring.c_str());
  if (img.channel_id > 3 && img.channel_id < 12)
    ncfAddAttr(*var, "units", "K");
  else
    ncfAddAttr(*var, "units", "mW m^-2 sr^-1 (cm^-1)^-1");
	ncfAddAttr(*var, "add_offset", 0.0);
  ncfAddAttr(*var, "scale_factor", 1.0);
  ncfAddAttr(*var, "chnum", img.channel_id);
  if (!var->put(data, opts.ysize(), opts.xsize()))
		throw std::runtime_error("writing image values failed");
#endif

















	/*
	float range[2];
  NcVar *var = ncf.add_var("x_range", ncFloat, sdim);
  if (!var->is_valid()) throw std::runtime_error("adding x_range variable failed");
	var->add_att("units", "degrees");
	range[0] = opts.lonmin;
	range[1] = opts.lonmax;
  if (!var->put(range, 2))
		throw std::runtime_error("writing x_range values failed");

  var = ncf.add_var("y_range", ncFloat, sdim);
  if (!var->is_valid()) throw std::runtime_error("adding y_range variable failed");
	var->add_att("units", "degrees");
	range[0] = opts.latmin;
	range[1] = opts.latmax;
  if (!var->put(range, 2))
		throw std::runtime_error("writing y_range values failed");

  var = ncf.add_var("spacing", ncFloat, sdim);
  if (!var->is_valid()) throw std::runtime_error("adding spacing variable failed");
	//var->add_att("units", "degrees");
	range[0] = opts.lonstep;
	range[1] = opts.latstep;
  if (!var->put(range, 2))
		throw std::runtime_error("writing spacing values failed");

  var = ncf.add_var("dimension", ncFloat, sdim);
  if (!var->is_valid()) throw std::runtime_error("adding dimension variable failed");
	//var->add_att("units", "degrees");
	range[0] = opts.lonstep;
	range[1] = opts.latstep;
  if (!var->put(range, 2))
		throw std::runtime_error("writing dimension values failed");
	*/



#if 0
  // Add Global Attributes
  //cerr << "global." << endl;

  ncfAddAttr(ncf, "Satellite", Image::spacecraftName(img.spacecraft_id).c_str());
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
  ncfAddAttr(ncf, "AreaStartPix", img.x0 );
  //if (! ncf.add_att("AreaStartPix", (int) m.grid.sp.X0 ) ) return false;
  ncfAddAttr(ncf, "AreaStartLin", img.y0 );
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
  ncfAddAttr(ncf, "history", "Created from SAF HDF5 data");

  // Dimensions
  //cerr << "dimensions." << endl;

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
#endif

  // Close NetCDF output
  (void) ncf.close( );

//  cout << "Wrote file " << NcName << "." << endl;
}

class GDTNetCDFExporter : public ImageConsumer
{
	//GDTExportOptions opts;
public:
	GDTNetCDFExporter(/*const GDTExportOptions& opts*/) /*: opts(opts)*/ {}
	virtual void processImage(auto_ptr<Image> img)
	{
		ExportGDTNetCDF(/*opts,*/ *img, img->defaultFilename + ".grd");
	}
};

std::auto_ptr<ImageConsumer> createGDTNetCDFExporter(/*const GDTExportOptions& opts*/)
{
	return std::auto_ptr<ImageConsumer>(new GDTNetCDFExporter(/*opts*/));
}

}

// vim:set ts=2 sw=2:
