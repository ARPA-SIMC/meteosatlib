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

using namespace std;

namespace tut {

#define TESTFILE "MSG_Seviri_1_5_Infrared_10_8_channel_20051219_1415.nc"

struct importnetcdf_shar : public ImportTest
{
        importnetcdf_shar() : ImportTest("MsatNetCDF", TESTFILE)
        {
        }

        ~importnetcdf_shar()
        {
        }

        void checkGeneralImageData(GDALDataset* dataset)
        {
                const char* val = dataset->GetMetadataItem(MD_MSAT_DATETIME, MD_DOMAIN_MSAT);
                gen_ensure(val != NULL);
                gen_ensure_equals(string(val), "2005-12-19 14:15:00");

                val = dataset->GetMetadataItem(MD_MSAT_SPACECRAFT_ID, MD_DOMAIN_MSAT);
                gen_ensure(val != NULL);
                gen_ensure_equals(string(val), "55");
                val = dataset->GetMetadataItem(MD_MSAT_SPACECRAFT, MD_DOMAIN_MSAT);
                gen_ensure(val != NULL);
                gen_ensure_equals(string(val), "MSG1");

                // TODO gen_ensure_equals(defaultFilename(*dataset), "MSG1_Seviri_1_5_IR_108_20051219_1415");
                // TODO gen_ensure_equals(defaultShortName(*dataset), "IR_108");

                GDALRasterBand* b = dataset->GetRasterBand(1);
                gen_ensure(b != NULL);

                // TODO gen_ensure_equals(defaultFilename(*b), "MSG1_Seviri_1_5_IR_108_20051219_1415");
                // TODO gen_ensure_equals(defaultShortName(*dataset), "IR_108");

                val = b->GetMetadataItem(MD_MSAT_CHANNEL_ID, MD_DOMAIN_MSAT);
                gen_ensure(val != NULL);
                gen_ensure_equals(string(val), "9");
                val = b->GetMetadataItem(MD_MSAT_CHANNEL, MD_DOMAIN_MSAT);
                gen_ensure(val != NULL);
                gen_ensure_equals(string(val), "IR_108");

                gen_ensure_equals(string(b->GetUnitType()), "K");

                int valid;
                gen_ensure_equals(b->GetOffset(&valid), 0);
                gen_ensure_equals(valid, TRUE);
                gen_ensure_equals(b->GetScale(&valid), 1);
                gen_ensure_equals(valid, TRUE);

                // gen_ensure_equals(b->scalesToInt(), false);

#if 0
                gen_ensure_equals(b->column_offset, 1856);
                gen_ensure_equals(b->line_offset, 1856);
                gen_ensure_equals(b->column_res, Image::columnResFromSeviriDX(3608));
                gen_ensure_equals(b->line_res, Image::columnResFromSeviriDX(3608));
                gen_ensure_equals(b->x0, 1499);
                gen_ensure_equals(b->y0, 199);
#endif

#if 0
                gen_ensure_equals(img.defaultFilename, "MSG1_Seviri_IR_108_channel_20051219_1415");
#endif
        }

        void checkFullImageData(GDALDataset* dataset)
        {
                checkGeneralImageData(dataset);

                gen_ensure_equals(dataset->GetRasterXSize(), 1300);
                gen_ensure_equals(dataset->GetRasterYSize(), 700);

                int xs, ys;
                double rx, ry;
                msat::dataset::decodeGeotransform(dataset, xs, ys, rx, ry);
                gen_ensure_equals(xs, 1856 - 1499);
                gen_ensure_equals(ys, 1856 - 199);
                gen_ensure_equals(rx, METEOSAT_PIXELSIZE_X);
                gen_ensure_equals(ry, METEOSAT_PIXELSIZE_Y);

                GDALRasterBand* b = dataset->GetRasterBand(1);

                int valid;
                gen_ensure_equals(b->GetOffset(&valid), 0);
                gen_ensure_equals(valid, TRUE);
                gen_ensure_equals(b->GetScale(&valid), 1);
                gen_ensure_equals(valid, TRUE);
                double missing = b->GetNoDataValue(&valid);
                gen_ensure_equals(missing, 0);
                gen_ensure_equals(valid, TRUE);

                gen_ensure_similar(readFloat32(b, 0, 0), 230.3, 0.01);
                gen_ensure_similar(readFloat32(b, 10, 10), 240.6, 0.01);
                gen_ensure_similar(readFloat32(b, 1299, 0), missing, 0.000001);
        }

        void checkCroppedImageData(GDALDataset* dataset)
        {
                checkGeneralImageData(dataset);

                gen_ensure_equals(dataset->GetRasterXSize(), 200);
                gen_ensure_equals(dataset->GetRasterYSize(), 50);

                int xs, ys;
                double rx, ry;
                msat::dataset::decodeGeotransform(dataset, xs, ys, rx, ry);
                gen_ensure_equals(xs, 1856 - 1599);
                gen_ensure_equals(ys, 1856 - 299);
                gen_ensure_equals(rx, METEOSAT_PIXELSIZE_X);
                gen_ensure_equals(ry, METEOSAT_PIXELSIZE_Y);

                GDALRasterBand* b = dataset->GetRasterBand(1);

                int valid;
                gen_ensure_equals(b->GetOffset(&valid), 0);
                gen_ensure_equals(valid, TRUE);
                gen_ensure_equals(b->GetScale(&valid), 1);
                gen_ensure_equals(valid, TRUE);
                double missing = b->GetNoDataValue(&valid);
                gen_ensure_equals(missing, 0);
                gen_ensure_equals(valid, TRUE);

                gen_ensure_similar(readFloat32(b, 0, 0), 251.4, 0.01);
                gen_ensure_similar(readFloat32(b, 10, 10), 266.5, 0.01);
        }
};
TESTGRP(importnetcdf);

static msat::proj::ImageBox cropArea(msat::proj::ImagePoint(100, 100), msat::proj::ImagePoint(100+200, 100+50));

// Test opening
template<> template<>
void to::test<1>()
{
	FOR_DRIVER("MsatNetCDF");
	auto_ptr<GDALDataset> dataset = openDS();
	gen_ensure(dataset.get() != 0);
	gen_ensure_equals(string(GDALGetDriverShortName(dataset->GetDriver())), "MsatNetCDF");
	gen_ensure_equals(dataset->GetRasterCount(), 1);
}

// Import a full NetCDF product
template<> template<>
void to::test<2>()
{
	test_tag("fullNetCDF");
	auto_ptr<GDALDataset> dataset = openPlain();
	test_untag();

	GDALRasterBand* b = dataset->GetRasterBand(1);
	gen_ensure_equals(b->GetRasterDataType(), GDT_Float32);
	gen_ensure_equals(b->GetOffset(), 0);
	gen_ensure_equals(b->GetScale(), 1);
	// TODO gen_ensure_equals(b->getOriginalBpp(), 32);
}

// Try reimporting an exported grib (msat/msat)
template<> template<>
void to::test<3>()
{
	test_tag("fullNetCDFRecodedGribMsat");
        auto_ptr<GDALDataset> dataset = openRecoded("MsatGRIB", false, "TEMPLATE=msat/msat");
	test_untag();

	// Set it here because in the test data it's missing
	// TODO dataset->GetRasterBand(1)->SetNoDataValue(1);

	GDALRasterBand* b = dataset->GetRasterBand(1);
	gen_ensure_equals(b->GetRasterDataType(), GDT_Float64);

	//gen_ensure_similar(b->GetOffset(), -1, 0.00001);
	gen_ensure_similar(b->GetOffset(), 0, 0.00001);
	//gen_ensure_similar(b->GetScale(), 0.01, 0.0001);
	gen_ensure_similar(b->GetScale(), 1, 0.0001);
	//gen_ensure_equals(b->getOriginalBpp(), 15);
	// TODO gen_ensure_equals(b->getOriginalBpp(), 16);
#if 0
	gen_ensure_equals(img->data->scalesToInt, true);
	gen_ensure_imagedata_similar(*img->data, *imgs[0]->data, 0.0001);
#endif
}

// Try reimporting an exported grib (msat/ecmwf)
template<> template<>
void to::test<4>()
{
	test_tag("fullNetCDFRecodedGribEcmwf");
        auto_ptr<GDALDataset> dataset = openRecoded("MsatGRIB", false, "TEMPLATE=msat/ecmwf");
	test_untag();

	GDALRasterBand* b = dataset->GetRasterBand(1);
	gen_ensure_equals(b->GetRasterDataType(), GDT_Float64);

	// Set it here because in the test data it's missing
	// TODO dataset->GetRasterBand(1)->SetNoDataValue(1);

	//gen_ensure_similar(b->GetOffset(), -1, 0.00001);
	gen_ensure_similar(b->GetOffset(), 0, 0.00001);
	//gen_ensure_similar(b->GetScale(), 0.01, 0.0001);
	gen_ensure_similar(b->GetScale(), 1, 0.0001);
	//gen_ensure_equals(b->getOriginalBpp(), 15);
	// TODO gen_ensure_equals(b->getOriginalBpp(), 16);
#if 0
	gen_ensure_equals(img->data->scalesToInt, true);
	gen_ensure_imagedata_similar(*img->data, *imgs[0]->data, 0.0001);
#endif
}

// Try reimporting an exported subarea grib (msat/msat)
template<> template<>
void to::test<5>()
{
	test_tag("croppedNetCDFRecodedGribMsat");
        auto_ptr<GDALDataset> dataset = openRecodedCropped(cropArea, "MsatGRIB", false, "TEMPLATE=msat/msat");
	test_untag();

	GDALRasterBand* b = dataset->GetRasterBand(1);
	gen_ensure_equals(b->GetRasterDataType(), GDT_Float64);

	gen_ensure_equals(b->GetOffset(), 0);
	gen_ensure_equals(b->GetScale(), 1);
	//gen_ensure_similar(b->GetOffset(), -247.6, 0.00001);
	//gen_ensure_similar(b->GetScale(), 0.01, 0.0001);
	//gen_ensure_equals(b->getOriginalBpp(), 12);
	// TODO gen_ensure_equals(b->getOriginalBpp(), 16);
#if 0
	gen_ensure_equals(img->data->scalesToInt, true);
	gen_ensure_imagedata_similar(*img->data, *imgs[0]->data, 0.0001);
#endif
}

// Try reimporting an exported subarea grib (msat/ecmwf)
template<> template<>
void to::test<6>()
{
	test_tag("croppedNetCDFRecodedGribEcmwf");
        auto_ptr<GDALDataset> dataset = openRecodedCropped(cropArea, "MsatGRIB", false, "TEMPLATE=msat/ecmwf");
	test_untag();

	GDALRasterBand* b = dataset->GetRasterBand(1);
	gen_ensure_equals(b->GetRasterDataType(), GDT_Float64);

	//gen_ensure_similar(b->GetOffset(), -247.6, 0.00001);
	gen_ensure_similar(b->GetOffset(), 0, 0.00001);
	//gen_ensure_similar(b->GetScale(), 0.01, 0.0001);
	gen_ensure_similar(b->GetScale(), 1, 0.0001);
	//gen_ensure_equals(b->getOriginalBpp(), 12);
	// TODO gen_ensure_equals(b->getOriginalBpp(), 16);

#if 0
	gen_ensure_equals(img->data->scalesToInt, true);
	gen_ensure_imagedata_similar(*img->data, *imgs[0]->data, 0.0001);
#endif
}

// Try reimporting an exported netcdf24
template<> template<>
void to::test<7>()
{
	test_tag("fullNetCDFRecodedNetCDF24");
        auto_ptr<GDALDataset> dataset = openRecoded("MsatNetCDF24", false);
	test_untag();

	GDALRasterBand* b = dataset->GetRasterBand(1);
	gen_ensure_equals(b->GetRasterDataType(), GDT_Float32);

	gen_ensure_equals(b->GetOffset(), 0);
	gen_ensure_equals(b->GetScale(), 1);
	// TODO gen_ensure_equals(b->getOriginalBpp(), 32);
#if 0
	gen_ensure_equals(img->data->scalesToInt, false);
	gen_ensure_imagedata_similar(*img->data, *imgs[0]->data, 0.0001);
#endif
}

// Try reimporting a subarea exported to netcdf24
template<> template<>
void to::test<8>()
{
	test_tag("croppedNetCDFRecodedNetCDF24");
        auto_ptr<GDALDataset> dataset = openRecodedCropped(cropArea, "MsatNetCDF24", false);
	test_untag();

	GDALRasterBand* b = dataset->GetRasterBand(1);
	gen_ensure_equals(b->GetRasterDataType(), GDT_Float32);

	gen_ensure_equals(b->GetOffset(), 0);
	gen_ensure_equals(b->GetScale(), 1);
	// TODO gen_ensure_equals(b->getOriginalBpp(), 32);
#if 0
	gen_ensure_equals(img->data->scalesToInt, false);
	gen_ensure_imagedata_similar(*img->data, *imgs[0]->data, 0.0001);
#endif
}

// Test plain CreateCopy
template<> template<>
void to::test<9>()
{
	test_tag("fullNetCDFRecodedNetCDF");
        auto_ptr<GDALDataset> dataset = openRecoded("MsatNetCDF", false);
	test_untag();

	GDALRasterBand* b = dataset->GetRasterBand(1);
	gen_ensure_equals(b->GetRasterDataType(), GDT_Float32);
	gen_ensure_equals(b->GetOffset(), 0);
	gen_ensure_equals(b->GetScale(), 1);
	// TODO gen_ensure_equals(b->getOriginalBpp(), 32);
#if 0
	gen_ensure_equals(img->data->scalesToInt, false);
	gen_ensure_imagedata_similar(*img->data, *imgs[0]->data, 0.0001);
#endif
}

// Try reimporting a subarea exported to netcdf
template<> template<>
void to::test<10>()
{
	test_tag("croppedNetCDFRecodedNetCDF");
        auto_ptr<GDALDataset> dataset = openRecodedCropped(cropArea, "MsatNetCDF", false);
	test_untag();

	GDALRasterBand* b = dataset->GetRasterBand(1);
	gen_ensure_equals(b->GetRasterDataType(), GDT_Float32);

	gen_ensure_equals(b->GetOffset(), 0);
	gen_ensure_equals(b->GetScale(), 1);
#if 0
	gen_ensure_equals(img->data->bpp, 32);
	gen_ensure_equals(img->data->scalesToInt, false);

	gen_ensure_imagedata_similar(*img->data, *imgs[0]->data, 0.0001);
#endif
}

}

/* vim:set ts=4 sw=4: */
