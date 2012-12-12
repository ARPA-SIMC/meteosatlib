//-----------------------------------------------------------------------------
//
//  Copyright (C) 2004--2006  Graziano Giuliani <giuliani@lamma.rete.toscana.it>
//  Copyright (C) 2006--2012  ARPA-SIM <urpsim@smr.arpa.emr.it>
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

#ifndef __MSG_NATIVE_PACKETHEADER_H__
#define __MSG_NATIVE_PACKETHEADER_H__

#include <iostream>
#include <msat/hrit/MSG_time_cds.h>

class GP_Packet_Header {
  public:
    unsigned char  HeaderVersionNo;
    unsigned char  PacketType;
    unsigned char  SubHeaderType;
    unsigned char  SourceFacilityId;
    unsigned char  SourceEnvId;
    unsigned char  SourceInstanceId;
    unsigned long  SourceSUId;
    unsigned char  SourceCPUId[4];
    unsigned char  DestFacilityId;
    unsigned char  DestEnvId;
    unsigned short SequenceCount;
    unsigned long  PacketLength;
};

class GP_Packet_subHeader {
  public:
    unsigned char SubHeaderVersionNo;
    unsigned char ChecksumFlag;
    unsigned char Acknowledgement[4];
    unsigned char ServiceType;
    unsigned char ServiceSubtype;
    MSG_time_cds_short PacketTime;
    unsigned short SpacecraftId;
};

class IMPF_Packet_Header {
  public:
    void read_from(const unsigned char *buf);
    const static int pkh_len = 38;
    GP_Packet_Header    gp_packet_header;
    GP_Packet_subHeader gp_packet_subheader;

    friend std::ostream& operator<< ( std::ostream& os, IMPF_Packet_Header &h );
};

#endif
