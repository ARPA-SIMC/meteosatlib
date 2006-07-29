//-----------------------------------------------------------------------------
//
//  File        : OpenMTP_image_line.cpp
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

#include <string>
#include <iostream>
#include <fstream>
#include <openmtp/OpenMTP_image_line.h>

OpenMTP_image_line::OpenMTP_image_line( ) { }
OpenMTP_image_line::OpenMTP_image_line( std::ifstream &file, int npixels )
{
  read(file, npixels);
}
OpenMTP_image_line::~OpenMTP_image_line( ) { }

void OpenMTP_image_line::read( std::ifstream &file, int npixels)
{
  npix = npixels;
  file.read((char *) line, npixels+32);
  if (file.fail( ))
  {
    std::cerr << "Read error from OpenMTP file: Image Line." << std::endl;
    throw;
  }
  return;
}

int OpenMTP_image_line::slot_number( ) { return m.int4(line); }
int OpenMTP_image_line::line_number( ) { return m.int4(line+4); }
short OpenMTP_image_line::errps( ) { return m.int2(line+8); }
short OpenMTP_image_line::radpos( ) { return m.int2(line+10); }
short OpenMTP_image_line::radpos_start( ) { return m.int2(line+30); }
unsigned char *OpenMTP_image_line::linevals( ) { return line+32; }
