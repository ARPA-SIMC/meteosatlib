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

#undef PERFORM_SLOW_TESTS

using namespace std;

namespace tut {

#define TESTFILE "H:MSG1:HRV:200611141200"

struct importxrithrv_shar : public ImportTest
{
	importxrithrv_shar() : ImportTest("MsatXRIT", TESTFILE)
	{
	}

	~importxrithrv_shar()
	{
	}

#define native_offset -1.63196
#define native_scale 0.0319993

	void checkGeneralImageData(GDALDataset* dataset)
	{
                const char* val = dataset->GetMetadataItem(MD_MSAT_DATETIME, MD_DOMAIN_MSAT);
                gen_ensure(val != NULL);
                gen_ensure_equals(string(val), "2006-11-14 12:00:00");

                val = dataset->GetMetadataItem(MD_MSAT_SPACECRAFT_ID, MD_DOMAIN_MSAT);
                gen_ensure(val != NULL);
                gen_ensure_equals(string(val), "55");
                val = dataset->GetMetadataItem(MD_MSAT_SPACECRAFT, MD_DOMAIN_MSAT);
                gen_ensure(val != NULL);
                gen_ensure_equals(string(val), "MSG1");

		// TODO gen_ensure_equals(defaultFilename(*dataset), "MSG1_Seviri_1_5_HRV_20061114_1200");
		// TODO gen_ensure_equals(defaultShortName(*dataset), "HRV");

                GDALRasterBand* b = dataset->GetRasterBand(1);

		// TODO gen_ensure_equals(defaultFilename(*b), "MSG1_Seviri_1_5_HRV_20061114_1200");
		// TODO gen_ensure_equals(defaultShortName(*dataset), "HRV");

                val = b->GetMetadataItem(MD_MSAT_CHANNEL_ID, MD_DOMAIN_MSAT);
                gen_ensure(val != NULL);
                gen_ensure_equals(string(val), "12");
                val = b->GetMetadataItem(MD_MSAT_CHANNEL, MD_DOMAIN_MSAT);
                gen_ensure(val != NULL);
                gen_ensure_equals(string(val), "HRV");

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

		gen_ensure_equals(dataset->GetRasterXSize(), 11136);
		gen_ensure_equals(dataset->GetRasterYSize(), 11136);

		int xs, ys;
		double rx, ry;
                msat::dataset::decodeGeotransform(dataset, xs, ys, rx, ry);
		gen_ensure_equals(xs, 5568);
		gen_ensure_equals(ys, 5568);
		gen_ensure_similar(rx, METEOSAT_PIXELSIZE_X_HRV, 0.0001);
		gen_ensure_similar(ry, METEOSAT_PIXELSIZE_Y_HRV, 0.0001);

                GDALRasterBand* b = dataset->GetRasterBand(1);

		if (b->GetRasterDataType() == GDT_Float64)
		{
			int valid;
			double missing = b->GetNoDataValue(&valid);
			gen_ensure_equals(valid, TRUE);
			gen_ensure_equals(missing, -1.63196);
#define SC(x) ((x)*native_scale + native_offset)
			gen_ensure_similar(readFloat32(b, 0, 0), missing, 0.00001);
			gen_ensure_similar(readFloat32(b, 10, 10), missing, 0.00001);

			gen_ensure_similar(readFloat32(b,  5000, 2000), missing, 0.00001); // Missing segment before Europe part
			gen_ensure_similar(readFloat32(b,  2500, 2900), missing, 0.00001); // Missing data left of Europe part
			gen_ensure_similar(readFloat32(b,  4000, 2900), SC( 90), 0.00001); // Dark spot in Europe part
			gen_ensure_similar(readFloat32(b,  4800, 2900), SC(286), 0.00001); // Bright spot in Europe part
			gen_ensure_similar(readFloat32(b,  9200, 2900), missing, 0.00001); // Missing data right of Europe part
			gen_ensure_similar(readFloat32(b,  7000, 3030), missing, 0.00001); // Missing data between the two parts
			gen_ensure_similar(readFloat32(b,  5300, 3150), missing, 0.00001); // Missing data left of Africa part
			gen_ensure_similar(readFloat32(b,  8600, 3111), SC(100), 0.00001); // Dark spot in Africa part
			// 90 in stabilised area
			gen_ensure_similar(readFloat32(b,  8300, 3150), SC(254), 0.00001); // Bright spot in Africa part
			// 252 in stabilised area
			gen_ensure_similar(readFloat32(b, 10410, 3100), missing, 0.00001); // Missing data right of Africa part
			gen_ensure_similar(readFloat32(b,  6400, 3500), missing, 0.00001); // Missing segment after Africa part
#undef SC
		} else {
			int valid;
			int missing = b->GetNoDataValue(&valid);
			gen_ensure_equals(valid, TRUE);
			gen_ensure_equals(missing, 0);

			gen_ensure_equals(readInt32(b, 0, 0), missing);
			gen_ensure_equals(readInt32(b, 10, 10), missing);

			gen_ensure_equals(readInt32(b,  5000, 2000), missing); // Missing segment before Europe part
			gen_ensure_equals(readInt32(b,  2500, 2900), missing); // Missing data left of Europe part
			gen_ensure_equals(readInt32(b,  4000, 2900), 90); // Dark spot in Europe part
			gen_ensure_equals(readInt32(b,  4800, 2900), 286); // Bright spot in Europe part
			gen_ensure_equals(readInt32(b,  9200, 2900), missing); // Missing data right of Europe part
			gen_ensure_equals(readInt32(b,  7000, 3030), missing); // Missing data between the two parts
			gen_ensure_equals(readInt32(b,  5300, 3150), missing); // Missing data left of Africa part
			gen_ensure_equals(readInt32(b,  8600, 3111),  90); // Dark spot in Africa part
			gen_ensure_equals(readInt32(b,  8300, 3150), 252); // Bright spot in Africa part
			gen_ensure_equals(readInt32(b, 10410, 3100), missing); // Missing data right of Africa part
			gen_ensure_equals(readInt32(b,  6400, 3500), missing); // Missing segment after Africa part
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
		gen_ensure_equals(xs, 5568 - 9000);
		gen_ensure_equals(ys, 5568 - 2900);
		gen_ensure_similar(rx, METEOSAT_PIXELSIZE_X_HRV, 0.01);
		gen_ensure_similar(ry, METEOSAT_PIXELSIZE_Y_HRV, 0.01);

                GDALRasterBand* b = dataset->GetRasterBand(1);

		if (b->GetRasterDataType() == GDT_Float64)
		{
			int valid;
			double missing = b->GetNoDataValue(&valid);
			gen_ensure_equals(missing, -1.63196);
			gen_ensure_equals(valid, TRUE);

#define SC(x) ((x)*native_scale + native_offset)
			gen_ensure_similar(readFloat32(b,  31,  19), SC(116), 0.0001); // Dark spot in Europe part
			gen_ensure_similar(readFloat32(b,  31,  27), SC(210), 0.0001); // Bright spot in Europe part
			gen_ensure_similar(readFloat32(b, 100,  60), missing, 0.0001); // Missing data right of Europe part
			gen_ensure_similar(readFloat32(b,  20, 125), missing, 0.0001); // Missing data between the two parts
			gen_ensure_similar(readFloat32(b,  84, 148), SC(138), 0.0001); // Dark spot in Africa part
			gen_ensure_similar(readFloat32(b,  72, 143), SC(219), 0.0001); // Bright spot in Africa part
#undef SC
		} else {
			int valid;
			int missing = b->GetNoDataValue(&valid);
			gen_ensure_equals(missing, 0);
			gen_ensure_equals(valid, TRUE);

			gen_ensure_equals(readInt32(b,  31,  19), 116); // Dark spot in Europe part
			gen_ensure_equals(readInt32(b,  31,  27), 210); // Bright spot in Europe part
			gen_ensure_equals(readInt32(b, 100,  60), missing); // Missing data right of Europe part
			gen_ensure_equals(readInt32(b,  20, 125), missing); // Missing data between the two parts
			gen_ensure_equals(readInt32(b,  84, 148), 138); // Dark spot in Africa part
			gen_ensure_equals(readInt32(b,  72, 143), 219); // Bright spot in Africa part
		}
	}
};
TESTGRP(importxrithrv);

static msat::proj::ImageBox cropArea(msat::proj::ImagePoint(9000, 2900), msat::proj::ImagePoint(9000+150, 2900+300));

// Test opening
template<> template<>
void to::test<1>()
{
    unique_ptr<GDALDataset> dataset = openDS();
    gen_ensure(dataset.get() != 0);
    gen_ensure_equals(string(GDALGetDriverShortName(dataset->GetDriver())), "MsatXRIT");

    // Check that we have a raster band of the proper type
    gen_ensure_equals(dataset->GetRasterCount(), 1);
}

// Import a full XRIT product
template<> template<>
void to::test<2>()
{
    test_tag("fullHRV");
    unique_ptr<GDALDataset> dataset = openPlain();
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
    std::unique_ptr<Image> img = imgs.shift();

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
    test_tag("fullHRVRecodedGribMsat");
    unique_ptr<GDALDataset> dataset = openRecoded("MsatGRIB", false, "TEMPLATE=msat/msat");
    test_untag();

    // TODO gen_ensure_equals(b->getOriginalBpp(), 10);
#if 0
    gen_ensure_equals(img->defaultFilename, "MSG1_Seviri_HRV_channel_20061114_1200");

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
    test_tag("fullHritHRVRecodedGribEcmwf");
    unique_ptr<GDALDataset> dataset = openRecoded("MsatGRIB", false, "TEMPLATE=msat/ecmwf");
    test_untag();

    // TODO gen_ensure_equals(b->getOriginalBpp(), 10);
#if 0
    gen_ensure_equals(img->defaultFilename, "MSG1_Seviri_HRV_channel_20061114_1200");

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
    test_tag("croppedXRITHRVRecodedGribMsat");
    unique_ptr<GDALDataset> dataset = openRecodedCropped(cropArea, "MsatGRIB", false, "TEMPLATE=msat/msat");
    test_untag();

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
    test_tag("croppedXRITHRVRecodedGribEcmwf");
    unique_ptr<GDALDataset> dataset = openRecodedCropped(cropArea, "MsatGRIB", false, "TEMPLATE=msat/ecmwf");
    test_untag();

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
    test_tag("fulliHritHRVRecodedNetCDF24");
    unique_ptr<GDALDataset> dataset = openRecoded("MsatNetCDF24", false);
    test_untag();

    // TODO gen_ensure_equals(b->getOriginalBpp(), 10);
#if 0
    gen_ensure_equals(img->defaultFilename, "MSG1_Seviri_HRV_channel_20061114_1200");

    //gen_ensure_equals(img->name, ""); // unverified
    gen_ensure_equals(img->column_factor, 40927014);
    gen_ensure_equals(img->line_factor, 40927014);
    gen_ensure_similar(img->data->slope, 0.031999f, 0.00001);
    gen_ensure_similar(img->data->offset, -1.63196f, 0.00001);
    gen_ensure_equals(img->data->bpp, 32); // unverified
    gen_ensure_equals(img->data->scalesToInt, false);

    gen_ensure_imagedata_similar(*img->data, *imgs[0]->data, 0.0001);
#endif
#endif
}

// Import a subarea of an XRIT product and pass it from netcdf24
template<> template<>
void to::test<8>()
{
    test_tag("croppedXRITHRVRecodedNetCDF24");
    unique_ptr<GDALDataset> dataset = openRecodedCropped(cropArea, "MsatNetCDF24", false);
    test_untag();

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
    test_tag("fullHritHRVRecodedNetCDF");
    unique_ptr<GDALDataset> dataset = openRecoded("MsatNetCDF", false);
    test_untag();

    // TODO gen_ensure_equals(b->getOriginalBpp(), 16);
#if 0
    gen_ensure_equals(img->defaultFilename, "MSG1_Seviri_HRV_channel_20061114_1200");

    //gen_ensure_equals(img->name, ""); // unverified
    gen_ensure_equals(img->column_factor, 40927014);
    gen_ensure_equals(img->line_factor, 40927014);
    gen_ensure_similar(img->data->slope, 0.031999f, 0.00001);
    gen_ensure_similar(img->data->offset, -1.63196f, 0.00001);
    gen_ensure_equals(img->data->bpp, 32); // unverified
    gen_ensure_equals(img->data->scalesToInt, false);

    gen_ensure_imagedata_similar(*img->data, *imgs[0]->data, 0.0001);
#endif
#endif
}

// Import a subarea of an XRIT product and pass it from netcdf
template<> template<>
void to::test<10>()
{
    test_tag("croppedXRITHRVRecodedNetCDF");
    unique_ptr<GDALDataset> dataset = openRecodedCropped(cropArea, "MsatNetCDF", false);
    test_untag();

    // TODO gen_ensure_equals(b->getOriginalBpp(), 16);
#if 0
    gen_ensure_equals(img->defaultFilename, "MSG1_Seviri_HRV_channel_20061114_1200");

    //gen_ensure_equals(img->name, ""); // unverified
    gen_ensure_equals(img->column_res, 40927014*exp2(-16));
    gen_ensure_equals(img->line_res, 40927014*exp2(-16));
    gen_ensure_equals(img->data->slope, 1);
    gen_ensure_equals(img->data->offset, 0);
    gen_ensure_equals(img->data->bpp, 32);
    gen_ensure_equals(img->data->scalesToInt, false);

    gen_ensure_imagedata_similar(*img->data, *imgs[0]->data, 0.0001);
#endif
}

// Reproduce a bug
template<> template<>
void to::test<11>()
{
#if 0
    unique_ptr<msat::Image> dataset = openro(TESTFILE);
    unique_ptr<msat::Image> dataset1 = recode(*dataset, cropArea, false, "GRIB", "TEMPLATE=msat/msat");

    msat::grib::Dataset* d = dynamic_cast<msat::grib::Dataset*>(dataset1.get());
    gen_ensure(d);

    msat::grib::Grib& g = d->handle();
    gen_ensure_equals(g.get_long_oneof("Xo", "geometry.xo"), 9000);
    gen_ensure_equals(g.get_long_oneof("Yo", "geometry.yo"), 2900);
#endif
}

}
