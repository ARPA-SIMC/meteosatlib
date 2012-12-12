/*
 * Copyright: (C) 2004, 2005, 2006 Deneys S. Maartens
 * Copyright: (C) 2012  ARPA-SIM <urpsim@smr.arpa.emr.it>
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

// SYSTEM INCLUDES
//
#include <iostream>

// PROJECT INCLUDES
//
#include "ByteSex.hh"

// LOCAL INCLUDES
//
#include "FileHeader.hh" // class implemented

// FORWARD REFERENCES
//
using namespace omtp_ids;

// *********************************************************************

// **************************** PRIVATE    *****************************

// **************************** PROTECTED  *****************************

// **************************** PUBLIC     *****************************

// ============================ LIFECYCLE  =============================

FileHeader::FileHeader() :
	m_no_records(0),

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
	for (int i = 0; i < offset; i++) {
		os.put('\0');
	}

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

	if (offset < 0) {
		is.setstate(std::ios::failbit);
	}
	// no seekg() available on gzstream
	for (int i = 0; i < offset; i++) {
		is.get();
	}

	return is;
}

// ============================ OPERATIONS =============================

std::ostream&
FileHeader::debug(std::ostream& os) const
{
	os << "FileHeader :";
	os << "\n  fortran bytes        : \"";
	for (int i = 0; i < FORTRAN_LEN; i++) {
		os << static_cast<int>(m_fortran[i]);
	}
	os << "\"";

	os << "\n  number of records    : " << m_no_records;
	os << "\n  year                 : " << m_year;
	os << "\n  julian_day           : " << m_julian_day;
	os << "\n  hour                 : " << m_hour;
	os << "\n  minute               : " << m_minute;

	os << "\n  process year         : " << m_process_year;
	os << "\n  process julian_day   : " << m_process_julian_day;
	os << "\n  process hour         : " << m_process_hour;
	os << "\n  process minute       : " << m_process_minute;

	os << "\n  number of channels   : " << m_no_channels;
	os << "\n  vis channel id       : " << m_vis_id;
	os << "\n  wv channel id        : " << m_wv_id;
	os << "\n  ir channel id        : " << m_ir_id;

	os << "\n  satellite            : \"";
	for (int i = 0; i < SATELLITE_LEN; i++) {
		os << m_satellite[i];
	}
	os << "\"";

	os << "\n  satellite id         : " << m_satellite_id;
	os << "\n  record length        : " << m_record_length;

	os << "\n  vis no average       : " << m_no_vis_average;
	os << "\n  ir no average        : " << m_no_ir_average;
	os << "\n  wv no average        : " << m_no_wv_average;
	os << "\n  averaging type       : " << m_averaging_type;

	os << "\n  sample interval      : " << m_sample_interval;
	os << "\n  ir calibration       : " << m_ir_calibration;
	os << "\n  wv calibration       : " << m_wv_calibration;
	os << "\n  fine adjustment      : " << m_fine_adjustment;
	os << "\n  ir space count       : " << m_ir_space_count;
	os << "\n  wv space count       : " << m_wv_space_count;
	os << '\n';

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
