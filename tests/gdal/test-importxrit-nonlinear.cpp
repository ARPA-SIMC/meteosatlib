#include "test-utils.h"

#define PERFORM_SLOW_TESTS

using namespace std;

namespace tut {

#define TESTFILE "H:MSG1:IR_039:200611130800"

struct importxrit_nonlinear_shar : public ImportTest
{
	importxrit_nonlinear_shar() : ImportTest("MsatXRIT", TESTFILE)
	{
	}

	~importxrit_nonlinear_shar()
	{
	}

        void checkGeneralImageData(GDALDataset* dataset)
        {
                const char* val = dataset->GetMetadataItem(MD_MSAT_DATETIME, MD_DOMAIN_MSAT);
                gen_ensure(val != NULL);
                gen_ensure_equals(string(val), "2006-11-13 08:00:00");

                val = dataset->GetMetadataItem(MD_MSAT_SPACECRAFT_ID, MD_DOMAIN_MSAT);
                gen_ensure(val != NULL);
                gen_ensure_equals(string(val), "55");
                val = dataset->GetMetadataItem(MD_MSAT_SPACECRAFT, MD_DOMAIN_MSAT);
                gen_ensure(val != NULL);
                gen_ensure_equals(string(val), "MSG1");

                // TODO gen_ensure_equals(defaultFilename(*dataset), "MSG1_Seviri_1_5_IR_039_20061113_0800");
                // TODO gen_ensure_equals(defaultShortName(*dataset), "IR_039");

                GDALRasterBand* b = dataset->GetRasterBand(1);

                // TODO gen_ensure_equals(defaultFilename(*b), "MSG1_Seviri_1_5_IR_039_20061113_0800");
                // TODO gen_ensure_equals(defaultShortName(*dataset), "IR_039");

                val = b->GetMetadataItem(MD_MSAT_CHANNEL_ID, MD_DOMAIN_MSAT);
                gen_ensure(val != NULL);
                gen_ensure_equals(string(val), "4");
                val = b->GetMetadataItem(MD_MSAT_CHANNEL, MD_DOMAIN_MSAT);
                gen_ensure(val != NULL);
                gen_ensure_equals(string(val), "IR_039");

                gen_ensure_equals(string(b->GetUnitType()), "K");

                int valid;
                gen_ensure_equals(b->GetOffset(&valid), 0);
                gen_ensure_equals(valid, TRUE);
                gen_ensure_equals(b->GetScale(&valid), 1);
                gen_ensure_equals(valid, TRUE);

                // TODO gen_ensure_equals(b->scalesToInt(), false);

                // TODO switch (fl)
                // TODO {
                // TODO 	case FL_GRIB:
                // TODO 		gen_ensure_equals(b->getOriginalBpp(), 16);
                // TODO 		break;
                // TODO 	default:
                // TODO 		gen_ensure_equals(b->getOriginalBpp(), 32);
                // TODO 		break;
                // TODO }

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

                int valid;
                gen_ensure_equals(b->GetOffset(&valid), 0);
                gen_ensure_equals(valid, TRUE);
                gen_ensure_equals(b->GetScale(&valid), 1);
                gen_ensure_equals(valid, TRUE);
                double missing = b->GetNoDataValue(&valid);
                gen_ensure_equals(valid, TRUE);
                gen_ensure_equals(missing, 0.0);

                // TODO double missing = b->scaledNoDataValue();
                // TODO gen_ensure_equals(b->scaled(0, 0), missing);
                // TODO gen_ensure_equals(b->scaled(10, 10), missing);

                // TODO // TODO: now that the image has been moved as the concept of HRV frame
                // TODO // changed, choose a different set of points with non-missing values
                // TODO gen_ensure_similar(b->scaled(2540, 2950), missing, 0.001); // unverified
                // TODO gen_ensure_similar(b->scaled(2550, 2970), missing, 0.001); // unverified
                // TODO gen_ensure_similar(b->scaled(2560, 2990), missing, 0.001); // unverified
                // TODO gen_ensure_similar(b->scaled(2570, 3010), missing, 0.001); // unverified
                // TODO gen_ensure_similar(b->scaled(2580, 3030), missing, 0.001); // unverified
                // TODO gen_ensure_similar(b->scaled(2590, 3050), missing, 0.001); // unverified
                // TODO gen_ensure_similar(b->scaled(2600, 3070), missing, 0.001); // unverified
                // TODO gen_ensure_similar(b->scaled(2610, 3090), missing, 0.001); // unverified
                // TODO gen_ensure_similar(b->scaled(2620, 3110), missing, 0.001); // unverified
                // TODO gen_ensure_similar(b->scaled(2630, 3130), missing, 0.001); // unverified
                // TODO gen_ensure_similar(b->scaled(2640, 3150), missing, 0.001); // unverified

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
                gen_ensure_equals(xs, 1856 - 2540);
                gen_ensure_equals(ys, 1856 - 2950);
                gen_ensure_similar(rx, METEOSAT_PIXELSIZE_X, 0.00001);
                gen_ensure_similar(ry, METEOSAT_PIXELSIZE_Y, 0.00001);

                GDALRasterBand* b = dataset->GetRasterBand(1);

                int valid;
                gen_ensure_equals(b->GetOffset(&valid), 0);
                gen_ensure_equals(valid, TRUE);
                gen_ensure_equals(b->GetScale(&valid), 1);
                gen_ensure_equals(valid, TRUE);
                double missing = b->GetNoDataValue(&valid);
                //gen_ensure_equals(missing, 255);
                gen_ensure_equals(valid, TRUE);
#if 0
                double missing = b->scaledNoDataValue();

                // TODO: now that the image has been moved as the concept of HRV frame
                // changed, choose a different set of points with non-missing values
                gen_ensure_similar(b->scaled(  0,   0), missing, 0.001); // unverified
                gen_ensure_similar(b->scaled( 10,  20), missing, 0.001); // unverified
                gen_ensure_similar(b->scaled( 20,  40), missing, 0.001); // unverified
                gen_ensure_similar(b->scaled( 30,  60), missing, 0.001); // unverified
                gen_ensure_similar(b->scaled( 40,  80), missing, 0.001); // unverified
                gen_ensure_similar(b->scaled( 50, 100), missing, 0.001); // unverified
                gen_ensure_similar(b->scaled( 60, 120), missing, 0.001); // unverified
                gen_ensure_similar(b->scaled( 70, 140), missing, 0.001); // unverified
                gen_ensure_similar(b->scaled( 80, 160), missing, 0.001); // unverified
                gen_ensure_similar(b->scaled( 90, 180), missing, 0.01); // unverified
                gen_ensure_similar(b->scaled(100, 200), missing, 0.01); // unverified
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
#endif
        }
};
TESTGRP(importxrit_nonlinear);

static msat::proj::ImageBox cropArea(msat::proj::ImagePoint(2540, 2950), msat::proj::ImagePoint(2540+150, 2950+300));


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
    test_tag("fullXRIT");
    unique_ptr<GDALDataset> dataset = openPlain();
    test_untag();


    ////gen_ensure_equals(img->name, ""); // unverified
    //gen_ensure_equals(img->column_res, 40927014*exp2(-16));
    //gen_ensure_equals(img->line_res, 40927014*exp2(-16));

    //test_tag("fullXRIT");
    //checkFullImageData(*img);
    //test_untag();

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
    FOR_DRIVER("MsatXRIT");
#if 0
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
#endif
}

// Try reimporting an exported grib (msat/msat)
template<> template<>
void to::test<4>()
{
#ifdef PERFORM_SLOW_TESTS
    test_tag("fullHritRecodedGribMsat");
    unique_ptr<GDALDataset> dataset = openRecoded("MsatGRIB", false, "TEMPLATE=msat/msat");
    test_untag();

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
void to::test<5>()
{
#ifdef PERFORM_SLOW_TESTS
    test_tag("fullHritRecodedGribEcmwf");
    unique_ptr<GDALDataset> dataset = openRecoded("MsatGRIB", false, "TEMPLATE=msat/ecmwf");
    test_untag();

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
void to::test<6>()
{
    test_tag("croppedXRITRecodedGribMsat");
    unique_ptr<GDALDataset> dataset = openRecodedCropped(cropArea, "MsatGRIB", false, "TEMPLATE=msat/msat");
    test_untag();

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

// Import a subarea of an XRIT product and pass it from grib
template<> template<>
void to::test<7>()
{
    test_tag("croppedXRITRecodedGribEcmwf");
    unique_ptr<GDALDataset> dataset = openRecodedCropped(cropArea, "MsatGRIB", false, "TEMPLATE=msat/ecmwf");
    test_untag();

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
void to::test<8>()
{
#ifdef PERFORM_SLOW_TESTS
    test_tag("fulliHritRecodedNetCDF24");
    unique_ptr<GDALDataset> dataset = openRecoded("MsatNetCDF24", false);
    test_untag();

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
void to::test<9>()
{
    test_tag("croppedXRITRecodedNetCDF24");
    unique_ptr<GDALDataset> dataset = openRecodedCropped(cropArea, "MsatNetCDF24", false);
    test_untag();

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
void to::test<10>()
{
#ifdef PERFORM_SLOW_TESTS
    test_tag("fullHritRecodedNetCDF");
    unique_ptr<GDALDataset> dataset = openRecoded("MsatNetCDF", false);
    test_untag();

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
void to::test<11>()
{
    test_tag("croppedXRITRecodedNetCDF");
    unique_ptr<GDALDataset> dataset = openRecodedCropped(cropArea, "MsatNetCDF", false);
    test_untag();

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

}
