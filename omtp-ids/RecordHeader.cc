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

// SYSTEM INCLUDES
//
#include <cstring>
#include <iostream>

// PROJECT INCLUDES
//
#include "ByteSex.hh"

// LOCAL INCLUDES
//
#include "RecordHeader.hh" // class implemented

// FORWARD REFERENCES
//
using namespace omtp_ids;

// **************************** PRIVATE    *****************************

// **************************** PROTECTED  *****************************

// **************************** PUBLIC     *****************************

// ============================ LIFECYCLE  =============================

RecordHeader::RecordHeader() :
	m_record_header_length(RECORDHEADER_LEN),
	m_no_scanlines(0),
	m_record_length(0),
	m_zero(0)
{
	memcpy(m_fortran, FORTRAN, FORTRAN_LEN);
}

// ============================ OPERATORS  =============================

std::ostream&
operator<<(std::ostream& os,
	   const RecordHeader& recordheader)
{
	os.write(recordheader.m_fortran, FORTRAN_LEN);
	ByteSex::big::write2(os, recordheader.m_record_header_length);
	ByteSex::big::write2(os, recordheader.m_no_scanlines        );
	ByteSex::big::write2(os, recordheader.m_record_length       );
	ByteSex::big::write2(os, recordheader.m_zero                );

	return os;
}

std::istream&
operator>>(std::istream& is,
	   RecordHeader& recordheader)
{
	recordheader = RecordHeader();

	is.read(recordheader.m_fortran, FORTRAN_LEN);
	recordheader.m_record_header_length = ByteSex::big::read2(is);
	recordheader.m_no_scanlines         = ByteSex::big::read2(is);
	recordheader.m_record_length        = ByteSex::big::read2(is);
	recordheader.m_zero                 = ByteSex::big::read2(is);

	return is;
}

// ============================ OPERATIONS =============================

std::ostream&
RecordHeader::debug(std::ostream& os) const
{
	os << "RecordHeader :";

	os << "\n  fortran bytes        : \"";
	for (int i = 0; i < FORTRAN_LEN; i++) {
		os << static_cast<int>(m_fortran[i]);
	}
	os << "\"";

	os << "\n  record header length : " << m_record_header_length;
	os << "\n  number of scanlines  : " << m_no_scanlines;
	os << "\n  record length        : " << m_record_length;
	os << "\n  zero                 : " << m_zero;
	os << '\n';

	return os;
}

// ============================ INQUIRY    =============================

// ============================ ACCESS     =============================

void
RecordHeader::fortran(const char* fortran)
{
	const int len = strlen(fortran);
	const int copy_len = (len <= FORTRAN_LEN)
		? len
		: FORTRAN_LEN;

	memcpy(m_fortran, fortran, copy_len);
}
