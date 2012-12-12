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

#define TESTFILE "MSG_Seviri_1_5_Infrared_10_8_channel_20051219_1415.nc"

struct msat_georef_netcdf_shar
{
	GDALDataset* ds;
	GeoReferencer* gr;

	msat_georef_netcdf_shar() : ds(0), gr(0)
	{
		ds = openro(TESTFILE).release();
		gr = new GeoReferencer(ds);
	}

	~msat_georef_netcdf_shar()
	{
		if (ds) delete ds;
		if (gr) delete gr;
	}
};
TESTGRP(msat_georef_netcdf);

// Test the subsatellite point
template<> template<>
void to::test<1>()
{
	double lat, lon, px, py;
	int x, y;

	// Middle point maps to 0
        gr->pixelToProjected(1856-1499, 1856-199, px, py);
        gen_ensure_similar(px, 0, 0.000000001);
        gen_ensure_similar(py, 0, 0.000000001);

        gr->projectedToLatlon(0, 0, lat, lon);
        gen_ensure_equals(lat, 0);
        gen_ensure_equals(lon, 0);

        gr->latlonToProjected(0, 0, px, py);
        gen_ensure_equals(px, 0);
        gen_ensure_equals(py, 0);

        gr->projectedToPixel(0, 0, x, y);
        gen_ensure_equals(x, 1856-1499);
        gen_ensure_equals(y, 1856-199);

        gr->pixelToLatlon(1856-1499, 1856-199, lat, lon);
        gen_ensure_similar(lat, 0, 0.000000001);
        gen_ensure_similar(lon, 0, 0.000000001);

        gr->latlonToPixel(0, 0, x, y);
	gen_ensure_equals(x, 1856-1499);
	gen_ensure_equals(y, 1856-199);
}

// Test known points
template<> template<>
void to::test<2>()
{
        double lat, lon;

        gr->pixelToLatlon(0, 0, lat, lon);
        gen_ensure_similar(lat,  59.567048, 0.0001);
        gen_ensure_similar(lon, -21.214644, 0.0001);

        gr->pixelToLatlon(10, 10, lat, lon);
        gen_ensure_similar(lat,  58.740333, 0.0001);
        gen_ensure_similar(lon, -20.028070, 0.0001);

        gr->pixelToLatlon(100, 100, lat, lon);
        gen_ensure_similar(lat,  52.518690, 0.0001);
        gen_ensure_similar(lon, -12.273015, 0.0001);

        gr->pixelToLatlon(1000, 1000, lat, lon);
        gen_ensure_similar(lat, 18.514848, 0.0001);
        gen_ensure_similar(lon, 18.968364, 0.0001);

        gr->pixelToLatlon(1000, 100, lat, lon);
        gen_ensure_similar(lat, 53.965508, 0.0001);
        gen_ensure_similar(lon, 34.081831, 0.0001);

        gr->pixelToLatlon(300, 100, lat, lon);
        gen_ensure_similar(lat, 52.301633, 0.0001);
        gen_ensure_similar(lon, -2.681350, 0.0001);

        gr->pixelToLatlon(100, 300, lat, lon);
        gen_ensure_similar(lat, 42.531680, 0.0001);
        gen_ensure_similar(lon, -9.892650, 0.0001);
}

// Test known points
template<> template<>
void to::test<3>()
{
	int x, y;

        gr->latlonToPixel(40, 10, x, y);
        gen_ensure_equals(x, 628);
        gen_ensure_equals(y, 360);

        gr->latlonToPixel(10, 10, x, y);
        gen_ensure_equals(x, 719);
        gen_ensure_equals(y, 1293);

        gr->latlonToPixel(10, 40, x, y);
        gen_ensure_equals(x, 1641);
        gen_ensure_equals(y, 1308);

        gr->latlonToPixel(40, -10, x, y);
	gen_ensure_equals(x, 86);
	gen_ensure_equals(y, 360);
}

}

/* vim:set ts=4 sw=4: */
