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
#include <ImportNetCDF.h>
#include <proj/Geos.h>

using namespace msat;

namespace tut {

struct importnetcdf_shar
{
	importnetcdf_shar()
	{
	}

	~importnetcdf_shar()
	{
	}
};
TESTGRP(importnetcdf);

proj::ImageBox ncCropArea(proj::ImagePoint(100, 100), proj::ImagePoint(100+200, 100+50));

static void checkGeneralImageData(Image& img)
{
	gen_ensure_equals(img.year, 2005);
	gen_ensure_equals(img.month, 12);
	gen_ensure_equals(img.day, 19);
	gen_ensure_equals(img.hour, 14);
	gen_ensure_equals(img.minute, 15);
	proj::Geos* p = dynamic_cast<proj::Geos*>(img.proj.get());
	gen_ensure(p != 0);
	gen_ensure_equals(p->sublon, 0);
	gen_ensure_equals(img.channel_id, 9);
	gen_ensure_equals(img.spacecraft_id, 55);
	//gen_ensure_equals(img.column_factor, Image::columnFactorFromSeviriDX(3608));
	//gen_ensure_equals(img.line_factor, Image::columnFactorFromSeviriDX(3608));
	gen_ensure_equals(img.column_offset, 1856);
	gen_ensure_equals(img.line_offset, 1856);
	gen_ensure_equals(img.defaultFilename, "MSG1_Seviri_IR_108_channel_20051219_1415");
	gen_ensure_equals(img.shortName, "IR_108");
	gen_ensure_equals(img.unit, "K");
}

static void checkFullImageData(Image& img)
{
	checkGeneralImageData(img);

	gen_ensure_equals(img.data->columns, 1300);
	gen_ensure_equals(img.data->lines, 700);
	gen_ensure_equals(img.x0, 1500);
	gen_ensure_equals(img.y0, 200);
	gen_ensure_similar(img.data->scaled(0, 0), 231.3, 0.01);
	gen_ensure_similar(img.data->scaled(10, 10), 241.6, 0.01);
}

static void checkCroppedImageData(Image& img)
{
	checkGeneralImageData(img);

	gen_ensure_equals(img.data->columns, 200);
	gen_ensure_equals(img.data->lines, 50);
	gen_ensure_equals(img.x0, 1600);
	gen_ensure_equals(img.y0, 300);
	gen_ensure_similar(img.data->scaled(0, 0), 252.4, 0.01);
	gen_ensure_similar(img.data->scaled(10, 10), 267.5, 0.01);
}


// Test the isNetCDF function
template<> template<>
void to::test<1>()
{
	gen_ensure(isNetCDF(DATA_DIR "/MSG_Seviri_1_5_Infrared_10_8_channel_20051219_1415.nc"));
	gen_ensure(!isNetCDF(DATA_DIR "/MSG_Seviri_1_5_Infrared_9_7_channel_20060426_1945.grb"));
}

// Import a full NetCDF product
template<> template<>
void to::test<2>()
{
	std::auto_ptr<ImageImporter> imp(createNetCDFImporter(DATA_DIR "/MSG_Seviri_1_5_Infrared_10_8_channel_20051219_1415.nc"));
	ImageVector imgs;
	imp->read(imgs);

	gen_ensure_equals(imgs.size(), 1u);
	Image* img = imgs[0];

	gen_ensure_equals(img->data->slope, 1);
	gen_ensure_equals(img->data->offset, 1);
	gen_ensure_equals(img->data->bpp, 32);
	gen_ensure_equals(img->data->scalesToInt, false);

	test_tag("fullNetCDF");
	checkFullImageData(*img);
	test_untag();
}

// Import a subarea of a NetCDF product
template<> template<>
void to::test<3>()
{
	std::auto_ptr<ImageImporter> imp(createNetCDFImporter(DATA_DIR "/MSG_Seviri_1_5_Infrared_10_8_channel_20051219_1415.nc"));
	imp->cropImgArea = ncCropArea;
	ImageVector imgs;
	imp->read(imgs);

	gen_ensure_equals(imgs.size(), 1u);
	Image* img = imgs[0];

	gen_ensure_equals(img->data->slope, 1);
	gen_ensure_equals(img->data->offset, 1);
	gen_ensure_equals(img->data->bpp, 32);
	gen_ensure_equals(img->data->scalesToInt, false);

	test_tag("croppedNetCDF");
	checkCroppedImageData(*img);
	test_untag();
}

// Try reimporting an exported grib
template<> template<>
void to::test<4>()
{
	// Read the grib
	std::auto_ptr<ImageImporter> imp(createNetCDFImporter(DATA_DIR "/MSG_Seviri_1_5_Infrared_10_8_channel_20051219_1415.nc"));
	ImageVector imgs;
	imp->read(imgs);
	gen_ensure_equals(imgs.size(), 1u);
	std::auto_ptr<Image> img = recodeThroughGrib(*imgs[0]);

	gen_ensure_similar(img->data->slope, 0.01, 0.0001);
	gen_ensure_similar(img->data->offset, -1, 0.0001);
	gen_ensure_equals(img->data->bpp, 15);
	gen_ensure_equals(img->data->scalesToInt, true);

	test_tag("fullNetCDFRecodedGrib");
	checkFullImageData(*img);
	test_untag();

	gen_ensure_imagedata_similar(*img->data, *imgs[0]->data, 0.0001);
}

// Try reimporting an exported subarea grib
template<> template<>
void to::test<5>()
{
	// Read and crop the grib
	std::auto_ptr<ImageImporter> imp(createNetCDFImporter(DATA_DIR "/MSG_Seviri_1_5_Infrared_10_8_channel_20051219_1415.nc"));
	imp->cropImgArea = ncCropArea;
	ImageVector imgs;
	imp->read(imgs);
	gen_ensure_equals(imgs.size(), 1u);
	std::auto_ptr<Image> img = recodeThroughGrib(*imgs[0]);

	gen_ensure_similar(img->data->slope, 0.01, 0.0001);
	gen_ensure_similar(img->data->offset, -247.6, 0.00001);
	gen_ensure_equals(img->data->bpp, 12);
	gen_ensure_equals(img->data->scalesToInt, true);

	test_tag("croppedNetCDFRecodedGrib");
	checkCroppedImageData(*img);
	test_untag();

	gen_ensure_imagedata_similar(*img->data, *imgs[0]->data, 0.0001);
}

// Try reimporting an exported netcdf24
template<> template<>
void to::test<6>()
{
	// Read the grib
	std::auto_ptr<ImageImporter> imp(createNetCDFImporter(DATA_DIR "/MSG_Seviri_1_5_Infrared_10_8_channel_20051219_1415.nc"));
	ImageVector imgs;
	imp->read(imgs);
	gen_ensure_equals(imgs.size(), 1u);
	std::auto_ptr<Image> img = recodeThroughNetCDF24(*imgs[0]);

	gen_ensure_equals(img->data->slope, 1);
	gen_ensure_equals(img->data->offset, 0);
	gen_ensure_equals(img->data->bpp, 32);
	gen_ensure_equals(img->data->scalesToInt, false);

	test_tag("fullNetCDFRecodedNetCDF24");
	checkFullImageData(*img);
	test_untag();

	gen_ensure_imagedata_similar(*img->data, *imgs[0]->data, 0.0001);
}

// Try reimporting a subarea exported to netcdf24
template<> template<>
void to::test<7>()
{
	// Read and crop the grib
	std::auto_ptr<ImageImporter> imp(createNetCDFImporter(DATA_DIR "/MSG_Seviri_1_5_Infrared_10_8_channel_20051219_1415.nc"));
	imp->cropImgArea = ncCropArea;
	ImageVector imgs;
	imp->read(imgs);
	gen_ensure_equals(imgs.size(), 1u);
	std::auto_ptr<Image> img = recodeThroughNetCDF24(*imgs[0]);

	gen_ensure_equals(img->data->slope, 1);
	gen_ensure_equals(img->data->offset, 0);
	gen_ensure_equals(img->data->bpp, 32);
	gen_ensure_equals(img->data->scalesToInt, false);

	test_tag("croppedNetCDFRecodedNetCDF24");
	checkCroppedImageData(*img);
	test_untag();

	gen_ensure_imagedata_similar(*img->data, *imgs[0]->data, 0.0001);
}

// Try reimporting an exported netcdf
template<> template<>
void to::test<8>()
{
	// Read the grib
	std::auto_ptr<ImageImporter> imp(createNetCDFImporter(DATA_DIR "/MSG_Seviri_1_5_Infrared_10_8_channel_20051219_1415.nc"));
	ImageVector imgs;
	imp->read(imgs);
	gen_ensure_equals(imgs.size(), 1u);
	std::auto_ptr<Image> img = recodeThroughNetCDF(*imgs[0]);

	gen_ensure_equals(img->data->slope, 1);
	gen_ensure_equals(img->data->offset, 0);
	gen_ensure_equals(img->data->bpp, 32);
	gen_ensure_equals(img->data->scalesToInt, false);

	test_tag("fullNetCDFRecodedNetCDF");
	checkFullImageData(*img);
	test_untag();

	gen_ensure_imagedata_similar(*img->data, *imgs[0]->data, 0.0001);
}

// Try reimporting a subarea exported to netcdf
template<> template<>
void to::test<9>()
{
	// Read and crop the grib
	std::auto_ptr<ImageImporter> imp(createNetCDFImporter(DATA_DIR "/MSG_Seviri_1_5_Infrared_10_8_channel_20051219_1415.nc"));
	imp->cropImgArea = ncCropArea;
	ImageVector imgs;
	imp->read(imgs);
	gen_ensure_equals(imgs.size(), 1u);
	std::auto_ptr<Image> img = recodeThroughNetCDF(*imgs[0]);

	gen_ensure_equals(img->data->slope, 1);
	gen_ensure_equals(img->data->offset, 0);
	gen_ensure_equals(img->data->bpp, 32);
	gen_ensure_equals(img->data->scalesToInt, false);

	test_tag("croppedNetCDFRecodedNetCDF");
	checkCroppedImageData(*img);
	test_untag();

	gen_ensure_imagedata_similar(*img->data, *imgs[0]->data, 0.0001);
}

}

/* vim:set ts=4 sw=4: */
