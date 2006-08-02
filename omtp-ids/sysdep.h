/* $Id$ */
#ifndef DSM_SYSDEP_H
#define DSM_SYSDEP_H

// SYSTEM INCLUDES
//
#include <sys/types.h>

// PROJECT INCLUDES
//

// LOCAL INCLUDES
//

// FORWARD REFERENCES
//

// X***********************X********************X*****************************
//
// NAME:
//   debug -
//
// TYPE:                   C-DECLARATION
// SYNOPSIS:               uintXX_t support
// DESCRIPTION:            This file was created to provide the uintXX_t
//                         definition on systems who don't have it,
//                         notably Linux.  Linux does have u_intXX_t, so
//                         use it.
// EXAMPLES:
// FILES:                  sysdep.h
// SEE ALSO:
//
// AUTHOR:                 Deneys Maartens
// VERSION:                $Revision: 1.1.1.1 $
// DATE:                   $Date: 2004/07/16 11:05:26 $
// COPYRIGHT:              Deneys Maartens (C) 1998-2003
//
// X***********************X********************X*****************************

// EXTERNAL REFERENCES
//

#if defined(HAVE_STDINT_H)
# include <stdint.h>
#elif defined(HAVE_INTTYPES_H)
# include <inttypes.h>
#elif  defined(_WIN32)
#include <win32defs.h>
#else
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
typedef long long int int64_t;
#ifdef DSM_64BIT
typedef u_int64_t uint64_t;
#endif // def DSM_64BIT
#endif // not def __INTTYPES_INCLUDED

#endif // DSM_SYSDEP_H
