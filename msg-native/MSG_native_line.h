
#ifndef __MSG_NATIVE_LINE__
#define __MSG_NATIVE_LINE__

#include <fstream>
#include <ctype.h>

#include <hrit/MSG_time_cds.h>
#include <msg-native/MSG_native_packetheader.h>

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
