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

#ifndef __MSG_NATIVE_LINE__
#define __MSG_NATIVE_LINE__

#include <fstream>
#include <ctype.h>

#include <msat/hrit/MSG_time_cds.h>
#include <msat/msg-native/MSG_native_packetheader.h>

typedef enum {
  LV_NOT_DERIVED             = 0,
  LV_NOMINAL                 = 1,
  LV_BASED_ON_MISSING_DATA   = 2,
  LV_BASED_ON_CORRUPTED_DATA = 3,
  LV_BASED_ON_REPLACED_DATA  = 4
} t_enum_line_validity;

typedef enum {
  LQ_NOT_DERIVED = 0,
  LQ_NOMINAL     = 1,
  LQ_USABLE      = 2,
  LQ_SUSPECT     = 3,
  LQ_DO_NOT_USE  = 4
} t_enum_quality;

class MSG_native_line_validity {
  public:
    unsigned char value;
    friend std::ostream& operator<< ( std::ostream& os,
                                      MSG_native_line_validity &v );
};

class MSG_native_line_radiometric_quality {
  public:
    unsigned char value;
    friend std::ostream& operator<< ( std::ostream& os,
                                      MSG_native_line_radiometric_quality &r );
};

class MSG_native_line_geometric_quality {
  public:
    unsigned char value;
    friend std::ostream& operator<< ( std::ostream& os,
                                      MSG_native_line_geometric_quality &g );
};

class MSG_native_lineheader {
  public:
    const static int lhlen = 27;
    unsigned char LINE1_5Version;
    unsigned short SatelliteId;
    MSG_time_cds_expanded TrueRepeatCycleStart;
    unsigned long LineNumberInGrid;
    unsigned char ChannelId;
    MSG_time_cds_short L10LineMeanAcquisitionTime;
    MSG_native_line_validity LineValidity;
    MSG_native_line_radiometric_quality LineRadiometricQuality;
    MSG_native_line_geometric_quality LineGeometricQuality;
    void read_from(unsigned char *buf);
    friend std::ostream& operator<< ( std::ostream& os,
                                      MSG_native_lineheader &l );
};

class MSG_native_linedata {
  public:
    MSG_native_linedata( );
    ~MSG_native_linedata( );

    void to_sample(unsigned short **samples, long *nsample);

    size_t datasize;
    unsigned char *data_10bit;
};

class MSG_native_line {
  public:
    IMPF_Packet_Header    pkh;
    MSG_native_lineheader header;
    MSG_native_linedata   data;
    void read( std::ifstream &in );
    friend std::ostream& operator<< ( std::ostream& os, MSG_native_line &l );
};

#endif
