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
#include <ImportXRIT.h>

namespace tut {

struct importxrit_shar
{
	importxrit_shar()
	{
	}

	~importxrit_shar()
	{
	}
};
TESTGRP(importxrit);

// Import a full XRIT product
template<> template<>
void to::test<1>()
{
	XRITImportOptions opts;
	opts.directory = "data";
	opts.resolution = "H";
	opts.productid1 = "MSG1";
	opts.productid2 = "HRV";
	opts.timing = "200604261945";
	opts.subarea = false;
	//int AreaLinStart, AreaNlin, AreaPixStart, AreaNpix;

	std::auto_ptr<ImageData> img = importXRIT(opts);

	gen_ensure_equals(img->name, ""); // unverified
	gen_ensure_equals(img->columns, 5568);
	gen_ensure_equals(img->lines, 11136);
	gen_ensure_equals(img->slope, 1);
	gen_ensure_equals(img->offset, 0);
	gen_ensure_equals(img->year, 2006);
	gen_ensure_equals(img->month, 4);
	gen_ensure_equals(img->day, 26);
	gen_ensure_equals(img->hour, 19);
	gen_ensure_equals(img->minute, 45);
	gen_ensure_equals(img->projection, "");
	gen_ensure_equals(img->channel_id, 12);
	gen_ensure_equals(img->spacecraft_id, 321); // unverified
	gen_ensure_equals(img->column_factor, -40927014);
	gen_ensure_equals(img->line_factor, -40927014);
	gen_ensure_equals(img->column_offset, -2);
	gen_ensure_equals(img->line_offset, 5566);
	gen_ensure_equals(img->bpp, 32); // unverified
	gen_ensure_equals(img->unscaled(0, 0), 0); // unverified
	gen_ensure_equals(img->unscaled(10, 10), 0); // unverified
	gen_ensure_equals(img->scaled(0, 0), 0); // unverified
	gen_ensure_equals(img->scaled(10, 10), 0); // unverified
}

// Import a subarea of an XRIT product
template<> template<>
void to::test<2>()
{
	XRITImportOptions opts;
	opts.directory = "data";
	opts.resolution = "H";
	opts.productid1 = "MSG1";
	opts.productid2 = "HRV";
	opts.timing = "200604261945";
	opts.subarea = true;
	opts.AreaLinStart = 2000;
	opts.AreaNlin = 300;
	opts.AreaPixStart = 1000;
	opts.AreaNpix = 400;

	std::auto_ptr<ImageData> img = importXRIT(opts);

	gen_ensure_equals(img->name, ""); // unverified
	gen_ensure_equals(img->columns, 400);
	gen_ensure_equals(img->lines, 300);
	gen_ensure_equals(img->slope, 1);
	gen_ensure_equals(img->offset, 0);
	gen_ensure_equals(img->year, 2006);
	gen_ensure_equals(img->month, 4);
	gen_ensure_equals(img->day, 26);
	gen_ensure_equals(img->hour, 19);
	gen_ensure_equals(img->minute, 45);
	gen_ensure_equals(img->projection, "");
	gen_ensure_equals(img->channel_id, 12);
	gen_ensure_equals(img->spacecraft_id, 321); // unverified
	gen_ensure_equals(img->column_factor, -40927014);
	gen_ensure_equals(img->line_factor, -40927014);
	gen_ensure_equals(img->column_offset, -2);
	gen_ensure_equals(img->line_offset, 5566);
	gen_ensure_equals(img->bpp, 32); // unverified
	gen_ensure_equals(img->unscaled(0, 0), 0); // unverified
	gen_ensure_equals(img->unscaled(10, 10), 0); // unverified
	gen_ensure_equals(img->scaled(0, 0), 0); // unverified
	gen_ensure_equals(img->scaled(10, 10), 0); // unverified
}

}

/* vim:set ts=4 sw=4: */
