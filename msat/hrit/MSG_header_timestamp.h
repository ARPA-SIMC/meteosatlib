//-----------------------------------------------------------------------------
//
//  Copyright (C) 2004--2006  Graziano Giuliani <giuliani@lamma.rete.toscana.it>
//  Copyright (C) 2006--2012  ARPAE-SIMC <urpsim@arpae.it>
//
//  File        : MSG_header_timestamp.h
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

#ifndef __MSG_HEADER_TIMESTAMP_H__
#define __MSG_HEADER_TIMESTAMP_H__

#include <iostream>

#include <msat/hrit/MSG_machine.h>
#include <msat/hrit/MSG_time_cds.h>

class MSG_header_timestamp: public MSG_time_cds_short {
  public:
    MSG_header_timestamp( );
    MSG_header_timestamp(unsigned const char_1 *buff );
    ~MSG_header_timestamp( );

    void read_from( unsigned const char_1 *buff );

    //Overloaded << operator
    friend std::ostream& operator<< ( std::ostream& os,
                                      const MSG_header_timestamp &h );

};

#endif
