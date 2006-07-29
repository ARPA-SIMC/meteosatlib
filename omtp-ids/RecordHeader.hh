// $Id: RecordHeader.hh,v 1.1.1.1 2004/07/16 11:05:26 giuliani Exp $
#ifndef DSM_RECORDHEADER_HH
#define DSM_RECORDHEADER_HH

// SYSTEM INCLUDES
//
#include <iosfwd>

// PROJECT INCLUDES
//

// LOCAL INCLUDES
//
#include <omtp-ids/Constants.hh>

// FORWARD REFERENCES
//

// *********************************************************************
//
// NAME:
//   RecordHeader -
//
// TYPE:          C++-CLASS
// SYNOPSIS:
// DESCRIPTION:
// EXAMPLES:
// FILES:         RecordHeader.hh, RecordHeader.cc
// SEE ALSO:
//
// AUTHOR:        Deneys Maartens
// VERSION:       $Rev$
// DATE:          $Date: 2004/07/16 11:05:26 $
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

class RecordHeader
{
	private:

	protected:

		char m_fortran[omtp_ids::FORTRAN_LEN];
		int m_record_header_length;
		int m_no_scanlines;
		int m_record_length;
		int m_zero;

	public:

		// LIFECYCLE

		RecordHeader();

		// OPERATORS

		friend std::ostream& operator<<(std::ostream& os,
						const RecordHeader& recordheader);
		friend std::istream& operator>>(std::istream& is,
						RecordHeader& recordheader);

		// OPERATIONS

		std::ostream& debug(std::ostream& os) const;

		// INQUIRY

		// ACCESS

		// get
		const char* fortran() const;
		int record_header_length() const;
		int no_scanlines() const;
		int record_length() const;
		int zero() const;

		// set
		void fortran(const char* fortran);
		void record_header_length(const int record_header_length);
		void no_scanlines(const int no_scanlines);
		void record_length(const int record_length);
		void zero(const int zero);
};

// EXTERNAL REFERENCES
//

inline
const char*
RecordHeader::fortran() const
{
	return m_fortran;
}

inline
int
RecordHeader::record_header_length() const
{
	return m_record_header_length;
}

inline
int
RecordHeader::no_scanlines() const
{
	return m_no_scanlines;
}

inline
int
RecordHeader::record_length() const
{
	return m_record_length;
}

inline
int
RecordHeader::zero() const
{
	return m_zero;
}

inline
void
RecordHeader::record_header_length(const int record_header_length)
{
	m_record_header_length = record_header_length;
}

inline
void
RecordHeader::no_scanlines(const int no_scanlines)
{
	m_no_scanlines = no_scanlines;
}

inline
void
RecordHeader::record_length(const int record_length)
{
	m_record_length = record_length;
}

inline
void
RecordHeader::zero(const int zero)
{
	m_zero = zero;
}

#endif // DSM_RECORDHEADER_HH
