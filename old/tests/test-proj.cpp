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
#include <msat/facts.h>
#include <proj/Geos.h>
#include <proj/Mercator.h>
#include <proj/Polar.h>

using namespace msat;

namespace tut {

struct proj_shar
{
	proj_shar()
	{
	}

	~proj_shar()
	{
	}
};
TESTGRP(proj);

// Test the Geos projection mapping functions
template<> template<>
void to::test<1>()
{
	using namespace msat::proj;

	Geos g(0.0, ORBIT_RADIUS);
	gen_ensure_equals(g.sublon, 0.0);
	gen_ensure_equals(g.orbitRadius, ORBIT_RADIUS);

	ProjectedPoint p;
	g.mapToProjected(MapPoint(45.0, 13.0), p);
	gen_ensure_similar(p.x,  1.54158, 0.00001);
	gen_ensure_similar(p.y, -6.77408, 0.00001);

	MapPoint m;
	g.projectedToMap(p, m);
	gen_ensure_similar(m.lat, 45.0, 0.00001);
	gen_ensure_similar(m.lon, 13.0, 0.00001);
}

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

}

/* vim:set ts=4 sw=4: */
