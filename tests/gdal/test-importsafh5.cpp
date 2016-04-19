#include "utils.h"
#include "msat/facts.h"

using namespace std;
using namespace msat::tests;

namespace {

#define TESTFILE "SAFNWC_MSG1_CRR__05339_025_MSGEURO_____.h5"

struct Fixture : public GDALFixture
{
    using GDALFixture::GDALFixture;

    msat::proj::ImageBox get_crop_area() override
    {
        return msat::proj::ImageBox(msat::proj::ImagePoint(100, 50), msat::proj::ImagePoint(100+500, 50+50));
    }

    void check_general_image_data(GDALDataset* dataset);
    void check_full_image_data(GDALDataset* dataset) override;
    void check_cropped_image_data(GDALDataset* dataset) override;
};

class Tests : public ImportTest<Fixture>
{
    using ImportTest::ImportTest;

    void register_tests() override;
} test("gdal_import_safh5", "MsatSAFH5", TESTFILE);

void Fixture::check_general_image_data(GDALDataset* dataset)
{
    const char* val = dataset->GetMetadataItem(MD_MSAT_DATETIME, MD_DOMAIN_MSAT);
    wassert(actual(val) == "2005-12-05 06:15:00");

    val = dataset->GetMetadataItem(MD_MSAT_SPACECRAFT_ID, MD_DOMAIN_MSAT);
    wassert(actual(val) == "55");
    val = dataset->GetMetadataItem(MD_MSAT_SPACECRAFT, MD_DOMAIN_MSAT);
    wassert(actual(val) == "MSG1");

    // TODO if (dataset->GetRasterCount() == 1)
    // TODO {
    // TODO 	gen_ensure_equals(defaultFilename(*dataset), "MSG1_Seviri_3_CRR_20051205_0615");
    // TODO 	gen_ensure_equals(defaultShortName(*dataset), "CRR");
    // TODO } else {
    // TODO 	gen_ensure_equals(defaultFilename(*dataset), "MSG1_Seviri_3_20051205_0615");
    // TODO 	gen_ensure_equals(defaultShortName(*dataset), "");
    // TODO }

    GDALRasterBand* b = dataset->GetRasterBand(1);
    wassert(actual(b != NULL).istrue());

    // TODO gen_ensure_equals(defaultFilename(*b), "MSG1_Seviri_3_CRR_20051205_0615");
    // TODO gen_ensure_equals(defaultShortName(*b), "CRR");

    val = b->GetMetadataItem(MD_MSAT_CHANNEL_ID, MD_DOMAIN_MSAT);
    wassert(actual(val) == "106");
    val = b->GetMetadataItem(MD_MSAT_CHANNEL, MD_DOMAIN_MSAT);
    wassert(actual(val) == "CRR");

    wassert(actual(b->GetUnitType()) == "NUMERIC");

    int valid;
    wassert(actual(b->GetOffset(&valid)) == 0);
    wassert(actual(valid) == TRUE);
    wassert(actual(b->GetScale(&valid)) == 1);
    wassert(actual(valid) == TRUE);

#if 0
    gen_ensure_equals(img.defaultFilename, "MSG1_Seviri_IR_108_channel_20051219_1415");
    gen_ensure_equals(img->defaultFilename, "SAF_MSGEURO_CRR_20051205_0615");
#endif
}

void Fixture::check_full_image_data(GDALDataset* dataset)
{
    check_general_image_data(dataset);

    string driver = dataset->GetDriver()->GetDescription();

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

    // GRIB cannot store fill values and SAF uses nonstandard
    // channels so we cannot deduce one
    if (driver != "MsatGRIB")
    {
        int valid;
        wassert(actual(b->GetNoDataValue(&valid)) == 255);
        wassert(actual(valid) == TRUE);
    }

    wassert(actual(gdal::read_int32(b, 0, 0)) == 0);
    wassert(actual(gdal::read_int32(b, 10, 10)) == 0);
    wassert(actual(gdal::read_int32(b, 516, 54)) == 3);
}

void Fixture::check_cropped_image_data(GDALDataset* dataset)
{
    check_general_image_data(dataset);

    string driver = dataset->GetDriver()->GetDescription();

    wassert(actual(dataset->GetRasterXSize()) == 500);
    wassert(actual(dataset->GetRasterYSize()) == 50);

    int xs, ys;
    double rx, ry;
    msat::dataset::decodeGeotransform(dataset, xs, ys, rx, ry);
    wassert(actual(xs) == 1856 - 1600);
    wassert(actual(ys) == 1856 - 250);
    wassert(actual(rx) == METEOSAT_PIXELSIZE_X);
    wassert(actual(ry) == METEOSAT_PIXELSIZE_Y);

    GDALRasterBand* b = dataset->GetRasterBand(1);

    // GRIB cannot store fill values and SAF uses nonstandard
    // channels so we cannot deduce one
    if (driver != "MsatGRIB")
    {
        int valid;
        wassert(actual(b->GetNoDataValue(&valid)) == 255);
        wassert(actual(valid) == TRUE);
    }

    wassert(actual(gdal::read_int32(b, 0, 0)) == 0);
    wassert(actual(gdal::read_int32(b, 10, 10)) == 0);
    wassert(actual(gdal::read_int32(b, 416, 4)) == 3);
}

void Tests::register_tests()
{
    ImportTest::register_tests();

    this->add_method("datatype", [](Fixture& f) {
        GDALRasterBand* b = f.dataset()->GetRasterBand(1);
        wassert(actual(b->GetRasterDataType()) == GDT_Byte);
        wassert(actual(b->GetOffset()) == 0);
        wassert(actual(b->GetScale()) == 1);
    });
}

}
