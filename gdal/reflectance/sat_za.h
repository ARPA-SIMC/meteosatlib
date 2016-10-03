#ifndef MSAT_GDALDRIVER_REFLECTANCE_SAT_ZA_H
#define MSAT_GDALDRIVER_REFLECTANCE_SAT_ZA_H

#include <gdal/reflectance/base.h>
#include <memory>
#include <set>

namespace msat {
namespace utils {

class SatZADataset : public ProxyDataset
{
public:
    SatZADataset(GDALDataset* prototype);
};

}
}
#endif
