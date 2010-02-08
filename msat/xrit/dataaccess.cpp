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

#include <msat/xrit/dataaccess.h>
#include <hrit/MSG_HRIT.h>
#include <stdexcept>

using namespace std;

namespace msat {
namespace xrit {

DataAccess::DataAccess() : npixperseg(0)
{
}

DataAccess::~DataAccess()
{
        // Delete the segment cache
        //if (m_segment) delete m_segment;
        for (std::deque<scache>::iterator i = segcache.begin();
                        i != segcache.end(); ++i)
                delete i->segment;
}

void DataAccess::read_file(const std::string& file, MSG_header& head) const
{
        // ProgressTask p("Reading segment " + segnames[idx]);
        std::ifstream hrit(file.c_str(), (std::ios::binary | std::ios::in));
        if (hrit.fail()) throw std::runtime_error(file + ": cannot open");
        head.read_from(hrit);
        hrit.close();
}

void DataAccess::read_file(const std::string& file, MSG_header& head, MSG_data& data) const
{
        // ProgressTask p("Reading segment " + segnames[idx]);
        std::ifstream hrit(file.c_str(), (std::ios::binary | std::ios::in));
        if (hrit.fail())
                throw std::runtime_error(file + ": cannot open");
        head.read_from(hrit);
        if (head.segment_id && head.segment_id->data_field_format == MSG_NO_FORMAT)
                throw std::runtime_error(file + ": product dumped in binary format");
        data.read_from(hrit, head);
        hrit.close();
}

void DataAccess::scanSegment(const MSG_header& header)
{
        // Decoding information
        int totalsegs = header.segment_id->planned_end_segment_sequence_number;
        int seglines = header.image_structure->number_of_lines;
#if 0
        cerr << "NCOL " << header.image_structure->number_of_columns << endl; 
        cerr << "NLIN " << header.image_structure->number_of_lines << endl; 
#endif
        columns = header.image_structure->number_of_columns;
        lines = seglines * totalsegs;
        npixperseg = columns * seglines;
        hrv = header.segment_id->spectral_channel_id == MSG_SEVIRI_1_5_HRV;

        // See if the image needs to be rotated
        swapX = header.image_navigation->column_scaling_factor < 0;
        swapY = header.image_navigation->line_scaling_factor < 0;
}

void DataAccess::scan(const std::vector<std::string>& segfiles, MSG_header& header)
{
        for (vector<string>::const_iterator i = segfiles.begin();
                        i != segfiles.end(); ++i)
        {
                //p.activity("Scanning segment " + *i);
                read_file(*i, header);
                if (header.segment_id->data_field_format == MSG_NO_FORMAT)
                        throw std::runtime_error(*i + ": product dumped in binary format");

                int idx = header.segment_id->sequence_number-1;
                if (idx < 0) continue;
                if ((size_t)idx >= segnames.size())
                        segnames.resize(idx + 1);
                segnames[idx] = *i;
        }

        if (segnames.empty()) throw std::runtime_error("no segments found");

        // Read common info just once from a random segment
        scanSegment(header);
}

MSG_data* DataAccess::segment(size_t idx) const
{
        // Check to see if the segment we need is the current one
        if (segcache.empty() || segcache.begin()->segno != idx)
        {
                // If not, check to see if we can find the segment in the cache
                std::deque<scache>::iterator i = segcache.begin();
                for ( ; i != segcache.end(); ++i)
                        if (i->segno == idx)
                                break;
                if (i == segcache.end())
                {
                        // Not in cache: we need to load it

                        // Do not load missing segments
                        if (idx >= segnames.size()) return 0;
                        if (segnames[idx].empty()) return 0;

                        // Remove the last recently used if the cache is full
                        if (segcache.size() == 2)
                        {
                                delete segcache.rbegin()->segment;
                                segcache.pop_back();
                        }

                        // Load the segment
                        // ProgressTask p("Reading segment " + segnames[idx]);
                        MSG_header header;
                        scache new_scache;
                        new_scache.segment = new MSG_data;
                        new_scache.segno = idx;
                        read_file(segnames[idx].c_str(), header, *new_scache.segment);

                        // Put it in the front
                        segcache.push_front(new_scache);
                } else {
                        // The segment is in the cache: bring it to the front
                        scache tmp = *i;
                        segcache.erase(i);
                        segcache.push_front(tmp);
                }
        }
        return segcache.begin()->segment;
}

}
}
