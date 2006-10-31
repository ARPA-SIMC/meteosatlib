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
#include <Image.h>
#include <proj/const.h>
#include <proj/Geos.h>

using namespace msat;

namespace tut {

struct image_shar
{
	image_shar()
	{
	}

	~image_shar()
	{
	}
};
TESTGRP(image);

// Test the column factor/seviri DX/DY functions
template<> template<>
void to::test<1>()
{
	// This computation should be exact
	gen_ensure_equals(Image::seviriDXFromColumnFactor(13642337), 3622); // Test with the real parameters
	gen_ensure_equals(Image::seviriDYFromLineFactor(13642337), 3622); // Test with the real parameters

	// This computation is necessarily approximated, as we truncate significant digits
	gen_ensure_equals(Image::columnFactorFromSeviriDX(3622), 13641224);
	gen_ensure_equals(Image::lineFactorFromSeviriDY(3622), 13641224);

	// This computation is necessarily approximated, as we truncate significant digits
	gen_ensure_equals(Image::columnFactorFromSeviriDX(Image::seviriDXFromColumnFactor(13642337)), 13641224);
	gen_ensure_equals(Image::lineFactorFromSeviriDY(Image::seviriDYFromLineFactor(13642337)), 13641224);

	// This computation should be exact
	gen_ensure_equals(Image::seviriDXFromColumnFactor(Image::columnFactorFromSeviriDX(3622)), 3622);
	gen_ensure_equals(Image::seviriDXFromColumnFactor(Image::columnFactorFromSeviriDX(3622)), 3622);
}

// Test coordinates to point conversion
template<> template<>
void to::test<2>()
{
	Image img;
	img.column_factor = 13642337;
	img.line_factor = 13642337;
	img.column_offset = 1500;
	img.line_offset = 200;
	img.proj.reset(new proj::Geos(0.0, ORBIT_RADIUS));
	int x, y;
	img.coordsToPixels(45, 10, x, y);
	gen_ensure_equals(x, 1747);
	gen_ensure_equals(y, -1213);

#if 0
	// Print some points to see how the mapping goes
	for (int la = -90; la < 90; la += 20)
		for (int lo = -180; lo < 180; lo += 40)
		{
			using namespace std;
			img.coordsToPixels(la, lo, x, y);
			cout << x << ' ' << y << endl;
		}
#endif
}

}

/* vim:set ts=4 sw=4: */
