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

#include "test-utils.h"

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <cstdio>


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

void my_ensure_imagedata_similar(const char* file, int line, const msat::ImageData& actual, const msat::ImageData& expected, const float& delta)
{
	if (actual.lines != expected.lines)
	{
		std::stringstream ss;
		ss << "Line count differ: expected " << expected.lines << " actual " << actual.lines;
		throw failure(__ensure_errmsg(file, line, ss.str()));
	}
	if (actual.columns != expected.columns)
	{
		std::stringstream ss;
		ss << "Column count differ: expected " << expected.columns << " actual " << actual.columns;
		throw failure(__ensure_errmsg(file, line, ss.str()));
	}
	for (size_t y = 0; y < expected.lines; ++y)
		for (size_t x = 0; x < expected.columns; ++x)
		{
			float v1 = actual.scaled(x,y);
			float v2 = expected.scaled(x,y);
			if (v1 == actual.missingValue && v2 == expected.missingValue)
				continue;
			if (v1 == actual.missingValue && v2 != expected.missingValue)
			{
				std::stringstream ss;
				ss << "at position " << x << "," << y << ": expected "
				   << v2 << " actual missing";
				throw failure(__ensure_errmsg(file, line, ss.str()));
			}
			if (v1 != actual.missingValue && v2 == expected.missingValue)
			{
				std::stringstream ss;
				ss << "at position " << x << "," << y << ": expected missing actual " << v1;
				throw failure(__ensure_errmsg(file, line, ss.str()));
			}
			if (v1 <= v2 - delta || v2 + delta <= v1)
			{
				std::stringstream ss;
				ss << "at position " << x << "," << y << ": expected "
				   << v2 << " actual " << v1;
				throw failure(__ensure_errmsg(file, line, ss.str()));
			}
		}
}

TempTestFile::TempTestFile(bool leave) : leave(leave)
{
	::mkdir(WORK_DIR, 0755);
	char* pn = tempnam(WORK_DIR, "test");
	pathname = pn;
	free(pn);
}

}
