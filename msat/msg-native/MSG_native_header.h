//-----------------------------------------------------------------------------
//
//  Copyright (C) 2004--2006  Graziano Giuliani <giuliani@lamma.rete.toscana.it>
//  Copyright (C) 2006--2012  ARPAE-SIMC <urpsim@arpae.it>
//
//  Authors     : Graziano Giuliani (Lamma Regione Toscana)
//              : Enrico Zini <enrico@enricozini.com>
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

#ifndef __MSG_NATIVE_HEADER_H__
#define __MSG_NATIVE_HEADER_H__

#include <fstream>
#include <iostream>
#include <string>
#include <msat/hrit/MSG_data.h>
#include <msat/msg-native/MSG_native_packetheader.h>

class U_MARF_Header {
  public:
    const static unsigned int mph_len = 5114;
    const static unsigned int mph_lines = 48;
    void read_from(const unsigned char *buf);
    std::string mphinfo[mph_lines];
};

class MSG_native_header {
  public:
    const static unsigned int l15_len = 445248;
    U_MARF_Header      mph_sph_header;
    IMPF_Packet_Header impf_packet_header;
    MSG_data_level_15_header l15;

    void read( std::ifstream &in );

    friend std::ostream& operator<< ( std::ostream& os, MSG_native_header &h );
};

#endif
