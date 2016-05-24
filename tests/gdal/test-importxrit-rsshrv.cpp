#include "utils.h"
#include "msat/facts.h"

using namespace std;
using namespace msat::tests;

namespace {

#define TESTFILE "rss/H:MSG2_RSS:HRV:201604281230"
#undef PERFORM_SLOW_TESTS
#define native_offset -1.5574
#define native_scale 0.030538

struct Fixture : public GDALFixture
{
    using GDALFixture::GDALFixture;

    msat::proj::ImageBox get_crop_area()
    {
        return msat::proj::ImageBox(msat::proj::ImagePoint(4200, 300), msat::proj::ImagePoint(4200 + 150, 300 + 200));
    }

    void check_general_image_data(GDALDataset* dataset);
    void check_full_image_data(GDALDataset* dataset);
    void check_cropped_image_data(GDALDataset* dataset);
};


class Tests : public ImportTest<Fixture>
{
    using ImportTest::ImportTest;

    void register_tests() override;
} test("gdal_import_xrit_rss_hrv", "MsatXRIT", TESTFILE);

void Fixture::check_general_image_data(GDALDataset* dataset)
{
    const char* val = dataset->GetMetadataItem(MD_MSAT_DATETIME, MD_DOMAIN_MSAT);
    wassert(actual(val) == "2016-04-28 12:30:00");

    val = dataset->GetMetadataItem(MD_MSAT_SPACECRAFT_ID, MD_DOMAIN_MSAT);
    wassert(actual(val) == "56");
    val = dataset->GetMetadataItem(MD_MSAT_SPACECRAFT, MD_DOMAIN_MSAT);
    wassert(actual(val) == "MSG2");

    GDALRasterBand* b = dataset->GetRasterBand(1);

    val = b->GetMetadataItem(MD_MSAT_CHANNEL_ID, MD_DOMAIN_MSAT);
    wassert(actual(val) == "12");
    val = b->GetMetadataItem(MD_MSAT_CHANNEL, MD_DOMAIN_MSAT);
    wassert(actual(val) == "HRV");

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
        wassert(actual(b->GetScale(&valid)).almost_equal(native_scale, 6));
        wassert(actual(valid) == TRUE);
    }
}

void Fixture::check_full_image_data(GDALDataset* dataset)
{
    check_general_image_data(dataset);

    wassert(actual(dataset->GetRasterXSize()) == 11136);
    wassert(actual(dataset->GetRasterYSize()) == 11136);

    int xs, ys;
    double rx, ry;
    msat::dataset::decodeGeotransform(dataset, xs, ys, rx, ry);
    wassert(actual(xs) == 5568);
    wassert(actual(ys) == 5568);
    wassert(actual(rx).almost_equal(METEOSAT_PIXELSIZE_X_HRV, 1));
    wassert(actual(ry).almost_equal(METEOSAT_PIXELSIZE_Y_HRV, 1));

    GDALRasterBand* b = dataset->GetRasterBand(1);

    if (b->GetRasterDataType() == GDT_Float64)
    {
        int valid;
        double missing = b->GetNoDataValue(&valid);
        wassert(actual(valid) == TRUE);
        wassert(actual(missing) == -1.63196);
#define SC(x) ((x)*native_scale + native_offset)
        wassert(actual((double)gdal::read_float32(b,     0,    0)).almost_equal(missing, 4));
        wassert(actual((double)gdal::read_float32(b,    10,   10)).almost_equal(missing, 4));
        wassert(actual((double)gdal::read_float32(b,  4200,  200)).almost_equal(missing, 4)); // Missing segment before RSS part
        wassert(actual((double)gdal::read_float32(b,  4200,  300)).almost_equal(missing, 4)); // Missing data left RSS part
        wassert(actual((double)gdal::read_float32(b,  4160,  420)).almost_equal(SC(144), 4)); // Dark spot in RSS part
        wassert(actual((double)gdal::read_float32(b,  4160,  390)).almost_equal(SC(216), 4)); // Gray spot in RSS part
        wassert(actual((double)gdal::read_float32(b,  4246,  442)).almost_equal(SC(500), 4)); // Bright spot in RSS part
        wassert(actual((double)gdal::read_float32(b,  4200, 1200)).almost_equal(missing, 4)); // Missing segment after RSS part
        wassert(actual((double)gdal::read_float32(b,  6000, 9000)).almost_equal(missing, 4)); // Missing data after RSS part
#undef SC
    } else {
        int valid;
        int missing = b->GetNoDataValue(&valid);
        wassert(actual(valid) == TRUE);
        wassert(actual(missing) == 0);

        wassert(actual(gdal::read_int32(b,     0,    0)) == missing);
        wassert(actual(gdal::read_int32(b,    10,   10)) == missing);
        wassert(actual(gdal::read_int32(b,  4200,  200)) == missing); // Missing segment before RSS part
        wassert(actual(gdal::read_int32(b,  4200,  300)) == missing); // Missing data left RSS part
        wassert(actual(gdal::read_int32(b,  4160,  420)) == 144);     // Dark spot in RSS part
        wassert(actual(gdal::read_int32(b,  4160,  390)) == 216);     // Gray spot in RSS part
        wassert(actual(gdal::read_int32(b,  4246,  442)) == 500);     // Bright spot in RSS part
        wassert(actual(gdal::read_int32(b,  4200, 1200)) == missing); // Missing segment after RSS part
        wassert(actual(gdal::read_int32(b,  6000, 9000)) == missing); // Missing data after RSS part
    }
}

void Fixture::check_cropped_image_data(GDALDataset* dataset)
{
    check_general_image_data(dataset);

    wassert(actual(dataset->GetRasterXSize()) == 150);
    wassert(actual(dataset->GetRasterYSize()) == 200);

    int xs, ys;
    double rx, ry;
    msat::dataset::decodeGeotransform(dataset, xs, ys, rx, ry);
    wassert(actual(xs) == 5568 - 4200);
    wassert(actual(ys) == 5568 -  300);
    wassert(actual(rx).almost_equal(METEOSAT_PIXELSIZE_X_HRV, 1));
    wassert(actual(ry).almost_equal(METEOSAT_PIXELSIZE_Y_HRV, 1));

    GDALRasterBand* b = dataset->GetRasterBand(1);

    if (b->GetRasterDataType() == GDT_Float64)
    {
        int valid;
        double missing = b->GetNoDataValue(&valid);
        wassert(actual(missing) == -1.63196);
        wassert(actual(valid) == TRUE);

#define SC(x) ((x)*native_scale + native_offset)
        wassert(actual((double)gdal::read_float32(b,  25,  10)).almost_equal(missing, 3)); // Missing data left of the planet
        wassert(actual((double)gdal::read_float32(b, 100,  90)).almost_equal(SC(149), 3));     // Dark spot
        wassert(actual((double)gdal::read_float32(b,  34,  87)).almost_equal(SC(212), 3));     // Gray spot
        wassert(actual((double)gdal::read_float32(b,  46, 142)).almost_equal(SC(500), 3));     // Bright spot
#undef SC
    } else {
        int valid;
        int missing = b->GetNoDataValue(&valid);
        wassert(actual(missing) == 0);
        wassert(actual(valid) == TRUE);

        wassert(actual(gdal::read_int32(b,  25,  10)) == missing); // Missing data left of the planet
        wassert(actual(gdal::read_int32(b, 100,  90)) == 149);     // Dark spot
        wassert(actual(gdal::read_int32(b,  34,  87)) == 212);     // Gray spot
        wassert(actual(gdal::read_int32(b,  46, 142)) == 500);     // Bright spot
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
