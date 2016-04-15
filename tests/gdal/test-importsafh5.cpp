#include "test-utils.h"

using namespace std;

namespace tut {

#define TESTFILE "SAFNWC_MSG1_CRR__05339_025_MSGEURO_____.h5"

struct importsafh5_shar : public ImportTest
{
	importsafh5_shar() : ImportTest("MsatSAFH5", TESTFILE)
	{
	}

	~importsafh5_shar()
	{
	}

	void checkGeneralImageData(GDALDataset* dataset)
	{
		const char* val = dataset->GetMetadataItem(MD_MSAT_DATETIME, MD_DOMAIN_MSAT);
		gen_ensure(val != NULL);
		gen_ensure_equals(string(val), "2005-12-05 06:15:00");

		val = dataset->GetMetadataItem(MD_MSAT_SPACECRAFT_ID, MD_DOMAIN_MSAT);
		gen_ensure(val != NULL);
		gen_ensure_equals(string(val), "55");
		val = dataset->GetMetadataItem(MD_MSAT_SPACECRAFT, MD_DOMAIN_MSAT);
		gen_ensure(val != NULL);
		gen_ensure_equals(string(val), "MSG1");

		// TODO if (dataset->GetRasterCount() == 1)
		// TODO {
		// TODO 	gen_ensure_equals(defaultFilename(*dataset), "MSG1_Seviri_3_CRR_20051205_0615");
		// TODO 	gen_ensure_equals(defaultShortName(*dataset), "CRR");
		// TODO } else {
		// TODO 	gen_ensure_equals(defaultFilename(*dataset), "MSG1_Seviri_3_20051205_0615");
		// TODO 	gen_ensure_equals(defaultShortName(*dataset), "");
		// TODO }

		GDALRasterBand* b = dataset->GetRasterBand(1);
		gen_ensure(b != NULL);

		// TODO gen_ensure_equals(defaultFilename(*b), "MSG1_Seviri_3_CRR_20051205_0615");
		// TODO gen_ensure_equals(defaultShortName(*b), "CRR");

		val = b->GetMetadataItem(MD_MSAT_CHANNEL_ID, MD_DOMAIN_MSAT);
		gen_ensure(val != NULL);
		gen_ensure_equals(string(val), "106");
		val = b->GetMetadataItem(MD_MSAT_CHANNEL, MD_DOMAIN_MSAT);
		gen_ensure(val != NULL);
		gen_ensure_equals(string(val), "CRR");

		gen_ensure_equals(string(b->GetUnitType()), "NUMERIC");

		int valid;
		gen_ensure_equals(b->GetOffset(&valid), 0);
		gen_ensure_equals(valid, TRUE);
		gen_ensure_equals(b->GetScale(&valid), 1);
		gen_ensure_equals(valid, TRUE);

#if 0
		gen_ensure_equals(img.defaultFilename, "MSG1_Seviri_IR_108_channel_20051219_1415");
		gen_ensure_equals(img->defaultFilename, "SAF_MSGEURO_CRR_20051205_0615");
#endif
	}

	void checkFullImageData(GDALDataset* dataset)
	{
		checkGeneralImageData(dataset);
		string driver = dataset->GetDriver()->GetDescription();

		gen_ensure_equals(dataset->GetRasterXSize(), 1300);
		gen_ensure_equals(dataset->GetRasterYSize(), 700);

		int xs, ys;
		double rx, ry;
		msat::dataset::decodeGeotransform(dataset, xs, ys, rx, ry);
		gen_ensure_equals(xs, 1856 - 1500);
		gen_ensure_equals(ys, 1856 - 200);
		gen_ensure_equals(rx, METEOSAT_PIXELSIZE_X);
		gen_ensure_equals(ry, METEOSAT_PIXELSIZE_Y);

		GDALRasterBand* b = dataset->GetRasterBand(1);

		// GRIB cannot store fill values and SAF uses nonstandard
		// channels so we cannot deduce one
		if (driver != "MsatGRIB")
		{
			int valid;
			gen_ensure_equals(b->GetNoDataValue(&valid), 255);
			gen_ensure_equals(valid, TRUE);
		}

		gen_ensure_equals(readInt32(b, 0, 0), 0);
		gen_ensure_equals(readInt32(b, 10, 10), 0);
		gen_ensure_equals(readInt32(b, 516, 54), 3);
	}

	void checkCroppedImageData(GDALDataset* dataset)
	{
		checkGeneralImageData(dataset);
		string driver = dataset->GetDriver()->GetDescription();

		gen_ensure_equals(dataset->GetRasterXSize(), 500);
		gen_ensure_equals(dataset->GetRasterYSize(), 50);

		int xs, ys;
		double rx, ry;
		msat::dataset::decodeGeotransform(dataset, xs, ys, rx, ry);
		gen_ensure_equals(xs, 1856 - 1600);
		gen_ensure_equals(ys, 1856 - 250);
		gen_ensure_equals(rx, METEOSAT_PIXELSIZE_X);
		gen_ensure_equals(ry, METEOSAT_PIXELSIZE_Y);

		GDALRasterBand* b = dataset->GetRasterBand(1);

		// GRIB cannot store fill values and SAF uses nonstandard
		// channels so we cannot deduce one
		if (driver != "MsatGRIB")
		{
			int valid;
			gen_ensure_equals(b->GetNoDataValue(&valid), 255);
			gen_ensure_equals(valid, TRUE);
		}

		gen_ensure_equals(readInt32(b, 0, 0), 0);
		gen_ensure_equals(readInt32(b, 10, 10), 0);
		gen_ensure_equals(readInt32(b, 416, 4), 3);
	}
};
TESTGRP(importsafh5);

static msat::proj::ImageBox cropArea(msat::proj::ImagePoint(100, 50), msat::proj::ImagePoint(100+500, 50+50));

// Test opening
template<> template<>
void to::test<1>()
{
    unique_ptr<GDALDataset> dataset = openDS();
    gen_ensure(dataset.get() != 0);
    gen_ensure_equals(string(GDALGetDriverShortName(dataset->GetDriver())), "MsatSAFH5");
    gen_ensure_equals(dataset->GetRasterCount(), 3);
}

// Import a full SAFH5 product
template<> template<>
void to::test<2>()
{
    test_tag("fullSAFH5");
    unique_ptr<GDALDataset> dataset = openPlain();
    test_untag();

    // TODO msat::Band* b = dynamic_cast<msat::Band*>(dataset->GetRasterBand(1));
    // TODO gen_ensure_equals(b->getOriginalBpp(), 8);
}

#if 0
// Import a subarea of a SAFH5 product
template<> template<>
void to::test<3>()
{
    std::unique_ptr<ImageImporter> imp(createSAFH5Importer(DATA_DIR "/SAFNWC_MSG1_CRR__05339_025_MSGEURO_____.h5"));
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
#endif

// Try reimporting an exported grib (msat/msat)
template<> template<>
void to::test<3>()
{
    test_tag("fullSAFH5RecodedGribMsat");
    unique_ptr<GDALDataset> dataset = openRecoded("MsatGRIB", false, "TEMPLATE=msat/msat");
    test_untag();

    //GDALRasterBand* b = dataset->GetRasterBand(1);
    //gen_ensure_equals(b->getOriginalBpp(), 8);
    // TODO gen_ensure_equals(b->getOriginalBpp(), 16);
#if 0
    gen_ensure_equals(img->column_res, Image::columnResFromSeviriDX(Image::seviriDXFromColumnRes(13642337*exp2(-16))));
    gen_ensure_equals(img->line_res, Image::columnResFromSeviriDX(Image::seviriDXFromColumnRes(13642337*exp2(-16))));
    gen_ensure_equals(img->data->slope, 1);
    gen_ensure_equals(img->data->offset, 0);
    gen_ensure_equals(img->data->bpp, 3);
    gen_ensure_equals(img->data->scalesToInt, true);
    gen_ensure_equals(img->defaultFilename, "MSG1_Seviri_CRR_channel_20051205_0615");
    gen_ensure_imagedata_similar(*img->data, *imgs[0]->data, 0.0001);
#endif
}

// Try reimporting an exported grib (msat/ecmwf)
template<> template<>
void to::test<4>()
{
    test_tag("fullSAFH5RecodedGribEcmwf");
    unique_ptr<GDALDataset> dataset = openRecoded("MsatGRIB", false, "TEMPLATE=msat/ecmwf");
    test_untag();

    //GDALRasterBand* b = dataset->GetRasterBand(1);
    //gen_ensure_equals(b->getOriginalBpp(), 8);
    // TODO gen_ensure_equals(b->getOriginalBpp(), 16);
#if 0
    gen_ensure_equals(img->column_res, Image::columnResFromSeviriDX(Image::seviriDXFromColumnRes(13642337*exp2(-16))));
    gen_ensure_equals(img->line_res, Image::columnResFromSeviriDX(Image::seviriDXFromColumnRes(13642337*exp2(-16))));
    gen_ensure_equals(img->data->slope, 1);
    gen_ensure_equals(img->data->offset, 0);
    gen_ensure_equals(img->data->bpp, 3);
    gen_ensure_equals(img->data->scalesToInt, true);
    gen_ensure_equals(img->defaultFilename, "MSG1_Seviri_CRR_channel_20051205_0615");
    gen_ensure_imagedata_similar(*img->data, *imgs[0]->data, 0.0001);
#endif
}

// Try reimporting a subarea exported to grib (msat/msat)
template<> template<>
void to::test<5>()
{
    test_tag("croppedSAFH5RecodedGribMsat");
    unique_ptr<GDALDataset> dataset = openRecodedCropped(cropArea, "MsatGRIB", false, "TEMPLATE=msat/msat");
    test_untag();

    //GDALRasterBand* b = dataset->GetRasterBand(1);
    //gen_ensure_equals(b->getOriginalBpp(), 8);
    // TODO gen_ensure_equals(b->getOriginalBpp(), 16);
#if 0
    gen_ensure_equals(img->column_res, Image::columnResFromSeviriDX(Image::seviriDXFromColumnRes(13642337*exp2(-16))));
    gen_ensure_equals(img->line_res, Image::columnResFromSeviriDX(Image::seviriDXFromColumnRes(13642337*exp2(-16))));
    gen_ensure_equals(img->data->slope, 1);
    gen_ensure_equals(img->data->offset, 0);
    gen_ensure_equals(img->data->bpp, 2);
    gen_ensure_equals(img->data->scalesToInt, true);
    gen_ensure_equals(img->defaultFilename, "MSG1_Seviri_CRR_channel_20051205_0615");

    gen_ensure_imagedata_similar(*img->data, *imgs[0]->data, 0.0001);
#endif
}

// Try reimporting a subarea exported to grib (msat/ecmwf)
template<> template<>
void to::test<6>()
{
    test_tag("croppedSAFH5RecodedGribEcmwf");
    unique_ptr<GDALDataset> dataset = openRecodedCropped(cropArea, "MsatGRIB", false, "TEMPLATE=msat/ecmwf");
    test_untag();

    //GDALRasterBand* b = dataset->GetRasterBand(1);
    //gen_ensure_equals(b->getOriginalBpp(), 8);
    // TODO gen_ensure_equals(b->getOriginalBpp(), 16);
#if 0
    gen_ensure_equals(img->column_res, Image::columnResFromSeviriDX(Image::seviriDXFromColumnRes(13642337*exp2(-16))));
    gen_ensure_equals(img->line_res, Image::columnResFromSeviriDX(Image::seviriDXFromColumnRes(13642337*exp2(-16))));
    gen_ensure_equals(img->data->slope, 1);
    gen_ensure_equals(img->data->offset, 0);
    gen_ensure_equals(img->data->bpp, 2);
    gen_ensure_equals(img->data->scalesToInt, true);
    gen_ensure_equals(img->defaultFilename, "MSG1_Seviri_CRR_channel_20051205_0615");

    gen_ensure_imagedata_similar(*img->data, *imgs[0]->data, 0.0001);
#endif
}

// Try reimporting an exported netcdf24
template<> template<>
void to::test<7>()
{
    test_tag("fullSAFH5RecodedNetCDF24");
    unique_ptr<GDALDataset> dataset = openRecoded("MsatNetCDF24", false);
    test_untag();

    GDALRasterBand* b = dataset->GetRasterBand(1);
    ensure_equals(b->GetRasterDataType(), GDT_Byte);

    // TODO gen_ensure_equals(b->getOriginalBpp(), 8);
#if 0
    gen_ensure_equals(img->column_res, Image::columnResFromSeviriDX(Image::seviriDXFromColumnRes(13642337*exp2(-16))));
    gen_ensure_equals(img->line_res, Image::columnResFromSeviriDX(Image::seviriDXFromColumnRes(13642337*exp2(-16))));
    gen_ensure_equals(img->data->slope, 1);
    gen_ensure_equals(img->data->offset, 0);
    gen_ensure_equals(img->data->bpp, 3);
    gen_ensure_equals(img->data->scalesToInt, true);
    gen_ensure_equals(img->defaultFilename, "MSG1_Seviri_CRR_channel_20051205_0615");
    gen_ensure_imagedata_similar(*img->data, *imgs[0]->data, 0.0001);
#endif
}

// Try reimporting a subarea exported to netcdf24
template<> template<>
void to::test<8>()
{
    test_tag("croppedSAFH5RecodedNetCDF24");
    unique_ptr<GDALDataset> dataset = openRecodedCropped(cropArea, "MsatNetCDF24", false);
    test_untag();

    GDALRasterBand* b = dataset->GetRasterBand(1);
    ensure_equals(b->GetRasterDataType(), GDT_Byte);

    // TODO gen_ensure_equals(b->getOriginalBpp(), 8);
#if 0
    gen_ensure_equals(img->column_res, Image::columnResFromSeviriDX(Image::seviriDXFromColumnRes(13642337*exp2(-16))));
    gen_ensure_equals(img->line_res, Image::columnResFromSeviriDX(Image::seviriDXFromColumnRes(13642337*exp2(-16))));
    gen_ensure_equals(img->data->slope, 1);
    gen_ensure_equals(img->data->offset, 0);
    gen_ensure_equals(img->data->bpp, 2);
    gen_ensure_equals(img->data->scalesToInt, true);
    gen_ensure_equals(img->defaultFilename, "MSG1_Seviri_CRR_channel_20051205_0615");

    gen_ensure_imagedata_similar(*img->data, *imgs[0]->data, 0.0001);
#endif
}

// Try reimporting an exported netcdf
template<> template<>
void to::test<9>()
{
    test_tag("fullSAFH5RecodedNetCDF");
    unique_ptr<GDALDataset> dataset = openRecoded("MsatNetCDF", false);
    test_untag();

    //GDALRasterBand* b = dataset1->GetRasterBand(1);
    // TODO gen_ensure_equals(b->getOriginalBpp(), 8);
#if 0
    gen_ensure_equals(img->column_res, 13642337*exp2(-16));
    gen_ensure_equals(img->line_res, 13642337*exp2(-16));
    gen_ensure_equals(img->data->slope, 1);
    gen_ensure_equals(img->data->offset, 0);
    gen_ensure_equals(img->data->bpp, 32);
    gen_ensure_equals(img->data->scalesToInt, false);
    gen_ensure_equals(img->defaultFilename, "MSG1_Seviri_CRR_channel_20051205_0615");
    gen_ensure_imagedata_similar(*img->data, *imgs[0]->data, 0.0001);
#endif
}

// Try reimporting a subarea exported to netcdf
template<> template<>
void to::test<10>()
{
    test_tag("croppedSAFH5RecodedNetCDF");
    unique_ptr<GDALDataset> dataset = openRecodedCropped(cropArea, "MsatNetCDF", false);
    test_untag();

    GDALRasterBand* b = dataset->GetRasterBand(1);
    gen_ensure_equals(b->GetRasterDataType(), GDT_Byte);

    gen_ensure_equals(b->GetOffset(), 0);
    gen_ensure_equals(b->GetScale(), 1);
    // TODO gen_ensure_equals(b->getOriginalBpp(), 8);
#if 0
    gen_ensure_equals(img->column_res, 13642337*exp2(-16));
    gen_ensure_equals(img->line_res, 13642337*exp2(-16));
    gen_ensure_equals(img->data->slope, 1);
    gen_ensure_equals(img->data->offset, 0);
    gen_ensure_equals(img->data->bpp, 32);
    gen_ensure_equals(img->data->scalesToInt, false);
    gen_ensure_equals(img->defaultFilename, "MSG1_Seviri_CRR_channel_20051205_0615");

    gen_ensure_imagedata_similar(*img->data, *imgs[0]->data, 0.0001);
#endif
}

}
