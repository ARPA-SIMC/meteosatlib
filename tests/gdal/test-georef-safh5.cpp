#include "utils.h"

using namespace msat::tests;

namespace {

#define TESTFILE "SAFNWC_MSG1_CRR__05339_025_MSGEURO_____.h5"

class Tests : public FixtureTestCase<GDALFixture>
{
    using FixtureTestCase::FixtureTestCase;

    void register_tests() override;
} test("gdal_georef_safh5", "MsatSAFH5", TESTFILE);

void Tests::register_tests()
{

// Test the subsatellite point
add_method("subsatellite", [](Fixture& f) {
    GeoReferencer gr(f.dataset());
    double lat, lon, px, py;
    int x, y;

    // Middle point maps to 0
    gr.pixelToProjected(1856-1500, 1856-200, px, py);
    wassert(actual(px).almost_equal(0, 9));
    wassert(actual(py).almost_equal(0, 9));

    gr.projectedToLatlon(0, 0, lat, lon);
    wassert(actual(lat) == 0);
    wassert(actual(lon) == 0);

    gr.latlonToProjected(0, 0, px, py);
    wassert(actual(px) == 0);
    wassert(actual(py) == 0);

    gr.projectedToPixel(0, 0, x, y);
    wassert(actual(x) == 1856-1500);
    wassert(actual(y) == 1856-200);

    gr.pixelToLatlon(1856-1500, 1856-200, lat, lon);
    wassert(actual(lat).almost_equal(0, 9));
    wassert(actual(lon).almost_equal(0, 9));

    gr.latlonToPixel(0, 0, x, y);
    wassert(actual(x) == 1856-1500);
    wassert(actual(y) == 1856-200);
});

// Test known points
add_method("known1", [](Fixture& f) {
    GeoReferencer gr(f.dataset());
    double lat, lon;

    gr.pixelToLatlon(0, 0, lat, lon);
    wassert(actual(lat).almost_equal( 59.482613, 3));
    wassert(actual(lon).almost_equal(-21.091290, 3));

    gr.pixelToLatlon(10, 10, lat, lon);
    wassert(actual(lat).almost_equal( 58.659716, 3));
    wassert(actual(lon).almost_equal(-19.914807, 3));

    gr.pixelToLatlon(100, 100, lat, lon);
    wassert(actual(lat).almost_equal( 52.458281, 3));
    wassert(actual(lon).almost_equal(-12.205514, 3));

    gr.pixelToLatlon(1000, 1000, lat, lon);
    wassert(actual(lat).almost_equal(18.485677, 3));
    wassert(actual(lon).almost_equal(18.995697, 3));

    gr.pixelToLatlon(1000, 100, lat, lon);
    wassert(actual(lat).almost_equal(53.906167, 3));
    wassert(actual(lon).almost_equal(34.082836, 3));

    gr.pixelToLatlon(300, 100, lat, lon);
    wassert(actual(lat).almost_equal(52.243525, 3));
    wassert(actual(lon).almost_equal(-2.630467, 3));

    gr.pixelToLatlon(100, 300, lat, lon);
    wassert(actual(lat).almost_equal(42.487134, 3));
    wassert(actual(lon).almost_equal(-9.845648, 3));
});

// Test known points
add_method("known2", [](Fixture& f) {
    GeoReferencer gr(f.dataset());
    int x, y;

    gr.latlonToPixel(40, 10, x, y);
    wassert(actual(x) == 627);
    wassert(actual(y) == 359);

    gr.latlonToPixel(10, 10, x, y);
    wassert(actual(x) == 718);
    wassert(actual(y) == 1292);

    gr.latlonToPixel(10, 40, x, y);
    wassert(actual(x) == 1640);
    wassert(actual(y) == 1307);

    gr.latlonToPixel(40, -10, x, y);
    wassert(actual(x) == 85);
    wassert(actual(y) == 359);
});

}

}
