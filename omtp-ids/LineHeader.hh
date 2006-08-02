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
 * Line Header
 */
#ifndef DSM_LINEHEADER_HH
#define DSM_LINEHEADER_HH

// SYSTEM INCLUDES
//
#include <iosfwd>

// PROJECT INCLUDES
//
#include <omtp-ids/Constants.hh>

// LOCAL INCLUDES
//

// FORWARD REFERENCES
//

// *********************************************************************

class LineHeader
{
private:

protected:

	int m_length;
	int m_line;
	int m_start;
	int m_no_pixels;
	int m_channel_id;
	int m_quality;
	int m_pad[omtp_ids::PAD];

public:

	// LIFECYCLE

	LineHeader();

	// OPERATORS

	friend std::ostream& operator<<(std::ostream& os,
					const LineHeader& lineheader);
	friend std::istream& operator>>(std::istream& is,
					LineHeader& lineheader);

	// OPERATIONS

	std::ostream& debug(std::ostream& os) const;

	// INQUIRY

	// ACCESS

	// get
	int length() const;
	int line() const;
	int start() const;
	int no_pixels() const;
	int channel_id() const;
	int quality() const;
	const int* pad() const;

	// set
	void length(const int length);
	void line(const int line);
	void start(const int start);
	void no_pixels(const int no_pixels);
	void channel_id(const int channel_id);
	void quality(const int quality);
	void pad(const int* pad);
};

// EXTERNAL REFERENCES
//

inline
int
LineHeader::length() const
{
	return m_length;
}

inline
int
LineHeader::line() const
{
	return m_line;
}

inline
int
LineHeader::start() const
{
	return m_start;
}

inline
int
LineHeader::no_pixels() const
{
	return m_no_pixels;
}

inline
int
LineHeader::channel_id() const
{
	return m_channel_id;
}

inline
int
LineHeader::quality() const
{
	return m_quality;
}

inline
const int*
LineHeader::pad() const
{
	return m_pad;
}

inline
void
LineHeader::length(const int length)
{
	m_length = length;
}

inline
void
LineHeader::line(const int line)
{
	m_line = line;
}

inline
void
LineHeader::start(const int start)
{
	m_start = start;
}

inline
void
LineHeader::no_pixels(const int no_pixels)
{
	m_no_pixels = no_pixels;
}

inline
void
LineHeader::channel_id(const int channel_id)
{
	m_channel_id = channel_id;
}

inline
void
LineHeader::quality(const int quality)
{
	m_quality = quality;
}

#endif // DSM_LINEHEADER_HH
