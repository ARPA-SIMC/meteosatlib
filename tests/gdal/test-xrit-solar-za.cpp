#include "utils.h"
#include <cstdint>

using namespace std;
using namespace msat::tests;

namespace {

class Tests : public TestCase
{
    using TestCase::TestCase;

    void register_tests() override;
} test("gdal_xrit_solar_za");

void Tests::register_tests()
{

// Test opening channel 1 (VIS 0.6, with reflectance)
add_method("vis06", []{
    unique_ptr<GDALDataset> dataset = gdal::open_ro("H:MSG2:VIS006:200807150900");
    wassert(actual(dataset.get() != 0).istrue());
    wassert(actual(string(GDALGetDriverShortName(dataset->GetDriver()))) == "MsatXRIT");

    // Check that we have the real and the virtual raster bands
    wassert(actual(dataset->GetRasterCount()) == 1);

    // x:2000,y:3400
    GDALRasterBand* rb = dataset->GetRasterBand(1);
    uint16_t val;
    rb->RasterIO(GF_Read, 2000, 3400, 1, 1, &val, 1, 1, GDT_UInt16, 0, 0);
    wassert(actual(val) == 96);

    unique_ptr<GDALDataset> datasetr = gdal::open_ro("H:MSG2:VIS006a:200807150900");
    wassert(actual(dataset.get() != 0).istrue());
    wassert(actual(string(GDALGetDriverShortName(datasetr->GetDriver()))) == "MsatXRIT");

    // Check that we have the real and the virtual raster bands
    wassert(actual(datasetr->GetRasterCount()) == 1);

    rb = datasetr->GetRasterBand(1);
    float valr;
    rb->RasterIO(GF_Read, 2000, 3400, 1, 1, &valr, 1, 1, GDT_Float32, 0, 0);
    wassert(actual((double)valr).almost_equal(0.156248, 3));
});

// Test opening channel 4 (IR 0.39, with missing accessory channels)
add_method("ir039_missing", []{
    try {
        unique_ptr<GDALDataset> dataset = gdal::open_ro("H:MSG1:IR_039a:200611130800");
        wassert(actual(false).istrue());
    } catch (...) {
    }
});

// Test opening channel 4 (IR 0.39, with all accessory channels yet)
add_method("ir039", []{
    unique_ptr<GDALDataset> dataset = gdal::open_ro("H:MSG2:IR_039:201001191200");
    wassert(actual(dataset.get() != 0).istrue());
    wassert(actual(string(GDALGetDriverShortName(dataset->GetDriver()))) == "MsatXRIT");

    // Check that we have the real and the virtual raster bands
    wassert(actual(dataset->GetRasterCount()) == 1);

    // x:2000,y:350
    GDALRasterBand* rb = dataset->GetRasterBand(1);
    uint16_t val;
    rb->RasterIO(GF_Read, 2000, 350, 1, 1, &val, 1, 1, GDT_UInt16, 0, 0);
    wassert(actual(val) == 287);

    unique_ptr<GDALDataset> datasetr = gdal::open_ro("H:MSG2:IR_039a:201001191200");
    wassert(actual(dataset.get() != 0).istrue());
    wassert(actual(string(GDALGetDriverShortName(datasetr->GetDriver()))) == "MsatXRIT");

    // Check that we have the real and the virtual raster bands
    wassert(actual(datasetr->GetRasterCount()) == 1);

    rb = datasetr->GetRasterBand(1);
    float valr;
    rb->RasterIO(GF_Read, 2000, 350, 1, 1, &valr, 1, 1, GDT_Float32, 0, 0);
    wassert(actual((double)valr).almost_equal(0.33964, 3));
});

// Test opening channel 12 (HRV, with reflectance)
add_method("hrv", []{
    unique_ptr<GDALDataset> dataset = gdal::open_ro("H:MSG1:HRVa:200611141200");
    wassert(actual(dataset.get() != 0).istrue());
    wassert(actual(string(GDALGetDriverShortName(dataset->GetDriver()))) == "MsatXRIT");

    // Check that we have the real and the virtual raster bands
    wassert(actual(dataset->GetRasterCount()) == 1);
});

}

}
