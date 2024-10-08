#include "pixeltolatlon.h"
#include <stdexcept>

using namespace std;

namespace msat {
namespace utils {

PixelToLatlon::PixelToLatlon(GDALDataset* ds)
{
    if (ds->GetGeoTransform(geoTransform) != CE_None)
        throw std::runtime_error("no geotransform found in input dataset");

    const OGRSpatialReference* osr = ds->GetSpatialRef();
    if (!osr)
        throw std::runtime_error("no projection name found in input dataset");

    proj = osr->Clone();
    latlon = proj->CloneGeogCS();
    latlon->SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
    toLatLon = OGRCreateCoordinateTransformation(proj, latlon);
#if GDAL_VERSION_MAJOR >= 3
    toLatLon->SetEmitErrors(false);
#endif
}

PixelToLatlon::~PixelToLatlon()
{
    delete proj;
    delete latlon;
    delete toLatLon;
}

void PixelToLatlon::compute(int x, int y, int sx, int sy, double* lats, double* lons)
{
    int idx = 0;

    // Pixels to projected coordinates
    for (int iy = y; iy < y + sy; ++iy)
    {
        for (int ix = x; ix < x + sx; ++ix)
        {
            // Projected y
            lats[idx] = geoTransform[3]
                + geoTransform[4] * ix
                + geoTransform[5] * iy;

            // Projected x
            lons[idx] = geoTransform[0]
                + geoTransform[1] * ix
                + geoTransform[2] * iy;

            ++idx;
        }
    }

    // Projected coordinates to latlon
    toLatLon->Transform(sx * sy, lons, lats);
    // Ignore errors, since there usually are points in space that fail to
    // transform

    // if (!toLatLon->Transform(sx * sy, lons, lats))
    // {
    //     throw std::runtime_error("points failed to transform to lat,lon");
    // }
}

}
}
