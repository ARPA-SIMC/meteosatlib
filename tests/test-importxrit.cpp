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
#include <proj/Geos.h>
#include <math.h>

#undef PERFORM_SLOW_TESTS

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

static void checkGeneralImageData(Image& img)
{
	gen_ensure_equals(img.year, 2006);
	gen_ensure_equals(img.month, 11);
	gen_ensure_equals(img.day, 14);
	gen_ensure_equals(img.hour, 12);
	gen_ensure_equals(img.minute, 0);
	proj::Geos* p = dynamic_cast<proj::Geos*>(img.proj.get());
	gen_ensure(p != 0);
	gen_ensure_equals(p->sublon, 0);
	gen_ensure_equals(img.channel_id, 12);
	gen_ensure_equals(img.spacecraft_id, 55);
	gen_ensure_equals(img.column_offset, 5566);
	gen_ensure_equals(img.line_offset, 5566);
	//gen_ensure_equals(img.column_offset, -2);
	//gen_ensure_equals(img.line_offset, 462);
	//gen_ensure_equals(img.column_offset, 2060 - (100 + 2064));
	//gen_ensure_equals(img.line_offset, 5566 - 5500);
	gen_ensure_equals(img.shortName, "HRV");
	gen_ensure_equals(img.unit, "mW m^-2 sr^-1 (cm^-1)^-1");
}

static void checkFullImageData(Image& img)
{
	checkGeneralImageData(img);

	gen_ensure_equals(img.data->columns, 11136);
	gen_ensure_equals(img.data->lines, 11136);
	gen_ensure_equals(img.x0, 0);
	gen_ensure_equals(img.y0, 0);
	gen_ensure_equals(img.data->scaled(0, 0), img.data->missingValue);
	gen_ensure_equals(img.data->scaled(10, 10), img.data->missingValue);

	// TODO: now that the image has been moved as the concept of HRV frame
	// changed, choose a different set of points with non-missing values
	gen_ensure_similar(img.data->scaled(2540, 2950), img.data->missingValue, 0.001); // unverified
	gen_ensure_similar(img.data->scaled(2550, 2970), img.data->missingValue, 0.001); // unverified
	gen_ensure_similar(img.data->scaled(2560, 2990), img.data->missingValue, 0.001); // unverified
	gen_ensure_similar(img.data->scaled(2570, 3010), img.data->missingValue, 0.001); // unverified
	gen_ensure_similar(img.data->scaled(2580, 3030), img.data->missingValue, 0.001); // unverified
	gen_ensure_similar(img.data->scaled(2590, 3050), img.data->missingValue, 0.001); // unverified
	gen_ensure_similar(img.data->scaled(2600, 3070), img.data->missingValue, 0.001); // unverified
	gen_ensure_similar(img.data->scaled(2610, 3090), img.data->missingValue, 0.001); // unverified
	gen_ensure_similar(img.data->scaled(2620, 3110), img.data->missingValue, 0.001); // unverified
	gen_ensure_similar(img.data->scaled(2630, 3130), img.data->missingValue, 0.001); // unverified
	gen_ensure_similar(img.data->scaled(2640, 3150), img.data->missingValue, 0.001); // unverified

    /*
	gen_ensure_similar(img.data->scaled(2540, 2950), 6.07987, 0.001); // unverified
	gen_ensure_similar(img.data->scaled(2550, 2970), 6.27186, 0.001); // unverified
	gen_ensure_similar(img.data->scaled(2560, 2990), 4.73590, 0.001); // unverified
	gen_ensure_similar(img.data->scaled(2570, 3010), img.data->missingValue, 0.001); // unverified
	gen_ensure_similar(img.data->scaled(2580, 3030), img.data->missingValue, 0.001); // unverified
	gen_ensure_similar(img.data->scaled(2590, 3050), img.data->missingValue, 0.001); // unverified
	gen_ensure_similar(img.data->scaled(2600, 3070), 5.79187, 0.001); // unverified
	gen_ensure_similar(img.data->scaled(2610, 3090), 5.05589, 0.001); // unverified
	gen_ensure_similar(img.data->scaled(2620, 3110), 4.57590, 0.001); // unverified
	gen_ensure_similar(img.data->scaled(2630, 3130), 4.86389, 0.001); // unverified
	gen_ensure_similar(img.data->scaled(2640, 3150), 5.27988, 0.001); // unverified
	*/
}

static std::auto_ptr<ImageImporter> importer()
{
	return createXRITImporter(DATA_DIR "/H:MSG1:HRV:200611141200");
}
static std::auto_ptr<ImageImporter> croppedImporter()
{
	std::auto_ptr<ImageImporter> imp = importer();
	imp->cropImgArea = proj::ImageBox(proj::ImagePoint(2540, 2950), proj::ImagePoint(2540+150, 2950+300));
	return imp;
}

static void checkCroppedImageData(Image& img)
{
	checkGeneralImageData(img);

	gen_ensure_equals(img.data->columns, 150);
	gen_ensure_equals(img.data->lines, 300);
	gen_ensure_equals(img.x0, 2540);
	gen_ensure_equals(img.y0, 2950);
	// TODO: now that the image has been moved as the concept of HRV frame
	// changed, choose a different set of points with non-missing values
	gen_ensure_similar(img.data->scaled(  0,   0), img.data->missingValue, 0.001); // unverified
	gen_ensure_similar(img.data->scaled( 10,  20), img.data->missingValue, 0.001); // unverified
	gen_ensure_similar(img.data->scaled( 20,  40), img.data->missingValue, 0.001); // unverified
	gen_ensure_similar(img.data->scaled( 30,  60), img.data->missingValue, 0.001); // unverified
	gen_ensure_similar(img.data->scaled( 40,  80), img.data->missingValue, 0.001); // unverified
	gen_ensure_similar(img.data->scaled( 50, 100), img.data->missingValue, 0.001); // unverified
	gen_ensure_similar(img.data->scaled( 60, 120), img.data->missingValue, 0.001); // unverified
	gen_ensure_similar(img.data->scaled( 70, 140), img.data->missingValue, 0.001); // unverified
	gen_ensure_similar(img.data->scaled( 80, 160), img.data->missingValue, 0.001); // unverified
	gen_ensure_similar(img.data->scaled( 90, 180), img.data->missingValue, 0.01); // unverified
	gen_ensure_similar(img.data->scaled(100, 200), img.data->missingValue, 0.01); // unverified
	/*
	gen_ensure_similar(img.data->scaled(  0,   0), 6.07987, 0.001); // unverified
	gen_ensure_similar(img.data->scaled( 10,  20), 6.27186, 0.001); // unverified
	gen_ensure_similar(img.data->scaled( 20,  40), 4.73590, 0.001); // unverified
	gen_ensure_similar(img.data->scaled( 30,  60), img.data->missingValue, 0.001); // unverified
	gen_ensure_similar(img.data->scaled( 40,  80), img.data->missingValue, 0.001); // unverified
	gen_ensure_similar(img.data->scaled( 50, 100), img.data->missingValue, 0.001); // unverified
	gen_ensure_similar(img.data->scaled( 60, 120), 5.79187, 0.001); // unverified
	gen_ensure_similar(img.data->scaled( 70, 140), 5.05589, 0.001); // unverified
	gen_ensure_similar(img.data->scaled( 80, 160), 4.57590, 0.001); // unverified
	gen_ensure_similar(img.data->scaled( 90, 180), 4.86389, 0.01); // unverified
	gen_ensure_similar(img.data->scaled(100, 200), 5.27988, 0.01); // unverified
	*/
}


// Test the isXRIT function
template<> template<>
void to::test<1>()
{
	gen_ensure(isXRIT(DATA_DIR "/H:MSG1:HRV:200611141200"));
	gen_ensure(!isXRIT(DATA_DIR "/H:MSG1:HRV"));
}

// Import a full XRIT product
template<> template<>
void to::test<2>()
{
	ImageVector imgs(*importer());
	gen_ensure_equals(imgs.size(), 1u);
	std::auto_ptr<Image> img = imgs.shift();

	gen_ensure_equals(img->defaultFilename, "H_MSG1_Seviri_HRV_channel_20061114_1200");

	//gen_ensure_equals(img->name, ""); // unverified
	gen_ensure_equals(img->column_res, 40927014*exp2(-16));
	gen_ensure_equals(img->line_res, 40927014*exp2(-16));
	gen_ensure_similar(img->data->slope, 0.031999f, 0.00001);
	gen_ensure_similar(img->data->offset, -1.63196f, 0.00001);
	gen_ensure_equals(img->data->bpp, 10);
	gen_ensure_equals(img->data->scalesToInt, true);

	test_tag("fullXRIT");
	checkFullImageData(*img);
	test_untag();

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
	ImageVector imgs(*croppedImporter());
	gen_ensure_equals(imgs.size(), 1u);
	std::auto_ptr<Image> img = imgs.shift();

	gen_ensure_equals(img->defaultFilename, "H_MSG1_Seviri_HRV_channel_20061114_1200");

	//gen_ensure_equals(img->name, ""); // unverified
	gen_ensure_equals(img->column_res, 40927014*exp2(-16));
	gen_ensure_equals(img->line_res, 40927014*exp2(-16));
	gen_ensure_similar(img->data->slope, 0.0319993, 0.00001);
	gen_ensure_similar(img->data->offset, -1.6319643, 0.00001);
	gen_ensure_equals(img->data->bpp, 10); // unverified
	gen_ensure_equals(img->data->scalesToInt, true);

	test_tag("croppedXRIT");
	checkCroppedImageData(*img);
	test_untag();
}

// Import a full XRIT product and pass it from grib
template<> template<>
void to::test<4>()
{
}

// Import a subarea of an XRIT product and pass it from grib
template<> template<>
void to::test<5>()
{
}

// Import a full XRIT product and pass it from netcdf24
template<> template<>
void to::test<6>()
{
#if PERFORM_SLOW_TESTS
	-- Temporarily disabled as it currently uses too much ram

	XRITImportOptions opts;
	setSource(opts);
	std::auto_ptr<Image> img = importXRIT(opts);
	img = recodeThroughNetCDF24(*img);

	gen_ensure_equals(img->defaultFilename, "MSG1_Seviri_HRV_channel_20061114_1200");

	//gen_ensure_equals(img->name, ""); // unverified
	gen_ensure_equals(img->column_factor, 40927014);
	gen_ensure_equals(img->line_factor, 40927014);
	gen_ensure_similar(img->data->slope, 0.031999f, 0.00001);
	gen_ensure_similar(img->data->offset, -1.63196f, 0.00001);
	gen_ensure_equals(img->data->bpp, 32); // unverified
	gen_ensure_equals(img->data->scalesToInt, false);

	test_tag("fullXRITRecodedNetCDF24");
	checkFullImageData(*img);
	test_untag();

	gen_ensure_imagedata_similar(*img->data, *imgs[0]->data, 0.0001);
#endif
}

// Import a subarea of an XRIT product and pass it from netcdf24
template<> template<>
void to::test<7>()
{
	ImageVector imgs(*croppedImporter());
	gen_ensure_equals(imgs.size(), 1u);
	std::auto_ptr<Image> img = recodeThroughNetCDF24(*imgs[0]);

	gen_ensure_equals(img->defaultFilename, "MSG1_Seviri_HRV_channel_20061114_1200");

	//gen_ensure_equals(img->name, ""); // unverified
	gen_ensure_equals(img->column_res, facts::columnResFromSeviriDX(facts::seviriDXFromColumnRes(40927014*exp2(-16))));
	gen_ensure_equals(img->line_res, facts::lineResFromSeviriDY(facts::seviriDYFromLineRes(40927014*exp2(-16))));
	gen_ensure_similar(img->data->slope, 0.031999f, 0.00001);
	gen_ensure_similar(img->data->offset, -1.63196f, 0.00001);
	gen_ensure_equals(img->data->bpp, 9);
	gen_ensure_equals(img->data->scalesToInt, true);

	test_tag("croppedXRITRecodedNetCDF24");
	checkCroppedImageData(*img);
	test_untag();

	gen_ensure_imagedata_similar(*img->data, *imgs[0]->data, 0.0001);
}

}

/* vim:set ts=4 sw=4: */
