//-----------------------------------------------------------------------------
//
//  File        : db1_to_image.cpp
//  Description : Transform ThornSDS MSG file to Image format
//  Author      : Graziano Giuliani
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

#include <Magick++.h>

#undef PACKAGE_BUGREPORT
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION

#include <config.h>

#include <iostream>
#include <thornsds_db1/thornsds_db1.h>

int main (int argc, char *argv[])
{
  MSG_db1_data db1;
  char *format;
  char *image_overlay = 0;
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
    std::cerr << "Usage: " << argv[0] << " directory channel format"
              << " scale [overlay image]" << std::endl;
    std::cerr << "Example: " << argv[0]
              << " dati IR_120 gif 0.1 SEVIRI_OVERLAY.pgm" << std::endl;
    return -1;
  }

  format = strdup(argv[3]);
  gfactor = atof(argv[4]);

  if (argc > 5)
    image_overlay = strdup(argv[5]);
  
  db1.open(argv[1]);
  if (! db1.has_channel(argv[2]))
  {
    std::cerr << "Exit: this channel is not present." << std::endl;
    return -1;
  }

  db1.set_channel(argv[2]);

  if (! db1.is_data_ok( ))
  {
    std::cerr << "Exit: Data Open Error." << std::endl;
    return -1;
  }

  char outname[PATH_MAX];
  snprintf(outname, PATH_MAX, "%s_%s_%s.%s", db1.get_INFO_satellite_name( ),
           db1.get_channel_INFO_name( ),
           db1.get_INFO_schedule_start( ), format);
  for (int i = 0; i < (int) strlen(outname); i ++)
  {
    if (outname[i] == '/') outname[i] = '-';
    if (outname[i] == ' ') outname[i] = '_';
    if (outname[i] == ':') outname[i] = '-';
  }

  Magick::Image *image = new Magick::Image(db1.get_INFO_image_pixels( ),
                                           db1.get_INFO_image_lines( ),
                                           "I", Magick::ShortPixel,
                                           db1.get_RAW_data( ));
  image->normalize( );

  if (! db1.get_INFO_schedule_northsouth( )) image->rotate(180.0);

  if (image_overlay)
  {
    Magick::Image overlay;
    overlay.read("SEVIRI_OVERLAY.pgm");
    image->composite(overlay, 0, 0, Magick::PlusCompositeOp);
  }

  Magick::Geometry geom((int) ((float) db1.get_INFO_image_pixels( )*gfactor),
                        (int) ((float) db1.get_INFO_image_lines( )*gfactor));
  image->scale(geom);

  image->write(outname);

  db1.close( );
  return 0;
}
