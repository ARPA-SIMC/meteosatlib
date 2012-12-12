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
/*
 * Record
 */
#ifndef DSM_RECORD_HH
#define DSM_RECORD_HH

// SYSTEM INCLUDES
//
#include <iosfwd>

// PROJECT INCLUDES
//
#include <msat/omtp-ids/RecordHeader.hh>
#include <msat/omtp-ids/ScanLine.hh>

// LOCAL INCLUDES
//

// FORWARD REFERENCES
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
