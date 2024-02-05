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
#include "Record.hh" // class implemented

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
	   const Record& record)
{
	os << record.m_recordheader;

	const int no_scanlines = record.m_scanline.size();
	for (int i = 0; i < no_scanlines; i++) {
		os << record.m_scanline[i];
	}

	return os;
}

std::istream&
operator>>(std::istream& is,
	   Record& record)
{
	record = Record();

	is >> record.m_recordheader;
	if (!is.good()) {
		throw "failure while reading record header\n";
	}

	const int no_scanlines = record.m_recordheader.no_scanlines();
	record.m_scanline.resize(no_scanlines);

	for (int i = 0; i < no_scanlines; i++) {
		is >> record.m_scanline[i];
		if (!is.good()) {
			throw "failure while reading scan line\n";
		}
	}

	return is;
}

// ============================ OPERATIONS =============================

std::ostream&
Record::debug(std::ostream& os) const
{
	os << "-- Record --\n";
	m_recordheader.debug(os);

	const unsigned no_scanlines = m_scanline.size();
	for (unsigned i = 0; i < no_scanlines; i++) {
		m_scanline[i].debug(os);
	}

	return os;
}

// ============================ INQUIRY    =============================

// ============================ ACCESS     =============================
