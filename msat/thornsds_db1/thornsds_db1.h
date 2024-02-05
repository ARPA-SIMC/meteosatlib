//-----------------------------------------------------------------------------
//
//  Copyright (C) 2004--2006  Graziano Giuliani <giuliani@lamma.rete.toscana.it>
//  Copyright (C) 2006--2012  ARPAE-SIMC <urpsim@arpae.it>
//
//  File        : thornsds_db1.h
//  Description : ThornSDS DB1 file for MSG interface - interface
//  Authors     : Graziano Giuliani (Lamma Regione Toscana)
//              : Enrico Zini <enrico@enricozini.com>
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
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <msat/iniparser.h>

#define MAXCH     12
#define CHLEN      6
#define NVAR       4
#define VARLEN     2
#define INFOCHLEN 32

typedef enum {
  RADIANCE_V  = 0,
  RADIANCE_N  = 1,
  TEMPERATURE = 2,
  UNDEFINED   = 3
} t_enum_MSG_Variable;

class MSG_db1_data {

  public:
    MSG_db1_data( );
    ~MSG_db1_data( );
    void open(char *directory);
    void close( );

    bool has_channel(char *chname);
    void set_channel(char *chname);
    int chname_to_chnum(char *chname);

    bool is_data_ok( );

    int   get_AoI_npixels( );
    int   get_AoI_nlines( );
    char *get_AoI_name( );
    char *get_AoI_projection( );
    long  get_AoI_cfac( ); 
    long  get_AoI_lfac( ); 
    long  get_AoI_coff( ); 
    long  get_AoI_loff( ); 

    char *get_INFO_station_name( );
    char *get_INFO_satellite_name( );
    int   get_INFO_satellite_id( );
    int   get_INFO_satellite_orbit( );
    char *get_INFO_schedule_start( );
    char *get_INFO_schedule_end( );
    float get_INFO_schedule_elevation( );
    bool  get_INFO_schedule_northsouth( );
    char *get_INFO_schedule_location( );
    bool  get_INFO_schedule_day( );
    char *get_INFO_schedule_slot( );
    char *get_INFO_image_sensor( );
    int   get_INFO_image_pixels( );
    int   get_INFO_image_lines( );
    int   get_INFO_image_bitsperpixel( );
    bool  get_INFO_image_protected( );
    int   get_INFO_image_nchannels( );
    int   get_INFO_image_nproducts( );

    char *get_channel_INFO_name( );
    char *get_channel_INFO_variable( );
    char *get_channel_INFO_variable_code( );
    char *get_channel_INFO_units( );
    int   get_channel_INFO_bitsperpixel( );
    bool  get_channel_INFO_table( );
    float get_channel_Calibration_Slope( );
    float get_channel_Calibration_TargetCount( );
    float get_channel_Calibration_Vc( );
    float get_channel_Calibration_A( );
    float get_channel_Calibration_B( );
    float get_channel_Calibration_value_calibrated( int count );

    int get_raw_data_size( );
    int get_raw_data_nvals( );
    unsigned short *get_RAW_data( );
    int get_number_of_calibration( );
    float *get_calibration( );

    friend std::ostream& operator<< ( std::ostream& os, MSG_db1_data &d );
  private:
    int get_channel_number(char *chname);
    char infochuse[INFOCHLEN];
    char *dirname;
    struct stat chfstat;
    dictionary *AoI;
    dictionary *INFO;
    dictionary *Calibration;
    int chnum;
    unsigned short *rawdata;
    int ncalval;
    float *cal_values;
};
