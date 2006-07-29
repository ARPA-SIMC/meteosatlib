// $Id: FileHeader.cc,v 1.1.1.1 2004/07/16 11:05:26 giuliani Exp $
//
// NAME:
//   FileHeader -
//
// AUTHOR:        Deneys Maartens
// VERSION:       $Rev$
// DATE:          $Date: 2004/07/16 11:05:26 $
// COPYRIGHT:     Deneys Maartens (C) 2004
//
// LICENCE:
//   This program is free software; you can redistribute it and/or
//   modify it under the terms of the GNU General Public License as
//   published by the Free Software Foundation; either version 2, or (at
//   your option) any later version.
//
//   This program is distributed in the hope that it will be useful, but
//   WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//   General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program; if not, write to the Free Software
//   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//   02111-1307, USA.
//
// *********************************************************************

// SYSTEM INCLUDES
//
#include <iostream>

// PROJECT INCLUDES
//

// LOCAL INCLUDES
//
#include "ByteSex.hh"
#include "FileHeader.hh" // class implemented

// LOCAL CONSTANTS
//

using namespace omtp_ids;

// **************************** PRIVATE    *****************************

// **************************** PROTECTED  *****************************

// **************************** PUBLIC     *****************************

// ============================ LIFECYCLE  =============================

FileHeader::FileHeader()
 : m_no_records(0),

   m_year(0),
   m_julian_day(0),
   m_hour(0),
   m_minute(0),

   m_process_year(0),
   m_process_julian_day(0),
   m_process_hour(0),
   m_process_minute(0),

   m_no_channels(0),
   m_vis_id(0),
   m_wv_id(0),
   m_ir_id(0),

   m_satellite_id(0),
   m_record_length(0),

   m_no_vis_average(0),
   m_no_wv_average(0),
   m_no_ir_average(0),
   m_averaging_type(0),

   m_sample_interval(0),
   m_ir_calibration(0),
   m_wv_calibration(0),
   m_fine_adjustment(0),
   m_ir_space_count(0),
   m_wv_space_count(0)
{
	memcpy(m_fortran, FORTRAN, FORTRAN_LEN);
	memcpy(m_satellite, SATELLITE, SATELLITE_LEN + 1);
}

// ============================ OPERATORS  =============================

std::ostream&
operator<<(std::ostream& os,
	   const FileHeader& fileheader)
{
	os.write(fileheader.m_fortran, FORTRAN_LEN);
	ByteSex::big::write2(os, fileheader.m_no_records        );

	ByteSex::big::write2(os, fileheader.m_year              );
	ByteSex::big::write2(os, fileheader.m_julian_day        );
	ByteSex::big::write2(os, fileheader.m_hour              );
	ByteSex::big::write2(os, fileheader.m_minute            );

	ByteSex::big::write2(os, fileheader.m_process_year      );
	ByteSex::big::write2(os, fileheader.m_process_julian_day);
	ByteSex::big::write2(os, fileheader.m_process_hour      );
	ByteSex::big::write2(os, fileheader.m_process_minute    );

	ByteSex::big::write2(os, fileheader.m_no_channels       );
	ByteSex::big::write2(os, fileheader.m_vis_id            );
	ByteSex::big::write2(os, fileheader.m_wv_id             );
	ByteSex::big::write2(os, fileheader.m_ir_id             );

	os.write(fileheader.m_satellite, SATELLITE_LEN);
	ByteSex::big::write2(os, fileheader.m_satellite_id      );
	ByteSex::big::write2(os, fileheader.m_record_length     );

	ByteSex::big::write2(os, fileheader.m_no_vis_average    );
	ByteSex::big::write2(os, fileheader.m_no_wv_average     );
	ByteSex::big::write2(os, fileheader.m_no_ir_average     );
	ByteSex::big::write2(os, fileheader.m_averaging_type    );

	ByteSex::big::write2(os, fileheader.m_sample_interval   );
	ByteSex::big::write2(os, fileheader.m_ir_calibration    );
	ByteSex::big::write2(os, fileheader.m_wv_calibration    );
	ByteSex::big::write2(os, fileheader.m_fine_adjustment   );
	ByteSex::big::write2(os, fileheader.m_ir_space_count    );
	ByteSex::big::write2(os, fileheader.m_wv_space_count    );

	const int offset = fileheader.record_length()
		+ FORTRAN_LEN - FILEHEADER_LEN;
	for (int i = 0; i < offset; ++i)
		os.put('\0');

	return os;
}

std::istream&
operator>>(std::istream& is,
	   FileHeader& fileheader)
{
	fileheader = FileHeader();

	is.read(fileheader.m_fortran, FORTRAN_LEN);
	fileheader.m_no_records           = ByteSex::big::read2(is);

	fileheader.m_year                 = ByteSex::big::read2(is);
	fileheader.m_julian_day           = ByteSex::big::read2(is);
	fileheader.m_hour                 = ByteSex::big::read2(is);
	fileheader.m_minute               = ByteSex::big::read2(is);

	fileheader.m_process_year         = ByteSex::big::read2(is);
	fileheader.m_process_julian_day   = ByteSex::big::read2(is);
	fileheader.m_process_hour         = ByteSex::big::read2(is);
	fileheader.m_process_minute       = ByteSex::big::read2(is);

	fileheader.m_no_channels          = ByteSex::big::read2(is);
	fileheader.m_vis_id               = ByteSex::big::read2(is);
	fileheader.m_wv_id                = ByteSex::big::read2(is);
	fileheader.m_ir_id                = ByteSex::big::read2(is);

	is.read(fileheader.m_satellite, SATELLITE_LEN);
	fileheader.m_satellite[SATELLITE_LEN] = 0;
	fileheader.m_satellite_id         = ByteSex::big::read2(is);
	fileheader.m_record_length        = ByteSex::big::read2(is);

	fileheader.m_no_vis_average       = ByteSex::big::read2(is);
	fileheader.m_no_wv_average        = ByteSex::big::read2(is);
	fileheader.m_no_ir_average        = ByteSex::big::read2(is);
	fileheader.m_averaging_type       = ByteSex::big::read2(is);

	fileheader.m_sample_interval      = ByteSex::big::read2(is);
	fileheader.m_ir_calibration       = ByteSex::big::read2(is);
	fileheader.m_wv_calibration       = ByteSex::big::read2(is);
	fileheader.m_fine_adjustment      = ByteSex::big::read2(is);
	fileheader.m_ir_space_count       = ByteSex::big::read2(is);
	fileheader.m_wv_space_count       = ByteSex::big::read2(is);

	const int offset = fileheader.record_length()
		+ FORTRAN_LEN - FILEHEADER_LEN;
	is.seekg(offset, std::ios::cur);

	return is;
}

// ============================ OPERATIONS =============================

std::ostream&
FileHeader::debug(std::ostream& os) const
{
	os << "FileHeader :" << std::endl;
	os << "  fortran bytes        : \"";
	for (int i = 0; i < FORTRAN_LEN; ++i)
		os << static_cast<int>(m_fortran[i]);
	os << "\"" << std::endl;

	os << "  number of records    : " << m_no_records         << std::endl;
	os << "  year                 : " << m_year               << std::endl;
	os << "  julian_day           : " << m_julian_day         << std::endl;
	os << "  hour                 : " << m_hour               << std::endl;
	os << "  minute               : " << m_minute             << std::endl;

	os << "  process year         : " << m_process_year       << std::endl;
	os << "  process julian_day   : " << m_process_julian_day << std::endl;
	os << "  process hour         : " << m_process_hour       << std::endl;
	os << "  process minute       : " << m_process_minute     << std::endl;

	os << "  number of channels   : " << m_no_channels        << std::endl;
	os << "  vis channel id       : " << m_vis_id             << std::endl;
	os << "  wv channel id        : " << m_wv_id              << std::endl;
	os << "  ir channel id        : " << m_ir_id              << std::endl;
	os << "  satellite            : \"";
	for (int i = 0; i < SATELLITE_LEN; ++i)
		os << m_satellite[i];
	os << "\"" << std::endl;
	os << "  satellite id         : " << m_satellite_id       << std::endl;
	os << "  record length        : " << m_record_length      << std::endl;
	os << "  vis no average       : " << m_no_vis_average     << std::endl;
	os << "  ir no average        : " << m_no_ir_average      << std::endl;
	os << "  wv no average        : " << m_no_wv_average      << std::endl;
	os << "  averaging type       : " << m_averaging_type     << std::endl;
	os << "  sample interval      : " << m_sample_interval    << std::endl;
	os << "  ir calibration       : " << m_ir_calibration     << std::endl;
	os << "  wv calibration       : " << m_wv_calibration     << std::endl;
	os << "  fine adjustment      : " << m_fine_adjustment    << std::endl;
	os << "  ir space count       : " << m_ir_space_count     << std::endl;
	os << "  wv space count       : " << m_wv_space_count     << std::endl;

	return os;
}

// ============================ INQUIRY    =============================

// ============================ ACCESS     =============================

void
FileHeader::fortran(const char* fortran)
{
	const int len = strlen(fortran);
	const int copy_len = (len <= FORTRAN_LEN)
		? len
		: FORTRAN_LEN;

	memcpy(m_fortran, fortran, copy_len);
}

void
FileHeader::satellite(const char* satellite)
{
	const int len = strlen(satellite);
	const int copy_len = (len <= SATELLITE_LEN)
		? len
		: SATELLITE_LEN;

	memcpy(m_satellite, satellite, copy_len);
}
