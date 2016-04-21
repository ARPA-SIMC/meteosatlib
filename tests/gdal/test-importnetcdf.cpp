#include "utils.h"
#include "msat/facts.h"

using namespace std;
using namespace msat::tests;

namespace {

#define TESTFILE "MSG_Seviri_1_5_Infrared_10_8_channel_20051219_1415.nc"

struct Fixture : public GDALFixture
{
    using GDALFixture::GDALFixture;

    msat::proj::ImageBox get_crop_area()
    {
        return msat::proj::ImageBox(msat::proj::ImagePoint(100, 100), msat::proj::ImagePoint(100+200, 100+50));
    }

    void check_general_image_data(GDALDataset* dataset);
    void check_full_image_data(GDALDataset* dataset);
    void check_cropped_image_data(GDALDataset* dataset);
};

class Tests : public ImportTest<Fixture>
{
    using ImportTest::ImportTest;

    void register_tests() override;
} test("gdal_import_netcdf", "MsatNetCDF", TESTFILE);

void Fixture::check_general_image_data(GDALDataset* dataset)
{
    const char* val = dataset->GetMetadataItem(MD_MSAT_DATETIME, MD_DOMAIN_MSAT);
    wassert(actual(val) == "2005-12-19 14:15:00");

    val = dataset->GetMetadataItem(MD_MSAT_SPACECRAFT_ID, MD_DOMAIN_MSAT);
    wassert(actual(val) == "55");
    val = dataset->GetMetadataItem(MD_MSAT_SPACECRAFT, MD_DOMAIN_MSAT);
    wassert(actual(val) == "MSG1");

    // TODO gen_ensure_equals(defaultFilename(*dataset), "MSG1_Seviri_1_5_IR_108_20051219_1415");
    // TODO gen_ensure_equals(defaultShortName(*dataset), "IR_108");

    GDALRasterBand* b = dataset->GetRasterBand(1);
    wassert(actual(b != NULL).istrue());

    // TODO gen_ensure_equals(defaultFilename(*b), "MSG1_Seviri_1_5_IR_108_20051219_1415");
    // TODO gen_ensure_equals(defaultShortName(*dataset), "IR_108");

    val = b->GetMetadataItem(MD_MSAT_CHANNEL_ID, MD_DOMAIN_MSAT);
    wassert(actual(val) == "9");
    val = b->GetMetadataItem(MD_MSAT_CHANNEL, MD_DOMAIN_MSAT);
    wassert(actual(val) == "IR_108");

    wassert(actual(b->GetUnitType()) == "K");

    int valid;
    wassert(actual(b->GetOffset(&valid)) == 0);
    wassert(actual(valid) == TRUE);
    wassert(actual(b->GetScale(&valid)) == 1);
    wassert(actual(valid) == TRUE);

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

void Fixture::check_full_image_data(GDALDataset* dataset)
{
    check_general_image_data(dataset);

    wassert(actual(dataset->GetRasterXSize()) == 1300);
    wassert(actual(dataset->GetRasterYSize()) == 700);

    int xs, ys;
    double rx, ry;
    msat::dataset::decodeGeotransform(dataset, xs, ys, rx, ry);
    wassert(actual(xs) == 1856 - 1499);
    wassert(actual(ys) == 1856 - 199);
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

    wassert(actual((double)gdal::read_float32(b, 0, 0)).almost_equal(230.3, 2));
    wassert(actual((double)gdal::read_float32(b, 10, 10)).almost_equal(240.6, 2));
    wassert(actual((double)gdal::read_float32(b, 1299, 0)).almost_equal(missing, 5));
}

void Fixture::check_cropped_image_data(GDALDataset* dataset)
{
    check_general_image_data(dataset);

    wassert(actual(dataset->GetRasterXSize()) == 200);
    wassert(actual(dataset->GetRasterYSize()) == 50);

    int xs, ys;
    double rx, ry;
    msat::dataset::decodeGeotransform(dataset, xs, ys, rx, ry);
    wassert(actual(xs) == 1856 - 1599);
    wassert(actual(ys) == 1856 - 299);
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

    wassert(actual((double)gdal::read_float32(b, 0, 0)).almost_equal(251.4, 2));
    wassert(actual((double)gdal::read_float32(b, 10, 10)).almost_equal(266.5, 2));
}

void Tests::register_tests()
{
    ImportTest::register_tests();

    this->add_method("datatype", [](Fixture& f) {
        GDALRasterBand* b = f.dataset()->GetRasterBand(1);
        wassert(actual(b->GetRasterDataType()) == GDT_Float32);
        wassert(actual(b->GetOffset()) == 0);
        wassert(actual(b->GetScale()) == 1);
    });
}

}
