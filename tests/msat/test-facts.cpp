#include <msat/utils/tests.h>
#include <msat/facts.h>
#include <math.h>

using namespace msat;
using namespace msat::tests;

namespace {

class Tests : public TestCase
{
    using TestCase::TestCase;

    void register_tests() override;
} test("msat_facts");

void Tests::register_tests()
{

// Test the column factor/seviri DX/DY functions
add_method("seviri", []() {
    // This computation should be exact
    wassert(actual(facts::seviriDXFromColumnRes(13642337*exp2(-16))) == 3622); // Test with the real parameters
    wassert(actual(facts::seviriDYFromLineRes(13642337*exp2(-16))) == 3622); // Test with the real parameters

    // This computation is necessarily approximated, as we truncate significant digits
    wassert(actual(facts::columnResFromSeviriDX(3622)).almost_equal(13641224 * exp2(-16), 5));
    wassert(actual(facts::lineResFromSeviriDY(3622)).almost_equal(13641224 * exp2(-16), 5));

    // This computation is necessarily approximated, as we truncate significant digits
    wassert(actual(facts::columnResFromSeviriDX(facts::seviriDXFromColumnRes(13642337*exp2(-16)))).almost_equal(13641224*exp2(-16), 5));
    wassert(actual(facts::lineResFromSeviriDY(facts::seviriDYFromLineRes(13642337*exp2(-16)))).almost_equal(13641224*exp2(-16), 5));

    // This computation should be exact
    wassert(actual(facts::seviriDXFromColumnRes(facts::columnResFromSeviriDX(3622))) == 3622);
    wassert(actual(facts::seviriDXFromColumnRes(facts::columnResFromSeviriDX(3622))) == 3622);
});

// Test the satellite zenith angle calculation (FIXME: data not validated against known values, but seems to make sense)
add_method("sat_za", []() {
    wassert(actual(facts::sat_za(0, 0)) == 0);

    // Move north along the Greenwich meridian
    wassert(actual(round(facts::sat_za(10, 0) / M_PI * 180)) == 12);
    wassert(actual(round(facts::sat_za(20, 0) / M_PI * 180)) == 23);
    wassert(actual(round(facts::sat_za(30, 0) / M_PI * 180)) == 35);
    wassert(actual(round(facts::sat_za(40, 0) / M_PI * 180)) == 46);
    wassert(actual(round(facts::sat_za(50, 0) / M_PI * 180)) == 57);
    wassert(actual(round(facts::sat_za(60, 0) / M_PI * 180)) == 68);
    wassert(actual(round(facts::sat_za(70, 0) / M_PI * 180)) == 79);
    wassert(actual(round(facts::sat_za(80, 0) / M_PI * 180)) == 89);

    // Move south along the Greenwich meridian
    wassert(actual(round(facts::sat_za(-10, 0) / M_PI * 180)) == 12);
    wassert(actual(round(facts::sat_za(-20, 0) / M_PI * 180)) == 23);
    wassert(actual(round(facts::sat_za(-30, 0) / M_PI * 180)) == 35);
    wassert(actual(round(facts::sat_za(-40, 0) / M_PI * 180)) == 46);
    wassert(actual(round(facts::sat_za(-50, 0) / M_PI * 180)) == 57);
    wassert(actual(round(facts::sat_za(-60, 0) / M_PI * 180)) == 68);
    wassert(actual(round(facts::sat_za(-70, 0) / M_PI * 180)) == 79);
    wassert(actual(round(facts::sat_za(-80, 0) / M_PI * 180)) == 89);

    // Move east along the equator
    wassert(actual(round(facts::sat_za(0, 10) / M_PI * 180)) == 12);
    wassert(actual(round(facts::sat_za(0, 20) / M_PI * 180)) == 23);
    wassert(actual(round(facts::sat_za(0, 30) / M_PI * 180)) == 35);
    wassert(actual(round(facts::sat_za(0, 40) / M_PI * 180)) == 46);
    wassert(actual(round(facts::sat_za(0, 50) / M_PI * 180)) == 57);

    // Move diagonally
    wassert(actual(round(facts::sat_za(10, 10) / M_PI * 180)) == 17);
    wassert(actual(round(facts::sat_za(20, 20) / M_PI * 180)) == 33);
    wassert(actual(round(facts::sat_za(30, 30) / M_PI * 180)) == 48);
    wassert(actual(round(facts::sat_za(40, 40) / M_PI * 180)) == 62);
    wassert(actual(round(facts::sat_za(50, 50) / M_PI * 180)) == 74);
});

// Test the solar zenith angle calculation
// FIXME: data not validated against known values, but seems to make sense)
add_method("cos_sol_za", []() {
    // At an equinox
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 12, 0,   0, 0)).almost_equal( 1, 2));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 12, 0,  90, 0)).almost_equal( 0, 1));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 12, 0, -90, 0)).almost_equal( 0, 1));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 12, 0, 180, 0)).almost_equal(-1, 2));

    // Move north along the Greenwich meridian
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 12, 0, 10, 0)).almost_equal(0.98, 2));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 12, 0, 20, 0)).almost_equal(0.94, 2));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 12, 0, 30, 0)).almost_equal(0.86, 2));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 12, 0, 40, 0)).almost_equal(0.76, 2));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 12, 0, 45, 0)).almost_equal(0.705, 3));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 12, 0, 50, 0)).almost_equal(0.64, 2));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 12, 0, 60, 0)).almost_equal(0.50, 2));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 12, 0, 70, 0)).almost_equal(0.34, 2));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 12, 0, 80, 0)).almost_equal(0.17, 2));

    // Move south along the Greenwich meridian
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 12, 0, -10, 0)).almost_equal(0.98, 1));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 12, 0, -20, 0)).almost_equal(0.94, 1));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 12, 0, -30, 0)).almost_equal(0.86, 1));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 12, 0, -40, 0)).almost_equal(0.76, 1));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 12, 0, -45, 0)).almost_equal(0.71, 1));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 12, 0, -50, 0)).almost_equal(0.64, 1));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 12, 0, -60, 0)).almost_equal(0.50, 1));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 12, 0, -70, 0)).almost_equal(0.34, 1));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 12, 0, -80, 0)).almost_equal(0.17, 1));

    // Move east along the equator
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 12, 0, 0,  20)).almost_equal( 0.95, 1));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 12, 0, 0,  40)).almost_equal( 0.79, 1));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 12, 0, 0,  60)).almost_equal( 0.53, 1));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 12, 0, 0,  80)).almost_equal( 0.21, 1));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 12, 0, 0,  90)).almost_equal( 0.03, 1));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 12, 0, 0, 100)).almost_equal(-0.14, 1));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 12, 0, 0, 120)).almost_equal(-0.47, 1));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 12, 0, 0, 140)).almost_equal(-0.75, 1));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 12, 0, 0, 160)).almost_equal(-0.93, 1));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 12, 0, 0, 180)).almost_equal(-1.00, 1));

    // Move west along the equator
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 12, 0, 0,  -20)).almost_equal( 0.93, 1));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 12, 0, 0,  -40)).almost_equal( 0.75, 1));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 12, 0, 0,  -60)).almost_equal( 0.47, 1));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 12, 0, 0,  -80)).almost_equal( 0.14, 1));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 12, 0, 0,  -90)).almost_equal(-0.03, 1));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 12, 0, 0, -100)).almost_equal(-0.21, 1));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 12, 0, 0, -120)).almost_equal(-0.53, 1));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 12, 0, 0, -140)).almost_equal(-0.79, 1));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 12, 0, 0, -160)).almost_equal(-0.95, 1));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 12, 0, 0, -180)).almost_equal(-1.00, 1));

    // Hours of the day
    wassert(actual(facts::cos_sol_za(2013, 3, 21,  0, 0, 0, 0)).almost_equal(-1   , 2));
    wassert(actual(facts::cos_sol_za(2013, 3, 21,  3, 0, 0, 0)).almost_equal(-0.73, 2));
    wassert(actual(facts::cos_sol_za(2013, 3, 21,  6, 0, 0, 0)).almost_equal(-0.03, 2));
    wassert(actual(facts::cos_sol_za(2013, 3, 21,  9, 0, 0, 0)).almost_equal( 0.68, 2));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 12, 0, 0, 0)).almost_equal( 1   , 2));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 15, 0, 0, 0)).almost_equal( 0.73, 2));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 18, 0, 0, 0)).almost_equal( 0.03, 2));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 21, 0, 0, 0)).almost_equal(-0.68, 2));
    wassert(actual(facts::cos_sol_za(2013, 3, 22,  0, 0, 0, 0)).almost_equal(-1   , 2));

    // Move east along the equator, at 6UTC
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 6, 0, 0,   0)).almost_equal(-0.03, 1));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 6, 0, 0,  20)).almost_equal( 0.31, 1));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 6, 0, 0,  40)).almost_equal( 0.62, 1));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 6, 0, 0,  60)).almost_equal( 0.85, 1));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 6, 0, 0,  80)).almost_equal( 0.98, 1));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 6, 0, 0,  90)).almost_equal( 1.00, 1));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 6, 0, 0, 100)).almost_equal( 0.99, 1));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 6, 0, 0, 120)).almost_equal( 0.88, 1));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 6, 0, 0, 140)).almost_equal( 0.66, 1));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 6, 0, 0, 160)).almost_equal( 0.37, 1));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 6, 0, 0, 180)).almost_equal( 0.03, 1));

    // Move west along the equator, at 6UTC
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 6, 0, 0,  -20)).almost_equal(-0.37, 1));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 6, 0, 0,  -40)).almost_equal(-0.66, 1));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 6, 0, 0,  -60)).almost_equal(-0.88, 1));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 6, 0, 0,  -80)).almost_equal(-0.99, 1));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 6, 0, 0,  -90)).almost_equal(-1.00, 1));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 6, 0, 0, -100)).almost_equal(-0.98, 1));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 6, 0, 0, -120)).almost_equal(-0.85, 1));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 6, 0, 0, -140)).almost_equal(-0.62, 1));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 6, 0, 0, -160)).almost_equal(-0.31, 1));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 6, 0, 0, -180)).almost_equal( 0.03, 1));

    // Move south, at 6UTC UTC+6: should be the same as moving south at 12UTC
    // along the Greenwich meridian
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 6, 0, -10, 90)).almost_equal(0.98, 1));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 6, 0, -20, 90)).almost_equal(0.94, 1));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 6, 0, -30, 90)).almost_equal(0.86, 1));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 6, 0, -40, 90)).almost_equal(0.76, 1));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 6, 0, -45, 90)).almost_equal(0.71, 1));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 6, 0, -50, 90)).almost_equal(0.64, 1));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 6, 0, -60, 90)).almost_equal(0.50, 1));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 6, 0, -70, 90)).almost_equal(0.34, 1));
    wassert(actual(facts::cos_sol_za(2013, 3, 21, 6, 0, -80, 90)).almost_equal(0.17, 1));
});

}

}
