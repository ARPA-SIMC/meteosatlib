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

using namespace msat;

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

// Test the isXRIT function
template<> template<>
void to::test<1>()
{
	gen_ensure(isXRIT("data/H:MSG1:HRV:200604261945"));
	gen_ensure(!isXRIT("data/H:MSG1:HRV"));
}

// Import a full XRIT product
template<> template<>
void to::test<2>()
{
	XRITImportOptions opts;
	opts.directory = "data";
	opts.resolution = "H";
	opts.productid1 = "MSG1";
	opts.productid2 = "HRV";
	opts.timing = "200604261945";
	opts.subarea = false;
	//int AreaLinStart, AreaNlin, AreaPixStart, AreaNpix;

	std::auto_ptr<Image> img = importXRIT(opts);

	//gen_ensure_equals(img->name, ""); // unverified
	gen_ensure_equals(img->data->columns, 5568);
	gen_ensure_equals(img->data->lines, 11136);
	gen_ensure_equals(img->data->slope, 1);
	gen_ensure_equals(img->data->offset, 0);
	gen_ensure_equals(img->year, 2006);
	gen_ensure_equals(img->month, 4);
	gen_ensure_equals(img->day, 26);
	gen_ensure_equals(img->hour, 19);
	gen_ensure_equals(img->minute, 45);
	gen_ensure_equals(img->sublon, 0);
	gen_ensure_equals(img->channel_id, 12);
	gen_ensure_equals(img->spacecraft_id, 321); // unverified
	gen_ensure_equals(img->column_factor, -40927014);
	gen_ensure_equals(img->line_factor, -40927014);
	gen_ensure_equals(img->column_offset, -2);
	gen_ensure_equals(img->line_offset, 462);
	gen_ensure_equals(img->data->bpp, 32); // unverified
	gen_ensure_equals(img->data->unscaled(0, 0), 0); // unverified
	gen_ensure_equals(img->data->unscaled(10, 10), 0); // unverified
	gen_ensure_equals(img->data->scaled(0, 0), 0); // unverified
	gen_ensure_equals(img->data->scaled(10, 10), 0); // unverified
}

// Import a subarea of an XRIT product
template<> template<>
void to::test<3>()
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

	std::auto_ptr<Image> img = importXRIT(opts);

	//gen_ensure_equals(img->name, ""); // unverified
	gen_ensure_equals(img->data->columns, 400);
	gen_ensure_equals(img->data->lines, 300);
	gen_ensure_equals(img->data->slope, 1);
	gen_ensure_equals(img->data->offset, 0);
	gen_ensure_equals(img->year, 2006);
	gen_ensure_equals(img->month, 4);
	gen_ensure_equals(img->day, 26);
	gen_ensure_equals(img->hour, 19);
	gen_ensure_equals(img->minute, 45);
	gen_ensure_equals(img->sublon, 0);
	gen_ensure_equals(img->channel_id, 12);
	gen_ensure_equals(img->spacecraft_id, 321); // unverified
	gen_ensure_equals(img->column_factor, -40927014);
	gen_ensure_equals(img->line_factor, -40927014);
	gen_ensure_equals(img->column_offset, 998);
	gen_ensure_equals(img->line_offset, 2462);
	gen_ensure_equals(img->data->bpp, 32); // unverified
	gen_ensure_equals(img->data->unscaled(0, 0), 0); // unverified
	gen_ensure_equals(img->data->unscaled(10, 10), 0); // unverified
	gen_ensure_equals(img->data->scaled(0, 0), 0); // unverified
	gen_ensure_equals(img->data->scaled(10, 10), 0); // unverified
}

// Import a full XRIT product and pass it from grib
template<> template<>
void to::test<4>()
{
	XRITImportOptions opts;
	opts.directory = "data";
	opts.resolution = "H";
	opts.productid1 = "MSG1";
	opts.productid2 = "HRV";
	opts.timing = "200604261945";
	opts.subarea = false;
	std::auto_ptr<Image> img = importXRIT(opts);
	img = recodeThroughGrib(*img);

	//gen_ensure_equals(img->name, ""); // unverified
	gen_ensure_equals(img->data->columns, 5568);
	gen_ensure_equals(img->data->lines, 11136);
	gen_ensure_equals(img->data->slope, 1);
	gen_ensure_equals(img->data->offset, 0);
	gen_ensure_equals(img->year, 2006);
	gen_ensure_equals(img->month, 4);
	gen_ensure_equals(img->day, 26);
	gen_ensure_equals(img->hour, 19);
	gen_ensure_equals(img->minute, 45);
	gen_ensure_equals(img->sublon, 0);
	gen_ensure_equals(img->channel_id, 12);
	gen_ensure_equals(img->spacecraft_id, 321); // unverified
	gen_ensure_equals(img->column_factor, -40927014);
	gen_ensure_equals(img->line_factor, -40927014);
	gen_ensure_equals(img->column_offset, -2);
	gen_ensure_equals(img->line_offset, 462);
	gen_ensure_equals(img->data->bpp, 32); // unverified
	gen_ensure_equals(img->data->unscaled(0, 0), 0); // unverified
	gen_ensure_equals(img->data->unscaled(10, 10), 0); // unverified
	gen_ensure_equals(img->data->scaled(0, 0), 0); // unverified
	gen_ensure_equals(img->data->scaled(10, 10), 0); // unverified
}

// Import a subarea of an XRIT product and pass it from grib
template<> template<>
void to::test<5>()
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

	std::auto_ptr<Image> img = importXRIT(opts);
	img = recodeThroughGrib(*img);

	//gen_ensure_equals(img->name, ""); // unverified
	gen_ensure_equals(img->data->columns, 400);
	gen_ensure_equals(img->data->lines, 300);
	gen_ensure_equals(img->data->slope, 1);
	gen_ensure_equals(img->data->offset, 0);
	gen_ensure_equals(img->year, 2006);
	gen_ensure_equals(img->month, 4);
	gen_ensure_equals(img->day, 26);
	gen_ensure_equals(img->hour, 19);
	gen_ensure_equals(img->minute, 45);
	gen_ensure_equals(img->sublon, 0);
	gen_ensure_equals(img->channel_id, 12);
	gen_ensure_equals(img->spacecraft_id, 321); // unverified
	gen_ensure_equals(img->column_factor, -40927014);
	gen_ensure_equals(img->line_factor, -40927014);
	gen_ensure_equals(img->column_offset, 998);
	gen_ensure_equals(img->line_offset, 2462);
	gen_ensure_equals(img->data->bpp, 32); // unverified
	gen_ensure_equals(img->data->unscaled(0, 0), 0); // unverified
	gen_ensure_equals(img->data->unscaled(10, 10), 0); // unverified
	gen_ensure_equals(img->data->scaled(0, 0), 0); // unverified
	gen_ensure_equals(img->data->scaled(10, 10), 0); // unverified
}

}

/* vim:set ts=4 sw=4: */
