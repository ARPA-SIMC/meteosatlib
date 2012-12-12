/*
 * Copyright: (C) 2004, 2005, 2006 Deneys S. Maartens
 * Copyright: (C) 2012  ARPA-SIM <urpsim@smr.arpa.emr.it>
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
#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>

// PROJECT INCLUDES
//
#include "ByteSex.hh"
#include "FileHeader.hh"
#include "Record.hh"

// LOCAL INCLUDES
//
#include "OpenMTP-IDS.hh" // class implemented

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
	   const OpenMTP_IDS& openmtp_ids)
{
	os << openmtp_ids.m_fileheader;

	// the first record was just written..
	const unsigned no_records =
		openmtp_ids.m_fileheader.no_records() - 1;

	for (unsigned i = 0; i < no_records; i++) {
		os << openmtp_ids.m_record[i];
	}

	return os;
}

std::istream&
operator>>(std::istream& is,
	   OpenMTP_IDS& openmtp_ids)
{
	openmtp_ids = OpenMTP_IDS();

	is >> openmtp_ids.m_fileheader;
	if (!is.good()) {
		throw "failure while reading file header\n";
	}

	// the first record was just read..
	const unsigned no_records =
		openmtp_ids.m_fileheader.no_records() - 1;
	openmtp_ids.m_record.resize(no_records);

	for (unsigned i = 0; i < no_records; i++) {
		is >> openmtp_ids.m_record[i];
		if (!is.good()) {
			throw "failure while reading record\n";
		}
	}

	return is;
}

// ============================ OPERATIONS =============================

bool
OpenMTP_IDS::read(const char* filename)
{
	std::ifstream is(filename);
	if (!is) {
		std::string err;
		err += "could not open ";
		err += filename;
		err += " for reading";
		throw err.c_str();
	}

	is >> *this;

	if (!is.good()) {
		std::string err;
		err += "error while reading ";
		err += filename;
		throw err.c_str();
	}

	is.close();

	return true;
}

bool
OpenMTP_IDS::write(const char* filename)
{
	std::ofstream os(filename);
	if (!os) {
		std::string err;
		err += "could not open ";
		err += filename;
		err += " for writing";
		throw err.c_str();
	}

	os << *this;

	if (!os.good()) {
		std::string err;
		err += "error while writing ";
		err += filename;
		throw err.c_str();
	}

	os.close();
	return true;
}

std::ostream&
OpenMTP_IDS::debug(std::ostream& os) const
{
	os << "-- * OpenMTP-IDS * --\n";

	m_fileheader.debug(os);

	const unsigned no_records = m_record.size();
	for (unsigned i = 0; i < no_records; i++) {
		m_record[i].debug(os);
	}

	return os;
}

// ============================ INQUIRY    =============================

// ============================ ACCESS     =============================

// all the scanlines
std::vector<ScanLine>
OpenMTP_IDS::scanline() const
{
	unsigned last = 0;

	const unsigned no_records = m_record.size();
	for (unsigned i = 0; i < no_records; i++) {
		last += m_record[i].scanline().size();
	}

	return scanline(0, last - 1);
}

// a scanline
std::vector<ScanLine>
OpenMTP_IDS::scanline(const unsigned line) const
{
	return scanline(line, line + 1);
}

// all the scanlines in a range
std::vector<ScanLine>
OpenMTP_IDS::scanline(unsigned from,
		      unsigned to) const
{
	if (to <= from) {
		return std::vector<ScanLine>();
	}

	std::vector<ScanLine> lines;

	std::vector<Record>::const_iterator rec = m_record.begin();
	std::vector<Record>::const_iterator end_rec = m_record.end();
	while (rec != end_rec) {
		const std::vector<ScanLine>& scanline = rec->scanline();
		const unsigned size = scanline.size();

		if (from < size) {
			std::vector<ScanLine>::const_iterator begin = scanline.begin();
			std::vector<ScanLine>::const_iterator end = begin;

			begin += from;
			end += std::min(to, size);

			copy(begin, end, back_inserter(lines));
		}

		if (to < size) {
			// we found the end
			break;
		}

		from = (from < size) ? 0 : from - size;
		to -= size;

		rec++;
	}

	return lines;
}
