/*
 * DB-ALLe - Archive for punctual meteorological data
 *
 * Copyright (C) 2005,2006  ARPA-SIM <urpsim@smr.arpa.emr.it>
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

#include "tut.h"
#include <conv/Image.h>

#define TESTGRP(name) \
typedef test_group<name ## _shar> tg; \
typedef tg::object to; \
tg name ## _tg (#name);

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

std::auto_ptr<msat::Image> recodeThroughGrib(msat::Image& img, bool leaveFile = false);

}
