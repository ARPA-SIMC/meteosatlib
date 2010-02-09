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

}

/* vim:set ts=4 sw=4: */
