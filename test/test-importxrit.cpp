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
	gen_ensure_similar(img->data->slope, 0.031999f, 0.00001);
	gen_ensure_similar(img->data->offset, -1.63196f, 0.00001);
	gen_ensure_equals(img->year, 2006);
	gen_ensure_equals(img->month, 4);
	gen_ensure_equals(img->day, 26);
	gen_ensure_equals(img->hour, 19);
	gen_ensure_equals(img->minute, 45);
	gen_ensure_equals(img->sublon, 0);
	gen_ensure_equals(img->channel_id, 12);
	gen_ensure_equals(img->spacecraft_id, 55);
	gen_ensure_equals(img->column_factor, -40927014);
	gen_ensure_equals(img->line_factor, -40927014);
	gen_ensure_equals(img->column_offset, -2);
	gen_ensure_equals(img->line_offset, 462);
	gen_ensure_equals(img->data->bpp, 10);
	gen_ensure_equals(img->data->scalesToInt, true);
	gen_ensure_equals(img->data->scaled(0, 0), 0); // unverified
	gen_ensure_equals(img->data->scaled(10, 10), 0); // unverified
	gen_ensure_equals(img->data->scaled(356, 5569),  9.40779495239257812500); // unverified
	gen_ensure_equals(img->data->scaled(357, 5569),  7.29584074020385742188); // unverified
	gen_ensure_equals(img->data->scaled(358, 5569),  5.72787475585937500000); // unverified
	gen_ensure_equals(img->data->scaled(214, 5570), 12.28773117065429687500); // unverified
	gen_ensure_equals(img->data->scaled(215, 5570), 10.81576347351074218750); // unverified
	gen_ensure_equals(img->data->scaled(216, 5570), 11.00776004791259765625); // unverified
	gen_ensure_equals(img->data->scaled(466, 5571),  7.48783636093139648438); // unverified
	gen_ensure_equals(img->data->scaled(467, 5571),  8.31981849670410156250); // unverified
	gen_ensure_equals(img->data->scaled(468, 5571),  8.83180713653564453125); // unverified

#if 0
	-- look for nonempty values
	using namespace std;
	for (size_t y = 0; y < img->data->lines; ++y)
		for (size_t x = 0; x < img->data->columns; ++x)
			if (img->data->scaled(x, y) != 0)
				cout << "(" << x << ", " << y << "): " << img->data->scaled(x, y) << endl;
#endif
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
	opts.AreaLinStart = 5500;
	opts.AreaNlin = 300;
	opts.AreaPixStart = 100;
	opts.AreaNpix = 400;

	std::auto_ptr<Image> img = importXRIT(opts);

	//gen_ensure_equals(img->name, ""); // unverified
	gen_ensure_equals(img->data->columns, 400);
	gen_ensure_equals(img->data->lines, 300);
	gen_ensure_similar(img->data->slope, 0.0319993, 0.00001);
	gen_ensure_similar(img->data->offset, -1.6319643, 0.00001);
	gen_ensure_equals(img->year, 2006);
	gen_ensure_equals(img->month, 4);
	gen_ensure_equals(img->day, 26);
	gen_ensure_equals(img->hour, 19);
	gen_ensure_equals(img->minute, 45);
	gen_ensure_equals(img->sublon, 0);
	gen_ensure_equals(img->channel_id, 12);
	gen_ensure_equals(img->spacecraft_id, 55); // unverified
	gen_ensure_equals(img->column_factor, -40927014);
	gen_ensure_equals(img->line_factor, -40927014);
	gen_ensure_equals(img->column_offset, 98);
	gen_ensure_equals(img->line_offset, 5962);
	gen_ensure_equals(img->data->bpp, 10); // unverified
	gen_ensure_equals(img->data->scalesToInt, true);
	gen_ensure_equals(img->data->scaled(0, 0), 0); // unverified
	gen_ensure_equals(img->data->scaled(10, 10), 0); // unverified
	gen_ensure_equals(img->data->scaled(256, 69),  9.40779495239257812500); // unverified
	gen_ensure_equals(img->data->scaled(257, 69),  7.29584074020385742188); // unverified
	gen_ensure_equals(img->data->scaled(258, 69),  5.72787475585937500000); // unverified
	gen_ensure_equals(img->data->scaled(114, 70), 12.28773117065429687500); // unverified
	gen_ensure_equals(img->data->scaled(115, 70), 10.81576347351074218750); // unverified
	gen_ensure_equals(img->data->scaled(116, 70), 11.00776004791259765625); // unverified
	gen_ensure_equals(img->data->scaled(366, 71),  7.48783636093139648438); // unverified
	gen_ensure_equals(img->data->scaled(367, 71),  8.31981849670410156250); // unverified
	gen_ensure_equals(img->data->scaled(368, 71),  8.83180713653564453125); // unverified
}

// Import a full XRIT product and pass it from grib
template<> template<>
void to::test<4>()
{
#if 0
	-- Temporarily disabled as it currently uses too much ram

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
	gen_ensure_equals(img->spacecraft_id, 55); // unverified
	gen_ensure_equals(img->column_factor, -40927014);
	gen_ensure_equals(img->line_factor, -40927014);
	gen_ensure_equals(img->column_offset, -2);
	gen_ensure_equals(img->line_offset, 462);
	gen_ensure_equals(img->data->bpp, 32); // unverified
	gen_ensure_equals(img->data->scalesToInt, false);
	gen_ensure_equals(img->data->scaled(0, 0), 0); // unverified
	gen_ensure_equals(img->data->scaled(10, 10), 0); // unverified

	gen_ensure_imagedata_similar(*img->data, *imgs[0]->data, 0.0001);
#endif
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
	opts.AreaLinStart = 5500;
	opts.AreaNlin = 300;
	opts.AreaPixStart = 100;
	opts.AreaNpix = 400;

	std::auto_ptr<Image> imgorig = importXRIT(opts);
	std::auto_ptr<Image> img = recodeThroughGrib(*imgorig);

	//gen_ensure_equals(img->name, ""); // unverified
	gen_ensure_equals(img->data->columns, 400);
	gen_ensure_equals(img->data->lines, 300);
	gen_ensure_equals(img->data->slope, 0.01);
	gen_ensure_equals(img->data->offset, 0);
	gen_ensure_equals(img->year, 2006);
	gen_ensure_equals(img->month, 4);
	gen_ensure_equals(img->day, 26);
	gen_ensure_equals(img->hour, 19);
	gen_ensure_equals(img->minute, 45);
	gen_ensure_equals(img->sublon, 0);
	gen_ensure_equals(img->channel_id, 12);
	gen_ensure_equals(img->spacecraft_id, 55); // unverified
	gen_ensure_equals(img->column_factor, Image::columnFactorFromSeviriDX(Image::seviriDXFromColumnFactor(-40927014)));
	gen_ensure_equals(img->line_factor, Image::lineFactorFromSeviriDY(Image::seviriDYFromLineFactor(-40927014)));
	gen_ensure_equals(img->column_offset, 98);
	gen_ensure_equals(img->line_offset, 5962);
	gen_ensure_equals(img->data->bpp, 11); // unverified
	gen_ensure_equals(img->data->scalesToInt, true);
	gen_ensure_equals(img->data->scaled(0, 0), 0); // unverified
	gen_ensure_equals(img->data->scaled(10, 10), 0); // unverified
	gen_ensure_similar(img->data->scaled(256, 69),  9.41, 0.01); // unverified
	gen_ensure_similar(img->data->scaled(257, 69),  7.30, 0.01); // unverified
	gen_ensure_similar(img->data->scaled(258, 69),  5.73, 0.01); // unverified
	gen_ensure_similar(img->data->scaled(114, 70), 12.29, 0.01); // unverified
	gen_ensure_similar(img->data->scaled(115, 70), 10.82, 0.01); // unverified
	gen_ensure_similar(img->data->scaled(116, 70), 11.01, 0.01); // unverified
	gen_ensure_similar(img->data->scaled(366, 71),  7.49, 0.01); // unverified
	gen_ensure_similar(img->data->scaled(367, 71),  8.32, 0.01); // unverified
	gen_ensure_similar(img->data->scaled(368, 71),  8.83, 0.01); // unverified

	gen_ensure_imagedata_similar(*img->data, *imgorig->data, 0.01);
}

// Import a full XRIT product and pass it from netcdf24
template<> template<>
void to::test<6>()
{
#if 0
	-- Temporarily disabled as it currently uses too much ram

	XRITImportOptions opts;
	opts.directory = "data";
	opts.resolution = "H";
	opts.productid1 = "MSG1";
	opts.productid2 = "HRV";
	opts.timing = "200604261945";
	opts.subarea = false;
	std::auto_ptr<Image> img = importXRIT(opts);
	img = recodeThroughNetCDF24(*img);

	//gen_ensure_equals(img->name, ""); // unverified
	gen_ensure_equals(img->data->columns, 5568);
	gen_ensure_equals(img->data->lines, 11136);
	gen_ensure_similar(img->data->slope, 0.031999f, 0.00001);
	gen_ensure_similar(img->data->offset, -1.63196f, 0.00001);
	gen_ensure_equals(img->year, 2006);
	gen_ensure_equals(img->month, 4);
	gen_ensure_equals(img->day, 26);
	gen_ensure_equals(img->hour, 19);
	gen_ensure_equals(img->minute, 45);
	gen_ensure_equals(img->sublon, 0);
	gen_ensure_equals(img->channel_id, 12);
	gen_ensure_equals(img->spacecraft_id, 55); // unverified
	gen_ensure_equals(img->column_factor, -40927014);
	gen_ensure_equals(img->line_factor, -40927014);
	gen_ensure_equals(img->column_offset, -2);
	gen_ensure_equals(img->line_offset, 462);
	gen_ensure_equals(img->data->bpp, 32); // unverified
	gen_ensure_equals(img->data->scalesToInt, false);
	gen_ensure_equals(img->data->scaled(0, 0), 0); // unverified
	gen_ensure_equals(img->data->scaled(10, 10), 0); // unverified

	gen_ensure_imagedata_similar(*img->data, *imgs[0]->data, 0.0001);
#endif
}

// Import a subarea of an XRIT product and pass it from netcdf24
template<> template<>
void to::test<7>()
{
	XRITImportOptions opts;
	opts.directory = "data";
	opts.resolution = "H";
	opts.productid1 = "MSG1";
	opts.productid2 = "HRV";
	opts.timing = "200604261945";
	opts.subarea = true;
	opts.AreaLinStart = 5500;
	opts.AreaNlin = 300;
	opts.AreaPixStart = 100;
	opts.AreaNpix = 400;

	std::auto_ptr<Image> imgorig = importXRIT(opts);
	std::auto_ptr<Image> img = recodeThroughNetCDF24(*imgorig);

	//gen_ensure_equals(img->name, ""); // unverified
	gen_ensure_equals(img->data->columns, 400);
	gen_ensure_equals(img->data->lines, 300);
	gen_ensure_similar(img->data->slope, 0.031999f, 0.00001);
	gen_ensure_similar(img->data->offset, -1.63196f, 0.00001);
	gen_ensure_equals(img->year, 2006);
	gen_ensure_equals(img->month, 4);
	gen_ensure_equals(img->day, 26);
	gen_ensure_equals(img->hour, 19);
	gen_ensure_equals(img->minute, 45);
	gen_ensure_equals(img->sublon, 0);
	gen_ensure_equals(img->channel_id, 12);
	gen_ensure_equals(img->spacecraft_id, 55); // unverified
	gen_ensure_equals(img->column_factor, Image::columnFactorFromSeviriDX(Image::seviriDXFromColumnFactor(-40927014)));
	gen_ensure_equals(img->line_factor, Image::lineFactorFromSeviriDY(Image::seviriDYFromLineFactor(-40927014)));
	gen_ensure_equals(img->column_offset, 98);
	gen_ensure_equals(img->line_offset, 5962);
	gen_ensure_equals(img->data->bpp, 10);
	gen_ensure_equals(img->data->scalesToInt, true);
	gen_ensure_equals(img->data->scaled(0, 0), 0); // unverified
	gen_ensure_equals(img->data->scaled(10, 10), 0); // unverified
	gen_ensure_similar(img->data->scaled(256, 69),  9.41, 0.01); // unverified
	gen_ensure_similar(img->data->scaled(257, 69),  7.30, 0.01); // unverified
	gen_ensure_similar(img->data->scaled(258, 69),  5.73, 0.01); // unverified
	gen_ensure_similar(img->data->scaled(114, 70), 12.29, 0.01); // unverified
	gen_ensure_similar(img->data->scaled(115, 70), 10.82, 0.01); // unverified
	gen_ensure_similar(img->data->scaled(116, 70), 11.01, 0.01); // unverified
	gen_ensure_similar(img->data->scaled(366, 71),  7.49, 0.01); // unverified
	gen_ensure_similar(img->data->scaled(367, 71),  8.32, 0.01); // unverified
	gen_ensure_similar(img->data->scaled(368, 71),  8.83, 0.01); // unverified

	gen_ensure_imagedata_similar(*img->data, *imgorig->data, 0.0001);
}

}

/* vim:set ts=4 sw=4: */
