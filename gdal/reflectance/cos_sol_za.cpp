#include "cos_sol_za.h"
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

class CosSolZARasterBand : public ProxyRasterBand
{
public:
    // Utility class that converts pixel coordinates to lat,lon
    PixelToLatlon* p2ll = nullptr;

    // Julian day
    int jday;
    // Time of day in fractional hours
    double daytime;


    CosSolZARasterBand(CosSolZADataset* ds, int idx, GDALRasterBand* prototype)
    {
        poDS = ds;
        nBand = idx;
        eDataType = GDT_Float64;

        add_info(prototype, "CosSolZARasterBand");

        // Day time
        int ye, mo, da, ho, mi, se;
        if (sscanf(ds->datetime.c_str(), "%04d-%02d-%02d %02d:%02d:%02d",
                &ye, &mo, &da, &ho, &mi, &se) != 6)
            throw std::runtime_error("cannot parse file time");
        jday = msat::facts::jday(ye, mo, da);
        daytime = (double)ho + ((double)mi) / 60.0;

        p2ll = new PixelToLatlon(ds);
    }
    ~CosSolZARasterBand()
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
            dest[i] = facts::cos_sol_za(jday, daytime, lats[i], lons[i]);
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



CosSolZADataset::CosSolZADataset(GDALDataset* prototype)
{
    add_info(prototype, "CosSolZADataset");

    SetBand(1, new CosSolZARasterBand(this, 1, prototype->GetRasterBand(1)));
}

}
}
