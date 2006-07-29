//-----------------------------------------------------------------------------
//
//  File        : thornsds_db1.cpp
//  Description : ThornSDS DB1 file for MSG interface - implementation
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
#include <cstring>
#include <cstdio>
#include <cmath>
#include <thornsds_db1/thornsds_db1.h>

static char vname[NVAR][VARLEN] = { "V", "N", "T", "U" };

MSG_db1_data::MSG_db1_data( )
{
  ncalval     = -1;
  chnum       = 0;
  dirname     = 0;
  AoI         = 0;
  INFO        = 0;
  Calibration = 0;
  rawdata     = 0;
  cal_values  = 0;
}

MSG_db1_data::~MSG_db1_data( ) { this->close( ); }

void MSG_db1_data::close( )
{
  if (dirname) { free(dirname); dirname = 0; }
  if (AoI) { iniparser_free(AoI); AoI = 0; }
  if (INFO) { iniparser_free(INFO); INFO = 0; }
  if (Calibration) { iniparser_free(Calibration); Calibration = 0; }
  if (rawdata) { delete [ ] rawdata; rawdata = 0; }
  if (cal_values) { delete [ ] cal_values; cal_values = 0; }
  chnum = 0;
  ncalval = -1;
}

void MSG_db1_data::open( char *directory )
{
  char iname[PATH_MAX];
  this->dirname = strdup(directory);
  snprintf(iname, PATH_MAX, "%s/%s", directory, "AoI");
  AoI = iniparser_new(iname);
  if (AoI == 0)
    std::cerr << "No such file: " << iname << std::endl;
  snprintf(iname, PATH_MAX, "%s/%s", directory, "INFO.DBI");
  INFO = iniparser_new(iname);
  if (INFO == 0)
    std::cerr << "No such file: " << iname << std::endl;
  return;
}

bool MSG_db1_data::has_channel(char *chname)
{
  if (dirname == 0)
  {
    std::cerr << "Source not opened." << std::endl;
    return false;
  }
  char chfname[PATH_MAX];
  sprintf(chfname, "%s/%s.RAW", dirname, chname);
  if (stat(chfname, &chfstat) == 0)
    return true;
  return false;
}

void MSG_db1_data::set_channel(char *chname)
{
  if (! this->has_channel(chname))
  {
    if (dirname)
      std::cerr << "Channel not present in " << dirname << std::endl;
  }
  char iname[PATH_MAX];
  sprintf(iname, "%s/%s.Calibration", dirname, chname);
  Calibration = iniparser_new(iname);
  if (Calibration == 0)
  {
    std::cerr << "No such file: " << iname << std::endl;
    std::cerr << "No Calibration performed for " << chname << std::endl;
  }
  sprintf(iname, "%s/%s.RAW", dirname, chname);
  FILE *fp = fopen(iname, "r");
  if (! fp)
  {
    std::cerr << "Cannot open file: " << iname << std::endl;
    return;
  }
  rawdata = (unsigned short *) (new unsigned char[chfstat.st_size]);
  if (fread(rawdata, chfstat.st_size, 1, fp) != 1)
  {
    std::cerr << "Cannot read file: " << iname << std::endl;
    return;
  }
  fclose(fp);
  this->chnum = get_channel_number(chname);
  ncalval = (int) (pow(2.0, (double) get_INFO_image_bitsperpixel( )));
  cal_values = new float[ncalval];

  for (int i = 0; i < ncalval; i ++)
    cal_values[i] = get_channel_Calibration_value_calibrated(i);

  return;
}

int MSG_db1_data::get_channel_number(char *chname)
{
  int ich = 0;
  char *iname;
  while (ich < MAXCH)
  {
    ich ++;
    snprintf(infochuse, INFOCHLEN, "Channel%d:Name", ich);
    iname = iniparser_getstring(INFO, infochuse, "Undefined");
    if (! strncmp(chname, iname, CHLEN))
      return ich;
  }
  return 0;
}

int MSG_db1_data::chname_to_chnum(char *chname)
{
  if (! strncmp(chname, "VIS006", CHLEN)) return 1;
  if (! strncmp(chname, "VIS008", CHLEN)) return 2;
  if (! strncmp(chname, "IR_016", CHLEN)) return 3;
  if (! strncmp(chname, "IR_039", CHLEN)) return 4;
  if (! strncmp(chname, "WV_062", CHLEN)) return 5;
  if (! strncmp(chname, "WV_073", CHLEN)) return 6;
  if (! strncmp(chname, "IR_087", CHLEN)) return 7;
  if (! strncmp(chname, "IR_097", CHLEN)) return 8;
  if (! strncmp(chname, "IR_108", CHLEN)) return 9;
  if (! strncmp(chname, "IR_120", CHLEN)) return 10;
  if (! strncmp(chname, "IR_134", CHLEN)) return 11;
  if (! strncmp(chname, "HRV", CHLEN)) return 12;
  return 0;
}

bool MSG_db1_data::is_data_ok( )
{
  if (chnum > 0)
    return true;
  return false;
}

int MSG_db1_data::get_AoI_npixels( )
{
  if (! is_data_ok( )) return -1;
  return iniparser_getint(AoI, ":nPixels", chfstat.st_size/sizeof(short));
}

int MSG_db1_data::get_AoI_nlines( )
{
  if (! is_data_ok( )) return -1;
  return iniparser_getint(AoI, ":nLines", 1);
}

char *MSG_db1_data::get_AoI_name( )
{
  if (! is_data_ok( )) return 0;
  return iniparser_getstring(AoI, ":Name", "Undefined");
}

char *MSG_db1_data::get_AoI_projection( )
{
  if (! is_data_ok( )) return 0;
  return iniparser_getstring(AoI, ":Projection", "GRID");
}

long MSG_db1_data::get_AoI_cfac( )
{
  if (! is_data_ok( )) return 0;
  return (long) iniparser_getint(AoI, ":CFAC", 0);
}

long MSG_db1_data::get_AoI_lfac( )
{
  if (! is_data_ok( )) return 0;
  return (long) iniparser_getint(AoI, ":LFAC", 0);
}

long MSG_db1_data::get_AoI_coff( )
{
  if (! is_data_ok( )) return 0;
  return (long) iniparser_getint(AoI, ":COFF", 0);
}

long MSG_db1_data::get_AoI_loff( )
{
  if (! is_data_ok( )) return 0;
  return (long) iniparser_getint(AoI, ":LOFF", 0);
}

char *MSG_db1_data::get_INFO_station_name( )
{
  if (! is_data_ok( )) return 0;
  return iniparser_getstring(INFO, "Station:Name", "Undefined");
}

char *MSG_db1_data::get_INFO_satellite_name( )
{
  if (! is_data_ok( )) return 0;
  return iniparser_getstring(INFO, "Satellite:Name", "Undefined");
}

int MSG_db1_data::get_INFO_satellite_id( )
{
  if (! is_data_ok( )) return 0;
  return iniparser_getint(INFO, "Satellite:ID", 0);
}

int MSG_db1_data::get_INFO_satellite_orbit( )
{
  if (! is_data_ok( )) return 0;
  return iniparser_getint(INFO, "Satellite:Orbit", 0);
}

char *MSG_db1_data::get_INFO_schedule_start( )
{
  if (! is_data_ok( )) return 0;
  return iniparser_getstring(INFO, "Schedule:Start", "01/01/2000 00:00:00.000");
}

char *MSG_db1_data::get_INFO_schedule_end( )
{
  if (! is_data_ok( )) return 0;
  return iniparser_getstring(INFO, "Schedule:End", "01/01/2000 00:00:00.000");
}

float MSG_db1_data::get_INFO_schedule_elevation( )
{
  if (! is_data_ok( )) return 0;
  return (float) iniparser_getdouble(INFO, "Schedule:Elevation", 90.0);
}

bool MSG_db1_data::get_INFO_schedule_northsouth( )
{
  if (! is_data_ok( )) return 0;
  return (bool) iniparser_getboolean(INFO, "Schedule:NorthSouth", false);
}

char *MSG_db1_data::get_INFO_schedule_location( )
{
  if (! is_data_ok( )) return 0;
  return iniparser_getstring(INFO, "Schedule:Location", "OH");
}

bool MSG_db1_data::get_INFO_schedule_day( )
{
  if (! is_data_ok( )) return 0;
  return (bool) iniparser_getboolean(INFO, "Schedule:Day", true);
}

char *MSG_db1_data::get_INFO_schedule_slot( )
{
  if (! is_data_ok( )) return 0;
  return iniparser_getstring(INFO, "Schedule:Slot", "HRIT");
}

char *MSG_db1_data::get_INFO_image_sensor( )
{
  if (! is_data_ok( )) return 0;
  return iniparser_getstring(INFO, "Image:Sensor", "Seviri");
}

int MSG_db1_data::get_INFO_image_pixels( )
{
  if (! is_data_ok( )) return 0;
  return iniparser_getint(INFO, "Image:Pixels", chfstat.st_size/sizeof(short));
}

int MSG_db1_data::get_INFO_image_lines( )
{
  if (! is_data_ok( )) return 0;
  return iniparser_getint(INFO, "Image:Lines", 1);
}

int MSG_db1_data::get_INFO_image_bitsperpixel( )
{
  if (! is_data_ok( )) return 0;
  return iniparser_getint(INFO, "Image:BitsPerPixel", 10);
}

bool MSG_db1_data::get_INFO_image_protected( )
{
  if (! is_data_ok( )) return 0;
  return iniparser_getboolean(INFO, "Image:Protected", false);
}

int MSG_db1_data::get_INFO_image_nchannels( )
{
  if (! is_data_ok( )) return 0;
  return iniparser_getint(INFO, "Image:nChannels", 10);
}

int MSG_db1_data::get_INFO_image_nproducts( )
{
  if (! is_data_ok( )) return 0;
  return iniparser_getint(INFO, "Image:nProducts", 10);
}

char *MSG_db1_data::get_channel_INFO_name( )
{
  if (! is_data_ok( )) return 0;
  snprintf(infochuse, INFOCHLEN, "Channel%d:Name", chnum);
  return iniparser_getstring(INFO, infochuse, "Undefined");
}

char *MSG_db1_data::get_channel_INFO_variable( )
{
  if (! is_data_ok( )) return 0;
  snprintf(infochuse, INFOCHLEN, "Channel%d:Variable", chnum);
  return iniparser_getstring(INFO, infochuse, "Undefined");
}

char *MSG_db1_data::get_channel_INFO_variable_code( )
{
  if (! is_data_ok( )) return 0;
  char *varname = get_channel_INFO_variable( );
  int ich = chname_to_chnum(get_channel_INFO_name( ));
  if (varname[0] == 'T') return vname[TEMPERATURE];
  else if (varname[0] == 'R') 
  {
    if (ich <= 3) 
      return vname[RADIANCE_V];
    return vname[RADIANCE_N];
  }
  return vname[UNDEFINED];
}

char *MSG_db1_data::get_channel_INFO_units( )
{
  if (! is_data_ok( )) return 0;
  snprintf(infochuse, INFOCHLEN, "Channel%d:Units", chnum);
  return iniparser_getstring(INFO, infochuse, "Undefined");
}

int MSG_db1_data::get_channel_INFO_bitsperpixel( )
{
  if (! is_data_ok( )) return 0;
  snprintf(infochuse, INFOCHLEN, "Channel%d:BitsPerPixel", chnum);
  return iniparser_getint(INFO, infochuse, 10);
}

bool MSG_db1_data::get_channel_INFO_table( )
{
  if (! is_data_ok( )) return 0;
  snprintf(infochuse, INFOCHLEN, "Channel%d:Table", chnum);
  return iniparser_getboolean(INFO, infochuse, false);
}

float MSG_db1_data::get_channel_Calibration_Slope( )
{
  if (! is_data_ok( )) return 0;
  return (float) iniparser_getdouble(Calibration, "Calibration:Slope", 0.0);
}

float MSG_db1_data::get_channel_Calibration_TargetCount( )
{
  if (! is_data_ok( )) return 0;
  return (float) iniparser_getdouble(Calibration,
                              "Calibration:TargetCount", 0.0);
}

float MSG_db1_data::get_channel_Calibration_Vc( )
{
  if (! is_data_ok( )) return 0;
  return (float) iniparser_getdouble(Calibration, "Calibration:Vc", 0.0);
}

float MSG_db1_data::get_channel_Calibration_A( )
{
  if (! is_data_ok( )) return 0;
  return (float) iniparser_getdouble(Calibration, "Calibration:A", 0.0);
}

float MSG_db1_data::get_channel_Calibration_B( )
{
  if (! is_data_ok( )) return 0;
  return (float) iniparser_getdouble(Calibration, "Calibration:B", 0.0);
}

float MSG_db1_data::get_channel_Calibration_value_calibrated( int count )
{
  if (! is_data_ok( )) return 0;
  if (count < 0 && count >= ncalval) return 0.0;
  snprintf(infochuse, INFOCHLEN, "%s:%s(%d)", get_channel_INFO_variable( ),
           get_channel_INFO_variable_code( ), count);
  return (float) iniparser_getdouble(Calibration, infochuse, 0.0);
}

unsigned short *MSG_db1_data::get_RAW_data( )
{
  if (! is_data_ok( )) return 0;
  return rawdata;
}

float *MSG_db1_data::get_calibration( )
{
  if (! is_data_ok( )) return 0;
  return cal_values;
}

int MSG_db1_data::get_number_of_calibration( )
{
  if (! is_data_ok( )) return 0;
  return ncalval;
}

int MSG_db1_data::get_raw_data_size( )
{
  if (! is_data_ok( )) return 0;
  return chfstat.st_size;
}

int MSG_db1_data::get_raw_data_nvals( )
{
  if (! is_data_ok( )) return 0;
  return chfstat.st_size/sizeof(short);
}

std::ostream& operator<< ( std::ostream& os, MSG_db1_data &d )
{
  os << "------------------------------------------------------" << std::endl
     << "-           MSG DATA SATELLITE DB1 AoI               -" << std::endl
     << "------------------------------------------------------" << std::endl;
  os << "AoI Name         : " << d.get_AoI_name( ) << std::endl
     << "AoI nPixels      : " << d.get_AoI_npixels( ) << std::endl
     << "AoI nLines       : " << d.get_AoI_nlines( ) << std::endl
     << "AoI Projection   : " << d.get_AoI_projection( ) << std::endl
     << "AoI CFAC         : " << d.get_AoI_cfac( ) << std::endl
     << "AoI LFAC         : " << d.get_AoI_lfac( ) << std::endl
     << "AoI COFF         : " << d.get_AoI_coff( ) << std::endl
     << "AoI LOFF         : " << d.get_AoI_loff( ) << std::endl;
  os << "------------------------------------------------------" << std::endl
     << "-           MSG DATA SATELLITE DB1 INFO              -" << std::endl
     << "------------------------------------------------------" << std::endl;
  os << "Station Name     : " << d.get_INFO_station_name( ) << std::endl
     << "Satellite Name   : " << d.get_INFO_satellite_name( ) << std::endl
     << "Satellite ID     : " << d.get_INFO_satellite_id( ) << std::endl
     << "Satellite Orbit  : " << d.get_INFO_satellite_orbit( ) << std::endl
     << "Schedule Start   : " << d.get_INFO_schedule_start( ) << std::endl
     << "Schedule End     : " << d.get_INFO_schedule_end( ) << std::endl
     << "Elevation        : " << d.get_INFO_schedule_elevation( ) << std::endl
     << "NorthSouth       : " << d.get_INFO_schedule_northsouth( ) << std::endl
     << "Location         : " << d.get_INFO_schedule_location( ) << std::endl
     << "Day              : " << d.get_INFO_schedule_day( ) << std::endl
     << "Slot             : " << d.get_INFO_schedule_slot( ) << std::endl
     << "Image Sensor     : " << d.get_INFO_image_sensor( ) << std::endl
     << "Image Pixels     : " << d.get_INFO_image_pixels( ) << std::endl
     << "Image Lines      : " << d.get_INFO_image_lines( ) << std::endl
     << "Bits per Pixel   : " << d.get_INFO_image_bitsperpixel( ) << std::endl
     << "Image Protected  : " << d.get_INFO_image_protected( ) << std::endl
     << "Image Channels   : " << d.get_INFO_image_nchannels( ) << std::endl
     << "Image Products   : " << d.get_INFO_image_nproducts( ) << std::endl;
  os << "------------------------------------------------------" << std::endl
     << "-           MSG DATA CHANNEL DB1 INFO                -" << std::endl
     << "------------------------------------------------------" << std::endl;
  os << "Channel Name     : " << d.get_channel_INFO_name( ) << std::endl
     << "Channel Variable : " << d.get_channel_INFO_variable( ) << std::endl
     << "Channel Units    : " << d.get_channel_INFO_units( ) << std::endl
     << "Channel BPP      : " << d.get_channel_INFO_bitsperpixel( ) << std::endl
     << "Channel Table    : " << d.get_channel_INFO_table( ) << std::endl;
  os << "------------------------------------------------------" << std::endl
     << "-           MSG DATA CHANNEL CALIBRATION             -" << std::endl
     << "------------------------------------------------------" << std::endl;
  os << "Slope            : " << d.get_channel_Calibration_Slope( ) << std::endl
     << "Target Count     : " << d.get_channel_Calibration_TargetCount( )
     << std::endl
     << "Vc               : " << d.get_channel_Calibration_Vc( ) << std::endl
     << "A                : " << d.get_channel_Calibration_A( ) << std::endl
     << "B                : " << d.get_channel_Calibration_B( ) << std::endl;

  return os;
}
