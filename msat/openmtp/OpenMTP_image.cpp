//-----------------------------------------------------------------------------
//
//  Copyright (C) 2004--2006  Graziano Giuliani <giuliani@lamma.rete.toscana.it>
//  Copyright (C) 2006--2012  ARPAE-SIMC <urpsim@arpae.it>
//
//  File        : OpenMTP_image.cpp
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

#include <cstring>
#include <cmath>
#include <iostream>
#include <fstream>
#include "OpenMTP_image.h"

OpenMTP_image::OpenMTP_image( ) { image = 0; }
OpenMTP_image::OpenMTP_image( std::ifstream &file, OpenMTP_binary_header &h )
{
  image = 0;
  read(file, h);
}
OpenMTP_image::~OpenMTP_image( ) { if (image) delete image; }

void OpenMTP_image::read( std::ifstream &file, OpenMTP_binary_header &h )
{
  float rad = 0.0;
  float cc, sc;
  
  nlines = h.nlines( );
  npixels = h.npixels( );

  if (!image)
    image = new unsigned char[nlines*npixels];

  for (int i = 0; i < nlines; i ++)
  {
    line.read(file, npixels);
    memcpy(image+i*npixels, line.linevals( ), npixels);
  }

  for (int i = 0; i < 256; i ++)
    calibration[i] = 1.0;

  if (strcmp(h.satellite_name( ), "M7"))
  {
    cerr << "Warning: OpenMTP calibration only for Meteosat 7." << endl;
    cerr << "Cannot calibrate data. Set calibration to 1.0" << endl;
    return;
  }

  cc = h.mpef_calibration_coefficient( );
  sc = h.mpef_calibration_space_count( );

  cout << "Calibration Coefficient : " << cc << endl;
  cout << "Space Count             : " << sc << endl;

  if (h.is_ir_data( ))
  {
    for (int i = 0; i < 256; i ++)
    {
      if (i < sc) rad = 0.0;
      else rad = cc * ((float) i - sc);
      calibration[i] = -1255.5465/(log(rad) - 6.9618);
    }
  }
  else if (h.is_wv_data( ))
  {
    for (int i = 0; i < 256; i ++)
    {
      if (i < sc) rad = 0.0;
      else rad = cc * ((float) i - sc);
      calibration[i] = -2233.4882/(log(rad) - 9.2477);
    }
  }
  else if (h.is_vis_data( ))
  {
    for (int i = 0; i < 256; i ++)
      calibration[i] = 100.0 * ((float) i / 255.0);
  }
  else
    cerr << "Cannot calibrate data. Set calibration to 1.0" << endl;

  return;
}

unsigned char *OpenMTP_image::data( ) { return image; }

float *OpenMTP_image::cal( ) { return calibration; }
