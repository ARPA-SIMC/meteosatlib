#include "utils.h"
#include "msat/facts.h"

using namespace std;
using namespace msat::tests;

namespace {

#define TESTFILE "H:MSG2:VIS006:200807150900"
#define PERFORM_SLOW_TESTS
#define native_offset -1.02691
#define native_scale 0.0201355

struct Fixture : public GDALFixture
{
    using GDALFixture::GDALFixture;

    msat::proj::ImageBox get_crop_area()
    {
        return msat::proj::ImageBox(msat::proj::ImagePoint(2700, 3200), msat::proj::ImagePoint(2700+150, 3200+300));
    }

    void check_general_image_data(GDALDataset* dataset);
    void check_full_image_data(GDALDataset* dataset);
    void check_cropped_image_data(GDALDataset* dataset);
};


class Tests : public ImportTest<Fixture>
{
    using ImportTest::ImportTest;

    void register_tests() override;
} test("gdal_import_xrit", "MsatXRIT", TESTFILE);

void Fixture::check_general_image_data(GDALDataset* dataset)
{
    const char* val = dataset->GetMetadataItem(MD_MSAT_DATETIME, MD_DOMAIN_MSAT);
    wassert(actual(val) == "2008-07-15 09:00:00");

    val = dataset->GetMetadataItem(MD_MSAT_SPACECRAFT_ID, MD_DOMAIN_MSAT);
    wassert(actual(val) == "56");
    val = dataset->GetMetadataItem(MD_MSAT_SPACECRAFT, MD_DOMAIN_MSAT);
    wassert(actual(val) == "MSG2");

    // TODO gen_ensure_equals(defaultFilename(*dataset), "MSG2_Seviri_1_5_VIS006_20080715_0900");
    // TODO gen_ensure_equals(defaultShortName(*dataset), "VIS006");

    GDALRasterBand* b = dataset->GetRasterBand(1);

    // TODO gen_ensure_equals(defaultFilename(*b), "MSG2_Seviri_1_5_VIS006_20080715_0900");
    // TODO gen_ensure_equals(defaultShortName(*dataset), "VIS006");

    val = b->GetMetadataItem(MD_MSAT_CHANNEL_ID, MD_DOMAIN_MSAT);
    wassert(actual(val) == "1");
    val = b->GetMetadataItem(MD_MSAT_CHANNEL, MD_DOMAIN_MSAT);
    wassert(actual(val) == "VIS006");

    wassert(actual(b->GetUnitType()) == "mW m^-2 sr^-1 (cm^-1)^-1");

    if (b->GetRasterDataType() == GDT_Float64)
    {
        int valid;
        wassert(actual(b->GetOffset(&valid)) == 0);
        wassert(actual(valid) == TRUE);
        wassert(actual(b->GetScale(&valid)) == 1);
        wassert(actual(valid) == TRUE);
    } else {
        int valid;
        wassert(actual(b->GetOffset(&valid)).almost_equal(native_offset, 4));
        wassert(actual(valid) == TRUE);
        wassert(actual(b->GetScale(&valid)).almost_equal(native_scale, 7));
        wassert(actual(valid) == TRUE);
    }
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

    if (b->GetRasterDataType() == GDT_Float64)
    {
        // Prescaled
        int valid;
        double missing = b->GetNoDataValue(&valid);
        wassert(actual(valid) == TRUE);
        wassert(actual(missing).almost_equal(-1.02691, 4));
        wassert(actual((double)gdal::read_float32(b,    0,    0)).almost_equal(missing, 4));
        wassert(actual((double)gdal::read_float32(b,   10,   10)).almost_equal(missing, 4));

        wassert(actual((double)gdal::read_float32(b, 1800, 3100)).almost_equal(missing, 4));  // Missing segment
        wassert(actual((double)gdal::read_float32(b,  500, 3500)).almost_equal(missing, 4));  // Missing value on existing segment
#define SC(x) ((x)*native_scale + native_offset)
        wassert(actual((double)gdal::read_float32(b,  900, 3300)).almost_equal(SC(51), 3));       // Dark surface
        wassert(actual((double)gdal::read_float32(b, 2050, 3345)).almost_equal(SC(79), 3));       // Dark spot
        wassert(actual((double)gdal::read_float32(b, 2500, 3350)).almost_equal(SC(237), 3));      // Bright spot
#undef SC
    } else {
        // Packed
        int valid;
        int32_t missing = b->GetNoDataValue(&valid);
        wassert(actual(valid) == TRUE);
        wassert(actual(missing) == 0);
        wassert(actual(gdal::read_int32(b, 0, 0)) == missing);
        wassert(actual(gdal::read_int32(b, 10, 10)) == missing);

        wassert(actual(gdal::read_int32(b, 1800, 3100)) == missing);  // Missing segment
        wassert(actual(gdal::read_int32(b,  500, 3500)) == missing);  // Missing value on existing segment
        wassert(actual(gdal::read_int32(b,  900, 3300)) == 51);       // Dark surface
        wassert(actual(gdal::read_int32(b, 2050, 3345)) == 79);       // Dark spot
        wassert(actual(gdal::read_int32(b, 2500, 3350)) == 237);      // Bright spot
    }

    /*
   gen_ensure_similar(img.data->scaled(2540, 2950), 6.07987, 0.001); // unverified
   gen_ensure_similar(img.data->scaled(2550, 2970), 6.27186, 0.001); // unverified
   gen_ensure_similar(img.data->scaled(2560, 2990), 4.73590, 0.001); // unverified
   gen_ensure_similar(img.data->scaled(2570, 3010), img.data->missingValue, 0.001); // unverified
   gen_ensure_similar(img.data->scaled(2580, 3030), img.data->missingValue, 0.001); // unverified
   gen_ensure_similar(img.data->scaled(2590, 3050), img.data->missingValue, 0.001); // unverified
   gen_ensure_similar(img.data->scaled(2600, 3070), 5.79187, 0.001); // unverified
   gen_ensure_similar(img.data->scaled(2610, 3090), 5.05589, 0.001); // unverified
   gen_ensure_similar(img.data->scaled(2620, 3110), 4.57590, 0.001); // unverified
   gen_ensure_similar(img.data->scaled(2630, 3130), 4.86389, 0.001); // unverified
   gen_ensure_similar(img.data->scaled(2640, 3150), 5.27988, 0.001); // unverified
   */
}

void Fixture::check_cropped_image_data(GDALDataset* dataset)
{
    check_general_image_data(dataset);

    wassert(actual(dataset->GetRasterXSize()) == 150);
    wassert(actual(dataset->GetRasterYSize()) == 300);

    int xs, ys;
    double rx, ry;
    msat::dataset::decodeGeotransform(dataset, xs, ys, rx, ry);
    wassert(actual(xs) == 1856 - 2700);
    wassert(actual(ys) == 1856 - 3200);
    wassert(actual(rx).almost_equal(METEOSAT_PIXELSIZE_X, 4));
    wassert(actual(ry).almost_equal(METEOSAT_PIXELSIZE_Y, 4));

    GDALRasterBand* b = dataset->GetRasterBand(1);

    if (b->GetRasterDataType() == GDT_Float64)
    {
        // Prescaled
        int valid;
        double missing = b->GetNoDataValue(&valid);
        wassert(actual(valid) == TRUE);
        wassert(actual(missing).almost_equal(-1.02691, 4));

#define SC(x) ((x)*native_scale + native_offset)
        wassert(actual((double)gdal::read_float32(b,   0,   0)).almost_equal(missing, 3)); // unverified
        wassert(actual((double)gdal::read_float32(b,  10,  20)).almost_equal(missing, 3)); // unverified
        wassert(actual((double)gdal::read_float32(b,  20,  40)).almost_equal(missing, 3)); // unverified
        wassert(actual((double)gdal::read_float32(b,  30,  60)).almost_equal(SC(217), 3)); // unverified
        wassert(actual((double)gdal::read_float32(b,  40,  80)).almost_equal(SC(196), 3)); // unverified
        wassert(actual((double)gdal::read_float32(b,  50, 100)).almost_equal(SC(127), 3)); // unverified
        wassert(actual((double)gdal::read_float32(b,  60, 120)).almost_equal(SC(171), 3)); // unverified
        wassert(actual((double)gdal::read_float32(b,  70, 140)).almost_equal(SC(209), 3)); // unverified
        wassert(actual((double)gdal::read_float32(b,  80, 160)).almost_equal(SC(223), 3)); // unverified
        wassert(actual((double)gdal::read_float32(b,  90, 180)).almost_equal(SC(206), 3)); // unverified
        wassert(actual((double)gdal::read_float32(b, 100, 200)).almost_equal(missing, 3)); // unverified
#undef SC
    } else {
        // Packed
        int valid;
        double missing = b->GetNoDataValue(&valid);
        wassert(actual(missing) == 0);
        wassert(actual(valid) == TRUE);

        wassert(actual(gdal::read_int32(b,   0,   0)) == missing); // unverified
        wassert(actual(gdal::read_int32(b,  10,  20)) == missing); // unverified
        wassert(actual(gdal::read_int32(b,  20,  40)) == missing); // unverified
        wassert(actual(gdal::read_int32(b,  30,  60)) == 217); // unverified
        wassert(actual(gdal::read_int32(b,  40,  80)) == 196); // unverified
        wassert(actual(gdal::read_int32(b,  50, 100)) == 127); // unverified
        wassert(actual(gdal::read_int32(b,  60, 120)) == 171); // unverified
        wassert(actual(gdal::read_int32(b,  70, 140)) == 209); // unverified
        wassert(actual(gdal::read_int32(b,  80, 160)) == 223); // unverified
        wassert(actual(gdal::read_int32(b,  90, 180)) == 206); // unverified
        wassert(actual(gdal::read_int32(b, 100, 200)) == missing); // unverified
    }
    /*
   gen_ensure_similar(b->scaled(  0,   0), 6.07987, 0.001); // unverified
   gen_ensure_similar(b->scaled( 10,  20), 6.27186, 0.001); // unverified
   gen_ensure_similar(b->scaled( 20,  40), 4.73590, 0.001); // unverified
   gen_ensure_similar(b->scaled( 30,  60), b->missingValue, 0.001); // unverified
   gen_ensure_similar(b->scaled( 40,  80), b->missingValue, 0.001); // unverified
   gen_ensure_similar(b->scaled( 50, 100), b->missingValue, 0.001); // unverified
   gen_ensure_similar(b->scaled( 60, 120), 5.79187, 0.001); // unverified
   gen_ensure_similar(b->scaled( 70, 140), 5.05589, 0.001); // unverified
   gen_ensure_similar(b->scaled( 80, 160), 4.57590, 0.001); // unverified
   gen_ensure_similar(b->scaled( 90, 180), 4.86389, 0.01); // unverified
   gen_ensure_similar(b->scaled(100, 200), 5.27988, 0.01); // unverified
   */
}

void Tests::register_tests()
{
    ImportTest::register_tests();

    this->add_method("datatype", [](Fixture& f) {
        wassert(actual(f.dataset()->GetRasterCount()) == 1);
        GDALRasterBand* b = f.dataset()->GetRasterBand(1);
        wassert(actual(b->GetRasterDataType()) == GDT_UInt16);
        wassert(actual(b->GetOffset()).almost_equal(-1.02691, 5));
        wassert(actual(b->GetScale()).almost_equal(0.0201355, 5));
    });
}

}
