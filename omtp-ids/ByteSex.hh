// $Id$
#ifndef DSM_BYTESEX_HH
#define DSM_BYTESEX_HH

// SYSTEM INCLUDES
//
#include "sysdep.h"
#include <iosfwd>

// PROJECT INCLUDES
//

// LOCAL INCLUDES
//

// FORWARD REFERENCES
//

// *********************************************************************
//
// NAME:
//   ByteSex - Big-Endian / Little-Endian Converter
//
// TYPE:          C++-NAMESPACE
// SYNOPSIS:      Endian data reader.
// DESCRIPTION:
//   Big-Endian means a pointer to a multi-byte datatype will point to
//   the Most Significant Byte (MSB) of the type, as apposed to the
//   Least Significant Byte (LSB) of a Little-Endian type.
//
//   For a 2-byte datatype like a uint16_t:
//
//            MSB LSB
//   bytes:  | 0 | 1 |
//
//   The hexadecimal value of `1' stored in 16 bits is 0x0001.  When
//   examining the MSB you will see 0x00 on a big-endian system, and the
//   LSB will contain a value of 0x01.
//
//   Little-Endian means a pointer to a multi-byte datatype will point
//   to the Least Significant Byte (LSB) of the type, as opposed to the
//   Most Significant Byte (MSB) of a Big-Endian type.
//
// EXAMPLES:
// FILES:         ByteSex.hh, ByteSex.cc
// SEE ALSO:
//
// PROJECT:       Meteosatlib
//
// AUTHOR:        Deneys Maartens
// VERSION:       $Rev$
// DATE:          $Date: 2004/07/16 11:05:26 $
// COPYRIGHT:     Deneys Maartens (C) 2004
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

		static std::ostream& write2(std::ostream& os, uint16_t u,
					    int bytes = 2);
		static std::ostream& write4(std::ostream& os, uint32_t u,
					    int bytes = 4);
#ifdef DSM_64BIT
		static std::ostream& write8(std::ostream& os, uint64_t u,
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

		static std::ostream& write2(std::ostream& os, uint16_t u,
					    int bytes = 2);
		static std::ostream& write4(std::ostream& os, uint32_t u,
					    int bytes = 4);
#ifdef DSM_64BIT
		static std::ostream& write8(std::ostream& os, uint64_t u,
					    int bytes = 8);
#endif // def DSM_64BIT
	};
}

// EXTERNAL REFERENCES
//

#endif // DSM_BYTESEX_HH
