#ifndef MSAT_GDALDRIVER_REFLECTANCE_JDAY_H
#define MSAT_GDALDRIVER_REFLECTANCE_JDAY_H

#include <gdal/reflectance/base.h>
#include <memory>
#include <set>

namespace msat {
namespace utils {

class JDayDataset : public ProxyDataset
{
public:
    JDayDataset(GDALDataset* prototype);
};

}
}
#endif
