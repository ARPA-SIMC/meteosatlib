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

#include <conv/Image.h>

// GRIB Interface

#include <GRIB.h>

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

namespace msat {

bool isGrib(const std::string& filename)
{
	if (access(filename.c_str(), F_OK) != 0)
		return false;
	return filename.substr(filename.size() - 4) == ".grb";
}

auto_ptr<Image> importGrib(GRIB_MESSAGE& m)
{
	// Read image data
	auto_ptr< ImageDataWithPixels<float> > res(new ImageDataWithPixels<float>(m.grid.nx, m.grid.ny));
  memcpy(res->pixels, m.field.vals, m.grid.nxny * sizeof(float));

	auto_ptr< Image > img(new Image());
	img->setData(res.release());

	img->year = m.gtime.year;
	img->month = m.gtime.month;
	img->day = m.gtime.day;
	img->hour = m.gtime.hour;
	img->minute = m.gtime.minute;

	switch (m.grid.type)
	{
		case GRIB_GRID_SPACEVIEW:
			img->column_offset = (int)m.grid.sp.X0;	// probably need some (x-1)*2
			img->line_offset = (int)m.grid.sp.Y0;		// probably need some (x-1)*2
			img->sublon = m.grid.sp.lop;
			break;
		default:
		{
			std::stringstream str;
			str << "GRIB projection " << m.grid.type << " is not supported";
			throw std::runtime_error(str.str());
		}
	}

	switch (m.level.type)
	{
		case GRIB_LEVEL_SATELLITE_METEOSAT8:
			img->channel_id = (int)m.level.lv1 << 8 + (int)m.level.lv2;
			break;
		default:
		{
			std::stringstream str;
			str << "GRIB level " << m.level.type << " is not supported";
			throw std::runtime_error(str.str());
		}
	}

#if 0
	-- TODO:
  std::string name;
  int spacecraft_id;
  int column_factor;
  int line_factor;
#endif

#if 0
  GRIB_MSG_PDS pds;

  if (m.get_pds_size() < sizeof(GRIB_MSG_PDS) + 20)
    throw std::runtime_error("The Grib PDS does not seem to have information I need");

  unsigned char *tmp = m.get_pds_values(m.get_pds_size( ) -
         sizeof(GRIB_MSG_PDS), sizeof(GRIB_MSG_PDS));
  if (tmp == 0)
  {
    std::cerr << "GRIB seems not MSG HIMET extended..." << std::endl;
    throw;
  }
  memcpy(&pds, tmp, sizeof(GRIB_MSG_PDS));
  delete [ ] tmp;

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
  if (! ncf.add_att("Column_Scale_Factor", pds.cfac) ) return false;
  if (! ncf.add_att("Line_Scale_Factor", pds.lfac) ) return false;
  if (! ncf.add_att("Column_Offset", pds.coff) ) return false;
  if (! ncf.add_att("Line_Offset", pds.loff) ) return false;
  if (! ncf.add_att("Orbit_Radius", pds.sh) ) return false;
#endif 

  return img;
}


class GribImageImporter : public ImageImporter
{
	std::string filename;

public:
	GribImageImporter(const std::string& filename)
		: filename(filename) {}

	virtual void read(ImageConsumer& output)
	{
		GRIB_FILE gf;
		GRIB_MESSAGE m;
	 
		if (gf.OpenRead(filename) != 0)
			throw std::runtime_error("cannot open Grib file " + filename);

		int count = 0;
		while (gf.ReadMessage(m) == 0)
		{
			auto_ptr<Image> img = importGrib(m);
			output.processImage(*img);
			++count;
		}
		if (count == 0)
			throw std::runtime_error("cannot read any Grib messages from file " + filename);
	}
};

std::auto_ptr<ImageImporter> createGribImporter(const std::string& filename)
{
	return std::auto_ptr<ImageImporter>(new GribImageImporter(filename));
}

}

// vim:set ts=2 sw=2:
