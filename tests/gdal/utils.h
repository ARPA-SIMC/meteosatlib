#ifndef TEST_GDAL_UTILS_H
#define TEST_GDAL_UTILS_H

#include <msat/gdal/const.h>
#include <msat/gdal/dataset.h>
#include <msat/utils/tests.h>
#include <memory>

namespace msat {
namespace tests {

namespace gdal {

bool has_driver(const std::string& name);

std::unique_ptr<GDALDataset> open_ro(const std::string& name);

static inline int32_t read_int32(GDALRasterBand* rb, int x, int y)
{
    int32_t res;
    rb->RasterIO(GF_Read, x, y, 1, 1, &res, 1, 1, GDT_Int32, 0, 0);
    return res;
}

static inline float read_float32(GDALRasterBand* rb, int x, int y)
{
    float res;
    rb->RasterIO(GF_Read, x, y, 1, 1, &res, 1, 1, GDT_Float32, 0, 0);
    return res;
}

std::unique_ptr<GDALDataset> recode(GDALDataset* ds, bool leaveFile,
    const char* driver,
    const std::string& opt1 = std::string(),
    const std::string& opt2 = std::string(),
    const std::string& opt3 = std::string());

std::unique_ptr<GDALDataset> recode(GDALDataset* ds, const msat::proj::ImageBox& cropArea, bool leaveFile,
    const char* driver,
    const std::string& opt1 = std::string(),
    const std::string& opt2 = std::string(),
    const std::string& opt3 = std::string());

}

// RAII-style class to ensure cleanup of a temporary test file
class TempTestFile
{
    std::string pathname;
    bool leave;

public:
    TempTestFile(bool leave = false);
    TempTestFile(const std::string& pathname, bool leave = false) : pathname(pathname), leave(leave) { unlink(pathname.c_str()); }
    ~TempTestFile() { if (!leave) unlink(pathname.c_str()); }

    const std::string& name() const { return pathname; }
};

class GeoReferencer
{
protected:
    GDALDataset* ds;
    std::string projection;
    double geoTransform[6];
    double invGeoTransform[6];
    OGRSpatialReference* proj;
    OGRSpatialReference* latlon;
    OGRCoordinateTransformation* toLatLon;
    OGRCoordinateTransformation* fromLatLon;

public:
    GeoReferencer(GDALDataset* ds);
    ~GeoReferencer();

    void pixelToProjected(int x, int y, double& px, double& py) const;
    void projectedToPixel(double px, double py, int& x, int& y) const;

    void projectedToLatlon(double px, double py, double& lat, double& lon);
    void latlonToProjected(double lat, double lon, double& px, double& py);

    void pixelToLatlon(int x, int y, double& lat, double& lon)
    {
        double px, py;
        pixelToProjected(x, y, px, py);
        projectedToLatlon(px, py, lat, lon);
    }
    void latlonToPixel(double lat, double lon, int& x, int& y)
    {
        double px, py;
        latlonToProjected(lat, lon, px, py);
        projectedToPixel(px, py, x, y);
    }
};

struct GDALFixture : public Fixture
{
    std::string driver;
    std::string fname;
    GDALDataset* _dataset = nullptr;

    GDALFixture(const char* driver, const char* fname)
        : driver(driver), fname(fname)
    {
    }
    virtual ~GDALFixture();

    void test_setup();

    GDALDataset* dataset();

    virtual msat::proj::ImageBox get_crop_area() { throw std::runtime_error("get_crop_area called on a test fixture where it has not been implemented"); }
    virtual void check_full_image_data(GDALDataset* dataset) {}
    virtual void check_cropped_image_data(GDALDataset* dataset) {}
};

template<typename Fixture>
struct ImportTest : public FixtureTestCase<Fixture>
{
    using FixtureTestCase<Fixture>::FixtureTestCase;

    void register_tests() override
    {
        // Test that the image is read with the right driver
        this->add_method("driver", [](Fixture& f) {
            wassert(actual(f.dataset() != 0).istrue());
            wassert(actual(GDALGetDriverShortName(f.dataset()->GetDriver())) == f.driver);
            wassert(actual(f.dataset()->GetRasterCount()) >= 1);
        });

        // Test read on full image
        this->add_method("open_plain", [](Fixture& f) {
            wassert(f.check_full_image_data(f.dataset()));
        });

        // Try CreateCopy with GRIB msat/msat
        this->add_method("recode_grib_msat", [](Fixture& f) {
            std::unique_ptr<GDALDataset> recoded = wcallchecked(gdal::recode(f.dataset(), false, "MsatGRIB", "TEMPLATE=msat/msat"));
            wassert(f.check_full_image_data(recoded.get()));
        });

        // Try CreateCopy with GRIB msat/ecmwf
        this->add_method("recode_grib_ecmwf", [](Fixture& f) {
            std::unique_ptr<GDALDataset> recoded = wcallchecked(gdal::recode(f.dataset(), false, "MsatGRIB", "TEMPLATE=msat/ecmwf"));
            wassert(f.check_full_image_data(recoded.get()));
        });

        // Try CreateCopy with NetCDF
        this->add_method("recode_grib_netcdf", [](Fixture& f) {
            std::unique_ptr<GDALDataset> recoded = wcallchecked(gdal::recode(f.dataset(), false, "MsatNetCDF"));
            wassert(f.check_full_image_data(recoded.get()));
        });

        // Try CreateCopy with NetCDF24
        this->add_method("recode_grib_netcdf24", [](Fixture& f) {
            std::unique_ptr<GDALDataset> recoded = wcallchecked(gdal::recode(f.dataset(), false, "MsatNetCDF24"));
            wassert(f.check_full_image_data(recoded.get()));
        });

        // Try reimporting a subarea exported to GRIB with the msat/msat template
        this->add_method("crop_recode_grib_msat", [](Fixture& f) {
            std::unique_ptr<GDALDataset> recoded = wcallchecked(gdal::recode(f.dataset(), f.get_crop_area(), false, "MsatGRIB", "TEMPLATE=msat/msat"));
            wassert(f.check_cropped_image_data(recoded.get()));
        });

        // Try reimporting a subarea exported to GRIB with the msat/ecmwf template
        this->add_method("crop_recode_grib_ecmwf", [](Fixture& f) {
            std::unique_ptr<GDALDataset> recoded = wcallchecked(gdal::recode(f.dataset(), f.get_crop_area(), false, "MsatGRIB", "TEMPLATE=msat/ecmwf"));
            wassert(f.check_cropped_image_data(recoded.get()));
        });

        // Try reimporting a subarea exported to NetCDF
        this->add_method("crop_recode_grib_netcdf", [](Fixture& f) {
            std::unique_ptr<GDALDataset> recoded = wcallchecked(gdal::recode(f.dataset(), f.get_crop_area(), false, "MsatNetCDF"));
            wassert(f.check_cropped_image_data(recoded.get()));
        });

        // Try reimporting a subarea exported to NetCDF24
        this->add_method("crop_recode_grib_netcdf24", [](Fixture& f) {
            std::unique_ptr<GDALDataset> recoded = wcallchecked(gdal::recode(f.dataset(), f.get_crop_area(), false, "MsatNetCDF24"));
            wassert(f.check_cropped_image_data(recoded.get()));
        });
    }
};

}
}

#endif
