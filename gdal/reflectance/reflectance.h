#ifndef MSAT_GDALDRIVER_REFLECTANCE_H
#define MSAT_GDALDRIVER_REFLECTANCE_H

#include "base.h"
#include <memory>
#include <set>
#include <array>

namespace msat {
namespace utils {

class ReflectanceDataset : public ProxyDataset
{
public:
    /// Channel for which we compute reflectance
    int channel_id;

    /**
     * Datasets that are been entrusted to this one for memory management, and
     * need to be deallocated on ReflectanceDataset destruction.
     */
    std::set<GDALDataset*> owned_datasets;

    /**
     * Source GDALRasterBands for each channel (defaults to nullptr when unset).
     */
    std::array<GDALRasterBand*, 12> sources{};

    ReflectanceDataset(int channel_id);
    ~ReflectanceDataset();

    /**
     * Add channels from ds as sources for reflectance computation.
     *
     * If take_ownership is true, then ownership of ds is passed to
     * ReflectanceDataset, and ds will be deallocated on ReflectanceDataset
     * destruction.
     *
     * If take_ownership is false, then ds is assumed to be managed by the
     * caller and kept alive for the whole lifetime of ReflectanceDataset.
     */
    void add_source(GDALDataset* ds, bool take_ownership=false);

    /**
     * Call after all needed add_source() calls have been made, to create the
     * reflectance GDALRasterBand for this dataset.
     */
    void init_rasterband();
};

struct PixelToLatlon;

class ReflectanceRasterBand : public ProxyRasterBand
{
public:
    // Utility class that converts pixel coordinates to lat,lon
    PixelToLatlon* p2ll = nullptr;

    // Julian day
    int jday;
    // Time of day in fractional hours
    double daytime;

    ReflectanceRasterBand(ReflectanceDataset* ds, int idx);
    ~ReflectanceRasterBand();

    const char* GetUnitType() override;
    double GetOffset(int* pbSuccess=NULL) override;
    double GetScale(int* pbSuccess=NULL) override;
    double GetNoDataValue(int* pbSuccess=NULL) override;
};

class SingleChannelReflectanceRasterBand : public ReflectanceRasterBand
{
public:
    /// Source GDALRasterBand
    GDALRasterBand* source_rb = nullptr;

    /// tr factor from MSG_data_RadiometricProc.cpp radiance_to_reflectance
    double tr;

    /// Cached slope of the source raster band
    double rad_slope;
    /// Cached offset of the source raster band
    double rad_offset;

    SingleChannelReflectanceRasterBand(ReflectanceDataset* ds, int idx);
    ~SingleChannelReflectanceRasterBand();

    CPLErr IReadBlock(int xblock, int yblock, void *buf) override;
};

class Reflectance39RasterBand : public ReflectanceRasterBand
{
protected:
    GDALRasterBand* source_ir039 = nullptr;
    GDALRasterBand* source_ir108 = nullptr;
    GDALRasterBand* source_ir134 = nullptr;

    // Cached slope and offsets for the three source bands

    double ir039_slope;
    double ir039_offset;
    double ir108_slope;
    double ir108_offset;
    double ir134_slope;
    double ir134_offset;

public:
    Reflectance39RasterBand(ReflectanceDataset* ds, int idx);
    ~Reflectance39RasterBand();

    CPLErr IReadBlock(int xblock, int yblock, void *buf) override;
};

}
}
#endif
