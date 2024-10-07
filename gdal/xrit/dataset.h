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
    double geotransform[6];
    OGRSpatialReference osr;

    XRITDataset(const xrit::FileAccess& fa);

    virtual bool init();

    const OGRSpatialReference* GetSpatialRef() const override;
    virtual CPLErr GetGeoTransform(double* tr);

};

}
}

#endif
