//---------------------------------------------------------------------------
//
//  File        :   HRI2NetCDF.cpp
//  Description :   Export Meteosat HRI format in NetCDF format
//  Project     :   Lamma 2003
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

#include <config.h>

#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <cstring>

// Unidata NetCDF

#include <netcdfcpp.h>

// HRI format interface

#include <msat/hri/HRI.h>

#define INSTITUTION "LaMMA Regione Toscana"

//
// Creates NetCDF product
//
static bool NetCDFProduct(char *inname)
{
  struct tm *tmtime;
  char NcName[1024];
  char title[128];
  char reftime[64];
  char projname[16];
  int wd, hg;
  int bpp;
  geolocation *geo;
  float *cal;
  NcVar *ivar;
  NcVar *tvar;
  NcDim *tdim;
  NcDim *ldim;
  NcDim *cdim;
  NcDim *caldim;

  Hri hri;

  hri.readfrom(inname, false);
  
  tmtime = hri.get_datetime( );

  for (int i = 0; i < hri.nimages; i ++)
  {
    // Build up output NetCDF file name and open it
    sprintf( NcName, "%s_%s_%s_%4d%02d%02d_%02d%02d.nc",
             hri.get_satellite_name( ), hri.get_area_name( ),
             hri.image[i].name.c_str( ),
             tmtime->tm_year + 1900, tmtime->tm_mon + 1, tmtime->tm_mday,
	   tmtime->tm_hour, tmtime->tm_min );
    NcFile ncf ( NcName , NcFile::Replace );
    if (! ncf.is_valid()) return false;

    // Fill arrays on creation
    ncf.set_fill(NcFile::Fill);

    // Add Global Attributes
    if (! ncf.add_att("Satellite", hri.get_satellite_name( ))) return false;
    sprintf(reftime, "%04d-%02d-%02d %02d:%02d:00 UTC",
        tmtime->tm_year + 1900, tmtime->tm_mon + 1, tmtime->tm_mday,
        tmtime->tm_hour, tmtime->tm_min);
    if (! ncf.add_att("Antenna", "Fixed") ) return false;
    if (! ncf.add_att("Receiver", "HIMET") ) return false;
    if (! ncf.add_att("Time", reftime) ) return false;
    if (! ncf.add_att("Area_Name", hri.get_area_name( ) ) ) return false;
    sprintf(projname, "GEOS(%3.1f)", hri.get_satellite_longitude( ));
    if (! ncf.add_att("Projection", projname) ) return false;
    if (! ncf.add_att("Columns", hri.image[i].npixels) ) return false;
    if (! ncf.add_att("Lines", hri.image[i].nlines) ) return false;
    if (! ncf.add_att("SampleX", hri.image[i].samplex) ) return false;
    if (! ncf.add_att("SampleY", hri.image[i].sampley) ) return false;
    geo = hri.get_geolocation( );
    if (! ncf.add_att("Column_Scale_Factor", geo->CFAC) ) return false;
    if (! ncf.add_att("Line_Scale_Factor", geo->LFAC) ) return false;
    if (! ncf.add_att("Column_Offset", geo->COFF) ) return false;
    if (! ncf.add_att("Line_Offset", geo->LOFF) ) return false;
    if (! ncf.add_att("Orbit_Radius", hri.get_orbit_radius( )) ) return false;
    if (! ncf.add_att("Longitude", hri.get_satellite_longitude( )) )
         return false;
    if (! ncf.add_att("NortPolar", hri.get_northpolar( )) ) return false;
    if (! ncf.add_att("NorthSouth", hri.get_northsouth( )) ) return false;
    if (! ncf.add_att("Institution", INSTITUTION) ) return false;
    if (! ncf.add_att("Conventions", "COARDS") ) return false;
    if (! ncf.add_att("history", "Created from raw data") ) return false;
    snprintf(title, 128, "%s product for %s", hri.get_satellite_name( ), reftime);
    if (! ncf.add_att("title", title) ) return false;

    // Dimensions
    wd = hri.image[i].npixels;
    hg = hri.image[i].nlines;
    bpp = (int) pow(2.0, hri.get_bits_per_pixel( ));

    tdim = ncf.add_dim("time");
    if (!tdim->is_valid()) return false;
    ldim = ncf.add_dim("line", hg);
    if (!ldim->is_valid()) return false;
    cdim = ncf.add_dim("column", wd);
    if (!cdim->is_valid()) return false;
    caldim = ncf.add_dim("calibration", bpp);
    if (!caldim->is_valid()) return false;

    // Get calibration values
    cal = hri.image[i].get_calibration( );

    // Add Calibration values
    NcVar *cvar = ncf.add_var("calibration", ncFloat, caldim);
    if (!cvar->is_valid()) return false;
    cvar->add_att("long_name", "Calibration coefficients");
    cvar->add_att("variable", hri.image[i].name.c_str( ));
    cvar->add_att("units", hri.image[i].units.c_str( ));
    if (!cvar->put(cal, bpp)) return false;

    tvar = ncf.add_var("time", ncDouble, tdim);
    if (!tvar->is_valid()) return false;
    tvar->add_att("long_name", "Time");
    tvar->add_att("units", "seconds since 2000-01-01 00:00:00 UTC");
    double atime;
    time_t ttime;
    extern long timezone;
    ttime = mktime(tmtime);
    atime = ttime - 946684800 - timezone;
    if (!tvar->put(&atime, 1)) return false;

    ivar = ncf.add_var(hri.image[i].name.c_str( ), ncByte, tdim, ldim, cdim);
    if (!ivar->is_valid()) return false;
    if (!ivar->add_att("add_offset", 0.0)) return false;
    if (!ivar->add_att("scale_factor", 1.0)) return false;
    if (!ivar->add_att("chnum", hri.image[i].get_image_band( ))) return false;

    // Write output values
    if (!ivar->put((const ncbyte*)hri.image[i].get_image( ), 1, hg, wd)) return false;

    // Close NetCDF output
    (void) ncf.close( );

  }

  return( true );
}

/* ************************************************************************* */
/* Reads Data Images, performs calibration and store output in NetCDF format */
/* ************************************************************************* */

int main( int argc, char* argv[] )
{
  if (argc < 2)
  {
    std::cerr << "Usage: " << argv[0] << " HRI_file" << std::endl;
    return 1;
  }
  if (!strcmp(argv[1], "-V"))
  {
    std::cout << argv[0] << " " << PACKAGE_STRING << std::endl;
    return 0;
  }
  if (!NetCDFProduct(argv[1]))
  {
    std::cerr << "Created NetCDF product NOT OK" << std::endl;
    return 1;
  }

  return(0);
}

//---------------------------------------------------------------------------
