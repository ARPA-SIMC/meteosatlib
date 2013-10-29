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

// Test the solar zenith angle calculation
// FIXME: data not validated against known values, but seems to make sense)
template<> template<>
void to::test<3>()
{
    // At an equinox
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 12, 0, 0, 0), 1, 0.001);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 12, 0, 90, 0), 0, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 12, 0, -90, 0), 0, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 12, 0, 180, 0), -1, 0.001);

    // Move north along the Greenwich meridian
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 12, 0, 10, 0), 0.98, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 12, 0, 20, 0), 0.94, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 12, 0, 30, 0), 0.86, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 12, 0, 40, 0), 0.76, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 12, 0, 45, 0), 0.71, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 12, 0, 50, 0), 0.64, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 12, 0, 60, 0), 0.50, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 12, 0, 70, 0), 0.34, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 12, 0, 80, 0), 0.17, 0.01);

    // Move south along the Greenwich meridian
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 12, 0, -10, 0), 0.98, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 12, 0, -20, 0), 0.94, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 12, 0, -30, 0), 0.86, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 12, 0, -40, 0), 0.76, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 12, 0, -45, 0), 0.71, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 12, 0, -50, 0), 0.64, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 12, 0, -60, 0), 0.50, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 12, 0, -70, 0), 0.34, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 12, 0, -80, 0), 0.17, 0.01);

    // Move east along the equator
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 12, 0, 0,  20), 0.95, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 12, 0, 0,  40), 0.79, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 12, 0, 0,  60), 0.53, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 12, 0, 0,  80), 0.21, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 12, 0, 0,  90), 0.03, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 12, 0, 0, 100), -0.14, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 12, 0, 0, 120), -0.47, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 12, 0, 0, 140), -0.75, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 12, 0, 0, 160), -0.93, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 12, 0, 0, 180), -1, 0.01);

    // Move west along the equator
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 12, 0, 0,  -20), 0.93, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 12, 0, 0,  -40), 0.75, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 12, 0, 0,  -60), 0.47, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 12, 0, 0,  -80), 0.14, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 12, 0, 0,  -90), -0.03, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 12, 0, 0, -100), -0.21, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 12, 0, 0, -120), -0.53, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 12, 0, 0, -140), -0.79, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 12, 0, 0, -160), -0.95, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 12, 0, 0, -180), -1, 0.01);

    // Hours of the day
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 0, 0, 0, 0),  -1   , 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 3, 0, 0, 0),  -0.73, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 6, 0, 0, 0),  -0.03, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 9, 0, 0, 0),   0.68, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 12, 0, 0, 0),  1   , 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 15, 0, 0, 0),  0.73, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 18, 0, 0, 0),  0.03, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 21, 0, 0, 0), -0.68, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 22, 0, 0, 0, 0),  -1   , 0.01);

    // Move east along the equator, at 6UTC
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 6, 0, 0,   0), -0.03, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 6, 0, 0,  20),  0.31, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 6, 0, 0,  40),  0.62, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 6, 0, 0,  60),  0.85, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 6, 0, 0,  80),  0.98, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 6, 0, 0,  90),  1.00, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 6, 0, 0, 100),  0.99, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 6, 0, 0, 120),  0.88, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 6, 0, 0, 140),  0.66, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 6, 0, 0, 160),  0.37, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 6, 0, 0, 180),  0.03, 0.01);

    // Move west along the equator, at 6UTC
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 6, 0, 0,  -20), -0.37, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 6, 0, 0,  -40), -0.66, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 6, 0, 0,  -60), -0.88, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 6, 0, 0,  -80), -0.99, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 6, 0, 0,  -90), -1.00, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 6, 0, 0, -100), -0.98, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 6, 0, 0, -120), -0.85, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 6, 0, 0, -140), -0.62, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 6, 0, 0, -160), -0.31, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 6, 0, 0, -180),  0.03, 0.01);

    // Move south, at 6UTC UTC+6: should be the same as moving south at 12UTC
    // along the Greenwich meridian
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 6, 0, -10, 90), 0.98, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 6, 0, -20, 90), 0.94, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 6, 0, -30, 90), 0.86, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 6, 0, -40, 90), 0.76, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 6, 0, -45, 90), 0.71, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 6, 0, -50, 90), 0.64, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 6, 0, -60, 90), 0.50, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 6, 0, -70, 90), 0.34, 0.01);
    gen_ensure_similar(facts::cos_sol_za(2013, 3, 21, 6, 0, -80, 90), 0.17, 0.01);

}

}

/* vim:set ts=4 sw=4: */
