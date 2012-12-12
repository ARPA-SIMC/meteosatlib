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

#define TESTFILE "MSG_Seviri_1_5_Infrared_9_7_channel_20060426_1945.grb"

struct msat_georef_grib_shar
{
	GDALDataset* ds;
	GeoReferencer* gr;

	msat_georef_grib_shar() : ds(0), gr(0)
	{
	}

	~msat_georef_grib_shar()
	{
		if (ds) delete ds;
		if (gr) delete gr;
	}

	void start()
        {
                FOR_DRIVER("MsatGRIB");
                if (!ds) ds = openro(TESTFILE).release();
                if (!gr) gr = new GeoReferencer(ds);
        }
};
TESTGRP(msat_georef_grib);

// Test the subsatellite point
template<> template<>
void to::test<1>()
{
        start();
        double lat, lon, px, py;
        int x, y;

        // Middle point maps to 0
        gr->pixelToProjected(1856-1500, 1856-200, px, py);
        gen_ensure_similar(px, 0, 0.000001);
        gen_ensure_similar(py, 0, 0.000001);

        gr->projectedToLatlon(0, 0, lat, lon);
        gen_ensure_equals(lat, 0);
        gen_ensure_equals(lon, 0);

        gr->latlonToProjected(0, 0, px, py);
        gen_ensure_equals(px, 0);
        gen_ensure_equals(py, 0);

        gr->projectedToPixel(0, 0, x, y);
        gen_ensure_equals(x, 1856-1500);
        gen_ensure_equals(y, 1856-200);

        gr->pixelToLatlon(1856-1500, 1856-200, lat, lon);
        gen_ensure_similar(lat, 0, 0.000001);
        gen_ensure_similar(lon, 0, 0.000001);

        gr->latlonToPixel(0, 0, x, y);
        gen_ensure_equals(x, 1856-1500);
	gen_ensure_equals(y, 1856-200);
}

// Test known points
template<> template<>
void to::test<2>()
{
        start();
	double lat, lon;

        gr->pixelToLatlon(0, 0, lat, lon);
        gen_ensure_similar(lat,  59.482613, 0.0001);
        gen_ensure_similar(lon, -21.091290, 0.0001);

        gr->pixelToLatlon(10, 10, lat, lon);
        gen_ensure_similar(lat,  58.659716, 0.0001);
        gen_ensure_similar(lon, -19.914807, 0.0001);

        gr->pixelToLatlon(100, 100, lat, lon);
        gen_ensure_similar(lat,  52.458281, 0.0001);
        gen_ensure_similar(lon, -12.205514, 0.0001);

        gr->pixelToLatlon(1000, 1000, lat, lon);
        gen_ensure_similar(lat, 18.485677, 0.0001);
        gen_ensure_similar(lon, 18.995697, 0.0001);

        gr->pixelToLatlon(1000, 100, lat, lon);
        gen_ensure_similar(lat, 53.906167, 0.0001);
        gen_ensure_similar(lon, 34.082836, 0.0001);

        gr->pixelToLatlon(300, 100, lat, lon);
        gen_ensure_similar(lat, 52.243525, 0.0001);
        gen_ensure_similar(lon, -2.630467, 0.0001);

        gr->pixelToLatlon(100, 300, lat, lon);
        gen_ensure_similar(lat, 42.487134, 0.0001);
	gen_ensure_similar(lon, -9.845648, 0.0001);
}

// Test known points
template<> template<>
void to::test<3>()
{
        start();
	int x, y;
        gr->latlonToPixel(40, 10, x, y);
        gen_ensure_equals(x, 627);
        gen_ensure_equals(y, 359);

        gr->latlonToPixel(10, 10, x, y);
        gen_ensure_equals(x, 718);
        gen_ensure_equals(y, 1292);

        gr->latlonToPixel(10, 40, x, y);
        gen_ensure_equals(x, 1640);
        gen_ensure_equals(y, 1307);

        gr->latlonToPixel(40, -10, x, y);
	gen_ensure_equals(x, 85);
	gen_ensure_equals(y, 359);
}

}

/* vim:set ts=4 sw=4: */
