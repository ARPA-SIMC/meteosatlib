/*
 * image - Image conversion/display functions
 *
 * Copyright (C) 2005--2010  ARPAE-SIMC <urpsim@arpae.it>
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

#ifndef MSAT_TOOL_IMAGE_H
#define MSAT_TOOL_IMAGE_H

#include <string>
#include <memory>
#include <stdint.h>

struct GDALRasterBand;

namespace msat {

struct Stretch
{
    double min;
    double max;
    bool compute;

    Stretch() : compute(true) {}

    template<typename T>
    void compute_if_needed(GDALRasterBand& band, const T* vals, T& vmin, T& vmax);

    template<typename T>
    uint8_t* rescale(GDALRasterBand& band, const T* vals, T vmin, T vmax);
};

/// Export data from an ImageData to an image file
bool export_image(GDALRasterBand* band, const std::string& fileName, Stretch& s);

/// Display an image on screen
bool display_image(GDALRasterBand* band, Stretch& s);

}

#endif
