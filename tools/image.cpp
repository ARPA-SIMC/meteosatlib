/*
 * image - Image conversion/display functions
 *
 * Copyright (C) 2005--2010  ARPA-SIM <urpsim@smr.arpa.emr.it>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 *
 * Author: Enrico Zini <enrico@enricozini.com>
 */

#include "image.h"
#include <gdal.h>
#include <gdal_priv.h>
#include <stdint.h>
#include "config.h"
#include <Magick++.h>

using namespace std;

namespace msat {

template<typename T>
void Stretch::compute_if_needed(GDALRasterBand& band, const T* vals, T& vmin, T& vmax)
{
  if (!compute)
  {
    vmin = min;
    vmax = max;
    return;
  }

  size_t sx = band.GetXSize();
  size_t sy = band.GetYSize();
  T missing = (T)band.GetNoDataValue();

  // Compute min and max values
  vmin = missing;
  vmax = missing;
  for (size_t i = 0; i < sx*sy; ++i)
  {
    if (vals[i] == missing) continue;
    if (vmin == missing)
    {
      // First time we find a good value, init min and max with it
      vmin = vmax = vals[i];
      continue;
    }
    if (vals[i] < vmin) vmin = vals[i];
    if (vals[i] > vmax) vmax = vals[i];
  }
}

template<typename T>
uint8_t* Stretch::rescale(GDALRasterBand& band, const T* vals, T vmin, T vmax)
{
  size_t sx = band.GetXSize();
  size_t sy = band.GetYSize();
  uint8_t* res8 = new uint8_t[sx * sy];
  for (size_t i = 0; i < sx * sy; ++i)
  {
    if (vals[i] < vmin)
      res8[i] = 0;
    else if (vals[i] > vmax)
      res8[i] = 255;
    else
      res8[i] = (vals[i] - vmin) * 255 / (vmax - vmin);
  }
  return res8;
}

static std::auto_ptr<Magick::Image> imageToMagick(GDALRasterBand& band, Stretch& s)
{
        auto_ptr<Magick::Image> image;
        size_t sx = band.GetXSize();
        size_t sy = band.GetYSize();
        size_t tx = sx;
        size_t ty = sy;

        // Read the image data
        switch (band.GetRasterDataType())
        {
                case GDT_Byte: {
                        uint8_t* res = new uint8_t[tx * sy];
                        if (band.RasterIO(GF_Read, 0, 0, sx, sy, res, tx, ty, GDT_Byte, 0, 0) != CE_None)
                        {
                                delete[] res;
                                return std::auto_ptr<Magick::Image>(0);
                        }

                        // Compute min and max values
                        uint8_t vmin;
                        uint8_t vmax;
                        s.compute_if_needed(band, res, vmin, vmax);

                        // Rescale to 8 bits
                        uint8_t* res8 = s.rescale(band, res, vmin, vmax);
                        delete[] res;

                        image.reset(new Magick::Image(tx, ty, "I", Magick::CharPixel, res8));
                        delete[] res8;
                        break;
                }
                case GDT_UInt16: {
                        uint16_t* res = new uint16_t[tx * sy];
                        if (band.RasterIO(GF_Read, 0, 0, sx, sy, res, tx, ty, GDT_UInt16, 0, 0) != CE_None)
                        {
                                delete[] res;
                                return std::auto_ptr<Magick::Image>(0);
                        }

                        // Compute min and max values
                        uint16_t vmin;
                        uint16_t vmax;
                        s.compute_if_needed(band, res, vmin, vmax);

                        // Rescale to 8 bits
                        uint8_t* res8 = s.rescale(band, res, vmin, vmax);
                        delete[] res;

                        image.reset(new Magick::Image(tx, ty, "I", Magick::CharPixel, res8));
                        delete[] res8;
                        break;
                }
                default: {
                        float* res = new float[tx * ty];
                        if (band.RasterIO(GF_Read, 0, 0, sx, sy, res, tx, ty, GDT_Float32, 0, 0) != CE_None)
                        {
                                delete[] res;
                                return std::auto_ptr<Magick::Image>(0);
                        }

                        // Compute min and max values
                        float vmin = 0;
                        float vmax = 0;
                        s.compute_if_needed(band, res, vmin, vmax);

                        // Rescale to 8 bits
                        uint8_t* res8 = s.rescale(band, res, vmin, vmax);
                        delete[] res;

                        image.reset(new Magick::Image(tx, ty, "I", Magick::CharPixel, res8));
                        delete[] res8;
                        break;
                }
        }

        return image;
}

bool export_image(GDALRasterBand* band, const std::string& fileName, Stretch& s)
{
        // ProgressTask p("Exporting image to " + fileName);

        size_t pos = fileName.rfind('.');
        if (pos == string::npos)
        {
                CPLError(CE_Failure, CPLE_AppDefined, "file name %s has no extension, so I cannot guess the file format", fileName.c_str());
                return false;
        }

        auto_ptr<Magick::Image> image = imageToMagick(*band, s);

        // p.activity("Writing image to file");
        image->write(fileName);
        return true;
}

bool display_image(GDALRasterBand* band, Stretch& s)
{
        //ProgressTask p("Displaying image " + defaultFilename(band));
        auto_ptr<Magick::Image> image = imageToMagick(*band, s);

        if (image.get() == 0) return false;

        //p.activity("Displaying image");
        image->display();

        return true;
}

}

// vim:set ts=2 sw=2:
