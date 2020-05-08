#include "grib.h"
#include "utils.h"
#include <msat/gdal/const.h>
#include <msat/gdal/dataset.h>
#include <msat/facts.h>
#include <gdal/gdal_priv.h>
#include <gdal/ogr_spatialref.h>
#include "gdal/utils.h"
#include <memory>

#include "config.h"

// FIXME: make it configurable?
#define GRIB_META_CENTRE 98

using namespace std;

namespace msat {
namespace grib {

struct GRIBDataset;

class GRIBRasterBand : public GDALRasterBand
{
	Grib& grib;
	double missing;
	string unit;

public:
	GRIBRasterBand(GRIBDataset* ds, int idx /*, GDALDataType dt */);

	virtual const char* GetUnitType()
	{
		return unit.c_str();
	}

        virtual double GetOffset(int* pbSuccess=NULL)
	{
		if (pbSuccess) *pbSuccess = TRUE;
		return 0;
	}
        virtual double GetScale(int* pbSuccess=NULL)
	{
		if (pbSuccess) *pbSuccess = TRUE;
		return 1;
	}
        virtual double GetNoDataValue(int* pbSuccess=NULL)
	{
		if (pbSuccess) *pbSuccess = TRUE;
		return missing;
	}

	virtual CPLErr IReadBlock(int xblock, int yblock, void *buf)
	{
		if (xblock != 0 || yblock != 0)
		{
			CPLError(CE_Failure, CPLE_AppDefined, "Invalid block number");
			return CE_Failure;
		}

		size_t length = nRasterXSize * nRasterYSize;
		grib.get_double_array("values", (double*)buf, &length);
		if (length != (size_t)(nRasterXSize * nRasterYSize)) {
			CPLError(CE_Failure, CPLE_AppDefined, "Only %d values read instead of %d", (int)length, nRasterXSize * nRasterYSize);
			return CE_Failure;
		}
		return CE_None;
	}
};

class GRIBDataset : public GDALDataset
{
public:
	Grib grib;
	int spacecraft_id;
	string projWKT;

    GRIBDataset(Grib&& grib) : grib(move(grib)) {}

	bool init()
	{
		try {
			nRasterXSize = grib.get_long("numberOfPointsAlongXAxis");
			nRasterYSize = grib.get_long("numberOfPointsAlongYAxis");
			nBands = 1;

			// Datetime
			char buf[25];
			grib.formatTime(buf);
			if (SetMetadataItem(MD_MSAT_DATETIME, buf, MD_DOMAIN_MSAT) != CE_None)
				return false;

			// Spacecraft
			spacecraft_id = (int)grib.get_long_oneof("satelliteNumber", "satelliteIdentifier", "indicatorOfTypeOfLevel", NULL);
			snprintf(buf, 25, "%d", spacecraft_id);
			if (SetMetadataItem(MD_MSAT_SPACECRAFT_ID, buf, MD_DOMAIN_MSAT) != CE_None)
				return false;
			if (SetMetadataItem(MD_MSAT_SPACECRAFT,
						facts::spacecraftName(spacecraft_id),
						MD_DOMAIN_MSAT) != CE_None)
				return false;


			// Projection
			double lop = grib.get_double_oneof("longitudeOfSubSatellitePointInDegrees", "geography.lop", NULL);
			projWKT = dataset::spaceviewWKT(lop);

			// long bpp = grib.get_long("numberOfBitsContainingEachPackedValue");
			// if (bpp <= 32)
				// SetBand(1, new GRIBRasterBand(this, 1, GDT_Float32));
			// else
			SetBand(1, new GRIBRasterBand(this, 1 /*, GDT_Float64 */));
			return true;
		} catch (griberror& e) {
			return false;
		}
	}

    virtual CPLErr GetGeoTransform(double* tr)
	{
		try {
			// Compute geotransform matrix
			long column_offset = grib.get_long_oneof("xCoordinateOfSubSatellitePoint", "XpInGridLengths", NULL);
			long line_offset = grib.get_long_oneof("yCoordinateOfSubSatellitePoint", "YpInGridLengths", NULL);
			long x0 = grib.get_long_oneof("Xo", "geography.xo", NULL);
			long y0 = grib.get_long_oneof("Yo", "geography.yo", NULL);
			double psx = facts::pixelHSizeFromSeviriDX(grib.get_long("geography.dx"));
			double psy = facts::pixelVSizeFromSeviriDY(grib.get_long("geography.dy"));

            // fprintf(stderr, "Xp %ld, Yp %ld, x0 %ld, psx %f\n", column_offset, line_offset, x0, psx);

			tr[0] = -(column_offset - x0) * psx;
			tr[3] = (line_offset   - y0) * psy;
			tr[1] = psx;
			tr[5] = -psy;
			tr[2] = 0.0;
			tr[4] = 0.0;
			return CE_None;
		} catch (griberror& e) {
			return CE_Failure;
		}
	}

        virtual const char* GetProjectionRef()
	{
		return projWKT.c_str();
	}
};

GRIBRasterBand::GRIBRasterBand(GRIBDataset* ds, int idx /*, GDALDataType dt */)
    : grib(ds->grib)
{
    poDS = ds;
    nBand = idx;
    eDataType = GDT_Float64;

    nBlockXSize = ds->GetRasterXSize();
    nBlockYSize = ds->GetRasterYSize();

    // Channel
    long channel_id;
    if (!grib.get_long_ifexists("channelNumber", &channel_id))
      if (!grib.get_long_ifexists("level", &channel_id))
        {
	  long cw_scale = grib.get_long("scaleFactorOfCentralWaveNumber");
	  long cw_val = grib.get_long("scaledValueOfCentralWaveNumber");
	  
	  double central_vave_number = (double)cw_val * exp10(-cw_scale);
	  channel_id = facts::channel_from_central_wave_number(ds->spacecraft_id, central_vave_number);
        }
    
    char buf[25];
    snprintf(buf, 25, "%ld", channel_id);
    SetMetadataItem(MD_MSAT_CHANNEL_ID, buf, MD_DOMAIN_MSAT);
    string channelName = facts::channelName(ds->spacecraft_id, channel_id);
    SetMetadataItem(MD_MSAT_CHANNEL, channelName.c_str(), MD_DOMAIN_MSAT);
    
    SetDescription(channelName.c_str());
    
    unit = facts::channelUnit(ds->spacecraft_id, channel_id);
    
    // TODO glb_preferredBPP = grib.get_long("numberOfBitsContainingEachPackedValue");
    // TODO // This is pointless, as grib won't store it
    
    // Invent a suitable missing value instead of 9999 or whatever grib_api
    // has as default
    missing = facts::defaultScaledMissing(channel_id);
    grib.set_double("missingValue", missing);
}

GDALDataset* GRIBOpen(GDALOpenInfo* info)
{
    // We want a real file
#if GDAL_VERSION_MAJOR >= 2
    if (info->fpL == NULL) return NULL;
#else
    if (info->fp == NULL) return NULL;
#endif

	if (info->nHeaderBytes < 4)
		return NULL;

	// Look for a grib signature in the header
	string header((const char*)info->pabyHeader, 0, info->nHeaderBytes);
	if (header.find("GRIB") == string::npos)
		return NULL;

	string filename(info->pszFilename);

    Grib grib;
    if (grib.new_from_file(NULL, info->pszFilename) != CE_None)
        return NULL;

    // Create the dataset
    unique_ptr<GRIBDataset> ds(new GRIBDataset(move(grib)));

    // Initialise the dataset
    if (!ds->init()) return NULL;

    return msat::gdal::add_extras(ds.release(), info);
}

#if 0
namespace {
template<typename NCObject, typename T>
static int ncfAddAttr(NCObject& ncf, const char* name, const T& val)
{
        if (!ncf.add_att(name, val))
        {
                std::stringstream msg;
                msg << "cannot add attribute '" << name << "' set to " << val;
		CPLError(CE_Failure, CPLE_AppDefined, msg.str().c_str());
                return FALSE;
        }
        return TRUE;
}

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
}
#endif

namespace {

struct CreateGRIB
{
    Grib& grib;
    GDALDataset* src;
    GDALRasterBand* rb;
    OGRSpatialReference osr;
    std::vector<double> values;
    size_t count_missing = 0;
    double grib_missing = 0;

    CreateGRIB(Grib& grib, GDALDataset* src)
        : grib(grib), src(src), rb(src->GetRasterBand(1)), osr(src->GetProjectionRef())
    {
    }

    virtual ~CreateGRIB() {}

    virtual bool create()
    {
        // Sanity checks
        const char *stype = osr.GetAttrValue("PROJECTION");
        if (!stype)
        {
            CPLError(CE_Failure, CPLE_AppDefined, "GRIB for satellites requires a " SRS_PT_GEOSTATIONARY_SATELLITE " projection, but we have %s", stype);
            return false;
        }
        if (osr.GetProjParm(SRS_PP_SATELLITE_HEIGHT) != ORBIT_RADIUS_FOR_GDAL)
        {
            CPLError(CE_Failure, CPLE_AppDefined, "we are given a satellite height of %f but only %d is supported", osr.GetProjParm(SRS_PP_SATELLITE_HEIGHT), ORBIT_RADIUS_FOR_GDAL);
            return false;
        }

        // Read values
        values.resize(src->GetRasterXSize() * src->GetRasterYSize());
        if (rb->RasterIO(GF_Read, 0, 0, src->GetRasterXSize(), src->GetRasterYSize(), values.data(), src->GetRasterXSize(), src->GetRasterYSize(), GDT_Float64, 0, 0) != CE_None)
            return false;

        // Scale values and count missing values
        double missing = rb->GetNoDataValue();
        double offset = rb->GetOffset();
        double scale = rb->GetScale();
        grib_missing = missing * scale + offset;
        for (auto& v: values)
        {
            if (v == missing)
            {
                ++count_missing;
                v = grib_missing;
            }
            else
                v = v * scale + offset;
        }
        return true;
    }

    virtual bool bit_map_section()
    {
        // The purpose of the Bit-Map Section is to indicate the presence or
        // absence of data at each of the grid points, as applicable, in the
        // next occurrence of the Data Section.
        if (count_missing)
        {
            grib.set_long("bitmapPresent", 1);
            // TODO: does it get computed automatically if we set missingValue,
            // or do we need to compute the bitmap ourselves?
            grib.set_double("missingValue", grib_missing);
        } else {
            // 255 = A bit map does not apply to this product (grib2/tables/4/6.0.table)
            grib.set_long("bitmapPresent", 0);
        }
        return true;
    }

    virtual bool data_section()
    {
        // TODO: if grib_api computes the bitmap for us, yeah. If it turns out
        // that it does not, then we have a bitmap to compute, and values
        // probably needs to be shrunk to skip the missing values.
        if (count_missing == values.size())
        {
            CPLError(CE_Failure, CPLE_AppDefined, "All values to encode are missing, and GRIB cannot handle this");
            return false;
        }

        grib.set_double_array("values", values.data(), values.size());
        return true;
    }

};

struct CreateGRIB2 : public CreateGRIB
{
    using CreateGRIB::CreateGRIB;

    virtual bool indicator_section()
    {
        grib.set_long("discipline", 3);
        return true;
    }

    virtual bool identification_section()
    {
        grib.set_long("centre", GRIB_META_CENTRE);
        grib.set_long("subCentre",0);
        // Code table 1.0: Version implemented on 7 November 2007
        grib.set_long("tablesVersion", 4);
        grib.set_long("localTablesVersion", 0); // Not used
        // Code table 1.2: Observation time
        grib.set_long("significanceOfReferenceTime", 3);
        const char* sval = src->GetMetadataItem(MD_MSAT_DATETIME, MD_DOMAIN_MSAT);
        if (sval != NULL)
            grib.setTime(sval);
        // Code table 1.3: Operational products
        grib.set_long("productionStatusOfProcessedData", 0);
        // Code table 1.4: Analysis products
        grib.set_long("typeOfProcessedData", 0);
        return true;
    }

    virtual bool grid_definition_section()
    {
        // Template 3.90: Space view perspective or orthographic
        // (see /usr/share/grib_api/definitions/grib2/template.3.90.def)

        double gt[6];
        src->GetGeoTransform(gt);
        if (gt[2] != 0.0)
        {
            CPLError(CE_Failure, CPLE_AppDefined, "3rd element of geotransform matrix is not zero");
            return false;
        }
        if (gt[4] != 0.0)
        {
            CPLError(CE_Failure, CPLE_AppDefined, "5th element of geotransform matrix is not zero");
            return false;
        }

        int column_offset;
        int line_offset;
        // Check if we are HRV or not
        //fprintf(stderr, "%f - %f = %f\n", padfTransform[1], METEOSAT_PIXELSIZE_X_HRV, fabs(padfTransform[1] - METEOSAT_PIXELSIZE_X_HRV));
        if (fabs(gt[1] - METEOSAT_PIXELSIZE_X_HRV) > 0.0001)
        {
            //fprintf(stderr, "IS NOT HRV\n");
            if (fabs(gt[1] - METEOSAT_PIXELSIZE_X) > 0.0001)
            {
                CPLError(CE_Failure, CPLE_AppDefined, "2nd element of geotransform matrix has an unexpected value (got: %f, expected: %f)", gt[1], METEOSAT_PIXELSIZE_X);
                return false;
            }
            if (fabs(gt[5] - -METEOSAT_PIXELSIZE_Y) > 0.0001)
            {
                CPLError(CE_Failure, CPLE_AppDefined, "6th element of geotransform matrix has an unexpected value (got: %f, expected: %f)", gt[5], METEOSAT_PIXELSIZE_Y);
                return false;
            }

            column_offset = METEOSAT_IMAGE_NCOLUMNS / 2;
            line_offset = METEOSAT_IMAGE_NLINES / 2;
            //dx = Band::seviriDXFromCFAC(13642337);
            //dy = Band::seviriDYFromLFAC(13642337);
        } else {
            //fprintf(stderr, "IS HRV\n");
            if (fabs(gt[5] - -METEOSAT_PIXELSIZE_Y_HRV) > 0.0001)
            {
                CPLError(CE_Failure, CPLE_AppDefined, "6th element of geotransform matrix has an unexpected value (got: %f, expected: %f)", gt[5], METEOSAT_PIXELSIZE_Y_HRV);
                return false;
            }

            column_offset = METEOSAT_IMAGE_NCOLUMNS_HRV / 2;
            line_offset = METEOSAT_IMAGE_NLINES_HRV / 2;
            //dx = Band::seviriDXFromCFAC(40927000);
            //dy = Band::seviriDYFromLFAC(40927000);
        }

        // GRIB table 3.0 (see http://www.wmo.int/pages/prog/www/WMOCodes/WMO306_vI2/LatestVERSION/WMO306_vI2_GRIB2_CodeFlag_en.pdf)
        grib.set_long("sourceOfGridDefinition", 0);
        grib.set_long("numberOfDataPoints", values.size());
        // Code table 3.11: "There is no appended list"
        grib.set_long("interpretationOfNumberOfPoints", 0);
        // Template 3.90: "Space view perspective or orthographic"
        grib.set_long("gridDefinitionTemplateNumber", 90);

        // Code table 3.2: "Earth assumed oblate spheroid with size as determined by IAU in 1965 (major axis = 6,378,160.0 m, minor axis = 6,356,775.0 m, f = 1/297.0)"
        grib.set_long("shapeOfTheEarth", 2);

        grib.set_long("numberOfPointsAlongXAxis", src->GetRasterXSize());
        grib.set_long("numberOfPointsAlongYAxis", src->GetRasterYSize());

        double sublon = (double)osr.GetProjParm(SRS_PP_CENTRAL_MERIDIAN, 0.0);
        grib.set_long("latitudeOfSubSatellitePointInDegrees", 0);
        grib.set_long("longitudeOfSubSatellitePointInDegrees", (long)nearbyint(sublon * 1000));

        int dx = facts::seviriDXFromPixelHSize(gt[1]);
        int dy = facts::seviriDYFromPixelVSize(-gt[5]);
        grib.set_long("geography.dx", dx);
        grib.set_long("geography.dy", dy);

        int x0 = nearbyint(column_offset + gt[0] / gt[1]);
        int y0 = nearbyint(line_offset - gt[3] / -gt[5]);
        //fprintf(stderr, "xo %d yo %d\n", x0, y0);
        //fprintf(stderr, "xo %d co %d t0 %f t1 %f\n", x0, column_offset, padfTransform[0], padfTransform[1]);
        grib.set_long_oneof("geography.xo", x0,
                "Xo", x0,
                NULL);
        grib.set_long_oneof("geography.yo", y0,
                "Yo", y0,
                NULL);

        grib.set_long("geography.dx", dx);
        grib.set_long("geography.dy", dy);

        grib.set_long("xCoordinateOfSubSatellitePoint", column_offset);
        grib.set_long("yCoordinateOfSubSatellitePoint", line_offset);

        /* 0 = 00000000
           (1=0)  Points scan in +i direction
           (2=0)  Points scan in -j direction
           (3=0)  Adjacent points in i direction are consecutive 
           See grib1/8.table */
        grib.set_long("scanningMode",0);

        grib.set_long("orientationOfTheGridInDegrees", (long)nearbyint(SEVIRI_ORIENTATION));

        // Nr - altitude of the camera from the Earth's centre, measured in
        // units of the Earth's (equatorial)
        grib.set_long("Nr", (long)nearbyint(SEVIRI_CAMERA_H * 1000000));

        grib.set_long("Xo", x0);
        grib.set_long("Yo", y0);

        return true;
    }

    virtual bool product_definition_section()
    {
        // See http://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_temp4-31.shtml
        // See http://www.cosmo-model.org/content/model/documentation/grib/pdtemplate_4.32.htm
        grib.set_long("productDefinitionTemplateNumber", 31); // Image format products

        // Template.4.31
        // see /usr/share/grib_api/definitions/grib2/template.4.31.def

        // Table 4.1.3: Space Products
        // see /usr/share/grib_api/definitions/grib2/tables/4/4.1.3.table
        grib.set_long("parameterCategory", 0); // Image format products

        // Table 4.2.3.0: Space products, Image format products
        // see /usr/share/grib_api/definitions/grib2/tables/4/4.2.3.0.table
        // TODO: pick from this list according to channel:
        // 0 Scaled radiance (Numeric)
        // 1 Scaled albedo (Numeric)
        // 2 Scaled brightness temperature (Numeric)
        // 3 Scaled precipitable water (Numeric)
        // 4 Scaled lifted index (Numeric)
        // 5 Scaled cloud top pressure (Numeric)
        // 6 Scaled skin temperature (Numeric)
        // 7 Cloud mask (Code table 4.217)
        // 8 Pixel scene type (Code table 4.218)
        grib.set_long("parameterNumber", 0);

        // Table 4.3: Analysis
        // see /usr/share/grib_api/definitions/grib2/tables/4/4.3.table
        grib.set_long("typeOfGeneratingProcess", 0);

        // Observation generating process identifier (defined by originating centre)
        // TODO: this is just cargo-culted by the previous GRIB1 implementation and needs review
        grib.set_long("generatingProcessIdentifier",254);

        // Number of contributing spectral bands
        grib.set_long("NB", 1);

        // Spacecraft
        const char* sval = src->GetMetadataItem(MD_MSAT_SPACECRAFT_ID, MD_DOMAIN_MSAT);
        if (sval != NULL)
        {
            long spacecraft_id = strtoul(sval, NULL, 10);

            // BUFR code table 002020: METEOSAT Second Generation Programme (MSG)
            grib.set_long("satelliteSeries", 333);
            // BUFR code table 001007
            grib.set_long("satelliteNumber", spacecraft_id);
            // COMMON CODE TABLE C–8: Satellite instruments
            grib.set_long("instrumentType", 207); // Seviri

            // Central wave number should tell us which channel was used
            sval = rb->GetMetadataItem(MD_MSAT_CHANNEL_ID, MD_DOMAIN_MSAT);
            if (sval != NULL)
            {
                long channel_id = strtoul(sval, NULL, 10);
                grib.set_long("scaleFactorOfCentralWaveNumber", 0);
                grib.set_long("scaledValueOfCentralWaveNumber", (long)round(msat::facts::channel_central_wave_number(spacecraft_id, channel_id)));
            }
        }
        return true;
    }

    virtual bool data_representation_section()
    {
        // see /usr/share/grib_api/definitions/grib2/section.5.def
        grib.set_long("numberOfValues", src->GetRasterXSize() * src->GetRasterYSize() - count_missing);
        grib.set_long("dataRepresentationTemplateNumber", 0);
        // see /usr/share/grib_api/definitions/grib2/template.5.0.def
        // see /usr/share/grib_api/definitions/grib2/template.5.packing.def
        // see /usr/share/grib_api/definitions/grib2/template.5.original_values.def
        // TODO: is it autocomputed? should we compute it?
        return true;
    }

    bool create() override
    {
        grib.new_from_samples(NULL, "GRIB2");

        if (!CreateGRIB::create())
            return false;

        if (!indicator_section()) return false;
        if (!identification_section()) return false;
        if (!grid_definition_section()) return false;
        if (!product_definition_section()) return false;
        if (!data_representation_section()) return false;
        if (!bit_map_section()) return false;
        if (!data_section()) return false;

        return true;
    }
};

struct CreateGribWMO : public CreateGRIB2
{
    using CreateGRIB2::CreateGRIB2;
};

struct CreateGRIB1 : public CreateGRIB
{
    using CreateGRIB::CreateGRIB;

    virtual bool identification_section()
    {
        grib.set_long("centre", GRIB_META_CENTRE);
        grib.set_long("generatingProcessIdentifier",254);
        grib.set_long("gridDefinition",255);
        /* 127 = None Image data - (grib1/2.wmo.1.table)  */
        grib.set_long("indicatorOfParameter",127);
        // Spacecraft
        const char* sval = src->GetMetadataItem(MD_MSAT_SPACECRAFT_ID, MD_DOMAIN_MSAT);
        if (sval != NULL)
        {
            long spacecraft_id = strtoul(sval, NULL, 10);
            grib.set_long("indicatorOfTypeOfLevel", spacecraft_id);
        }
        sval = rb->GetMetadataItem(MD_MSAT_CHANNEL_ID, MD_DOMAIN_MSAT);
        if (sval != NULL)
        {
            long channel_id = strtoul(sval, NULL, 10);
            // grib.set_long_unchecked("channelNumber", channel_id);
            grib.set_long("level", channel_id);
        }
        // Code table 1.0: Version implemented on 7 November 2007
        //grib.set_long("tablesVersion", 4);
        //grib.set_long("localTablesVersion", 0); // Not used
        // Code table 1.2: Observation time
        //grib.set_long("significanceOfReferenceTime", 3);
        sval = src->GetMetadataItem(MD_MSAT_DATETIME, MD_DOMAIN_MSAT);
        if (sval != NULL)
            grib.setTime(sval);
        /* 0 = Minute (grib1/4.table)  */
        grib.set_long("indicatorOfUnitOfTimeRange",0);
        /* 0 = Forecast product valid at reference time + P1  (P1>0)  (grib1/5.table)  */
        grib.set_long("timeRangeIndicator",0);
        grib.set_long("numberIncludedInAverage",0);
        grib.set_long("numberMissingFromAveragesOrAccumulations",0);
        grib.set_long("subCentre",0);
        return true;
    }

    virtual bool grid_definition_section()
    {
        // Template 3.90: Space view perspective or orthographic
        // (see /usr/share/grib_api/definitions/grib2/template.3.90.def)

        double gt[6];
        src->GetGeoTransform(gt);
        if (gt[2] != 0.0)
        {
            CPLError(CE_Failure, CPLE_AppDefined, "3rd element of geotransform matrix is not zero");
            return false;
        }
        if (gt[4] != 0.0)
        {
            CPLError(CE_Failure, CPLE_AppDefined, "5th element of geotransform matrix is not zero");
            return false;
        }

        int column_offset;
        int line_offset;
        // Check if we are HRV or not
        //fprintf(stderr, "%f - %f = %f\n", padfTransform[1], METEOSAT_PIXELSIZE_X_HRV, fabs(padfTransform[1] - METEOSAT_PIXELSIZE_X_HRV));
        if (fabs(gt[1] - METEOSAT_PIXELSIZE_X_HRV) > 0.0001)
        {
            //fprintf(stderr, "IS NOT HRV\n");
            if (fabs(gt[1] - METEOSAT_PIXELSIZE_X) > 0.0001)
            {
                CPLError(CE_Failure, CPLE_AppDefined, "2nd element of geotransform matrix has an unexpected value (got: %f, expected: %f)", gt[1], METEOSAT_PIXELSIZE_X);
                return false;
            }
            if (fabs(gt[5] - -METEOSAT_PIXELSIZE_Y) > 0.0001)
            {
                CPLError(CE_Failure, CPLE_AppDefined, "6th element of geotransform matrix has an unexpected value (got: %f, expected: %f)", gt[5], METEOSAT_PIXELSIZE_Y);
                return false;
            }

            column_offset = METEOSAT_IMAGE_NCOLUMNS / 2;
            line_offset = METEOSAT_IMAGE_NLINES / 2;
            //dx = Band::seviriDXFromCFAC(13642337);
            //dy = Band::seviriDYFromLFAC(13642337);
        } else {
            //fprintf(stderr, "IS HRV\n");
            if (fabs(gt[5] - -METEOSAT_PIXELSIZE_Y_HRV) > 0.0001)
            {
                CPLError(CE_Failure, CPLE_AppDefined, "6th element of geotransform matrix has an unexpected value (got: %f, expected: %f)", gt[5], METEOSAT_PIXELSIZE_Y_HRV);
                return false;
            }

            column_offset = METEOSAT_IMAGE_NCOLUMNS_HRV / 2;
            line_offset = METEOSAT_IMAGE_NLINES_HRV / 2;
            //dx = Band::seviriDXFromCFAC(40927000);
            //dy = Band::seviriDYFromLFAC(40927000);
        }

        grib.set_long("numberOfVerticalCoordinateValues",0);
        grib.set_long("pvlLocation",255);

        // GRIB table 3.0 (see http://www.wmo.int/pages/prog/www/WMOCodes/WMO306_vI2/LatestVERSION/WMO306_vI2_GRIB2_CodeFlag_en.pdf)
        // Template 3.90: "Space view perspective or orthographic"
        grib.set_long("dataRepresentationType", 90);

        grib.set_long("numberOfPointsAlongXAxis", src->GetRasterXSize());
        grib.set_long("numberOfPointsAlongYAxis", src->GetRasterYSize());

        double sublon = (double)osr.GetProjParm(SRS_PP_CENTRAL_MERIDIAN, 0.0);
        grib.set_long("latitudeOfSubSatellitePointInDegrees", 0);
        grib.set_long("longitudeOfSubSatellitePointInDegrees", (long)nearbyint(sublon * 1000));

        /* 64 = 01000000
           (1=0)  Direction increments not given
           (2=1)  Earth assumed oblate spheroid with size as determined by IAU in 1965
           See  6378.160 km, 6356.775 km, f = 1/297.0
           (5=0)  u and v components resolved relative to easterly and northerly directions
           See grib1/7.table */
        grib.set_long("resolutionAndComponentFlags",64);

        int dx = facts::seviriDXFromPixelHSize(gt[1]);
        int dy = facts::seviriDYFromPixelVSize(-gt[5]);
        grib.set_long("geography.dx", dx);
        grib.set_long("geography.dy", dy);

        grib.set_long("XpInGridLengths", column_offset);
        grib.set_long("YpInGridLengths", line_offset);

        /* 0 = 00000000
           (1=0)  Points scan in +i direction
           (2=0)  Points scan in -j direction
           (3=0)  Adjacent points in i direction are consecutive 
           See grib1/8.table */
        grib.set_long("scanningMode",0);

        grib.set_long("orientationOfTheGridInDegrees", (long)nearbyint(SEVIRI_ORIENTATION));

        // Nr - altitude of the camera from the Earth's centre, measured in
        // units of the Earth's (equatorial)
        grib.set_long("altitudeOfTheCameraFromTheEarthSCenterMeasuredInUnitsOfTheEarth", (long)nearbyint(SEVIRI_CAMERA_H * 1000000));

        int x0 = nearbyint(column_offset + gt[0] / gt[1]);
        int y0 = nearbyint(line_offset - gt[3] / -gt[5]);
        //fprintf(stderr, "xo %d yo %d\n", x0, y0);
        //fprintf(stderr, "xo %d co %d t0 %f t1 %f\n", x0, column_offset, padfTransform[0], padfTransform[1]);
        grib.set_long("Xo", x0);
        grib.set_long("Yo", y0);

        return true;
    }

    bool create() override
    {
        grib.new_from_samples(NULL, "GRIB1");

        if (!CreateGRIB::create())
            return false;

        if (!identification_section()) return false;
        if (!grid_definition_section()) return false;
        if (!bit_map_section()) return false;
        if (!data_section()) return false;

        return true;
    }
};

struct CreateGribECMWF : public CreateGRIB1
{
    using CreateGRIB1::CreateGRIB1;

    bool identification_section() override
    {
        CreateGRIB1::identification_section();

        /* 24 = Satellite image simulation (grib1/localDefinitionNumber.98.table)  */
        grib.set_long("localDefinitionNumber", 24);

        // Spacecraft
        const char* sval = src->GetMetadataItem(MD_MSAT_SPACECRAFT_ID, MD_DOMAIN_MSAT);
        if (sval != NULL)
        {
            long spacecraft_id = strtoul(sval, NULL, 10);
            grib.set_long("satelliteIdentifier", spacecraft_id);
            grib.set_long("instrumentIdentifier", 207);
        }
        sval = rb->GetMetadataItem(MD_MSAT_CHANNEL_ID, MD_DOMAIN_MSAT);
        if (sval != NULL)
        {
            long channel_id = strtoul(sval, NULL, 10);
            grib.set_long("channelNumber", channel_id);
        }
        grib.set_long("functionCode",1);
        return true;
    }
};

struct CreateGribMsat : public CreateGRIB1
{
    using CreateGRIB1::CreateGRIB1;
};


}

GDALDataset* GRIBCreateCopy(const char* pszFilename, GDALDataset* src,
                              int bStrict, char** papszOptions,
                              GDALProgressFunc pfnProgress, void* pProgressData)
{
    try {
        const char* templateName = CSLFetchNameValue(papszOptions, "TEMPLATE");
        if (templateName == NULL)
            templateName = "msat/wmo";

        Grib grib;
        if (strcmp(templateName, "msat/wmo") == 0)
        {
            CreateGribWMO c(grib, src);
            if (!c.create()) return nullptr;
        } else if (strcmp(templateName, "msat/ecmwf") == 0) {
            CreateGribECMWF c(grib, src);
            if (!c.create()) return nullptr;
        } else if (strcmp(templateName, "msat/msat") == 0) {
            CreateGribMsat c(grib, src);
            if (!c.create()) return nullptr;
        }
        else
        {
            CPLError(CE_Failure, CPLE_AppDefined, "Unsupported template name '%s'", templateName);
            return nullptr;
        }

        grib.write(pszFilename);

        return (GDALDataset*)GDALOpen(pszFilename, GA_ReadOnly);
    } catch (griberror& e) {
        return nullptr;
    }
}

}
}

extern "C" {

void GDALRegister_MsatGRIB()
{
    if (!GDAL_CHECK_VERSION("MsatGRIB"))
        return;

    if (GDALGetDriverByName("MsatGRIB") == NULL)
    {
        unique_ptr<GDALDriver> driver(new GDALDriver());
        driver->SetDescription("MsatGRIB");
        driver->SetMetadataItem(GDAL_DMD_LONGNAME, "Meteosatlib GRIB via grib_api");
        //driver->SetMetadataItem(GDAL_DMD_HELPTOPIC, "frmt_various.html#JDEM");
        driver->SetMetadataItem(GDAL_DMD_EXTENSION, "grib");
        driver->pfnOpen = msat::grib::GRIBOpen;
        driver->pfnCreateCopy = msat::grib::GRIBCreateCopy;
        GetGDALDriverManager()->RegisterDriver(driver.release());
    }
}

}
