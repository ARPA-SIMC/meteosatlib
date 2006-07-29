
#ifndef __MSG_NATIVE_PACKETHEADER_H__
#define __MSG_NATIVE_PACKETHEADER_H__

#include <iostream>
#include <hrit/MSG_time_cds.h>

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
