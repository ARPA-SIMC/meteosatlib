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
 * OpenMTP-IDS - Open Meteosat Transition Programme -- ISSCP Data Set
 *
 * Reference:
 *   The Meteosat Archive
 *   EUM.FG.3: Format Guide 3 - IDS in OpenMTP
 *   Revision 1.1
 *   January 1998
 *   http://www.eumetsat.int
 */
#ifndef DSM_OPENMTP_IDS_HH
#define DSM_OPENMTP_IDS_HH

// SYSTEM INCLUDES
//
#include <iosfwd>
#include <vector>

// PROJECT INCLUDES
//
#include <omtp-ids/FileHeader.hh>
#include <omtp-ids/Record.hh>

// LOCAL INCLUDES
//

// FORWARD REFERENCES
//

// *********************************************************************

class OpenMTP_IDS
{
private:

protected:

	FileHeader m_fileheader;
	std::vector<Record> m_record;

public:

	// LIFECYCLE

	OpenMTP_IDS();
	OpenMTP_IDS(const OpenMTP_IDS& openmtp_ids);
	OpenMTP_IDS(const char* filename);

	// OPERATORS

	friend std::ostream& operator<<(std::ostream& os,
					const OpenMTP_IDS& openmtp_ids);
	friend std::istream& operator>>(std::istream& is,
					OpenMTP_IDS& openmtp_ids);

	// OPERATIONS

	bool read(const char* filename);
	bool write(const char* filename);

	std::ostream& debug(std::ostream& os) const;

	// INQUIRY

	// ACCESS

	// get
	const FileHeader& fileheader() const;
	FileHeader& fileheader();
	const std::vector<Record>& record() const;
	std::vector<Record>& record();

	// set
	void fileheader(const FileHeader& fileheader);
	void record(const std::vector<Record>& record);

	std::vector<ScanLine>
		scanline() const;
	std::vector<ScanLine>
		scanline(const unsigned line) const;
	std::vector<ScanLine>
		scanline(const unsigned from,
			 const unsigned to) const;

};

// EXTERNAL REFERENCES
//

inline
OpenMTP_IDS::OpenMTP_IDS()
{
	// empty
}

inline
OpenMTP_IDS::OpenMTP_IDS(const OpenMTP_IDS& openmtp_ids)
{
	this->operator=(openmtp_ids);
}

inline
OpenMTP_IDS::OpenMTP_IDS(const char* filename)
{
	if (!read(filename)) {
		throw "read failure";
	}
}

inline
const FileHeader&
OpenMTP_IDS::fileheader() const
{
	return m_fileheader;
}

inline
FileHeader&
OpenMTP_IDS::fileheader()
{
	return m_fileheader;
}

inline
const std::vector<Record>&
OpenMTP_IDS::record() const
{
	return m_record;
}

inline
std::vector<Record>&
OpenMTP_IDS::record()
{
	return m_record;
}

inline
void
OpenMTP_IDS::fileheader(const FileHeader& fileheader)
{
	m_fileheader = fileheader;
}

inline
void
OpenMTP_IDS::record(const std::vector<Record>& record)
{
	m_record = record;
}

#endif // DSM_OPENMTP_IDS_HH
