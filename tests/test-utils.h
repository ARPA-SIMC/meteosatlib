/*
 * test-utils - Test utils for meteosatlib GDAL bindings
 *
 * Copyright (C) 2005--2012  ARPA-SIM <urpsim@smr.arpa.emr.it>
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
#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include "tut.h"
#include <msat/gdal/const.h>
#include <msat/gdal/dataset.h>
#include <gdal_priv.h>
#include <msat/facts.h>
#include <memory>
#include <cstdlib>

#define TESTGRP(name) \
typedef test_group<name ## _shar> tg; \
typedef tg::object to; \
tg name ## _tg (#name);

/*
#define MSAT_WKT \
    "PROJCS[\"unnamed\"," \
    "GEOGCS[\"unnamed\"," \
        "DATUM[\"unknown\"," \
            "SPHEROID[\"unnamed\",6378169,295.488065897]]," \
        "PRIMEM[\"Greenwich\",0]," \
        "UNIT[\"degree\",0.0174532925199433]]," \
    "PROJECTION[\"Geostationary_Satellite\"]," \
    "PARAMETER[\"central_meridian\",0]," \
    "PARAMETER[\"satellite_height\",35785831]," \
    "PARAMETER[\"false_easting\",0]," \
    "PARAMETER[\"false_northing\",0]]"
//#define MSAT_GEOTRANS { x, METEOSAT_PIXELSIZE_X, 0, x, 0, -METEOSAT_PIXELSIZE_Y }
#define MSAT_GEOTRANS { 0, METEOSAT_PIXELSIZE_X, 0, 0, 0, -METEOSAT_PIXELSIZE_Y }
*/

namespace tut {

// Set and unset an extra string marker to be printed in error messages
void test_tag(const std::string& tag);
void test_untag();

#define CHECKED(...) if (__VA_ARGS__ != DBA_OK) throw DBAException(__FILE__, __LINE__)
#define INNER_CHECKED(...) if (__VA_ARGS__ != DBA_OK) throw DBAException(file, line)

std::string __ensure_errmsg(std::string f, int l, std::string msg);
#define gen_ensure(x) ensure (__ensure_errmsg(__FILE__, __LINE__, #x).c_str(), (x))
#define inner_ensure(x) ensure (__ensure_errmsg(file, line, #x).c_str(), (x))

template <class T,class Q>
void my_ensure_equals(const char* file, int line, const Q& actual, const T& expected)
{
	if( expected != actual )
	{
		std::stringstream ss;
		ss << "expected " << expected << " actual " << actual;
		throw failure(__ensure_errmsg(file, line, ss.str()));
	}
}
#define gen_ensure_equals(x, y) my_ensure_equals(__FILE__, __LINE__, (x), (y))
#define inner_ensure_equals(x, y) my_ensure_equals(file, line, (x), (y))

template <class T,class Q,class R>
void my_ensure_similar(const char* file, int line, const Q& actual, const T& expected, const R& delta)
{
	if( actual < expected - delta || expected + delta < actual )
	{
		std::stringstream ss;
		ss << "expected " << expected << " actual " << actual;
		throw failure(__ensure_errmsg(file, line, ss.str()));
	}
}
#define gen_ensure_similar(x, y, delta) my_ensure_similar(__FILE__, __LINE__, (x), (y), (delta))
#define inner_ensure_similar(x, y, delta) my_ensure_similar(file, line, (x), (y), (delta))

#if 0
void my_ensure_imagedata_similar(const char* file, int line, const msat::ImageData& actual, const msat::ImageData& expected, const float& delta);

#define gen_ensure_imagedata_similar(x, y, delta) my_ensure_imagedata_similar(__FILE__, __LINE__, (x), (y), (delta))
#define inner_ensure_imagedata_similar(x, y, delta) my_ensure_imagedata_similar(file, line, (x), (y), (delta))
#endif

class LocalEnv
{
	std::string key;
	std::string oldVal;
public:
	LocalEnv(const std::string& key, const std::string& val)
		: key(key)
	{
		const char* v = getenv(key.c_str());
		oldVal = v == NULL ? "" : v;
		setenv(key.c_str(), val.c_str(), 1);
	}
	~LocalEnv()
	{
		setenv(key.c_str(), oldVal.c_str(), 1);
	}
};

// RAII-style class to ensure cleanup of a temporary test file
class TempTestFile
{
	std::string pathname;
	bool leave;
public:
	TempTestFile(bool leave = false);
	TempTestFile(const std::string& pathname, bool leave = false) : pathname(pathname), leave(leave) { unlink(pathname.c_str()); }
	~TempTestFile() { if (!leave) unlink(pathname.c_str()); }

	const std::string& name() const { return pathname; }
};

}
#endif
