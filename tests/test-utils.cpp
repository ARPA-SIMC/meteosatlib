/*
 * test-utils - Test utils for meteosatlib GDAL bindings
 *
 * Copyright (C) 2005--2010  ARPA-SIM <urpsim@smr.arpa.emr.it>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 *
 * Author: Enrico Zini <enrico@enricozini.com>
 */

#include "test-utils.h"

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <gdal_priv.h>
#include <ogr_spatialref.h>
#include <msat/gdal/dataset.h>
/*
#include <msat/gdalutils.h>
#include <msat/GRIB.h>
#include <msat/XRIT.h>
#include <msat/NetCDF.h>
#include <msat/NetCDF24.h>
#include <msat/SAFH5.h>
*/
#include <stdexcept>

using namespace std;

namespace tut {

static std::string tag;

void test_tag(const std::string& ttag)
{
	tag = ttag;
}

void test_untag()
{
	tag = std::string();
}

std::string __ensure_errmsg(std::string file, int line, std::string msg)
{
	std::stringstream ss;
	ss << file << ":" << line << ": ";
	if (!tag.empty())
		ss << "[" << tag << "] ";
	ss << "'" << msg << "'";
	return ss.str();
}

TempTestFile::TempTestFile(bool leave) : leave(leave)
{
	char* pn = tempnam(".", "test");
	pathname = pn;
	free(pn);
}

}
