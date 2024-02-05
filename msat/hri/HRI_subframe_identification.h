//-----------------------------------------------------------------------------
//
//  Copyright (C) 2004--2006  Graziano Giuliani <giuliani@lamma.rete.toscana.it>
//  Copyright (C) 2006--2012  ARPAE-SIMC <urpsim@arpae.it>
//
//  File        : HRI_subframe_identification.h
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
#ifndef __HRI_SUBFRAME_IDENTIFICATION_H__
#define __HRI_SUBFRAME_IDENTIFICATION_H__

#include <msat/hri/HRI_machine.h>
#include <iostream>
#include <string>

class HRI_subframe_identification {
  public:
    HRI_subframe_identification( ) { }
    HRI_subframe_identification( char *hsi );
    ~HRI_subframe_identification( ) { }
    void readfrom( char hsi[32] );
    bool is_Meteosat( );
    bool is_GOES_E( );
    bool is_GOES_W( );
    bool is_GMS( );
    bool is_GOMS( );
    int meteosat_id;
    std::string satellite;
    std::string datetime;
    int year;
    int month;
    int day;
    int hour;
    int minute;
    struct tm tmtime;
    double longitude;
    unsigned char satellite_indicator[2];
    unsigned short julian_day;
    unsigned short nominal_time;
    unsigned short column_offset;
    unsigned short line_offset;

    // Overloaded >> operator
    friend std::ostream& operator<< ( std::ostream& os,
	                              HRI_subframe_identification& i )
    {
      os << "---------------------------------" << std::endl
	 << "-  HRI Subframe Identification  -" << std::endl
         << "---------------------------------" << std::endl
	 << "Satellite Name               : "
	 << i.satellite << std::endl
	 << "Reference Time               : "
	 << i.datetime
	 << "Julian day                   : "
	 << i.julian_day << std::endl
	 << "Column offset                : "
	 << i.column_offset << std::endl
	 << "Line offset                  : "
	 << i.line_offset << std::endl
	 << "Longitude                    : "
	 << i.longitude << std::endl;

      return os;
    }
  private:
    HRI_machine conv;
};

#endif
