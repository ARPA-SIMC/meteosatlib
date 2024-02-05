//-----------------------------------------------------------------------------
//
//  Copyright (C) 2004--2006  Graziano Giuliani <giuliani@lamma.rete.toscana.it>
//  Copyright (C) 2006--2012  ARPAE-SIMC <urpsim@arpae.it>
//
//  File        : HRI.h
//  Description : Meteosat HRI format interface
//  Project     : Meteosatlib
//  Authors     : Graziano Giuliani (Lamma Regione Toscana)
//              : Enrico Zini <enrico@enricozini.com>
//  References  : Meteosat High Resolution Image Dissemination
//                Doc. No. EUM TD 02 Revision 4 April 1998
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

#ifndef __HRI_H__
#define __HRI_H__

#include <ctime>
#include <msat/hri/HRI_subframe_label.h>
#include <msat/hri/HRI_subframe_identification.h>
#include <msat/hri/HRI_subframe_interpretation.h>
#include <msat/hri/HRI_subframe_keyslot.h>
#include <msat/hri/HRI_geolocation.h>
#include <msat/hri/HRI_image.h>

class Hri {
  public:
    Hri( ) { }
    Hri( char *hri_filename, bool IS_TECNAVIA );
    ~Hri( ) { }
    void readfrom( char *hrifile, bool IS_TECNAVIA );

    // Interface
    char * get_format( );
    struct tm *get_datetime( );
    char * get_satellite_name( );
    float get_satellite_longitude( );
    geolocation *get_geolocation( );
    float get_orbit_radius( );
    bool get_northpolar( );
    bool get_northsouth( );
    int get_bits_per_pixel( ) { return 8; }
    char *get_area_name( );

    // Overloaded >> operator
    friend std::ostream& operator<< ( std::ostream& os, Hri &h )
    {
      os << "##################################################" << std::endl
	 << "#           METEOSAT HRI FILE CONTENT            #" << std::endl
	 << "##################################################" << std::endl
	 << std::endl << h.label;
      if (h.seed[0]) os << "Primary Seed: " << std::hex
                      << (unsigned short) h.seed[0] << " "
                      << (unsigned short) h.seed[1] << " "
                      << (unsigned short) h.seed[2] << " "
                      << (unsigned short) h.seed[3] << " "
                      << (unsigned short) h.seed[4] << " "
                      << (unsigned short) h.seed[5] << " "
                      << (unsigned short) h.seed[6] << " "
                      << std::dec << std::endl << std::endl;
      os << h.ident;
      os << h.interp;
      if (h.label.is_A_format( ))
	os << h.keys;
      os << std::endl << "Format is " << h.label.format_code( )
         << std::endl << std::endl;
      return os;
    }
    int nimages;
    HRI_image image[3];
    unsigned char seed[8];
  private:
    void getbuff( ifstream &hri );
    void mod_getbuff( ifstream &hri, int offset );
    unsigned char *get_dataline( ifstream &hri, int format );
    unsigned char *mod_get_dataline( ifstream &hri, int format );
    static const int framesize     = 364;
    static const int mod_framesize = 2048;
    static const int interpsize    = 1360;
    static const int HRI_A_FORMAT  = 112;
    static const int HRI_BX_FORMAT = 48;
    char mod_framebuff[mod_framesize];
    char framebuff[framesize];
    HRI_subframe_label label;
    HRI_subframe_identification ident;
    HRI_subframe_interpretation interp;
    HRI_subframe_keyslot keys;
    HRI_geolocation geo;
};

#endif
