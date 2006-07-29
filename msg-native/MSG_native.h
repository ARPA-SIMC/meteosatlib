
#ifndef __MSG_NATIVE_H__
#define __MSG_NATIVE_H__
 
#include <list>

#include <msg-native/MSG_native_header.h>
#include <msg-native/MSG_native_trailer.h>
#include <msg-native/MSG_native_line.h>

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
