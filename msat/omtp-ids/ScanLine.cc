/*
 * Copyright: (C) 2004, 2005, 2006 Deneys S. Maartens
 * Copyright: (C) 2012  ARPAE-SIMC <urpsim@arpae.it>
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

// SYSTEM INCLUDES
//
#include <iostream>

// PROJECT INCLUDES
//

// LOCAL INCLUDES
//
#include "ScanLine.hh" // class implemented

// FORWARD REFERENCES
//
using namespace omtp_ids;

// **************************** PRIVATE    *****************************

// **************************** PROTECTED  *****************************

// **************************** PUBLIC     *****************************

// ============================ LIFECYCLE  =============================

// ============================ OPERATORS  =============================

std::ostream&
operator<<(std::ostream& os,
	   const ScanLine& scanline)
{
	os << scanline.m_lineheader;

	const unsigned no_pixels = scanline.m_linepixel.size();

	for (unsigned i = 0; i < no_pixels; i++) {
		os.put(scanline.m_linepixel[i]);
	}

	const int offset = scanline.m_lineheader.length()
		- no_pixels - LINEHEADER_LEN;
	for (int i = 0; i < offset; i++) {
		os.put('\0');
	}

	return os;
}

std::istream&
operator>>(std::istream& is,
	   ScanLine& scanline)
{
	scanline = ScanLine();

	is >> scanline.m_lineheader;
	if (!is.good()) {
		throw "failure while reading line header\n";
	}

	const int no_pixels = scanline.m_lineheader.no_pixels();
	scanline.m_linepixel.resize(no_pixels);

	for (int i = 0; i < no_pixels; i++) {
		scanline.m_linepixel[i] = is.get();
		if (!is.good()) {
			throw "failure while reading line pixel\n";
		}
	}

	const int offset = scanline.m_lineheader.length()
		- no_pixels - LINEHEADER_LEN;

	if (offset < 0) {
		is.setstate(std::ios::failbit);
	}
	// no seekg() available on gzstream
	for (int i = 0; i < offset; i++) {
		is.get();
	}

	return is;
}

// ============================ OPERATIONS =============================

std::ostream&
ScanLine::debug(std::ostream& os) const
{
	os << "ScanLine :";
	os << "\n  number of linepixels : " << m_linepixel.size();
	os << "\n";
	m_lineheader.debug(os);
	return os;
}

// ============================ INQUIRY    =============================

// ============================ ACCESS     =============================
