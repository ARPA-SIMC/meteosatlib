/*
 * Copyright (C) 2010  ARPAE-SIMC <urpsim@arpae.it>
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

#ifndef MSAT_GDAL_NETCDF_UTILS_H
#define MSAT_GDAL_NETCDF_UTILS_H

#include <netcdfcpp.h>
#include <stdexcept>
#include <gdal_priv.h>

namespace msat {
namespace netcdf {

// TODO: redo the default with a compiler error
template<typename Sample>
static inline Sample getAttr(const NcAtt& a) { throw std::runtime_error("requested to read attribute from unknown C++ type"); }
template<> inline ncbyte getAttr<ncbyte>(const NcAtt& a) { return a.as_ncbyte(0); }
template<> inline char getAttr<char>(const NcAtt& a) { return a.as_char(0); }
template<> inline short getAttr<short>(const NcAtt& a) { return a.as_short(0); }
template<> inline int getAttr<int>(const NcAtt& a) { return a.as_int(0); }
template<> inline float getAttr<float>(const NcAtt& a) { return a.as_float(0); }
template<> inline double getAttr<double>(const NcAtt& a) { return a.as_double(0); }
template<> inline const char* getAttr<const char*>(const NcAtt& a) { return a.as_string(0); }

template<typename Sample, typename NC>
static inline Sample getAttr(const NC& el, const char* name, Sample def)
{
        NcAtt* a = el.get_att(name);
        if (a == NULL) return def;
        return getAttr<Sample>(*a);
}

template<typename NCObject, typename T>
static int ncfAddAttr(NCObject& ncf, const char* name, const T& val)
{
        if (!ncf.add_att(name, val))
        {
                std::stringstream msg;
                msg << "cannot add attribute '" << name << "' set to " << val;
                CPLError(CE_Failure, CPLE_AppDefined, "%s", msg.str().c_str());
                return FALSE;
        }
        return TRUE;
}

time_t forecastSeconds2000(const char* timestr);

class NetCDFRasterBand : public GDALRasterBand
{
public:
        NcVar* var;
        bool _unsigned;
        int channel_id;

        NetCDFRasterBand(GDALDataset* ds, int idx, NcVar* var);

        virtual const char* GetUnitType();
        virtual double GetOffset(int* pbSuccess=NULL);
        virtual double GetScale(int* pbSuccess=NULL);
        virtual double GetNoDataValue(int* pbSuccess=NULL);
        virtual CPLErr IReadBlock(int xblock, int yblock, void *buf);
};

NcVar* rasterBandToNcVar(GDALRasterBand* rb, NcFile& ncf, NcDim* tdim, NcDim* ldim, NcDim* cdim);

}
}

#endif
