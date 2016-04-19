#include "utils.h"

using namespace msat::tests;

namespace {

#define TESTFILE "H:MSG1:HRV:200611141200"

class Tests : public FixtureTestCase<GDALFixture>
{
    using FixtureTestCase::FixtureTestCase;

    void register_tests() override;
} test("gdal_georef_xrit_hrv", "MsatXRIT", TESTFILE);

void Tests::register_tests()
{

// Test the subsatellite point
add_method("subsatellite", [](Fixture& f) {
    GeoReferencer gr(f.dataset());
    double lat, lon, px, py;
    int x, y;

    // Middle point maps to 0
    gr.pixelToProjected(5568, 5568, px, py);
    wassert(actual(px) == 0);
    wassert(actual(py) == 0);

    gr.projectedToLatlon(0, 0, lat, lon);
    wassert(actual(lat) == 0);
    wassert(actual(lon) == 0);

    gr.latlonToProjected(0, 0, px, py);
    wassert(actual(px) == 0);
    wassert(actual(py) == 0);

    gr.projectedToPixel(0, 0, x, y);
    wassert(actual(x) == 5568);
    wassert(actual(y) == 5568);

    gr.pixelToLatlon(5568, 5568, lat, lon);
    wassert(actual(lat) == 0);
    wassert(actual(lon) == 0);

    gr.latlonToPixel(0, 0, x, y);
    wassert(actual(x) == 5568);
    wassert(actual(y) == 5568);
});

// Test known points
add_method("known1", [](Fixture& f) {
    GeoReferencer gr(f.dataset());
    double lat, lon;

    gr.pixelToLatlon(6415, 1365, lat, lon);
    wassert(actual(lat).almost_equal(44.5273, 3));
    wassert(actual(lon).almost_equal(11.3008, 3));

    gr.pixelToLatlon(6651,4473, lat, lon);
    wassert(actual(lat).almost_equal(10.01540, 3));
    wassert(actual(lon).almost_equal( 9.98602, 3));

    gr.pixelToLatlon(4481,6659, lat, lon);
    wassert(actual(lat).almost_equal( -9.97845, 3));
    wassert(actual(lon).almost_equal(-10.02220, 3));

    gr.pixelToLatlon(2723,9932, lat, lon);
    wassert(actual(lat).almost_equal(-49.9556, 3));
    wassert(actual(lon).almost_equal(-49.9816, 3));

    gr.pixelToLatlon(8409,9932, lat, lon);
    wassert(actual(lat).almost_equal(-49.9414, 3));
    wassert(actual(lon).almost_equal( 49.8518, 3));
});

// Test known points
add_method("known2", [](Fixture& f) {
    GeoReferencer gr(f.dataset());
    int x, y;

    gr.latlonToPixel(44.4949, 11.3211, x, y);
    wassert(actual(x) == 6417);
    wassert(actual(y) == 1367);

    gr.latlonToPixel(10, 10, x, y);
    wassert(actual(x) == 6653);
    wassert(actual(y) == 4475);

    gr.latlonToPixel(-10, -10, x, y);
    wassert(actual(x) == 4483);
    wassert(actual(y) == 6661);

    gr.latlonToPixel(10, -10, x, y);
    wassert(actual(x) == 4483);
    wassert(actual(y) == 4475);

    gr.latlonToPixel(-10, 10, x, y);
    wassert(actual(x) == 6653);
    wassert(actual(y) == 6661);

    gr.latlonToPixel(-50, -50, x, y);
    wassert(actual(x) == 2725);
    wassert(actual(y) == 9934);

    gr.latlonToPixel(50, -50, x, y);
    wassert(actual(x) == 2725);
    wassert(actual(y) == 1202);

    gr.latlonToPixel(-50, 50, x, y);
    wassert(actual(x) == 8411);
    wassert(actual(y) == 9934);

    // (50.000000,50.000000) -> (8409,1200)
    // (80.000000,80.000000) -> (6499,271)
    // (-80.000000,80.000000) -> (6499,10861)
    // (80.000000,-80.000000) -> (4633,271)

#if 0
    gr.pixelToLatlon(4633,10861, lat, lon);
    wassert(actual(lat).almost_equal(-71.679389, 3);
    wassert(actual(lon).almost_equal(-31.643353, 3);
    gr.latlonToPixel(-80, -80, x, y);
    wassert(actual(x) == 4633);
    wassert(actual(y) == 10861);
#endif
});

}

}
