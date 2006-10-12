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
#include <ImportGRIB.h>

using namespace msat;

namespace tut {

struct importgrib_shar
{
	importgrib_shar()
	{
	}

	~importgrib_shar()
	{
	}
};
TESTGRP(importgrib);

// Test the isGrib function
template<> template<>
void to::test<1>()
{
	gen_ensure(isGrib("data/MSG_Seviri_1_5_Infrared_9_7_channel_20060426_1945.grb"));
	gen_ensure(!isGrib("data/MSG_Seviri_1_5_Infrared_10_8_channel_20051219_1415.nc"));
}

// Import a full Grib product
template<> template<>
void to::test<2>()
{
	std::auto_ptr<ImageImporter> imp(createGribImporter("data/MSG_Seviri_1_5_Infrared_9_7_channel_20060426_1945.grb"));
	ImageVector imgs;
	imp->read(imgs);

	gen_ensure_equals(imgs.size(), 1u);
	Image* img = imgs[0];

	gen_ensure_equals(img->data->columns, 1300);
	gen_ensure_equals(img->data->lines, 700);
	gen_ensure_similar(img->data->slope, 0.1, 0.001);
	gen_ensure_equals(img->data->offset, 0);
	gen_ensure_equals(img->year, 2006);
	gen_ensure_equals(img->month, 4);
	gen_ensure_equals(img->day, 26);
	gen_ensure_equals(img->hour, 19);
	gen_ensure_equals(img->minute, 45);
	gen_ensure_equals(img->sublon, 0);
	gen_ensure_equals(img->channel_id, 2049);
	gen_ensure_equals(img->spacecraft_id, 55);
	gen_ensure_equals(img->column_factor, Image::columnFactorFromSeviriDX(3608));
	gen_ensure_equals(img->line_factor, Image::columnFactorFromSeviriDX(3608));
	gen_ensure_equals(img->column_offset, 1500);
	gen_ensure_equals(img->line_offset, 200);
	gen_ensure_equals(img->data->bpp, 11);
	gen_ensure_equals(img->data->scalesToInt, true);
	gen_ensure_equals(img->data->scaled(0, 0), 97.699997f);
	gen_ensure_equals(img->data->scaled(10, 10), 98.099998f);
}

// Import a subarea of a Grib product
template<> template<>
void to::test<3>()
{
	std::auto_ptr<ImageImporter> imp(createGribImporter("data/MSG_Seviri_1_5_Infrared_9_7_channel_20060426_1945.grb"));
	imp->cropX = 100;
	imp->cropY = 100;
	imp->cropWidth = 200;
	imp->cropHeight = 50;
	ImageVector imgs;
	imp->read(imgs);

	gen_ensure_equals(imgs.size(), 1u);
	Image* img = imgs[0];

	gen_ensure_equals(img->data->columns, 200);
	gen_ensure_equals(img->data->lines, 50);
	gen_ensure_similar(img->data->slope, 0.1, 0.001);
	gen_ensure_equals(img->data->offset, 0);
	gen_ensure_equals(img->year, 2006);
	gen_ensure_equals(img->month, 4);
	gen_ensure_equals(img->day, 26);
	gen_ensure_equals(img->hour, 19);
	gen_ensure_equals(img->minute, 45);
	gen_ensure_equals(img->sublon, 0);
	gen_ensure_equals(img->channel_id, 2049);
	gen_ensure_equals(img->spacecraft_id, 55);
	gen_ensure_equals(img->column_factor, Image::columnFactorFromSeviriDX(3608));
	gen_ensure_equals(img->line_factor, Image::columnFactorFromSeviriDX(3608));
	gen_ensure_equals(img->column_offset, 1600);
	gen_ensure_equals(img->line_offset, 300);
	gen_ensure_equals(img->data->bpp, 11);
	gen_ensure_equals(img->data->scalesToInt, true);
	gen_ensure_equals(img->data->scaled(0, 0), 100.50f);
	gen_ensure_equals(img->data->scaled(10, 10), 97.800003f);

}

// Try reimporting an exported grib
template<> template<>
void to::test<4>()
{
	// Read the grib
	std::auto_ptr<ImageImporter> imp(createGribImporter("data/MSG_Seviri_1_5_Infrared_9_7_channel_20060426_1945.grb"));
	ImageVector imgs;
	imp->read(imgs);
	gen_ensure_equals(imgs.size(), 1u);
	std::auto_ptr<Image> img = recodeThroughGrib(*imgs[0]);

	// Check the contents
	gen_ensure_equals(img->data->columns, 1300);
	gen_ensure_equals(img->data->lines, 700);
	gen_ensure_similar(img->data->slope, 0.1, 0.001);
	gen_ensure_equals(img->data->offset, 0);
	gen_ensure_equals(img->year, 2006);
	gen_ensure_equals(img->month, 4);
	gen_ensure_equals(img->day, 26);
	gen_ensure_equals(img->hour, 19);
	gen_ensure_equals(img->minute, 45);
	gen_ensure_equals(img->sublon, 0);
	gen_ensure_equals(img->channel_id, 2049);
	gen_ensure_equals(img->spacecraft_id, 55);
	gen_ensure_equals(img->column_factor, Image::columnFactorFromSeviriDX(3608));
	gen_ensure_equals(img->line_factor, Image::columnFactorFromSeviriDX(3608));
	gen_ensure_equals(img->column_offset, 1500);
	gen_ensure_equals(img->line_offset, 200);
	gen_ensure_equals(img->data->bpp, 11);
	gen_ensure_equals(img->data->scalesToInt, true);
	gen_ensure_equals(img->data->scaled(0, 0), 97.699997f);
	gen_ensure_equals(img->data->scaled(10, 10), 98.099998f);

	gen_ensure_imagedata_similar(*img->data, *imgs[0]->data, 0.0001);
}

// Try reimporting an exported subarea grib
template<> template<>
void to::test<5>()
{
	// Read and crop the grib
	std::auto_ptr<ImageImporter> imp(createGribImporter("data/MSG_Seviri_1_5_Infrared_9_7_channel_20060426_1945.grb"));
	imp->cropX = 100;
	imp->cropY = 100;
	imp->cropWidth = 200;
	imp->cropHeight = 50;
	ImageVector imgs;
	imp->read(imgs);
	gen_ensure_equals(imgs.size(), 1u);
	std::auto_ptr<Image> img = recodeThroughGrib(*imgs[0]);

	gen_ensure_equals(img->data->columns, 200);
	gen_ensure_equals(img->data->lines, 50);
	gen_ensure_similar(img->data->slope, 0.1, 0.001);
	gen_ensure_equals(img->data->offset, -91);
	gen_ensure_equals(img->year, 2006);
	gen_ensure_equals(img->month, 4);
	gen_ensure_equals(img->day, 26);
	gen_ensure_equals(img->hour, 19);
	gen_ensure_equals(img->minute, 45);
	gen_ensure_equals(img->sublon, 0);
	gen_ensure_equals(img->channel_id, 2049);
	gen_ensure_equals(img->spacecraft_id, 55);
	gen_ensure_equals(img->column_factor, Image::columnFactorFromSeviriDX(3608));
	gen_ensure_equals(img->line_factor, Image::columnFactorFromSeviriDX(3608));
	gen_ensure_equals(img->column_offset, 1600);
	gen_ensure_equals(img->line_offset, 300);
	gen_ensure_equals(img->data->bpp, 8);
	gen_ensure_equals(img->data->scalesToInt, true);
	gen_ensure_equals(img->data->scaled(0, 0), 100.50f);
	gen_ensure_equals(img->data->scaled(10, 10), 97.800003f);

	gen_ensure_imagedata_similar(*img->data, *imgs[0]->data, 0.0001);
}

// Try reimporting an exported netcdf24
template<> template<>
void to::test<6>()
{
	// Read the grib
	std::auto_ptr<ImageImporter> imp(createGribImporter("data/MSG_Seviri_1_5_Infrared_9_7_channel_20060426_1945.grb"));
	ImageVector imgs;
	imp->read(imgs);
	gen_ensure_equals(imgs.size(), 1u);
	std::auto_ptr<Image> img = recodeThroughNetCDF24(*imgs[0]);

	// Check the contents
	gen_ensure_equals(img->data->columns, 1300);
	gen_ensure_equals(img->data->lines, 700);
	gen_ensure_similar(img->data->slope, 0.1, 0.001);
	gen_ensure_equals(img->data->offset, 0);
	gen_ensure_equals(img->year, 2006);
	gen_ensure_equals(img->month, 4);
	gen_ensure_equals(img->day, 26);
	gen_ensure_equals(img->hour, 19);
	gen_ensure_equals(img->minute, 45);
	gen_ensure_equals(img->sublon, 0);
	gen_ensure_equals(img->channel_id, 2049);
	gen_ensure_equals(img->spacecraft_id, 55);
	gen_ensure_equals(img->column_factor, Image::columnFactorFromSeviriDX(3608));
	gen_ensure_equals(img->line_factor, Image::columnFactorFromSeviriDX(3608));
	gen_ensure_equals(img->column_offset, 1500);
	gen_ensure_equals(img->line_offset, 200);
	gen_ensure_equals(img->data->bpp, 11);
	gen_ensure_equals(img->data->scalesToInt, true);
	gen_ensure_similar(img->data->scaled(0, 0), 97.7, 0.001);
	gen_ensure_similar(img->data->scaled(10, 10), 98.1, 0.001);

	gen_ensure_imagedata_similar(*img->data, *imgs[0]->data, 0.0001);
}

// Try reimporting an exported subarea netcdf24
template<> template<>
void to::test<7>()
{
	// Read and crop the grib
	std::auto_ptr<ImageImporter> imp(createGribImporter("data/MSG_Seviri_1_5_Infrared_9_7_channel_20060426_1945.grb"));
	imp->cropX = 100;
	imp->cropY = 100;
	imp->cropWidth = 200;
	imp->cropHeight = 50;
	ImageVector imgs;
	imp->read(imgs);
	gen_ensure_equals(imgs.size(), 1u);
	std::auto_ptr<Image> img = recodeThroughNetCDF24(*imgs[0]);

	gen_ensure_equals(img->data->columns, 200);
	gen_ensure_equals(img->data->lines, 50);
	gen_ensure_similar(img->data->slope, 0.1, 0.001);
	gen_ensure_equals(img->data->offset, 0);
	gen_ensure_equals(img->year, 2006);
	gen_ensure_equals(img->month, 4);
	gen_ensure_equals(img->day, 26);
	gen_ensure_equals(img->hour, 19);
	gen_ensure_equals(img->minute, 45);
	gen_ensure_equals(img->sublon, 0);
	gen_ensure_equals(img->channel_id, 2049);
	gen_ensure_equals(img->spacecraft_id, 55);
	gen_ensure_equals(img->column_factor, Image::columnFactorFromSeviriDX(3608));
	gen_ensure_equals(img->line_factor, Image::columnFactorFromSeviriDX(3608));
	gen_ensure_equals(img->column_offset, 1600);
	gen_ensure_equals(img->line_offset, 300);
	gen_ensure_equals(img->data->bpp, 11);
	gen_ensure_equals(img->data->scalesToInt, true);
	gen_ensure_equals(img->data->scaled(0, 0), 100.50f);
	gen_ensure_equals(img->data->scaled(10, 10), 97.800003f);

	gen_ensure_imagedata_similar(*img->data, *imgs[0]->data, 0.0001);
}

}

/* vim:set ts=4 sw=4: */
