//-----------------------------------------------------------------------------
//
//  Copyright (C) 2004--2006  Graziano Giuliani <giuliani@lamma.rete.toscana.it>
//  Copyright (C) 2006--2012  ARPAE-SIMC <urpsim@arpae.it>
//
//  File        : OpenMTP_binary_header.cpp
//  Description : Meteosat OpenMTP format interface
//  Project     : Meteosatlib
//  Authors     : Graziano Giuliani (Lamma Regione Toscana)
//              : Enrico Zini <enrico@enricozini.com>
//  References  : Eumetsat EUM-FG-1 Format Guide: OpenMTP format
//                Revision 2.1 April 2000
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

#include <string.h>
#include <iostream>
#include <fstream>
#include "OpenMTP_machine.h"
#include "OpenMTP_binary_header.h"

OpenMTP_binary_header::OpenMTP_binary_header( ) { }
OpenMTP_binary_header::OpenMTP_binary_header( std::ifstream &file )
{
  read(file);
}
OpenMTP_binary_header::~OpenMTP_binary_header( ) { }

void OpenMTP_binary_header::read( std::ifstream &file )
{
  unsigned char *pnt;

  memset(header, 0, BUFLEN);

  file.read((char *) header, BINARY_HEADER_FIRST_SECTION_LENGTH);
  if (file.fail( ))
  {
    std::cerr << "Read error : BINARY Header, first section." << std::endl;
    throw;
  }

  pnt = header + BINARY_HEADER_FIRST_SECTION_LENGTH;

  if (is_rectified())
  {
    file.seekg(BINARY_HEADER_SECOND_SECTION_LENGTH, ios::cur);
    if (file.fail( ))
    {
      std::cerr << "Read error : BINARY Header, second section" << std::endl;
      throw;
    }
  }
  else
  {
    file.read((char *) pnt, BINARY_HEADER_SECOND_SECTION_LENGTH);
    if (file.fail( ))
    {
      std::cerr << "Read error : BINARY Header, second section" << std::endl;
      throw;
    }
  }

  pnt = pnt + BINARY_HEADER_SECOND_SECTION_LENGTH;
  if (is_visible_composite())
  {
    file.read((char *) pnt, BINARY_HEADER_THIRD_SECTION_VIS_CMP_LENGTH);
    if (file.fail( ))
    {
      std::cerr << "Read error : BINARY Header, third section" << std::endl;
      throw;
    }
  }
  else
  {
    file.read((char *) pnt, BINARY_HEADER_THIRD_SECTION_NORMAL_LEGTH);
    if (file.fail( ))
    {
      std::cerr << "Read error : BINARY Header, third section" << std::endl;
      throw;
    }
  }

  return;
}

char *OpenMTP_binary_header::field_name ( )
{
  static char tmp[9];
  memcpy(tmp, header, 8);
  tmp[8] = 0;
  return tmp;
}

char *OpenMTP_binary_header::satellite_name ( )
{
  static char tmp[3];
  memcpy(tmp, header+32, 2);
  tmp[2] = 0;
  return tmp;
}

char *OpenMTP_binary_header::rectification_method_name ( )
{
  static char tmp[16];
  memcpy(tmp, header+72, 15);
  tmp[15] = 0;
  return tmp;
}

char *OpenMTP_binary_header::phenomena_index ( )
{
  static char tmp[9];
  memcpy(tmp, header+115, 8);
  tmp[8] = 0;
  return tmp;
}
int OpenMTP_binary_header::year( )
{
  return m.int4(header+8);
}
int OpenMTP_binary_header::jday( )
{
  return m.int4(header+12);
}
int OpenMTP_binary_header::slot( )
{
  return m.int4(header+16);
}
bool OpenMTP_binary_header::is_no_data_slot( )
{
  return (m.int4(header+16) == 0);
}
bool OpenMTP_binary_header::is_normal_slot( )
{
  return (m.int4(header+16) == 1);
}
bool OpenMTP_binary_header::is_special_slot( )
{
  return (m.int4(header+16) == 2);
}
int OpenMTP_binary_header::dtype( )
{
  return m.int4(header+20);
}
int OpenMTP_binary_header::date( )
{
  return m.int4(header+24);
}
int OpenMTP_binary_header::time( )
{
  return m.int4(header+28);
}
int OpenMTP_binary_header::proc( )
{
  return m.int4(header+36);
}
bool OpenMTP_binary_header::is_raw_data( )
{
  return (m.int4(header+36) == 0);
}
bool OpenMTP_binary_header::is_ir_processed( )
{
  return (m.int4(header+36) == 1);
}
bool OpenMTP_binary_header::is_vis_processed( )
{
  return (m.int4(header+36) == 2);
}
bool OpenMTP_binary_header::is_wv_processed( )
{
  return (m.int4(header+36) == 3);
}
bool OpenMTP_binary_header::is_rectified_to_next_neighbour( )
{
  return (m.int4(header+36) == 5);
}
bool OpenMTP_binary_header::is_rectified( )
{
  return (m.int4(header+36) > 3);
}
int OpenMTP_binary_header::chan( )
{
  return m.int4(header+40);
}
bool OpenMTP_binary_header::is_no_data( )
{
  return (m.int4(header+40) == 0 );
}
bool OpenMTP_binary_header::is_VISs_data( )
{
  return (m.int4(header+40) == 1 );
}
bool OpenMTP_binary_header::is_VISn_data( )
{
  return (m.int4(header+40) == 2 );
}
bool OpenMTP_binary_header::is_visible_composite( )
{
  return (m.int4(header+40) == 3);
}
bool OpenMTP_binary_header::is_vis_data( )
{
  return (is_VISs_data( ) || is_VISn_data( ) ||
          is_visible_composite( ));
}
bool OpenMTP_binary_header::is_ir1_data( )
{
  return (m.int4(header+40) == 4);
}
bool OpenMTP_binary_header::is_ir2_data( )
{
  return (m.int4(header+40) == 5);
}
bool OpenMTP_binary_header::is_ir_data( )
{
  return (is_ir1_data( ) || is_ir2_data( ));
}
bool OpenMTP_binary_header::is_wv1_data( )
{
  return (m.int4(header+40) == 6);
}
bool OpenMTP_binary_header::is_wv2_data( )
{
  return (m.int4(header+40) == 7);
}
bool OpenMTP_binary_header::is_wv_data( )
{
  return (is_wv1_data( ) || is_wv2_data( ));
}
float OpenMTP_binary_header::mpef_calibration_coefficient( )
{
  return m.r4_from_char5(header+44);
}
float OpenMTP_binary_header::mpef_calibration_space_count( )
{
  return m.r4_from_char3(header+49);
}
int OpenMTP_binary_header::mpef_calibration_day( )
{
  return m.int4_from_char3(header+52);
}
int OpenMTP_binary_header::mpef_calibration_slot( )
{
  return m.int4_from_char2(header+55);
}
int OpenMTP_binary_header::header_2_record_size( )
{
  return m.int4(header+60);
}
int OpenMTP_binary_header::image_line_record_size( )
{
  return m.int4(header+64);
}
int OpenMTP_binary_header::offset_first_pixel_in_line( )
{
  return m.int4(header+68);
}
int OpenMTP_binary_header::deformation_model( )
{
  return m.int4(header+87);
}
bool OpenMTP_binary_header::is_deformation_none( )
{
  return (m.int4(header+87) == 0);
}
bool OpenMTP_binary_header::is_deformation_batch( )
{
  return (m.int4(header+87) == 1);
}
bool OpenMTP_binary_header::is_deformation_realtime( )
{
  return (m.int4(header+87) == 2);
}
int OpenMTP_binary_header::resampling_method( )
{
  return m.int4(header+91);
}
bool OpenMTP_binary_header::is_resampling_none( )
{
  return (m.int4(header+91) == 0);
}
bool OpenMTP_binary_header::is_resampling_nearest_neighbour( )
{
  return (m.int4(header+91) == 1);
}
bool OpenMTP_binary_header::is_resampling_splines_4x4( )
{
  return (m.int4(header+91) == 2);
}
float OpenMTP_binary_header::subsatellite_point( )
{
  return m.float4(header+95);
}
int OpenMTP_binary_header::origin( )
{
  return m.int4(header+111);
}
bool OpenMTP_binary_header::is_south_east( )
{
  return (m.int4(header+111) == 0);
}
bool OpenMTP_binary_header::is_north_east( )
{
  return (m.int4(header+111) == 1);
}
bool OpenMTP_binary_header::is_north_west( )
{
  return (m.int4(header+111) == 2);
}
bool OpenMTP_binary_header::is_south_west( )
{
  return (m.int4(header+111) == 3);
}
int OpenMTP_binary_header::first_line( )
{
  return m.int4(header+123);
}
int OpenMTP_binary_header::first_pixel( )
{
  return m.int4(header+127);
}
int OpenMTP_binary_header::nlines( )
{
  return m.int4(header+131);
}
int OpenMTP_binary_header::npixels( )
{
  return m.int4(header+135);
}
int OpenMTP_binary_header::image_quality( )
{
  return m.int4(header+5155);
}
bool OpenMTP_binary_header::is_image_quality_nominal( )
{
  return (m.int4(header+5155) == 0);
}
bool OpenMTP_binary_header::is_image_attitude_unknown( )
{
  return (m.int4(header+5155) == 1);
}
bool OpenMTP_binary_header::is_image_orbit_unknown( )
{
  return (m.int4(header+5155) == 2);
}
bool OpenMTP_binary_header::is_image_horizon_incomplete( )
{
  return (m.int4(header+5155) == 3);
}
bool OpenMTP_binary_header::is_image_deformation_not_calculated( )
{
  return (m.int4(header+5155) == 4);
}
bool OpenMTP_binary_header::is_image_RADOPOS_LID_inconsistent( )
{
  return (m.int4(header+5155) == 5);
}
bool OpenMTP_binary_header::is_image_HR_not_interpreted( )
{
  return (m.int4(header+5155) == 6);
}
char *OpenMTP_binary_header::missing_line_table( )
{
  static char tmp[2501];
  memcpy(tmp, header+155, 2500);
  tmp[2500] = 0;
  return tmp;
}
char *OpenMTP_binary_header::missing_line_table_VISn( )
{
  static char tmp[2501];
  memcpy(tmp, header+2655, 2500);
  tmp[2500] = 0;
  return tmp;
}
bool OpenMTP_binary_header::is_line_present( int line_index )
{
  return (*(header+155+line_index) > 0);
}
bool OpenMTP_binary_header::is_line_present_VISn( int line_index )
{
  return (*(header+2655+line_index) > 0);
}
int OpenMTP_binary_header::urect_nominal_end_time( )
{
  return m.int4(header+5175);
}
int OpenMTP_binary_header::urect_processing_performed( )
{
  return m.int4(header+5179);
}
bool OpenMTP_binary_header::is_urect_raw_data( )
{
  return (m.int4(header+5179) == 0);
}
bool OpenMTP_binary_header::is_urect_preprocessed_data( )
{
  return (m.int4(header+5179) == 1);
}
int OpenMTP_binary_header::urect_special_output( )
{
  return m.int4(header+5183);
}
int OpenMTP_binary_header::is_urect_special_output_no_data( )
{
  return (m.int4(header+5183) == 0);
}
int OpenMTP_binary_header::is_urect_special_output_VISs_IR_WV( )
{
  return (m.int4(header+5183) == 1);
}
int OpenMTP_binary_header::is_urect_special_output_VISn( )
{
  return (m.int4(header+5183) == 2);
}
int OpenMTP_binary_header::is_urect_special_output_VISs_VISn( )
{
  return (m.int4(header+5183) == 3);
}
int OpenMTP_binary_header::urect_radiometer_refpos( )
{
  return m.int4(header+5187);
}
int OpenMTP_binary_header::urect_refpos_line_number( )
{
  return m.int4(header+5191);
}
short OpenMTP_binary_header::urect_scanning_law( )
{
  return m.int2(header+5195);
}
short OpenMTP_binary_header::urect_number_of_received_subimage( )
{
  return m.int2(header+5197);
}
short *OpenMTP_binary_header::urect_first_line_in_subimage( )
{
  static short tmp[20];

  for (int i = 0; i < 20; i ++)
    tmp[i] = m.int2(header+5199+2*i);
  return tmp;
}
short *OpenMTP_binary_header::urect_nlines_in_subimage( )
{
  static short tmp[20];

  for (int i = 0; i < 20; i ++)
    tmp[i] = m.int2(header+5239+2*i);
  return tmp;
}
short *OpenMTP_binary_header::urect_curr_decoded_rad_pos_of_first_line_in_subimage( )
{
  static short tmp[20];

  for (int i = 0; i < 20; i ++)
    tmp[i] = m.int2(header+5279+2*i);
  return tmp;
}
int *OpenMTP_binary_header::urect_raw_image_histogram( )
{
  static int tmp[256];

  for (int i = 0; i < 20; i ++)
    tmp[i] = m.int4(header+5319+4*i);
  return tmp;
}
int *OpenMTP_binary_header::urect_raw_image_histogram_VISn( )
{
  static int tmp[256];

  for (int i = 0; i < 20; i ++)
    tmp[i] = m.int4(header+6343+4*i);
  return tmp;
}
double OpenMTP_binary_header::urec_start_image( )
{
  return m.float8(header+7367);
}
double OpenMTP_binary_header::urec_end_image( )
{
  return m.float8(header+7375);
}
double *OpenMTP_binary_header::urec_orbit_coord_in_geoframe_start( )
{
  static double tmp[6];
  for (int i = 0; i < 6; i ++)
    tmp[i] = m.float8(header+7383+i*8);
  return tmp;
}
double *OpenMTP_binary_header::urec_orbit_coord_in_geoframe_end( )
{
  static double tmp[6];
  for (int i = 0; i < 6; i ++)
    tmp[i] = m.float8(header+7431+i*8);
  return tmp;
}
float *OpenMTP_binary_header::urec_cartesian_attitude_start( )
{
  static float tmp[4];
  tmp[0] = m.float4(header+7479);
  tmp[1] = m.float4(header+7479+4);
  tmp[2] = m.float4(header+7479+8);
  tmp[3] = m.float4(header+7479+12);
  return tmp;
}
float *OpenMTP_binary_header::urec_cartesian_attitude_end( )
{
  static float tmp[4];
  tmp[0] = m.float4(header+7491);
  tmp[1] = m.float4(header+7491+4);
  tmp[2] = m.float4(header+7491+8);
  tmp[3] = m.float4(header+7491+12);
  return tmp;
}
short *OpenMTP_binary_header::urec_horizon_information( )
{
  static short tmp[12];
  for (int i = 0; i < 12; i ++)
      tmp[i] = m.int2(header+7503+i*2);
  return tmp;
}
double OpenMTP_binary_header::urec_time_of_first_pixel( )
{
  return m.float8(header+7527);
}
bool *OpenMTP_binary_header::urec_status_flags( )
{
  static bool tmp[16];
  for (int i = 0; i < 16; i ++)
    tmp[i] = *(header+7559+i);
  return tmp;
}
bool OpenMTP_binary_header::is_urec_horizon_analysis_ok( )
{
  return (*(header+7559)>0);
}
bool OpenMTP_binary_header::is_urec_spin_speed_fit_ok( )
{
  return (*(header+7560)>0);
}
bool OpenMTP_binary_header::is_urec_orbit_offset_vector_fit_ok( )
{
  return (*(header+7561)>0);
}
bool OpenMTP_binary_header::is_urec_pixel_resampling_rate_fit_ok( )
{
  return (*(header+7562)>0);
}
bool OpenMTP_binary_header::is_urec_attitude_refinement_ok( )
{
  return (*(header+7563)>0);
}
bool OpenMTP_binary_header::is_urec_automatic_landmark_ok( )
{
  return (*(header+7564)>0);
}
bool OpenMTP_binary_header::is_urec_actual_image_movement_fit_ok( )
{
  return (*(header+7565)>0);
}
bool OpenMTP_binary_header::is_urec_calculation_of_deformation_ok( )
{
  return (*(header+7566)>0);
}
bool OpenMTP_binary_header::is_urec_geometrical_LPEF_ok( )
{
  return (*(header+7567)>0);
}
bool OpenMTP_binary_header::is_urec_rectification_segmentation_ok( )
{
  return (*(header+7568)>0);
}
bool OpenMTP_binary_header::is_urec_amplitude_processing_ok( )
{
  return (*(header+7569)>0);
}
short OpenMTP_binary_header::urec_ir_channel_in_use( )
{
  return m.int2(header+7575);
}
short OpenMTP_binary_header::urec_radiometer_step_offset_start( )
{
  return m.int2(header+7577);
}
short *OpenMTP_binary_header::urec_horizon_limit_information( )
{
  static short tmp[12];
  for (int i = 0; i < 12; i ++)
      tmp[i] = m.int2(header+7579+i*2);
  return tmp;
}
double *OpenMTP_binary_header::urec_times_of_horizon_scan_lines( )
{
  static double tmp[2];
  tmp[0] = m.float8(header+7579);
  tmp[1] = m.float8(header+7579+8);
  return tmp;
}
short OpenMTP_binary_header::urec_radiometer_step_south( )
{
  return m.int2(header+7619);
}
short OpenMTP_binary_header::urec_radiometer_step_north( )
{
  return m.int2(header+7621);
}
float OpenMTP_binary_header::urec_rmid( )
{
  return m.float4(header+7623);
}
double OpenMTP_binary_header::urec_tmid( )
{
  return m.float8(header+7627);
}
double OpenMTP_binary_header::urec_distance_es( )
{
  return m.float8(header+7635);
}
double OpenMTP_binary_header::urec_cone_angle_beta_south_obs( )
{
  return m.float8(header+7643);
}
double OpenMTP_binary_header::urec_cone_angle_beta_north_obs( )
{
  return m.float8(header+7651);
}
double OpenMTP_binary_header::urec_cone_angle_beta_south_exp( )
{
  return m.float8(header+7659);
}
double OpenMTP_binary_header::urec_cone_angle_beta_north_exp( )
{
  return m.float8(header+7667);
}
double OpenMTP_binary_header::urec_etas( )
{
  return m.float8(header+7675);
}
double OpenMTP_binary_header::urec_etan( )
{
  return m.float8(header+7683);
}
double OpenMTP_binary_header::urec_betasn( )
{
  return m.float8(header+7691);
}
double OpenMTP_binary_header::urec_betann( )
{
  return m.float8(header+7699);
}
double OpenMTP_binary_header::urec_step_parameter_F0_old( )
{
  return m.float8(header+7707);
}
double OpenMTP_binary_header::urec_step_parameter_F1_old( )
{
  return m.float8(header+7715);
}
double OpenMTP_binary_header::urec_step_parameter_F0_new( )
{
  return m.float8(header+7723);
}
double OpenMTP_binary_header::urec_step_parameter_F1_new( )
{
  return m.float8(header+7731);
}
double OpenMTP_binary_header::urec_spin_deviation_fit_const( )
{
  return m.float8(header+7755);
}
double OpenMTP_binary_header::urec_spin_deviation_fit_linear( )
{
  return m.float8(header+7763);
}
double OpenMTP_binary_header::urec_spin_deviation_fit_quad( )
{
  return m.float8(header+7771);
}
double OpenMTP_binary_header::urec_standard_deviation_spin_fit( )
{
  return m.float8(header+7779);
}
double OpenMTP_binary_header::urec_maximum_deviation_spin_fit( )
{
  return m.float8(header+7787);
}
int OpenMTP_binary_header::ngridpoints_deformation_matrix( )
{
  return m.int4(header+7811);
}
int OpenMTP_binary_header::grid_deformation_start( )
{
  return m.int4(header+7815);
}
int OpenMTP_binary_header::grid_deformation_end( )
{
  return m.int4(header+7819);
}
int OpenMTP_binary_header::grid_deformation_step( )
{
  return m.int4(header+7823);
}
float *OpenMTP_binary_header::deformation_matrix_x( )
{
  static float tmp[11025];

  for (int i = 0; i < 11025; i ++)
    tmp[i] = m.float4(header+7827+i*4);
  return tmp;
}
float *OpenMTP_binary_header::deformation_matrix_y( )
{
  static float tmp[11025];

  for (int i = 0; i < 11025; i ++)
    tmp[i] = m.float4(header+51927+i*4);
  return tmp;
}
int OpenMTP_binary_header::number_corrected_channels( )
{
  return m.int4(header+96027);
}
int OpenMTP_binary_header::chid_first_corrected( )
{
  return m.int4(header+96031);
}
float *OpenMTP_binary_header::line_geometric_correction_EW_1( )
{
  static float tmp[3030];

  for (int i = 0; i < 3030; i ++)
    tmp[i] = m.float4(header+96035+i*4);
  return tmp;
}
float *OpenMTP_binary_header::line_geometric_correction_NS_1( )
{
  static float tmp[3030];

  for (int i = 0; i < 3030; i ++)
    tmp[i] = m.float4(header+108155+i*4);
  return tmp;
}
float *OpenMTP_binary_header::line_radiometric_correction_offset_1( )
{
  static float tmp[3030];

  for (int i = 0; i < 3030; i ++)
    tmp[i] = m.float4(header+120275+i*4);
  return tmp;
}
float *OpenMTP_binary_header::line_radiometric_correction_gain_1( )
{
  static float tmp[3030];

  for (int i = 0; i < 3030; i ++)
    tmp[i] = m.float4(header+132395+i*4);
  return tmp;
}
int OpenMTP_binary_header::chid_second_corrected( )
{
  return m.int4(header+144515);
}
float *OpenMTP_binary_header::line_geometric_correction_EW_2( )
{
  static float tmp[3030];

  for (int i = 0; i < 3030; i ++)
    tmp[i] = m.float4(header+144519+i*4);
  return tmp;
}
float *OpenMTP_binary_header::line_geometric_correction_NS_2( )
{
  static float tmp[3030];

  for (int i = 0; i < 3030; i ++)
    tmp[i] = m.float4(header+156639+i*4);
  return tmp;
}
float *OpenMTP_binary_header::line_radiometric_correction_offset_2( )
{
  static float tmp[3030];

  for (int i = 0; i < 3030; i ++)
    tmp[i] = m.float4(header+168759+i*4);
  return tmp;
}
float *OpenMTP_binary_header::line_radiometric_correction_gain_2( )
{
  static float tmp[3030];

  for (int i = 0; i < 3030; i ++)
    tmp[i] = m.float4(header+180879+i*4);
  return tmp;
}
