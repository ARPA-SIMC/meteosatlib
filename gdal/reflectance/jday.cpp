#include "jday.h"
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

class JDayRasterBand : public ProxyRasterBand
{
public:
    // Julian day
    int16_t jday;

    JDayRasterBand(JDayDataset* ds, int idx, GDALRasterBand* prototype)
    {
        poDS = ds;
        nBand = idx;
        eDataType = GDT_Int16;

        add_info(prototype, "JDayRasterBand");

        // Day time
        int ye, mo, da, ho, mi, se;
        if (sscanf(ds->datetime.c_str(), "%04d-%02d-%02d %02d:%02d:%02d",
                &ye, &mo, &da, &ho, &mi, &se) != 6)
            throw std::runtime_error("cannot parse file time");
        jday = msat::facts::jday(ye, mo, da);
    }
    ~JDayRasterBand()
    {
    }

    const char* GetUnitType() override { return "day"; }

    CPLErr IReadBlock(int xblock, int yblock, void *buf) override
    {
        // Return the same julian day for every pixel
        int16_t* dest = (int16_t*) buf;
        for (int i = 0; i < nBlockXSize * nBlockYSize; ++i)
            dest[i] = jday;
        return CE_None;
    }
};



JDayDataset::JDayDataset(GDALDataset* prototype)
{
    add_info(prototype, "JDayDataset");

    SetBand(1, new JDayRasterBand(this, 1, prototype->GetRasterBand(1)));
}

}
}
