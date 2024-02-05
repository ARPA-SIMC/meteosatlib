/*
 * Copyright: (C) 2004, 2005, 2006 Deneys S. Maartens
 * Copyright: (C) 2012  ARPAE-SIMC <urpsim@arpae.it>
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
 * File Header
 */
#ifndef DSM_FILEHEADER_HH
#define DSM_FILEHEADER_HH

// SYSTEM INCLUDES
//
#include <iosfwd>
#include <cstring>

// PROJECT INCLUDES
//
#include <msat/omtp-ids/Constants.hh>

// LOCAL INCLUDES
//

// FORWARD REFERENCES
//

// *********************************************************************

class FileHeader
{
private:

protected:

	char m_fortran[omtp_ids::FORTRAN_LEN];
	int m_no_records;

	int m_year;
	int m_julian_day;
	int m_hour;
	int m_minute;

	int m_process_year;
	int m_process_julian_day;
	int m_process_hour;
	int m_process_minute;

	int m_no_channels;
	int m_vis_id;
	int m_wv_id;
	int m_ir_id;

	char m_satellite[omtp_ids::SATELLITE_LEN + 1];
	int m_satellite_id;
	int m_record_length;

	int m_no_vis_average;
	int m_no_wv_average;
	int m_no_ir_average;
	int m_averaging_type;

	int m_sample_interval;
	int m_ir_calibration;
	int m_wv_calibration;
	int m_fine_adjustment;
	int m_ir_space_count;
	int m_wv_space_count;

public:

	// LIFECYCLE

	FileHeader();

	// OPERATORS

	friend std::ostream& operator<<(std::ostream& os,
					const FileHeader& fileheader);
	friend std::istream& operator>>(std::istream& is,
					FileHeader& fileheader);

	// OPERATIONS

	std::ostream& debug(std::ostream& os) const;

	// INQUIRY

	// ACCESS

	// get
	const char* fortran() const;
	int no_records() const;

	int year() const;
	int julian_day() const;
	int hour() const;
	int minute() const;

	int process_year() const;
	int process_julian_day() const;
	int process_hour() const;
	int process_minute() const;

	int no_channels() const;
	int vis_id() const;
	int wv_id() const;
	int ir_id() const;

	const char* satellite() const;
	int satellite_id() const;
	int record_length() const;

	int no_vis_average() const;
	int no_wv_average() const;
	int no_ir_average() const;
	int averaging_type() const;

	int sample_interval() const;
	int ir_calibration() const;
	int wv_calibration() const;
	int fine_adjustment() const;
	int ir_space_count() const;
	int wv_space_count() const;

	// set
	void fortran(const char* fortran);
	void no_records(const int no_records);

	void year(const int year);
	void julian_day(const int julian);
	void hour(const int hour);
	void minute(const int minute);

	void process_year(const int process_year);
	void process_julian_day(const int process_julian_day);
	void process_hour(const int process_hour);
	void process_minute(const int process_minute);

	void no_channels(const int no_channels);
	void vis_id(const int vis_id);
	void wv_id(const int wv_id);
	void ir_id(const int ir_id);

	void satellite(const char* satellite);
	void satellite_id(const int satellite_id);
	void record_length(const int record_length);

	void no_vis_average(const int no_vis_average);
	void no_wv_average(const int no_wv_average);
	void no_ir_average(const int no_ir_average);
	void averaging_type(const int averaging_type);

	void sample_interval(const int sample_interval);
	void ir_calibration(const int ir_calibration);
	void wv_calibration(const int wv_calibration);
	void fine_adjustment(const int fine_adjustment);
	void ir_space_count(const int ir_space_count);
	void wv_space_count(const int wv_spage_count);
};

// EXTERNAL REFERENCES
//

inline
const char*
FileHeader::fortran() const
{
	return m_fortran;
}

inline
int
FileHeader::no_records() const
{
	return m_no_records;
}

inline
int
FileHeader::year() const
{
	return m_year;
}

inline
int
FileHeader::julian_day() const
{
	return m_julian_day;
}

inline
int
FileHeader::hour() const
{
	return m_hour;

}

inline
int
FileHeader::minute() const
{
	return m_minute;
}

inline
int
FileHeader::process_year() const
{
	return m_process_year;
}

inline
int
FileHeader::process_julian_day() const
{
	return m_process_julian_day;
}

inline
int
FileHeader::process_hour() const
{
	return m_process_hour;
}

inline
int
FileHeader::process_minute() const
{
	return m_process_minute;
}

inline
int
FileHeader::no_channels() const
{
	return m_no_channels;
}

inline
int
FileHeader::vis_id() const
{
	return m_vis_id;
}

inline
int
FileHeader::wv_id() const
{
	return m_wv_id;
}

inline
int
FileHeader::ir_id() const
{
	return m_ir_id;
}

inline
const char*
FileHeader::satellite() const
{
	return m_satellite;
}

inline
int
FileHeader::satellite_id() const
{
	return m_satellite_id;
}

inline
int
FileHeader::record_length() const
{
	return m_record_length;
}

inline
int
FileHeader::no_vis_average() const
{
	return m_no_vis_average;
}

inline
int
FileHeader::no_wv_average() const
{
	return m_no_wv_average;
}

inline
int
FileHeader::no_ir_average() const
{
	return m_no_ir_average;
}

inline
int
FileHeader::averaging_type() const
{
	return m_averaging_type;
}

inline
int
FileHeader::sample_interval() const
{
	return m_sample_interval;
}

inline
int
FileHeader::ir_calibration() const
{
	return m_ir_calibration;
}

inline
int
FileHeader::wv_calibration() const
{
	return m_wv_calibration;
}

inline
int
FileHeader::fine_adjustment() const
{
	return m_fine_adjustment;
}

inline
int
FileHeader::ir_space_count() const
{
	return m_ir_space_count;
}

inline
int
FileHeader::wv_space_count() const
{
	return m_wv_space_count;
}

inline
void
FileHeader::no_records(const int no_records)
{
	m_no_records = no_records;
}

inline
void
FileHeader::year(const int year)
{
	m_year = year;
}

inline
void
FileHeader::julian_day(const int julian_day)
{
	m_julian_day = julian_day;
}

inline
void
FileHeader::hour(const int hour)
{
	m_hour = hour;
}

inline
void
FileHeader::minute(const int minute)
{
	m_minute = minute;
}

inline
void
FileHeader::process_year(const int process_year)
{
	m_process_year = process_year;
}

inline
void
FileHeader::process_julian_day(const int process_julian_day)
{
	m_process_julian_day = process_julian_day;
}

inline
void
FileHeader::process_hour(const int process_hour)
{
	m_process_hour = process_hour;
}

inline
void
FileHeader::process_minute(const int process_minute)
{
	m_process_minute = process_minute;
}

inline
void
FileHeader::no_channels(const int no_channels)
{
	m_no_channels = no_channels;
}

inline
void
FileHeader::vis_id(const int vis_id)
{
	m_vis_id = vis_id;
}

inline
void
FileHeader::wv_id(const int wv_id)
{
	m_wv_id = wv_id;
}

inline
void
FileHeader::ir_id(const int ir_id)
{
	m_ir_id = ir_id;
}

inline
void
FileHeader::satellite_id(const int satellite_id)
{
	m_satellite_id = satellite_id;
}

inline
void
FileHeader::record_length(const int record_length)
{
	m_record_length = record_length;
}

inline
void
FileHeader::no_vis_average(const int no_vis_average)
{
	m_no_vis_average = no_vis_average;
}

inline
void
FileHeader::no_wv_average(const int no_wv_average)
{
	m_no_wv_average = no_wv_average;
}

inline
void
FileHeader::no_ir_average(const int no_ir_average)
{
	m_no_ir_average = no_ir_average;
}

inline
void
FileHeader::averaging_type(const int averaging_type)
{
	m_averaging_type = averaging_type;
}

inline
void
FileHeader::sample_interval(const int sample_interval)
{
	m_sample_interval = sample_interval;
}

inline
void
FileHeader::ir_calibration(const int ir_calibration)
{
	m_ir_calibration = ir_calibration;
}

inline
void
FileHeader::wv_calibration(const int wv_calibration)
{
	m_wv_calibration = wv_calibration;
}

inline
void
FileHeader::fine_adjustment(const int fine_adjustment)
{
	m_fine_adjustment = fine_adjustment;
}

inline
void
FileHeader::ir_space_count(const int ir_space_count)
{
	m_ir_space_count = ir_space_count;
}

inline
void
FileHeader::wv_space_count(const int wv_space_count)
{
	m_wv_space_count = wv_space_count;
}

#endif // DSM_FILEHEADER_HH
