//-----------------------------------------------------------------------------
//
//  Copyright (C) 2004--2006  Graziano Giuliani <giuliani@lamma.rete.toscana.it>
//  Copyright (C) 2006--2012  ARPAE-SIMC <urpsim@arpae.it>
//
//  File        : OpenMTP_image_line.h
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

#ifndef __OPENMTP_IMAGE_LINE_H__
#define __OPENMTP_IMAGE_LINE_H__

#include <iostream>
#include <msat/openmtp/OpenMTP_machine.h>

class OpenMTP_image_line {
  public:
    OpenMTP_image_line( );
    OpenMTP_image_line( std::ifstream &file, int npixels );
    ~OpenMTP_image_line( );

    void read( std::ifstream &file, int npixels);

    int slot_number( );
    int line_number( );
    short errps( );
    short radpos( );
    short radpos_start( );
    unsigned char *linevals( );

    // Overloaded << operator
    friend std::ostream& operator<< ( std::ostream& os, OpenMTP_image_line &l )
    {
      os << "Slot\t\t" << l.slot_number( ) << std::endl
         << "Lnum\t\t" << l.line_number( ) << std::endl
         << "Errps\t\t" << l.errps( ) << std::endl
         << "Radpos\t\t" << l.radpos( ) << std::endl
         << "Rpsta\t\t" << l.radpos_start( ) << std::endl;
      return os;
    }

  private:
    int npix;
    const static int MAX_PIXELS = 5000;
    const static int BUFLEN = 5032;
    unsigned char line[BUFLEN];
    OpenMTP_machine m;
};

#endif
