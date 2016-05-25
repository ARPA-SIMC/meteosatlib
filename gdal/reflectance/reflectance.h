#ifndef MSAT_GDALDRIVER_REFLECTANCE_H
#define MSAT_GDALDRIVER_REFLECTANCE_H

#include <gdal/gdal_priv.h>
#include <memory>
#include <set>

namespace msat {
namespace utils {

class ReflectanceDataset : public GDALDataset
{
public:
    /// Channel for which we compute reflectance
    int channel_id;

    /// True when at least one source has been added
    bool has_sources = false;

    /// Projection WKT string returned by GetProjectionRef
    std::string projWKT;

    /// Affine geotransform returned by GetGeoTransform
    double geotransform[6];

    /// Datetime metadata string
    std::string datetime;

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

    const char* GetProjectionRef() override;
    CPLErr GetGeoTransform(double* tr) override;
};

struct PixelToLatlon;

class ReflectanceRasterBand : public GDALRasterBand
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

#if 0
/// Rasterband returning cosine of satellite zenith angle
class SZARasterBand : public ReflectanceRasterBand
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
#endif

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

#if 0
class Reflectance39RasterBand : public ReflectanceRasterBand
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
#endif

}
}
#endif
