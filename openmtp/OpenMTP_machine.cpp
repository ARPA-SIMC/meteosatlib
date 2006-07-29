//-----------------------------------------------------------------------------
//
//  File        : OpenMTP_machine.cpp
//  Description : Meteosat OpenMTP format interface
//  Project     : Meteosatlib
//  Author      : Graziano Giuliani
//  References  : Eumetsat EUM-FG-1 Format Guide: OpenMTP format
//                Revision 2.1 April 2000
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

#include <openmtp/OpenMTP_machine.h>

OpenMTP_machine::OpenMTP_machine( )
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

OpenMTP_machine::~OpenMTP_machine( ) { }

short OpenMTP_machine::int2( unsigned char *buff )
{
  if (isbig)
    return *((short *) buff);

  char i2[2];
  i2[0] = buff[1];
  i2[1] = buff[0];
  return *((short *) i2);
}

int OpenMTP_machine::int4( unsigned char *buff )
{
  if (isbig)
    return *((int *) buff);

  char i4[4];
  i4[0] = buff[3];
  i4[1] = buff[2];
  i4[2] = buff[1];
  i4[3] = buff[0];
  return *((int *) i4);
}

double OpenMTP_machine::float8( unsigned char *buff )
{
  if (isbig)
    return *((double *) buff);

  char r8[8];
  r8[0] = buff[7];
  r8[1] = buff[6];
  r8[2] = buff[5];
  r8[3] = buff[4];
  r8[4] = buff[3];
  r8[5] = buff[2];
  r8[6] = buff[1];
  r8[7] = buff[0];
  return *((double *) r8);
}

float OpenMTP_machine::float4( unsigned char *buff )
{
  if (isbig)
    return *((float *) buff);

  char r4[4];
  r4[0] = buff[3];
  r4[1] = buff[2];
  r4[2] = buff[1];
  r4[3] = buff[0];
  return *((float *) r4);
}

float OpenMTP_machine::r4_from_char5( unsigned char *buff )
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

float OpenMTP_machine::r4_from_char3( unsigned char *buff )
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

int OpenMTP_machine::int4_from_char3( unsigned char *buff )
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

int OpenMTP_machine::int4_from_char2( unsigned char *buff )
{
  short int tmp = 0;
  if (buff[0] != 32)
    tmp += (buff[0] - 48) * 10;
  if (buff[1] != 32)
    tmp += (buff[1] - 48);
  return tmp;
}
