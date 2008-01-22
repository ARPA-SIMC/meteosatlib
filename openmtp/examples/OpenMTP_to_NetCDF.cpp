//---------------------------------------------------------------------------
//
//  File        :   OpenMTP_to_NetCDF.cpp
//  Description :   Export Meteosat OpenMTP format in NetCDF format
//  Project     :   Cetemps 2003
//  Author      :   Graziano Giuliani
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

// Unidata NetCDF

#include <netcdfcpp.h>

// OpenMTP format interface

#include <openmtp/OpenMTP.h>

#define INSTITUTION "LaMMA Regione Toscana"

//
// Creates NetCDF product
//
bool NetCDFProduct( char *inpath )
{
  OpenMTP omtp;
  struct tm tmtime;
  char NcName[1024];
  char title[64];
  char reftime[64];
  int cfac, lfac, coff, loff;
  float scale;
  int wd, hg;
  int bpp;
  float *cal;
  NcVar *ivar;
  NcVar *tvar;
  NcDim *tdim;
  NcDim *ldim;
  NcDim *cdim;
  NcDim *caldim;

  omtp.open( inpath );

  tmtime = omtp.get_datetime( );

  // Build up output NetCDF file name and open it
  sprintf( NcName, "%s_%s_%s.nc", omtp.get_satellite_code( ),
           omtp.get_field_name( ), omtp.get_timestr( ));
  NcFile ncf ( NcName , NcFile::Replace );
  if (! ncf.is_valid()) return false;

  // Fill arrays on creation
  ncf.set_fill(NcFile::Fill);

  // Add Global Attributes
  if (! ncf.add_att("Satellite", omtp.get_satellite_name( ))) return false;
  if (! ncf.add_att("Antenna", "Meteosat Archive")) return false;
  if (! ncf.add_att("Receiver", "Meteosat Archive")) return false;
  sprintf(reftime, "%04d-%02d-%02d %02d:%02d:00 UTC",
      tmtime.tm_year + 1900, tmtime.tm_mon + 1, tmtime.tm_mday,
      tmtime.tm_hour, tmtime.tm_min);
  if (! ncf.add_att("Time", reftime) ) return false;

  if (omtp.is_A_format( ))
  {
    if (! ncf.add_att("Area_Name", "AFormat" ) ) return false;
    if (omtp.is_ir_data( ) || omtp.is_wv_data( ))
    {
      cfac = -9102222;
      lfac = -9102222;
      coff = 1248;
      loff = 1249;
      scale = 1.0;
    }
    else if (omtp.is_vis_data( ))
    {
      cfac = -18204444;
      lfac = -18204444;
      coff = 2500;
      loff = 2500;
      scale = 1.0;
    }
    else
      throw "Unsupported AFormat image (?)\n";
  }
  else if (omtp.is_B_format( ))
  {
    if (! ncf.add_att("Area_Name", "BFormat" ) ) return false;
    cfac = -18204444;
    lfac = -18204444;
    coff = 1248;
    loff = -1118;
    scale = 2.0;
  }
  else
    throw "Currently supporting only A or B format images\n";

  if (! ncf.add_att("Projection", "GEOS(0.0)") ) return false;
  if (! ncf.add_att("Columns", omtp.npixels( ) * (int) scale) ) return false;
  if (! ncf.add_att("Lines", omtp.nlines( ) * (int) scale) ) return false;
  if (! ncf.add_att("SampleX", scale) ) return false;
  if (! ncf.add_att("SampleY", scale) ) return false;
  if (! ncf.add_att("Column_Scale_Factor", cfac) ) return false;
  if (! ncf.add_att("Line_Scale_Factor", lfac) ) return false;
  if (! ncf.add_att("Column_Offset", coff) ) return false;
  if (! ncf.add_att("Line_Offset", loff) ) return false;
  if (! ncf.add_att("Orbit_Radius", omtp.orbit_radius( )) ) return false;
  if (! ncf.add_att("Longitude", omtp.subsatellite_point( )) ) return false;
//  if (! ncf.add_att("NortPolar", omtp.is_NortPolar( )) ) return false;
//  if (! ncf.add_att("NorthSouth", omtp.is_NorthSouth( )) ) return false;
  if (! ncf.add_att("Institution", INSTITUTION) ) return false;
  if (! ncf.add_att("Conventions", "COARDS") ) return false;
  if (! ncf.add_att("history", "Created from OpenMTP format data") )
    return false;
  sprintf(title, "OpenMTP %s product for %04d-%02d-%02d %02d:%02d:00 UTC",
            omtp.get_field_name( ), tmtime.tm_year + 1900, tmtime.tm_mon + 1,
            tmtime.tm_mday, tmtime.tm_hour, tmtime.tm_min);
  if (! ncf.add_att("title", title) ) return false;

  // Dimensions
  wd = omtp.npixels( );
  hg = omtp.nlines( );
  bpp = (int) pow(2.0, omtp.bits_per_pixel( ));

  // Define dimensions
  tdim = ncf.add_dim("time");
  if (!tdim->is_valid()) return false;
  ldim = ncf.add_dim("line", hg);
  if (!ldim->is_valid()) return false;
  cdim = ncf.add_dim("column", wd);
  if (!cdim->is_valid()) return false;
  caldim = ncf.add_dim("calibration", bpp);
  if (!caldim->is_valid()) return false;

  // Get calibration values
  cal = omtp.get_calibration( );

  // Add Calibration values
  NcVar *cvar = ncf.add_var("calibration", ncFloat, caldim);
  if (!cvar->is_valid()) return false;
  cvar->add_att("long_name", "Calibration coefficients");
  cvar->add_att("variable", omtp.get_chname( ));
  cvar->add_att("units", omtp.get_chunit( ));
  if (!cvar->put(cal, bpp)) return false;

  tvar = ncf.add_var("time", ncDouble, tdim);
  if (!tvar->is_valid()) return false;
  tvar->add_att("long_name", "Time");
  tvar->add_att("units", "seconds since 2000-01-01 00:00:00 UTC");
  double atime;
  time_t ttime;
  extern long timezone;
  ttime = mktime(&tmtime);
  atime = ttime - 946684800 - timezone;
  if (!tvar->put(&atime, 1)) return false;

  // Define channel values variable
  ivar = ncf.add_var(omtp.get_chname( ), ncByte, tdim, ldim, cdim);
  if (!ivar->is_valid()) return false;
  if (!ivar->add_att("add_offset", 0.0)) return false;
  if (!ivar->add_att("scale_factor", 1.0)) return false;

  // Write output values
  if (!ivar->put((ncbyte*)omtp.get_image( ), 1, hg, wd)) return false;

  // Close NetCDF output
  (void) ncf.close( );

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
