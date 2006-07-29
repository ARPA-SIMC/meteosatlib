#include <msg-native/MSG_native_trailer.h>

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
