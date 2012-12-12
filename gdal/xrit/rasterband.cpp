/*
 * Copyright (C) 2010--2012  ARPA-SIM <urpsim@smr.arpa.emr.it>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 *
 * Author: Enrico Zini <enrico@enricozini.com>
 */

#include "rasterband.h"
#include "dataset.h"
#include <msat/gdal/const.h>
#include <msat/facts.h>
#include <stdint.h>

namespace msat {
namespace xrit {

XRITRasterBand::XRITRasterBand(XRITDataset* ds, int idx)
    : xds(ds), calibration(0)
{
    poDS = ds;
    nBand = idx;
}

XRITRasterBand::~XRITRasterBand()
{
    if (calibration) delete[] calibration;
}

bool XRITRasterBand::init(MSG_data& PRO_data, MSG_data& EPI_data, MSG_header& header)
{
    if (xds->da.hrv)
    {
        nBlockXSize = 11136;
        nBlockYSize = 1;
    } else {
        nBlockXSize = 3712;
        nBlockYSize = 1;
    }

    /// Channel
    channel_id = header.segment_id->spectral_channel_id;
    char buf[25];
    snprintf(buf, 25, "%d", channel_id);
    SetMetadataItem(MD_MSAT_CHANNEL_ID, buf, MD_DOMAIN_MSAT);
    const char* channelName = facts::channelName(xds->spacecraft_id, channel_id);
    SetMetadataItem(MD_MSAT_CHANNEL, channelName, MD_DOMAIN_MSAT);

    // Set name
    SetDescription(channelName);

    // Get offset and slope
    PRO_data.prologue->radiometric_proc.get_slope_offset(channel_id, slope, offset, linear);

    // Get calibration values
    if (!linear)
    {
        int bpp = header.image_structure->number_of_bits_per_pixel;
        calibration = PRO_data.prologue->radiometric_proc.get_calibration(channel_id, bpp);
    }

    if (linear)
        eDataType = GDT_UInt16;
    else
    {
        eDataType = GDT_Float32;
        slope = 1;
        offset = 0;
    }

    return true;
}

const char* XRITRasterBand::GetUnitType()
{
    return facts::channelUnit(xds->spacecraft_id, channel_id);
}

CPLErr XRITRasterBand::IReadBlock(int xblock, int yblock, void *buf)
{
    if (xblock != 0)
    {
        CPLError(CE_Failure, CPLE_AppDefined, "Invalid block number");
        return CE_Failure;
    }

    size_t linestart = xds->da.line_start(yblock);

    if (linear)
    {
        bzero(buf, nBlockXSize * sizeof(uint16_t));
        xds->da.line_read(yblock, (MSG_SAMPLE*)buf + linestart);
    } else {
        float* fbuf = (float*)buf;
        MSG_SAMPLE rawbuf[xds->da.columns];
        xds->da.line_read(yblock, rawbuf);
        for (size_t i = 0; i < linestart; ++i)
            fbuf[i] = 0.0;
        for (size_t i = 0; i < xds->da.columns; ++i)
        {
            float res = calibration[rawbuf[i]];
            if (res < 0 || isnan(res)) res = 0;
            fbuf[linestart + i] = res;
        }
        for (size_t i = linestart + xds->da.columns; i < (size_t)nBlockXSize; ++i)
            fbuf[i] = 0.0;
    }

    return CE_None;
}

double XRITRasterBand::GetOffset(int* pbSuccess)
{
    if (pbSuccess) *pbSuccess = TRUE;
    return offset;
}

double XRITRasterBand::GetScale(int* pbSuccess)
{
    if (pbSuccess) *pbSuccess = TRUE;
    return slope;
}

double XRITRasterBand::GetNoDataValue(int* pbSuccess)
{
    if (pbSuccess) *pbSuccess = TRUE;
    return 0.0;
}

}
}
