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

static std::auto_ptr<Magick::Image> imageToMagick(GDALRasterBand& band)
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
                        image.reset(new Magick::Image(tx, ty, "I", Magick::CharPixel, res));
                        delete[] res;
                        image->normalize();
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
                        uint16_t vmin = res[0];
                        uint16_t vmax = res[0];
                        for (unsigned i = 0; i < tx*ty; ++i)
                        {
                            if (res[i] < vmin) vmin = res[i];
                            if (res[i] > vmax) vmax = res[i];
                        }

                        // Rescale to 8 bits
                        uint8_t* res8 = new uint8_t[tx * ty];
                        for (unsigned i = 0; i < tx*ty; ++i)
                            res8[i] = (res[i] - vmin) * 256 / (vmax - vmin);
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
                        bool first = true;
                        float vmin = 0;
                        float vmax = 0;
                        for (unsigned i = 0; i < tx*ty; ++i)
                        {
                            if (isnan(res[i])) continue;
                            if (first)
                            {
                                vmin = vmax = res[i];
                                first = false;
                                continue;
                            }
                            if (res[i] < vmin) vmin = res[i];
                            if (res[i] > vmax) vmax = res[i];
                        }

                        // Rescale to 8 bits
                        uint8_t* res8 = new uint8_t[tx * ty];
                        for (unsigned i = 0; i < tx*ty; ++i)
                            res8[i] = (res[i] - vmin) * 256 / (vmax - vmin);
                        delete[] res;

                        image.reset(new Magick::Image(tx, ty, "I", Magick::CharPixel, res8));
                        delete[] res8;
                        break;
                }
        }

        return image;
}

bool export_image(GDALRasterBand* band, const std::string& fileName)
{
        // ProgressTask p("Exporting image to " + fileName);

        size_t pos = fileName.rfind('.');
        if (pos == string::npos)
        {
                CPLError(CE_Failure, CPLE_AppDefined, "file name %s has no extension, so I cannot guess the file format", fileName.c_str());
                return false;
        }

        auto_ptr<Magick::Image> image = imageToMagick(*band);

        // p.activity("Writing image to file");
        image->write(fileName);
        return true;
}

bool display_image(GDALRasterBand* band)
{
        //ProgressTask p("Displaying image " + defaultFilename(band));
        auto_ptr<Magick::Image> image = imageToMagick(*band);

        if (image.get() == 0) return false;

        //p.activity("Displaying image");
        image->display();

        return true;
}

}

// vim:set ts=2 sw=2:
