//-----------------------------------------------------------------------------
//
//  Copyright (C) 2004--2006  Graziano Giuliani <giuliani@lamma.rete.toscana.it>
//  Copyright (C) 2006--2012  ARPA-SIM <urpsim@smr.arpa.emr.it>
//
//  File        : OpenMTP_ascii_header.h
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

#include <string>
#include <iostream>
#include <fstream>
#include "OpenMTP_ascii_header.h"

OpenMTP_ascii_header::OpenMTP_ascii_header( ) { }
OpenMTP_ascii_header::OpenMTP_ascii_header( std::ifstream &file )
{
  read(file);
}
OpenMTP_ascii_header::~OpenMTP_ascii_header( ) { }

void OpenMTP_ascii_header::read( std::ifstream &file )
{
  file.read(header, ASCII_HEADER_LENGTH);
  if (file.fail( ))
  {
    std::cerr << "Read error from OpenMTP file: ASCII Header." << std::endl;
    throw;
  }
  header[ASCII_HEADER_LENGTH] = 0;
  return;
}
