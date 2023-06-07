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

#include "netcdf.h"
#include <msat/gdal/const.h>
#include <msat/gdal/dataset.h>
#include "utils.h"
#include <msat/facts.h>
#include <gdal.h>
#include <gdal_priv.h>
#include <gdal/ogr_spatialref.h>
#include "gdal/utils.h"
#include <string>
#include <sstream>
#include <memory>
#include <cstdlib>
#include <cstdio>
#include <stdexcept>
#include <math.h>
#include "config.h"

using namespace std;

namespace msat {
namespace netcdf {

class NetCDF24Dataset : public GDALDataset
{
protected:
        string projWKT;
public:
        NcFile* nc;
        double geotransform[6];
        int spacecraft_id;
        OGRSpatialReference* osr = nullptr;

        NetCDF24Dataset(NcFile* nc) : nc(nc) {}
        ~NetCDF24Dataset()
        {
                if (nc != NULL) delete nc;
        }
        virtual bool init();

#if GDAL_VERSION_MAJOR < 3
        virtual const char* GetProjectionRef() override
        {
                return projWKT.c_str();
        }
#else
        const OGRSpatialReference* GetSpatialRef() const override {
            return osr;
        }
#endif

        virtual CPLErr GetGeoTransform(double* tr)
        {
                if (projWKT.empty()) return CE_Failure;
                memcpy(tr, geotransform, 6 * sizeof(double));
                return CE_None;
        }
};

class NetCDF24RasterBand : public NetCDFRasterBand
{
public:
        NetCDF24RasterBand(NetCDF24Dataset* ds, int idx, NcVar* var) : NetCDFRasterBand(ds, idx, var)
        {
                /// Channel
                if (NcAtt* a = var->get_att("L1"))
                {
                        channel_id = getAttr<int>(*a);

                        // Channel
                        char buf[25];
                        snprintf(buf, 25, "%d", channel_id);
                        SetMetadataItem(MD_MSAT_CHANNEL_ID, buf, MD_DOMAIN_MSAT);
                        SetMetadataItem(MD_MSAT_CHANNEL,
                                        facts::channelName(ds->spacecraft_id, channel_id),
                                        MD_DOMAIN_MSAT);
                }
        }
};

bool NetCDF24Dataset::init()
{
        NcFile& ncf = *nc;
        int itmp;
        std::string stmp;
        char buf[25];

        //ds->nRasterXSize = 0;
        //ds->nRasterYSize = 0;
        //ds->setQualityFromPathname(filename);

        // Sanity checks
        itmp = getAttr<int>(ncf, "GribEditionNumber", 1);
        if (itmp != 1)
        {
                CPLError(CE_Failure, CPLE_AppDefined, "Message has unsupported edition number %d (only 1 is supported)", itmp);
                return false;
        }
        itmp = getAttr<int>(ncf, "LevelType", 0);
        if (itmp != 55 && itmp != 56)
        {
                CPLError(CE_Failure, CPLE_AppDefined, "Only Meteosat 8 and Meteosat 9 data is currently imported");
                return false;
        }

        /// Spacecraft
        spacecraft_id = getAttr<int>(ncf, "SatelliteID", 0);
        if (spacecraft_id != 0)
        {
                const char* spacecraft_name = facts::spacecraftName(spacecraft_id);
                snprintf(buf, 25, "%d", spacecraft_id);
                if (SetMetadataItem(MD_MSAT_SPACECRAFT_ID, buf, MD_DOMAIN_MSAT) != CE_None)
                        return false;
                if (SetMetadataItem(MD_MSAT_SPACECRAFT, spacecraft_name, MD_DOMAIN_MSAT) != CE_None)
                        return false;
        }

        /// Image time
        int year = getAttr<int>(ncf, "Year", 0);
        int month = getAttr<int>(ncf, "Month", 0);
        int day = getAttr<int>(ncf, "Day", 0);
        int hour = getAttr<int>(ncf, "Hour", 0);
        int minute = getAttr<int>(ncf, "Minute", 0);
        int second = getAttr<int>(ncf, "Second", 0);
        snprintf(buf, 20, "%04d-%02d-%02d %02d:%02d:%02d", year, month, day, hour, minute, second);
        if (SetMetadataItem(MD_MSAT_DATETIME, buf, MD_DOMAIN_MSAT) != CE_None)
                return false;

        /// Projection
        NcVar* proj = nc->get_var("Projection");
        if (NcAtt* a = proj->get_att("Lop"))
        {
                projWKT = dataset::spaceviewWKT(getAttr<float>(*a));
                osr = new OGRSpatialReference(projWKT.c_str());

                // Compute geotransform matrix
#if 0
                column_res = Image::columnResFromSeviriDX(PGET(int, "DX"));
                line_res = Image::lineResFromSeviriDY(PGET(int, "DY"));
                column_offset = PGET(int, "Xp");
                line_offset =   PGET(int, "Yp");
#endif
                if (!(a = proj->get_att("Xp")))
                {
                        CPLError(CE_Failure, CPLE_AppDefined, "Projection variable does not have Xp attribute");
                        return false;
                }
                long column_offset = getAttr<int>(*a);

                if (!(a = proj->get_att("Yp")))
                {
                        CPLError(CE_Failure, CPLE_AppDefined, "Projection variable does not have Yp attribute");
                        return false;
                }
                long line_offset = getAttr<int>(*a);

                if (!(a = proj->get_att("X0")))
                {
                        CPLError(CE_Failure, CPLE_AppDefined, "Projection variable does not have X0 attribute");
                        return false;
                }
                long x0 = getAttr<int>(*a) - 1;

                if (!(a = proj->get_att("Y0")))
                {
                        CPLError(CE_Failure, CPLE_AppDefined, "Projection variable does not have Y0 attribute");
                        return false;
                }
                long y0 = getAttr<int>(*a) - 1;

                if (!(a = proj->get_att("DX")))
                {
                        CPLError(CE_Failure, CPLE_AppDefined, "Projection variable does not have DX attribute");
                        return false;
                }
                double psx = facts::pixelHSizeFromSeviriDX(getAttr<int>(*a));

                if (!(a = proj->get_att("DY")))
                {
                        CPLError(CE_Failure, CPLE_AppDefined, "Projection variable does not have DY attribute");
                        return false;
                }
                double psy = facts::pixelVSizeFromSeviriDY(getAttr<int>(*a));

                geotransform[0] = -(column_offset - x0) * psx;
                geotransform[3] = (line_offset   - y0) * psy;
                geotransform[1] = psx;
                geotransform[5] = -psy;
                geotransform[2] = 0.0;
                geotransform[4] = 0.0;
        }

        int nextBand = 1;
        for (int i = 0; i < ncf.num_vars(); ++i)
        {
                NcVar* var = ncf.get_var(i);
                if (strcmp(var->name(), "Projection") == 0) continue;
                if (strcmp(var->name(), "time") == 0) continue;
                if (var->num_dims() != 3)
                {
                        CPLError(CE_Warning, CPLE_AppDefined, "ignoring variable %s which has %d dimensions instead of 3", var->name(), var->num_dims());
                        continue;
                }
                if (var->get_dim(0)->size() != 1)
                {
                        CPLError(CE_Warning, CPLE_AppDefined, "ignoring variable %s which has %ld items in the time dimension instead of 1", var->name(), var->get_dim(0)->size());
                        continue;
                }

                // Get raster size
                int sx = var->get_dim(2)->size();
                int sy = var->get_dim(1)->size();

                if (nextBand == 1)
                {
                        // If it's the first band, we also set the dataset
                        // raster size
                        nRasterXSize = sx;
                        nRasterYSize = sy;
                }

                SetBand(nextBand, new NetCDF24RasterBand(this, nextBand, var));
                ++nextBand;
        }

        return true;
}

GDALDataset* NetCDF24Open(GDALOpenInfo* info)
{
    // We want a real file
#if GDAL_VERSION_MAJOR >= 2
    if (info->fpL == NULL) return NULL;
#else
    if (info->fp == NULL) return NULL;
#endif

    NcError nce(NcError::silent_nonfatal);

    // Try opening and seeing if it is a NetCDF24 file
    unique_ptr<NcFile> nc;
    try {
        nc.reset(new NcFile(info->pszFilename, NcFile::ReadOnly));
        if (!nc->is_valid()) return NULL;
        if (nc->get_att("GribEditionNumber") == NULL)
            return NULL;
    } catch (...) {
        return NULL;
    }

    // Create the dataset
    unique_ptr<NetCDF24Dataset> ds(new NetCDF24Dataset(nc.release()));

    // Initialise the generic dataset bits using information from the
    // NetCDF data
    if (!ds->init()) return NULL;

    return msat::gdal::add_extras(ds.release(), info);
}

GDALDataset* NetCDF24CreateCopy(const char* pszFilename, GDALDataset* src, 
                              int bStrict, char** papszOptions, 
                              GDALProgressFunc pfnProgress, void* pProgressData)
{
        NcError nce(NcError::silent_nonfatal);

        // Build up output NetCDF24 file name and open it
        NcFile ncf(pszFilename, NcFile::Replace);
        if (!ncf.is_valid())
        {
                CPLError(CE_Failure, CPLE_AppDefined, "Cannot create NetCDF24 file %s: %s", pszFilename, nce.get_errmsg());
                return NULL;
        }

        // Fill arrays on creation
        ncf.set_fill(NcFile::Fill);

        //
        // Add Global Attributes
        //

        // Add fixed attributes
        if (!ncfAddAttr(ncf, "GribEditionNumber", 1)) return NULL;
        if (!ncfAddAttr(ncf, "GeneratingCentre", 200)) return NULL;
        if (!ncfAddAttr(ncf, "GeneratingSubCentre", 0)) return NULL;
        if (!ncfAddAttr(ncf, "GeneratingProcess", 254)) return NULL;  // FIXME: unsure
        if (!ncfAddAttr(ncf, "Parameter", 127)) return NULL;      // Image data
        if (!ncfAddAttr(ncf, "L2", 0)) return NULL;

        // Spacecraft
        const char* sval = src->GetMetadataItem(MD_MSAT_SPACECRAFT_ID, MD_DOMAIN_MSAT);
        if (sval == NULL)
        {
                CPLError(CE_Failure, CPLE_AppDefined, "Metadata %s not found in source dataset", MD_MSAT_SPACECRAFT_ID);
                return NULL;
        }
        int spacecraft_id = (int)strtoul(sval, NULL, 10);
        if (!ncfAddAttr(ncf, "LevelType", spacecraft_id)) return NULL;
        if (!ncfAddAttr(ncf, "SatelliteID", spacecraft_id)) return NULL;

        // Datetime
        time_t fs2000 = 0;
        sval = src->GetMetadataItem(MD_MSAT_DATETIME, MD_DOMAIN_MSAT);
        if (sval != NULL)
        {
                fs2000 = forecastSeconds2000(sval);
                string tmp = sval;
                tmp += " UTC";
                if (!ncfAddAttr(ncf, "Time", tmp.c_str())) return NULL;

                int year, month, day, hour, minute, second;
                if (sscanf(sval, "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second) != 6)
                        return 0;

                if (!ncfAddAttr(ncf, "Year",   year)) return NULL;
                if (!ncfAddAttr(ncf, "Month",  month)) return NULL;
                if (!ncfAddAttr(ncf, "Day",    day)) return NULL;
                if (!ncfAddAttr(ncf, "Hour",   hour)) return NULL;
                if (!ncfAddAttr(ncf, "Minute", minute)) return NULL;
                if (!ncfAddAttr(ncf, "Second", second)) return NULL;
        }

        /// Projection
        NcVar *projVar = ncf.add_var("Projection", ncInt);
        if (!projVar->is_valid())
        {
                CPLError(CE_Failure, CPLE_AppDefined, "adding projection variable failed");
                return NULL;
        }

        OGRSpatialReference osr(*src->GetSpatialRef());

        // Sanity checks, and computation of target values
        const char* stype = osr.GetAttrValue("PROJECTION");
        if (!stype)
        {
                CPLError(CE_Failure, CPLE_AppDefined, "projection set to latlon but only %s is supported", SRS_PT_GEOSTATIONARY_SATELLITE);
                return NULL;
        }
        string type = stype;
        if (type != SRS_PT_GEOSTATIONARY_SATELLITE)
        {
                CPLError(CE_Failure, CPLE_AppDefined, "projection set to %s but only %s is supported", type.c_str(), SRS_PT_GEOSTATIONARY_SATELLITE);
                return NULL;
        }
        if (osr.GetProjParm(SRS_PP_SATELLITE_HEIGHT) != ORBIT_RADIUS_FOR_GDAL)
        {
                CPLError(CE_Failure, CPLE_AppDefined, "satellite height set to %f but only %d is supported", osr.GetProjParm(SRS_PP_SATELLITE_HEIGHT), ORBIT_RADIUS_FOR_GDAL);
                return NULL;
        }

        // Set the projection parameters in the NetCDF file
        float sublon = (float)osr.GetProjParm(SRS_PP_CENTRAL_MERIDIAN, 0.0);
        if (!ncfAddAttr(*projVar, "Lap", 0.0)) return NULL;
        if (!ncfAddAttr(*projVar, "Lop", sublon)) return NULL;
        if (!ncfAddAttr(*projVar, "Orientation", SEVIRI_ORIENTATION)) return NULL;
        if (!ncfAddAttr(*projVar, "Nz", SEVIRI_CAMERA_H)) return NULL;

        // Geotransform matrix

        // Sanity checks, and computation of target values
        double gt[6];
        if (src->GetGeoTransform(gt) != CE_None)
        {
                CPLError(CE_Failure, CPLE_AppDefined, "source dataset does not have a geotransform matrix");
                return NULL;
        }
        if (gt[2] != 0.0)
        {
                CPLError(CE_Failure, CPLE_AppDefined, "3rd element of geotransform matrix is not zero");
                return NULL;
        }
        if (gt[4] != 0.0)
        {
                CPLError(CE_Failure, CPLE_AppDefined, "5rd element of geotransform matrix is not zero");
                return NULL;
        }

        int column_offset;
        int line_offset;
        // Check if we are HRV or not
        if (fabs(gt[1] - METEOSAT_PIXELSIZE_X_HRV) > 0.0001)
        {
                if (fabs(gt[1] - METEOSAT_PIXELSIZE_X) > 0.0001)
                {
                        CPLError(CE_Failure, CPLE_AppDefined, "2nd element of geotransform matrix has an unexpected value (got: %f, expected: %f)", gt[1], METEOSAT_PIXELSIZE_X);
                        return NULL;
                }
                if (fabs(gt[5] - -METEOSAT_PIXELSIZE_Y) > 0.0001)
                {
                        CPLError(CE_Failure, CPLE_AppDefined, "6th element of geotransform matrix has an unexpected value (got: %f, expected: %f)", gt[5], METEOSAT_PIXELSIZE_Y);
                        return NULL;
                }

                column_offset = METEOSAT_IMAGE_NCOLUMNS / 2;
                line_offset = METEOSAT_IMAGE_NLINES / 2;
        } else {
                if (fabs(gt[5] - -METEOSAT_PIXELSIZE_Y_HRV) > 0.0001)
                {
                        CPLError(CE_Failure, CPLE_AppDefined, "6th element of geotransform matrix has an unexpected value (got: %f, expected: %f)", gt[5], METEOSAT_PIXELSIZE_Y_HRV);
                        return NULL;
                }

                column_offset = METEOSAT_IMAGE_NCOLUMNS_HRV / 2;
                line_offset = METEOSAT_IMAGE_NLINES_HRV / 2;
        }
        int x0 = nearbyint(column_offset + gt[0] / gt[1]);
        int y0 = nearbyint(line_offset - gt[3] / -gt[5]);
        int dx = facts::seviriDXFromPixelHSize(gt[1]);
        int dy = facts::seviriDYFromPixelVSize(-gt[5]);

        if (!ncfAddAttr(*projVar, "X0", x0 + 1)) return NULL;
        if (!ncfAddAttr(*projVar, "Y0", y0 + 1)) return NULL;
        if (!ncfAddAttr(*projVar, "DX", dx)) return NULL;
        if (!ncfAddAttr(*projVar, "DY", dy)) return NULL;
        if (!ncfAddAttr(*projVar, "Xp", column_offset)) return NULL;
        if (!ncfAddAttr(*projVar, "Yp", line_offset)) return NULL;

#if 0
                // Set variables in NetCDF

                if (!ncfAddAttr(ncf, "Area_Name", "SpaceView")) return NULL;
                char projname[16];
                sprintf(projname, "GEOS(%3.1f)", sublon);
                if (!ncfAddAttr(ncf, "Projection", projname)) return NULL;
                if (!ncfAddAttr(ncf, "Orbit_Radius", ORBIT_RADIUS)) return NULL;
                //if (! ncf.add_att("Orbit_Radius", pds.sh) ) return false;
                if (!ncfAddAttr(ncf, "Longitude", sublon)) return NULL;


                if (!ncfAddAttr(ncf, "Column_Scale_Factor", column_res)) return NULL;
                if (!ncfAddAttr(ncf, "Line_Scale_Factor", line_res)) return NULL;
                //ncfAddAttr(*nc, "Column_Scale_Factor", -13642337);
                //ncfAddAttr(*nc, "Line_Scale_Factor", -13642337);
                //if (!ncfAddAttr(ncf, "Column_Scale_Factor", img.column_res * exp2(16))) return NULL;
                //if (!ncfAddAttr(ncf, "Line_Scale_Factor", img.line_res * exp2(16))) return NULL;

                if (!ncfAddAttr(ncf, "AreaStartPix", x0 + 1)) return NULL;
                if (!ncfAddAttr(ncf, "AreaStartLin", y0 + 1)) return NULL;
                if (!ncfAddAttr(ncf, "SampleX", 1.0 )) return NULL;
                if (!ncfAddAttr(ncf, "SampleY", 1.0 )) return NULL;
                if (!ncfAddAttr(ncf, "Column_Offset", column_offset)) return NULL;
                if (!ncfAddAttr(ncf, "Line_Offset", line_offset)) return NULL;

                if (!ncfAddAttr(ncf, "NortPolar", 1)) return NULL;
                if (!ncfAddAttr(ncf, "NorthSouth", column_res >= 0 ? 1 : 0)) return NULL;
        }

        //if (!ncf.add_att("Columns", 3712 ) ) return NULL;
        //if (!ncf.add_att("Lines", 3712 ) ) return NULL;
        if (!ncfAddAttr(ncf, "Columns", src->GetRasterXSize())) return NULL;
        if (!ncfAddAttr(ncf, "Lines", src->GetRasterYSize())) return NULL;

        if (!ncfAddAttr(ncf, "title", src->GetDescription())) return NULL;

        sval = src->GetMetadataItem(MD_MSAT_INSTITUTION, MD_DOMAIN_MSAT);
        if (sval == NULL)
        {
                sval = getenv("MSAT_DEFAULT_INSTITUTION");
                if (sval == NULL)
                        sval = MSAT_DEFAULT_INSTITUTION;
        }
        if (!ncfAddAttr(ncf, "Institution", sval)) return NULL;
        sval = src->GetMetadataItem(MD_MSAT_PRODUCT_TYPE, MD_DOMAIN_MSAT);
        if (sval == NULL)
        {
                sval = getenv("MSAT_DEFAULT_PRODUCT_TYPE");
                if (sval == NULL)
                        sval = "Processed products";
        }
        if (!ncfAddAttr(ncf, "Type", sval)) return NULL;
        if (!ncfAddAttr(ncf, "Version", PACKAGE_VERSION)) return NULL;
        if (!ncfAddAttr(ncf, "Conventions", "COARDS")) return NULL;
        // if (!ncfAddAttr(ncf, "history", img.history.c_str())) return NULL;
#endif

        //
        // Add dimensions
        //
        //cerr << "dimensions." << endl;

        NcDim *tdim = ncf.add_dim("time");
        if (!tdim->is_valid())
        {
                CPLError(CE_Failure, CPLE_AppDefined, "Cannot add time dimension");
                return NULL;
        }
        NcDim *ldim = ncf.add_dim("line", src->GetRasterYSize());
        if (!ldim->is_valid())
        {
                CPLError(CE_Failure, CPLE_AppDefined, "Cannot add line dimension");
                return NULL;
        }
        NcDim *cdim = ncf.add_dim("column", src->GetRasterXSize());
        if (!cdim->is_valid())
        {
                CPLError(CE_Failure, CPLE_AppDefined, "Cannot add column dimension");
                return NULL;
        }

        //
        // Add Calibration values
        //
        //cerr << "calibration. cs: " << channelstring << endl;

        NcVar *tvar = ncf.add_var("time", ncDouble, tdim);
        if (!tvar->is_valid())
        {
                CPLError(CE_Failure, CPLE_AppDefined, "Cannot add time variable");
                return NULL;
        }
        tvar->add_att("long_name", "Time");
        tvar->add_att("units", "seconds since 2000-01-01 00:00:00 UTC");
        double atime = (double)fs2000;
        if (!tvar->put(&atime, 1))
        {
                CPLError(CE_Failure, CPLE_AppDefined, "Cannot set time variable");
                return NULL;
        }

        for (int i = 1; i <= src->GetRasterCount(); ++i)
        {
                GDALRasterBand* rb = src->GetRasterBand(i);
                NcVar* ivar = rasterBandToNcVar(rb, ncf, tdim, ldim, cdim);

                const char* sval = rb->GetMetadataItem(MD_MSAT_CHANNEL_ID, MD_DOMAIN_MSAT);
                if (sval != NULL)
                {
                        int channel_id = (int)strtoul(sval, NULL, 10);
                        if (!ncfAddAttr(*ivar, "L1", channel_id)) return NULL;
                        if (!ncfAddAttr(*ivar, "channel", channel_id)) return NULL;
                        const char* channel_name = facts::channelName(spacecraft_id, channel_id);
                        if (!ncfAddAttr(*ivar, "channelName", channel_name)) return NULL;
                }
        }

        // Close NetCDF output
        (void) ncf.close( );

        //  cout << "Wrote file " << NcName << "." << endl;

        return (GDALDataset*)GDALOpen(pszFilename, GA_ReadOnly);
}

}
}

extern "C" {

void GDALRegister_MsatNetCDF24()
{
    if (!GDAL_CHECK_VERSION("MsatNetCDF24"))
        return;

    if (GDALGetDriverByName("MsatNetCDF24") == NULL)
    {
        unique_ptr<GDALDriver> driver(new GDALDriver());
        driver->SetDescription("MsatNetCDF24");
        driver->SetMetadataItem(GDAL_DMD_LONGNAME, "Meteosatlib NetCDF24");
        //driver->SetMetadataItem(GDAL_DMD_HELPTOPIC, "frmt_various.html#JDEM");
        driver->SetMetadataItem(GDAL_DMD_EXTENSION, "nc24");
        driver->pfnOpen = msat::netcdf::NetCDF24Open;
        driver->pfnCreateCopy = msat::netcdf::NetCDF24CreateCopy;
        GetGDALDriverManager()->RegisterDriver(driver.release());
    }
}

}
