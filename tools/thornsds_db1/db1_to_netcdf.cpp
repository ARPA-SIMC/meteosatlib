//-----------------------------------------------------------------------------
//
//  File        : db1_to_netcdf.cpp
//  Description : Transform ThornSDS MSG file to NetCDF format
//  Author      : Graziano Giuliani
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
//-----------------------------------------------------------------------------

#include <config.h>

#include <iostream>
#include <msat/thornsds_db1/thornsds_db1.h>
#include <netcdfcpp.h>

#define TITLE "Observation File from MSG-SEVIRI"
#define INSTITUTION "CETEMPS-AQUILA"
#define TYPE "Obs file"
#define NCVERSION 0.0

int main (int argc, char *argv[])
{
  MSG_db1_data db1;

  if (argc > 1)
  {
    if (!strcmp(argv[1], "-V"))
    {
      std::cout << argv[0] << " " << PACKAGE_STRING << std::endl;
      return 0;
    }
  }

  if (argc < 3)
  {
    std::cerr << "Usage: " << argv[0] << " directory channel" << std::endl;
    std::cerr << "Example: " << argv[0] << " dati IR_120" << std::endl;
    return -1;
  }

  db1.open(argv[1]);
  if (! db1.has_channel(argv[2]))
  {
    std::cerr << "Exit: this channel is not present." << std::endl;
    return -1;
  }

  db1.set_channel(argv[2]);

  if (! db1.is_data_ok( ))
  {
    std::cerr << "Exit: Data Open Error." << std::endl;
    return -1;
  }

  char outname[PATH_MAX];
  snprintf(outname, PATH_MAX, "%s_%s_%s.nc", db1.get_INFO_satellite_name( ),
           db1.get_channel_INFO_name( ), db1.get_INFO_schedule_start( ));
  for (int i = 0; i < (int) strlen(outname); i ++)
  {
    if (outname[i] == '/') outname[i] = '-';
    if (outname[i] == ' ') outname[i] = '_';
    if (outname[i] == ':') outname[i] = '-';
  }

  struct tm tmtime;
  char reftime[64];
  int wd = db1.get_INFO_image_pixels( );
  int hg = db1.get_INFO_image_lines( );
  int ncal = db1.get_number_of_calibration( );
  unsigned short *pixels = db1.get_RAW_data( );
  float *cal = db1.get_calibration( );
  NcVar *ivar;
  NcVar *tvar;
  NcDim *tdim;
  NcDim *ldim;
  NcDim *cdim;
  NcDim *caldim;

  float sublon = 0.0;
  int cfac = db1.get_AoI_cfac( );
  int lfac = db1.get_AoI_lfac( );
  int loff = db1.get_AoI_coff( );
  int coff = db1.get_AoI_loff( );
  int spix = 1;
  int slin = 1;
  float sh = 42164.0;

  NcFile ncf ( outname , NcFile::Replace );
  if (! ncf.is_valid()) return false;

  // Fill arrays on creation
  ncf.set_fill(NcFile::Fill);

  // Add Global Attributes
  if (! ncf.add_att("Satellite", db1.get_INFO_satellite_name( )))
	            return false;
  int year,month,day,hour,minute,second;
  sscanf(db1.get_INFO_schedule_start( ), "%02d/%02d/%04d %02d:%02d:%02d.000",
         &day, &month, &year, &hour, &minute, &second);
  sprintf(reftime, "%04d-%02d-%02d %02d:%02d:%02d UTC",
          year, month, day, hour, minute, second);
  if (! ncf.add_att("Antenna", "Fixed") ) return false;
  if (! ncf.add_att("Receiver", db1.get_INFO_station_name( )) ) return false;
  if (! ncf.add_att("Time", reftime) ) return false;
  if (! ncf.add_att("Area_Name", db1.get_AoI_name( ) ) ) return false;
  if (! ncf.add_att("Projection", db1.get_AoI_projection( )) ) return false;
  if (! ncf.add_att("Columns", wd ) ) return false;
  if (! ncf.add_att("Lines", hg ) ) return false;
  if (! ncf.add_att("SampleX", 1.0 ) ) return false;
  if (! ncf.add_att("SampleY", 1.0 ) ) return false;
  if (! ncf.add_att("AreaStartPix", spix ) ) return false;
  if (! ncf.add_att("AreaStartLin", slin ) ) return false;
  if (! ncf.add_att("Column_Scale_Factor", cfac) ) return false;
  if (! ncf.add_att("Line_Scale_Factor", lfac) ) return false;
  if (! ncf.add_att("Column_Offset", coff) ) return false;
  if (! ncf.add_att("Line_Offset", loff) ) return false;
  if (! ncf.add_att("Orbit_Radius", sh) ) return false;
  if (! ncf.add_att("Longitude", sublon) ) return false;
  if (! ncf.add_att("NortPolar", 1) ) return false;
  if (! ncf.add_att("NorthSouth", (! db1.get_INFO_schedule_northsouth( ))) )
      return false;
  if (! ncf.add_att("title", TITLE) ) return false;
  if (! ncf.add_att("Institution", INSTITUTION) ) return false;
  if (! ncf.add_att("Type", TYPE) ) return false;
  if (! ncf.add_att("Version", NCVERSION) ) return false;
  if (! ncf.add_att("Conventions", "COARDS") ) return false;
  if (! ncf.add_att("history", "Created from raw data") ) return false;

  // Dimensions

  tdim = ncf.add_dim("time");
  if (!tdim->is_valid()) return false;
  ldim = ncf.add_dim("line", hg);
  if (!ldim->is_valid()) return false;
  cdim = ncf.add_dim("column", wd);
  if (!cdim->is_valid()) return false;
  caldim = ncf.add_dim("calibration", ncal);
  if (!caldim->is_valid()) return false;

  // Add Calibration values
  NcVar *cvar = ncf.add_var("calibration", ncFloat, caldim);
  if (!cvar->is_valid()) return false;
  cvar->add_att("long_name", "Calibration coefficients");
  cvar->add_att("variable", db1.get_channel_INFO_variable( ));
  cvar->add_att("units", db1.get_channel_INFO_units( ));
  if (!cvar->put(cal, ncal)) return false;

  tvar = ncf.add_var("time", ncDouble, tdim);
  if (!tvar->is_valid()) return false;
  tvar->add_att("long_name", "Time");
  tvar->add_att("units", "seconds since 2000-01-01 00:00:00 UTC");
  double atime;
  time_t ttime;
  extern long timezone;
  tmtime.tm_year = year - 1900;
  tmtime.tm_mon = month - 1;
  tmtime.tm_mday = day;
  tmtime.tm_hour = hour;
  tmtime.tm_min = minute;
  tmtime.tm_sec = second;
  ttime = mktime(&tmtime);
  atime = ttime - 946684800 - timezone;
  if (!tvar->put(&atime, 1)) return false;

  ivar = ncf.add_var(db1.get_channel_INFO_name( ), ncShort, tdim, ldim, cdim);
  if (!ivar->is_valid()) return false;
  if (!ivar->add_att("add_offset", 0.0)) return false;
  if (!ivar->add_att("scale_factor", 1.0)) return false;
  if (!ivar->add_att("chnum", db1.chname_to_chnum(argv[2]))) return false;

  // Write output values
  if (!ivar->put((const short int *) pixels, 1, hg, wd)) return false;

  // Close NetCDF output
  (void) ncf.close( );

  db1.close( );
  return 0;
}
