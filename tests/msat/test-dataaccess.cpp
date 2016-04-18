#include <msat/utils/tests.h>
#include <msat/xrit/dataaccess.h>
#include <msat/xrit/fileaccess.h>
#include <msat/hrit/MSG_HRIT.h>

using namespace msat::xrit;
using namespace msat::tests;

namespace tut {

#define TESTDATA_LINEAR    DATA_DIR "/H:MSG2:VIS006:200807150900"
#define TESTDATA_HRV       DATA_DIR "/H:MSG1:HRV:200611141200"
#define TESTDATA_NONLINEAR DATA_DIR "/H:MSG1:IR_039:200611130800"

class Tests : public TestCase
{
    using TestCase::TestCase;

    void register_tests() override;
} test("msat_dataaccess");

void Tests::register_tests()
{

add_method("scan_linear", []() {
    FileAccess fa(TESTDATA_LINEAR);
    DataAccess da;

    MSG_data pro;
    MSG_data epi;
    MSG_header header;
    da.scan(fa, pro, epi, header);

    wassert(actual(da.npixperseg) == 3712u * 464u);
    wassert(actual(da.seglines) == 464u);
    wassert(actual(da.swapX) == true);
    wassert(actual(da.swapY) == true);
    wassert(actual(da.hrv) == false);
    wassert(actual(da.segnames.size()) == 1u);
    wassert(actual(da.columns) == 3712u);
    wassert(actual(da.lines) == 3712u);
    wassert(actual(da.SouthLineActual) == 1u);
    wassert(actual(da.WestColumnActual) == 1u);

    wassert(actual(da.line_start(   0)) == 0u);
    wassert(actual(da.line_start(1856)) == 0u);
    wassert(actual(da.line_start(3712)) == 0u);
    wassert(actual(da.line_start(9999)) == 0u);

    wassert(actual(da.segment(0) != NULL).istrue());

    MSG_SAMPLE buf[3712];
    wassert(da.line_read(0, buf));
});

add_method("scan_nonlinear", []() {
    FileAccess fa(TESTDATA_NONLINEAR);
    DataAccess da;

    MSG_data pro;
    MSG_data epi;
    MSG_header header;
    da.scan(fa, pro, epi, header);

    wassert(actual(da.npixperseg) == 3712u * 464u);
    wassert(actual(da.seglines) == 464u);
    wassert(actual(da.swapX) == true);
    wassert(actual(da.swapY) == true);
    wassert(actual(da.hrv) == false);
    wassert(actual(da.segnames.size()) == 1u);
    wassert(actual(da.columns) == 3712u);
    wassert(actual(da.lines) == 3712u);
    wassert(actual(da.SouthLineActual) == 1u);
    wassert(actual(da.WestColumnActual) == 1u);

    wassert(actual(da.line_start(   0)) == 0u);
    wassert(actual(da.line_start(1856)) == 0u);
    wassert(actual(da.line_start(3712)) == 0u);
    wassert(actual(da.line_start(9999)) == 0u);

    wassert(actual(da.segment(0) != NULL).istrue());

    MSG_SAMPLE buf[3712];
    wassert(da.line_read(0, buf));
});

add_method("scan_hrv", []() {
    FileAccess fa(TESTDATA_HRV);
    DataAccess da;

    MSG_data pro;
    MSG_data epi;
    MSG_header header;
    da.scan(fa, pro, epi, header);

    wassert(actual(da.npixperseg) == 11136u * 232u);
    wassert(actual(da.seglines) == 464u);
    wassert(actual(da.swapX) == true);
    wassert(actual(da.swapY) == true);
    wassert(actual(da.hrv) == true);
    wassert(actual(da.segnames.size()) == 18u);
    wassert(actual(da.columns) == 5568u);
    wassert(actual(da.lines) == 11136u);

    wassert(actual(da.LowerEastColumnActual) == 1u);
    wassert(actual(da.LowerSouthLineActual) == 1u);
    wassert(actual(da.LowerWestColumnActual) == 5568u);
    wassert(actual(da.LowerNorthLineActual) == 8128u);
    wassert(actual(da.UpperEastColumnActual) == 2064u);
    wassert(actual(da.UpperSouthLineActual) == 8129u);
    wassert(actual(da.UpperWestColumnActual) == 7631u);
    wassert(actual(da.UpperNorthLineActual) == 11136u);

    wassert(actual(da.line_start(    0)) == 11136u-7631u);
    wassert(actual(da.line_start( 1856)) == 11136u-7631u);
    wassert(actual(da.line_start( 3007)) == 11136u-7631u);
    wassert(actual(da.line_start( 3008)) == 11136u-5568u);
    wassert(actual(da.line_start( 9999)) == 11136u-5568u);
    wassert(actual(da.line_start(11135)) == 11136u-5568u);
    wassert(actual(da.line_start(11136)) == 0u);

    wassert(actual(da.segment(0) == NULL).istrue());
    wassert(actual(da.segment(17) != NULL).istrue());
    wassert(actual(da.segment(18) == NULL).istrue());

    MSG_SAMPLE buf[11136];
    wassert(da.line_read(0, buf));
});

}

}
