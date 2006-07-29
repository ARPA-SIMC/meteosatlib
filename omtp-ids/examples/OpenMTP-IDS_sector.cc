// $Id: OpenMTP-IDS_sector.cc,v 1.2 2004/12/10 09:54:32 giuliani Exp $

#include <config.h>

// SYSTEM INCLUDES
//
#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>
#include <cmath>

#include <libgen.h>

// PROJECT INCLUDES
//

// LOCAL INCLUDES
//
#include <omtp-ids/OpenMTP-IDS.hh>

// FORWARD REFERENCES
//
using namespace omtp_ids;

// *********************************************************************
//
// NAME:
//   OpenMTP-IDS_sector - OpenMTP-IDS sector extraction
//
// TYPE:          C++-PROGRAMME
// SYNOPSIS:
// DESCRIPTION:
// EXAMPLES:
// FILES:         OpenMTP-IDS_sector.cc
// SEE ALSO:      OpenMTP-IDS library
//
// AUTHOR:        Deneys Maartens
// VERSION:       $Rev$
// DATE:          $Date: 2004/12/10 09:54:32 $
// COPYRIGHT:     Deneys Maartens (C) 2004
//
// LICENCE:
//   This program is free software; you can redistribute it and/or
//   modify it under the terms of the GNU General Public License as
//   published by the Free Software Foundation; either version 2, or (at
//   your option) any later version.
//
//   This program is distributed in the hope that it will be useful, but
//   WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//   General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program; if not, write to the Free Software
//   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//   02111-1307, USA.
//
// *********************************************************************

// LOCAL CONSTANTS
//

static const unsigned LINE_0  = 3 * (1770 / 2);
static const unsigned LINE_1  = 3 * (2110 / 2);
static const unsigned PIXEL_0 = (1092 / 2);
static const unsigned PIXEL_1 = (1604 / 2);

int
record_lines(const int lines, const int pixels)
{
	return static_cast<int>
		(rint(sqrt((8 * lines)
			   / static_cast<double>(pixels))));
}


int
no_records(const int scanlines, const int lines_per_record)
{
	return static_cast<int>
		(ceil(scanlines
		      / static_cast<double>(lines_per_record)));
}


std::vector<Record>
repack(std::vector<ScanLine> scanline)
{
	const int pixels = scanline[0].linepixel().size();
	const int lines = record_lines(scanline.size(), pixels);
	const int records = no_records(scanline.size(), lines);

	Record rec;
	rec.recordheader().no_scanlines(lines);
	rec.recordheader()
		.record_length(lines * (pixels + LINEHEADER_LEN) +
			       RECORDHEADER_LEN);

	std::vector<Record> record;
	fill_n(back_inserter(record), records, rec);

	std::vector<ScanLine>::iterator line = scanline.begin();
	std::vector<ScanLine>::iterator end = scanline.end();

	ScanLine dummy;
	dummy.linepixel().resize(pixels);
	dummy.lineheader().length(pixels + LINEHEADER_LEN);
	dummy.lineheader().no_pixels(pixels);

	for (int i = 0; i < records; ++i)
		for (int j = 0; j < lines; j++)
			if (line != end) {
				record[i].scanline().push_back(*line);
				++line;
			} else
				record[i].scanline().push_back(dummy);

	return record;
}


void
sector_line(ScanLine& line)
{
	std::vector<unsigned char>::iterator begin = line.linepixel().begin();
	std::vector<unsigned char>::iterator end = begin;

	begin += PIXEL_0;
	end += PIXEL_1;

	std::vector<unsigned char> pixels;
	copy(begin, end, back_inserter(pixels));
	const int no_pixels = pixels.size();

	line.lineheader().length(LINEHEADER_LEN + no_pixels);
	line.lineheader().start(PIXEL_0);
	line.lineheader().no_pixels(no_pixels);
	line.linepixel(pixels);
}


std::vector<Record>
sector(const OpenMTP_IDS& omtp_ids)
{
	std::vector<ScanLine> lines = omtp_ids.scanline(LINE_0, LINE_1);

	for_each(lines.begin(), lines.end(), sector_line);
	return repack(lines);
}


bool
openmtp_ids_sector(const OpenMTP_IDS& omtp_ids)
{
	OpenMTP_IDS ids;
	ids.record(sector(omtp_ids));
	std::cout << "write..." << std::endl;
	ids.fileheader(omtp_ids.fileheader());
	ids.fileheader().no_records(ids.record().size() + 1);
	ids.fileheader().record_length(ids.record()[0].recordheader().record_length());

	return ids.write("omtp-ids-sector.ids");
}


// EXTERNAL REFERENCES
//

// reads an openmtp-ids file, specified on the command line, and writes
// a sector of the image to file.
//
// the output filename is opmp-ids-sector.ids
//
// returns EXIT_SUCCESS if the sector was successfully written
int
main(int argc, char* argv[])
{
	if (argc < 2) {
		std::cout << "Usage: "
			<< basename(argv[0])
			<< " OpenMTP_filename\n";

		return EXIT_FAILURE;
	}

        if (!strcmp(argv[1], "-V")) {
                std::cout << argv[0] << " " << PACKAGE_STRING << std::endl;
                return 0;
        }

	std::cout << "read..." << std::endl;
	OpenMTP_IDS omtp_ids;
	try {
		omtp_ids = OpenMTP_IDS(argv[1]);
	} catch (char* err) {
		std::cout << basename(argv[0]) << ": "
			<< err << std::endl;
		return EXIT_FAILURE;
	}

	if (!openmtp_ids_sector(omtp_ids))
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}

