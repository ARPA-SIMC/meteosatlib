
#ifndef __MSG_NATIVE_TRAILER_H__
#define __MSG_NATIVE_TRAILER_H__

#include <fstream>
#include <iostream>
#include <hrit/MSG_data.h>
#include <msg-native/MSG_native_packetheader.h>

class MSG_native_trailer {
  public:
    const static unsigned int l15_len = 380325;
    IMPF_Packet_Header impf_packet_header;
    MSG_data_level_15_trailer l15;

    void read( std::ifstream &in );

    friend std::ostream& operator<< ( std::ostream& os, MSG_native_trailer &t );
};

#endif
