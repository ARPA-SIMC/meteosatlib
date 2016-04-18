#include <msat/utils/tests.h>
#include <msat/xrit/fileaccess.h>

using namespace msat;
using namespace msat::tests;

namespace {

#define TESTDATA_LINEAR    DATA_DIR "/H:MSG2:VIS006:200807150900"
#define TESTDATA_HRV       DATA_DIR "/H:MSG1:HRV:200611141200"
#define TESTDATA_NONLINEAR DATA_DIR "/H:MSG1:IR_039:200611130800"

class Tests : public TestCase
{
    using TestCase::TestCase;

    void register_tests() override;
} test("msat_fileaccess");

void Tests::register_tests()
{

// Test the isXRIT function
add_method("isXRIT", []() {
    // Shortened
    wassert(actual(xrit::isValid(TESTDATA_LINEAR)).istrue());
    wassert(actual(xrit::isValid(TESTDATA_HRV)).istrue());
    wassert(actual(xrit::isValid(TESTDATA_NONLINEAR)).istrue());
    wassert(actual(xrit::isValid(DATA_DIR "/H:MSG1:HRV")).isfalse());
    wassert(actual(xrit::isValid(DATA_DIR)).isfalse());

    // Segment filename
    wassert(actual(xrit::isValid("/foo/bar/H-000-MSG1__-MSG1________-IR_039___-000001___-200611130800-C_")).istrue());
});

// Test parser with shortened names
add_method("shortnames", []() {
    xrit::FileAccess fa;
    fa.parse(TESTDATA_LINEAR);
    wassert(actual(fa.directory) == DATA_DIR);
    wassert(actual(fa.resolution) == "H");
    wassert(actual(fa.productid1) == "MSG2");
    wassert(actual(fa.productid2) == "VIS006");
    wassert(actual(fa.timing) == "200807150900");
});

// Test parser with segment filenames
add_method("segments", []() {
    xrit::FileAccess fa;
    fa.parse("/foo/bar/H-000-MSG1__-MSG1________-IR_039___-000001___-200611130800-C_");
    wassert(actual(fa.directory) == "/foo/bar");
    wassert(actual(fa.resolution) == "H");
    wassert(actual(fa.productid1) == "MSG1");
    wassert(actual(fa.productid2) == "IR_039");
    wassert(actual(fa.timing) == "200611130800");
});

// Test parser with alternate channel
add_method("altchannel", []() {
    xrit::FileAccess fa1;
    fa1.parse(TESTDATA_LINEAR);

    xrit::FileAccess fa;
    fa.parse(fa1, "PIPPO");

    wassert(actual(fa.directory) == DATA_DIR);
    wassert(actual(fa.resolution) == "H");
    wassert(actual(fa.productid1) == "MSG2");
    wassert(actual(fa.productid2) == "PIPPO");
    wassert(actual(fa.timing) == "200807150900");
});

}

}
