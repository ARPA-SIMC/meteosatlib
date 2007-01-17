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
#include <math.h>

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
	gen_ensure_equals(Image::seviriDXFromColumnRes(13642337*exp2(-16)), 3622); // Test with the real parameters
	gen_ensure_equals(Image::seviriDYFromLineRes(13642337*exp2(-16)), 3622); // Test with the real parameters

	// This computation is necessarily approximated, as we truncate significant digits
	gen_ensure_similar(Image::columnResFromSeviriDX(3622), 13641224*exp2(-16), 0.000001);
	gen_ensure_similar(Image::lineResFromSeviriDY(3622), 13641224*exp2(-16), 0.000001);

	// This computation is necessarily approximated, as we truncate significant digits
	gen_ensure_similar(Image::columnResFromSeviriDX(Image::seviriDXFromColumnRes(13642337*exp2(-16))), 13641224*exp2(-16), 0.000001);
	gen_ensure_similar(Image::lineResFromSeviriDY(Image::seviriDYFromLineRes(13642337*exp2(-16))), 13641224*exp2(-16), 0.000001);

	// This computation should be exact
	gen_ensure_equals(Image::seviriDXFromColumnRes(Image::columnResFromSeviriDX(3622)), 3622);
	gen_ensure_equals(Image::seviriDXFromColumnRes(Image::columnResFromSeviriDX(3622)), 3622);
}

// Test coordinates to point conversion
template<> template<>
void to::test<2>()
{
	Image img;
	img.x0 = 0;
	img.y0 = 0;
	img.column_res = 40927014*exp2(-16);
	img.line_res = 40927014*exp2(-16);
	img.column_offset = 5566;
	img.line_offset = 5566;
	img.proj.reset(new proj::Geos(0.0, ORBIT_RADIUS));
	int x, y;
	img.coordsToPixels(45, 10, x, y);
	gen_ensure_equals(x, 6310);
	gen_ensure_equals(y, 1330);

	double nlat, nlon;
	img.pixelsToCoords(x, y, nlat, nlon);
	gen_ensure_similar(nlat, 45.0, 0.01);
	gen_ensure_similar(nlon, 10.0, 0.01);

#if 0
	for (double la = -90; la < 90; la += 20)
		for (double lo = -180; lo < 180; lo += 40)
		{
			img.coordsToPixels(la, lo, x, y);
			img.pixelsToCoords(x, y, nlat, nlon);
			gen_ensure_similar(nlat, la, 0.01);
			gen_ensure_similar(nlon, lo, 0.01);
		}
#endif
}

}

/* vim:set ts=4 sw=4: */
