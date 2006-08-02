// $Id$
//
// NAME:
//   LineHeader -
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

// SYSTEM INCLUDES
//
#include <cstring>
#include <iostream>

// PROJECT INCLUDES
//

// LOCAL INCLUDES
//
#include "LineHeader.hh" // class implemented
#include "ByteSex.hh"

// LOCAL CONSTANTS
//

using namespace omtp_ids;

// **************************** PRIVATE    *****************************

// **************************** PROTECTED  *****************************

// **************************** PUBLIC     *****************************

// ============================ LIFECYCLE  =============================

LineHeader::LineHeader()
 : m_length(0),
   m_line(0),
   m_start(0),
   m_no_pixels(0),
   m_channel_id(0),
   m_quality(0)
{
	for (int i = 0; i < PAD; ++i)
		m_pad[i] = 0;
}

// ============================ OPERATORS  =============================

std::ostream&
operator<<(std::ostream& os,
	   const LineHeader& lineheader)
{
	ByteSex::big::write2(os, lineheader.m_length    );
	ByteSex::big::write2(os, lineheader.m_line      );
	ByteSex::big::write2(os, lineheader.m_start     );
	ByteSex::big::write2(os, lineheader.m_no_pixels );
	ByteSex::big::write2(os, lineheader.m_channel_id);
	ByteSex::big::write2(os, lineheader.m_quality   );

	for (int i = 0; i < PAD; ++i)
		ByteSex::big::write2(os, lineheader.m_pad[i]);

	return os;
}

std::istream&
operator>>(std::istream& is,
	   LineHeader& lineheader)
{
	lineheader = LineHeader();

	lineheader.m_length               = ByteSex::big::read2(is);
	lineheader.m_line                 = ByteSex::big::read2(is);
	lineheader.m_start                = ByteSex::big::read2(is);
	lineheader.m_no_pixels            = ByteSex::big::read2(is);
	lineheader.m_channel_id           = ByteSex::big::read2(is);
	lineheader.m_quality              = ByteSex::big::read2(is);

	for (int i = 0; i < PAD; ++i)
		lineheader.m_pad[i] = ByteSex::big::read2(is);

	return is;
}

// ============================ OPERATIONS =============================

std::ostream&
LineHeader::debug(std::ostream& os) const
{
	os << "LineHeader :" << std::endl;
	os << "  length               : " << m_length     << std::endl;
	os << "  line                 : " << m_line       << std::endl;
	os << "  start                : " << m_start      << std::endl;
	os << "  no_pixels            : " << m_no_pixels  << std::endl;
	os << "  channel_id           : " << m_channel_id << std::endl;
	os << "  quality              : " << m_quality    << std::endl;
	os << "  pad                  : ";

	for (int i = 0; i < PAD; ++i) {
		os << m_pad[i];
		if (i != PAD - 1)
			os << ", ";
	}
	os << std::endl;

	return os;
}

// ============================ INQUIRY    =============================

// ============================ ACCESS     =============================

void
LineHeader::pad(const int* pad)
{
	for (int i = 0; i < PAD; ++i)
		m_pad[i] = pad[i];
}
