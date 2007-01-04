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
 * System dependent configuration
 *
 * - uintXX_t support
 *   To provide the uintXX_t definition on systems who don't have it.
 */
#ifndef DSM_SYSDEP_H
#define DSM_SYSDEP_H

// SYSTEM INCLUDES
//
#include <sys/types.h>

// PROJECT INCLUDES
//
#include <config.h>

// LOCAL INCLUDES
//

// FORWARD REFERENCES
//

// EXTERNAL REFERENCES
//

#if defined(HAVE_STDINT_H)
# include <stdint.h>
#elif defined(HAVE_INTTYPES_H)
# include <inttypes.h>
#elif defined(_WIN32)
# include <win32defs.h>
#else
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
typedef long long int int64_t;
#ifdef DSM_64BIT
typedef u_int64_t uint64_t;
#endif // DSM_64BIT
#endif

#endif // DSM_SYSDEP_H
