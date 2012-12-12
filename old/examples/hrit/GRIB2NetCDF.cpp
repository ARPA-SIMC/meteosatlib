//---------------------------------------------------------------------------
//
//  File        :   GRIB2NetCDF.cpp
//  Description :   Export MSG HRIT format in NetCDF format
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
#include <cstring>

#include "config.h"

// Unidata NetCDF

#include <netcdfcpp.h>

// HRI format interface

#include <hrit/MSG_HRIT.h>

// GRIB Interface

#include <grib/GRIB.h>

#define TITLE "Observation File from MSG-SEVIRI"
#define INSTITUTION "HIMET"
#define TYPE "Obs file"
#define HIMET_VERSION 0.0

#define PATH_SEPARATOR "/"
// For windows use #define PATH_SEPARATOR "\"

void usage(char *pname);
char *chname(char *chdesc, int len);
bool NetCDFProduct(char *gribname);

/* ************************************************************************* */
/* Reads Data Images, performs calibration and store output in NetCDF format */
/* ************************************************************************* */

int main( int argc, char* argv[] )
{
  char *pname = strdup(argv[0]);
  if (argc != 2)
  {
    usage(pname);
    return 1;
  }

  if (! strcmp(argv[1], "-V"))
  {
    std::cout << pname << " " << PACKAGE_STRING << std::endl;
    return 0;
  }

  char *gname = strdup(argv[1]);

  std::cout << "Writing NetCDF output..." << std::endl;
  if(!NetCDFProduct(gname))
    throw "Created NetCDF product NOT OK";

  return(0);
}

void usage(char *pname)
{
  std::cout << pname << ": Convert GRIB files to NetCDF format."
	    << std::endl;
  std::cout << std::endl << "Usage:" << std::endl << "\t"
            << pname << " filename.grb"
            << std::endl << std::endl
            << "Example: " << std::endl << "\t" << pname
            << " MSG_Seviri_1_5_Water_Vapour_6_2_channel_20041228_1345.grb"
            << std::endl;
  return;
}

char *chname(char *chdesc, int len)
{
  char *name = new char[len];
  for (int i = 0; i < len; i ++)
    if (chdesc[i] == ' ' || chdesc[i] == '.') name[i] = '_';
    else name[i] = chdesc[i];
  return name;
}


//
// Creates NetCDF product
//
bool NetCDFProduct(char *gribname)
{
  char NcName[1024];
  // char title[64];
  char reftime[64];
  char projname[16];
  NcVar *ivar;
  NcVar *tvar;
  NcDim *tdim;
  NcDim *ldim;
  NcDim *cdim;

  GRIB_FILE gf;
  GRIB_MESSAGE m;
 
  int ret = gf.OpenRead(gribname);
  if (ret != 0) return -1;

  ret = gf.ReadMessage(m);
  if (ret != 0) return -1;

  int npix = m.grid.nx;
  int nlin = m.grid.ny;

  size_t total_size = m.grid.nxny;
  float *pixels = new float[total_size];
  memcpy(pixels, m.field.vals, total_size*sizeof(float));

  GRIB_MSG_PDS pds;
  
  if (m.get_pds_size( ) < sizeof(GRIB_MSG_PDS) + 20)
  {
    std::cerr << "Does not seem to have information I need in the PDS"
              << std::endl;
    throw;
  }

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

//---------------------------------------------------------------------------
