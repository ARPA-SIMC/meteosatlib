#ifndef MSAT_GDALDRIVER_XRIT_RASTERBAND_H
#define MSAT_GDALDRIVER_XRIT_RASTERBAND_H

#include <gdal/gdal_priv.h>
#include <msat/hrit/MSG_HRIT.h>

namespace msat {
namespace xrit {

class XRITDataset;

class XRITRasterBand : public GDALRasterBand
{
public:
    XRITDataset* xds;
    double slope;
    double offset;
    bool linear;
    int channel_id;
    float* calibration;

    XRITRasterBand(XRITDataset* ds, int idx);
    ~XRITRasterBand();

    bool init(MSG_data& PRO_data, MSG_data& EPI_data, MSG_header& header);

    const char* GetUnitType() override;

    CPLErr IReadBlock(int xblock, int yblock, void *buf) override;

    double GetOffset(int* pbSuccess=NULL) override;
    double GetScale(int* pbSuccess=NULL) override;
    double GetNoDataValue(int* pbSuccess=NULL) override;
};

}
}
#endif

