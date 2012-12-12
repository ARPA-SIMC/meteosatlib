//-----------------------------------------------------------------------------
//
//  Copyright (C) 2004--2006  Graziano Giuliani <giuliani@lamma.rete.toscana.it>
//  Copyright (C) 2006--2012  ARPA-SIM <urpsim@smr.arpa.emr.it>
//
//  File        : OpenMTP_image.h
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

#ifndef __OPENMTP_IMAGE_H__
#define __OPENMTP_IMAGE_H__

#include <string>
#include <iostream>
#include <fstream>
#include <msat/openmtp/OpenMTP_binary_header.h>
#include <msat/openmtp/OpenMTP_image_line.h>

class OpenMTP_image {
  public:
    OpenMTP_image( );
    OpenMTP_image( std::ifstream &file, OpenMTP_binary_header &h );
    ~OpenMTP_image( );

    void read( std::ifstream &file, OpenMTP_binary_header &h );

    unsigned char *data( );

    float *cal( );

    // Overloaded << operator
    friend std::ostream& operator<< ( std::ostream& os, OpenMTP_image &im )
    {
      for (int j = 0; j < im.nlines; j ++)
        for (int i = 0; i < im.npixels; i ++)
          os << "Pixel (" << j << "," << i << ")\t"
             << *(im.calibration+(*(im.image+j*im.npixels+i)))
             << std::endl;
      return os;
    }

    unsigned char *image;
    float calibration[256];
  private:
    OpenMTP_image_line line;
    int npixels;
    int nlines;
};

#endif
