//---------------------------------------------------------------------------
//
//  File        :   native2Image.cpp
//  Description :   Export MSG Native format in Image format
//  Project     :   Lamma 2005
//  Author      :   Graziano Giuliani (Lamma Regione Toscana)
//  Source      :   n/a
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
//---------------------------------------------------------------------------
#include <iostream>
#include <fstream>
#include <cstring>
#include <limits.h>
#include <msat/msg-native/MSG_native.h>

#include <Magick++.h>

#undef PACKAGE_BUGREPORT
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION

#include <config.h>

int main(int argc, char **argv)
{
  MSG_native native;
  char *format;
  float gfactor;

  if (argc > 1)
  {
    if (!strcmp(argv[1], "-V"))
    {
      std::cout << argv[0] << " " << PACKAGE_STRING << std::endl;
      return 0;
    }
  }

  if (argc < 5)
  {
    std::cerr << "Usage: " << argv[0] << " file channel format scale"
              << std::endl;
    std::cerr << "Example: " << argv[0]
              << " MSG1-SEVI-MSG15-0100-NA-20040809131237.412000000Z-5766.nat"
              << " 8 gif 0.1"
              << std::endl;
    return -1;
  }

  int ichn = atoi(argv[2]);
  format = strdup(argv[3]);
  gfactor = atof(argv[4]);

  if (! native.open(argv[1]))
  {
    std::cerr << "Exit: Data Open Error." << std::endl;
    return -1;
  }

  native.read( );

  Magick::Image *image = new Magick::Image(native.pixels(ichn),
                         native.lines(ichn), "I", Magick::ShortPixel,
                         native.data(ichn)); 
  char outname[PATH_MAX];
  snprintf(outname, PATH_MAX, "%s.%s", "MSG1", format);

  image->normalize( );
  image->rotate(180.0);
  image->write(outname);

  Magick::Geometry geom((int) ((float) native.pixels(ichn)*gfactor),
                        (int) ((float) native.lines(ichn) *gfactor));
  image->scale(geom);

  native.close( );
  return 0;
}
