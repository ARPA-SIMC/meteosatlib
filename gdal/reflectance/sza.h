#ifndef MSAT_GDALDRIVER_REFLECTANCE_SZA_H
#define MSAT_GDALDRIVER_REFLECTANCE_SZA_H

#include <gdal/reflectance/base.h>
#include <memory>
#include <set>

namespace msat {
namespace utils {

class SZADataset : public ProxyDataset
{
public:
    SZADataset(GDALDataset* prototype);
};

}
}
#endif
