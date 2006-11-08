//---------------------------------------------------------------------------
//
//  File        :   ExportNetCDF24.cpp
//  Description :   Export data from an ImageData into a NetCDF24 file
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

//#include "../config.h"

#include <conv/ExportNetCDF24.h>
#include <proj/Geos.h>
#include <grib/GRIB.h>

#include <netcdfcpp.h>

#include <sstream>
#include <stdexcept>

// For MSG_channel_name
#include <hrit/MSG_HRIT.h>

#include <proj/const.h>
#include "NetCDFUtils.h"

using namespace std;

namespace msat {

//
// Creates NetCDF24 product
//
void ExportNetCDF24(const Image& img, const std::string& fileName)
{
  // Build up output NetCDF file name and open it
  NcFile ncf(fileName.c_str(), NcFile::Replace);
  if (!ncf.is_valid())
    throw std::runtime_error("Creating a NcFile structure for file " + fileName);

  // Fill arrays on creation
  ncf.set_fill(NcFile::Fill);

  // Add Global Attributes
	ncfAddAttr(ncf, "GribEditionNumber", 1);
	ncfAddAttr(ncf, "GeneratingCentre", 200);
	ncfAddAttr(ncf, "GeneratingSubCentre", 0);
	ncfAddAttr(ncf, "GeneratingProcess", 254);	// FIXME: unsure
	ncfAddAttr(ncf, "Parameter", GRIB_PARAMETER_IMG_D);
	ncfAddAttr(ncf, "LevelType", GRIB_LEVEL_SATELLITE_METEOSAT8);
	ncfAddAttr(ncf, "L1", img.channel_id);
	ncfAddAttr(ncf, "L2", 0);
	ncfAddAttr(ncf, "Year", img.year);
	ncfAddAttr(ncf, "Month", img.month);
	ncfAddAttr(ncf, "Day", img.day);
	ncfAddAttr(ncf, "Hour", img.hour);
	ncfAddAttr(ncf, "Minute", img.minute);
	ncfAddAttr(ncf, "SatelliteID", img.spacecraft_id);

// They don't seem to be currently filled up in the GRIB exporter
//	ncfAddAttr(ncf, "TimeRange", );
//	ncfAddAttr(ncf, "TimeRangeUnits", );
//	ncfAddAttr(ncf, "P1", );
//	ncfAddAttr(ncf, "P2", );

  NcDim *nameDim = ncf.add_dim("name", 1);
  if (!nameDim->is_valid()) throw std::runtime_error("adding name dimension failed");

  NcVar *projVar = ncf.add_var("Projection", ncInt);
  if (!projVar->is_valid()) throw std::runtime_error("adding projection variable failed");

	if (proj::Geos* p = dynamic_cast<proj::Geos*>(img.proj.get()))
	{
		ncfAddAttr(*projVar, "Lap", 0.0);
		ncfAddAttr(*projVar, "Lop", p->sublon);
		ncfAddAttr(*projVar, "DX", img.seviriDX());
		ncfAddAttr(*projVar, "DY", img.seviriDY());
		ncfAddAttr(*projVar, "Xp", img.column_offset);
		ncfAddAttr(*projVar, "Yp", img.line_offset);
		ncfAddAttr(*projVar, "X0", img.x0);
		ncfAddAttr(*projVar, "Y0", img.y0); 
		ncfAddAttr(*projVar, "Orientation", SEVIRI_ORIENTATION);
		ncfAddAttr(*projVar, "Nz", SEVIRI_CAMERA_H);
	} else
		throw std::runtime_error("image has projection " + (img.proj.get() ? img.proj->format() : "(null)") + " instead of geostationary");

	// Does not seem to be currently filled up in the GRIB exporter
	//	ncfAddAttr(*projVar, "ScanningMode", );

  //ncfAddAttr(ncf, "AreaStartPix", (int) 3712/2-img.column_offset + 1 );
  //if (! ncf.add_att("AreaStartPix", (int) m.grid.sp.X0 ) ) return false;
  //ncfAddAttr(ncf, "AreaStartLin", (int) 3712/2-img.line_offset + 1 );
  //if (! ncf.add_att("AreaStartLin", (int) m.grid.sp.Y0 ) ) return false;

  //ncfAddAttr(ncf, "title", TITLE);
  //ncfAddAttr(ncf, "Institution", INSTITUTION);
  //ncfAddAttr(ncf, "Type", TYPE);
  //ncfAddAttr(ncf, "Version", ARPA_VERSION);
  //ncfAddAttr(ncf, "Conventions", "COARDS");
  //ncfAddAttr(ncf, "history", "Created from SAF HDF5 data");

  // Dimensions
  NcDim *tdim = ncf.add_dim("time");
  if (!tdim->is_valid()) throw std::runtime_error("adding time dimension failed");
  NcDim *ldim = ncf.add_dim("line", img.data->lines);
  if (!ldim->is_valid()) throw std::runtime_error("adding line dimension failed");
  NcDim *cdim = ncf.add_dim("column", img.data->columns);
  if (!cdim->is_valid()) throw std::runtime_error("adding column dimension failed");

  NcVar *tvar = ncf.add_var("time", ncDouble, tdim);
  if (!tvar->is_valid()) throw std::runtime_error("adding time variable failed");
  tvar->add_att("long_name", "Time");
  tvar->add_att("units", "seconds since 2000-01-01 00:00:00 UTC");
  double atime = (double) img.forecastSeconds2000();
  if (!tvar->put(&atime, 1)) throw std::runtime_error("setting time variable failed");

	stringstream channelName;
	channelName << "Channel" << img.channel_id;

	if (img.data->scalesToInt)
	{
		std::auto_ptr<NcEncoder> enc = createEncoder(img.data->bpp);
		NcVar *ivar = ncf.add_var(channelName.str().c_str(), enc->getType(), tdim, ldim, cdim);
		if (!ivar->is_valid()) throw std::runtime_error("adding " + channelName.str() + " variable failed");
		ncfAddAttr(*ivar, "add_offset", img.data->offset);
		ncfAddAttr(*ivar, "scale_factor", img.data->slope);
		ncfAddAttr(*ivar, "channel", img.channel_id);
		ncfAddAttr(*ivar, "channelName",
				MSG_channel_name((t_enum_MSG_spacecraft)img.spacecraft_id, img.channel_id).c_str());

		if (img.channel_id > 3 && img.channel_id < 12)
			ivar->add_att("units", "K");
		else
			ivar->add_att("units", "mW m^-2 sr^-1 (cm^-1)^-1");

		// Write output values
		enc->setData(*ivar, img);
	} else {
		NcVar *ivar = ncf.add_var(channelName.str().c_str(), ncFloat, tdim, ldim, cdim);
		if (!ivar->is_valid()) throw std::runtime_error("adding " + channelName.str() + " variable failed");
		ncfAddAttr(*ivar, "add_offset", 0.0);
		ncfAddAttr(*ivar, "scale_factor", 1.0);
		ncfAddAttr(*ivar, "channel", img.channel_id);
		ncfAddAttr(*ivar, "channelName",
				MSG_channel_name((t_enum_MSG_spacecraft)img.spacecraft_id, img.channel_id).c_str());

		if (img.channel_id > 3 && img.channel_id < 12)
			ivar->add_att("units", "K");
		else
			ivar->add_att("units", "mW m^-2 sr^-1 (cm^-1)^-1");

		// Write output values
		float *pixels = img.data->allScaled();
		if (!ivar->put(pixels, 1, img.data->lines, img.data->columns))
			throw std::runtime_error("writing image values failed");
		delete [ ] pixels;
	}

  // Close NetCDF output
  (void) ncf.close( );
}


class NetCDF24Exporter : public ImageConsumer
{
public:
	virtual void processImage(auto_ptr<Image> img)
	{
		ExportNetCDF24(*img, img->defaultFilename() + ".nc");
	}
};

std::auto_ptr<ImageConsumer> createNetCDF24Exporter()
{
	return std::auto_ptr<ImageConsumer>(new NetCDF24Exporter());
}

}

// vim:set ts=2 sw=2:
