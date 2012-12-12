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

#include <msat/hrit/MSG_machine.h>
#include <msat/msg-native/MSG_native_packetheader.h>

void IMPF_Packet_Header::read_from(const unsigned char *buf)
{
  gp_packet_header.HeaderVersionNo  = *buf;
  gp_packet_header.PacketType       = *(buf+1);
  gp_packet_header.SubHeaderType    = *(buf+2);
  gp_packet_header.SourceFacilityId = *(buf+3);
  gp_packet_header.SourceEnvId      = *(buf+4);
  gp_packet_header.SourceInstanceId = *(buf+5);
  gp_packet_header.SourceSUId       = get_ui4((const unsigned char *) buf+6);
  gp_packet_header.SourceCPUId[0]   = *(buf+10);
  gp_packet_header.SourceCPUId[1]   = *(buf+11);
  gp_packet_header.SourceCPUId[2]   = *(buf+12);
  gp_packet_header.SourceCPUId[3]   = *(buf+13);
  gp_packet_header.DestFacilityId   = *(buf+14);
  gp_packet_header.DestEnvId        = *(buf+15);
  gp_packet_header.SequenceCount    = get_ui2((const unsigned char *) buf+16);
  gp_packet_header.PacketLength     = get_ui4((const unsigned char *) buf+18);
  gp_packet_subheader.SubHeaderVersionNo = *(buf+22);
  gp_packet_subheader.ChecksumFlag = *(buf+23);
  gp_packet_subheader.Acknowledgement[0] = *(buf+24);
  gp_packet_subheader.Acknowledgement[1] = *(buf+25);
  gp_packet_subheader.Acknowledgement[2] = *(buf+26);
  gp_packet_subheader.Acknowledgement[3] = *(buf+27);
  gp_packet_subheader.ServiceType = *(buf+28);
  gp_packet_subheader.ServiceSubtype = *(buf+29);
  gp_packet_subheader.PacketTime.read_from((const unsigned char *) buf+30);
  gp_packet_subheader.SpacecraftId = get_ui2((const unsigned char *) buf+36);
  return;
}

std::ostream& operator<< ( std::ostream& os, IMPF_Packet_Header &h )
{
  GP_Packet_Header *ph = &(h.gp_packet_header);
  GP_Packet_subHeader *psh = &(h.gp_packet_subheader);
  os << "------------------------------------------------------" << std::endl
     << "-             MSG NATIVE PACKET HEADER               -" << std::endl
     << "------------------------------------------------------" << std::endl
     << "HeaderVersionNo             : " << (short) ph->HeaderVersionNo
     << std::endl
     << "PacketType                  : " << (short) ph->PacketType << std::endl
     << "SubHeaderType               : " << (short) ph->SubHeaderType
     << std::endl
     << "SourceFacilityId            : " << (short) ph->SourceFacilityId
     << std::endl
     << "SourceEnvId                 : " << (short) ph->SourceEnvId << std::endl
     << "SourceInstanceId            : " << (short) ph->SourceInstanceId
     << std::endl
     << "SourceSUId                  : " << ph->SourceSUId << std::endl
     << "SourceCPUId                 : " << (short) ph->SourceCPUId[0]
     << " " << (short) ph->SourceCPUId[1] << " " << (short) ph->SourceCPUId[2]
     << " " << (short) ph->SourceCPUId[3] << std::endl
     << "DestFacilityId              : " << (short) ph->DestFacilityId
     << std::endl
     << "DestEnvId                   : " << (short) ph->DestEnvId << std::endl
     << "SequenceCount               : " << (short) ph->SequenceCount
     << std::endl
     << "PacketLength                : " << ph->PacketLength << std::endl
     << "------------------------------------------------------" << std::endl
     << "-             MSG NATIVE PACKET SUBHEADER            -" << std::endl
     << "------------------------------------------------------" << std::endl
     << "SubHeaderVersionNo          : " << (short) psh->SubHeaderVersionNo
     << std::endl
     << "ChecksumFlag                : " << (short) psh->ChecksumFlag
     << std::endl
     << "Acknowledgement             : " << (short) psh->Acknowledgement[0]
     << " " << (short) psh->Acknowledgement[1] << " "
     << (short) psh->Acknowledgement[2]
     << " " << (short) psh->Acknowledgement[3] << std::endl
     << "ServiceType                 : " << (short) psh->ServiceType
     << std::endl
     << "ServiceSubtype              : " << (short) psh->ServiceSubtype
     << std::endl
     << "PacketTime                  : " << psh->PacketTime.get_timestring()
     << std::endl
     << "SpacecraftId                : " << psh->SpacecraftId << std::endl;
  return os;
}
