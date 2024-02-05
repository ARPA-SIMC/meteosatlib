/*
 * Copyright (C) 2010--2012  ARPAE-SIMC <urpsim@arpae.it>
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

#include "utils.h"
#include <msat/facts.h>
#include <string>

using namespace std;

namespace msat {
namespace netcdf {

time_t forecastSeconds2000(const char* timestr)
{
        const time_t s_epoch_2000 = 946684800;
        struct tm itm;

        int year, month, day, hour, minute, second;
        if (sscanf(timestr, "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second) != 6)
                return 0;

        bzero(&itm, sizeof(struct tm));
        itm.tm_year = year - 1900;
        itm.tm_mon  = month - 1;
        itm.tm_mday = day;
        itm.tm_hour = hour;
        itm.tm_min  = minute;

        // This ugly thing, or use the GNU extension timegm
        const char* tz = getenv("TZ");
        setenv("TZ", "", 1);
        tzset();
        time_t res = mktime(&itm);
        if (tz)
                setenv("TZ", tz, 1);
        else
                unsetenv("TZ");
        tzset();

        // Also add timezone (and declare it as extern, see timezone(3) manpage) to
        // get the seconds in local time
        return res - s_epoch_2000;
}


NetCDFRasterBand::NetCDFRasterBand(GDALDataset* ds, int idx, NcVar* var) : var(var), _unsigned(false), channel_id(0)
{
        poDS = ds;
        nBand = idx;

        nBlockXSize = var->get_dim(2)->size();
        nBlockYSize = var->get_dim(1)->size();

        // Choose data type
        string _Unsigned = getAttr(*var, "_Unsigned", "false");
        _unsigned = _Unsigned == "true";
        switch (var->type())
        {
                case ncNoType: eDataType = GDT_Unknown; break;
                case ncByte:   eDataType = GDT_Byte; break;
                case ncChar:   eDataType = GDT_Byte; break;
                case ncShort:  eDataType = _unsigned ? GDT_UInt16 : GDT_Int16; break;
                case ncInt:    eDataType = _unsigned ? GDT_UInt32 : GDT_Int32; break;
                case ncFloat:  eDataType = GDT_Float32; break;
                case ncDouble: eDataType = GDT_Float64; break;
        }

        SetDescription(var->name());
}

const char* NetCDFRasterBand::GetUnitType()
{
        NcError nce(NcError::silent_nonfatal);
        NcAtt* a = var->get_att("units");
        if (a != NULL)
                return a->as_string(0);
        else
                return "";
}

double NetCDFRasterBand::GetOffset(int* pbSuccess)
{
        NcError nce(NcError::silent_nonfatal);
        NcAtt* a = var->get_att("add_offset");
        if (a != NULL)
        {
                if (pbSuccess) *pbSuccess = TRUE;
                double res = getAttr<double>(*a);
                return res;
        } else {
                if (pbSuccess) *pbSuccess = FALSE;
                return 0;
        }
}

double NetCDFRasterBand::GetScale(int* pbSuccess)
{
        NcError nce(NcError::silent_nonfatal);
        NcAtt* a = var->get_att("scale_factor");
        if (a != NULL)
        {
                if (pbSuccess) *pbSuccess = TRUE;
                return getAttr<double>(*a);
        } else {
                if (pbSuccess) *pbSuccess = FALSE;
                return 1;
        }
}

double NetCDFRasterBand::GetNoDataValue(int* pbSuccess)
{
        NcError nce(NcError::silent_nonfatal);
        if (NcAtt* a = var->get_att("_FillValue"))
        {
                if (pbSuccess) *pbSuccess = TRUE;
                if (_unsigned)
                        switch (var->type())
                        {
                                case ncByte:   return (unsigned char)getAttr<ncbyte>(*a);
                                case ncChar:   return (unsigned char)getAttr<ncbyte>(*a);
                                case ncShort:  return (unsigned short)getAttr<short int>(*a);
                                case ncInt:    return (unsigned int)getAttr<int>(*a);
                                default:       return getAttr<double>(*a);
                        }
                else
                        return getAttr<double>(*a);
        }

        if (NcAtt* a = var->get_att("missing_value"))
        {
                if (pbSuccess) *pbSuccess = TRUE;
                return getAttr<double>(*a);
        }

        // TODO: set _FillValue so that reading the data does the right thing
        // var.add_att("_FillValue", img.missingValue);

        if (pbSuccess) *pbSuccess = TRUE;
        return facts::defaultPackedMissing(channel_id);
}

CPLErr NetCDFRasterBand::IReadBlock(int xblock, int yblock, void *buf)
{
        NcError nce(NcError::silent_nonfatal);
        if (xblock != 0 || yblock != 0)
        {
                CPLError(CE_Failure, CPLE_AppDefined, "Invalid block number");
                return CE_Failure;
        }
        
        bool res = false;

        switch (eDataType)
        {
                case GDT_Byte:    res = var->get((ncbyte*)buf, 1, nBlockYSize, nBlockXSize); break;
                case GDT_Int16:
                case GDT_UInt16:  res = var->get( (short*)buf, 1, nBlockYSize, nBlockXSize); break;
                case GDT_Int32:
                case GDT_UInt32:  res = var->get(  (long*)buf, 1, nBlockYSize, nBlockXSize); break;
                case GDT_Float32: res = var->get( (float*)buf, 1, nBlockYSize, nBlockXSize); break;
                case GDT_Float64: res = var->get((double*)buf, 1, nBlockYSize, nBlockXSize); break;
                default:
                        CPLError(CE_Failure, CPLE_AppDefined, "Unsupported raster band data type %d", (int)eDataType);
                        break;
        }

        if (!res)
        {
                CPLError(CE_Failure, CPLE_AppDefined, "reading image pixels failed");
                return CE_Failure;
        }

        return CE_None;
}

template<typename DTYPE>
bool copy_data(NcVar& dst, GDALRasterBand& src, GDALDataType outType)
{
    DTYPE* pixels = new DTYPE[src.GetXSize()*src.GetYSize()];
    if (src.RasterIO(GF_Read, 0, 0, src.GetXSize(), src.GetYSize(), pixels, src.GetXSize(), src.GetYSize(), outType, 0, 0) != CE_None)
    {
        delete[] pixels;
        return false;
    }
    if (!dst.put(pixels, 1, src.GetYSize(), src.GetXSize()))
    {
        delete[] pixels;
        CPLError(CE_Failure, CPLE_AppDefined, "Cannot write image values");
        return false;
    }
    delete[] pixels;
    return true;
}

NcVar* rasterBandToNcVar(GDALRasterBand* rb, NcFile& ncf, NcDim* tdim, NcDim* ldim, NcDim* cdim)
{
    NcError nce(NcError::silent_nonfatal);
    GDALDataType dtype = rb->GetRasterDataType();
    NcType dtdst;
    GDALDataType gdaldst;
    // See http://www.unidata.ucar.edu/software/netcdf/docs/BestPractices.html#Unsigned%20Data
    bool _Unsigned = false;
    switch (dtype)
    {
        case GDT_Unknown:
            CPLError(CE_Failure, CPLE_AppDefined, "Source raster band has unknown data type");
            return NULL;
        case GDT_Byte: dtdst = ncByte; gdaldst = GDT_Byte; _Unsigned = true; break;
        case GDT_Int16: dtdst = ncShort; gdaldst = GDT_Int16; break;
        case GDT_UInt16: dtdst = ncShort; gdaldst = GDT_UInt16; _Unsigned = true; break;
        case GDT_Int32: dtdst = ncInt; gdaldst = GDT_Int32; break;
        case GDT_UInt32: dtdst = ncInt; gdaldst = GDT_UInt32; _Unsigned = true; break;
        case GDT_Float32: dtdst = ncFloat; gdaldst = GDT_Float32; break;
        case GDT_Float64: dtdst = ncDouble; gdaldst = GDT_Float64; break;
        case GDT_CInt16:
        case GDT_CInt32:
        case GDT_CFloat32:
        case GDT_CFloat64:
            CPLError(CE_Failure, CPLE_AppDefined, "Source raster band has complex data type, not supported by NetCDF");
            return NULL;
        default:
            CPLError(CE_Failure, CPLE_AppDefined, "Source raster band has unknown data type");
            return NULL;
    }

    NcVar *ivar = ncf.add_var(rb->GetDescription(), dtdst, tdim, ldim, cdim);
    if (!ivar || !ivar->is_valid())
    {
        CPLError(CE_Failure, CPLE_AppDefined, "Cannot add variable '%s': %s", rb->GetDescription(), nce.get_errmsg());
        return NULL;
    }
    if (!ncfAddAttr(*ivar, "add_offset", rb->GetOffset())) return NULL;
    if (!ncfAddAttr(*ivar, "scale_factor", rb->GetScale())) return NULL;
    if (!ncfAddAttr(*ivar, "units", rb->GetUnitType())) return NULL;
    if (_Unsigned)
        if (!ncfAddAttr(*ivar, "_Unsigned", "true")) return NULL;

    // Write output values
    //cerr << "output." << endl;
    bool res = false;
    switch (dtdst)
    {
        case ncByte:
            if (!ncfAddAttr(*ivar, "_FillValue", (int8_t)rb->GetNoDataValue())) return NULL;
            res = copy_data<ncbyte>(*ivar, *rb, gdaldst);
            break;
        case ncShort:
            if (!ncfAddAttr(*ivar, "_FillValue", (int16_t)rb->GetNoDataValue())) return NULL;
            res = copy_data<int16_t>(*ivar, *rb, gdaldst);
            break;
        case ncInt:
            if (!ncfAddAttr(*ivar, "_FillValue", (int32_t)rb->GetNoDataValue())) return NULL;
            res = copy_data<int32_t>(*ivar, *rb, gdaldst);
            break;
        case ncFloat:
            if (!ncfAddAttr(*ivar, "_FillValue", (float)rb->GetNoDataValue())) return NULL;
            res = copy_data<float>(*ivar, *rb, gdaldst);
            break;
        case ncDouble:
            if (!ncfAddAttr(*ivar, "_FillValue", (double)rb->GetNoDataValue())) return NULL;
            res = copy_data<double>(*ivar, *rb, gdaldst);
            break;
        default:
            CPLError(CE_Failure, CPLE_AppDefined, "programming error: an unsupported target data type has been selected");
            return NULL;
    }
    if (!res)
        return nullptr;
    return ivar;
}

}
}
