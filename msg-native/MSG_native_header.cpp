#include <msg-native/MSG_native_header.h>

void U_MARF_Header::read_from(const unsigned char *buf)
{
  mphinfo[0]   = (char *) buf;
  mphinfo[1]   = (char *) buf+80;
  mphinfo[2]   = (char *) buf+160;
  mphinfo[3]   = (char *) buf+240;
  mphinfo[4]   = (char *) buf+320;
  mphinfo[5]   = (char *) buf+400;
  mphinfo[6]   = (char *) buf+480;
  mphinfo[6]  += (char *) buf+526;
  mphinfo[7]   = (char *) buf+542;
  mphinfo[7]  += (char *) buf+588;
  mphinfo[8]   = (char *) buf+604;
  mphinfo[8]  += (char *) buf+650;
  mphinfo[9]   = (char *) buf+666;
  mphinfo[9]  += (char *) buf+712;
  mphinfo[10]  = (char *) buf+728;
  mphinfo[10] += (char *) buf+774;
  mphinfo[11]  = (char *) buf+2154;
  mphinfo[12]  = (char *) buf+2234;
  mphinfo[13]  = (char *) buf+2314;
  mphinfo[14]  = (char *) buf+2394;
  mphinfo[15]  = (char *) buf+2474;
  mphinfo[16]  = (char *) buf+2554;
  mphinfo[17]  = (char *) buf+2634;
  mphinfo[18]  = (char *) buf+2714;
  mphinfo[19]  = (char *) buf+2794;
  mphinfo[20]  = (char *) buf+2874;
  mphinfo[21]  = (char *) buf+2954;
  mphinfo[22]  = (char *) buf+3034;
  mphinfo[23]  = (char *) buf+3114;
  mphinfo[24]  = (char *) buf+3194;
  mphinfo[25]  = (char *) buf+3274;
  mphinfo[26]  = (char *) buf+3354;
  mphinfo[27]  = (char *) buf+3434;
  mphinfo[28]  = (char *) buf+3514;
  mphinfo[29]  = (char *) buf+3594;
  mphinfo[30]  = (char *) buf+3674;
  mphinfo[31]  = (char *) buf+3754;
  mphinfo[32]  = (char *) buf+3834;
  mphinfo[33]  = (char *) buf+3914;
  mphinfo[34]  = (char *) buf+3994;
  mphinfo[35]  = (char *) buf+4074;
  mphinfo[36]  = (char *) buf+4154;
  mphinfo[37]  = (char *) buf+4234;
  mphinfo[38]  = (char *) buf+4314;
  mphinfo[39]  = (char *) buf+4394;
  mphinfo[40]  = (char *) buf+4474;
  mphinfo[41]  = (char *) buf+4554;
  mphinfo[42]  = (char *) buf+4634;
  mphinfo[43]  = (char *) buf+4714;
  mphinfo[44]  = (char *) buf+4794;
  mphinfo[45]  = (char *) buf+4874;
  mphinfo[46]  = (char *) buf+4954;
  mphinfo[47]  = (char *) buf+5034;
  return;
}

void MSG_native_header::read( std::ifstream &in )
{
  unsigned char ubuf[mph_sph_header.mph_len];
  unsigned char lbuf[impf_packet_header.pkh_len];
  unsigned char l15buf[l15_len];
  in.read((char *) ubuf, mph_sph_header.mph_len);
  if (in.fail( ))
  {
    std::cerr << "Read error from Native file: U-MARF Header." << std::endl;
    throw;
  }
  mph_sph_header.read_from(ubuf);

  in.read((char *) lbuf, impf_packet_header.pkh_len);
  if (in.fail( ))
  {
    std::cerr << "Read error from Native file: IMPF Header." << std::endl;
    throw;
  }
  impf_packet_header.read_from(lbuf);

  if (l15_len != impf_packet_header.gp_packet_header.PacketLength - 15)
  {
    std::cerr << "Read error from Native file: Level 1.5 Header." << std::endl;
    throw;
  }

  in.read((char *) l15buf, l15_len);
  if (in.fail( ))
  {
    std::cerr << "Read error from Native file: Level 1.5 Header." << std::endl;
    throw;
  }
  unsigned char *x = l15buf;
  unsigned char *p = l15buf + 1;
  l15.sat_status.read_from(p);
  p += MSG_SATELLITE_STATUS_LEN;
  l15.image_acquisition.read_from(p);
  p += MSG_IMAGE_ACQUISITION_LEN;
  l15.celestial_events.read_from(p);
  p += MSG_CELESTIAL_EVENTS_LEN;
  l15.image_description.read_from(p);
  p += MSG_IMAGE_DESCRIPTION_LEN;
  l15.radiometric_proc.read_from(p);
  p += MSG_DATA_RADIOMETRIC_PROC_LEN;
  l15.geometric_proc.read_from(p);
  p += MSG_GEOMETRIC_PROCESSING_LEN;
  l15.IMPF_config.read_from(p);
  p += MSG_IMPF_CONFIGURATION_LEN;
  if (((unsigned int) (p-x)) != l15_len)
  {
    std::cerr << "Read error from Native file: Level 1.5 Header." << std::endl;
    throw;
  }

  return;
}

std::ostream& operator<< ( std::ostream& os, MSG_native_header &h )
{
  os << "######################################################" << std::endl
     << "#                                                    #" << std::endl
     << "#            U-MARF HEADER INFORMATIONS              #" << std::endl
     << "#                                                    #" << std::endl
     << "######################################################" << std::endl;
  for (int i = 0; i < (int) h.mph_sph_header.mph_lines; i ++)
    os << h.mph_sph_header.mphinfo[i];
  os << "######################################################" << std::endl
     << "######################################################" << std::endl;


  os << h.impf_packet_header << h.l15;

  return os;
}
