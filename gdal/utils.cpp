#include <cpl_string.h>
#include "msat/gdal/const.h"
#include "msat/hrit/MSG_channel.h"
#include "gdal/reflectance/reflectance.h"
#include "utils.h"

using namespace std;

namespace msat {
namespace gdal {

GDALDataset* add_extras(GDALDataset* src, GDALOpenInfo* info)
{
    int idx = CSLFindName(info->papszOpenOptions, "MSAT_COMPUTE");
    if (idx == -1) return src;
    std::string val(CPLParseNameValue(info->papszOpenOptions[idx], nullptr));

    if (val == "reflectance")
    {
        GDALRasterBand* rb = src->GetRasterBand(1);
        const char* str_id = rb->GetMetadataItem(MD_MSAT_CHANNEL_ID, MD_DOMAIN_MSAT);
        if (str_id == nullptr)
            throw std::runtime_error("cannot compute reflectance of a channel without " MD_DOMAIN_MSAT "/" MD_MSAT_CHANNEL_ID " metadata");
        unsigned channel_id = strtoul(str_id, nullptr, 10);
        if (channel_id == MSG_SEVIRI_1_5_IR_3_9)
            throw std::runtime_error("IR 0.39 reflectance cannot yet be computed via MSAT_COMPUTE open option");

        unique_ptr<msat::utils::ReflectanceDataset> rds(new msat::utils::ReflectanceDataset(channel_id));
        rds->add_source(src, true);
        rds->init_rasterband();
        return rds.release();
    }

    return src;
}

}
}
