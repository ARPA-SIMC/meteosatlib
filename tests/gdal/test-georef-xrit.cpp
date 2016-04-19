#include "utils.h"

using namespace msat::tests;

namespace {

#define TESTFILE "H:MSG2:VIS006:200807150900"

class Tests : public FixtureTestCase<GDALFixture>
{
    using FixtureTestCase::FixtureTestCase;

    void register_tests() override;
} test("gdal_georef_xrit", "MsatXRIT", TESTFILE);

void Tests::register_tests()
{

// Test the subsatellite point
add_method("subsatellite", [](Fixture& f) {
    GeoReferencer gr(f.dataset());
    double lat, lon, px, py;
    int x, y;

    // Middle point maps to 0
    gr.pixelToProjected(1856, 1856, px, py);
    wassert(actual(px) == 0);
    wassert(actual(py) == 0);

    gr.projectedToLatlon(0, 0, lat, lon);
    wassert(actual(lat) == 0);
    wassert(actual(lon) == 0);

    gr.latlonToProjected(0, 0, px, py);
    wassert(actual(px) == 0);
    wassert(actual(py) == 0);

    gr.projectedToPixel(0, 0, x, y);
    wassert(actual(x) == 1856);
    wassert(actual(y) == 1856);

    gr.pixelToLatlon(1856, 1856, lat, lon);
    wassert(actual(lat) == 0);
    wassert(actual(lon) == 0);

    gr.latlonToPixel(0, 0, x, y);
    wassert(actual(x) == 1856);
    wassert(actual(y) == 1856);
});

// Test known points
add_method("known1", [](Fixture& f) {
    GeoReferencer gr(f.dataset());
    double lat, lon;

    gr.pixelToLatlon(2139, 456, lat, lon);
    wassert(actual(lat).almost_equal(44.4821, 3));
    wassert(actual(lon).almost_equal(11.3180, 3));

    gr.pixelToLatlon(2218,1492, lat, lon);
    wassert(actual(lat).almost_equal( 9.987689, 3));
    wassert(actual(lon).almost_equal(10.013122, 3));

    gr.pixelToLatlon(1494,2220, lat, lon);
    wassert(actual(lat).almost_equal( -9.987689, 3));
    wassert(actual(lon).almost_equal(-10.013122, 3));

    gr.pixelToLatlon(908, 3311, lat, lon);
    wassert(actual(lat).almost_equal(-49.973000, 3));
    wassert(actual(lon).almost_equal(-49.984040, 3));

    gr.pixelToLatlon(2804,3311, lat, lon);
    wassert(actual(lat).almost_equal(-49.973000, 3));
    wassert(actual(lon).almost_equal( 49.984040, 3));
});

// Test known points
add_method("known2", [](Fixture& f) {
    GeoReferencer gr(f.dataset());
    int x, y;

    gr.latlonToPixel(44.4949, 11.3211, x, y);
    wassert(actual(x) == 2139);
    wassert(actual(y) == 456);

    gr.latlonToPixel(10, 10, x, y);
    wassert(actual(x) == 2218);
    wassert(actual(y) == 1492);

    gr.latlonToPixel(-10, -10, x, y);
    wassert(actual(x) == 1494);
    wassert(actual(y) == 2220);

    gr.latlonToPixel(10, -10, x, y);
    wassert(actual(x) == 1494);
    wassert(actual(y) == 1492);

    gr.latlonToPixel(-10, 10, x, y);
    wassert(actual(x) == 2218);
    wassert(actual(y) == 2220);

    gr.latlonToPixel(-50, -50, x, y);
    wassert(actual(x) == 908);
    wassert(actual(y) == 3311);

    gr.latlonToPixel(50, -50, x, y);
    wassert(actual(x) == 908);
    wassert(actual(y) == 401);

    gr.latlonToPixel(-50, 50, x, y);
    wassert(actual(x) == 2804);
    wassert(actual(y) == 3311);

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
