#include "reflectance.h"
#include <msat/gdal/const.h>
#include <string>
#include <stdexcept>

using namespace std;

namespace msat {
namespace utils {

void ProxyDataset::add_info(GDALDataset* ds, const std::string& dsname)
{
    const char* proj = ds->GetProjectionRef();
    if (proj == nullptr)
        throw std::runtime_error(dsname + ": trying to add source without a projection definition");

    double gt[6];
    if (ds->GetGeoTransform(gt) == CE_Failure)
        throw std::runtime_error(dsname + ": trying to add source without affine geotransform coefficients");

    const char* mdtime = ds->GetMetadataItem(MD_MSAT_DATETIME, MD_DOMAIN_MSAT);
    if (mdtime == nullptr)
        throw std::runtime_error(dsname + ": trying to add source without " MD_DOMAIN_MSAT "/" MD_MSAT_DATETIME " metadata");

    if (!has_sources)
    {
        projWKT = proj;
        memcpy(geotransform, gt, 6 * sizeof(double));
        char** metadata = ds->GetMetadata(MD_DOMAIN_MSAT);
        if (metadata == nullptr)
            throw std::runtime_error(dsname + ": trying to add source without " MD_DOMAIN_MSAT " metadata");
        if (SetMetadata(metadata, MD_DOMAIN_MSAT) == CE_Failure)
            throw std::runtime_error(dsname + ": cannot set metadata from source dataset");
        datetime = mdtime;

        nRasterXSize = ds->GetRasterXSize();
        nRasterYSize = ds->GetRasterYSize();
    } else {
        if (projWKT != proj)
            throw std::runtime_error(dsname + ": inconsistent projection definitions in source datasets");
        if (memcmp(geotransform, gt, 6 * sizeof(double)) != 0)
            throw std::runtime_error(dsname + ": inconsistent affine geotransform coefficients in source datasets");
        if (datetime != mdtime)
            throw std::runtime_error(dsname + ": inconsistent datetime in source datasets");
        if (nRasterXSize != ds->GetRasterXSize())
            throw std::runtime_error(dsname + ": inconsistent raster X size in source datasets");
        if (nRasterYSize != ds->GetRasterYSize())
            throw std::runtime_error(dsname + ": inconsistent raster Y size in source datasets");
    }

    has_sources = true;
}

#if GDAL_VERSION_MAJOR < 3
const char* ProxyDataset::GetProjectionRef()
#else
const char* ProxyDataset::GetProjectionRef() const
#endif
{
    return projWKT.c_str();
}

CPLErr ProxyDataset::GetGeoTransform(double* tr)
{
    memcpy(tr, geotransform, 6 * sizeof(double));
    return CE_None;
}

void ProxyRasterBand::add_info(GDALRasterBand* rb, const std::string& rbname)
{
    rb->GetBlockSize(&nBlockXSize, &nBlockYSize);

    // Initialize metadata from source raster band
    char** metadata = rb->GetMetadata(MD_DOMAIN_MSAT);
    if (metadata == nullptr)
        throw std::runtime_error(rbname + ": trying to use a source GDALRasterBand without " MD_DOMAIN_MSAT " metadata");
    if (SetMetadata(metadata, MD_DOMAIN_MSAT) == CE_Failure)
        throw std::runtime_error(rbname + ": cannot set metadata from source raster band");
}

}
}
