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
#include <fstream>

// PROJECT INCLUDES
//

// LOCAL INCLUDES
//
#include "ByteSex.hh" // class implemented

// FORWARD REFERENCES
//

// *********************************************************************

// **************************** PRIVATE    *****************************

// **************************** PROTECTED  *****************************

// **************************** PUBLIC     *****************************

// read 2 big-endian bytes (16 bits)
//
uint16_t
ByteSex::big::read2(std::istream& is,
		    const int bytes)
{
	uint16_t u16 = 0;

	switch (bytes) {
	default:
	case 4: u16 |= static_cast<uint16_t>(is.get()) << 0x18;
	case 3: u16 |= static_cast<uint16_t>(is.get()) << 0x10;
	case 2: u16 |= static_cast<uint16_t>(is.get()) << 0x08;
	case 1: u16 |=                       is.get()         ;
	case 0:                                               ;
	}
	return u16;
}

// read 4 big-endian bytes (32 bits)
//
uint32_t
ByteSex::big::read4(std::istream& is,
		    const int bytes)
{
	uint32_t u32 = 0;

	switch (bytes) {
	default:
	case 4: u32 |= static_cast<uint32_t>(is.get()) << 0x18;
	case 3: u32 |= static_cast<uint32_t>(is.get()) << 0x10;
	case 2: u32 |= static_cast<uint32_t>(is.get()) << 0x08;
	case 1: u32 |=                       is.get()         ;
	case 0:                                               ;
	}
	return u32;
}

#ifdef DSM_64BIT
// read 8 big-endian bytes (64 bits)
//
uint64_t
ByteSex::big::read8(std::istream& is,
		    const int bytes)
{
	uint64_t u64 = 0;

	switch (bytes) {
	default:
	case 8: u64 |= static_cast<uint64_t>(is.get()) << 0x38;
	case 7: u64 |= static_cast<uint64_t>(is.get()) << 0x30;
	case 6: u64 |= static_cast<uint64_t>(is.get()) << 0x28;
	case 5: u64 |= static_cast<uint64_t>(is.get()) << 0x20;
	case 4: u64 |= static_cast<uint64_t>(is.get()) << 0x18;
	case 3: u64 |= static_cast<uint64_t>(is.get()) << 0x10;
	case 2: u64 |= static_cast<uint64_t>(is.get()) << 0x08;
	case 1: u64 |=                       is.get()         ;
	case 0:                                               ;
	}
	return u64;
}
#endif // def DSM_64BIT

// write 0-2 of 2 bytes (16 bits) as big-endian
//
std::ostream&
ByteSex::big::write2(std::ostream& os,
		     const uint16_t u16,
		     const int bytes)
{
	switch (bytes) {
	default:
	case 2: os.put((u16 >> 0x08) & 0xFF);
	case 1: os.put( u16          & 0xFF);
	case 0:                             ;
	}
	return os;
}

// write 0-4 of 4 bytes (32 bits) as big-endian
//
std::ostream&
ByteSex::big::write4(std::ostream& os,
		     const uint32_t u32,
		     const int bytes)
{
	switch (bytes) {
	default:
	case 4: os.put((u32 >> 0x18) & 0xFF);
	case 3: os.put((u32 >> 0x10) & 0xFF);
	case 2: os.put((u32 >> 0x08) & 0xFF);
	case 1: os.put( u32          & 0xFF);
	case 0:                             ;
	}
	return os;
}

#ifdef DSM_64BIT
// write 0-8 of 8 bytes (64 bits) as big-endian
//
std::ostream&
ByteSex::big::write8(std::ostream& os,
		     const uint64_t u64,
		     const int bytes)
{
	switch (bytes) {
	default:
	case 8: os.put((u64 >> 0x38) & 0xFF);
	case 7: os.put((u64 >> 0x30) & 0xFF);
	case 6: os.put((u64 >> 0x28) & 0xFF);
	case 5: os.put((u64 >> 0x20) & 0xFF);
	case 4: os.put((u64 >> 0x18) & 0xFF);
	case 3: os.put((u64 >> 0x10) & 0xFF);
	case 2: os.put((u64 >> 0x08) & 0xFF);
	case 1: os.put( u64          & 0xFF);
	case 0:                             ;
	}
	return os;
}
#endif // def DSM_64BIT

// read 2 little-endian bytes (16 bits)
//
uint16_t
ByteSex::little::read2(std::istream& is,
		       const int bytes)
{
	uint16_t u16 = 0;

	int bits = 0;
	switch (bytes) {
	default:
	case 2: u16 |=                       is.get()         ; bits += 8;
	case 1: u16 |= static_cast<uint16_t>(is.get()) << bits           ;
	case 0:                                                          ;
	}
	return u16;
}

// read 4 little-endian bytes (32 bits)
//
uint32_t
ByteSex::little::read4(std::istream& is,
		       const int bytes)
{
	uint32_t u32 = 0;

	int bits = 0;
	switch (bytes) {
	default:
	case 4: u32 |=                       is.get()         ; bits += 8;
	case 3: u32 |= static_cast<uint32_t>(is.get()) << bits; bits += 8;
	case 2: u32 |= static_cast<uint32_t>(is.get()) << bits; bits += 8;
	case 1: u32 |= static_cast<uint32_t>(is.get()) << bits           ;
	case 0:                                                          ;
	}
	return u32;
}

#ifdef DSM_64BIT
// read 8 little-endian bytes (64 bits)
//
uint64_t
ByteSex::little::read8(std::istream& is,
		       int bytes)
{
	uint64_t u64 = 0;

	int bits = 0;
	switch (bytes) {
	default:
	case 8: u64 |=                       is.get()         ; bits += 8;
	case 7: u64 |= static_cast<uint64_t>(is.get()) << bits; bits += 8;
	case 6: u64 |= static_cast<uint64_t>(is.get()) << bits; bits += 8;
	case 5: u64 |= static_cast<uint64_t>(is.get()) << bits; bits += 8;
	case 4: u64 |= static_cast<uint64_t>(is.get()) << bits; bits += 8;
	case 3: u64 |= static_cast<uint64_t>(is.get()) << bits; bits += 8;
	case 2: u64 |= static_cast<uint64_t>(is.get()) << bits; bits += 8;
	case 1: u64 |= static_cast<uint64_t>(is.get()) << bits           ;
	case 0:                                                          ;
	}
	return u64;
}
#endif // def DSM_64BIT

// write 0-2 of 2 bytes (16 bits) as little-endian
//
std::ostream&
ByteSex::little::write2(std::ostream& os,
			uint16_t u16,
			int bytes)
{
	int bits = 0;
	switch (bytes) {
	default:
	case 2: os.put( u16          & 0xFF); bits += 8;
	case 1: os.put((u16 >> bits) & 0xFF)           ;
	case 0:                                        ;
	}
	return os;
}

// write 0-4 of 4 bytes (32 bits) as little-endian
//
std::ostream&
ByteSex::little::write4(std::ostream& os,
			uint32_t u32,
			int bytes)
{
	int bits = 0;
	switch (bytes) {
	default:
	case 4: os.put( u32          & 0xFF); bits += 8;
	case 3: os.put((u32 >> bits) & 0xFF); bits += 8;
	case 2: os.put((u32 >> bits) & 0xFF); bits += 8;
	case 1: os.put((u32 >> bits) & 0xFF)           ;
	case 0:                                        ;
	}
	return os;
}

#ifdef DSM_64BIT
// write 0-8 of 8 bytes (64 bits) as little-endian
//
std::ostream&
ByteSex::little::write8(std::ostream& os,
			uint64_t u64,
			int bytes)
{
	int bits = 0;
	switch (bytes) {
	default:
	case 8: os.put( u64          & 0xFF); bits += 8;
	case 7: os.put((u64 >> bits) & 0xFF); bits += 8;
	case 6: os.put((u64 >> bits) & 0xFF); bits += 8;
	case 5: os.put((u64 >> bits) & 0xFF); bits += 8;
	case 4: os.put((u64 >> bits) & 0xFF); bits += 8;
	case 3: os.put((u64 >> bits) & 0xFF); bits += 8;
	case 2: os.put((u64 >> bits) & 0xFF); bits += 8;
	case 1: os.put((u64 >> bits) & 0xFF)           ;
	case 0:                                        ;
	}
	return os;
}
#endif // def DSM_64BIT

// ============================ LIFECYCLE  =============================

// ============================ OPERATORS  =============================

// ============================ OPERATIONS =============================

// ============================ INQUIRY    =============================

// ============================ ACCESS     =============================
