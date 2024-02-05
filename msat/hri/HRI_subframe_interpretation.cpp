//-----------------------------------------------------------------------------
//
//  Copyright (C) 2004--2006  Graziano Giuliani <giuliani@lamma.rete.toscana.it>
//  Copyright (C) 2006--2012  ARPAE-SIMC <urpsim@arpae.it>
//
//  File        : HRI_subframe_interpretation.cpp
//  Description : Meteosat HRI format interface
//  Project     : Meteosatlib
//  Authors     : Graziano Giuliani (Lamma Regione Toscana)
//              : Enrico Zini <enrico@enricozini.com>
//  References  : Meteosat High Resolution Image Dissemination
//                Doc. No. EUM TD 02 Revision 4 April 1998
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
#include <msat/hri/HRI_subframe_interpretation.h>
#include <cstring>

HRI_subframe_interpretation::HRI_subframe_interpretation( char *hsi )
{
  readfrom(hsi);
}

void HRI_subframe_interpretation::readfrom( char *hsi )
{
  memset(&cal,  0, sizeof(HRI_calibration_data));
  memset(&oper, 0, sizeof(HRI_spacecraft_operation_data));
  memset(&imag, 0, sizeof(HRI_imagery_data));
  memset(&mesg, 0, sizeof(HRI_administrative_message));

  cal.bbc1ir     = conv.r4_from_char6(hsi);
  cal.bbsd1i     = conv.r4_from_char3(hsi+6);
  cal.bbc1wv     = conv.r4_from_char6(hsi+9);
  cal.bbsd1w     = conv.r4_from_char3(hsi+15);
  cal.bb1t       = conv.r4_from_char5(hsi+18);
  cal.bb2t       = conv.r4_from_char5(hsi+23);
  cal.bbc2ir     = conv.r4_from_char6(hsi+28);
  cal.bbsd2i     = conv.r4_from_char3(hsi+34);
  cal.bbc2wv     = conv.r4_from_char6(hsi+37);
  cal.bbsd2w     = conv.r4_from_char3(hsi+43);
  cal.time1_day  = conv.i2_from_char3(hsi+46);
  cal.time1_slot = conv.i2_from_char2(hsi+46+3);
  cal.calir      = conv.r4f_from_char5(hsi+51);
  cal.irspc      = conv.r4f_from_char3(hsi+56);
  cal.time2_day  = conv.i2_from_char3(hsi+59);
  cal.time2_slot = conv.i2_from_char2(hsi+59+3);
  cal.calwv      = conv.r4f_from_char5(hsi+64);
  cal.wvspc      = conv.r4f_from_char3(hsi+69);
  cal.time3_day  = conv.i2_from_char3(hsi+72);
  cal.time3_slot = conv.i2_from_char2(hsi+72+3);
  cal.ir_gain    = conv.i2_from_char2(hsi+92);
  cal.wv_gain    = conv.i2_from_char2(hsi+92+2);
  cal.v1_gain    = conv.i2_from_char2(hsi+92+4);
  cal.v2_gain    = conv.i2_from_char2(hsi+92+6);
 
  oper.degsra   = conv.r8_from_buff((const unsigned char *) hsi+104);
  oper.degsde   = conv.r8_from_buff((const unsigned char *) hsi+112);
  oper.degnra   = conv.r8_from_buff((const unsigned char *) hsi+120);
  oper.degnde   = conv.r8_from_buff((const unsigned char *) hsi+128);
  oper.finatt_x = conv.r8_from_buff((const unsigned char *) hsi+136);
  oper.finatt_y = conv.r8_from_buff((const unsigned char *) hsi+136+8);
  oper.finatt_z = conv.r8_from_buff((const unsigned char *) hsi+136+16);
  oper.farade_1 = conv.r8_from_buff((const unsigned char *) hsi+160);
  oper.farade_2 = conv.r8_from_buff((const unsigned char *) hsi+160+8);
  oper.nrslot   = conv.i4_from_buff((const unsigned char *) hsi+176);
  oper.spndur   = conv.r4_from_buff((const unsigned char *) hsi+180);
  oper.flecl    = (*(hsi+184) ? true : false);
  oper.fldec    = (*(hsi+185) ? true : false);
  oper.flman    = (*(hsi+186) ? true : false);
  oper.flmode   = (*(hsi+187) ? true : false);
  oper.flir1    = (*(hsi+188) ? true : false);
  oper.flir2    = (*(hsi+189) ? true : false);
  oper.flwv1    = (*(hsi+190) ? true : false);
  oper.flwv2    = (*(hsi+191) ? true : false);
  oper.flvis1   = (*(hsi+192) ? true : false);
  oper.flvis2   = (*(hsi+193) ? true : false);
  oper.flvis3   = (*(hsi+194) ? true : false);
  oper.flvis4   = (*(hsi+195) ? true : false);

  imag.imstat_1  = (*(hsi+232) ? true : false);
  imag.imstat_2  = (*(hsi+233) ? true : false);
  imag.imstat_3  = (*(hsi+234) ? true : false);
  imag.imstat_4  = (*(hsi+235) ? true : false);
  imag.imstat_5  = (*(hsi+236) ? true : false);
  imag.imstat_6  = (*(hsi+237) ? true : false);
  imag.imstat_7  = (*(hsi+238) ? true : false);
  imag.imstat_8  = (*(hsi+239) ? true : false);
  imag.imstat_9  = (*(hsi+240) ? true : false);
  imag.imstat_10 = (*(hsi+241) ? true : false);
  imag.imstat_11 = (*(hsi+242) ? true : false);
  imag.limhor_1  = conv.i2_from_buff((const unsigned char *) hsi+248);
  imag.limhor_2  = conv.i2_from_buff((const unsigned char *) hsi+248+2);
  imag.limhor_3  = conv.i2_from_buff((const unsigned char *) hsi+248+4);
  imag.limhor_4  = conv.i2_from_buff((const unsigned char *) hsi+248+6);
  imag.limhor_5  = conv.i2_from_buff((const unsigned char *) hsi+248+8);
  imag.limhor_6  = conv.i2_from_buff((const unsigned char *) hsi+248+10);
  imag.limhor_7  = conv.i2_from_buff((const unsigned char *) hsi+248+12);
  imag.limhor_8  = conv.i2_from_buff((const unsigned char *) hsi+248+14);
  imag.limhor_9  = conv.i2_from_buff((const unsigned char *) hsi+248+16);
  imag.limhor_10 = conv.i2_from_buff((const unsigned char *) hsi+248+18);
  imag.limhor_11 = conv.i2_from_buff((const unsigned char *) hsi+248+20);
  imag.limhor_12 = conv.i2_from_buff((const unsigned char *) hsi+248+22);
  imag.satdis    = conv.r8_from_buff((const unsigned char *) hsi+272);
  imag.sorbof_x  = conv.r8_from_buff((const unsigned char *) hsi+280);
  imag.sorbof_y  = conv.r8_from_buff((const unsigned char *) hsi+288);
  imag.sorbof_z  = conv.r8_from_buff((const unsigned char *) hsi+296);
  imag.norbof_x  = conv.r8_from_buff((const unsigned char *) hsi+304);
  imag.norbof_y  = conv.r8_from_buff((const unsigned char *) hsi+312);
  imag.norbof_z  = conv.r8_from_buff((const unsigned char *) hsi+320);
  imag.xddifm    = conv.r4_from_buff((const unsigned char *) hsi+328);
  imag.yddifm    = conv.r4_from_buff((const unsigned char *) hsi+332);
  imag.xscm      = conv.r4_from_buff((const unsigned char *) hsi+336);
  imag.yscm      = conv.r4_from_buff((const unsigned char *) hsi+340);
  imag.conds_1   = (*(hsi+344) ? true : false);
  imag.conds_2   = (*(hsi+345) ? true : false);
  imag.conds_3   = (*(hsi+346) ? true : false);
  imag.conds_4   = (*(hsi+347) ? true : false);
  imag.lowdyn_ir = conv.i2_from_buff((const unsigned char *) hsi+348);
  imag.lowdyn_v1 = conv.i2_from_buff((const unsigned char *) hsi+350);
  imag.lowdyn_v2 = conv.i2_from_buff((const unsigned char *) hsi+352);
  imag.lowdyn_wv = conv.i2_from_buff((const unsigned char *) hsi+354);
  imag.higdyn_ir = conv.i2_from_buff((const unsigned char *) hsi+356);
  imag.higdyn_v1 = conv.i2_from_buff((const unsigned char *) hsi+358);
  imag.higdyn_v2 = conv.i2_from_buff((const unsigned char *) hsi+360);
  imag.higdyn_wv = conv.i2_from_buff((const unsigned char *) hsi+362);
  imag.mvis1     = conv.r4_from_buff((const unsigned char *) hsi+364);
  imag.mvis2     = conv.r4_from_buff((const unsigned char *) hsi+368);
  imag.snnom_ir  = conv.r4_from_buff((const unsigned char *) hsi+372);
  imag.snnom_v1  = conv.r4_from_buff((const unsigned char *) hsi+376);
  imag.snnom_v2  = conv.r4_from_buff((const unsigned char *) hsi+380);
  imag.snnom_wv  = conv.r4_from_buff((const unsigned char *) hsi+384);
  imag.snnlin    = conv.i4_from_buff((const unsigned char *) hsi+388);
  imag.snrep_ir  = conv.r4_from_buff((const unsigned char *) hsi+392);
  imag.snrep_v1  = conv.r4_from_buff((const unsigned char *) hsi+396);
  imag.snrep_v2  = conv.r4_from_buff((const unsigned char *) hsi+400);
  imag.snrep_wv  = conv.r4_from_buff((const unsigned char *) hsi+404);
  imag.snrwp_ir  = conv.r4_from_buff((const unsigned char *) hsi+408);
  imag.snrwp_v1  = conv.r4_from_buff((const unsigned char *) hsi+412);
  imag.snrwp_v2  = conv.r4_from_buff((const unsigned char *) hsi+416);
  imag.snrwp_wv  = conv.r4_from_buff((const unsigned char *) hsi+420);
  imag.swmnep_ir = conv.r4_from_buff((const unsigned char *) hsi+424);
  imag.swmnep_v1 = conv.r4_from_buff((const unsigned char *) hsi+428);
  imag.swmnep_v2 = conv.r4_from_buff((const unsigned char *) hsi+432);
  imag.swmnep_wv = conv.r4_from_buff((const unsigned char *) hsi+436);
  imag.swmnwp_ir = conv.r4_from_buff((const unsigned char *) hsi+440);
  imag.swmnwp_v1 = conv.r4_from_buff((const unsigned char *) hsi+444);
  imag.swmnwp_v2 = conv.r4_from_buff((const unsigned char *) hsi+448);
  imag.swmnwp_wv = conv.r4_from_buff((const unsigned char *) hsi+452);
  imag.snmxep_ir = conv.i2_from_buff((const unsigned char *) hsi+456);
  imag.snmxep_v1 = conv.i2_from_buff((const unsigned char *) hsi+458);
  imag.snmxep_v2 = conv.i2_from_buff((const unsigned char *) hsi+460);
  imag.snmxep_wv = conv.i2_from_buff((const unsigned char *) hsi+462);
  imag.snmxwp_ir = conv.i2_from_buff((const unsigned char *) hsi+466);
  imag.snmxwp_v1 = conv.i2_from_buff((const unsigned char *) hsi+466);
  imag.snmxwp_v2 = conv.i2_from_buff((const unsigned char *) hsi+468);
  imag.snmxwp_wv = conv.i2_from_buff((const unsigned char *) hsi+470);

  memcpy(mesg.admin_message, hsi+560, 800);
  return;
}
