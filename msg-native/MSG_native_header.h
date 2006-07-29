
#ifndef __MSG_NATIVE_HEADER_H__
#define __MSG_NATIVE_HEADER_H__

#include <fstream>
#include <iostream>
#include <string>
#include <hrit/MSG_data.h>
#include <msg-native/MSG_native_packetheader.h>

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
