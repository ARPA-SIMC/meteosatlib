#include "utils.h"
#include "msat/facts.h"

using namespace std;
using namespace msat::tests;

namespace {

#define TESTFILE "H:MSG1:IR_039:200611130800"
#define PERFORM_SLOW_TESTS

struct Fixture : public GDALFixture
{
    using GDALFixture::GDALFixture;

    msat::proj::ImageBox get_crop_area() override
    {
        return msat::proj::ImageBox(msat::proj::ImagePoint(2540, 2950), msat::proj::ImagePoint(2540+150, 2950+300));

    }

    void check_general_image_data(GDALDataset* dataset);
    void check_full_image_data(GDALDataset* dataset) override;
    void check_cropped_image_data(GDALDataset* dataset) override;
};


class Tests : public ImportTest<Fixture>
{
    using ImportTest::ImportTest;

    void register_tests() override;
} test("gdal_import_xrit_nonlinear", "MsatXRIT", TESTFILE);

void Fixture::check_general_image_data(GDALDataset* dataset)
{
    const char* val = dataset->GetMetadataItem(MD_MSAT_DATETIME, MD_DOMAIN_MSAT);
    wassert(actual(val) == "2006-11-13 08:00:00");

    val = dataset->GetMetadataItem(MD_MSAT_SPACECRAFT_ID, MD_DOMAIN_MSAT);
    wassert(actual(val) == "55");
    val = dataset->GetMetadataItem(MD_MSAT_SPACECRAFT, MD_DOMAIN_MSAT);
    wassert(actual(val) == "MSG1");

    GDALRasterBand* b = dataset->GetRasterBand(1);

    val = b->GetMetadataItem(MD_MSAT_CHANNEL_ID, MD_DOMAIN_MSAT);
    wassert(actual(val) == "4");
    val = b->GetMetadataItem(MD_MSAT_CHANNEL, MD_DOMAIN_MSAT);
    wassert(actual(val) == "IR_039");

    wassert(actual(b->GetUnitType()) == "K");

    int valid;
    wassert(actual(b->GetOffset(&valid)) == 0);
    wassert(actual(valid) == TRUE);
    wassert(actual(b->GetScale(&valid)) == 1);
    wassert(actual(valid) == TRUE);
}

void Fixture::check_full_image_data(GDALDataset* dataset)
{
    check_general_image_data(dataset);

    wassert(actual(dataset->GetRasterXSize()) == 3712);
    wassert(actual(dataset->GetRasterYSize()) == 3712);

    int xs, ys;
    double rx, ry;
    msat::dataset::decodeGeotransform(dataset, xs, ys, rx, ry);
    wassert(actual(xs) == 1856);
    wassert(actual(ys) == 1856);
    wassert(actual(rx).almost_equal(METEOSAT_PIXELSIZE_X, 3));
    wassert(actual(ry).almost_equal(METEOSAT_PIXELSIZE_Y, 3));

    GDALRasterBand* b = dataset->GetRasterBand(1);

    int valid;
    wassert(actual(b->GetOffset(&valid)) == 0);
    wassert(actual(valid) == TRUE);
    wassert(actual(b->GetScale(&valid)) == 1);
    wassert(actual(valid) == TRUE);
    double missing = b->GetNoDataValue(&valid);
    wassert(actual(valid) == TRUE);
    wassert(actual(missing) == 0.0);
}

void Fixture::check_cropped_image_data(GDALDataset* dataset)
{
    check_general_image_data(dataset);

    wassert(actual(dataset->GetRasterXSize()) == 150);
    wassert(actual(dataset->GetRasterYSize()) == 300);

    int xs, ys;
    double rx, ry;
    msat::dataset::decodeGeotransform(dataset, xs, ys, rx, ry);
    wassert(actual(xs) == 1856 - 2540);
    wassert(actual(ys) == 1856 - 2950);
    wassert(actual(rx).almost_equal(METEOSAT_PIXELSIZE_X, 4));
    wassert(actual(ry).almost_equal(METEOSAT_PIXELSIZE_Y, 4));

    GDALRasterBand* b = dataset->GetRasterBand(1);

    int valid;
    wassert(actual(b->GetOffset(&valid)) == 0);
    wassert(actual(valid) == TRUE);
    wassert(actual(b->GetScale(&valid)) == 1);
    wassert(actual(valid) == TRUE);
    //double missing = b->GetNoDataValue(&valid);
    //gen_ensure_equals(missing, 255);
    //wassert(actual(valid) == TRUE);
}

void Tests::register_tests()
{
    ImportTest::register_tests();

    this->add_method("datatype", [](Fixture& f) {
        wassert(actual(f.dataset()->GetRasterCount()) == 1);
        GDALRasterBand* b = f.dataset()->GetRasterBand(1);
        wassert(actual(b->GetRasterDataType()) == GDT_Float32);
        wassert(actual(b->GetOffset()) == 0);
        wassert(actual(b->GetScale()) == 1);
    });
}

}
