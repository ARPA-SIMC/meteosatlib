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

    virtual const char* GetUnitType();

    virtual CPLErr IReadBlock(int xblock, int yblock, void *buf);

    virtual double GetOffset(int* pbSuccess=NULL);
    virtual double GetScale(int* pbSuccess=NULL);
    virtual double GetNoDataValue(int* pbSuccess=NULL);
};

}
}

// vim:set sw=2:
#endif

