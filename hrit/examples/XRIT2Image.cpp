//---------------------------------------------------------------------------
//
//  File        :   XRIT2Image.cpp
//  Description :   Export MSG HRIT format in Image format
//  Project     :   Lamma 2004
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

#include <Magick++.h>

#undef PACKAGE_BUGREPORT
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION

#include <config.h>

#include <iostream>
#include <fstream>
#include <cstring>

#include <hrit/MSG_HRIT.h>

#include <getopt.h>
#include <glob.h>

#define PATH_SEPARATOR "/"
// For windows use #define PATH_SEPARATOR "\"

void usage(char *pname);
std::string underscoreit(std::string base, int final_len);

int main(int argc, char *argv[ ])
{
  char *directory = NULL;
  char *overlay_image = NULL;
  char *resolution = NULL;
  char *productid1 = NULL;
  char *productid2 = NULL;
  char *timing = NULL;
  char *format = NULL;

  char *pname = strdup(argv[0]);

  float geometry = 1.0;
  bool normalize = false;
  bool do_overlay = false;

  while (1) {
    int option_index = 0;
    int c;

    static struct option long_options[] = {
      {"directory", 1, 0, 'd'},
      {"resolution", 1, 0, 'r'},
      {"productid1", 1, 0, 's'},
      {"productid2", 1, 0, 'c'},
      {"outformat", 1, 0, 'f'},
      {"outscale", 1, 0, 'g'},
      {"overlay", 1, 0, 'o'},
      {"time", 1, 0, 't'},
      {"normalize", 0, 0, 'n'},
      {"version", 0, 0, 'V'},
      {"help", 0, 0, 'h'},
      {0, 0, 0, 0}
    };

    c = getopt_long (argc, argv, "d:r:s:c:f:g:o:t:nVh?",
                     long_options, &option_index);
    if (c == -1)
      break;

    switch (c)
    {
      case 'd':
       directory = strdup(optarg);
       break;
      case 'r':
       resolution = strdup(optarg);
       break;
      case 's':
       productid1 = strdup(optarg);
       break;
      case 'c':
       productid2 = strdup(optarg);
       break;
      case 'f':
       format = strdup(optarg);
       break;
      case 'g':
       geometry = strtod(optarg, (char **)NULL);
       break;
      case 'o':
       do_overlay = true;
       overlay_image = strdup(optarg);
       break;
      case 't':
       timing = strdup(optarg);
       break;
      case 'n':
       normalize = true;
       break;
      case 'V':
       std::cout << pname << " " << PACKAGE_STRING << std::endl;
       return 0;
      case '?':
      case 'h':
       usage(pname);
       return(0);
       break;
      default:
       std::cerr << "?? getopt returned character code"
                 << std::oct << c << "??" << std::endl;
       usage(pname);
       return(1);
    }
  }

  if (resolution == NULL || productid1 == NULL ||
      productid2 == NULL || timing == NULL)
  {
    usage(pname);
    return(1);
  }

  std::string filename;
  if (directory) filename = directory;
  else filename = ".";

  filename = filename + PATH_SEPARATOR + resolution;
  filename = filename + "-???-??????-";
  filename = filename + underscoreit(productid1, 12) + "-";
  filename = filename + underscoreit(productid2, 9) + "-";
  filename = filename + "0?????___" + "-";
  filename = filename + timing + "-" + "?_";

  glob_t globbuf;
  globbuf.gl_offs = 1;

  if ((glob(filename.c_str( ), GLOB_DOOFFS, NULL, &globbuf)) != 0)
  {
    std::cerr << "No such file(s)." << std::endl;
    return 1;
  }

  int nsegments = globbuf.gl_pathc;

  MSG_header *header;
  MSG_data *msgdat;

  header = new MSG_header[nsegments];
  msgdat = new MSG_data[nsegments];

  for (int i = 0; i < nsegments; i ++)
  {
    std::ifstream hrit(globbuf.gl_pathv[i+1],
                       (std::ios::binary | std::ios::in));
    if (hrit.fail())
    {
      std::cerr << "Cannot open input hrit file "
                << globbuf.gl_pathv[i+1] << std::endl;
      return 1;
    }
    header[i].read_from(hrit);
    msgdat[i].read_from(hrit, header[i]);
    hrit.close( );
    std::cout << header[i];
  }

  globfree(&globbuf);

  if (header[0].segment_id->data_field_format == MSG_NO_FORMAT)
  {
    std::cout << "Product dumped in binary format." << std::endl;
    return 0;
  }

  int totalsegs = header[0].segment_id->planned_end_segment_sequence_number;
  int *segsindexes = new int[totalsegs]; 

  for (int i = 0; i < totalsegs; i ++)
    segsindexes[i] = -1;

  for (int i = 0; i < nsegments; i ++)
    segsindexes[header[i].segment_id->sequence_number-1] = i;

  filename = resolution;
  filename = filename + "-" + productid1 + "-" + productid2 + "-" +
             timing;
  if (format)
    filename = filename + "." + format;
  else
    filename = filename + ".jpg";

  int npix = header[0].image_structure->number_of_columns;
  int nlin = header[0].image_structure->number_of_lines;
  size_t npixperseg = npix*nlin;

  size_t total_size = totalsegs*npixperseg;
  MSG_SAMPLE *pixels = new MSG_SAMPLE[total_size];
  memset(pixels, 0, total_size*sizeof(MSG_SAMPLE));
  size_t pos = 0;
  for (int i = 0; i < totalsegs; i ++)
  {
    if (segsindexes[i] >= 0)
      memcpy(pixels+pos, msgdat[segsindexes[i]].image->data,
             npixperseg*sizeof(MSG_SAMPLE));
    pos += npixperseg;
  }

  Magick::Image *image = new Magick::Image(npix, nlin*totalsegs,
                                     "I", Magick::ShortPixel, pixels);
  if (normalize) image->normalize( );
  image->rotate(180.0);
  if (header[0].segment_id->spectral_channel_id < 12)
  {
    if (do_overlay)
    {
      Magick::Image overlay;
      overlay.read(overlay_image);
      image->composite(overlay, 0, 0, Magick::PlusCompositeOp);
    }
  }

  if (geometry < 1.0)
  {
    Magick::Geometry geom((int) ((float) npix*geometry),
                  (int) ((float) nlin*totalsegs*geometry));
    image->scale(geom);
  }

  image->write(filename);

  delete image;

  delete [ ] pixels;
  delete [ ] header;
  delete [ ] msgdat;
  delete [ ] segsindexes;
  return 0;
}

void usage(char *pname)
{
  std::cout << pname << ": Convert HRIT/LRIT files to pgm format." << std::endl;
  std::cout << std::endl << "Usage:" << std::endl << "\t"
            << pname << " [-d directory] [-f format] [-n] [-g scale]"
            << " [-o overlay_image] -r resol"
            << " -s prodid1 -c prodid2 -t time"
            << std::endl << std::endl
            << "Example: " << std::endl << "\t" << pname
            << " -d data/HRIT -f jpg -g 0.2 -r H -s MSG1 -c HRV -t 200402101315"
            << std::endl;
  return;
}

std::string underscoreit(std::string base, int final_len)
{
  char *tmp;
  int mlen;

  tmp = (char *) malloc((final_len+1) * sizeof(char));
  tmp[final_len] = 0;
  memset(tmp, 0x5F, final_len);
  mlen = strlen(base.c_str( ));
  if (mlen > final_len) mlen = final_len;
  memcpy(tmp, base.c_str( ), mlen);
  std::string retval = tmp;
  free(tmp);
  return retval;
}
