#include "utils.h"
#include <cstdint>

using namespace std;
using namespace msat::tests;

namespace {

class Tests : public TestCase
{
    using TestCase::TestCase;

    void register_tests() override;
} test("gdal_xrit_reflectance");

void Tests::register_tests()
{

// Test opening channel 1 (VIS 0.6, with reflectance)
add_method("vis06", []{
    unique_ptr<GDALDataset> dataset = gdal::open_ro("H:MSG2:VIS006:200807150900");
    wassert(actual(dataset.get() != 0).istrue());
    wassert(actual(GDALGetDriverShortName(dataset->GetDriver())) == "MsatXRIT");

    // Check that we have the real and the virtual raster bands
    wassert(actual(dataset->GetRasterCount()) == 1);

    // x:2000,y:3400
    GDALRasterBand* rb = dataset->GetRasterBand(1);
    uint16_t val;
    wassert(actual(rb->RasterIO(GF_Read, 2000, 3400, 1, 1, &val, 1, 1, GDT_UInt16, 0, 0)) == CE_None);
    wassert(actual(val) == 96);

    unique_ptr<GDALDataset> datasetr = gdal::open_ro("H:MSG2:VIS006r:200807150900");
    wassert(actual(dataset.get() != 0).istrue());
    wassert(actual(string(GDALGetDriverShortName(datasetr->GetDriver()))) == "MsatXRIT");

    // Check that we have the real and the virtual raster bands
    wassert(actual(datasetr->GetRasterCount()) == 1);

    rb = datasetr->GetRasterBand(1);
    float valr;
    wassert(actual(rb->RasterIO(GF_Read, 2000, 3400, 1, 1, &valr, 1, 1, GDT_Float32, 0, 0)) == CE_None);
    wassert(actual((double)valr).almost_equal(25.9648, 3));
});

// Test opening channel 4 (IR 0.39, with missing accessory channels)
add_method("ir039_missing", []{
    try {
        unique_ptr<GDALDataset> dataset = gdal::open_ro("H:MSG1:IR_039r:200611130800");
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
    wassert(actual(rb->RasterIO(GF_Read, 2000, 350, 1, 1, &val, 1, 1, GDT_UInt16, 0, 0)) == CE_None);
    wassert(actual(val) == 287);

    unique_ptr<GDALDataset> datasetr = gdal::open_ro("H:MSG2:IR_039r:201001191200");
    wassert(actual(dataset.get() != 0).istrue());
    wassert(actual(string(GDALGetDriverShortName(datasetr->GetDriver()))) == "MsatXRIT");

    // Check that we have the real and the virtual raster bands
    wassert(actual(datasetr->GetRasterCount()) == 1);

    rb = datasetr->GetRasterBand(1);
    float valr;
    wassert(actual(rb->RasterIO(GF_Read, 2000, 350, 1, 1, &valr, 1, 1, GDT_Float32, 0, 0)) == CE_None);
    wassert(actual((double)valr).almost_equal(22.3242, 3));
});

// Test opening channel 12 (HRV, with reflectance)
add_method("hrv", []{
    unique_ptr<GDALDataset> dataset = gdal::open_ro("H:MSG1:HRVr:200611141200");
    wassert(actual(dataset.get() != 0).istrue());
    wassert(actual(string(GDALGetDriverShortName(dataset->GetDriver()))) == "MsatXRIT");

    // Check that we have the real and the virtual raster bands
    wassert(actual(dataset->GetRasterCount()) == 1);
});



// Test opening channel 1 (VIS 0.6, with reflectance)
add_method("new_vis06", []{
    unique_ptr<GDALDataset> dataset = gdal::open_ro("H:MSG2:VIS006:200807150900");
    wassert(actual(dataset.get() != 0).istrue());
    wassert(actual(GDALGetDriverShortName(dataset->GetDriver())) == "MsatXRIT");

    // Check that we have the real and the virtual raster bands
    wassert(actual(dataset->GetRasterCount()) == 1);

    // x:2000,y:3400
    GDALRasterBand* rb = dataset->GetRasterBand(1);
    uint16_t val;
    wassert(actual(rb->RasterIO(GF_Read, 2000, 3400, 1, 1, &val, 1, 1, GDT_UInt16, 0, 0)) == CE_None);
    wassert(actual(val) == 96);

    CPLStringList opts(nullptr);
    opts.SetNameValue("MSAT_COMPUTE", "reflectance");
    unique_ptr<GDALDataset> datasetr = gdal::open_ro("H:MSG2:VIS006:200807150900", opts);
    wassert(actual(dataset.get() != 0).istrue());
    wassert(actual(string(GDALGetDriverShortName(datasetr->GetDriver()))) == "MsatXRIT");

    // Check that we have the real and the virtual raster bands
    wassert(actual(datasetr->GetRasterCount()) == 1);

    rb = datasetr->GetRasterBand(1);
    float valr;
    wassert(actual(rb->RasterIO(GF_Read, 2000, 3400, 1, 1, &valr, 1, 1, GDT_Float32, 0, 0)) == CE_None);
    wassert(actual((double)valr).almost_equal(25.9648, 3));
});


// Test opening channel 4 (IR 0.39), computing satellite zenith angle
add_method("new_ir039_sat_za", []{
    CPLStringList opts(nullptr);
    opts.SetNameValue("MSAT_COMPUTE", "sat_za");
    unique_ptr<GDALDataset> dataset = gdal::open_ro("H:MSG1:IR_039:200611130800", opts);
    wassert(actual(dataset.get() != 0).istrue());
    wassert(actual(GDALGetDriverShortName(dataset->GetDriver())) == "MsatXRIT");

    // Check that we have the real and the virtual raster bands
    wassert(actual(dataset->GetRasterCount()) == 1);

    GDALRasterBand* rb = dataset->GetRasterBand(1);
    float valr;
    wassert(actual(rb->RasterIO(GF_Read, 2000, 3400, 1, 1, &valr, 1, 1, GDT_Float32, 0, 0)) == CE_None);
    wassert(actual((double)valr).almost_equal(0.623, 3));
});

// Test opening channel 4 (IR 0.39, with missing accessory channels)
add_method("new_ir039_missing", []{
    try {
        CPLStringList opts(nullptr);
        opts.SetNameValue("MSAT_COMPUTE", "reflectance");
        unique_ptr<GDALDataset> dataset = gdal::open_ro("H:MSG1:IR_039:200611130800", opts);
        wassert(actual(false).istrue());
    } catch (...) {
    }
});

// Test opening channel 4 (IR 0.39, with all accessory channels yet)
add_method("new_ir039", []{
#if 0
    // FIXME: this is not yet supported, because to compute reflectance of IR
    // 0.39 we need two more additional channels

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

    CPLStringList opts(nullptr);
    opts.SetNameValue("MSAT_COMPUTE", "reflectance");
    unique_ptr<GDALDataset> datasetr = gdal::open_ro("H:MSG2:IR_039:201001191200", opts);
    wassert(actual(dataset.get() != 0).istrue());
    wassert(actual(string(GDALGetDriverShortName(datasetr->GetDriver()))) == "MsatXRIT");

    // Check that we have the real and the virtual raster bands
    wassert(actual(datasetr->GetRasterCount()) == 1);

    rb = datasetr->GetRasterBand(1);
    float valr;
    rb->RasterIO(GF_Read, 2000, 350, 1, 1, &valr, 1, 1, GDT_Float32, 0, 0);
    wassert(actual((double)valr).almost_equal(22.3242, 3));
#endif
});

// Test opening channel 12 (HRV, with reflectance)
add_method("new_hrv", []{
    CPLStringList opts(nullptr);
    opts.SetNameValue("MSAT_COMPUTE", "reflectance");
    unique_ptr<GDALDataset> dataset = gdal::open_ro("H:MSG1:HRV:200611141200", opts);
    wassert(actual(dataset.get() != 0).istrue());
    wassert(actual(string(GDALGetDriverShortName(dataset->GetDriver()))) == "MsatXRIT");

    // Check that we have the real and the virtual raster bands
    wassert(actual(dataset->GetRasterCount()) == 1);
});

}

}
