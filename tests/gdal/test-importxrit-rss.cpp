#include "utils.h"
#include "msat/facts.h"

using namespace std;
using namespace msat::tests;

namespace {

#define TESTFILE "rss/H:MSG2_RSS:VIS006:201604281230"
#define PERFORM_SLOW_TESTS
#define native_offset -1.0664
#define native_scale 0.0209097

struct Fixture : public GDALFixture
{
    using GDALFixture::GDALFixture;

    msat::proj::ImageBox get_crop_area()
    {
        return msat::proj::ImageBox(msat::proj::ImagePoint(1400, 100), msat::proj::ImagePoint(1400 + 150, 100 + 300));
    }

    void check_general_image_data(GDALDataset* dataset);
    void check_full_image_data(GDALDataset* dataset);
    void check_cropped_image_data(GDALDataset* dataset);
};


class Tests : public ImportTest<Fixture>
{
    using ImportTest::ImportTest;

    void register_tests() override;
} test("gdal_import_xrit_rss", "MsatXRIT", TESTFILE);

void Fixture::check_general_image_data(GDALDataset* dataset)
{
    const char* val = dataset->GetMetadataItem(MD_MSAT_DATETIME, MD_DOMAIN_MSAT);
    wassert(actual(val) == "2016-04-28 12:30:00");

    val = dataset->GetMetadataItem(MD_MSAT_SPACECRAFT_ID, MD_DOMAIN_MSAT);
    wassert(actual(val) == "56");
    val = dataset->GetMetadataItem(MD_MSAT_SPACECRAFT, MD_DOMAIN_MSAT);
    wassert(actual(val) == "MSG2");

    // TODO gen_ensure_equals(defaultFilename(*dataset), "MSG2_Seviri_1_5_VIS006_20080715_0900");
    // TODO gen_ensure_equals(defaultShortName(*dataset), "VIS006");

    GDALRasterBand* b = dataset->GetRasterBand(1);

    // TODO gen_ensure_equals(defaultFilename(*b), "MSG2_Seviri_1_5_VIS006_20080715_0900");
    // TODO gen_ensure_equals(defaultShortName(*dataset), "VIS006");

    val = b->GetMetadataItem(MD_MSAT_CHANNEL_ID, MD_DOMAIN_MSAT);
    wassert(actual(val) == "1");
    val = b->GetMetadataItem(MD_MSAT_CHANNEL, MD_DOMAIN_MSAT);
    wassert(actual(val) == "VIS006");

    wassert(actual(b->GetUnitType()) == "mW m^-2 sr^-1 (cm^-1)^-1");

    if (b->GetRasterDataType() == GDT_Float64)
    {
        int valid;
        wassert(actual(b->GetOffset(&valid)) == 0);
        wassert(actual(valid) == TRUE);
        wassert(actual(b->GetScale(&valid)) == 1);
        wassert(actual(valid) == TRUE);
    } else {
        int valid;
        wassert(actual(b->GetOffset(&valid)).almost_equal(native_offset, 4));
        wassert(actual(valid) == TRUE);
        wassert(actual(b->GetScale(&valid)).almost_equal(native_scale, 7));
        wassert(actual(valid) == TRUE);
    }
}

void Fixture::check_full_image_data(GDALDataset* dataset)
{
    check_general_image_data(dataset);

    wassert(actual(dataset->GetRasterXSize()) == 3712);
    wassert(actual(dataset->GetRasterYSize()) == 3712);

    int xs, ys;
    double rx, ry;
    msat::dataset::decodeGeotransform(dataset, xs, ys, rx, ry);
    wassert(actual(xs) == 1856);
    wassert(actual(ys) == 1856);
    wassert(actual(rx).almost_equal(METEOSAT_PIXELSIZE_X, 3));
    wassert(actual(ry).almost_equal(METEOSAT_PIXELSIZE_Y, 3));

    GDALRasterBand* b = dataset->GetRasterBand(1);

    if (b->GetRasterDataType() == GDT_Float64)
    {
        // Prescaled
        int valid;
        double missing = b->GetNoDataValue(&valid);
        wassert(actual(valid) == TRUE);
        wassert(actual(missing).almost_equal(-1.02691, 4));
        wassert(actual((double)gdal::read_float32(b,    0,    0)).almost_equal(missing, 4));
        wassert(actual((double)gdal::read_float32(b,   10,   10)).almost_equal(missing, 4));

        wassert(actual((double)gdal::read_float32(b, 2000, 2000)).almost_equal(missing, 4));  // Missing segment
        wassert(actual((double)gdal::read_float32(b, 1000,  100)).almost_equal(missing, 4));  // Missing value on existing segment
#define SC(x) ((x)*native_scale + native_offset)
        wassert(actual((double)gdal::read_float32(b, 1550,  420)).almost_equal(SC( 92), 3));  // Dark point
        wassert(actual((double)gdal::read_float32(b, 2000,  120)).almost_equal(SC(177), 3));  // Grey point
        wassert(actual((double)gdal::read_float32(b, 1920,  350)).almost_equal(SC(671), 3));  // Bright point
#undef SC
    } else {
        // Packed
        int valid;
        int32_t missing = b->GetNoDataValue(&valid);
        wassert(actual(valid) == TRUE);
        wassert(actual(missing) == 0);
        wassert(actual(gdal::read_int32(b,    0,    0)) == missing);
        wassert(actual(gdal::read_int32(b,   10,   10)) == missing);

        wassert(actual(gdal::read_int32(b, 2000, 2000)) == missing);  // Missing segment
        wassert(actual(gdal::read_int32(b, 1000,  100)) == missing);  // Missing value on existing segment
        wassert(actual(gdal::read_int32(b, 1550,  420)) ==  92);      // Dark point
        wassert(actual(gdal::read_int32(b, 2000,  120)) == 177);      // Grey point
        wassert(actual(gdal::read_int32(b, 1920,  350)) == 671);      // Bright point
    }
}

void Fixture::check_cropped_image_data(GDALDataset* dataset)
{
    check_general_image_data(dataset);

    wassert(actual(dataset->GetRasterXSize()) == 150);
    wassert(actual(dataset->GetRasterYSize()) == 300);

    int xs, ys;
    double rx, ry;
    msat::dataset::decodeGeotransform(dataset, xs, ys, rx, ry);
    wassert(actual(xs) == 1856 - 1400);
    wassert(actual(ys) == 1856 - 100);
    wassert(actual(rx).almost_equal(METEOSAT_PIXELSIZE_X, 4));
    wassert(actual(ry).almost_equal(METEOSAT_PIXELSIZE_Y, 4));

    GDALRasterBand* b = dataset->GetRasterBand(1);

    if (b->GetRasterDataType() == GDT_Float64)
    {
        // Prescaled
        int valid;
        double missing = b->GetNoDataValue(&valid);
        wassert(actual(valid) == TRUE);
        wassert(actual(missing).almost_equal(-1.02691, 4));

        wassert(actual((double)gdal::read_float32(b,  0,   0)).almost_equal(missing, 4));  // Missing value on existing segment
#define SC(x) ((x)*native_scale + native_offset)
        wassert(actual((double)gdal::read_float32(b,  5,  70)).almost_equal(SC(119), 3));  // Dark point
        wassert(actual((double)gdal::read_float32(b, 15, 135)).almost_equal(SC(153), 3));  // Grey point
        wassert(actual((double)gdal::read_float32(b, 62, 102)).almost_equal(SC(670), 3));  // Bright point
#undef SC
    } else {
        // Packed
        int valid;
        double missing = b->GetNoDataValue(&valid);
        wassert(actual(missing) == 0);
        wassert(actual(valid) == TRUE);

        wassert(actual(gdal::read_int32(b,  0,   0)) == missing);  // Missing value on existing segment
        wassert(actual(gdal::read_int32(b,  5,  70)) == 119);      // Dark point
        wassert(actual(gdal::read_int32(b, 15, 135)) == 153);      // Grey point
        wassert(actual(gdal::read_int32(b, 62, 102)) == 670);      // Bright point
    }
}

void Tests::register_tests()
{
    ImportTest::register_tests();

    this->add_method("datatype", [](Fixture& f) {
        wassert(actual(f.dataset()->GetRasterCount()) == 1);
        GDALRasterBand* b = f.dataset()->GetRasterBand(1);
        wassert(actual(b->GetRasterDataType()) == GDT_UInt16);
        wassert(actual(b->GetOffset()).almost_equal(native_offset, 4));
        wassert(actual(b->GetScale()).almost_equal(native_scale, 2));
    });
}

}
