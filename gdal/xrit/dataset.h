#ifndef MSAT_GDALDRIVER_XRIT_DATASET_H
#define MSAT_GDALDRIVER_XRIT_DATASET_H

#include <msat/xrit/fileaccess.h>
#include <msat/xrit/dataaccess.h>
#include <gdal/gdal_priv.h>
#include <string>

namespace msat {
namespace xrit {

class XRITDataset : public GDALDataset
{
protected:
    std::string projWKT;
public:
    xrit::FileAccess fa;
    xrit::DataAccess da;
    int spacecraft_id;
    double geotransform[6];
    OGRSpatialReference* osr = nullptr;

    XRITDataset(const xrit::FileAccess& fa);
    ~XRITDataset();

    virtual bool init();

#if GDAL_VERSION_MAJOR < 3
    virtual const char* GetProjectionRef() override;
#else
    const OGRSpatialReference* GetSpatialRef() const override;
#endif
    virtual CPLErr GetGeoTransform(double* tr);

};

}
}

#endif
