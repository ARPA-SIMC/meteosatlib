//-----------------------------------------------------------------------------
//
//  File        : OpenMTP.h
//  Description : Meteosat OpenMTP format interface
//  Project     : Meteosatlib
//  Author      : Graziano Giuliani
//  References  : Eumetsat EUM-FG-1 Format Guide: OpenMTP format
//                Revision 2.1 April 2000
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
#ifndef __OPENMTP_H__
#define __OPENMTP_H__

#include <iostream>
#include <fstream>
#include <time.h>
#include <stdlib.h>
#include <openmtp/OpenMTP_ascii_header.h>
#include <openmtp/OpenMTP_binary_header.h>
#include <openmtp/OpenMTP_image.h>

class OpenMTP {
  public:
    OpenMTP( );
    ~OpenMTP( );

    void open( char *inname );

    void read( std::ifstream &file );
    
    struct tm &get_datetime( );

    char * get_datatimestr( );
    char * get_satellite_code( );
    char * get_satellite_name( );
    char * get_timestr( );
    bool is_A_format( );
    bool is_B_format( );
    bool is_subwindow( );
    int nlines( );
    int npixels( );
    int bits_per_pixel( );

    bool is_ir_data( );
    bool is_wv_data( );
    bool is_vis_data( );

    float orbit_radius( );
    float subsatellite_point( );

    float *get_calibration( );
    unsigned char *get_image( );
    const char *get_chname( );
    const char *get_chunit( );
    const char *get_field_name( );

    // Overloaded << operator
    friend std::ostream& operator<< ( std::ostream& os, OpenMTP &omtp )
    {
      os << omtp.ascii_header
         << omtp.binary_header
         << omtp.image;
      return os;
    }

  private:
    float nominal_orbit_radius;
    const static int OpenMTP_bpp = 8;
    OpenMTP_ascii_header ascii_header;
    OpenMTP_binary_header binary_header;
    OpenMTP_image image;
};

#endif
