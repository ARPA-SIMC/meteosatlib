//-----------------------------------------------------------------------------
//
//  Copyright (C) 2004--2006  Graziano Giuliani <giuliani@lamma.rete.toscana.it>
//  Copyright (C) 2006--2012  ARPAE-SIMC <urpsim@arpae.it>
//
//  File        : HRI_subframe_label.cpp
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
#include <msat/hri/HRI_subframe_label.h>
#include <string>

HRI_subframe_label::HRI_subframe_label( char hsl[24] )
{
  readfrom(hsl);
}

void HRI_subframe_label::readfrom( char hsl[24] )
{
  number_of_frames_in_subframe    =
                 conv.i2_from_buff((const unsigned char *) hsl);
  total_number_of_subframes       =
                 conv.i2_from_buff((const unsigned char *) hsl+2);
  current_subframe_number         =
                 conv.i2_from_buff((const unsigned char *) hsl+4);
  image_line_number               =
                 conv.i2_from_buff((const unsigned char *) hsl+6);
  image_number_from_mission_start =
                 conv.i4_from_buff((const unsigned char *) hsl+8);
  format_indicator                = hsl[12];
  VISs_indicator                  = hsl[13];
  VISn_indicator                  = hsl[14];
  IR_indicator                    = hsl[15];
  WV_indicator                    = hsl[16];
  grid_indicator                  = hsl[17];
  encryption_indicator            = hsl[18];
  scan_direction_indicator        = hsl[19];
  key_number_VIS_s_indicator      = hsl[20];
  key_number_VIS_n_indicator      = hsl[21];
  key_number_IR_indicator         = hsl[22];
  key_number_WV_indicator         = hsl[23];
  return;
}

bool HRI_subframe_label::is_A_format( )
{
  if (format_indicator == 0) return true;
  return false;
}

bool HRI_subframe_label::is_B_format( )
{
  if (format_indicator == 255) return true;
  return false;
}

bool HRI_subframe_label::is_X_format( )
{
  if (format_indicator == 15) return true;
  return false;
}

bool HRI_subframe_label::is_VISs_present( )
{
  if (VISs_indicator) return true;
  return false;
}

bool HRI_subframe_label::is_VISn_present( )
{
  if (VISn_indicator) return true;
  return false;
}

bool HRI_subframe_label::is_IR_present( )
{
  if (IR_indicator) return true;
  return false;
}

bool HRI_subframe_label::is_WV_present( )
{
  if (WV_indicator) return true;
  return false;
}

bool HRI_subframe_label::is_VISs_1st_half_present( )
{
  if (VISs_indicator == 240) return true;
  return false;
}

bool HRI_subframe_label::is_VISs_2nd_half_present( )
{
  if (VISs_indicator == 15) return true;
  return false;
}

bool HRI_subframe_label::is_VISn_1st_half_present( )
{
  if (VISn_indicator == 240) return true;
  return false;
}

bool HRI_subframe_label::is_VISn_2nd_half_present( )
{
  if (VISn_indicator == 15) return true;
  return false;
}

bool HRI_subframe_label::is_VISs_reduced( )
{
  if (VISs_indicator == 255) return true;
  return false;
}

bool HRI_subframe_label::is_VISn_reduced( )
{
  if (VISn_indicator == 255) return true;
  return false;
}

bool HRI_subframe_label::is_VISs_encrypted( )
{
  if (encryption_indicator & 8) return true;
  return false;
}

bool HRI_subframe_label::is_VISn_encrypted( )
{
  if (encryption_indicator & 4) return true;
  return false;
}

bool HRI_subframe_label::is_IR_encrypted( )
{
  if (encryption_indicator & 2) return true;
  return false;
}

bool HRI_subframe_label::is_WV_encrypted( )
{
  if (encryption_indicator & 1) return true;
  return false;
}

bool HRI_subframe_label::is_scan_SN_EW( )
{
  if (scan_direction_indicator == 0) return true;
  return false;
}

bool HRI_subframe_label::is_scan_NS_EW( )
{
  if (scan_direction_indicator == 240) return true;
  return false;
}

bool HRI_subframe_label::is_scan_SN_WE( )
{
  if (scan_direction_indicator == 15) return true;
  return false;
}

bool HRI_subframe_label::is_scan_NS_WE( )
{
  if (scan_direction_indicator == 255) return true;
  return false;
}

bool HRI_subframe_label::is_grid_present( )
{
  if (grid_indicator) return true;
  return false;
}

std::string HRI_subframe_label::format_code( )
{
  static std::string tmp;
  bool hasv = false;

  if      (is_A_format( )) tmp = "A";
  else if (is_B_format( )) tmp = "B";
  else if (is_X_format( )) tmp = "X";
  else
  {
    std::cerr << "Unknown format code in HRI read" << std::endl;
    throw;
  }

  if (is_IR_present( )) tmp += "I";
  if (is_VISs_present( ) || is_VISn_present( )) { tmp += "V"; hasv = true; }
  if (is_WV_present( )) tmp += "W";
  else if (hasv && (is_VISs_reduced( ) || is_VISn_reduced( ))) tmp += "H";
  return tmp;
}
