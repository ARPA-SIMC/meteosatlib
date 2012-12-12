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

#define TESTFILE "H:MSG1:HRV:200611141200"

struct msat_georef_xrithrv_shar
{
	GDALDataset* ds;
	GeoReferencer* gr;

	msat_georef_xrithrv_shar() : ds(0), gr(0)
	{
	
	}

	~msat_georef_xrithrv_shar()
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
TESTGRP(msat_georef_xrithrv);

// Test the subsatellite point
template<> template<>
void to::test<1>()
{
        start();
	double lat, lon, px, py;
	int x, y;

        // Middle point maps to 0
        gr->pixelToProjected(5568, 5568, px, py);
        gen_ensure_equals(px, 0);
        gen_ensure_equals(py, 0);

        gr->projectedToLatlon(0, 0, lat, lon);
        gen_ensure_equals(lat, 0);
        gen_ensure_equals(lon, 0);

        gr->latlonToProjected(0, 0, px, py);
        gen_ensure_equals(px, 0);
        gen_ensure_equals(py, 0);

        gr->projectedToPixel(0, 0, x, y);
        gen_ensure_equals(x, 5568);
        gen_ensure_equals(y, 5568);

        gr->pixelToLatlon(5568, 5568, lat, lon);
        gen_ensure_equals(lat, 0);
        gen_ensure_equals(lon, 0);

        gr->latlonToPixel(0, 0, x, y);
	gen_ensure_equals(x, 5568);
	gen_ensure_equals(y, 5568);
}

// Test known points
template<> template<>
void to::test<2>()
{
        start();
	double lat, lon;

        gr->pixelToLatlon(6415, 1365, lat, lon);
	gen_ensure_similar(lat, 44.5273, 0.0001);
	gen_ensure_similar(lon, 11.3008, 0.0001);

	gr->pixelToLatlon(6651,4473, lat, lon);
	gen_ensure_similar(lat, 10.01540, 0.0001);
	gen_ensure_similar(lon,  9.98602, 0.0001);

	gr->pixelToLatlon(4481,6659, lat, lon);
	gen_ensure_similar(lat,  -9.97845, 0.0001);
	gen_ensure_similar(lon, -10.02220, 0.0001);

	gr->pixelToLatlon(2723,9932, lat, lon);
	gen_ensure_similar(lat, -49.9556, 0.0001);
	gen_ensure_similar(lon, -49.9816, 0.0001);

	gr->pixelToLatlon(8409,9932, lat, lon);
	gen_ensure_similar(lat, -49.9414, 0.0001);
	gen_ensure_similar(lon,  49.8518, 0.0001);
}

// Test known points
template<> template<>
void to::test<3>()
{
        start();
	int x, y;

        gr->latlonToPixel(44.4949, 11.3211, x, y);
	gen_ensure_equals(x, 6417);
	gen_ensure_equals(y, 1367);

        gr->latlonToPixel(10, 10, x, y);
        gen_ensure_equals(x, 6653);
        gen_ensure_equals(y, 4475);

        gr->latlonToPixel(-10, -10, x, y);
        gen_ensure_equals(x, 4483);
        gen_ensure_equals(y, 6661);

        gr->latlonToPixel(10, -10, x, y);
        gen_ensure_equals(x, 4483);
        gen_ensure_equals(y, 4475);

        gr->latlonToPixel(-10, 10, x, y);
        gen_ensure_equals(x, 6653);
        gen_ensure_equals(y, 6661);

        gr->latlonToPixel(-50, -50, x, y);
        gen_ensure_equals(x, 2725);
        gen_ensure_equals(y, 9934);

        gr->latlonToPixel(50, -50, x, y);
        gen_ensure_equals(x, 2725);
        gen_ensure_equals(y, 1202);

        gr->latlonToPixel(-50, 50, x, y);
        gen_ensure_equals(x, 8411);
        gen_ensure_equals(y, 9934);

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

#if 0
// Test the Mercator projection mapping functions
template<> template<>
void to::test<2>()
{
	using namespace msat::proj;

	Mercator proj;

	ProjectedPoint p;
	proj.mapToProjected(MapPoint(45.0, 13.0), p);
	gen_ensure_similar(p.x, 0.0722222, 0.00001);
	gen_ensure_similar(p.y, -0.28055, 0.00001);

	MapPoint m;
	proj.projectedToMap(p, m);
	gen_ensure_similar(m.lat, 45.0, 0.00001);
	gen_ensure_similar(m.lon, 13.0, 0.00001);
}

// Test the Polar projection mapping functions
template<> template<>
void to::test<3>()
{
	using namespace msat::proj;

	Polar proj(10, true);

	ProjectedPoint p;
	proj.mapToProjected(MapPoint(45.0, 13.0), p);
	gen_ensure_similar(p.x, 0.0216783, 0.00001);
	gen_ensure_similar(p.y, 0.413646, 0.00001);

	MapPoint m;
	proj.projectedToMap(p, m);
	gen_ensure_similar(m.lat, 45.0, 0.00001);
	gen_ensure_similar(m.lon, 13.0, 0.00001);
}
#endif

}

/* vim:set ts=4 sw=4: */
