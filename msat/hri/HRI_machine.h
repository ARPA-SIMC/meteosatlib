//-----------------------------------------------------------------------------
//
//  Copyright (C) 2004--2006  Graziano Giuliani <giuliani@lamma.rete.toscana.it>
//  Copyright (C) 2006--2012  ARPA-SIM <urpsim@smr.arpa.emr.it>
//
//  File        : HRI_machine.h
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
#ifndef __HRI_MACHINE_H__
#define __HRI_MACHINE_H__

class HRI_machine {
  public:
    HRI_machine();
    ~HRI_machine();
    double r8_from_buff( const unsigned char *buff );
    float r4_from_buff( const unsigned char *buff );
    short int i2_from_buff( const unsigned char *buff );
    int i4_from_buff( const unsigned char *buff );
    float r4_from_char6( const char *buff );
    float r4_from_char3( const char *buff );
    float r4f_from_char3( const char *buff );
    float r4_from_char5( const char *buff );
    short int i2_from_char3( const char *buff );
    short int i2_from_char2( const char *buff );
    float r4f_from_char5( const char *buff );
  private:
    bool isbig;
};

#endif
