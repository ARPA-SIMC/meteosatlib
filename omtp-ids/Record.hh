// $Id$
#ifndef DSM_RECORD_HH
#define DSM_RECORD_HH

// SYSTEM INCLUDES
//
#include <iosfwd>

// PROJECT INCLUDES
//

// LOCAL INCLUDES
//
#include <omtp-ids/RecordHeader.hh>
#include <omtp-ids/ScanLine.hh>

// FORWARD REFERENCES
//

// *********************************************************************
//
// NAME:
//   Record -
//
// TYPE:          C++-CLASS
// SYNOPSIS:
// DESCRIPTION:
// EXAMPLES:
// FILES:         Record.hh, Record.cc
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

class Record
{
	private:

	protected:

		RecordHeader m_recordheader;
		std::vector<ScanLine> m_scanline;

	public:

		// LIFECYCLE

		// OPERATORS

		friend std::ostream& operator<<(std::ostream& os,
						const Record& record);
		friend std::istream& operator>>(std::istream& is,
						Record& record);

		const ScanLine& operator[](const unsigned index) const;
		ScanLine& operator[](const unsigned index);

		// OPERATIONS

		std::ostream& debug(std::ostream& os) const;

		// INQUIRY

		// ACCESS

		// get
		const RecordHeader& recordheader() const;
		RecordHeader& recordheader();
		const std::vector<ScanLine>& scanline() const;
		std::vector<ScanLine>& scanline();

		// set
		void recordheader(const RecordHeader& recordheader);
		void scanline(const std::vector<ScanLine>& scanline);
};

// EXTERNAL REFERENCES
//

inline
const ScanLine&
Record::operator[](const unsigned index) const
{
	return m_scanline[index];
}

inline
ScanLine&
Record::operator[](const unsigned index)
{
	return m_scanline[index];
}

inline
const RecordHeader&
Record::recordheader() const
{
	return m_recordheader;
}

inline
RecordHeader&
Record::recordheader()
{
	return m_recordheader;
}

inline
const std::vector<ScanLine>&
Record::scanline() const
{
	return m_scanline;
}

inline
std::vector<ScanLine>&
Record::scanline()
{
	return m_scanline;
}

inline
void
Record::recordheader(const RecordHeader& recordheader)
{
	m_recordheader = recordheader;
}

inline
void
Record::scanline(const std::vector<ScanLine>& scanline)
{
	m_scanline = scanline;
}

#endif // DSM_RECORD_HH
