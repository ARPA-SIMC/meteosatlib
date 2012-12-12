//-----------------------------------------------------------------------------
//
//  Copyright (C) 2004--2006  Graziano Giuliani <giuliani@lamma.rete.toscana.it>
//  Copyright (C) 2006--2012  ARPA-SIM <urpsim@smr.arpa.emr.it>
//
//  File        : OpenMTP_machine.h
//  Description : Meteosat OpenMTP format interface
//  Project     : Meteosatlib
//  Authors     : Graziano Giuliani (Lamma Regione Toscana)
//              : Enrico Zini <enrico@enricozini.com>
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

#ifndef __OPENMTP_MACHINE_H__
#define __OPENMTP_MACHINE_H__

class OpenMTP_machine {

  public:

    OpenMTP_machine( );
    ~OpenMTP_machine( );

    short int2( unsigned char *buff );
    int int4( unsigned char *buff );
    double float8( unsigned char *buff );
    float float4( unsigned char *buff );
    float r4_from_char5( unsigned char *buff );
    float r4_from_char3( unsigned char *buff );
    int int4_from_char3( unsigned char *buff );
    int int4_from_char2( unsigned char *buff );

  private:

    bool isbig;

};

#endif
