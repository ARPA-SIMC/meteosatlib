/* $Id$
 * Copyright: (C) 2004, 2005, 2006 Deneys S. Maartens
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
 * USA.
 */
/*
 * OpenMTP-IDS_to_pgm - OpenMTP-IDS to pgm conversion
 */

// SYSTEM INCLUDES
//
#include <iostream>
#include <fstream>

// PROJECT INCLUDES
//
#include <config.h>

// LOCAL INCLUDES
//
#include <omtp-ids/OpenMTP-IDS.hh>

// FORWARD REFERENCES
//

// *********************************************************************

const char*
filename(unsigned id)
{
	omtp_ids::Channel_Id cid =
		static_cast<omtp_ids::Channel_Id>(id);

	switch (cid) {
		case omtp_ids::VIS_1   : // fall
		case omtp_ids::VIS_2   : // fall
		case omtp_ids::VIS_1_2 : // fall
		case omtp_ids::VIS_3   : // fall
		case omtp_ids::VIS_4   : // fall
		case omtp_ids::VIS_2_3 : // fall
		case omtp_ids::VIS_2_4 : // fall
		case omtp_ids::VIS_3_4 : return "vis.pgm";
		case omtp_ids::IR_1    : // fall
		case omtp_ids::IR_2    : return "ir.pgm";
		case omtp_ids::WV_1    : // fall
		case omtp_ids::WV_2    : return "wv.pgm";
		default                : return "unknown-channel.pgm";
	}
}

// write an image to a pgm file
//
// returns true on success and false on failure
bool
write_pgm(const char* filename,
	  const char* comment,
	  const std::vector<std::vector<unsigned char> > image)
{
	const unsigned size_x = image.size();
	if (!size_x) {
		return false;
	}

	const unsigned size_y_0 = image[0].size();
	if (!size_y_0) {
		return false;
	}

	std::ofstream os(filename);
	if (!os.good()) {
		return false;
	}

	os << "P5" << std::endl;
	if (comment) {
		os << "# " << comment << std::endl;
	}
	os << size_y_0 << " " << size_x << std::endl
		<< "255" << std::endl;

	for (int x = 0; x < (int) size_x; ++x) {
		const unsigned size_y = image[x].size();
		if (size_y != size_y_0) {
			os.close();
			return false;
		}
		for (int y = 0; y < (int) size_y; ++y) {
			os.put(image[x][y]);
		}
	}
	os.close();
	std::cout << "wrote " << filename << std::endl;
	return true;
}

// write a channel to a pgm
//
// returns true if the channel was written to file successfully and
// false on failure
bool
write_channel_pgm(unsigned channel_id, const OpenMTP_IDS& omtp_ids)
{
	if (channel_id == omtp_ids::NO_DATA) {
		return false;
	}

	std::vector<std::vector<unsigned char> > image;

	const unsigned no_recs = omtp_ids.fileheader().no_records() - 1;

	for (unsigned rec = 0; rec < no_recs; ++rec) {
		const Record& r = omtp_ids.record()[rec];

		const unsigned no_scans = r.recordheader().no_scanlines();
		for (unsigned scan = 0; scan < no_scans; ++ scan) {
			const ScanLine& s = r.scanline()[scan];

			if (s.lineheader().channel_id()
			    == (int) channel_id) {
				image.push_back(s.linepixel());
			}
		}
	}

	return write_pgm(filename(channel_id), 0, image);
}

// write the three channels in an ids file to pgm's
//
// returns true if any of the channels was written successfully, and
// false of all three writes failed
bool
openmtp_ids_to_pgm(const OpenMTP_IDS& omtp_ids)
{
	const FileHeader& fh = omtp_ids.fileheader();

	bool ret = false;

	ret = write_channel_pgm(fh.vis_id(), omtp_ids);
	ret = write_channel_pgm(fh.ir_id(),  omtp_ids);
	ret = write_channel_pgm(fh.wv_id(),  omtp_ids);

	return (! ret);
}

// EXTERNAL REFERENCES
//

// reads an openmtp-ids file, specified on the command line, and writes
// the channel data to pgm files
//
// the output filenames are:
//    `vis.pgm' for the visual channel
//    `ir.pgm' for the infra-red channel
//    `wv.pgm' for the water vapour channel
//
// returns EXIT_SUCCESS if any of the tree channels was successfully
// written
int
main(int argc, char* argv[])
{
	if (argc < 2) {
		std::cout << "Usage : " << argv[0]
			<< " OpenMTP_filename\n";

		return EXIT_FAILURE;
	}

	if (!strcmp(argv[1], "-V")) {
		std::cout << argv[0] << " " << PACKAGE_STRING << std::endl;
		return 0;
	}

	OpenMTP_IDS omtp_ids;
	try {
		omtp_ids = OpenMTP_IDS(argv[1]);
	} catch (const char* err) {
		std::cout << argv[0] << ": " << err << std::endl;
		return EXIT_FAILURE;
	}

	if (!openmtp_ids_to_pgm(omtp_ids)) {
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
