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
    typedef enum {
        PP_NONE,
        PP_REFLECTANCE,
        PP_SZA,
    } Effect;
    xrit::FileAccess fa;
    xrit::DataAccess da;
    int spacecraft_id;
    std::string projWKT;
    double geotransform[6];
    Effect effect;

    XRITDataset(const xrit::FileAccess& fa, Effect effect=PP_NONE);

    virtual bool init();

    virtual const char* GetProjectionRef();
    virtual CPLErr GetGeoTransform(double* tr);

};

}
}

#endif
