#include "sat_za.h"
#include "pixeltolatlon.h"
#include <msat/gdal/const.h>
#include <msat/facts.h>
#include <ogr_spatialref.h>
#include <msat/hrit/MSG_channel.h>
#include <string>
#include <stdexcept>
#include <stdint.h>

using namespace std;

namespace msat {
namespace utils {

class SatZARasterBand : public ProxyRasterBand
{
public:
    // Utility class that converts pixel coordinates to lat,lon
    PixelToLatlon* p2ll = nullptr;

    SatZARasterBand(SatZADataset* ds, int idx, GDALRasterBand* prototype)
    {
        poDS = ds;
        nBand = idx;
        eDataType = GDT_Float64;

        add_info(prototype, "SatZARasterBand");

        p2ll = new PixelToLatlon(ds);
    }
    ~SatZARasterBand()
    {
        delete p2ll;
    }

    const char* GetUnitType() override { return ""; }

    CPLErr IReadBlock(int xblock, int yblock, void *buf) override
    {
        // Precompute pixel georeferentiation
        std::vector<double> lats(nBlockXSize * nBlockYSize);
        std::vector<double> lons(nBlockXSize * nBlockYSize);
        p2ll->compute(xblock * nBlockXSize, yblock * nBlockYSize, nBlockXSize, nBlockYSize, lats.data(), lons.data());

        // Compute satellite zenith angles
        double* dest = (double*) buf;
        for (int i = 0; i < nBlockXSize * nBlockYSize; ++i)
        {
            // Just the solar zenith angle
            dest[i] = facts::sat_za(lats[i], lons[i]);
            // Normalise outliars
            switch (fpclassify(dest[i]))
            {
                case FP_NAN:
                case FP_SUBNORMAL:
                case FP_ZERO: dest[i] = 0.0; break;
                case FP_INFINITE:
                case FP_NORMAL:
                    if (dest[i] < 0.0) dest[i] = 0.0;
                    if (dest[i] > 1.0) dest[i] = 1.0;
                    break;
            }
        }

        return CE_None;
    }
};



SatZADataset::SatZADataset(GDALDataset* prototype)
{
    add_info(prototype, "SatZADataset");

    SetBand(1, new SatZARasterBand(this, 1, prototype->GetRasterBand(1)));
}

}
}
