#include "utils.h"
#include "msat/facts.h"

using namespace std;
using namespace msat::tests;

namespace {

#define TESTFILE "MSG_Seviri_1_5_Infrared_9_7_channel_20060426_1945.grb"

struct Fixture : public GDALFixture
{
    using GDALFixture::GDALFixture;

    msat::proj::ImageBox get_crop_area() override
    {
        return msat::proj::ImageBox(msat::proj::ImagePoint(100, 100), msat::proj::ImagePoint(100+200, 100+50));
    }

    void check_general_image_data(GDALDataset* dataset);
    void check_full_image_data(GDALDataset* dataset) override;
    void check_cropped_image_data(GDALDataset* dataset) override;
};


class Tests : public ImportTest<Fixture>
{
    using ImportTest::ImportTest;

    void register_tests() override;
} test("gdal_import_grib", "MsatGRIB", TESTFILE);

void Fixture::check_general_image_data(GDALDataset* dataset)
{
    const char* val = dataset->GetMetadataItem(MD_MSAT_DATETIME, MD_DOMAIN_MSAT);
    wassert(actual(val) == "2006-04-26 19:45:00");

    val = dataset->GetMetadataItem(MD_MSAT_SPACECRAFT_ID, MD_DOMAIN_MSAT);
    wassert(actual(val) == "55");
    val = dataset->GetMetadataItem(MD_MSAT_SPACECRAFT, MD_DOMAIN_MSAT);
    wassert(actual(val) == "MSG1");

    GDALRasterBand* b = dataset->GetRasterBand(1);
    wassert(actual(b != NULL).istrue());

    val = b->GetMetadataItem(MD_MSAT_CHANNEL_ID, MD_DOMAIN_MSAT);
    wassert(actual(val) == "8");
    val = b->GetMetadataItem(MD_MSAT_CHANNEL, MD_DOMAIN_MSAT);
    wassert(actual(val) == "IR_097");

    wassert(actual(b->GetUnitType()) == "K");

    // TODO gen_ensure_equals(defaultFilename(*dataset), "MSG1_Seviri_1_5_IR_097_20060426_1945");
    // TODO gen_ensure_equals(defaultShortName(*dataset), "IR_097");

    // TODO gen_ensure_equals(defaultFilename(*b), "MSG1_Seviri_1_5_IR_097_20060426_1945");
    // TODO gen_ensure_equals(defaultShortName(*dataset), "IR_097");

    int valid;
    wassert(actual(b->GetOffset(&valid)) == 0);
    wassert(actual(valid) == TRUE);
    wassert(actual(b->GetScale(&valid)) == 1);
    wassert(actual(valid) == TRUE);

    // TODO gen_ensure_equals(b->scalesToInt(), false);

#if 0
    gen_ensure_equals(b->column_offset, 1856);
    gen_ensure_equals(b->line_offset, 1856);
    gen_ensure_equals(b->column_res, Image::columnResFromSeviriDX(3608));
    gen_ensure_equals(b->line_res, Image::columnResFromSeviriDX(3608));
    gen_ensure_equals(b->x0, 1499);
    gen_ensure_equals(b->y0, 199);
#endif
    //gen_ensure_equals(b->getOriginalBpp(), 11);
    // TODO gen_ensure(b->getOriginalBpp() >= 11);

#if 0
    gen_ensure_equals(img.defaultFilename, "MSG1_Seviri_IR_108_channel_20051219_1415");
    //gen_ensure_equals(img.channel_id, 2049);
    //gen_ensure_similar(img.column_res, Image::columnResFromSeviriDX(3608), 0.00001);
    //gen_ensure_similar(img.line_res, Image::columnResFromSeviriDX(3608), 0.00001);
    //gen_ensure_equals(img.column_offset, 1856);
    //gen_ensure_equals(img.line_offset, 1856);
    //gen_ensure_equals(img.defaultFilename, "MSG1_Seviri_unknown55_2049_channel_20060426_1945");
    //gen_ensure_equals(img.shortName, "unknown55_2049");
    //gen_ensure_equals(img.unit, "unknown");
#endif
}

void Fixture::check_full_image_data(GDALDataset* dataset)
{
    check_general_image_data(dataset);

    wassert(actual(dataset->GetRasterXSize()) == 1300);
    wassert(actual(dataset->GetRasterYSize()) == 700);

    int xs, ys;
    double rx, ry;
    msat::dataset::decodeGeotransform(dataset, xs, ys, rx, ry);
    wassert(actual(xs) == 1856 - 1500);
    wassert(actual(ys) == 1856 - 200);
    wassert(actual(rx) == METEOSAT_PIXELSIZE_X);
    wassert(actual(ry) == METEOSAT_PIXELSIZE_Y);

    GDALRasterBand* b = dataset->GetRasterBand(1);

    int valid;
    wassert(actual(b->GetOffset(&valid)) == 0);
    wassert(actual(valid) == TRUE);
    wassert(actual(b->GetScale(&valid)) == 1);
    wassert(actual(valid) == TRUE);
    double missing = b->GetNoDataValue(&valid);
    wassert(actual(missing) == 0);
    wassert(actual(valid) == TRUE);

    wassert(actual((double)gdal::read_float32(b, 0, 0)).almost_equal(97.7, 2));
    wassert(actual((double)gdal::read_float32(b, 10, 10)).almost_equal(98.1, 2));
    //gen_ensure_similar(b->scaled(1299, 0), b->scaledNoDataValue(), 0.000001);
    // Unfortunately, this grib does not have missing values
    wassert(actual((double)gdal::read_float32(b, 1299, 0)) == 0);
}

void Fixture::check_cropped_image_data(GDALDataset* dataset)
{
    check_general_image_data(dataset);

    wassert(actual(dataset->GetRasterXSize()) == 200);
    wassert(actual(dataset->GetRasterYSize()) == 50);

    int xs, ys;
    double rx, ry;
    msat::dataset::decodeGeotransform(dataset, xs, ys, rx, ry);
    wassert(actual(xs) == 1856 - 1600);
    wassert(actual(ys) == 1856 - 300);
    wassert(actual(rx) == METEOSAT_PIXELSIZE_X);
    wassert(actual(ry) == METEOSAT_PIXELSIZE_Y);

    GDALRasterBand* b = dataset->GetRasterBand(1);

    int valid;
    wassert(actual(b->GetOffset(&valid)) == 0);
    wassert(actual(valid) == TRUE);
    wassert(actual(b->GetScale(&valid)) == 1);
    wassert(actual(valid) == TRUE);
    double missing = b->GetNoDataValue(&valid);
    wassert(actual(missing) == 0);
    wassert(actual(valid) == TRUE);

    wassert(actual((double)gdal::read_float32(b, 0, 0)) == 100.50f);
    wassert(actual((double)gdal::read_float32(b, 10, 10)).almost_equal(97.8f, 3));
}

void Tests::register_tests()
{
    ImportTest::register_tests();

    this->add_method("datatype", [](Fixture& f) {
        GDALRasterBand* b = f.dataset()->GetRasterBand(1);
        wassert(actual(b->GetRasterDataType()) == GDT_Float64);
        //wassert(actual(b->GetOffset()) == 0);
        //wassert(actual(b->GetScale()) == 1);
        wassert(actual(b->GetOffset()).almost_equal(0, 4));
        wassert(actual(b->GetScale()).almost_equal(1, 4));
    });
}

}
