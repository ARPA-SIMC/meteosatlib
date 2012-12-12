/*
 * Copyright (C) 2005--2010  ARPA-SIM <urpsim@smr.arpa.emr.it>
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

#define PERFORM_SLOW_TESTS

using namespace std;

namespace tut {

#define TESTFILE "H:MSG2:VIS006:200807150900"

struct importxrit_shar : public ImportTest
{
	importxrit_shar() : ImportTest("MsatXRIT", TESTFILE)
	{
	}

	~importxrit_shar()
	{
	}

#define native_offset -1.02691
#define native_scale 0.0201355

        void checkGeneralImageData(GDALDataset* dataset)
        {
                const char* val = dataset->GetMetadataItem(MD_MSAT_DATETIME, MD_DOMAIN_MSAT);
                gen_ensure(val != NULL);
                gen_ensure_equals(string(val), "2008-07-15 09:00:00");

                val = dataset->GetMetadataItem(MD_MSAT_SPACECRAFT_ID, MD_DOMAIN_MSAT);
                gen_ensure(val != NULL);
                gen_ensure_equals(string(val), "56");
                val = dataset->GetMetadataItem(MD_MSAT_SPACECRAFT, MD_DOMAIN_MSAT);
                gen_ensure(val != NULL);
                gen_ensure_equals(string(val), "MSG2");

                // TODO gen_ensure_equals(defaultFilename(*dataset), "MSG2_Seviri_1_5_VIS006_20080715_0900");
                // TODO gen_ensure_equals(defaultShortName(*dataset), "VIS006");

                GDALRasterBand* b = dataset->GetRasterBand(1);

                // TODO gen_ensure_equals(defaultFilename(*b), "MSG2_Seviri_1_5_VIS006_20080715_0900");
                // TODO gen_ensure_equals(defaultShortName(*dataset), "VIS006");

                val = b->GetMetadataItem(MD_MSAT_CHANNEL_ID, MD_DOMAIN_MSAT);
                gen_ensure(val != NULL);
                gen_ensure_equals(string(val), "1");
                val = b->GetMetadataItem(MD_MSAT_CHANNEL, MD_DOMAIN_MSAT);
                gen_ensure(val != NULL);
                gen_ensure_equals(string(val), "VIS006");

                gen_ensure_equals(string(b->GetUnitType()), "mW m^-2 sr^-1 (cm^-1)^-1");

		if (b->GetRasterDataType() == GDT_Float64)
		{
			int valid;
			gen_ensure_equals(b->GetOffset(&valid), 0);
			gen_ensure_equals(valid, TRUE);
			gen_ensure_equals(b->GetScale(&valid), 1);
			gen_ensure_equals(valid, TRUE);
		} else {
			int valid;
			gen_ensure_similar(b->GetOffset(&valid), native_offset, 0.00001);
			gen_ensure_equals(valid, TRUE);
			gen_ensure_similar(b->GetScale(&valid), native_scale, 0.0000001);
			gen_ensure_equals(valid, TRUE);
		}

#if 0
                //gen_ensure_equals(img->defaultFilename, "H_MSG1_Seviri_HRV_channel_20061114_1200");
#endif
        }

        void checkFullImageData(GDALDataset* dataset)
        {
                checkGeneralImageData(dataset);

                gen_ensure_equals(dataset->GetRasterXSize(), 3712);
                gen_ensure_equals(dataset->GetRasterYSize(), 3712);

                int xs, ys;
                double rx, ry;
                msat::dataset::decodeGeotransform(dataset, xs, ys, rx, ry);
                gen_ensure_equals(xs, 1856);
                gen_ensure_equals(ys, 1856);
                gen_ensure_similar(rx, METEOSAT_PIXELSIZE_X, 0.0001);
                gen_ensure_similar(ry, METEOSAT_PIXELSIZE_Y, 0.0001);

                GDALRasterBand* b = dataset->GetRasterBand(1);

		if (b->GetRasterDataType() == GDT_Float64)
		{
			// Prescaled
			int valid;
			double missing = b->GetNoDataValue(&valid);
			gen_ensure_equals(valid, TRUE);
			gen_ensure_similar(missing, -1.02691, 0.00001);
			gen_ensure_similar(readFloat32(b, 0, 0), missing, 0.00001);
			gen_ensure_similar(readFloat32(b, 10, 10), missing, 0.00001);

			gen_ensure_similar(readFloat32(b, 1800, 3100), missing, 0.00001);  // Missing segment
			gen_ensure_similar(readFloat32(b,  500, 3500), missing, 0.00001);  // Missing value on existing segment
#define SC(x) ((x)*native_scale + native_offset)
			gen_ensure_similar(readFloat32(b,  900, 3300), SC(51), 0.0001);       // Dark surface
			gen_ensure_similar(readFloat32(b, 2050, 3345), SC(79), 0.0001);       // Dark spot
			gen_ensure_similar(readFloat32(b, 2500, 3350), SC(237), 0.0001);      // Bright spot
#undef SC
		} else {
			// Packed
			int valid;
			int32_t missing = b->GetNoDataValue(&valid);
			gen_ensure_equals(valid, TRUE);
			gen_ensure_equals(missing, 0);
			gen_ensure_equals(readInt32(b, 0, 0), missing);
			gen_ensure_equals(readInt32(b, 10, 10), missing);

			gen_ensure_equals(readInt32(b, 1800, 3100), missing);  // Missing segment
			gen_ensure_equals(readInt32(b,  500, 3500), missing);  // Missing value on existing segment
			gen_ensure_equals(readInt32(b,  900, 3300), 51);       // Dark surface
			gen_ensure_equals(readInt32(b, 2050, 3345), 79);       // Dark spot
			gen_ensure_equals(readInt32(b, 2500, 3350), 237);      // Bright spot
		}

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

        void checkCroppedImageData(GDALDataset* dataset)
        {
                checkGeneralImageData(dataset);

                gen_ensure_equals(dataset->GetRasterXSize(), 150);
                gen_ensure_equals(dataset->GetRasterYSize(), 300);

                int xs, ys;
                double rx, ry;
                msat::dataset::decodeGeotransform(dataset, xs, ys, rx, ry);
                gen_ensure_equals(xs, 1856 - 2700);
                gen_ensure_equals(ys, 1856 - 3200);
                gen_ensure_similar(rx, METEOSAT_PIXELSIZE_X, 0.00001);
                gen_ensure_similar(ry, METEOSAT_PIXELSIZE_Y, 0.00001);

                GDALRasterBand* b = dataset->GetRasterBand(1);

		if (b->GetRasterDataType() == GDT_Float64)
		{
			// Prescaled
			int valid;
			double missing = b->GetNoDataValue(&valid);
			gen_ensure_equals(valid, TRUE);
			gen_ensure_similar(missing, -1.02691, 0.00001);

#define SC(x) ((x)*native_scale + native_offset)
			gen_ensure_similar(readFloat32(b,   0,   0), missing, 0.0001); // unverified
			gen_ensure_similar(readFloat32(b,  10,  20), missing, 0.0001); // unverified
			gen_ensure_similar(readFloat32(b,  20,  40), missing, 0.0001); // unverified
			gen_ensure_similar(readFloat32(b,  30,  60), SC(217), 0.0001); // unverified
			gen_ensure_similar(readFloat32(b,  40,  80), SC(196), 0.0001); // unverified
			gen_ensure_similar(readFloat32(b,  50, 100), SC(127), 0.0001); // unverified
			gen_ensure_similar(readFloat32(b,  60, 120), SC(171), 0.0001); // unverified
			gen_ensure_similar(readFloat32(b,  70, 140), SC(209), 0.0001); // unverified
			gen_ensure_similar(readFloat32(b,  80, 160), SC(223), 0.0001); // unverified
			gen_ensure_similar(readFloat32(b,  90, 180), SC(206), 0.0001); // unverified
			gen_ensure_similar(readFloat32(b, 100, 200), missing, 0.0001); // unverified
#undef SC
		} else {
			// Packed
			int valid;
			double missing = b->GetNoDataValue(&valid);
			gen_ensure_equals(missing, 0);
			gen_ensure_equals(valid, TRUE);

			gen_ensure_equals(readInt32(b,   0,   0), missing); // unverified
			gen_ensure_equals(readInt32(b,  10,  20), missing); // unverified
			gen_ensure_equals(readInt32(b,  20,  40), missing); // unverified
			gen_ensure_equals(readInt32(b,  30,  60), 217); // unverified
			gen_ensure_equals(readInt32(b,  40,  80), 196); // unverified
			gen_ensure_equals(readInt32(b,  50, 100), 127); // unverified
			gen_ensure_equals(readInt32(b,  60, 120), 171); // unverified
			gen_ensure_equals(readInt32(b,  70, 140), 209); // unverified
			gen_ensure_equals(readInt32(b,  80, 160), 223); // unverified
			gen_ensure_equals(readInt32(b,  90, 180), 206); // unverified
			gen_ensure_equals(readInt32(b, 100, 200), missing); // unverified
		}
                /*
                   gen_ensure_similar(b->scaled(  0,   0), 6.07987, 0.001); // unverified
                   gen_ensure_similar(b->scaled( 10,  20), 6.27186, 0.001); // unverified
                   gen_ensure_similar(b->scaled( 20,  40), 4.73590, 0.001); // unverified
                   gen_ensure_similar(b->scaled( 30,  60), b->missingValue, 0.001); // unverified
                   gen_ensure_similar(b->scaled( 40,  80), b->missingValue, 0.001); // unverified
                   gen_ensure_similar(b->scaled( 50, 100), b->missingValue, 0.001); // unverified
                   gen_ensure_similar(b->scaled( 60, 120), 5.79187, 0.001); // unverified
                   gen_ensure_similar(b->scaled( 70, 140), 5.05589, 0.001); // unverified
                   gen_ensure_similar(b->scaled( 80, 160), 4.57590, 0.001); // unverified
                   gen_ensure_similar(b->scaled( 90, 180), 4.86389, 0.01); // unverified
                   gen_ensure_similar(b->scaled(100, 200), 5.27988, 0.01); // unverified
                   */
        }
};
TESTGRP(importxrit);

static msat::proj::ImageBox cropArea(msat::proj::ImagePoint(2700, 3200), msat::proj::ImagePoint(2700+150, 3200+300));

// Test opening
template<> template<>
void to::test<1>()
{
	auto_ptr<GDALDataset> dataset = openDS();
	gen_ensure(dataset.get() != 0);
	gen_ensure_equals(string(GDALGetDriverShortName(dataset->GetDriver())), "MsatXRIT");

	// Check that we have a raster band of the proper type
	gen_ensure_equals(dataset->GetRasterCount(), 1);
}

// Import a full XRIT product
template<> template<>
void to::test<2>()
{
	test_tag("fullXRIT");
	auto_ptr<GDALDataset> dataset = openPlain();
	test_untag();

	// TODO gen_ensure_equals(b->getOriginalBpp(), 10);

	////gen_ensure_equals(img->name, ""); // unverified
	//gen_ensure_equals(img->column_res, 40927014*exp2(-16));
	//gen_ensure_equals(img->line_res, 40927014*exp2(-16));

#if 0
	-- look for nonempty values
	using namespace std;
	for (size_t y = 0; y < img->data->lines; ++y)
		for (size_t x = 0; x < img->data->columns; ++x)
			if (img->data->scaled(x, y) != 0)
				cout << "(" << x << ", " << y << "): " << img->data->scaled(x, y) << endl;
#endif
}

#if 0
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
#endif

// Try reimporting an exported grib (msat/msat)
template<> template<>
void to::test<3>()
{
#ifdef PERFORM_SLOW_TESTS
	test_tag("fullHritRecodedGribMsat");
        auto_ptr<GDALDataset> dataset = openRecoded("MsatGRIB", false, "TEMPLATE=msat/msat");
	test_untag();

        //GDALRasterBand* b = dataset->GetRasterBand(1);
        // TODO gen_ensure_equals(b->getOriginalBpp(), 16);
#if 0
	//gen_ensure_equals(img->name, ""); // unverified
	gen_ensure_equals(img->column_factor, Image::columnFactorFromSeviriDX(Image::seviriDXFromColumnFactor(40927014)));
	gen_ensure_equals(img->line_factor, Image::lineFactorFromSeviriDY(Image::seviriDYFromLineFactor(40927014)));
	gen_ensure_equals(img->data->slope, 1);
	gen_ensure_equals(img->data->offset, 0);
	gen_ensure_equals(img->data->bpp, 32); // unverified
	gen_ensure_equals(img->data->scalesToInt, false);
	gen_ensure_imagedata_similar(*img->data, *imgs[0]->data, 0.0001);
#endif
#endif
}

// Try reimporting an exported grib (msat/ecmwf)
template<> template<>
void to::test<4>()
{
#ifdef PERFORM_SLOW_TESTS
	test_tag("fullHritRecodedGribEcmwf");
        auto_ptr<GDALDataset> dataset = openRecoded("MsatGRIB", false, "TEMPLATE=msat/ecmwf");
	test_untag();

        //GDALRasterBand* b = dataset->GetRasterBand(1);
	// TODO gen_ensure_equals(b->getOriginalBpp(), 16);
#if 0
	//gen_ensure_equals(img->name, ""); // unverified
	gen_ensure_equals(img->column_factor, Image::columnFactorFromSeviriDX(Image::seviriDXFromColumnFactor(40927014)));
	gen_ensure_equals(img->line_factor, Image::lineFactorFromSeviriDY(Image::seviriDYFromLineFactor(40927014)));
	gen_ensure_equals(img->data->slope, 1);
	gen_ensure_equals(img->data->offset, 0);
	gen_ensure_equals(img->data->bpp, 32); // unverified
	gen_ensure_equals(img->data->scalesToInt, false);
	gen_ensure_imagedata_similar(*img->data, *imgs[0]->data, 0.0001);
#endif
#endif
}

// Import a subarea of an XRIT product and pass it from grib (msat/msat)
template<> template<>
void to::test<5>()
{
	test_tag("croppedXRITRecodedGribMsat");
        auto_ptr<GDALDataset> dataset = openRecodedCropped(cropArea, "MsatGRIB", false, "TEMPLATE=msat/msat");
	test_untag();

        //GDALRasterBand* b = dataset->GetRasterBand(1);
	// TODO gen_ensure_equals(b->getOriginalBpp(), 16);
#if 0
	gen_ensure_equals(img->defaultFilename, "MSG1_Seviri_HRV_channel_20061114_1200");

	//gen_ensure_equals(img->name, ""); // unverified
	gen_ensure_equals(img->column_res, Image::columnResFromSeviriDX(Image::seviriDXFromColumnRes(40927014*exp2(-16))));
	gen_ensure_equals(img->line_res, Image::lineResFromSeviriDY(Image::seviriDYFromLineRes(40927014*exp2(-16))));
	gen_ensure_similar(img->data->slope, 0.0001, 0.0000001);
	//gen_ensure_equals(img->data->offset, -3.3f);
	gen_ensure_similar(img->data->offset, -3.2959, 0.00001);
	gen_ensure_equals(img->data->bpp, 16); // unverified
	gen_ensure_equals(img->data->scalesToInt, true);

	gen_ensure_imagedata_similar(*img->data, *origimg->data, 0.01);
#endif
}

// Import a subarea of an XRIT product and pass it from grib (msat/ecmwf)
template<> template<>
void to::test<6>()
{
	test_tag("croppedXRITRecodedGribEcmwf");
        auto_ptr<GDALDataset> dataset = openRecodedCropped(cropArea, "MsatGRIB", false, "TEMPLATE=msat/ecmwf");
	test_untag();

        //GDALRasterBand* b = dataset->GetRasterBand(1);
	// TODO gen_ensure_equals(b->getOriginalBpp(), 16);
#if 0
	gen_ensure_equals(img->defaultFilename, "MSG1_Seviri_HRV_channel_20061114_1200");

	//gen_ensure_equals(img->name, ""); // unverified
	gen_ensure_equals(img->column_res, Image::columnResFromSeviriDX(Image::seviriDXFromColumnRes(40927014*exp2(-16))));
	gen_ensure_equals(img->line_res, Image::lineResFromSeviriDY(Image::seviriDYFromLineRes(40927014*exp2(-16))));
	gen_ensure_similar(img->data->slope, 0.0001, 0.0000001);
	//gen_ensure_equals(img->data->offset, -3.3f);
	gen_ensure_similar(img->data->offset, -3.2959, 0.00001);
	gen_ensure_equals(img->data->bpp, 16); // unverified
	gen_ensure_equals(img->data->scalesToInt, true);

	gen_ensure_imagedata_similar(*img->data, *origimg->data, 0.01);
#endif
}

// Try reimporting an exported netcdf24
template<> template<>
void to::test<7>()
{
#ifdef PERFORM_SLOW_TESTS
	test_tag("fulliHritRecodedNetCDF24");
        auto_ptr<GDALDataset> dataset = openRecoded("MsatNetCDF24", false);
	test_untag();

        GDALRasterBand* b = dataset->GetRasterBand(1);
	gen_ensure_equals(b->GetRasterDataType(), GDT_UInt16);

	// TODO gen_ensure_equals(b->getOriginalBpp(), 16);
#if 0
	gen_ensure_equals(img->defaultFilename, "MSG1_Seviri_HRV_channel_20061114_1200");

	//gen_ensure_equals(img->name, ""); // unverified
	gen_ensure_equals(img->column_factor, 40927014);
	gen_ensure_equals(img->line_factor, 40927014);
	gen_ensure_similar(img->data->slope, 0.031999f, 0.00001);
	gen_ensure_similar(img->data->offset, -1.63196f, 0.00001);
	gen_ensure_equals(img->data->scalesToInt, false);
	gen_ensure_imagedata_similar(*img->data, *imgs[0]->data, 0.0001);
#endif
#endif
}

// Import a subarea of an XRIT product and pass it from netcdf24
template<> template<>
void to::test<8>()
{
	test_tag("croppedXRITRecodedNetCDF24");
        auto_ptr<GDALDataset> dataset = openRecodedCropped(cropArea, "MsatNetCDF24", false);
	test_untag();

        GDALRasterBand* b = dataset->GetRasterBand(1);
	gen_ensure_equals(b->GetRasterDataType(), GDT_UInt16);
	// TODO gen_ensure_equals(b->getOriginalBpp(), 16);
#if 0
	gen_ensure_equals(img->defaultFilename, "MSG1_Seviri_HRV_channel_20061114_1200");

	//gen_ensure_equals(img->name, ""); // unverified
	gen_ensure_equals(img->column_res, Image::columnResFromSeviriDX(Image::seviriDXFromColumnRes(40927014*exp2(-16))));
	gen_ensure_equals(img->line_res, Image::lineResFromSeviriDY(Image::seviriDYFromLineRes(40927014*exp2(-16))));
	gen_ensure_similar(img->data->slope, 0.031999f, 0.00001);
	gen_ensure_similar(img->data->offset, -1.63196f, 0.00001);
	gen_ensure_equals(img->data->bpp, 9);
	gen_ensure_equals(img->data->scalesToInt, true);

	gen_ensure_imagedata_similar(*img->data, *imgs[0]->data, 0.0001);
#endif
}

// Try reimporting an exported netcdf
template<> template<>
void to::test<9>()
{
#ifdef PERFORM_SLOW_TESTS
	test_tag("fullHritRecodedNetCDF");
        auto_ptr<GDALDataset> dataset = openRecoded("MsatNetCDF", false);
	test_untag();

        //GDALRasterBand* b = dataset->GetRasterBand(1);
	// TODO gen_ensure_equals(b->GetRasterDataType(), GDT_Int16);

	// TODO gen_ensure_equals(b->getOriginalBpp(), 16);
#if 0
	gen_ensure_equals(img->defaultFilename, "MSG1_Seviri_HRV_channel_20061114_1200");

	//gen_ensure_equals(img->name, ""); // unverified
	gen_ensure_equals(img->column_factor, 40927014);
	gen_ensure_equals(img->line_factor, 40927014);
	gen_ensure_similar(img->data->slope, 0.031999f, 0.00001);
	gen_ensure_similar(img->data->offset, -1.63196f, 0.00001);
	gen_ensure_equals(img->data->scalesToInt, false);

	gen_ensure_imagedata_similar(*img->data, *imgs[0]->data, 0.0001);
#endif
#endif
}

// Import a subarea of an XRIT product and pass it from netcdf
template<> template<>
void to::test<10>()
{
	test_tag("croppedXRITRecodedNetCDF");
        auto_ptr<GDALDataset> dataset = openRecodedCropped(cropArea, "MsatNetCDF", false);
	test_untag();

        GDALRasterBand* b = dataset->GetRasterBand(1);
	// TODO gen_ensure_equals(b->GetRasterDataType(), GDT_Int16);
	// TODO gen_ensure_equals(b->getOriginalBpp(), 16);
#if 0
	gen_ensure_equals(img->defaultFilename, "MSG1_Seviri_HRV_channel_20061114_1200");

	//gen_ensure_equals(img->name, ""); // unverified
	gen_ensure_equals(img->column_res, 40927014*exp2(-16));
	gen_ensure_equals(img->line_res, 40927014*exp2(-16));
	gen_ensure_equals(img->data->slope, 1);
	gen_ensure_equals(img->data->offset, 0);
	gen_ensure_equals(img->data->scalesToInt, false);

	gen_ensure_imagedata_similar(*img->data, *imgs[0]->data, 0.0001);
#endif
}

}

/* vim:set ts=4 sw=4: */
