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
public:
    xrit::FileAccess fa;
    xrit::DataAccess da;
    int spacecraft_id;
    std::string projWKT;
    double geotransform[6];

    XRITDataset(const xrit::FileAccess& fa);

    virtual bool init();

#if GDAL_VERSION_MAJOR < 3
    virtual const char* GetProjectionRef() override;
#else
    const char* _GetProjectionRef() override;
    const OGRSpatialReference* GetSpatialRef() const override;
#endif
    virtual CPLErr GetGeoTransform(double* tr);

};

}
}

#endif
