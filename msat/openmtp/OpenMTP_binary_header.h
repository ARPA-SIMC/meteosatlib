//-----------------------------------------------------------------------------
//
//  Copyright (C) 2004--2006  Graziano Giuliani <giuliani@lamma.rete.toscana.it>
//  Copyright (C) 2006--2012  ARPAE-SIMC <urpsim@arpae.it>
//
//  File        : OpenMTP_binary_header.h
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

#ifndef __OPENMTP_BINARY_HEADER_H__
#define __OPENMTP_BINARY_HEADER_H__

#include <string.h>
#include <iostream>
#include <fstream>
#include <msat/openmtp/OpenMTP_machine.h>

using namespace std;

class OpenMTP_binary_header {
  public:
    OpenMTP_binary_header( );
    OpenMTP_binary_header( ifstream &file );
    ~OpenMTP_binary_header( );

    void read( ifstream &file );

    char *field_name ( );
    char *satellite_name ( );
    char *rectification_method_name ( );
    char *phenomena_index ( );
    int year( );
    int jday( );
    int slot( );
    bool is_no_data_slot( );
    bool is_normal_slot( );
    bool is_special_slot( );
    int dtype( );
    int date( );
    int time( );
    int proc( );
    bool is_raw_data( );
    bool is_ir_processed( );
    bool is_vis_processed( );
    bool is_wv_processed( );
    bool is_rectified_to_next_neighbour( );
    bool is_rectified( );
    int chan( );
    bool is_no_data( );
    bool is_VISs_data( );
    bool is_VISn_data( );
    bool is_visible_composite( );
    bool is_vis_data( );
    bool is_ir1_data( );
    bool is_ir2_data( );
    bool is_ir_data( );
    bool is_wv1_data( );
    bool is_wv2_data( );
    bool is_wv_data( );
    float mpef_calibration_coefficient( );
    float mpef_calibration_space_count( );
    int mpef_calibration_day( );
    int mpef_calibration_slot( );
    int header_2_record_size( );
    int image_line_record_size( );
    int offset_first_pixel_in_line( );
    int deformation_model( );
    bool is_deformation_none( );
    bool is_deformation_batch( );
    bool is_deformation_realtime( );
    int resampling_method( );
    bool is_resampling_none( );
    bool is_resampling_nearest_neighbour( );
    bool is_resampling_splines_4x4( );
    float subsatellite_point( );
    int origin( );
    bool is_south_east( );
    bool is_north_east( );
    bool is_north_west( );
    bool is_south_west( );
    int first_line( );
    int first_pixel( );
    int nlines( );
    int npixels( );
    int image_quality( );
    bool is_image_quality_nominal( );
    bool is_image_attitude_unknown( );
    bool is_image_orbit_unknown( );
    bool is_image_horizon_incomplete( );
    bool is_image_deformation_not_calculated( );
    bool is_image_RADOPOS_LID_inconsistent( );
    bool is_image_HR_not_interpreted( );
    char *missing_line_table( );
    char *missing_line_table_VISn( );
    bool is_line_present( int line_index );
    bool is_line_present_VISn( int line_index );
    int urect_nominal_end_time( );
    int urect_processing_performed( );
    bool is_urect_raw_data( );
    bool is_urect_preprocessed_data( );
    int urect_special_output( );
    int is_urect_special_output_no_data( );
    int is_urect_special_output_VISs_IR_WV( );
    int is_urect_special_output_VISn( );
    int is_urect_special_output_VISs_VISn( );
    int urect_radiometer_refpos( );
    int urect_refpos_line_number( );
    short urect_scanning_law( );
    short urect_number_of_received_subimage( );
    short *urect_first_line_in_subimage( );
    short *urect_nlines_in_subimage( );
    short *urect_curr_decoded_rad_pos_of_first_line_in_subimage( );
    int *urect_raw_image_histogram( );
    int *urect_raw_image_histogram_VISn( );
    double urec_start_image( );
    double urec_end_image( );
    double *urec_orbit_coord_in_geoframe_start( );
    double *urec_orbit_coord_in_geoframe_end( );
    float *urec_cartesian_attitude_start( );
    float *urec_cartesian_attitude_end( );
    short *urec_horizon_information( );
    double urec_time_of_first_pixel( );
    bool *urec_status_flags( );
    bool is_urec_horizon_analysis_ok( );
    bool is_urec_spin_speed_fit_ok( );
    bool is_urec_orbit_offset_vector_fit_ok( );
    bool is_urec_pixel_resampling_rate_fit_ok( );
    bool is_urec_attitude_refinement_ok( );
    bool is_urec_automatic_landmark_ok( );
    bool is_urec_actual_image_movement_fit_ok( );
    bool is_urec_calculation_of_deformation_ok( );
    bool is_urec_geometrical_LPEF_ok( );
    bool is_urec_rectification_segmentation_ok( );
    bool is_urec_amplitude_processing_ok( );
    short urec_ir_channel_in_use( );
    short urec_radiometer_step_offset_start( );
    short *urec_horizon_limit_information( );
    double *urec_times_of_horizon_scan_lines( );
    short urec_radiometer_step_south( );
    short urec_radiometer_step_north( );
    float urec_rmid( );
    double urec_tmid( );
    double urec_distance_es( );
    double urec_cone_angle_beta_south_obs( );
    double urec_cone_angle_beta_north_obs( );
    double urec_cone_angle_beta_south_exp( );
    double urec_cone_angle_beta_north_exp( );
    double urec_etas( );
    double urec_etan( );
    double urec_betasn( );
    double urec_betann( );
    double urec_step_parameter_F0_old( );
    double urec_step_parameter_F1_old( );
    double urec_step_parameter_F0_new( );
    double urec_step_parameter_F1_new( );
    double urec_spin_deviation_fit_const( );
    double urec_spin_deviation_fit_linear( );
    double urec_spin_deviation_fit_quad( );
    double urec_standard_deviation_spin_fit( );
    double urec_maximum_deviation_spin_fit( );
    int ngridpoints_deformation_matrix( );
    int grid_deformation_start( );
    int grid_deformation_end( );
    int grid_deformation_step( );
    float *deformation_matrix_x( );
    float *deformation_matrix_y( );
    int number_corrected_channels( );
    int chid_first_corrected( );
    float *line_geometric_correction_EW_1( );
    float *line_geometric_correction_NS_1( );
    float *line_radiometric_correction_offset_1( );
    float *line_radiometric_correction_gain_1( );
    int chid_second_corrected( );
    float *line_geometric_correction_EW_2( );
    float *line_geometric_correction_NS_2( );
    float *line_radiometric_correction_offset_2( );
    float *line_radiometric_correction_gain_2( );

    // Overloaded << operator
    friend std::ostream& operator<< ( std::ostream& os,
                                      OpenMTP_binary_header &h )
    {
      float *ftmp;
      int i, j;

      os << "Product Type\t" << h.field_name( ) << std::endl 
         << "Year\t\t" << h.year( ) << std::endl 
         << "Jday\t\t" << h.jday( ) << std::endl 
         << "Slot\t\t" << h.slot( ) << std::endl 
         << "Dtype\t\t" << h.dtype( ) << std::endl 
         << "Date\t\t" << h.date( ) << std::endl 
         << "Time\t\t" << h.time( ) << std::endl 
         << "Satellite Name\t" << h.satellite_name( ) << std::endl 
         << "Proc\t\t" << h.proc( ) << std::endl 
         << "CalC0\t\t" << h.mpef_calibration_coefficient( ) << std::endl 
         << "Space\t\t" << h.mpef_calibration_space_count( ) << std::endl 
         << "Calctim\t\t" << "Day " << h.mpef_calibration_day( )
         << " Slot " << h.mpef_calibration_slot( ) << std::endl
         << "Rec2siz\t\t" << h.header_2_record_size( ) << std::endl
         << "Lrecsize\t" << h.image_line_record_size( ) << std::endl
         << "Loffset\t\t" << h.offset_first_pixel_in_line( ) << std::endl
         << "Rtmet\t\t" << h.rectification_method_name( ) << std::endl 
         << "Dmmod\t\t" << h.deformation_model( ) << std::endl
         << "Rsmet\t\t" << h.resampling_method( ) << std::endl
         << "Ssp\t\t" << h.subsatellite_point( ) << std::endl
         << "Origin\t\t" << h.origin( ) << std::endl
         << "Idx\t\t" << h.phenomena_index( ) << std::endl
         << "Line1\t\t" << h.first_line( ) << std::endl
         << "Pixel1\t\t" << h.first_pixel( ) << std::endl
         << "Nlines\t\t" << h.nlines( ) << std::endl
         << "Npixels\t\t" << h.npixels( ) << std::endl
         << "Mlt1\t\t" << h.missing_line_table( ) << std::endl
         << "Mlt2\t\t" << h.missing_line_table_VISn( ) << std::endl
         << "Imgqua\t\t" << h.image_quality( ) << std::endl;
      if (! h.is_rectified())
      {
        short *stmp;
        int *itmp;
        double *dtmp;
        bool *btmp;
        os << "Int\t\t" << h.urect_nominal_end_time( ) << std::endl
           << "Imp\t\t" << h.urect_processing_performed( ) << std::endl
           << "Spr\t\t" << h.urect_special_output( ) << std::endl
           << "Rpr\t\t" << h.urect_radiometer_refpos( ) << std::endl
           << "Lre\t\t" << h.urect_refpos_line_number( ) << std::endl
           << "Lb0\t\t" << h.urect_scanning_law( ) << std::endl
           << "Nsi\t\t" << h.urect_number_of_received_subimage( ) << std::endl;
        stmp = h.urect_first_line_in_subimage( );
        for (i = 0; i < 20; i ++)
          os << "Fls " << i << "\t\t" << stmp[i] << std::endl;
        stmp = h.urect_nlines_in_subimage( );
        for (i = 0; i < 20; i ++)
          os << "Nsl " << i << "\t\t" << stmp[i] << std::endl;
        stmp = h.urect_curr_decoded_rad_pos_of_first_line_in_subimage();
        for (i = 0; i < 20; i ++)
          os << "Rdpsim " << i << "\t\t" << stmp[i] << std::endl;
        itmp = h.urect_raw_image_histogram( );
        for (i = 0; i < 256; i ++)
          os << "Hist1 " << i << "\t\t" << itmp[i] << std::endl;
        itmp = h.urect_raw_image_histogram_VISn( );
        for (i = 0; i < 256; i ++)
          os << "Hist2 " << i << "\t\t" << itmp[i] << std::endl;
        os << "Timef\t\t" << h.urec_start_image( ) << std::endl
           << "Timel\t\t" << h.urec_end_image( ) << std::endl;
        dtmp = h.urec_orbit_coord_in_geoframe_start( );
        for (i = 0; i < 6; i ++)
          os << "Orbf " << i << "\t\t" << dtmp[i] << std::endl;
        dtmp = h.urec_orbit_coord_in_geoframe_end( );
        for (i = 0; i < 6; i ++)
          os << "Orbl " << i << "\t\t" << dtmp[i] << std::endl;
        ftmp = h.urec_cartesian_attitude_start( );
        for (i = 0; i < 4; i ++)
          os << "Attf " << i << "\t\t" << ftmp[i] << std::endl;
        ftmp = h.urec_cartesian_attitude_end( );
        for (i = 0; i < 4; i ++)
          os << "Attl " << i << "\t\t" << ftmp[i] << std::endl;
        stmp = h.urec_horizon_information( );
        for (j = 0; j < 4; j ++)
          for (i = 0; i < 3; i ++)
            os << "Earco (" << j+1 << "," << i+1 << ")\t"
               << stmp[j*3+i] << std::endl;
        os << "Htime\t\t" << h.urec_time_of_first_pixel( ) << std::endl;
        btmp = h.urec_status_flags( );
        for (i = 0; i < 16; i ++)
          os << "Status " << i << "\t" << btmp[i] << std::endl;
        os << "Irchan\t\t" << h.urec_ir_channel_in_use( ) << std::endl
           << "Lstart\t\t"
           << h.urec_radiometer_step_offset_start( ) << std::endl;
        stmp = h.urec_horizon_limit_information( );
        for (j = 0; j < 4; j ++)
          for (i = 0; i < 3; i ++)
            os << "Horlim (" << j+1 << "," << i+1 << ")\t"
               << stmp[j*3+i] << std::endl;
        dtmp = h.urec_times_of_horizon_scan_lines( );
        os << "Hortim South" << "\t" << dtmp[0] << std::endl
           << "Hortim North" << "\t" << dtmp[1] << std::endl
           << "Ls\t\t" << h.urec_radiometer_step_south( ) << std::endl
           << "Ln\t\t" << h.urec_radiometer_step_north( ) << std::endl
           << "Rmid\t\t" << h.urec_rmid( ) << std::endl
           << "Tmid\t\t" << h.urec_tmid( ) << std::endl
           << "Distan\t\t" << h.urec_distance_es( ) << std::endl
           << "Betaso\t\t" << h.urec_cone_angle_beta_south_obs( ) << std::endl
           << "Betano\t\t" << h.urec_cone_angle_beta_north_obs( ) << std::endl
           << "Betase\t\t" << h.urec_cone_angle_beta_south_exp( ) << std::endl
           << "Betane\t\t" << h.urec_cone_angle_beta_north_exp( ) << std::endl
           << "Etas\t\t" << h.urec_etas( ) << std::endl
           << "Etan\t\t" << h.urec_etan( ) << std::endl
           << "Betasn\t\t" << h.urec_betasn( ) << std::endl
           << "Betann\t\t" << h.urec_betann( ) << std::endl
           << "F0old\t\t" << h.urec_step_parameter_F0_old( ) << std::endl
           << "F1old\t\t" << h.urec_step_parameter_F1_old( ) << std::endl
           << "F0new\t\t" << h.urec_step_parameter_F0_new( ) << std::endl
           << "F1new\t\t" << h.urec_step_parameter_F1_new( ) << std::endl
           << "S0\t\t" << h.urec_spin_deviation_fit_const( ) << std::endl
           << "S1\t\t" << h.urec_spin_deviation_fit_linear( ) << std::endl
           << "S2\t\t" << h.urec_spin_deviation_fit_quad( ) << std::endl
           << "Sigmas\t\t"
           << h.urec_standard_deviation_spin_fit( ) << std::endl;
      }
      os << "Ndgrp\t\t" << h.ngridpoints_deformation_matrix( ) << std::endl
         << "Dmstrt\t\t" << h.grid_deformation_start( ) << std::endl
         << "Dmend\t\t" << h.grid_deformation_end( ) << std::endl
         << "Dmstep\t\t" << h.grid_deformation_step( ) << std::endl;
      ftmp = h.deformation_matrix_x( );
      for (j = 0; j < 105; j ++)
        for (i = 0; i < 105; i ++)
          os << "Defmax (" << j+1 << "," << i+1 << ")\t"
             << ftmp[j*105+i] << std::endl;
      ftmp = h.deformation_matrix_y( );
      for (j = 0; j < 105; j ++)
        for (i = 0; i < 105; i ++)
          os << "Defmay (" << j+1 << "," << i+1 << ")\t"
             << ftmp[j*105+i] << std::endl;
      os << "Ncor\t\t" << h.number_corrected_channels() << std::endl
         << "Chid1\t\t" << h.chid_first_corrected( ) << std::endl;
      ftmp = h.line_geometric_correction_EW_1( );
      for (i = 0; i < 3030; i ++)
        os << "Ewgeo1 " << i << "\t" << ftmp[i] << std::endl;
      ftmp = h.line_geometric_correction_NS_1( );
      for (i = 0; i < 3030; i ++)
        os << "Nsgeo1 " << i << "\t" << ftmp[i] << std::endl;
      ftmp = h.line_radiometric_correction_offset_1( );
      for (i = 0; i < 3030; i ++)
        os << "Roff1 " << i << "\t" << ftmp[i] << std::endl;
      ftmp = h.line_radiometric_correction_gain_1( );
      for (i = 0; i < 3030; i ++)
        os << "Rgain1 " << i << "\t" << ftmp[i] << std::endl;
      if (h.is_visible_composite())
      {
        os << "Chid2\t\t" << h.chid_second_corrected( ) << std::endl;
        ftmp = h.line_geometric_correction_EW_2( );
        for (i = 0; i < 3030; i ++)
          os << "Ewgeo2 " << i << "\t" << ftmp[i] << std::endl;
        ftmp = h.line_geometric_correction_NS_2( );
        for (i = 0; i < 3030; i ++)
          os << "Nsgeo2 " << i << "\t" << ftmp[i] << std::endl;
        ftmp = h.line_radiometric_correction_offset_2( );
        for (i = 0; i < 3030; i ++)
          os << "Roff2 " << i << "\t" << ftmp[i] << std::endl;
        ftmp = h.line_radiometric_correction_gain_2( );
        for (i = 0; i < 3030; i ++)
          os << "Rgain2 " << i << "\t" << ftmp[i] << std::endl;
      }
      return os;
    }

  private:
    const static int BINARY_HEADER_FIRST_SECTION_LENGTH         = 5175;
    const static int BINARY_HEADER_SECOND_SECTION_LENGTH        = 2636;
    const static int BINARY_HEADER_THIRD_SECTION_NORMAL_LEGTH   = 136704;
    const static int BINARY_HEADER_THIRD_SECTION_VIS_CMP_LENGTH = 185188;
    const static int BUFLEN = 192999;
    unsigned char header[BUFLEN];
    OpenMTP_machine m;
};

#endif
