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

#ifndef __MSG_NATIVE_H__
#define __MSG_NATIVE_H__
 
#include <list>

#include <msat/msg-native/MSG_native_header.h>
#include <msat/msg-native/MSG_native_trailer.h>
#include <msat/msg-native/MSG_native_line.h>

class MSG_native {
  public:
    MSG_native( );
    ~MSG_native( );

    const static int SEVIRI_CHANNELS = 12;
    const static int VIS_06_CHANNEL  = 0;
    const static int VIS_08_CHANNEL  = 1;
    const static int IR_1_6_CHANNEL  = 2;
    const static int IR_3_9_CHANNEL  = 3;
    const static int IR_6_2_CHANNEL  = 4;
    const static int IR_7_3_CHANNEL  = 5;
    const static int IR_8_7_CHANNEL  = 6;
    const static int IR_9_7_CHANNEL  = 7;
    const static int IR_10_8_CHANNEL = 8;
    const static int IR_12_0_CHANNEL = 9;
    const static int IR_13_4_CHANNEL = 10;
    const static int HRV_CHANNEL     = 11;

    MSG_native_header header;
    MSG_native_trailer trailer;
    std::list <MSG_native_line> line[SEVIRI_CHANNELS];

    bool open( char *name );
    void read( );
    void close( );

    int lines(int channel);
    int pixels(int channel);

    unsigned short *data(int channel);

    bool pgmdump(int channel, char *filename);

    friend std::ostream& operator<< ( std::ostream& os, MSG_native &m );
  private:
    std::ifstream in;
    long headerpos;
    long datapos;
    long trailerpos;
    int nchannels;
    bool selected_channel[SEVIRI_CHANNELS];
    int numberlines[SEVIRI_CHANNELS];
    int startline[SEVIRI_CHANNELS];
    int endline[SEVIRI_CHANNELS];
    int numbercolumns[SEVIRI_CHANNELS];
    int startcolumn[SEVIRI_CHANNELS];
    int endcolumn[SEVIRI_CHANNELS];
};

#endif
