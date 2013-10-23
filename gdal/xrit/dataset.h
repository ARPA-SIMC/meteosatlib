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

#ifndef MSAT_GDALDRIVER_XRIT_DATASET_H
#define MSAT_GDALDRIVER_XRIT_DATASET_H

#include <msat/xrit/fileaccess.h>
#include <msat/xrit/dataaccess.h>
#include <gdal/gdal_priv.h>
#include <string>

namespace msat {
namespace xrit {

class XRITDataset : public GDALDataset
{
public:
    typedef enum {
        PP_NONE,
        PP_REFLECTANCE,
        PP_SZA,
    } Effect;
    xrit::FileAccess fa;
    xrit::DataAccess da;
    int spacecraft_id;
    std::string projWKT;
    double geotransform[6];
    Effect effect;

    XRITDataset(const std::string& fname, Effect effect=PP_NONE);

    virtual bool init();

    virtual const char* GetProjectionRef();
    virtual CPLErr GetGeoTransform(double* tr);

};

}
}

#endif
