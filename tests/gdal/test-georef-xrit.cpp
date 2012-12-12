/*
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

namespace tut {

#define TESTFILE "H:MSG2:VIS006:200807150900"

struct msat_georef_hrit_shar
{
	GDALDataset* ds;
	GeoReferencer* gr;

	msat_georef_hrit_shar() : ds(0), gr(0)
	{
	}

	~msat_georef_hrit_shar()
	{
		if (ds) delete ds;
		if (gr) delete gr;
	}

	void start()
        {
                FOR_DRIVER("MsatXRIT");
                if (!ds) ds = openro(TESTFILE).release();
                if (!gr) gr = new GeoReferencer(ds);
        }
};
TESTGRP(msat_georef_hrit);

// Test the subsatellite point
template<> template<>
void to::test<1>()
{
        start();
	double lat, lon, px, py;
	int x, y;

	// Middle point maps to 0
	gr->pixelToProjected(1856, 1856, px, py);
	gen_ensure_equals(px, 0);
        gen_ensure_equals(py, 0);

        gr->projectedToLatlon(0, 0, lat, lon);
        gen_ensure_equals(lat, 0);
        gen_ensure_equals(lon, 0);

        gr->latlonToProjected(0, 0, px, py);
        gen_ensure_equals(px, 0);
        gen_ensure_equals(py, 0);

        gr->projectedToPixel(0, 0, x, y);
        gen_ensure_equals(x, 1856);
        gen_ensure_equals(y, 1856);

        gr->pixelToLatlon(1856, 1856, lat, lon);
        gen_ensure_equals(lat, 0);
        gen_ensure_equals(lon, 0);

        gr->latlonToPixel(0, 0, x, y);
	gen_ensure_equals(x, 1856);
	gen_ensure_equals(y, 1856);
}

// Test known points
template<> template<>
void to::test<2>()
{
        start();
	double lat, lon;

        gr->pixelToLatlon(2139, 456, lat, lon);
	gen_ensure_similar(lat, 44.4821, 0.0001);
	gen_ensure_similar(lon, 11.3180, 0.0001);

	gr->pixelToLatlon(2218,1492, lat, lon);
	gen_ensure_similar(lat,  9.987689, 0.0001);
	gen_ensure_similar(lon, 10.013122, 0.0001);

	gr->pixelToLatlon(1494,2220, lat, lon);
	gen_ensure_similar(lat,  -9.987689, 0.0001);
	gen_ensure_similar(lon, -10.013122, 0.0001);

	gr->pixelToLatlon(908, 3311, lat, lon);
	gen_ensure_similar(lat, -49.973000, 0.0001);
	gen_ensure_similar(lon, -49.984040, 0.0001);

	gr->pixelToLatlon(2804,3311, lat, lon);
	gen_ensure_similar(lat, -49.973000, 0.0001);
	gen_ensure_similar(lon,  49.984040, 0.0001);
}

// Test known points
template<> template<>
void to::test<3>()
{
        start();
	int x, y;

        gr->latlonToPixel(44.4949, 11.3211, x, y);
        gen_ensure_equals(x, 2139);
        gen_ensure_equals(y, 456);

        gr->latlonToPixel(10, 10, x, y);
        gen_ensure_equals(x, 2218);
        gen_ensure_equals(y, 1492);

        gr->latlonToPixel(-10, -10, x, y);
        gen_ensure_equals(x, 1494);
        gen_ensure_equals(y, 2220);

        gr->latlonToPixel(10, -10, x, y);
        gen_ensure_equals(x, 1494);
        gen_ensure_equals(y, 1492);

        gr->latlonToPixel(-10, 10, x, y);
        gen_ensure_equals(x, 2218);
        gen_ensure_equals(y, 2220);

        gr->latlonToPixel(-50, -50, x, y);
        gen_ensure_equals(x, 908);
        gen_ensure_equals(y, 3311);

        gr->latlonToPixel(50, -50, x, y);
        gen_ensure_equals(x, 908);
        gen_ensure_equals(y, 401);

        gr->latlonToPixel(-50, 50, x, y);
        gen_ensure_equals(x, 2804);
	gen_ensure_equals(y, 3311);

// (50.000000,50.000000) -> (8409,1200)
// (80.000000,80.000000) -> (6499,271)
// (-80.000000,80.000000) -> (6499,10861)
// (80.000000,-80.000000) -> (4633,271)

#if 0
	gr->pixelToLatlon(4633,10861, lat, lon);
	gen_ensure_similar(lat, -71.679389, 0.0001);
	gen_ensure_similar(lon, -31.643353, 0.0001);
    gr->latlonToPixel(-80, -80, x, y);
	gen_ensure_equals(x, 4633);
	gen_ensure_equals(y, 10861);
#endif
}

}

/* vim:set ts=4 sw=4: */
