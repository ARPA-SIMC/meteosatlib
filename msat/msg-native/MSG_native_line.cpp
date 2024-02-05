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

#include <msat/msg-native/MSG_native_line.h>
#include <msat/hrit/MSG_machine.h>

MSG_native_linedata::MSG_native_linedata( )
{
  data_10bit = 0;
  datasize = 0;
}

MSG_native_linedata::~MSG_native_linedata( )
{
  if (data_10bit)
  {
    delete [ ] data_10bit;
    data_10bit = 0;
  }
  datasize = 0;
}

std::ostream& operator<< ( std::ostream& os, MSG_native_line_validity &v )
{
  switch (v.value)
  {
    case LV_NOT_DERIVED:
      os << "Not Derived.";
      break;
    case LV_NOMINAL:
      os << "Nominal.";
      break;
    case LV_BASED_ON_MISSING_DATA:
      os << "Based on missing data.";
      break;
    case LV_BASED_ON_CORRUPTED_DATA:
      os << "Based on corrupted data.";
      break;
    case LV_BASED_ON_REPLACED_DATA:
      os << "Based on replaced or interpolated data.";
      break;
    default:
      os << "Unknown.";
      break;
  }
  return os;
}

std::ostream& operator<< ( std::ostream& os,
                                    MSG_native_line_radiometric_quality &r )
{
  switch (r.value)
  {
    case LQ_NOT_DERIVED:
      os << "Not Derived.";
      break;
    case LQ_NOMINAL:
      os << "Nominal.";
      break;
    case LQ_USABLE:
      os << "Usable.";
      break;
    case LQ_SUSPECT:
      os << "Suspect.";
      break;
    case LQ_DO_NOT_USE:
      os << "Do not use.";
      break;
    default:
      os << "Unknown.";
      break;
  }
  return os;
}

std::ostream& operator<< ( std::ostream& os,
                                    MSG_native_line_geometric_quality &g )
{
  switch (g.value)
  {
    case LQ_NOT_DERIVED:
      os << "Not Derived.";
      break;
    case LQ_NOMINAL:
      os << "Nominal.";
      break;
    case LQ_USABLE:
      os << "Usable.";
      break;
    case LQ_SUSPECT:
      os << "Suspect.";
      break;
    case LQ_DO_NOT_USE:
      os << "Do not use.";
      break;
    default:
      os << "Unknown.";
      break;
  }
  return os;
}

std::ostream& operator<< ( std::ostream& os, MSG_native_lineheader &l )
{
  os << "------------------------------------------------------" << std::endl
     << "-               MSG NATIVE LINE HEADER               -" << std::endl
     << "------------------------------------------------------" << std::endl
     << "LINE1_5Version              : " << (short) l.LINE1_5Version
     << std::endl
     << "SatelliteId                 : " << l.SatelliteId << std::endl
     << "TrueRepeatCycleStart        : "
     << l.TrueRepeatCycleStart.get_timestring( ) << std::endl
     << "LineNumberInGrid            : " << l.LineNumberInGrid << std::endl
     << "ChannelId                   : " << (short) l.ChannelId << std::endl
     << "L10LineMeanAcquisitionTime  : "
     << l.L10LineMeanAcquisitionTime.get_timestring( ) << std::endl
     << "LineValidity                : " << l.LineValidity << std::endl
     << "LineRadiometricQuality      : " << l.LineRadiometricQuality
     << std::endl
     << "LineGeometricQuality        : " << l.LineGeometricQuality << std::endl;
  return os;
}

void MSG_native_lineheader::read_from(unsigned char *buff)
{
  LINE1_5Version = *buff;
  SatelliteId = get_ui2(buff+1);
  TrueRepeatCycleStart.read_from(buff+3);
  LineNumberInGrid = get_ui4(buff+13);
  ChannelId = *(buff+17);
  L10LineMeanAcquisitionTime.read_from(buff+18);
  LineValidity.value = *(buff+24);
  LineRadiometricQuality.value = *(buff+25);
  LineGeometricQuality.value = *(buff+26);
  return;
}

void MSG_native_linedata::to_sample(unsigned short **samples, long *nsample)
{
  const long bitsperpixel = 10;
  const long bitsperbyte = 8;
  *nsample = (datasize * bitsperbyte) / bitsperpixel;
  if (*samples == 0) *samples = new unsigned short[*nsample];

  unsigned short *ps = *samples;
  unsigned char  *pc = data_10bit;
  unsigned int ipos = 0;
  while (ipos < datasize)
  {
    ps[0] = (((unsigned short) pc[0]      ) << 2) |
            (((unsigned short) pc[1]      ) >> 6);
    ps[1] = (((unsigned short) pc[1] &  63) << 4) |
            (((unsigned short) pc[2]      ) >> 4);
    ps[2] = (((unsigned short) pc[2] &  15) << 6) |
            (((unsigned short) pc[3]      ) >> 2);
    ps[3] = (((unsigned short) pc[3] &   3) << 8) |
            (((unsigned short) pc[4]      )     );
    pc   += 5;
    ipos += 5;
    ps   += 4;
  }
  return;
}

void MSG_native_line::read( std::ifstream &in )
{
  unsigned char lbuf[pkh.pkh_len];
  unsigned char lhbuf[header.lhlen];
  in.read((char *) lbuf, pkh.pkh_len);
  if (in.fail( ))
  {
    std::cerr << "Read error from Native file: Packet Header." << std::endl;
    throw;
  }
  pkh.read_from(lbuf);
  data.datasize = pkh.gp_packet_header.PacketLength - 15 - header.lhlen;
  if (data.datasize < 0)
  {
    std::cerr << "Read error from Native file: Packet Size." << std::endl;
    throw;
  }
  data.data_10bit = new unsigned char[data.datasize];
  in.read((char *) lhbuf, header.lhlen);
  if (in.fail( ))
  {
    std::cerr << "Read error from Native file: Line Header." << std::endl;
    throw;
  }
  header.read_from(lhbuf);
  in.read((char *) data.data_10bit, data.datasize);
  if (in.fail( ))
  {
    std::cerr << "Read error from Native file: Line Data." << std::endl;
    throw;
  }
  return;
}

std::ostream& operator<< ( std::ostream& os, MSG_native_line &l )
{
  os << l.pkh << l.header;
  long ns;
  unsigned short *p = 0;
  l.data.to_sample(&p, &ns);
  std::cout << "Got " << ns << " samples from channel "
            << (short) l.header.ChannelId
            << ", line count " << l.header.LineNumberInGrid << std::endl;
  for (int i = 0; i < ns; i ++)
    os << p[i] << std::endl;
  return os;
}
