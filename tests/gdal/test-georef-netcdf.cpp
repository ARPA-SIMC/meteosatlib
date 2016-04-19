#include "utils.h"

using namespace msat::tests;

namespace {

#define TESTFILE "MSG_Seviri_1_5_Infrared_10_8_channel_20051219_1415.nc"

class Tests : public FixtureTestCase<GDALFixture>
{
    using FixtureTestCase::FixtureTestCase;

    void register_tests() override;
} test("gdal_georef_netcdf", "MsatNetCDF", TESTFILE);

void Tests::register_tests()
{

// Test the subsatellite point
add_method("subsatellite", [](Fixture& f) {
    GeoReferencer gr(f.dataset());
    double lat, lon, px, py;
    int x, y;

    // Middle point maps to 0
    gr.pixelToProjected(1856-1499, 1856-199, px, py);
    wassert(actual(px).almost_equal(0, 9));
    wassert(actual(py).almost_equal(0, 9));

    gr.projectedToLatlon(0, 0, lat, lon);
    wassert(actual(lat) == 0);
    wassert(actual(lon) == 0);

    gr.latlonToProjected(0, 0, px, py);
    wassert(actual(px) == 0);
    wassert(actual(py) == 0);

    gr.projectedToPixel(0, 0, x, y);
    wassert(actual(x) == 1856-1499);
    wassert(actual(y) == 1856-199);

    gr.pixelToLatlon(1856-1499, 1856-199, lat, lon);
    wassert(actual(lat).almost_equal(0, 9));
    wassert(actual(lon).almost_equal(0, 9));

    gr.latlonToPixel(0, 0, x, y);
    wassert(actual(x) == 1856-1499);
    wassert(actual(y) == 1856-199);
});

// Test known points
add_method("known1", [](Fixture& f) {
    GeoReferencer gr(f.dataset());
    double lat, lon;

    gr.pixelToLatlon(0, 0, lat, lon);
    wassert(actual(lat).almost_equal( 59.567048, 3));
    wassert(actual(lon).almost_equal(-21.214644, 3));

    gr.pixelToLatlon(10, 10, lat, lon);
    wassert(actual(lat).almost_equal( 58.740333, 3));
    wassert(actual(lon).almost_equal(-20.028070, 3));

    gr.pixelToLatlon(100, 100, lat, lon);
    wassert(actual(lat).almost_equal( 52.518690, 3));
    wassert(actual(lon).almost_equal(-12.273015, 3));

    gr.pixelToLatlon(1000, 1000, lat, lon);
    wassert(actual(lat).almost_equal(18.514848, 3));
    wassert(actual(lon).almost_equal(18.968364, 3));

    gr.pixelToLatlon(1000, 100, lat, lon);
    wassert(actual(lat).almost_equal(53.965508, 3));
    wassert(actual(lon).almost_equal(34.081831, 3));

    gr.pixelToLatlon(300, 100, lat, lon);
    wassert(actual(lat).almost_equal(52.301633, 3));
    wassert(actual(lon).almost_equal(-2.681350, 3));

    gr.pixelToLatlon(100, 300, lat, lon);
    wassert(actual(lat).almost_equal(42.531680, 3));
    wassert(actual(lon).almost_equal(-9.892650, 3));
});

// Test known points
add_method("known2", [](Fixture& f) {
    GeoReferencer gr(f.dataset());
    int x, y;

    gr.latlonToPixel(40, 10, x, y);
    wassert(actual(x) == 628);
    wassert(actual(y) == 360);

    gr.latlonToPixel(10, 10, x, y);
    wassert(actual(x) == 719);
    wassert(actual(y) == 1293);

    gr.latlonToPixel(10, 40, x, y);
    wassert(actual(x) == 1641);
    wassert(actual(y) == 1308);

    gr.latlonToPixel(40, -10, x, y);
    wassert(actual(x) == 86);
    wassert(actual(y) == 360);
});

}

}
