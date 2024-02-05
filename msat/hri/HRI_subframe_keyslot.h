//-----------------------------------------------------------------------------
//
//  Copyright (C) 2004--2006  Graziano Giuliani <giuliani@lamma.rete.toscana.it>
//  Copyright (C) 2006--2012  ARPAE-SIMC <urpsim@arpae.it>
//
//  File        : HRI_subframe_keyslot.h
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
#ifndef __HRI_SUBFRAME_KEYSLOT_H__
#define __HRI_SUBFRAME_KEYSLOT_H__

#include <msat/hri/HRI_machine.h>
#include <iostream>

typedef struct {
  bool usage;
  unsigned short int key_number;
  unsigned short int station_number;
  unsigned char message_key[8];
} HRI_keyslot;

class HRI_subframe_keyslot {
  public:
    HRI_subframe_keyslot( ) { }
    HRI_subframe_keyslot( char buf[1440] );
    ~HRI_subframe_keyslot( ) { }
    void readfrom( char buf[1440] );
    bool has_keys( short int station_number );
    HRI_keyslot &get_key( short int station_number, short int keynum );
    // Overloaded >> operator
    friend std::ostream& operator<< ( std::ostream& os,
                                      HRI_subframe_keyslot& k )
    {
      os << "---------------------------------" << std::endl
	 << "-             HRI Keys          -" << std::endl
	 << "---------------------------------" << std::endl;
      for (int i = 0; i < 120; i++)
      {
	if (k.keys[i].usage)
	{
	  os << "Station number      : " << k.keys[i].station_number
	     << std::endl
	     << "Key number          : " << k.keys[i].key_number
	     << std::endl
	     << "KEY                 : " << std::hex
	     << (short) k.keys[i].message_key[0]
	     << (short) k.keys[i].message_key[1]
	     << (short) k.keys[i].message_key[2]
	     << (short) k.keys[i].message_key[3]
	     << (short) k.keys[i].message_key[4]
	     << (short) k.keys[i].message_key[5]
	     << (short) k.keys[i].message_key[6]
	     << (short) k.keys[i].message_key[7]
	     << std::endl << std::dec;
	}
      }
      return os;
    }
  private:
    HRI_keyslot keys[120];
};

#endif
