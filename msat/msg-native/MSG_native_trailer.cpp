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

#include <msat/msg-native/MSG_native_trailer.h>

void MSG_native_trailer::read( std::ifstream &in )
{
  unsigned char lbuf[impf_packet_header.pkh_len];
  unsigned char l15buf[l15_len];

  in.read((char *) lbuf, impf_packet_header.pkh_len);
  if (in.fail( ))
  {
    std::cerr << "Read error from Native file: IMPF Header." << std::endl;
    throw;
  }
  impf_packet_header.read_from(lbuf);

  if (l15_len != impf_packet_header.gp_packet_header.PacketLength - 15)
  {
    std::cerr << "Trailer Size: "
              << "Read error from Native file: Level 1.5 Trailer." << std::endl;
    throw;
  }

  in.read((char *) l15buf, l15_len);
  if (in.fail( ))
  {
    std::cerr << "Trailer: "
              << "Read error from Native file: Level 1.5 Trailer." << std::endl;
    throw;
  }
  unsigned char *x = l15buf;
  unsigned char *p = l15buf + 1;
  l15.product_stats.read_from(p);
  p += MSG_IMAGE_PRODUCT_STATS_LEN;
  l15.navig_result.read_from(p);
  p += MSG_NAVIGATION_EXTR_RESULT_LEN;
  l15.radiometric_qlty.read_from(p);
  p += MSG_RADIOMETRIC_QUALITY_LEN;
  l15.geometric_qlty.read_from(p);
  p += MSG_GEOMETRIC_QUALITY_LEN;
  l15.timelin_comple.read_from(p);
  p += MSG_TIMELINESS_COMPLETENESS_LEN;
  if (((unsigned int) (p-x)) != l15_len)
  {
    std::cerr << "Trailer checksum: "
              << "Read error from Native file: Level 1.5 Trailer." << std::endl;
    throw;
  }

  return;
}

std::ostream& operator<< ( std::ostream& os, MSG_native_trailer &t )
{
  os << t.impf_packet_header << t.l15;

  return os;
}
