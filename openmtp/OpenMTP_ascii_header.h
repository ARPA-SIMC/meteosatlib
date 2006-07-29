//-----------------------------------------------------------------------------
//
//  File        : OpenMTP_ascii_header.h
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

#ifndef __OPENMTP_ASCII_HEADER_H__
#define __OPENMTP_ASCII_HEADER_H__

#include <string>
#include <iostream>
#include <fstream>

class OpenMTP_ascii_header {
  public:
    OpenMTP_ascii_header( );
    OpenMTP_ascii_header( std::ifstream &file );
    ~OpenMTP_ascii_header( );

    void read( std::ifstream &file );

    // Overloaded << operator
    friend std::ostream& operator<< ( std::ostream& os,
                                      OpenMTP_ascii_header &h )
    {
      os << h.header;
      return os;
    }

  private:
    const static int ASCII_HEADER_LENGTH = 1345;
    const static int BUFLEN = 1346;
    char header[BUFLEN];

};

#endif
