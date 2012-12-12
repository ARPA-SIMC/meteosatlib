//-----------------------------------------------------------------------------
//
//  Copyright (C) 2004--2006  Graziano Giuliani <giuliani@lamma.rete.toscana.it>
//  Copyright (C) 2006--2012  ARPA-SIM <urpsim@smr.arpa.emr.it>
//
//  File        : HRI_subframe_label.h
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
#ifndef __HRI_SUBFRAME_LABEL_H__
#define __HRI_SUBFRAME_LABEL_H__

#include <msat/hri/HRI_machine.h>
#include <iostream>

class HRI_subframe_label {
  public:
    HRI_subframe_label( ) { }
    HRI_subframe_label( char hsl[24] );
    ~HRI_subframe_label( ) { }
    void readfrom( char hsl[24] );
    bool is_A_format( );
    bool is_B_format( );
    bool is_X_format( );
    bool is_VISs_present( );
    bool is_VISn_present( );
    bool is_VISs_1st_half_present( );
    bool is_VISs_2nd_half_present( );
    bool is_VISn_1st_half_present( );
    bool is_VISn_2nd_half_present( );
    bool is_VISs_reduced( );
    bool is_VISn_reduced( );
    bool is_IR_present( );
    bool is_WV_present( );
    bool is_grid_present( );
    bool is_VISs_encrypted( );
    bool is_VISn_encrypted( );
    bool is_IR_encrypted( );
    bool is_WV_encrypted( );
    bool is_scan_SN_EW( );
    bool is_scan_NS_EW( );
    bool is_scan_SN_WE( );
    bool is_scan_NS_WE( );
    unsigned short number_of_frames_in_subframe;
    unsigned short total_number_of_subframes;
    unsigned short current_subframe_number;
    unsigned short image_line_number;
    unsigned int image_number_from_mission_start;
    unsigned char format_indicator;
    unsigned char VISs_indicator;
    unsigned char VISn_indicator;
    unsigned char IR_indicator;
    unsigned char WV_indicator;
    unsigned char grid_indicator;
    unsigned char encryption_indicator;
    unsigned char scan_direction_indicator;
    unsigned char key_number_VIS_s_indicator;
    unsigned char key_number_VIS_n_indicator;
    unsigned char key_number_IR_indicator;
    unsigned char key_number_WV_indicator;

    std::string format_code( );

    // Overloaded >> operator
    friend std::ostream& operator<< ( std::ostream& os,
	                              HRI_subframe_label& l )
    {
      os << "---------------------------------" << std::endl
	 << "-       HRI Subframe Label      -" << std::endl
         << "---------------------------------" << std::endl
	 << "Number of frames in Subframe : "
	 << l.number_of_frames_in_subframe << std::endl
	 << "Total number of subframes    : "
	 << l.total_number_of_subframes << std::endl
	 << "Current subframe number      : "
	 << l.current_subframe_number << std::endl
	 << "Image line number            : "
	 << l.image_line_number << std::endl
	 << "Global image number          : "
	 << l.image_number_from_mission_start << std::endl
	 << "Format                       : ";
      if (l.is_A_format()) os << "A";
      else if (l.is_B_format()) os << "B";
      else if (l.is_X_format()) os << "X";
      else os << "Unknown";
      os << std::endl;
      os << "Scan direction               : ";
      if (l.is_scan_SN_EW()) os << "S to N and E to W" << std::endl;
      else if (l.is_scan_NS_EW()) os << "N to S and E to W" << std::endl;
      else if (l.is_scan_SN_WE()) os << "S to N and W to E" << std::endl;
      else if (l.is_scan_NS_WE()) os << "N to S and W to E" << std::endl;
      else os << "Unknown" << std::endl;
      os << std::endl;
      if (l.is_VISs_present())
      {
	if (l.is_VISs_reduced())
	  os << "VIS-s reduced channel present";
	else if (l.is_VISs_1st_half_present())
	  os << "VIS-s channel first half line present";
	else if (l.is_VISs_2nd_half_present())
	  os << "VIS-s second half of line present";
	else os << "VIS-s channel presence unknown";
        os << std::endl;
        if (l.is_VISs_encrypted( ))
	{
	  os << "VIS-s channel encrypted with key :"
	     << l.key_number_VIS_s_indicator << std::endl;
	}
      }
      else os << "VIS-s channel not present" << std::endl;
      if (l.is_VISn_present())
      {
	if (l.is_VISn_reduced())
	  os << "VIS-n reduced channel present";
	else if (l.is_VISn_1st_half_present())
	  os << "VIS-n channel first half line present";
	else if (l.is_VISn_2nd_half_present())
	  os << "VIS-n second half of line present";
	else os << "VIS-n channel presence unknown";
        os << std::endl;
        if (l.is_VISn_encrypted( ))
	{
	  os << "VIS-n channel encrypted with key :"
	     << l.key_number_VIS_n_indicator << std::endl;
	}
      }
      else os << "VIS-n channel not present" << std::endl;
      if (l.is_IR_present())
      {
	os << "IR channel present" << std::endl;
	if (l.is_IR_encrypted())
	  os << "IR channel encrypted with key :"
	     << l.key_number_IR_indicator << std::endl;
      }
      else os << "IR channel not present" << std::endl;
      if (l.is_WV_present())
      {
	os << "WV channel present" << std::endl;
	if (l.is_WV_encrypted())
	  os << "WV channel encrypted with key :"
	     << l.key_number_WV_indicator << std::endl;
      }
      else os << "WV channel not present" << std::endl;
      if (l.is_grid_present())
	os << "Grid information present" << std::endl;
      else
        os << "Grid information not present" << std::endl;
      os << std::endl;
      
      return os;
    };
  private:
    HRI_machine conv;
};

#endif
