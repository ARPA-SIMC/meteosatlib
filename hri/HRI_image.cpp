//-----------------------------------------------------------------------------
//
//  File        : HRI_image.cpp
//  Description : Meteosat HRI format interface
//  Project     : Meteosatlib
//  Author      : Graziano Giuliani (Lamma Regione Toscana)
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
#include <hri/HRI_image.h>
#include <iostream>
#include <cassert>
#include <cstring>
#include <cmath>

HRI_image::HRI_image( )
{
  data = 0;
  aline = 0;
  npixels = 0;
  nlines = 0;
  lastpix = 0;
  lastlin = 0;
  halflin = 0;
  size = 0;
  calibrated = false;
}

HRI_image::HRI_image( HRI_image_satellite s,
                      HRI_image_format f,
		      HRI_image_band b )
{
  set_format_band(s, f, b);
}

HRI_image::~HRI_image( )
{
  if (data)  delete [ ] data;
  if (aline) delete [ ] aline;
}

void HRI_image::set_format_band( HRI_image_satellite s,
                                 HRI_image_format f,
				 HRI_image_band b )
{
  sat = s;
  band = b;
  switch (f)
  {
    case A_FORMAT:
      switch (b)
      {
	case IR_BAND:
	  name = "IR";
	  units = "K";
	  long_name = "AFormat_IRBand";
	  npixels = 2500;
	  nlines  = 2500;
	  lastpix = 2499;
	  lastlin = 2499;
	  halflin = 1250;
          samplex = 1.0;
          sampley = 1.0;
	  size    = 6250000;
	  break;
	case WV_BAND:
	  name = "WV";
	  units = "K";
	  long_name = "AFormat_WVBand";
	  npixels = 2500;
	  nlines  = 2500;
	  lastpix = 2499;
	  lastlin = 2499;
	  halflin = 1250;
          samplex = 1.0;
          sampley = 1.0;
	  size    = 6250000;
	  break;
	case VH_BAND:
	  name = "VIS";
	  units = "%";
	  long_name = "AFormat_VHBand";
	  npixels = 2500;
	  nlines  = 2500;
	  lastpix = 2499;
	  lastlin = 2499;
	  halflin = 1250;
          samplex = 1.0;
          sampley = 1.0;
	  size    = 6250000;
	  break;
	case VIS_BAND:
	  name = "VIS";
	  units = "%";
	  long_name = "AFormat_VISBand";
          npixels = 5000;
	  nlines  = 5000;
	  lastpix = 4999;
	  lastlin = 4999;
	  halflin = 2500;
          samplex = 1.0;
          sampley = 1.0;
	  size    = 25000000;
	  break;
        default:
	  std::cerr << "Undefined band for A Format image in HRI_Image"
                    << std::endl;
	  throw;
	  break;
      }
      break;
    case B_FORMAT:
      switch (b)
      {
	case IR_BAND:
	  name = "IR";
	  units = "K";
	  long_name = "BFormat_IRBand";
	  npixels = 1250;
	  nlines  = 625;
	  lastpix = 1249;
	  lastlin = 624;
	  halflin = 625;
          samplex = 2.0;
          sampley = 2.0;
	  size    = 781250;
	  break;
	case WV_BAND:
	  name = "WV";
	  units = "K";
	  long_name = "BFormat_WVBand";
	  npixels = 1250;
	  nlines  = 625;
	  lastpix = 1249;
	  lastlin = 624;
	  halflin = 625;
          samplex = 2.0;
          sampley = 2.0;
	  size    = 781250;
	  break;
	case VIS_BAND:
	  name = "VIS";
	  units = "%";
	  long_name = "BFormat_VISBand";
	  npixels = 2500;
	  nlines  = 1250;
	  lastpix = 2499;
	  lastlin = 1249;
	  halflin = 1250;
          samplex = 1.0;
          sampley = 1.0;
	  size    = 3125000;
	  break;
	default:
	  std::cerr << "Undefined band for B Format image in HRI_Image"
                    << std::endl;
	  throw;
	  break;
      }
      break;
    case X_FORMAT:
      switch (b)
      {
	case IR_BAND:
	  name = "IR";
	  units = "K";
	  long_name = "XFormat_IRBand";
	  npixels = 1250;
	  nlines  = 1250;
	  lastpix = 1249;
	  lastlin = 1249;
	  halflin = 625;
          samplex = 2.0;
          sampley = 2.0;
	  size    = 1562500;
	  break;
	case WV_BAND:
	  name = "WV";
	  units = "K";
	  long_name = "XFormat_WVBand";
	  npixels = 1250;
	  nlines  = 1250;
	  lastpix = 1249;
	  lastlin = 1249;
	  halflin = 625;
          samplex = 2.0;
          sampley = 2.0;
	  size    = 1562500;
	  break;
	case VH_BAND:
	  name = "VH";
	  units = "%";
	  long_name = "BFormat_VHBand";
	  npixels = 1250;
	  nlines  = 1250;
	  lastpix = 1249;
	  lastlin = 1249;
	  halflin = 625;
          samplex = 2.0;
          sampley = 2.0;
	  size    = 1562500;
	  break;
	default:
	  std::cerr << "Undefined band for X Format image in HRI_Image"
                    << std::endl;
	  throw;
	  break;
      }
      break;
    default:
      std::cerr << "Undefined format in HRI_image" << std::endl;
      throw;
  }
  data  = new unsigned char[size];
  aline = new unsigned char[npixels];
  assert(data);
  assert(aline);
  return;
}

void HRI_image::put_line( unsigned char *dataline, int linesize, int linenum )
{
  if (size == 0) return;
  if (linesize != npixels)
  {
    std::cerr << "Invalid line size : " << linesize << std::endl;
    std::cerr << "Maximum line size for this image is " << npixels << std::endl;
    throw;
  }
  if (linenum < 0 || linenum > lastlin)
  {
    std::cerr << "Out of range line number : " << linenum << std::endl;
    std::cerr << "Maximum line number for this image is "
              << lastlin << std::endl;
    throw;
  }
  memcpy(data+linenum*npixels, dataline, linesize);
  return;
}

void HRI_image::put_halfline( unsigned char *dataline, int linesize,
                              int linenum, bool firsthalf )
{
  if (size == 0) return;
  if (linesize != halflin)
  {
    std::cerr << "Invalid half line size : " << linesize << std::endl;
    std::cerr << "Maximum half line size for this image is "
              << halflin << std::endl;
    throw;
  }
  if (linenum < 0 || linenum > lastlin)
  {
    std::cerr << "Out of range line number : " << linenum << std::endl;
    std::cerr << "Maximum line number for this image is "
              << lastlin << std::endl;
    throw;
  }
  if (firsthalf)
    memcpy(data+linenum*npixels, dataline, linesize);
  else
    memcpy(data+linenum*npixels+halflin-2, dataline, linesize);
  return;
}

unsigned char * HRI_image::get_line( int linenum )
{
  if (size == 0) return 0;
  if (linenum < 0 || linenum > lastlin)
  {
    std::cerr << "Out of range line number : " << linenum << std::endl;
    std::cerr << "Maximum line number for this image is "
              << lastlin << std::endl;
    throw;
  }
  memcpy(aline, data+linenum*npixels, npixels);
  return aline;
}

unsigned char * HRI_image::get_image( )
{
  return data;
}

int HRI_image::get_image_band( )
{
  return (int) band;
}

unsigned char HRI_image::get_pixel( int linenum, int pixelnum )
{
  if (size == 0) return 0;
  if (linenum  < 0 || linenum  > lastlin)
  {
    std::cerr << "Out of range line number : " << linenum << std::endl;
    std::cerr << "Maximum line number for this image is "
              << lastlin << std::endl;
    throw;
  }
  if (pixelnum < 0 || pixelnum > lastpix)
  {
    std::cerr << "Out of range pixel number : " << pixelnum  << std::endl;
    std::cerr << "Maximum pixel number for this image is "
              << lastpix << std::endl;
    throw;
  }
  return *(data+linenum*npixels+pixelnum);
}

void HRI_image::set_calibration( calibration_coefficients *c )
{
  calc.mpef_absolute = c->mpef_absolute;
  calc.space_count   = c->space_count;
  calibrated = true;
}

float * HRI_image::get_calibration( )
{
  float rad   = 0.0;
  float cc, sc;

  cc = calc.mpef_absolute;
  sc = calc.space_count;

  for (int i = 0; i < 256; i ++)
    calibration[i] = 1.0;

  if (sat == METEOSAT)
  {
    switch (band)
    {
      case IR_BAND:
	if (calibrated)
	{
	  for (int i = 0; i < 256; i ++)
	  {
            if (i < sc) rad = 0.0;
	    else rad = cc * ((float) i - sc);
	    calibration[i] = -1255.5465/(log(rad) - 6.9618);
          }
        }
	else
          std::cerr << "Cannot calibrate data. Set calibration to 1.0"
                    << std::endl;
	break;
      case WV_BAND:
	if (calibrated)
	{
	  for (int i = 0; i < 256; i ++)
	  {
            if (i < sc) rad = 0.0;
	    else rad = cc * ((float) i - sc);
	    calibration[i] = -2233.4882/(log(rad) - 9.2477);
	  }
	}
	else
          std::cerr << "Cannot calibrate data. Set calibration to 1.0"
                    << std::endl;
	break;
      case VIS_BAND:
      case VH_BAND:
	for (int i = 0; i < 256; i ++)
	  calibration[i] = 100.0 * ((float) i / 255.0);
	break;
      default:
        std::cerr << "Cannot calibrate data. Set calibration to 1.0"
                  << std::endl;
	break;
    }
    return calibration;
  }

  const static float gms_ir_cal[256] = {
    130.00, 130.00, 130.00, 130.00, 130.00, 130.00, 130.00, 130.00,
    130.00, 130.00, 130.00, 130.00, 130.00, 130.00, 134.19, 147.65,
    155.73, 161.75, 166.62, 170.77, 174.40, 177.64, 180.47, 183.18,
    185.70, 188.05, 190.26, 192.35, 194.33, 196.22, 198.03, 199.76,
    201.42, 203.02, 204.57, 206.07, 207.52, 208.92, 210.29, 211.62,
    212.91, 214.17, 215.40, 216.60, 217.78, 218.92, 220.05, 221.15,
    222.23, 223.29, 224.33, 225.35, 226.35, 227.33, 228.30, 229.26,
    230.20, 231.12, 232.03, 232.93, 233.81, 234.68, 235.54, 236.39,
    237.23, 238.06, 238.87, 239.68, 240.48, 241.27, 242.05, 242.82,
    243.58, 244.33, 245.08, 245.81, 246.54, 247.26, 247.98, 248.69,
    249.39, 250.08, 250.77, 251.46, 252.13, 252.80, 253.47, 254.12,
    254.78, 255.42, 256.07, 256.70, 257.33, 257.96, 258.58, 259.20,
    259.81, 260.42, 261.03, 261.62, 262.22, 262.81, 263.40, 263.98,
    264.56, 265.13, 265.70, 266.27, 266.83, 267.39, 267.95, 268.50,
    269.05, 269.60, 270.14, 270.68, 271.21, 271.75, 272.28, 272.80,
    273.33, 273.85, 274.37, 274.88, 275.39, 275.90, 276.41, 276.92,
    277.42, 277.92, 278.41, 278.91, 279.40, 279.89, 280.37, 280.86,
    281.34, 281.82, 282.30, 282.77, 283.24, 283.71, 284.18, 284.65,
    285.11, 285.58, 286.04, 286.49, 286.95, 287.40, 287.86, 288.31,
    288.75, 289.20, 289.65, 290.09, 290.53, 290.97, 291.41, 291.84,
    292.28, 292.71, 293.14, 293.57, 293.99, 294.42, 294.84, 295.27,
    295.69, 296.11, 296.52, 296.94, 297.36, 297.77, 298.18, 298.59,
    299.00, 299.41, 299.81, 300.22, 300.62, 301.02, 301.42, 301.82,
    302.22, 302.62, 303.01, 303.41, 303.80, 304.19, 304.58, 304.97,
    305.36, 305.74, 306.13, 306.51, 306.90, 307.28, 307.66, 308.04,
    308.42, 308.79, 309.17, 309.54, 309.92, 310.29, 310.66, 311.03,
    311.40, 311.77, 312.14, 312.51, 312.87, 313.24, 313.60, 313.96,
    314.32, 314.68, 315.04, 315.40, 315.76, 316.12, 316.47, 316.83,
    317.18, 317.53, 317.88, 318.24, 318.59, 318.94, 319.28, 319.63,
    319.98, 320.32, 320.67, 321.01, 321.36, 321.70, 322.04, 322.38,
    322.72, 323.06, 323.40, 323.74, 324.07, 324.41, 324.74, 325.08,
    325.41, 325.75, 326.08, 326.41, 326.74, 327.07, 327.40, 327.73 };
  const static float gms_vh_cal[256] = {
    0.000000, 0.000000, 0.000000, 0.000000, 0.000252, 0.000252, 0.000252,
    0.000252, 0.001008, 0.001008, 0.001008, 0.001008, 0.002268, 0.002268,
    0.002268, 0.002268, 0.004031, 0.004031, 0.004031, 0.004031, 0.006299,
    0.006299, 0.006299, 0.006299, 0.009070, 0.009070, 0.009070, 0.009070,
    0.012346, 0.012346, 0.012346, 0.012346, 0.016125, 0.016125, 0.016125,
    0.016125, 0.020408, 0.020408, 0.020408, 0.020408, 0.025195, 0.025195,
    0.025195, 0.025195, 0.030486, 0.030486, 0.030486, 0.030486, 0.036281,
    0.036281, 0.036281, 0.036281, 0.042580, 0.042580, 0.042580, 0.042580,
    0.049383, 0.049383, 0.049383, 0.049383, 0.056689, 0.056689, 0.056689,
    0.056689, 0.064500, 0.064500, 0.064500, 0.064500, 0.072814, 0.072814,
    0.072814, 0.072814, 0.081633, 0.081633, 0.081633, 0.081633, 0.090955,
    0.090955, 0.090955, 0.090955, 0.100781, 0.100781, 0.100781, 0.100781,
    0.111111, 0.111111, 0.111111, 0.111111, 0.121945, 0.121945, 0.121945,
    0.121945, 0.133283, 0.133283, 0.133283, 0.133283, 0.145125, 0.145125,
    0.145125, 0.145125, 0.157470, 0.157470, 0.157470, 0.157470, 0.170320,
    0.170320, 0.170320, 0.170320, 0.183673, 0.183673, 0.183673, 0.183673,
    0.197531, 0.197531, 0.197531, 0.197531, 0.211892, 0.211892, 0.211892,
    0.211892, 0.226757, 0.226757, 0.226757, 0.226757, 0.242126, 0.242126,
    0.242126, 0.242126, 0.258000, 0.258000, 0.258000, 0.258000, 0.274376,
    0.274376, 0.274376, 0.274376, 0.291257, 0.291257, 0.291257, 0.291257,
    0.308642, 0.308642, 0.308642, 0.308642, 0.326531, 0.326531, 0.326531,
    0.326531, 0.344923, 0.344923, 0.344923, 0.344923, 0.363820, 0.363820,
    0.363820, 0.363820, 0.383220, 0.383220, 0.383220, 0.383220, 0.403124,
    0.403124, 0.403124, 0.403124, 0.423532, 0.423532, 0.423532, 0.423532,
    0.444444, 0.444444, 0.444444, 0.444444, 0.465860, 0.465860, 0.465860,
    0.465860, 0.487780, 0.487780, 0.487780, 0.487780, 0.510204, 0.510204,
    0.510204, 0.510204, 0.533132, 0.533132, 0.533132, 0.533132, 0.556563,
    0.556563, 0.556563, 0.556563, 0.580499, 0.580499, 0.580499, 0.580499,
    0.604938, 0.604938, 0.604938, 0.604938, 0.629882, 0.629882, 0.629882,
    0.629882, 0.655329, 0.655329, 0.655329, 0.655329, 0.681280, 0.681280,
    0.681280, 0.681280, 0.707735, 0.707735, 0.707735, 0.707735, 0.734694,
    0.734694, 0.734694, 0.734694, 0.762157, 0.762157, 0.762157, 0.762157,
    0.790123, 0.790123, 0.790123, 0.790123, 0.818594, 0.818594, 0.818594,
    0.818594, 0.847569, 0.847569, 0.847569, 0.847569, 0.877047, 0.877047,
    0.877047, 0.877047, 0.907029, 0.907029, 0.907029, 0.907029, 0.937516,
    0.937516, 0.937516, 0.937516, 0.968506, 0.968506, 0.968506, 0.968506,
    1.000000, 1.000000, 1.000000, 1.000000 };

  switch (band)
  {
    case IR_BAND:
      switch (sat)
      {
        case GOES_E:
	  for (int i = 0; i < 79; i ++)
	    calibration[i] = 163.0 + (float) i;
	  for (int i = 79; i < 256; i ++)
	    calibration[i] = ((float) i + 405.0) / 2.0;
          break;
        case GOES_W:
	  for (int i = 0; i < 256; i ++)
	    calibration[i] = ((float) i / 255.0) * (318.1 - 159.1) + 159.1;
          break;
        case GMS:
	  memcpy(calibration, gms_ir_cal, 256*sizeof(float));
          break;
        case INDOX:
	  if (!calibrated)
	  {
	    cc = 0.074;
	    sc = 5.0;
	  }
	  for (int i = 0; i < 256; i ++)
	  {
            if (i < sc) rad = 0.0;
	    else rad = cc * ((float) i - sc);
	    calibration[i] = -1272.2000/(log(rad) - 6.7348);
          }
          break;
        default:
          std::cerr << "Cannot calibrate data. Set calibration to 1.0"
                    << std::endl;
          break;
      }
      break;
    case WV_BAND:
      switch (sat)
      {
	case INDOX:
	  if (!calibrated)
	  {
	    cc = 0.0078;
	    sc = 6.0;
	  }
	  for (int i = 0; i < 256; i ++)
	  {
            if (i < sc) rad = 0.0;
	    else rad = cc * ((float) i - sc);
	    calibration[i] = -2264.9000/(log(rad) - 9.1124);
          }
          break;
	default:
          std::cerr << "Cannot calibrate data. Set calibration to 1.0"
                    << std::endl;
	  break;
      }
      break;
    case VH_BAND:
      switch (sat)
      {
        case GOES_E:
        case GOES_W:
	case INDOX:
	  for (int i = 0; i < 256; i ++)
	    calibration[i] = 100.0 * ((float) i / 255.0);
          break;
        case GMS:
	  memcpy(calibration, gms_vh_cal, 256*sizeof(float));
          break;
        default:
          std::cerr << "Cannot calibrate data. Set calibration to 1.0"
                    << std::endl;
          break;
      }
      break;
    default:
      std::cerr << "Cannot calibrate data. Set calibration to 1.0" << std::endl;
      break;
  }
  return calibration;
}
