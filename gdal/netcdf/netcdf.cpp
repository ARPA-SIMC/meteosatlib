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

class NetCDFDataset : public GDALDataset
{
public:
        NcFile* nc;
        string projWKT;
        int spacecraft_id;

        NetCDFDataset(NcFile* nc) : nc(nc) {}
        ~NetCDFDataset()
        {
                if (nc != NULL) delete nc;
        }
        virtual bool init();

        virtual const char* GetProjectionRef()
        {
                return projWKT.c_str();
        }

        virtual CPLErr GetGeoTransform(double* tr)
        {
                NcError nce(NcError::silent_nonfatal);
                NcFile& ncf = *nc;

                // Compute geotransform matrix
                NcAtt* asp = ncf.get_att("AreaStartPix");
                NcAtt* asl = ncf.get_att("AreaStartLin");
                if (asp && asl)
                {
                        const int column_offset = getAttr<int>(ncf, "Column_Offset", 1856);
                        const int line_offset = getAttr<int>(ncf, "Line_Offset", 1856);
                        const int x0 = getAttr<int>(*asp) - 1;
                        const int y0 = getAttr<int>(*asl) - 1;
                        const double psx = facts::pixelHSizeFromCFAC(abs(getAttr<int>(ncf, "Column_Scale_Factor", -13642337)) * exp2(-16));
                        const double psy = facts::pixelVSizeFromLFAC(abs(getAttr<int>(ncf, "Line_Scale_Factor", -13642337)) * exp2(-16));

                        // Only valid for non-HRV
                        tr[0] = -(column_offset - x0) * psx;
                        tr[3] = (line_offset   - y0) * psy;
                        tr[1] = psx;
                        tr[5] = -psy;
                        tr[2] = 0.0;
                        tr[4] = 0.0;
                        return CE_None;
                } else {
                        CPLError(CE_Failure, CPLE_AppDefined, "Cannot find AreaStartPix and AreaStartLin in NetCDF file");
                        return CE_Failure;
                }
        }
};

class MsatNetCDFRasterBand : public NetCDFRasterBand
{
public:
        bool offset1bug;

        MsatNetCDFRasterBand(NetCDFDataset* ds, int idx, NcVar* var) : NetCDFRasterBand(ds, idx, var), offset1bug(false)
        {
                /// Channel
                NcAtt* a = var->get_att("chnum");
                if (a != NULL)
                {
                        channel_id = getAttr<int>(*a);

                        // Channel
                        char buf[25];
                        snprintf(buf, 25, "%d", channel_id);
                        SetMetadataItem(MD_MSAT_CHANNEL_ID, buf, MD_DOMAIN_MSAT);
                        SetMetadataItem(MD_MSAT_CHANNEL,
                                        facts::channelName(ds->spacecraft_id, channel_id),
                                        MD_DOMAIN_MSAT);
                } else
                        channel_id = 0;

                // Detect the old encoding bug that set offset to 1 instead of 0
                string version = getAttr<const char*>(*ds->nc, "Version", "0");
                offset1bug = (version == "0");
        }

        virtual double GetOffset(int* pbSuccess=NULL)
        {
                double res = NetCDFRasterBand::GetOffset(pbSuccess);
                if (offset1bug && res == 1.0)
                        return 0.0;
                else
                        return res;
        }
};

bool NetCDFDataset::init()
{
        NcFile& ncf = *nc;
        float ftmp;
        std::string stmp;
        char buf[25];

        // Sanity checks
        ftmp = getAttr<float>(ncf, "SampleX", 1.0);
        if (ftmp != 1.0)
        {
                CPLError(CE_Failure, CPLE_AppDefined, "SampleX should have been 1.0 but is %f instead", ftmp);
                return NULL;
        }
        ftmp = getAttr<float>(ncf, "SampleY", 1.0);
        if (ftmp != 1.0)
        {
                CPLError(CE_Failure, CPLE_AppDefined, "SampleY should have been 1.0 but is %f instead", ftmp);
                return NULL;
        }
        ftmp = getAttr<float>(ncf, "Orbit_Radius", ORBIT_RADIUS);
        if (ftmp != ORBIT_RADIUS)
                CPLError(CE_Warning, CPLE_AppDefined, "Orbit_Radius should have been %f but is %f instead: ignoring it.", ORBIT_RADIUS, ftmp);

#if 0
        if (GETDEF(int, "Columns", 3712) != 3712)
                cerr << "Columns should have been 3712 but is " << GETDEF(int, "Columns", 3712) << " instead: ignoring it" << endl;
        if (GETDEF(int, "Lines", 3712) != 3712)
                cerr << "Lines should have been 3712 but is " << GETDEF(int, "Lines", 3712) << " instead: ignoring it" << endl;
#endif


        /// Spacecraft
        stmp = getAttr<const char*>(ncf, "Satellite", "");
        if (!stmp.empty())
        {
                spacecraft_id = facts::spacecraftID(stmp);
                snprintf(buf, 25, "%d", spacecraft_id);
                if (SetMetadataItem(MD_MSAT_SPACECRAFT_ID, buf, MD_DOMAIN_MSAT) != CE_None)
                        return false;
                if (SetMetadataItem(MD_MSAT_SPACECRAFT, stmp.c_str(), MD_DOMAIN_MSAT) != CE_None)
                        return false;
        } else
                spacecraft_id = 0;


        /// Image time
        stmp = getAttr<const char*>(ncf, "Time", "0000-00-00 00:00:00 UTC");
        int year, month, day, hour, min, sec;
        if (sscanf(stmp.c_str(), "%04d-%02d-%02d %02d:%02d:%02d UTC",
                                &year, &month, &day, &hour, &min, &sec) != 6)
        {
                CPLError(CE_Failure, CPLE_AppDefined, "could not parse Time attribute \"%s\"", stmp.c_str());
                return false;
        }
        snprintf(buf, 20, "%04d-%02d-%02d %02d:%02d:%02d", year, month, day, hour, min, sec);
        if (SetMetadataItem(MD_MSAT_DATETIME, buf, MD_DOMAIN_MSAT) != CE_None)
                return false;


        /// Projection
        ftmp = getAttr<float>(ncf, "Longitude", 0);
        projWKT = dataset::spaceviewWKT(ftmp);


#if 0
        /// History
        if (NcAtt* a = nc->get_att("title"))
                gld_history = a->as_string(0);
        else
                gld_history.clear();
        // ds->setQualityFromPathname(filename);
#endif

        int nextBand = 1;
        for (int i = 0; i < ncf.num_vars(); ++i)
        {
                NcVar* var = ncf.get_var(i);
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

                SetBand(nextBand, new MsatNetCDFRasterBand(this, nextBand, var));
                ++nextBand;
        }

        return true;
}

GDALDataset* NetCDFOpen(GDALOpenInfo* info)
{
    // We want a real file
#if GDAL_VERSION_MAJOR == 2
    if (info->fpL == NULL) return NULL;
#else
    if (info->fp == NULL) return NULL;
#endif

    NcError nce(NcError::silent_nonfatal);

    // Try opening and seeing if it is a NetCDF file
    unique_ptr<NcFile> nc;
    try {
        nc.reset(new NcFile(info->pszFilename, NcFile::ReadOnly));
        if (!nc->is_valid()) return NULL;
        if (nc->get_att("Satellite") == NULL)
            return NULL;
    } catch (...) {
        return NULL;
    }

    // Create the dataset
    unique_ptr<NetCDFDataset> ds(new NetCDFDataset(nc.release()));

    // Initialise the generic dataset bits using information from the
    // NetCDF data
    if (!ds->init()) return NULL;

    return msat::gdal::add_extras(ds.release(), info);
}

GDALDataset* NetCDFCreateCopy(const char* pszFilename, GDALDataset* src, 
                              int bStrict, char** papszOptions, 
                              GDALProgressFunc pfnProgress, void* pProgressData)
{
        NcError nce(NcError::silent_nonfatal);

        // Build up output NetCDF file name and open it
        NcFile ncf(pszFilename, NcFile::Replace);
        if (!ncf.is_valid())
        {
                CPLError(CE_Failure, CPLE_AppDefined, "Cannot create NetCDF file %s: %s", pszFilename, nce.get_errmsg());
                return NULL;
        }

        // Fill arrays on creation
        ncf.set_fill(NcFile::Fill);

        //
        // Add Global Attributes
        //

        // Spacecraft name
        const char* sval = src->GetMetadataItem(MD_MSAT_SPACECRAFT, MD_DOMAIN_MSAT);
        if (sval != NULL)
                if (!ncfAddAttr(ncf, "Satellite", sval)) return NULL;

        if (!ncfAddAttr(ncf, "Antenna", "Fixed")) return NULL;
        if (!ncfAddAttr(ncf, "Receiver", "HIMET")) return NULL;

        // Datetime
        time_t fs2000 = 0;
        sval = src->GetMetadataItem(MD_MSAT_DATETIME, MD_DOMAIN_MSAT);
        if (sval != NULL)
        {
                fs2000 = forecastSeconds2000(sval);
                string tmp = sval;
                tmp += " UTC";
                if (!ncfAddAttr(ncf, "Time", tmp.c_str())) return NULL;
        }

        // Projection
        OGRSpatialReference osr(src->GetProjectionRef());
	string projection;
	if (const char* proj = osr.GetAttrValue("PROJECTION"))
		projection = proj;
        if (projection.empty())
        {
		// Uncropped image goes from (90,-180) to (-90, 180)
		// Image goes from (90,-180)+(x0/column_offset, y0/line_offset)
		// Image goes to (90,-180)+((x0+columns)/column_offset, (y0+lines)/line_offset)
		// Steps are 1/column_offset and 1/line_offset

		//xsubsat/column_offset = lrint(-gt[0] / gt[1]) / column_offset;
		//ysubsat/line_offset = lrint(-gt[3] / gt[5]) / line_offset;
		
		// nRasterXSize * nRasterYSize
		dataset::GeoReferencer gr;
		if (gr.init(src)) return NULL;
		int sx = src->GetRasterXSize();
		int sy = src->GetRasterYSize();
		float lats[sy];
		float lons[sx];
		for (int y = 0; y < sy; ++y)
		{
			double lat, lon;
			if (gr.pixelToLatlon(0, y, lat, lon) != CE_None) return NULL;
			lats[y] = lat;
		}
		for (int x = 0; x < sx; ++x)
		{
			double lat, lon;
			if (gr.pixelToLatlon(x, 0, lat, lon) != CE_None) return NULL;
			lons[x] = lon;
		}

		NcDim* xdim = ncf.add_dim("longitude", sx);
		if (!xdim->is_valid()) throw std::runtime_error("adding longitude dimension failed");
		NcDim* ydim = ncf.add_dim("latitude", sy);
		if (!ydim->is_valid()) throw std::runtime_error("adding latitude dimension failed");

		NcVar* xvar = ncf.add_var("longitude", ncFloat, xdim);
		if (!xvar->is_valid()) throw std::runtime_error("adding lon variable failed");
		ncfAddAttr(*xvar, "long_name", "longitude");
		ncfAddAttr(*xvar, "units", "degrees_east");
		ncfAddAttr(*xvar, "modulo", 360.0);
		ncfAddAttr(*xvar, "topology", "circular");
		if (!xvar->put(lons, sx))
			throw std::runtime_error("writing longitude values failed");

		NcVar* yvar = ncf.add_var("latitude", ncFloat, ydim);
		if (!yvar->is_valid()) throw std::runtime_error("adding lat variable failed");
		ncfAddAttr(*yvar, "long_name", "latitude");
		ncfAddAttr(*yvar, "units", "degrees_north");
		if (!yvar->put(lats, sy))
			throw std::runtime_error("writing latitude values failed");
        }
        else if (projection == SRS_PT_GEOSTATIONARY_SATELLITE)
        {
                // Sanity checks, and computation of target values
                if (osr.GetProjParm(SRS_PP_SATELLITE_HEIGHT) != ORBIT_RADIUS_FOR_GDAL)
                {
                        CPLError(CE_Failure, CPLE_AppDefined, "Satellite height is '%f' but only '%d' is supported",
                                        osr.GetProjParm(SRS_PP_SATELLITE_HEIGHT), ORBIT_RADIUS_FOR_GDAL);
                        return NULL;
                }
                float sublon = (float)osr.GetProjParm(SRS_PP_CENTRAL_MERIDIAN, 0.0);

                double geotransform[6];
                if (src->GetGeoTransform(geotransform) != CE_None)
                {
                        CPLError(CE_Failure, CPLE_AppDefined, "Cannot read geotransform matrix");
                        return NULL;
                }

                if (geotransform[2] != 0.0)
                {
                        CPLError(CE_Failure, CPLE_AppDefined, "3rd element of geotransform matrix is not zero");
                        return NULL;
                }
                if (geotransform[4] != 0.0)
                {
                        CPLError(CE_Failure, CPLE_AppDefined, "5rd element of geotransform matrix is not zero");
                        return NULL;
                }
                #if 0
                if (fabs(padfTransform[1] - METEOSAT_PIXELSIZE_X) > 0.00001)
                        throw std::runtime_error("2nd element of geotransform matrix has an unexpected value");
                if (fabs(padfTransform[5] - -METEOSAT_PIXELSIZE_Y) > 0.00001)
                        throw std::runtime_error("6th element of geotransform matrix has an unexpected value");
                #endif

                // Set geotransform parameters in NetCDF
                int column_offset = 1856;
                int line_offset = 1856;
                double psx = geotransform[1];
                double psy = -geotransform[5];
                // FIXME: anything better?
                if (psx - METEOSAT_PIXELSIZE_X_HRV < 0.0001)
                        column_offset = 3712;
                if (psy - METEOSAT_PIXELSIZE_Y_HRV < 0.0001)
                        line_offset = 3712;
                int x0 = nearbyint(column_offset + geotransform[0] / psx);
                int y0 = nearbyint(line_offset - geotransform[3] / psy);
                double column_res = facts::CFACFromPixelHSize(psx) * exp2(16);
                double line_res = facts::LFACFromPixelVSize(psy) * exp2(16);


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
        } else {
		CPLError(CE_Failure, CPLE_AppDefined, "Cannot export to projection %s", projection.c_str());
		return NULL;
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

                sval = rb->GetMetadataItem(MD_MSAT_CHANNEL_ID, MD_DOMAIN_MSAT);
                if (sval != NULL)
                        if (!ncfAddAttr(*ivar, "chnum", (int)strtoul(sval, NULL, 10))) return NULL;
        }

        // Close NetCDF output
        (void) ncf.close( );

        //  cout << "Wrote file " << NcName << "." << endl;

        return (GDALDataset*)GDALOpen(pszFilename, GA_ReadOnly);
}

}
}

extern "C" {

void GDALRegister_MsatNetCDF()
{
    if (!GDAL_CHECK_VERSION("MsatNetCDF"))
        return;

    if (GDALGetDriverByName("MsatNetCDF") == NULL)
    {
        unique_ptr<GDALDriver> driver(new GDALDriver());
        driver->SetDescription("MsatNetCDF");
        driver->SetMetadataItem(GDAL_DMD_LONGNAME, "Meteosatlib NetCDF");
        //driver->SetMetadataItem(GDAL_DMD_HELPTOPIC, "frmt_various.html#JDEM");
        driver->SetMetadataItem(GDAL_DMD_EXTENSION, "nc");
        driver->pfnOpen = msat::netcdf::NetCDFOpen;
        driver->pfnCreateCopy = msat::netcdf::NetCDFCreateCopy;
        GetGDALDriverManager()->RegisterDriver(driver.release());
    }
}

}
