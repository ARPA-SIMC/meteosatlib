#include "utils.h"
#include "msat/facts.h"

using namespace std;
using namespace msat::tests;

namespace {

#define TESTFILE "H:MSG1:HRV:200611141200"
#undef PERFORM_SLOW_TESTS
#define native_offset -1.63196
#define native_scale 0.0319993

struct Fixture : public GDALFixture
{
    using GDALFixture::GDALFixture;

    msat::proj::ImageBox get_crop_area() override
    {
        return msat::proj::ImageBox(msat::proj::ImagePoint(9000, 2900), msat::proj::ImagePoint(9000+150, 2900+300));
    }

    void check_general_image_data(GDALDataset* dataset);
    void check_full_image_data(GDALDataset* dataset) override;
    void check_cropped_image_data(GDALDataset* dataset) override;
};


class Tests : public ImportTest<Fixture>
{
    using ImportTest::ImportTest;

    void register_tests() override;
} test("gdal_import_xrit_hrv", "MsatXRIT", TESTFILE);

void Fixture::check_general_image_data(GDALDataset* dataset)
{
    const char* val = dataset->GetMetadataItem(MD_MSAT_DATETIME, MD_DOMAIN_MSAT);
    wassert(actual(val) == "2006-11-14 12:00:00");

    val = dataset->GetMetadataItem(MD_MSAT_SPACECRAFT_ID, MD_DOMAIN_MSAT);
    wassert(actual(val) == "55");
    val = dataset->GetMetadataItem(MD_MSAT_SPACECRAFT, MD_DOMAIN_MSAT);
    wassert(actual(val) == "MSG1");

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
        wassert(actual((double)gdal::read_float32(b, 0, 0)).almost_equal(missing, 4));
        wassert(actual((double)gdal::read_float32(b, 10, 10)).almost_equal(missing, 4));

        wassert(actual((double)gdal::read_float32(b,  5000, 2000)).almost_equal(missing, 4)); // Missing segment before Europe part
        wassert(actual((double)gdal::read_float32(b,  2500, 2900)).almost_equal(missing, 4)); // Missing data left of Europe part
        wassert(actual((double)gdal::read_float32(b,  4000, 2900)).almost_equal(SC( 90), 4)); // Dark spot in Europe part
        wassert(actual((double)gdal::read_float32(b,  4800, 2900)).almost_equal(SC(286), 4)); // Bright spot in Europe part
        wassert(actual((double)gdal::read_float32(b,  9200, 2900)).almost_equal(missing, 4)); // Missing data right of Europe part
        wassert(actual((double)gdal::read_float32(b,  7000, 3030)).almost_equal(missing, 4)); // Missing data between the two parts
        wassert(actual((double)gdal::read_float32(b,  5300, 3150)).almost_equal(missing, 4)); // Missing data left of Africa part
        wassert(actual((double)gdal::read_float32(b,  8600, 3111)).almost_equal(SC(90), 4)); // Dark spot in Africa part
        // 90 in stabilised area
        wassert(actual((double)gdal::read_float32(b,  8300, 3150)).almost_equal(SC(252), 4)); // Bright spot in Africa part
        // 252 in stabilised area
        wassert(actual((double)gdal::read_float32(b, 10410, 3100)).almost_equal(missing, 4)); // Missing data right of Africa part
        wassert(actual((double)gdal::read_float32(b,  6400, 3500)).almost_equal(missing, 4)); // Missing segment after Africa part
#undef SC
    } else {
        int valid;
        int missing = b->GetNoDataValue(&valid);
        wassert(actual(valid) == TRUE);
        wassert(actual(missing) == 0);

        wassert(actual(gdal::read_int32(b, 0, 0)) == missing);
        wassert(actual(gdal::read_int32(b, 10, 10)) == missing);

        wassert(actual(gdal::read_int32(b,  5000, 2000)) == missing); // Missing segment before Europe part
        wassert(actual(gdal::read_int32(b,  2500, 2900)) == missing); // Missing data left of Europe part
        wassert(actual(gdal::read_int32(b,  4000, 2900)) == 90); // Dark spot in Europe part
        wassert(actual(gdal::read_int32(b,  4800, 2900)) == 286); // Bright spot in Europe part
        wassert(actual(gdal::read_int32(b,  9200, 2900)) == missing); // Missing data right of Europe part
        wassert(actual(gdal::read_int32(b,  7000, 3030)) == missing); // Missing data between the two parts
        wassert(actual(gdal::read_int32(b,  5300, 3150)) == missing); // Missing data left of Africa part
        wassert(actual(gdal::read_int32(b,  8600, 3111)) ==  90); // Dark spot in Africa part
        wassert(actual(gdal::read_int32(b,  8300, 3150)) == 252); // Bright spot in Africa part
        wassert(actual(gdal::read_int32(b, 10410, 3100)) == missing); // Missing data right of Africa part
        wassert(actual(gdal::read_int32(b,  6400, 3500)) == missing); // Missing segment after Africa part
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
    wassert(actual(xs) == 5568 - 9000);
    wassert(actual(ys) == 5568 - 2900);
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
        wassert(actual((double)gdal::read_float32(b,  31,  19)).almost_equal(SC(116), 3)); // Dark spot in Europe part
        wassert(actual((double)gdal::read_float32(b,  31,  27)).almost_equal(SC(210), 3)); // Bright spot in Europe part
        wassert(actual((double)gdal::read_float32(b, 100,  60)).almost_equal(missing, 3)); // Missing data right of Europe part
        wassert(actual((double)gdal::read_float32(b,  20, 125)).almost_equal(missing, 3)); // Missing data between the two parts
        wassert(actual((double)gdal::read_float32(b,  84, 148)).almost_equal(SC(138), 3)); // Dark spot in Africa part
        wassert(actual((double)gdal::read_float32(b,  72, 143)).almost_equal(SC(219), 3)); // Bright spot in Africa part
#undef SC
    } else {
        int valid;
        int missing = b->GetNoDataValue(&valid);
        wassert(actual(missing) == 0);
        wassert(actual(valid) == TRUE);

        wassert(actual(gdal::read_int32(b,  31,  19)) == 116); // Dark spot in Europe part
        wassert(actual(gdal::read_int32(b,  31,  27)) == 210); // Bright spot in Europe part
        wassert(actual(gdal::read_int32(b, 100,  60)) == missing); // Missing data right of Europe part
        wassert(actual(gdal::read_int32(b,  20, 125)) == missing); // Missing data between the two parts
        wassert(actual(gdal::read_int32(b,  84, 148)) == 138); // Dark spot in Africa part
        wassert(actual(gdal::read_int32(b,  72, 143)) == 219); // Bright spot in Africa part
    }
}

void Tests::register_tests()
{
    ImportTest::register_tests();

    this->add_method("datatype", [](Fixture& f) {
        wassert(actual(f.dataset()->GetRasterCount()) == 1);
        GDALRasterBand* b = f.dataset()->GetRasterBand(1);
        wassert(actual(b->GetRasterDataType()) == GDT_UInt16);
        wassert(actual(b->GetOffset()).almost_equal(-1.63196, 5));
        wassert(actual(b->GetScale()).almost_equal(0.03200, 5));
    });
}

}
