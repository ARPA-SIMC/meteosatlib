//-----------------------------------------------------------------------------
//
//  Copyright (C) 2004--2006  Graziano Giuliani <giuliani@lamma.rete.toscana.it>
//  Copyright (C) 2006--2012  ARPA-SIM <urpsim@smr.arpa.emr.it>
//
//  File        : HRI_subframe_interpretation.h
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
#ifndef __HRI_SUBFRAME_INTERPRETATION_H__
#define __HRI_SUBFRAME_INTERPRETATION_H__

#include <msat/hri/HRI_machine.h>
#include <iostream>

typedef struct {
  float bbc1ir;
  float bbsd1i;
  float bbc1wv;
  float bbsd1w;
  float bb1t;
  float bb2t;
  float bbc2ir;
  float bbsd2i;
  float bbc2wv;
  float bbsd2w;
  short int time1_day;
  short int time1_slot;
  float calir;
  float irspc;
  short int time2_day;
  short int time2_slot;
  float calwv;
  float wvspc;
  short int time3_day;
  short int time3_slot;
  short int ir_gain;
  short int wv_gain;
  short int v1_gain;
  short int v2_gain;
} HRI_calibration_data;

typedef struct {
  double degsra;
  double degsde;
  double degnra;
  double degnde;
  double finatt_x;
  double finatt_y;
  double finatt_z;
  double farade_1;
  double farade_2;
  int nrslot;
  float spndur;
  bool flecl;
  bool fldec;
  bool flman;
  bool flmode;
  bool flir1;
  bool flir2;
  bool flwv1;
  bool flwv2;
  bool flvis1;
  bool flvis2;
  bool flvis3;
  bool flvis4;
} HRI_spacecraft_operation_data;

typedef struct {
  bool imstat_1;
  bool imstat_2;
  bool imstat_3;
  bool imstat_4;
  bool imstat_5;
  bool imstat_6;
  bool imstat_7;
  bool imstat_8;
  bool imstat_9;
  bool imstat_10;
  bool imstat_11;
  short int limhor_1;
  short int limhor_2;
  short int limhor_3;
  short int limhor_4;
  short int limhor_5;
  short int limhor_6;
  short int limhor_7;
  short int limhor_8;
  short int limhor_9;
  short int limhor_10;
  short int limhor_11;
  short int limhor_12;
  double satdis;
  double sorbof_x;
  double sorbof_y;
  double sorbof_z;
  double norbof_x;
  double norbof_y;
  double norbof_z;
  float xddifm;
  float yddifm;
  float xscm;
  float yscm;
  bool conds_1;
  bool conds_2;
  bool conds_3;
  bool conds_4;
  short int lowdyn_ir;
  short int lowdyn_wv;
  short int lowdyn_v1;
  short int lowdyn_v2;
  short int higdyn_ir;
  short int higdyn_wv;
  short int higdyn_v1;
  short int higdyn_v2;
  float mvis1;
  float mvis2;
  float snnom_ir;
  float snnom_wv;
  float snnom_v1;
  float snnom_v2;
  int snnlin;
  float snrep_ir;
  float snrep_wv;
  float snrep_v1;
  float snrep_v2;
  float snrwp_ir;
  float snrwp_wv;
  float snrwp_v1;
  float snrwp_v2;
  float swmnep_ir;
  float swmnep_wv;
  float swmnep_v1;
  float swmnep_v2;
  float swmnwp_ir;
  float swmnwp_wv;
  float swmnwp_v1;
  float swmnwp_v2;
  short int snmxep_ir;
  short int snmxep_wv;
  short int snmxep_v1;
  short int snmxep_v2;
  short int snmxwp_ir;
  short int snmxwp_wv;
  short int snmxwp_v1;
  short int snmxwp_v2;
} HRI_imagery_data;

typedef struct {
  char admin_message[800];
} HRI_administrative_message;

class HRI_subframe_interpretation {
  public:
    HRI_subframe_interpretation( ) { }
    HRI_subframe_interpretation( char *buf );
    ~HRI_subframe_interpretation( ) { }
    void readfrom( char *buf );

    HRI_calibration_data cal;
    HRI_spacecraft_operation_data oper;
    HRI_imagery_data imag;
    HRI_administrative_message mesg;

    // Overloaded >> operator
    friend std::ostream& operator<< ( std::ostream& os,
	                              HRI_subframe_interpretation& i )
    {
      os << "---------------------------------" << std::endl
	 << "-  HRI Subframe Interpretation  -" << std::endl
         << "---------------------------------" << std::endl
	 << "-----------------------------" << std::endl
	 << "- Calibration Data          -" << std::endl
	 << "-----------------------------" << std::endl
	 << "Black body count IR space    : "
	 << i.cal.bbc1ir << std::endl
	 << "STD black body IR space      : "
	 << i.cal.bbsd1i << std::endl 
	 << "Black body count WV space    : "
	 << i.cal.bbc1wv << std::endl
	 << "STD black body WV space      : "
	 << i.cal.bbsd1w << std::endl
	 << "Cold black body temperature  : "
	 << i.cal.bb1t << std::endl
	 << "Hot black body temperature   : "
	 << i.cal.bb2t << std::endl
	 << "Black body count IR nominal  : "
	 << i.cal.bbc2ir << std::endl
	 << "STD black body IR nominal    : "
	 << i.cal.bbsd2i << std::endl
	 << "Black body count WV nominal  : "
	 << i.cal.bbc2wv << std::endl
	 << "STD black body WV nominal    : "
	 << i.cal.bbsd2w << std::endl
	 << "Timestamp for BB calibration : "
	 << "Day " << i.cal.time1_day
	 << ", Slot " << i.cal.time1_slot << std::endl
	 << "MPEF absolute IR calibration : "
	 << i.cal.calir << std::endl
	 << "IR space count               : "
	 << i.cal.irspc << std::endl
	 << "Timestamp for IR calibration : "
	 << "Day " << i.cal.time2_day
	 << ", Slot " << i.cal.time2_slot << std::endl
	 << "MPEF absolute WV calibration : "
	 << i.cal.calwv << std::endl
	 << "WV space count               : "
	 << i.cal.wvspc << std::endl
	 << "Timestamp for WV calibration : "
	 << "Day " << i.cal.time3_day
	 << ", Slot " << i.cal.time3_slot << std::endl
	 << "IR gain                      : "
	 << i.cal.ir_gain << std::endl
	 << "WV gain                      : "
	 << i.cal.wv_gain << std::endl
	 << "V1 gain                      : "
	 << i.cal.v1_gain << std::endl
	 << "V2 gain                      : "
	 << i.cal.v2_gain << std::endl
	 << "-----------------------------" << std::endl
	 << "- Spacecraft Operation Data -" << std::endl
	 << "-----------------------------" << std::endl
         << "Right ascension attitude S   : "
	 << i.oper.degsra << std::endl
         << "Declination attitude S       : "
	 << i.oper.degsde << std::endl
         << "Right ascension attitude N   : "
	 << i.oper.degnra << std::endl
         << "Declination attitude N       : "
	 << i.oper.degnde << std::endl
         << "Refined attitude X           : "
	 << i.oper.finatt_x << std::endl
         << "Refined attitude Y           : "
	 << i.oper.finatt_y << std::endl
         << "Refined attitude Z           : "
	 << i.oper.finatt_z << std::endl
         << "Right ascension refined      : "
	 << i.oper.farade_1 << std::endl
         << "Right declination refined    : "
	 << i.oper.farade_2 << std::endl
         << "Number of slots for refined  : "
	 << i.oper.nrslot << std::endl
         << "Spin duration - nominal      : "
	 << i.oper.spndur << std::endl
         << "Eclipse operation            : "
	 << i.oper.flecl << std::endl
         << "Decontamination              : "
	 << i.oper.fldec << std::endl
         << "Manoeuvre                    : "
	 << i.oper.flman << std::endl
         << "Earth/Sun mode               : "
	 << i.oper.flmode << std::endl
         << "IR 1 on                      : "
	 << i.oper.flir1 << std::endl
         << "IR 2 on                      : "
	 << i.oper.flir2 << std::endl
         << "WV 1 on                      : "
	 << i.oper.flwv1 << std::endl
         << "WV 2 on                      : "
	 << i.oper.flwv2 << std::endl
         << "VIS 1 on                     : "
	 << i.oper.flvis1 << std::endl
         << "VIS 2 on                     : "
	 << i.oper.flvis2 << std::endl
         << "VIS 3 on                     : "
	 << i.oper.flvis3 << std::endl
         << "VIS 4 on                     : "
	 << i.oper.flvis4 << std::endl
	 << "-----------------------------" << std::endl
	 << "- Imagery Data              -" << std::endl
	 << "-----------------------------" << std::endl
         << "Horizon analysis performed   : "
	 << i.imag.imstat_1 << std::endl
         << "Speed fit                    : "
	 << i.imag.imstat_2 << std::endl
         << "Orbit offset vector fit      : "
	 << i.imag.imstat_3 << std::endl
         << "Pixel sampling rate fit      : "
	 << i.imag.imstat_4 << std::endl
         << "Attitude refinement iteration: "
	 << i.imag.imstat_5 << std::endl
         << "Automatic landmark reg.      : "
	 << i.imag.imstat_6 << std::endl
         << "Image frame movement fit     : "
	 << i.imag.imstat_7 << std::endl
         << "Calculation of deformed vector "
	 << i.imag.imstat_8 << std::endl
         << "Completion of geometrical proc."
	 << i.imag.imstat_9 << std::endl
         << "Completion of rectification    "
	 << i.imag.imstat_10 << std::endl
         << "Completion of amplitude proc.  "
	 << i.imag.imstat_10 << std::endl
         << "Southern line                : "
	 << i.imag.limhor_1 << std::endl
         << "Southern line, first pixel   : "
	 << i.imag.limhor_2 << std::endl
         << "Southern line, last pixel    : "
	 << i.imag.limhor_3 << std::endl
         << "Northern line                : "
	 << i.imag.limhor_4 << std::endl
         << "Northern line, first pixel   : "
	 << i.imag.limhor_5 << std::endl
         << "Northern line, last pixel    : "
	 << i.imag.limhor_6 << std::endl
         << "Eastern pixel                : "
	 << i.imag.limhor_7 << std::endl
         << "Eastern pixel, bottom line   : "
	 << i.imag.limhor_8 << std::endl
         << "Eastern pixel, top line      : "
	 << i.imag.limhor_9 << std::endl
         << "Western pixel                : "
	 << i.imag.limhor_10 << std::endl
         << "Western pixel, bottom line   : "
	 << i.imag.limhor_11 << std::endl
         << "Western pixel, top line      : "
	 << i.imag.limhor_12 << std::endl
         << "Satellite distance           : "
	 << i.imag.satdis << std::endl
         << "X offset vector in image S   : "
	 << i.imag.sorbof_x << std::endl
         << "Y offset vector in image S   : "
	 << i.imag.sorbof_y << std::endl
         << "Z offset vector in image S   : "
	 << i.imag.sorbof_z << std::endl
         << "X offset vector in image N   : "
	 << i.imag.norbof_x << std::endl
         << "Y offset vector in image N   : "
	 << i.imag.norbof_y << std::endl
         << "Z offset vector in image N   : "
	 << i.imag.norbof_z << std::endl
         << "Max deformation diff. X Col. : "
	 << i.imag.xddifm << std::endl
         << "Max deformation diff. Y Col. : "
	 << i.imag.yddifm << std::endl
         << "Max deformation diff. X Lin. : "
	 << i.imag.xscm << std::endl
         << "Max deformation diff. Y Lin. : "
	 << i.imag.yscm << std::endl
         << "Earth in field E/W           : "
	 << i.imag.conds_1 << std::endl
         << "Earth in field S/N           : "
	 << i.imag.conds_2 << std::endl
         << "E/W horizon in margin        : "
	 << i.imag.conds_3 << std::endl
         << "S/N horizon in margin        : "
	 << i.imag.conds_4 << std::endl
         << "Lowest count in histogram IR : "
	 << i.imag.lowdyn_ir << std::endl
         << "Lowest count in histogram WV : "
	 << i.imag.lowdyn_wv << std::endl
         << "Lowest count in histogram V1 : "
	 << i.imag.lowdyn_v1 << std::endl
         << "Lowest count in histogram V2 : "
	 << i.imag.lowdyn_v2 << std::endl
         << "Highest count in histogram IR: "
	 << i.imag.higdyn_ir << std::endl
         << "Highest count in histogram WV: "
	 << i.imag.higdyn_wv << std::endl
         << "Highest count in histogram V1: "
	 << i.imag.higdyn_v1 << std::endl
         << "Highest count in histogram V2: "
	 << i.imag.higdyn_v2 << std::endl
         << "Mean value of VIS 1 or 3     : "
	 << i.imag.mvis1 << std::endl
         << "Mean value of VIS 2 or 4     : "
	 << i.imag.mvis2 << std::endl
         << "S/N ratio in space corner IR : "
	 << i.imag.snnom_ir << std::endl
         << "S/N ratio in space corner WV : "
	 << i.imag.snnom_wv << std::endl
         << "S/N ratio in space corner V1 : "
	 << i.imag.snnom_v1 << std::endl
         << "S/N ratio in space corner V2 : "
	 << i.imag.snnom_v2 << std::endl
         << "Lines for nominal calculation: "
	 << i.imag.snnlin << std::endl
         << "S/N ratios eastern part IR   : "
	 << i.imag.snrep_ir << std::endl
         << "S/N ratios eastern part WV   : "
	 << i.imag.snrep_wv << std::endl
         << "S/N ratios eastern part V1   : "
	 << i.imag.snrep_v1 << std::endl
         << "S/N ratios eastern part V2   : "
	 << i.imag.snrep_v2 << std::endl
         << "S/N ratios western part IR   : "
	 << i.imag.snrwp_ir << std::endl
         << "S/N ratios western part WV   : "
	 << i.imag.snrwp_wv << std::endl
         << "S/N ratios western part V1   : "
	 << i.imag.snrwp_v1 << std::endl
         << "S/N ratios western part V2   : "
	 << i.imag.snrwp_v2 << std::endl
         << "Mean noise count east IR     : "
	 << i.imag.swmnep_ir << std::endl
         << "Mean noise count east WV     : "
	 << i.imag.swmnep_wv << std::endl
         << "Mean noise count east V1     : "
	 << i.imag.swmnep_v1 << std::endl
         << "Mean noise count east V2     : "
	 << i.imag.swmnep_v2 << std::endl
         << "Mean noise count west IR     : "
	 << i.imag.swmnwp_ir << std::endl
         << "Mean noise count west WV     : "
	 << i.imag.swmnwp_wv << std::endl
         << "Mean noise count west V1     : "
	 << i.imag.swmnwp_v1 << std::endl
         << "Mean noise count west V2     : "
	 << i.imag.swmnwp_v2 << std::endl
         << "Max space count east IR      : "
	 << i.imag.snmxep_ir << std::endl
         << "Max space count east WV      : "
	 << i.imag.snmxep_wv << std::endl
         << "Max space count east V1      : "
	 << i.imag.snmxep_v1 << std::endl
         << "Max space count east V2      : "
	 << i.imag.snmxwp_v2 << std::endl
         << "Max space count west IR      : "
	 << i.imag.snmxwp_ir << std::endl 
         << "Max space count west WV      : "
	 << i.imag.snmxwp_wv << std::endl
         << "Max space count west V1      : "
	 << i.imag.snmxwp_v1 << std::endl
         << "Max space count west V2      : "
	 << i.imag.snmxep_v2 << std::endl
	 << "-----------------------------" << std::endl
	 << "- Administrative Messages   -" << std::endl
	 << "-----------------------------" << std::endl
	 << i.mesg.admin_message << std::endl
	 << "-----------------------------" << std::endl;
      return os;
    }
  private:
    HRI_machine conv;
};

#endif
