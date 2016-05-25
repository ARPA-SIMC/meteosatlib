#ifndef MSAT_GDALDRIVER_GDAL_UTILS_H
#define MSAT_GDALDRIVER_GDAL_UTILS_H

#include <gdal/gdal_priv.h>
#include <string>

namespace msat {
namespace gdal {

/**
 * If GDALOpenInfo contains flags that require a change of behaviour for the
 * dataset, like computing the reflectance of the channel, wrap src into the
 * relevant feature implementation and return the resulting dataset.
 *
 * The wrapper dataset will take care of memory managing src.
 *
 * If info does not ask for any special behaviour, just return src.
 */
GDALDataset* add_extras(GDALDataset* src, GDALOpenInfo* info);

}
}
#endif
