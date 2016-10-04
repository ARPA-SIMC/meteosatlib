#include "xrit.h"
#include "dataset.h"
#include "rasterband.h"
#include "gdal/reflectance/reflectance.h"
#include "gdal/reflectance/cos_sol_za.h"
#include "gdal/utils.h"
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
    bool do_reflectance = false;
    bool do_sza = false;
    FileAccess fa(info->pszFilename);
    if (!fa.productid2.empty())
    {
        switch (fa.productid2[fa.productid2.size() - 1])
        {
            case 'r': do_reflectance = true; break;
            case 'a': do_sza = true; break;
        }

        // Remove the suffix
        if (do_reflectance || do_sza)
            fa.productid2.resize(fa.productid2.size() - 1);
    }

    if (do_reflectance)
    {
        if (fa.productid2 == "IR_039")
        {
            std::unique_ptr<XRITDataset> ds039(new XRITDataset(fa));
            if (!ds039->init()) return NULL;
            std::unique_ptr<XRITDataset> ds108(new XRITDataset(FileAccess(fa, "IR_108")));
            if (!ds108->init()) return NULL;
            std::unique_ptr<XRITDataset> ds134(new XRITDataset(FileAccess(fa, "IR_134")));
            if (!ds134->init()) return NULL;

            unique_ptr<msat::utils::ReflectanceDataset> rds(new msat::utils::ReflectanceDataset(MSG_SEVIRI_1_5_IR_3_9));
            rds->add_source(ds039.release(), true);
            rds->add_source(ds108.release(), true);
            rds->add_source(ds134.release(), true);
            rds->init_rasterband();
            return rds.release();
        } else {
            std::unique_ptr<XRITDataset> ds(new XRITDataset(fa));
            if (!ds->init()) return NULL;
            XRITRasterBand* rb = dynamic_cast<XRITRasterBand*>(ds->GetRasterBand(1));
            unique_ptr<msat::utils::ReflectanceDataset> rds(new msat::utils::ReflectanceDataset(rb->channel_id));
            rds->add_source(ds.release(), true);
            rds->init_rasterband();
            return rds.release();
        }
    } else if (do_sza) {
        std::unique_ptr<XRITDataset> ds(new XRITDataset(fa));
        if (!ds->init()) return NULL;
        unique_ptr<msat::utils::CosSolZADataset> rds(new msat::utils::CosSolZADataset(ds.get()));
        return rds.release();
    } else {
        std::unique_ptr<XRITDataset> ds(new XRITDataset(fa));
        if (!ds->init()) return NULL;
        return msat::gdal::add_extras(ds.release(), info);
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
