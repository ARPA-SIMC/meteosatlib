//-----------------------------------------------------------------------------
//
//  Copyright (C) 2004--2006  Graziano Giuliani <giuliani@lamma.rete.toscana.it>
//  Copyright (C) 2006--2012  ARPAE-SIMC <urpsim@arpae.it>
//
//  File        : MSG_header_timestamp.cpp
//  Description : MSG HRIT-LRIT format interface
//  Project     : Meteosatlib
//  Authors     : Graziano Giuliani (Lamma Regione Toscana)
//              : Enrico Zini <enrico@enricozini.com>
//  References  : MSG/SPE/057 LRIT-HRIT Mission Specific Implementation,
//                V. 4.1 9 Mar 2001
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

#include <msat/hrit/MSG_header_timestamp.h>

MSG_header_timestamp::MSG_header_timestamp( ) { }

MSG_header_timestamp::MSG_header_timestamp( unsigned const char_1 *buff )
{
  this->read_from( buff );
}

MSG_header_timestamp::~MSG_header_timestamp( ) { }

void MSG_header_timestamp::read_from( unsigned const char_1 *buff )
{
  (void) MSG_time_cds_short::read_from(buff+4);
}

std::ostream& operator<< ( std::ostream& os, const MSG_header_timestamp &h )
{
  os << "------------------------------------------------------" << std::endl
     << "-                MSG TIMESTAMP HEADER                -" << std::endl
     << "------------------------------------------------------" << std::endl;
  os << *reinterpret_cast<const MSG_time_cds_short *> (&h);
  return os;
}
