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
 * ByteSex - Big-Endian / Little-Endian data reader / writer
 *
 * Big-Endian means a pointer to a multi-byte datatype will point to the
 * Most Significant Byte (MSB) of the type, as apposed to the Least
 * Significant Byte (LSB) of a Little-Endian type.
 *
 * For a 2-byte data type like a uint16_t:
 *
 *            MSB LSB
 *   bytes:  | 0 | 1 |
 *
 * The hexadecimal value of `1' stored in 16 bits is 0x0001.  When
 * examining the MSB you will see 0x00 on a big-endian system, and the
 * LSB will contain a value of 0x01.
 *
 * Little-Endian means a pointer to a multi-byte data type will point to
 * the Least Significant Byte (LSB) of the type, as opposed to the Most
 * Significant Byte (MSB) of a Big-Endian type.
 */
#ifndef DSM_BYTESEX_HH
#define DSM_BYTESEX_HH

// SYSTEM INCLUDES
//
#include <iosfwd>

// PROJECT INCLUDES
//
#include "sysdep.h"

// LOCAL INCLUDES
//

// FORWARD REFERENCES
//

// *********************************************************************

namespace ByteSex
{
	struct big
	{
		static uint16_t read2(std::istream& is, int bytes = 2);
		static uint32_t read4(std::istream& is, int bytes = 4);
#ifdef DSM_64BIT
		static uint64_t read8(std::istream& is, int bytes = 8);
#endif // def DSM_64BIT

		static std::ostream& write2(std::ostream& os,
					    uint16_t u,
					    int bytes = 2);
		static std::ostream& write4(std::ostream& os,
					    uint32_t u,
					    int bytes = 4);
#ifdef DSM_64BIT
		static std::ostream& write8(std::ostream& os,
					    uint64_t u,
					    int bytes = 8);
#endif // def DSM_64BIT
	};

	struct little
	{
		static uint16_t read2(std::istream& is, int bytes = 2);
		static uint32_t read4(std::istream& is, int bytes = 4);
#ifdef DSM_64BIT
		static uint64_t read8(std::istream& is, int bytes = 8);
#endif // def DSM_64BIT

		static std::ostream& write2(std::ostream& os,
					    uint16_t u,
					    int bytes = 2);
		static std::ostream& write4(std::ostream& os,
					    uint32_t u,
					    int bytes = 4);
#ifdef DSM_64BIT
		static std::ostream& write8(std::ostream& os,
					    uint64_t u,
					    int bytes = 8);
#endif // def DSM_64BIT
	};
}

// EXTERNAL REFERENCES
//

#endif // DSM_BYTESEX_HH
