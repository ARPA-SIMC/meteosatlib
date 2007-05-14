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
#include <ImportSAFH5.h>
#include <proj/Geos.h>
#include <math.h>

using namespace msat;

namespace tut {

struct importsafh5_shar
{
	importsafh5_shar()
	{
	}

	~importsafh5_shar()
	{
	}
};
TESTGRP(importsafh5);

static void checkGeneralImageData(Image& img)
{
	gen_ensure_equals(img.year, 2005);
	gen_ensure_equals(img.month, 12);
	gen_ensure_equals(img.day, 5);
	gen_ensure_equals(img.hour, 6);
	gen_ensure_equals(img.minute, 15);
	proj::Geos* p = dynamic_cast<proj::Geos*>(img.proj.get());
	gen_ensure(p != 0);
	gen_ensure_equals(p->sublon, 0);
	gen_ensure_equals(img.channel_id, 546);
	gen_ensure_equals(img.column_offset, 1856);
	gen_ensure_equals(img.line_offset, 1856);
	gen_ensure_equals(img.spacecraft_id, 55); // it is GP_SC_ID, but shouldn't it be 55?
	gen_ensure_equals(img.shortName, "CRR");
	gen_ensure_equals(img.unit, "NUMERIC");
}

static void checkFullImageData(Image& img)
{
	checkGeneralImageData(img);

	gen_ensure_equals(img.data->columns, 1300);
	gen_ensure_equals(img.data->lines, 700);
	gen_ensure_equals(img.x0, 1500);
	gen_ensure_equals(img.y0, 200);
	gen_ensure_equals(img.data->scaled(0, 0), 0.0f);
	gen_ensure_equals(img.data->scaled(10, 10), 0.0f);
	gen_ensure_equals(img.data->scaled(516, 54), 3);
}

static void checkCroppedImageData(Image& img)
{
	checkGeneralImageData(img);

	gen_ensure_equals(img.data->columns, 500);
	gen_ensure_equals(img.data->lines, 50);
	gen_ensure_equals(img.x0, 1600);
	gen_ensure_equals(img.y0, 250);
	gen_ensure_equals(img.data->scaled(0, 0), 0.0f);
	gen_ensure_equals(img.data->scaled(10, 10), 0.0f);
	gen_ensure_equals(img.data->scaled(416, 4), 3);
}

proj::ImageBox safCropArea(proj::ImagePoint(100, 50), proj::ImagePoint(100+500, 50+50));

// Test the isSAFH5 function
template<> template<>
void to::test<1>()
{
	gen_ensure(isSAFH5(DATA_DIR "/SAFNWC_MSG1_CRR__05339_025_MSGEURO_____.h5"));
	gen_ensure(!isSAFH5(DATA_DIR "/MSG_Seviri_1_5_Infrared_10_8_channel_20051219_1415.nc"));
}

// Import a full SAFH5 product
template<> template<>
void to::test<2>()
{
	std::auto_ptr<ImageImporter> imp(createSAFH5Importer(DATA_DIR "/SAFNWC_MSG1_CRR__05339_025_MSGEURO_____.h5"));
	ImageVector imgs;
	imp->read(imgs);

	gen_ensure_equals(imgs.size(), 3u);
	Image* img = imgs[0];

	gen_ensure_equals(img->column_res, 13642337*exp2(-16));
	gen_ensure_equals(img->line_res, 13642337*exp2(-16));
	gen_ensure_equals(img->data->slope, 1);
	gen_ensure_equals(img->data->offset, 0);
	gen_ensure_equals(img->data->bpp, 3);
	gen_ensure_equals(img->data->scalesToInt, true);
	gen_ensure_equals(img->defaultFilename, "SAF_MSGEURO_CRR_20051205_0615");

	test_tag("fullSAFH5");
	checkFullImageData(*img);
	test_untag();
}

// Import a subarea of a SAFH5 product
template<> template<>
void to::test<3>()
{
	std::auto_ptr<ImageImporter> imp(createSAFH5Importer(DATA_DIR "/SAFNWC_MSG1_CRR__05339_025_MSGEURO_____.h5"));
	imp->cropImgArea = safCropArea;
	ImageVector imgs;
	imp->read(imgs);

	gen_ensure_equals(imgs.size(), 3u);
	Image* img = imgs[0];

	gen_ensure_equals(img->column_res, 13642337*exp2(-16));
	gen_ensure_equals(img->line_res, 13642337*exp2(-16));
	gen_ensure_equals(img->data->slope, 1);
	gen_ensure_equals(img->data->offset, 0);
	gen_ensure_equals(img->data->bpp, 3);
	gen_ensure_equals(img->data->scalesToInt, true);
	gen_ensure_equals(img->defaultFilename, "SAF_MSGEURO_CRR_20051205_0615");

	test_tag("croppedSAFH5");
	checkCroppedImageData(*img);
	test_untag();
}

// Try reimporting an exported grib
template<> template<>
void to::test<4>()
{
	// Read the grib
	std::auto_ptr<ImageImporter> imp(createSAFH5Importer(DATA_DIR "/SAFNWC_MSG1_CRR__05339_025_MSGEURO_____.h5"));
	ImageVector imgs;
	imp->read(imgs);
	gen_ensure_equals(imgs.size(), 3u);
	std::auto_ptr<Image> img = recodeThroughGrib(*imgs[0]);

	gen_ensure_equals(img->column_res, Image::columnResFromSeviriDX(Image::seviriDXFromColumnRes(13642337*exp2(-16))));
	gen_ensure_equals(img->line_res, Image::columnResFromSeviriDX(Image::seviriDXFromColumnRes(13642337*exp2(-16))));
	gen_ensure_equals(img->data->slope, 1);
	gen_ensure_equals(img->data->offset, 0);
	gen_ensure_equals(img->data->bpp, 3);
	gen_ensure_equals(img->data->scalesToInt, true);
	gen_ensure_equals(img->defaultFilename, "MSG1_Seviri_CRR_channel_20051205_0615");

	test_tag("fullSAFH5RecodedGrib");
	checkFullImageData(*img);
	test_untag();

	gen_ensure_imagedata_similar(*img->data, *imgs[0]->data, 0.0001);
}

// Try reimporting a subarea exported to grib
template<> template<>
void to::test<5>()
{
	// Read and crop the grib
	std::auto_ptr<ImageImporter> imp(createSAFH5Importer(DATA_DIR "/SAFNWC_MSG1_CRR__05339_025_MSGEURO_____.h5"));
	imp->cropImgArea = safCropArea;
	ImageVector imgs;
	imp->read(imgs);
	gen_ensure_equals(imgs.size(), 3u);
	std::auto_ptr<Image> img = recodeThroughGrib(*imgs[0]);

	gen_ensure_equals(img->column_res, Image::columnResFromSeviriDX(Image::seviriDXFromColumnRes(13642337*exp2(-16))));
	gen_ensure_equals(img->line_res, Image::columnResFromSeviriDX(Image::seviriDXFromColumnRes(13642337*exp2(-16))));
	gen_ensure_equals(img->data->slope, 1);
	gen_ensure_equals(img->data->offset, 0);
	gen_ensure_equals(img->data->bpp, 2);
	gen_ensure_equals(img->data->scalesToInt, true);
	gen_ensure_equals(img->defaultFilename, "MSG1_Seviri_CRR_channel_20051205_0615");

	test_tag("croppedSAFH5RecodedGrib");
	checkCroppedImageData(*img);
	test_untag();

	gen_ensure_imagedata_similar(*img->data, *imgs[0]->data, 0.0001);
}

// Try reimporting an exported netcdf24
template<> template<>
void to::test<6>()
{
	// Read the grib
	std::auto_ptr<ImageImporter> imp(createSAFH5Importer(DATA_DIR "/SAFNWC_MSG1_CRR__05339_025_MSGEURO_____.h5"));
	ImageVector imgs;
	imp->read(imgs);
	gen_ensure_equals(imgs.size(), 3u);
	std::auto_ptr<Image> img = recodeThroughNetCDF24(*imgs[0]);

	gen_ensure_equals(img->column_res, Image::columnResFromSeviriDX(Image::seviriDXFromColumnRes(13642337*exp2(-16))));
	gen_ensure_equals(img->line_res, Image::columnResFromSeviriDX(Image::seviriDXFromColumnRes(13642337*exp2(-16))));
	gen_ensure_equals(img->data->slope, 1);
	gen_ensure_equals(img->data->offset, 0);
	gen_ensure_equals(img->data->bpp, 3);
	gen_ensure_equals(img->data->scalesToInt, true);
	gen_ensure_equals(img->defaultFilename, "MSG1_Seviri_CRR_channel_20051205_0615");

	test_tag("fullSAFH5RecodedNetCDF24");
	checkFullImageData(*img);
	test_untag();

	gen_ensure_imagedata_similar(*img->data, *imgs[0]->data, 0.0001);
}

// Try reimporting a subarea exported to netcdf24
template<> template<>
void to::test<7>()
{
	// Read and crop the grib
	std::auto_ptr<ImageImporter> imp(createSAFH5Importer(DATA_DIR "/SAFNWC_MSG1_CRR__05339_025_MSGEURO_____.h5"));
	imp->cropImgArea = safCropArea;
	ImageVector imgs;
	imp->read(imgs);
	gen_ensure_equals(imgs.size(), 3u);
	std::auto_ptr<Image> img = recodeThroughNetCDF24(*imgs[0]);

	gen_ensure_equals(img->column_res, Image::columnResFromSeviriDX(Image::seviriDXFromColumnRes(13642337*exp2(-16))));
	gen_ensure_equals(img->line_res, Image::columnResFromSeviriDX(Image::seviriDXFromColumnRes(13642337*exp2(-16))));
	gen_ensure_equals(img->data->slope, 1);
	gen_ensure_equals(img->data->offset, 0);
	gen_ensure_equals(img->data->bpp, 2);
	gen_ensure_equals(img->data->scalesToInt, true);
	gen_ensure_equals(img->defaultFilename, "MSG1_Seviri_CRR_channel_20051205_0615");

	test_tag("croppedSAFH5RecodedNetCDF24");
	checkCroppedImageData(*img);
	test_untag();

	gen_ensure_imagedata_similar(*img->data, *imgs[0]->data, 0.0001);
}

// Try reimporting an exported netcdf
template<> template<>
void to::test<8>()
{
	// Read the grib
	std::auto_ptr<ImageImporter> imp(createSAFH5Importer(DATA_DIR "/SAFNWC_MSG1_CRR__05339_025_MSGEURO_____.h5"));
	ImageVector imgs;
	imp->read(imgs);
	gen_ensure_equals(imgs.size(), 3u);
	std::auto_ptr<Image> img = recodeThroughNetCDF(*imgs[0]);

	gen_ensure_equals(img->column_res, 13642337*exp2(-16));
	gen_ensure_equals(img->line_res, 13642337*exp2(-16));
	gen_ensure_equals(img->data->slope, 1);
	gen_ensure_equals(img->data->offset, 0);
	gen_ensure_equals(img->data->bpp, 32);
	gen_ensure_equals(img->data->scalesToInt, false);
	gen_ensure_equals(img->defaultFilename, "MSG1_Seviri_CRR_channel_20051205_0615");

	test_tag("fullSAFH5RecodedNetCDF");
	checkFullImageData(*img);
	test_untag();

	gen_ensure_imagedata_similar(*img->data, *imgs[0]->data, 0.0001);
}

// Try reimporting a subarea exported to netcdf
template<> template<>
void to::test<9>()
{
	// Read and crop the grib
	std::auto_ptr<ImageImporter> imp(createSAFH5Importer(DATA_DIR "/SAFNWC_MSG1_CRR__05339_025_MSGEURO_____.h5"));
	imp->cropImgArea = safCropArea;
	ImageVector imgs;
	imp->read(imgs);
	gen_ensure_equals(imgs.size(), 3u);
	std::auto_ptr<Image> img = recodeThroughNetCDF(*imgs[0]);

	gen_ensure_equals(img->column_res, 13642337*exp2(-16));
	gen_ensure_equals(img->line_res, 13642337*exp2(-16));
	gen_ensure_equals(img->data->slope, 1);
	gen_ensure_equals(img->data->offset, 0);
	gen_ensure_equals(img->data->bpp, 32);
	gen_ensure_equals(img->data->scalesToInt, false);
	gen_ensure_equals(img->defaultFilename, "MSG1_Seviri_CRR_channel_20051205_0615");

	test_tag("croppedSAFH5RecodedNetCDF");
	checkCroppedImageData(*img);
	test_untag();

	gen_ensure_imagedata_similar(*img->data, *imgs[0]->data, 0.0001);
}

}

/* vim:set ts=4 sw=4: */
