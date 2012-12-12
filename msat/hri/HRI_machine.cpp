//-----------------------------------------------------------------------------
//
//  Copyright (C) 2004--2006  Graziano Giuliani <giuliani@lamma.rete.toscana.it>
//  Copyright (C) 2006--2012  ARPA-SIM <urpsim@smr.arpa.emr.it>
//
//  File        : HRI_machine.cpp
//  Description : Meteosat HRI format interface
//  Project     : Meteosatlib
//  Authors     : Graziano Giuliani (Lamma Regione Toscana)
//              : Enrico Zini <enrico@enricozini.com>
//  References  : Meteosat High Resolution Image Dissemination
//                Doc. No. EUM TD 02 Revision 4 April 1998
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
//-----------------------------------------------------------------------------
#include <msat/hri/HRI_machine.h>

HRI_machine::HRI_machine()
{
  union {
    unsigned char byte[4];
    int val;
  } word;
  word.val = 0;
  word.byte[3] = 0x1;
  if (word.val != 1)
    isbig = false;
  else
    isbig = true;
}

HRI_machine::~HRI_machine() { }

float HRI_machine::r4_from_char6( const char *buff )
{
  float tmp = 0;
  if (buff[0] != 32)
	tmp += (buff[0] - 48 ) * 100;
  if (buff[1] != 32)
    tmp += (buff[1] - 48 ) * 10;
  if (buff[2] != 32)
    tmp += buff[2] - 48;
  if (buff[3] != 32)
    tmp += (buff[3] - 48) * 0.1;
  if (buff[4] != 32)
    tmp += (buff[4] - 48) * 0.01;
  if (buff[5] != 32)
    tmp += (buff[5] - 48) * 0.001;
  return tmp;
}

float HRI_machine::r4_from_char3( const char *buff )
{
  float tmp = 0;
  if (buff[0] != 32)
	tmp += buff[0] - 48 ;
  if (buff[1] != 32)
    tmp += (buff[1] - 48) * 0.1;
  if (buff[2] != 32)
    tmp += (buff[2] - 48) * 0.01;
  return tmp;
}

float HRI_machine::r4f_from_char3( const char *buff )
{
  float tmp = 0;
  if (buff[0] != 32)
	tmp += (buff[0] - 48) * 10;
  if (buff[1] != 32)
    tmp += (buff[1] - 48);
  if (buff[2] != 32)
    tmp += (buff[2] - 48) * 0.1;
  return tmp;
}

float HRI_machine::r4_from_char5( const char *buff )
{
  float tmp = 0;
  if (buff[0] != 32)
	tmp += (buff[0] - 48 ) * 100;
  if (buff[1] != 32)
    tmp += (buff[1] - 48 ) * 10;
  if (buff[2] != 32)
    tmp += buff[2] - 48;
  if (buff[3] != 32)
    tmp += (buff[3] - 48) * 0.1;
  if (buff[4] != 32)
    tmp += (buff[4] - 48) * 0.01;
  return tmp;
}

short int HRI_machine::i2_from_char3( const char *buff )
{
  short int tmp = 0;
  if (buff[0] != 32)
	tmp += (buff[0] - 48) * 100;
  if (buff[1] != 32)
    tmp += (buff[1] - 48) * 10;
  if (buff[2] != 32)
    tmp += (buff[2] - 48);
  return tmp;
}

short int HRI_machine::i2_from_char2( const char *buff )
{
  short int tmp = 0;
  if (buff[0] != 32)
    tmp += (buff[0] - 48) * 10;
  if (buff[1] != 32)
    tmp += (buff[1] - 48);
  return tmp;
}

float HRI_machine::r4f_from_char5( const char *buff )
{
  float tmp = 0;
  if (buff[0] != 32)
	tmp += (buff[0] - 48) * 0.1;
  if (buff[1] != 32)
    tmp += (buff[1] - 48) * 0.01;
  if (buff[2] != 32)
    tmp += (buff[2] - 48) * 0.001;
  if (buff[3] != 32)
    tmp += (buff[3] - 48) * 0.0001;
  if (buff[4] != 32)
    tmp += (buff[4] - 48) * 0.00001;
  return tmp;
}

static inline void leftshift(unsigned long *pnt, int n)
{
   register unsigned long hiw, low;

   hiw = pnt[0];
   low = pnt[1];

   while (n-- > 0)
   {
      hiw = (hiw << 1) | ((low >> 31) & 0x00000001);
      low <<= 1;
   }

   pnt[0] = hiw;
   pnt[1] = low;
   return;
}

static inline void rightshift(unsigned long *pnt, int n)
{
   register unsigned long hiw, low;

   hiw = pnt[0];
   low = pnt[1];

   while (n-- > 0)
   {
      low = (hiw << 31) | ((low >> 1) & 0x7FFFFFFF);
      hiw >>= 1;
   }

   pnt[0] = hiw;
   pnt[1] = low;
   return;
}

float HRI_machine::r4_from_buff( const unsigned char *buff )
{
   unsigned long uval;
   unsigned long *pnt, sign, fraction;
   long exponent;
   unsigned char swp[4];

   if (isbig)
     pnt = (unsigned long *) buff;
   else
   {
     swp[3] = buff[0];
     swp[2] = buff[1];
     swp[1] = buff[2];
     swp[0] = buff[3];
     pnt = (unsigned long *) swp;
   }

   if ((pnt[0] & 0x7FFFFFFF) == 0)
      return *((float *) pnt);

   sign     =   pnt[0] & 0x80000000;
   exponent = ((pnt[0] & 0x7F000000) >> 24) - 64;
   fraction =   pnt[0] & 0x00FFFFFF;

   if (exponent >= 0)
      exponent <<= 2;
   else
      exponent = -((-exponent) << 2);

   exponent = exponent - 1;
   if (fraction)
   {
      while ((fraction & 0x00800000) == 0)
      {
         fraction <<= 1;
         exponent = exponent - 1;
      }
   }

   fraction &= 0x007FFFFF;
   if ((exponent += 127) >= 255)
      uval = sign | 0x7F7FFFFF;
   else if (exponent <= 0)
      uval = sign;
   else
      uval = sign | (exponent << 23) | fraction;

   return *((float *) (&uval));
}

double HRI_machine::r8_from_buff( const unsigned char *buff )
{
   int idxh, idxl;
   unsigned long tmp, sign, fraction[2], *pnt;
   unsigned long uval[2];
   long exponent;
   unsigned char swp[8];

   if (isbig)
     pnt = (unsigned long *) buff;
   else
   {
     swp[3] = buff[0];
     swp[2] = buff[1];
     swp[1] = buff[2];
     swp[0] = buff[3];
     swp[7] = buff[4];
     swp[6] = buff[5];
     swp[5] = buff[6];
     swp[4] = buff[7];
     pnt = (unsigned long *) swp;
   }

   if (isbig) { idxl = 0; idxh = 1; }
   else       { idxl = 1; idxh = 0; }

   tmp = pnt[0];
   if ((tmp & 0x7FFFFFFF) == 0)
   {
     uval[idxl] = pnt[0];
     uval[idxh] = pnt[1];
     return *((double *) uval);
   }

   sign        = pnt[0] & 0x80000000;
   exponent    = ((pnt[0] & 0x7F000000) >> 24) - 64;
   fraction[0] = pnt[0] & 0x00FFFFFF;
   fraction[1] = pnt[1];

   if (exponent >= 0)
      exponent <<= 2;
   else
      exponent = -((-exponent) << 2);

   if (fraction[0])
   {
      while ((fraction[0] & 0x01000000) == 0)
      {
         leftshift(fraction, 1);
         exponent -= 1;
      }
   }

   rightshift(fraction, 4);

   fraction[0] &= 0x000FFFFF;
   if ((exponent += 1023) >= 2047)
   {
      uval[idxl] = sign | 0X7FEFFFFF;
      uval[idxh] = 0xFFFFFFFF;
   }
   else if (exponent <= 0)
   {
      uval[idxl] = sign;
      uval[idxh] = 0;
   }
   else
   {
      uval[idxl] = sign | (exponent << 20) | fraction[0];
      uval[idxh] = fraction[1];
   }
   return *((double *) uval);
}

short int HRI_machine::i2_from_buff( const unsigned char *buff )
{
  if (isbig) return *((short int *) buff);
  unsigned char sw[2];
  sw[0] = buff[1];
  sw[1] = buff[0];
  return *((int *) sw);
}

int HRI_machine::i4_from_buff( const unsigned char *buff )
{
  if (isbig) return *((int *) buff);
  unsigned char sw[4];
  sw[0] = buff[3];
  sw[1] = buff[2];
  sw[2] = buff[1];
  sw[3] = buff[0];
  return *((int *) sw);
}
