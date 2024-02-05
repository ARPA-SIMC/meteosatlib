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
/*
 * Scan Line
 */
#ifndef DSM_SCANLINE_HH
#define DSM_SCANLINE_HH

// SYSTEM INCLUDES
//
#include <iosfwd>
#include <vector>

// PROJECT INCLUDES
//
#include <msat/omtp-ids/LineHeader.hh>

// LOCAL INCLUDES
//

// FORWARD REFERENCES
//

// *********************************************************************

class ScanLine
{
private:

protected:

	LineHeader m_lineheader;
	std::vector<unsigned char> m_linepixel;

public:

	// LIFECYCLE

	// OPERATORS

	int operator[](const unsigned index) const;
	int operator[](const unsigned index);

	friend std::ostream& operator<<(std::ostream& os,
					const ScanLine& scanline);
	friend std::istream& operator>>(std::istream& is,
					ScanLine& scanline);

	// OPERATIONS

	std::ostream& debug(std::ostream& os) const;

	// INQUIRY

	// ACCESS

	// get
	const LineHeader& lineheader() const;
	LineHeader& lineheader();
	const std::vector<unsigned char>& linepixel() const;
	std::vector<unsigned char>& linepixel();

	// set
	void lineheader(const LineHeader& lineheader);
	void linepixel(const std::vector<unsigned char>& linepixel);
};

// EXTERNAL REFERENCES
//

inline
int
ScanLine::operator[](const unsigned index) const
{
	return m_linepixel[index];
}

inline
int
ScanLine::operator[](const unsigned index)
{
	return m_linepixel[index];
}

inline
const LineHeader&
ScanLine::lineheader() const
{
	return m_lineheader;
}

inline
LineHeader&
ScanLine::lineheader()
{
	return m_lineheader;
}

inline
const std::vector<unsigned char>&
ScanLine::linepixel() const
{
	return m_linepixel;
}

inline
std::vector<unsigned char>&
ScanLine::linepixel()
{
	return m_linepixel;
}

inline
void
ScanLine::lineheader(const LineHeader& lineheader)
{
	m_lineheader = lineheader;
}

inline
void
ScanLine::linepixel(const std::vector<unsigned char>& linepixel)
{
	m_linepixel = linepixel;
}

#endif // DSM_SCANLINE_HH
