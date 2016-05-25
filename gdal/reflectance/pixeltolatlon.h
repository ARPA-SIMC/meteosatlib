#ifndef MSAT_GDALDRIVER_REFLECTANCE_PIXELTOLATLON_H
#define MSAT_GDALDRIVER_REFLECTANCE_PIXELTOLATLON_H

#include <gdal/gdal_priv.h>
#include <ogr_spatialref.h>

namespace msat {
namespace utils {

struct PixelToLatlon
{
    double geoTransform[6];
    OGRSpatialReference* proj = nullptr;
    OGRSpatialReference* latlon = nullptr;
    OGRCoordinateTransformation* toLatLon = nullptr;

    PixelToLatlon(GDALDataset* ds);
    ~PixelToLatlon();

    void compute(int x, int y, int sx, int sy, double* lats, double* lons);
};

}
}
#endif
