#include <msat/gdal/const.h>
#include <msat/gdal/dataset.h>
#include "dataset.h"
#include "rasterband.h"
#include <msat/facts.h>
#include <msat/hrit/MSG_HRIT.h>
#include <memory>
#include <ctime>

using namespace std;

namespace msat {
namespace xrit {

XRITDataset::XRITDataset(const xrit::FileAccess& fa)
    : fa(fa), spacecraft_id(0)
{
}

XRITDataset::~XRITDataset() {
    delete osr;
}

#if GDAL_VERSION_MAJOR < 3
const char* XRITDataset::GetProjectionRef()
{
    return projWKT.c_str();
}
#else
const OGRSpatialReference* XRITDataset::GetSpatialRef() const {
    return osr;
}
#endif

CPLErr XRITDataset::GetGeoTransform(double* tr)
{
    memcpy(tr, geotransform, 6 * sizeof(double));
    return CE_None;
}

bool XRITDataset::init()
{
    char buf[25];

    // Scan segment headers
    MSG_data PRO_data;
    MSG_data EPI_data;
    MSG_header header;
    da.scan(fa, PRO_data, EPI_data, header);

    if (da.hrv)
    {
        nRasterXSize = 11136;
        nRasterYSize = 11136;
    } else {
        nRasterXSize = 3712;
        nRasterYSize = 3712;
    }

    /// Spacecraft
    spacecraft_id = facts::spacecraftIDFromHRIT(header.segment_id->spacecraft_id);
    snprintf(buf, 25, "%d", spacecraft_id);
    if (SetMetadataItem(MD_MSAT_SPACECRAFT_ID, buf, MD_DOMAIN_MSAT) != CE_None)
        return false;
    string spacecraft_name = facts::spacecraftName(spacecraft_id);
    if (SetMetadataItem(MD_MSAT_SPACECRAFT, spacecraft_name.c_str(), MD_DOMAIN_MSAT) != CE_None)
        return false;


    /// Image time
    struct tm *tmtime = PRO_data.prologue->image_acquisition.PlannedAquisitionTime.TrueRepeatCycleStart.get_timestruct( );
    strftime(buf, 20, "%Y-%m-%d %H:%M:00", tmtime);
    if (SetMetadataItem(MD_MSAT_DATETIME, buf, MD_DOMAIN_MSAT) != CE_None)
        return false;


    /// Projection
    projWKT = dataset::spaceviewWKT(header.image_navigation->subsatellite_longitude);
    osr = new OGRSpatialReference(projWKT.c_str());

    /// Geotransform matrix
    double pixelSizeX, pixelSizeY;
    int column_offset, line_offset, x0 = 0, y0 = 0;
    if (da.hrv)
    {
        pixelSizeX = 1000 * PRO_data.prologue->image_description.ReferenceGridHRV.ColumnDirGridStep;
        pixelSizeY = 1000 * PRO_data.prologue->image_description.ReferenceGridHRV.LineDirGridStep;

        // Since we are omitting the first (11136-UpperWestColumnActual) of the
        // rotated image, we need to shift the column offset accordingly
        // FIXME: don't we have a way to compute this from the HRIT data?
        //img->column_offset = 5568 - (11136 - data->UpperWestColumnActual - 1);
        column_offset = 5568;
        line_offset = 5568;
    } else {
        pixelSizeX = 1000 * PRO_data.prologue->image_description.ReferenceGridVIS_IR.ColumnDirGridStep;
        pixelSizeY = 1000 * PRO_data.prologue->image_description.ReferenceGridVIS_IR.LineDirGridStep;

        column_offset = 1856;
        line_offset = 1856;
    }
    //img->geotransform[0] = -(band->column_offset - band->x0) * band->column_res;
    //img->geotransform[3] = -(band->line_offset   - band->y0) * band->line_res;
    geotransform[0] = -(column_offset - x0) * fabs(pixelSizeX);
    geotransform[3] = (line_offset   - y0) * fabs(pixelSizeY);
    //img->geotransform[1] = band->column_res;
    //img->geotransform[5] = band->line_res;
    geotransform[1] = fabs(pixelSizeX);
    geotransform[5] = -fabs(pixelSizeY);
    geotransform[2] = 0.0;
    geotransform[4] = 0.0;


    // Raster bands
    unique_ptr<XRITRasterBand> rb(new XRITRasterBand(this, 1));
    if (!rb->init(PRO_data, EPI_data, header)) return false;
    SetBand(1, rb.release());

    return true;
}


}
}


