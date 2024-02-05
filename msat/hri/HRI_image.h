//-----------------------------------------------------------------------------
//
//  Copyright (C) 2004--2006  Graziano Giuliani <giuliani@lamma.rete.toscana.it>
//  Copyright (C) 2006--2012  ARPAE-SIMC <urpsim@arpae.it>
//
//  File        : HRI_image.h
//  Description : Meteosat HRI format interface
//  Project     : Meteosatlib
//  Authors     : Graziano Giuliani (Lamma Regione Toscana)
//              : Enrico Zini <enrico@enricozini.com>
//  References  : Meteosat High Resolution Image Dissemination
//                Doc. No. EUM TD 02 Revision 4 April 1998
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
#ifndef __HRI_IMAGE_H__
#define __HRI_IMAGE_H__

#include <string>

using namespace std;

typedef struct {
  float mpef_absolute;
  float space_count;
} calibration_coefficients;

typedef enum {
  METEOSAT = 0,
  GOES_E,
  GOES_W,
  GMS,
  INDOX
} HRI_image_satellite;

typedef enum {
  A_FORMAT = 0,
  B_FORMAT,
  X_FORMAT
} HRI_image_format;

typedef enum {
  IR_BAND = 0,
  WV_BAND,
  VIS_BAND,
  VH_BAND
} HRI_image_band;

class HRI_image {
  public:
    HRI_image( );
    HRI_image( HRI_image_satellite s, HRI_image_format f, HRI_image_band b );
    ~HRI_image( );
    void set_format_band( HRI_image_satellite s,
	                  HRI_image_format f,
			  HRI_image_band b );
    void set_calibration( calibration_coefficients *c );
    void put_line( unsigned char *dataline, int linesize, int linenum );
    void put_halfline( unsigned char *dataline, int linesize,
                       int linenum, bool firsthalf );
    unsigned char *get_line( int linenum );
    unsigned char *get_image( );
    unsigned char get_pixel( int linenum, int pixelnum );
    float *get_calibration( );
    std::string name;
    std::string long_name;
    std::string units;
    int npixels;
    int nlines;
    float samplex;
    float sampley;
    int get_image_band();
  private:
    bool calibrated;
    unsigned char *data;
    unsigned char *aline;
    calibration_coefficients calc;
    HRI_image_satellite sat;
    HRI_image_band band;
    float calibration[256];
    int lastpix;
    int lastlin;
    int halflin;
    int size;
};

#endif
