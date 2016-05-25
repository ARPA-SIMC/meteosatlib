#include "xrit.h"
#include "dataset.h"
#include "rasterband.h"
#include "gdal/reflectance/reflectance.h"

#include <string>
#include <memory>
#include <cctype>

using namespace std;

namespace msat {
namespace xrit {

GDALDataset* XRITOpen(GDALOpenInfo* info)
{
    // Se if it looks like a XRIT filename
    if (!msat::xrit::isValid(info->pszFilename))
        return NULL;

    // Check for a special product suffix referring to a computed version of
    // the channel
    XRITDataset::Effect effect = XRITDataset::PP_NONE;
    FileAccess fa(info->pszFilename);
    if (!fa.productid2.empty())
    {
        switch (fa.productid2[fa.productid2.size() - 1])
        {
            case 'r': effect = XRITDataset::PP_REFLECTANCE; break;
            case 'a': effect = XRITDataset::PP_SZA; break;
        }

        // Remove the suffix
        if (effect != XRITDataset::PP_NONE)
            fa.productid2.resize(fa.productid2.size() - 1);
    }

    if (effect == XRITDataset::PP_REFLECTANCE)
    {
        if (fa.productid2 == "IR_039")
        {
            std::unique_ptr<XRITDataset> ds(new XRITDataset(fa, effect));
            if (!ds->init()) return NULL;
            return ds.release();
        } else {
            std::unique_ptr<XRITDataset> ds(new XRITDataset(fa));
            if (!ds->init()) return NULL;
            XRITRasterBand* rb = dynamic_cast<XRITRasterBand*>(ds->GetRasterBand(1));
            unique_ptr<msat::utils::ReflectanceDataset> rds(new msat::utils::ReflectanceDataset(rb->channel_id));
            rds->add_source(ds.release(), true);
            rds->init_rasterband();
            return rds.release();
        }
    } else {
        std::unique_ptr<XRITDataset> ds(new XRITDataset(fa, effect));
        if (!ds->init()) return NULL;
        return ds.release();
    }
}

}
}

extern "C" {

void GDALRegister_MsatXRIT()
{
    if (!GDAL_CHECK_VERSION("MsatXRIT"))
        return;

    if (GDALGetDriverByName("MsatXRIT") == NULL)
    {
        unique_ptr<GDALDriver> driver(new GDALDriver());
        driver->SetDescription("MsatXRIT");
        driver->SetMetadataItem(GDAL_DMD_LONGNAME, "Meteosat xRIT (via Meteosatlib)");
        //driver->SetMetadataItem(GDAL_DMD_HELPTOPIC, "frmt_various.html#JDEM");
        //driver->SetMetadataItem(GDAL_DMD_EXTENSION, "mem");
        driver->pfnOpen = msat::xrit::XRITOpen;
        GetGDALDriverManager()->RegisterDriver(driver.release());
    }
}

}
