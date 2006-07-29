
#ifndef __MSG_HRIT_H__
#define __MSG_HRIT_H__

#include <iostream>
#include <fstream>
#include <hrit/MSG_header.h>
#include <hrit/MSG_data.h>

class MSG_HRIT {
  public:
    bool open( char *name );
    void read( );
    void close( );

    friend std::ostream& operator<< ( std::ostream& os, const MSG_HRIT &hrit );

    MSG_header l15_head;
    MSG_data   l15_data;

  private:
    std::ifstream hrit_ifstream;
};

#endif
