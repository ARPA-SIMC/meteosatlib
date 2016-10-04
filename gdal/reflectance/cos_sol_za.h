#ifndef MSAT_GDALDRIVER_REFLECTANCE_COS_SOL_ZA_H
#define MSAT_GDALDRIVER_REFLECTANCE_COS_SOL_ZA_H

#include <gdal/reflectance/base.h>
#include <memory>
#include <set>

namespace msat {
namespace utils {

class CosSolZADataset : public ProxyDataset
{
public:
    CosSolZADataset(GDALDataset* prototype);
};

}
}
#endif
