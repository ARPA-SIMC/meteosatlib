/*
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

#include "test-utils.h"
#include <msat/facts.h>
#include <math.h>

using namespace msat;

namespace tut {

struct facts_shar
{
	facts_shar()
	{
	}

	~facts_shar()
	{
	}
};
TESTGRP(facts);

// Test the column factor/seviri DX/DY functions
template<> template<>
void to::test<1>()
{
	// This computation should be exact
	gen_ensure_equals(facts::seviriDXFromColumnRes(13642337*exp2(-16)), 3622); // Test with the real parameters
	gen_ensure_equals(facts::seviriDYFromLineRes(13642337*exp2(-16)), 3622); // Test with the real parameters

	// This computation is necessarily approximated, as we truncate significant digits
	gen_ensure_similar(facts::columnResFromSeviriDX(3622), 13641224*exp2(-16), 0.000001);
	gen_ensure_similar(facts::lineResFromSeviriDY(3622), 13641224*exp2(-16), 0.000001);

	// This computation is necessarily approximated, as we truncate significant digits
	gen_ensure_similar(facts::columnResFromSeviriDX(facts::seviriDXFromColumnRes(13642337*exp2(-16))), 13641224*exp2(-16), 0.000001);
	gen_ensure_similar(facts::lineResFromSeviriDY(facts::seviriDYFromLineRes(13642337*exp2(-16))), 13641224*exp2(-16), 0.000001);

	// This computation should be exact
	gen_ensure_equals(facts::seviriDXFromColumnRes(facts::columnResFromSeviriDX(3622)), 3622);
	gen_ensure_equals(facts::seviriDXFromColumnRes(facts::columnResFromSeviriDX(3622)), 3622);
}

// Test the satellite zenith angle calculation (FIXME: data not validated against known values, but seems to make sense)
template<> template<>
void to::test<2>()
{
    gen_ensure_equals(facts::sat_za(0, 0), 0);

    // Move north along the Greenwich meridian
    gen_ensure_equals(round(facts::sat_za(10, 0) / M_PI * 180), 12);
    gen_ensure_equals(round(facts::sat_za(20, 0) / M_PI * 180), 23);
    gen_ensure_equals(round(facts::sat_za(30, 0) / M_PI * 180), 35);
    gen_ensure_equals(round(facts::sat_za(40, 0) / M_PI * 180), 46);
    gen_ensure_equals(round(facts::sat_za(50, 0) / M_PI * 180), 57);
    gen_ensure_equals(round(facts::sat_za(60, 0) / M_PI * 180), 68);
    gen_ensure_equals(round(facts::sat_za(70, 0) / M_PI * 180), 79);
    gen_ensure_equals(round(facts::sat_za(80, 0) / M_PI * 180), 89);

    // Move south along the Greenwich meridian
    gen_ensure_equals(round(facts::sat_za(-10, 0) / M_PI * 180), 12);
    gen_ensure_equals(round(facts::sat_za(-20, 0) / M_PI * 180), 23);
    gen_ensure_equals(round(facts::sat_za(-30, 0) / M_PI * 180), 35);
    gen_ensure_equals(round(facts::sat_za(-40, 0) / M_PI * 180), 46);
    gen_ensure_equals(round(facts::sat_za(-50, 0) / M_PI * 180), 57);
    gen_ensure_equals(round(facts::sat_za(-60, 0) / M_PI * 180), 68);
    gen_ensure_equals(round(facts::sat_za(-70, 0) / M_PI * 180), 79);
    gen_ensure_equals(round(facts::sat_za(-80, 0) / M_PI * 180), 89);

    // Move east along the equator
    gen_ensure_equals(round(facts::sat_za(0, 10) / M_PI * 180), 12);
    gen_ensure_equals(round(facts::sat_za(0, 20) / M_PI * 180), 23);
    gen_ensure_equals(round(facts::sat_za(0, 30) / M_PI * 180), 35);
    gen_ensure_equals(round(facts::sat_za(0, 40) / M_PI * 180), 46);
    gen_ensure_equals(round(facts::sat_za(0, 50) / M_PI * 180), 57);

    // Move diagonally
    gen_ensure_equals(round(facts::sat_za(10, 10) / M_PI * 180), 17);
    gen_ensure_equals(round(facts::sat_za(20, 20) / M_PI * 180), 33);
    gen_ensure_equals(round(facts::sat_za(30, 30) / M_PI * 180), 48);
    gen_ensure_equals(round(facts::sat_za(40, 40) / M_PI * 180), 62);
    gen_ensure_equals(round(facts::sat_za(50, 50) / M_PI * 180), 74);
}

}

/* vim:set ts=4 sw=4: */
