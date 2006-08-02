/* $Id$
 * Copyright: (C) 2004, 2005, 2006 Deneys S. Maartens
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
 * USA.
 */
/*
 * Constants used throughout the omtp-ids library
 */
#ifndef DSM_CONSTANTS_HH
#define DSM_CONSTANTS_HH

// SYSTEM INCLUDES
//

// PROJECT INCLUDES
//

// LOCAL INCLUDES
//

// FORWARD REFERENCES
//

// *********************************************************************

namespace omtp_ids {

	// Header lengths
	const int FILEHEADER_LEN   = 60; // bytes
	const int LINEHEADER_LEN   = 16; // bytes
	const int RECORDHEADER_LEN = 8; // bytes

	// Fortran bytes
	const int FORTRAN_LEN = 2; // bytes
	const char FORTRAN[] = { 0, 1 };

	// FileHeader satellite name
	const int SATELLITE_LEN = 8; // bytes
	const char SATELLITE[] = "METEOSAT";

	// LineHeader pad numbers
	const int PAD = 2; // integers

	// FileHeader satellite id
	const int METEOSAT_3 = 16;
	const int METEOSAT_4 = 19;
	const int METEOSAT_5 = 20;
	const int METEOSAT_6 = 21;
	const int MTP_1      = 150;
	const int MTP_2      = 151;

	// FileHeader channel ID
	enum Channel_Id {
		NO_DATA = 0,
		VIS_1,
		VIS_2,
		VIS_1_2,
		IR_1,
		IR_2,
		WV_1,
		WV_2,
		VIS_3,
		VIS_4,
		VIS_2_3,
		VIS_2_4,
		VIS_3_4
	};

#if 0
	struct DataType {
		int pixels_per_line;
		int scan_line_size;
		int scan_line_per_record;
		int record_lenght;
		int no_records;
		int no_vis_average;
		int no_ir_average;
		int no_wv_average;
	};

	const DataType AC = {
		/* .pixels_per_line       = */ 400   ,
		/* .scan_line_size        = */ 416   ,
		/* .scan_lines_per_record = */ 48    ,
		/* .record_length         = */ 19976 ,
		/* .no_records            = */ 26    ,
		/* .no_vis_average        = */ 4     ,
		/* .no_ir_average         = */ 1     ,
		/* .no_wv_average         = */ 1
	};

	const DataType B1 = {
		/* .pixels_per_line       = */ 1248  ,
		/* .scan_line_size        = */ 1264  ,
		/* .scan_lines_per_record = */ 12    ,
		/* .record_length         = */ 15200 ,
		/* .no_records            = */ 313   ,
		/* .no_vis_average        = */ 16    ,
		/* .no_ir_average         = */ 4     ,
		/* .no_wv_average         = */ 4
	};

	const DataType B2 = {
		/* .pixels_per_line       = */ 416   ,
		/* .scan_line_size        = */ 432   ,
		/* .scan_lines_per_record = */ 36    ,
		/* .record_length         = */ 15560 ,
		/* .no_records            = */ 36    ,
		/* .no_vis_average        = */ 144   ,
		/* .no_ir_average         = */ 36    ,
		/* .no_wv_average         = */ 36
	};
#endif
}

// EXTERNAL REFERENCES
//

#endif // DSM_CONSTANTS_HH
