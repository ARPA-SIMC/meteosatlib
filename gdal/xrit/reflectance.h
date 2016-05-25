#ifndef MSAT_GDALDRIVER_XRIT_REFLECTANCE_H
#define MSAT_GDALDRIVER_XRIT_REFLECTANCE_H

#include <msat/xrit/fileaccess.h>
#include <msat/xrit/dataaccess.h>
#include <gdal/gdal_priv.h>
#include "rasterband.h"
#include <memory>

namespace msat {
namespace xrit {

struct PixelToLatlon;

class BaseReflectanceRasterBand : public XRITRasterBand
{
public:
    // Utility class that converts pixel coordinates to lat,lon
    PixelToLatlon* p2ll;

    // Julian day
    int jday;
    // Time of day in fractional hours
    double daytime;

    double rad_slope;
    double rad_offset;

    BaseReflectanceRasterBand(XRITDataset* ds, int idx);
    ~BaseReflectanceRasterBand();

    bool init(MSG_data& PRO_data, MSG_data& EPI_data, MSG_header& header);

    virtual const char* GetUnitType();
    virtual double GetOffset(int* pbSuccess=NULL);
    virtual double GetScale(int* pbSuccess=NULL);
    virtual double GetNoDataValue(int* pbSuccess=NULL);
};

/// Rasterband returning cosine of satellite zenith angle
class SZARasterBand : public BaseReflectanceRasterBand
{
public:
    // tr factor from MSG_data_RadiometricProc.cpp radiance_to_reflectance
    double tr;

    SZARasterBand(XRITDataset* ds, int idx);
    ~SZARasterBand();

    bool init(MSG_data& PRO_data, MSG_data& EPI_data, MSG_header& header);

    virtual CPLErr IReadBlock(int xblock, int yblock, void *buf);

    virtual const char* GetUnitType();
};

class ReflectanceRasterBand : public BaseReflectanceRasterBand
{
public:
    // tr factor from MSG_data_RadiometricProc.cpp radiance_to_reflectance
    double tr;

    ReflectanceRasterBand(XRITDataset* ds, int idx);
    ~ReflectanceRasterBand();

    bool init(MSG_data& PRO_data, MSG_data& EPI_data, MSG_header& header);

    virtual CPLErr IReadBlock(int xblock, int yblock, void *buf);
};

class Reflectance39RasterBand : public BaseReflectanceRasterBand
{
protected:
    xrit::FileAccess fa_ir108;
    xrit::FileAccess fa_ir134;
    xrit::DataAccess da_ir108;
    xrit::DataAccess da_ir134;
    float* cal108;
    float* cal134;

public:
    Reflectance39RasterBand(XRITDataset* ds, int idx);
    ~Reflectance39RasterBand();

    bool init(MSG_data& PRO_data, MSG_data& EPI_data, MSG_header& header);

    virtual CPLErr IReadBlock(int xblock, int yblock, void *buf);
};

}
}

// vim:set sw=4:
#endif
