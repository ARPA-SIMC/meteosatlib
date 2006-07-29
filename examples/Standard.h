//-----------------------------------------------------------------------------
//
//  File        : Standard.h
//  Description : Common standard definitions
//  Project     : CETEMPS 2003
//  Author      : Graziano Giuliani (CETEMPS - University of L'Aquila
//  References  : LRIT/HRIT GLobal Specification par. 4.4 pag. 20-28
//                Doc. No. CGMS 03 Issue 2.6 12 August 1999
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//---------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#ifndef __STANDARD_H__
#define __STANDARD_H__

// Boolean type if undefined: use a byte

#ifndef BOOL
#define BOOL char
#endif

// Boolean TRUE/FALSE

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

// Degree/radian conversion

#ifndef DTR
#define DTR 0.017453292
#endif

#ifndef RTD
#define RTD 57.29577951
#endif

#ifndef M_1_PI
#define M_1_PI 0.31830988618379067154
#endif

#endif
