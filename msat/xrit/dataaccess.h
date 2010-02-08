#ifndef MSAT_XRIT_DATAACCESS_H
#define MSAT_XRIT_DATAACCESS_H

/*
 * xrit/dataaccess - Higher level data access for xRIT files
 *
 * Copyright (C) 2007--2010  ARPA-SIM <urpsim@smr.arpa.emr.it>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Enrico Zini <enrico@enricozini.org>
 */

#include <string>
#include <vector>
#include <deque>
#include <hrit/MSG_data_image.h>

struct MSG_header;
struct MSG_data;

namespace msat {
namespace xrit {

/**
 * Higher level data access for xRIT files
 */
struct DataAccess
{
        /// Number of pixels in every segment
        size_t npixperseg;

        /// True if the image needs to be swapped horizontally
        bool swapX;

        /// True if the image needs to be swapped vertically
        bool swapY;

        /// True if the image is an HRV image divided in two parts
        bool hrv;

        /// Pathnames of the segment files, indexed with their index
        std::vector<std::string> segnames;

        struct scache
        {
                MSG_data* segment;
                size_t segno;
        };
        /// Segment cache
        mutable std::deque<scache> segcache;

        /// Calibration vector
        float* calibration;

        /// Length of a scanline
        size_t columns;

        /// Number of scanlines
        size_t lines;


        DataAccess();
        ~DataAccess();

        /**
         * Read a xRIT file (prologue, epilogue or segment)
         */
        void read_file(const std::string& file, MSG_header& head, MSG_data& data) const;

        /**
         * Read only the xRIT header of a file
         */
        void read_file(const std::string& file, MSG_header& head) const;

        /**
         * Return the X offset at which the given line starts
         *
         * This is always 0 for non-HRV. For HRV, it is the offset a line needs
         * to be shifted to geographically align it in the combined image.
         */
        size_t line_start(size_t line);

        /**
         * Return the MSG_data corresponding to the segment with the given index.
         *
         * The pointer could be invalidated by another call to segment()
         */
        MSG_data* segment(size_t idx) const;
};

}
}

#endif
