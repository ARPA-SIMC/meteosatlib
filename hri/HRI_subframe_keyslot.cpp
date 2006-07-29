//-----------------------------------------------------------------------------
//
//  File        : HRI_subframe_keyslot.cpp
//  Description : Meteosat HRI format interface
//  Project     : Meteosatlib
//  Author      : Graziano Giuliani (Lamma Regione Toscana)
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
#include <hri/HRI_subframe_keyslot.h>
#include <cstring>

HRI_subframe_keyslot::HRI_subframe_keyslot( char buf[1440] )
{
  readfrom(buf);
}

void HRI_subframe_keyslot::readfrom( char buff[1440] )
{
  memset(keys, 0, 120*sizeof(HRI_keyslot));
  for (int i = 0; i < 120; i ++)
  {
    keys[i].usage = buff[i*12] ? true : false;
    if (keys[i].usage)
    {
      keys[i].key_number     = (unsigned char) buff[i*12+1];
      keys[i].station_number = 16*(unsigned char) buff[i*12+2] +
	                       (unsigned char) buff[i*12+3];
      keys[i].message_key[0] = (unsigned char) buff[i*12+4];
      keys[i].message_key[1] = (unsigned char) buff[i*12+5];
      keys[i].message_key[2] = (unsigned char) buff[i*12+6];
      keys[i].message_key[3] = (unsigned char) buff[i*12+7];
      keys[i].message_key[4] = (unsigned char) buff[i*12+8];
      keys[i].message_key[5] = (unsigned char) buff[i*12+9];
      keys[i].message_key[6] = (unsigned char) buff[i*12+10];
      keys[i].message_key[7] = (unsigned char) buff[i*12+11];
    }
  }
  return;
}

bool HRI_subframe_keyslot::has_keys( short int station_number )
{
  for (int i = 0; i < 120; i ++)
  {
    if (keys[i].usage && keys[i].station_number == station_number)
    return true;
  }
  return false;
}

HRI_keyslot & HRI_subframe_keyslot::get_key(short int station_number,
                                            short int keynum )
{
  for (int i = 0; i < 120; i ++)
  {
    if (keys[i].usage && keys[i].station_number == station_number &&
	keys[i].key_number == keynum)
      return keys[i];
  }
  static HRI_keyslot none;
  memset(&none, 0, sizeof(HRI_keyslot));
  return none;
}
