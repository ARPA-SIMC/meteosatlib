#ifndef MSAT_GDALDRIVER_REFLECTANCE_BASE_H
#define MSAT_GDALDRIVER_REFLECTANCE_BASE_H

#include <gdal/gdal_priv.h>
#include <memory>
#include <set>

namespace msat {
namespace utils {

class ProxyDataset : public GDALDataset
{
protected:
    std::string projection_ref;

public:
    /// True when at least one source has been added
    bool has_sources = false;

    /// Affine geotransform returned by GetGeoTransform
    double geotransform[6];

    /// Datetime metadata string
    std::string datetime;

    /**
     * Add information from the given dataset, raising an exception if they are
     * inconsistent with previously added ones.
     */
    void add_info(GDALDataset* ds, const std::string& dsname);

#if GDAL_VERSION_MAJOR < 3
    const char* GetProjectionRef() override;
#else
    const char* _GetProjectionRef() override;
    const OGRSpatialReference* GetSpatialRef() const override;
#endif
    CPLErr GetGeoTransform(double* tr) override;
};

class ProxyRasterBand : public GDALRasterBand
{
public:
    /// Add information from the given raster band
    void add_info(GDALRasterBand* rb, const std::string& rbname);
};

}
}
#endif
