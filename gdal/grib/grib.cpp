#include "grib.h"
#include "utils.h"
#include <msat/gdal/const.h>
#include <msat/gdal/dataset.h>
#include <msat/facts.h>
#include <gdal/gdal_priv.h>
#include <gdal/ogr_spatialref.h>
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
			spacecraft_id = (int)grib.get_long_oneof("satelliteIdentifier", "indicatorOfTypeOfLevel", NULL);
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
			long column_offset = grib.get_long_oneof("XpInGridLengths", "geography.Xp", NULL);
			long line_offset = grib.get_long_oneof("YpInGridLengths", "geography.Yp", NULL);
			long x0 = grib.get_long_oneof("Xo", "geography.xo", NULL);
			long y0 = grib.get_long_oneof("Yo", "geography.yo", NULL);
			double psx = facts::pixelHSizeFromSeviriDX(grib.get_long("geography.dx"));
			double psy = facts::pixelVSizeFromSeviriDY(grib.get_long("geography.dy"));

			//fprintf(stderr, "COFF %ld, x0 %ld, psx %f\n", column_offset, x0, psx);

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
	long channel_id = grib.get_long_oneof("channelNumber", "level", NULL);
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
#if GDAL_VERSION_MAJOR == 2
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

    return ds.release();
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

GDALDataset* GRIBCreateCopy(const char* pszFilename, GDALDataset* src, 
                              int bStrict, char** papszOptions, 
                              GDALProgressFunc pfnProgress, void* pProgressData)
{
    try {
        const char* templateName = CSLFetchNameValue(papszOptions, "TEMPLATE");
        if (templateName == NULL)
            templateName = "msat/ecmwf";

        Grib grib;
        grib.new_from_samples(NULL, "GRIB2");
        if (strcmp(templateName, "msat/wmo") == 0)
        {
            // http://www.cosmo-model.org/content/model/documentation/grib/pdtemplate_4.32.htm
            grib.set_long("tablesVersion", 4);

            grib.set_long("NV", 0);
            grib.set_long("productDefinitionTemplateNumber", 32);

            grib.set_long("centre", GRIB_META_CENTRE);
            grib.set_long("subCentre",0);

            // TODO: actually choose it wisely from:
            // 0    Temperature
            // 1    Moisture
            // 2    Momentum
            // 3    Mass
            // 4    Short-wave radiation
            // 5    Long-wave radiation
            // 6    Cloud
            // 20   Atmospheric chemical constituents
            grib.set_long("parameterCategory", 0);
            // TODO: actually choose it wisely from:
            // Parameter number according to Code Table 4.2: For every
            // discipline and category there is a group of parameter numbers.
            // Please see the official WMO Tables or the ECMWF Manual Pages for
            // these numbers.
            grib.set_long("parameterNumber", 0);
            grib.set_long("typeOfGeneratingProcess", 0);
            //grib.set_long("backgroundProcess", 0);
            //grib.set_long("generatingProcessIdentifier", 0);
            //grib.set_long("hoursAfterDataCutoff", 0);
            //grib.set_long("minutesAfterDataCutoff", 0);
            grib.set_long("indicatorOfUnitOfTimeRange", 0);
            grib.set_long("forecastTime", 0);
            grib.set_long("NB", 1);

            grib.set_long("satelliteSeries", 333);
            grib.set_long("satelliteNumber", 56);
            grib.set_long("instrumentType",207);

            // scaleFactorOfCentralWaveNumber Scale factor of central wave number of band nb
            // scaledValueOfCentralWaveNumber Scaled value of central wave number of band nb (units: m-1)

        } else if (strcmp(templateName, "msat/ecmwf") == 0) {
			grib.set_long("editionNumber",1);
			grib.set_long("gribTablesVersionNo",1);

			/* 98 = European Center for Medium-Range Weather Forecasts (grib1/0.table)  */
			grib.set_long("centre",GRIB_META_CENTRE);

			grib.set_long("generatingProcessIdentifier",254);
			grib.set_long("gridDefinition",255);

			/* 128 = 10000000
			   (1=1)  Section 2 included
			   (2=0)  Section 3 omited
			   See grib1/1.table */
			//grib.set_long("section1Flags",128);


			/* 127 = None Image data - (grib1/2.wmo.1.table)  */
			grib.set_long("indicatorOfParameter",127);


			/* 0 = Minute (grib1/4.table)  */
			grib.set_long("indicatorOfUnitOfTimeRange",0);

			//grib.set_long("periodOfTime",0);
			//grib.set_long("periodOfTimeIntervals",0);

			/* 0 = Forecast product valid at reference time + P1  (P1>0)  (grib1/5.table)  */
			grib.set_long("timeRangeIndicator",0);

			grib.set_long("numberIncludedInAverage",0);
			grib.set_long("numberMissingFromAveragesOrAccumulations",0);
			grib.set_long("centuryOfReferenceTimeOfData",21);

			/* 0 = Unknown code table entry (grib1/0.ecmf.table)  */
			grib.set_long("subCentre",0);

			grib.set_long("decimalScaleFactor",0);

			/* 24 = Satellite image simulation (grib1/localDefinitionNumber.98.table)  */
			grib.set_long("localDefinitionNumber",24);

			grib.set_long("satelliteIdentifier",56);
			grib.set_long("instrumentIdentifier",207);
			grib.set_long("channelNumber",1);
			grib.set_long("functionCode",1);
			grib.set_long("numberOfVerticalCoordinateValues",0);
			grib.set_long("pvlLocation",255);

			/* 90 = Space view perspective or orthographic grid (grib1/6.table)  */
			grib.set_long("dataRepresentationType",90);

			grib.set_long("latitudeOfSubSatellitePoint",0);
			grib.set_long("longitudeOfSubSatellitePoint",0);

			/* 64 = 01000000
			   (1=0)  Direction increments not given
			   (2=1)  Earth assumed oblate spheroid with size as determined by IAU in 1965
			   See  6378.160 km, 6356.775 km, f = 1/297.0
			   (5=0)  u and v components resolved relative to easterly and northerly directions
			   See grib1/7.table */
			grib.set_long("resolutionAndComponentFlags",64);

			//grib.set_long("apparentDiameterOfEarthInGridLengthsInXDirection",1732);
			//grib.set_long("apparentDiameterOfEarthInGridLengthsInYDirection",1754);

			/* 0 = 00000000
			   (1=0)  Points scan in +i direction
			   (2=0)  Points scan in -j direction
			   (3=0)  Adjacent points in i direction are consecutive 
			   See grib1/8.table */
			grib.set_long("scanningMode",0);

			grib.set_long_oneof("orientationOfTheGridInMillidegrees",180000, "orientationOfTheGridInDegrees", 180, NULL);
			//grib.set_long("altitudeOfTheCameraFromTheEarthSCenterMeasuredInUnitsOfTheEarth",6610710);
			grib.set_long("PLPresent",0);
			grib.set_long("sphericalHarmonics",0);
			grib.set_long("complexPacking",0);
			grib.set_long("integerPointValues",1);
			grib.set_long("additionalFlagPresent",0);
		}
		else if (strcmp(templateName, "msat/msat") == 0)
		{
			grib.set_long("editionNumber",1);
			grib.set_long("gribTablesVersionNo",1);

			/* 98 = European Center for Medium-Range Weather Forecasts (grib1/0.table)  */
			grib.set_long("centre",GRIB_META_CENTRE);

			grib.set_long("generatingProcessIdentifier",254);
			grib.set_long("gridDefinition",255);

			/* 128 = 10000000
			   (1=1)  Section 2 included
			   (2=0)  Section 3 omited
			   See grib1/1.table */
			//grib.set_long("section1Flags",128);


			/* 127 = None Image data - (grib1/2.wmo.1.table)  */
			grib.set_long("indicatorOfParameter",127);


			/* 55 = Unknown code table entry (grib1/3.table)  */
			grib.set_long("indicatorOfTypeOfLevel",55);

			/* 0 = Minute (grib1/4.table)  */
			grib.set_long("indicatorOfUnitOfTimeRange",0);

			//grib.set_long("periodOfTime",0);
			//grib.set_long("periodOfTimeIntervals",0);

			/* 0 = Forecast product valid at reference time + P1  (P1>0)  (grib1/5.table)  */
			grib.set_long("timeRangeIndicator",0);

			grib.set_long("numberIncludedInAverage",0);
			grib.set_long("numberMissingFromAveragesOrAccumulations",0);
			grib.set_long("centuryOfReferenceTimeOfData",21);

			/* 0 = Unknown code table entry (grib1/0.ecmf.table)  */
			grib.set_long("subCentre",0);

			grib.set_long("numberOfVerticalCoordinateValues",0);
			grib.set_long("pvlLocation",255);

			/* 90 = Space view perspective or orthographic grid (grib1/6.table)  */
			grib.set_long("dataRepresentationType",90);

			grib.set_long("latitudeOfSubSatellitePoint",0);
			grib.set_long("longitudeOfSubSatellitePoint",0);

			/* 64 = 01000000
			   (1=0)  Direction increments not given
			   (2=1)  Earth assumed oblate spheroid with size as determined by IAU in 1965
			   See  6378.160 km, 6356.775 km, f = 1/297.0
			   (5=0)  u and v components resolved relative to easterly and northerly directions
			   See grib1/7.table */
			grib.set_long("resolutionAndComponentFlags",64);

			/* 0 = 00000000
			   (1=0)  Points scan in +i direction
			   (2=0)  Points scan in -j direction
			   (3=0)  Adjacent points in i direction are consecutive 
			   See grib1/8.table */
			grib.set_long("scanningMode",0);

			grib.set_long_oneof("orientationOfTheGridInMillidegrees",180000, "orientationOfTheGridInDegrees", 180, NULL);
			//grib.set_long("altitudeOfTheCameraFromTheEarthSCenterMeasuredInUnitsOfTheEarth",6610684);
			grib.set_long("PLPresent",0);
			grib.set_long("sphericalHarmonics",0);
			grib.set_long("complexPacking",0);
			grib.set_long("integerPointValues",0);
			grib.set_long("additionalFlagPresent",0);
		}
		else
		{
			CPLError(CE_Failure, CPLE_AppDefined, "Unsupported template name '%s'", templateName);
			return NULL;
		}

		//fprintf(stderr, "nXYSize: %d %d\n", nXSize, nYSize);
		grib.set_long("numberOfPointsAlongXAxis", src->GetRasterXSize());
		grib.set_long("numberOfPointsAlongYAxis", src->GetRasterYSize());
		//checked(grib_set_long(gh, "numberOfBitsContainingEachPackedValue", 32));

		/// Time
		const char* sval = src->GetMetadataItem(MD_MSAT_DATETIME, MD_DOMAIN_MSAT);
		if (sval != NULL)
			grib.setTime(sval);

		// Spacecraft
		sval = src->GetMetadataItem(MD_MSAT_SPACECRAFT_ID, MD_DOMAIN_MSAT);
		if (sval != NULL)
		{
			long spacecraft_id = strtoul(sval, NULL, 10);

			/// Spacecraft
			int err = grib.set_long_unchecked("satelliteIdentifier", spacecraft_id);
			// It's ok if the key doesn't exist: it doesn't work in all templates
			if (err != GRIB_SUCCESS && err != GRIB_NOT_FOUND)
				checked(err);
			// Always set this, though
			grib.set_long("indicatorOfTypeOfLevel", spacecraft_id);
		}

		// Projection
		OGRSpatialReference osr(src->GetProjectionRef());

		// Sanity checks
		const char *stype = osr.GetAttrValue("PROJECTION");
		if (!stype)
		{
			CPLError(CE_Failure, CPLE_AppDefined, "GRIB for satellites requires a " SRS_PT_GEOSTATIONARY_SATELLITE " projection, but we have %s", stype);
			return NULL;
		}
		if (osr.GetProjParm(SRS_PP_SATELLITE_HEIGHT) != ORBIT_RADIUS_FOR_GDAL)
		{
			CPLError(CE_Failure, CPLE_AppDefined, "we are given a satellite height of %f but only %d is supported", osr.GetProjParm(SRS_PP_SATELLITE_HEIGHT), ORBIT_RADIUS_FOR_GDAL);
			return NULL;
		}

		// Set the projection parameters
		double sublon = (double)osr.GetProjParm(SRS_PP_CENTRAL_MERIDIAN, 0.0);

		grib.set_long_oneof("geography.lap", 0, "latitudeOfSubSatellitePointInDegrees", 0, NULL);
		grib.set_long_oneof("geography.lop", (long)nearbyint(sublon * 1000),
				"longitudeOfSubSatellitePointInDegrees", (long)nearbyint(sublon * 1000),
				NULL);
		grib.set_long_oneof("geography.orientationOfTheGrid", (long)nearbyint(SEVIRI_ORIENTATION * 1000),
				"orientationOfTheGridInMillidegrees", (long)nearbyint(SEVIRI_ORIENTATION * 1000),
				"orientationOfTheGridInDegrees", (long)nearbyint(SEVIRI_ORIENTATION),
				NULL);
		grib.set_long_unchecked("geography.nr", (long)nearbyint(SEVIRI_CAMERA_H * 1000000));

		// Set the data from the geotransform

		// Sanity checks
		double gt[6];
		src->GetGeoTransform(gt);
		if (gt[2] != 0.0)
		{
			CPLError(CE_Failure, CPLE_AppDefined, "3rd element of geotransform matrix is not zero");
			return NULL;
		}
		if (gt[4] != 0.0)
		{
			CPLError(CE_Failure, CPLE_AppDefined, "5th element of geotransform matrix is not zero");
			return NULL;
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
				return NULL;
			}
			if (fabs(gt[5] - -METEOSAT_PIXELSIZE_Y) > 0.0001)
			{
				CPLError(CE_Failure, CPLE_AppDefined, "6th element of geotransform matrix has an unexpected value (got: %f, expected: %f)", gt[5], METEOSAT_PIXELSIZE_Y);
				return NULL;
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
				return NULL;
			}

			column_offset = METEOSAT_IMAGE_NCOLUMNS_HRV / 2;
			line_offset = METEOSAT_IMAGE_NLINES_HRV / 2;
			//dx = Band::seviriDXFromCFAC(40927000);
			//dy = Band::seviriDYFromLFAC(40927000);
		}
		int x0 = nearbyint(column_offset + gt[0] / gt[1]);
		int y0 = nearbyint(line_offset - gt[3] / -gt[5]);
		int dx = facts::seviriDXFromPixelHSize(gt[1]);
		int dy = facts::seviriDYFromPixelVSize(-gt[5]);

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

		grib.set_long_oneof("XpInGridLengths", column_offset,
				"geography.Xp", column_offset,
				NULL);
		grib.set_long_oneof("YpInGridLengths", line_offset,
				"geography.Yp", line_offset,
				NULL);


		GDALRasterBand* rb = src->GetRasterBand(1);

		//RA+STERRBAND
		//grib.set_long("numberOfBitsContainingEachPackedValue", glb_preferredBPP);
		//grib.set_long("decimalScaleFactor", decimalDigitsOfScaledValues(glb_scale));

                sval = rb->GetMetadataItem(MD_MSAT_CHANNEL_ID, MD_DOMAIN_MSAT);
                if (sval != NULL)
		{
			long channel_id = strtoul(sval, NULL, 10);
			grib.set_long_unchecked("channelNumber", channel_id);
			grib.set_long("level", channel_id);
		}

		// Read values and scale them
		double missing = rb->GetNoDataValue();
		double offset = rb->GetOffset();
		double scale = rb->GetScale();
		size_t count = src->GetRasterXSize() * src->GetRasterYSize();

		double grib_missing = missing * scale + offset;
		grib.set_double("missingValue", grib_missing);

		double* values = new double[count];
		if (rb->RasterIO(GF_Read, 0, 0, src->GetRasterXSize(), src->GetRasterYSize(), values, src->GetRasterXSize(), src->GetRasterYSize(), GDT_Float64, 0, 0) != CE_None)
		{
			delete[] values;
			return NULL;
		}

		bool hasMissing = false;
		bool hasNonMissing = false;
		for (size_t i = 0; i < count; ++i)
		{
			if (values[i] == missing)
			{
				hasMissing = true;
				values[i] = grib_missing;
			}
			else
			{
				hasNonMissing = true;
				values[i] = values[i] * scale + offset;
			}
		}
		grib.set_long("bitmapPresent", hasMissing ? 1 : 0);

		if (!hasNonMissing)
		{
			delete[] values;
			CPLError(CE_Failure, CPLE_AppDefined, "All values to encode are missing, and GRIB cannot handle this");
			return NULL;
		}

		grib.set_double_array("values", values, count);
		delete[] values;

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
