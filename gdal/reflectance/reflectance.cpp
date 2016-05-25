#include "reflectance.h"
#include <msat/auto_arr_ptr.h>
#include <msat/gdal/const.h>
#include <msat/facts.h>
#include <ogr_spatialref.h>
//#include <msat/hrit/MSG_data_RadiometricProc.h>
#include <msat/hrit/MSG_channel.h>
#include <string>
#include <stdexcept>
#include <stdint.h>

using namespace std;

namespace msat {
namespace utils {

ReflectanceDataset::ReflectanceDataset(int channel_id)
    : channel_id(channel_id)
{
}

ReflectanceDataset::~ReflectanceDataset()
{
    for (auto& ds: owned_datasets)
        delete ds;
}

const char* ReflectanceDataset::GetProjectionRef()
{
    return projWKT.c_str();
}

CPLErr ReflectanceDataset::GetGeoTransform(double* tr)
{
    memcpy(tr, geotransform, 6 * sizeof(double));
    return CE_None;
}

void ReflectanceDataset::add_source(GDALDataset* ds, bool take_ownership)
{
    for (int i = 1; i <= ds->GetRasterCount(); ++i)
    {
        GDALRasterBand* rb = ds->GetRasterBand(i);
        const char* str_id = rb->GetMetadataItem(MD_MSAT_CHANNEL_ID, MD_DOMAIN_MSAT);
        if (str_id == nullptr) continue;
        // 1-based channel ID
        int id = strtoul(str_id, nullptr, 10);
        if (id < 1 || (unsigned)id > sources.size()) continue;

        const char* proj = ds->GetProjectionRef();
        if (proj == nullptr)
            throw std::runtime_error("ReflectanceDataset: trying to add source without a projection definition");

        double gt[6];
        if (ds->GetGeoTransform(gt) == CE_Failure)
            throw std::runtime_error("ReflectanceDataset: trying to add source without affine geotransform coefficients");

        const char* mdtime = ds->GetMetadataItem(MD_MSAT_DATETIME, MD_DOMAIN_MSAT);
        if (mdtime == nullptr)
            throw std::runtime_error("ReflectanceDataset: trying to add source without " MD_DOMAIN_MSAT "/" MD_MSAT_DATETIME " metadata");

        if (!has_sources)
        {
            projWKT = proj;
            memcpy(geotransform, gt, 6 * sizeof(double));
            char** metadata = ds->GetMetadata(MD_DOMAIN_MSAT);
            if (metadata == nullptr)
                throw std::runtime_error("ReflectanceDataset: trying to add source without " MD_DOMAIN_MSAT " metadata");
            if (SetMetadata(metadata, MD_DOMAIN_MSAT) == CE_Failure)
                throw std::runtime_error("ReflectanceDataset: cannot set metadata from source dataset");
            datetime = mdtime;

            nRasterXSize = ds->GetRasterXSize();
            nRasterYSize = ds->GetRasterYSize();
        } else {
            if (projWKT != proj)
                throw std::runtime_error("ReflectanceDataset: inconsistent projection definitions in source datasets");
            if (memcmp(geotransform, gt, 6 * sizeof(double)) != 0)
                throw std::runtime_error("ReflectanceDataset: inconsistent affine geotransform coefficients in source datasets");
            if (datetime != mdtime)
                throw std::runtime_error("ReflectanceDataset: inconsistent datetime in source datasets");
        }

        sources[id - 1] = rb;
        if (take_ownership)
            owned_datasets.insert(ds);
        has_sources = true;
    }
}


// cos(80deg)
#define cos80 0.173648178

struct PixelToLatlon
{
    double geoTransform[6];
    OGRSpatialReference* proj;
    OGRSpatialReference* latlon;
    OGRCoordinateTransformation* toLatLon;

    PixelToLatlon(GDALDataset* ds)
    {
        if (ds->GetGeoTransform(geoTransform) != CE_None)
            throw std::runtime_error("no geotransform found in input dataset");

        const char* projname = ds->GetProjectionRef();
        if (!projname || !projname[0])
            throw std::runtime_error("no projection name found in input dataset");

        proj = new OGRSpatialReference(projname);
        latlon = proj->CloneGeogCS();
        toLatLon = OGRCreateCoordinateTransformation(proj, latlon);
    }

    ~PixelToLatlon()
    {
        if (proj) delete proj;
        if (latlon) delete latlon;
        if (toLatLon) delete toLatLon;
    }

    void compute(int x, int y, int sx, int sy, double* lats, double* lons)
    {
        int idx = 0;

        // Pixels to projected coordinates
        for (int iy = y; iy < y + sy; ++iy)
        {
            for (int ix = x; ix < x + sx; ++ix)
            {
                // Projected y
                lats[idx] = geoTransform[3]
                    + geoTransform[4] * ix
                    + geoTransform[5] * iy;

                // Projected x
                lons[idx] = geoTransform[0]
                    + geoTransform[1] * ix
                    + geoTransform[2] * iy;

                ++idx;
            }
        }

        // Projected coordinates to latlon
        toLatLon->Transform(sx * sy, lons, lats);
        // Ignore errors, since there usually are points in space that fail to
        // transform

        // if (!toLatLon->Transform(sx * sy, lons, lats))
        // {
        //     throw std::runtime_error("points failed to transform to lat,lon");
        // }
    }
};


ReflectanceRasterBand::ReflectanceRasterBand(ReflectanceDataset* ds, int idx)
{
    poDS = ds;
    nBand = idx;
    eDataType = GDT_Float32;

    // Day time
    int ye, mo, da, ho, mi, se;
    if (sscanf(ds->datetime.c_str(), "%04d-%02d-%02d %02d:%02d:%02d",
            &ye, &mo, &da, &ho, &mi, &se) != 6)
        throw std::runtime_error("cannot parse file time");
    jday = msat::facts::jday(ye, mo, da);
    daytime = (double)ho + ((double)mi) / 60.0;

#if 0
    rad_slope  = PRO_data.prologue->radiometric_proc.ImageCalibration[channel_id - 1].Cal_Slope;
    rad_offset = PRO_data.prologue->radiometric_proc.ImageCalibration[channel_id - 1].Cal_Offset;
#endif

    p2ll = new PixelToLatlon(ds);
}

ReflectanceRasterBand::~ReflectanceRasterBand()
{
    delete p2ll;
}

const char* ReflectanceRasterBand::GetUnitType()
{
    return "%";
}

double ReflectanceRasterBand::GetOffset(int* pbSuccess)
{
    if (pbSuccess) *pbSuccess = TRUE;
    return 0.0;
}

double ReflectanceRasterBand::GetScale(int* pbSuccess)
{
    if (pbSuccess) *pbSuccess = TRUE;
    return 1.0;
}

double ReflectanceRasterBand::GetNoDataValue(int* pbSuccess)
{
    if (pbSuccess) *pbSuccess = TRUE;
    return 0.0;
}

#if 0

SZARasterBand::SZARasterBand(XRITDataset* ds, int idx)
    : ReflectanceRasterBand(ds, idx)
{
}

SZARasterBand::~SZARasterBand()
{
}

const char* SZARasterBand::GetUnitType()
{
    return "";
}

bool SZARasterBand::init(MSG_data& PRO_data, MSG_data& EPI_data, MSG_header& header)
{
    if (!ReflectanceRasterBand::init(PRO_data, EPI_data, header))
        return false;

    return true;
}

CPLErr SZARasterBand::IReadBlock(int xblock, int yblock, void *buf)
{
    // Precompute pixel georeferentiation
    auto_arr_ptr<double> lats(nBlockXSize * nBlockYSize);
    auto_arr_ptr<double> lons(nBlockXSize * nBlockYSize);
    p2ll->compute(xblock * nBlockXSize, yblock * nBlockYSize, nBlockXSize, nBlockYSize, lats.get(), lons.get());

    // Compute reflectances
    float* dest = (float*) buf;
    for (int i = 0; i < nBlockXSize * nBlockYSize; ++i)
    {
        // Just the solar zenith angle
        dest[i] = facts::cos_sol_za(jday, daytime, lats[i], lons[i]);
        // Normalise outliars
        switch (fpclassify(dest[i]))
        {
            case FP_NAN:
            case FP_SUBNORMAL:
            case FP_ZERO: dest[i] = 0.0; break;
            case FP_INFINITE:
            case FP_NORMAL:
                if (dest[i] < 0.0) dest[i] = 0.0;
                if (dest[i] > 1.0) dest[i] = 1.0;
                break;
        }
    }

    return CE_None;
}
#endif

SingleChannelReflectanceRasterBand::SingleChannelReflectanceRasterBand(ReflectanceDataset* ds, int idx)
    : ReflectanceRasterBand(ds, idx)
{
    source_rb = ds->sources[ds->channel_id - 1];
    if (!source_rb)
        throw std::runtime_error("SingleChannelReflectanceRasterBand: GDALRasterBand not found for channel " + std::to_string(ds->channel_id) + " metadata");

    source_rb->GetBlockSize(&nBlockXSize, &nBlockYSize);

    // Initialize metadata from source raster band
    char** metadata = source_rb->GetMetadata(MD_DOMAIN_MSAT);
    if (metadata == nullptr)
        throw std::runtime_error("SingleChannelReflectanceRasterBand: trying to use a source GDALRasterBand without " MD_DOMAIN_MSAT " metadata");
    if (SetMetadata(metadata, MD_DOMAIN_MSAT) == CE_Failure)
        throw std::runtime_error("SingleChannelReflectanceRasterBand: cannot set metadata from source raster band");

    // Pre-cache slope and offset
    int success;
    rad_slope = source_rb->GetScale(&success);
    if (!success) throw std::runtime_error("SingleChannelReflectanceRasterBand: source raster band has no meaningful Scale information");
    rad_offset = source_rb->GetOffset(&success);
    if (!success) throw std::runtime_error("SingleChannelReflectanceRasterBand: source raster band has no meaningful Offset information");

    // Compute pre-cached tr factor
    double esd = 1.0 - 0.0167 * cos( 2.0 * M_PI * (jday - 3) / 365.0);
    switch (ds->channel_id)
    {
        case MSG_SEVIRI_1_5_VIS_0_6: tr = 20.76 / (esd*esd); break;
        case MSG_SEVIRI_1_5_VIS_0_8: tr = 23.24 / (esd*esd); break;
        case MSG_SEVIRI_1_5_IR_1_6:  tr = 19.85 / (esd*esd); break;
        case MSG_SEVIRI_1_5_HRV:     tr = 25.11 / (esd*esd); break;
        default: throw std::runtime_error("SingleChannelReflectanceRasterBand: computing reflectance for channel " + std::to_string(ds->channel_id) + " is not implemented");
    }
}

SingleChannelReflectanceRasterBand::~SingleChannelReflectanceRasterBand()
{
}

CPLErr SingleChannelReflectanceRasterBand::IReadBlock(int xblock, int yblock, void *buf)
{
    // Read the raw data
    std::vector<uint16_t> raw(nBlockXSize * nBlockYSize);
    if (source_rb->ReadBlock(xblock, yblock, raw.data()) == CE_Failure)
        return CE_Failure;

    // Precompute pixel georeferentiation
    std::vector<double> lats(nBlockXSize * nBlockYSize);
    std::vector<double> lons(nBlockXSize * nBlockYSize);
    p2ll->compute(xblock * nBlockXSize, yblock * nBlockYSize, nBlockXSize, nBlockYSize, lats.data(), lons.data());

    // Compute reflectances
    float* dest = (float*)buf;
    for (int i = 0; i < nBlockXSize * nBlockYSize; ++i)
    {
        // From counts to radiance
        double radiance = raw[i] * rad_slope + rad_offset;
        double cossza = facts::cos_sol_za(jday, daytime, lats[i], lons[i]);
        // Use cos(80°) as lower bound, to avoid division by zero
        if (cossza < cos80) cossza = cos80;
        // From radiance to reflectance
        dest[i] = 100.0 * radiance / tr / cossza;
        // Normalise outliars
        switch (fpclassify(dest[i]))
        {
            case FP_NAN:
            case FP_SUBNORMAL:
            case FP_ZERO: dest[i] = 0.0; break;
            case FP_INFINITE:
            case FP_NORMAL:
                if (dest[i] < 0.0) dest[i] = 0.0;
                if (dest[i] > 100.0) dest[i] = 100.0;
                break;
        }
    }

    return CE_None;
}

#if 0

Reflectance39RasterBand::Reflectance39RasterBand(XRITDataset* ds, int idx)
    : ReflectanceRasterBand(ds, idx), cal108(0), cal134(0)
{
}

Reflectance39RasterBand::~Reflectance39RasterBand()
{
    if (cal108) delete[] cal108;
    if (cal134) delete[] cal134;
}

bool Reflectance39RasterBand::init(MSG_data& PRO_data, MSG_data& EPI_data, MSG_header& header)
{
    if (!ReflectanceRasterBand::init(PRO_data, EPI_data, header))
        return false;

    fa_ir108.parse(xds->fa, "IR_108");
    fa_ir134.parse(xds->fa, "IR_134");

    // Scan segment headers and read count -> BT calibration tables
    try {
        MSG_data PRO_data;
        MSG_data EPI_data;
        MSG_header header;
        da_ir108.scan(fa_ir108, PRO_data, EPI_data, header);

        int channel_id = header.segment_id->spectral_channel_id;
        int bpp = header.image_structure->number_of_bits_per_pixel;
        cal108 = PRO_data.prologue->radiometric_proc.get_calibration(channel_id, bpp);
    } catch (std::exception& e) {
        return false;
    }
    try {
        MSG_data PRO_data;
        MSG_data EPI_data;
        MSG_header header;
        da_ir134.scan(fa_ir134, PRO_data, EPI_data, header);

        int channel_id = header.segment_id->spectral_channel_id;
        int bpp = header.image_structure->number_of_bits_per_pixel;
        cal134 = PRO_data.prologue->radiometric_proc.get_calibration(channel_id, bpp);
    } catch (std::exception& e) {
        return false;
    }

    return true;
}

CPLErr Reflectance39RasterBand::IReadBlock(int xblock, int yblock, void *buf)
{
    // Read the IR 3.9 data
    uint16_t raw039[nBlockXSize * nBlockYSize];
    size_t linestart = xds->da.line_start(yblock);
    bzero(raw039, nBlockXSize * sizeof(uint16_t));
    xds->da.line_read(yblock, (MSG_SAMPLE*)raw039 + linestart);

    // Read the IR_10.8 channel
    uint16_t raw108[nBlockXSize * nBlockYSize];
    linestart = da_ir108.line_start(yblock);
    bzero(raw108, nBlockXSize * sizeof(uint16_t));
    da_ir108.line_read(yblock, (MSG_SAMPLE*)raw108 + linestart);

    // Read the IR_13.4 channel
    uint16_t raw134[nBlockXSize * nBlockYSize];
    linestart = da_ir134.line_start(yblock);
    bzero(raw134, nBlockXSize * sizeof(uint16_t));
    da_ir134.line_read(yblock, (MSG_SAMPLE*)raw134 + linestart);

    // Precompute pixel georeferentiation
    double lats[nBlockXSize * nBlockYSize];
    double lons[nBlockXSize * nBlockYSize];
    p2ll->compute(xblock * nBlockXSize, yblock * nBlockYSize, nBlockXSize, nBlockYSize, lats, lons);

    // Based on: [MMKM2010]
    //   "Cloud-Top Properties of Growing Cumulus prior to Convective Initiation as Measured
    //   by Meteosat Second Generation. Part II: Use of Visible Reflectance"
    // by:
    //   JOHN R. MECIKALSKI AND WAYNE M. MACKENZIE JR.
    //   Earth Systems Science Center, University of Alabama in Huntsville, Huntsville, Alabama
    //   MARIANNE KONIG
    //   European Organisation for the Exploitation of Meteorological Satellites (EUMETSAT), Darmstadt, Germany
    //   SAM MULLER
    //   Jupiter’s Call, LLC, Madison, Alabama
    // published on:
    //   JOURNAL OF APPLIED METEOROLOGY AND CLIMATOLOGY, VOLUME 49

    // IR 0.39 CO2 corrections and fine tuning from Jan Kanak's work on MSGProc software:
    //   Jan Kanak - Slovak Hydrometeorological Institute (SHMÚ)
    //   MSGProc - MSG Processing tools for Windows
    // http://www.eumetsat.int/Home/Main/AboutEUMETSAT/InternationalRelations/EasternEuropeanandBalkanCountries/SP_2011062115544756?l=en

    const double c1 = 0.0000119104;
    const double c2 = 1.43877;
    const double Vc = 2569.094;
    const double A = 0.9959;
    const double B = 3.471;
    double esd = 1.0 - 0.0167 * cos( 2.0 * M_PI * (jday - 3) / 365.0);

    // Compute reflectances
    float* dest = (float*) buf;
    for (int i = 0; i < nBlockXSize * nBlockYSize; ++i)
    {
        // We can compute radiance from counts straight away
        //double R_tot = (raw039[i] * rad_slope) + rad_offset;
        // But we use the Brightness Temperature instead, so we can apply CO2
        // correction
        double BT039 = calibration[raw039[i]];
        double BT108 = cal108[raw108[i]];
        double BT134 = cal134[raw134[i]];

        // Apply CO2 correction to BT039
        BT039 = pow(pow(BT039, 4)
                  + pow(BT108, 4)
                  - pow(BT108 - (BT108 - BT134)/4, 4),
                  0.25);

        // Apply Planck function to convert the CO2 corrected Brightness
        // Temperature to Radiance
        double R_tot = c1 * (Vc*Vc*Vc) / (exp(c2 * Vc / (A * BT039 + B)) - 1) + 0.015;  

        double R39_corr = pow((BT108 - 0.25 * (BT108 - BT134)) / BT108, 4);
        double R_therm = c1 * (Vc*Vc*Vc) / (exp(c2 * Vc / (A * BT108 + B)) - 1) * R39_corr;
        double cosTETA = facts::cos_sol_za(jday, daytime, lats[i], lons[i]);
        // Use cos(80°) as lower bound, to avoid division by zero
        if (cosTETA < cos80) cosTETA = cos80;
        double SAT = facts::sat_za(lats[i], lons[i]);
        // Original from MMKM2010:
        //double TOARAD = 4.92 / (esd*esd) * cosTETA * exp(-(1-R39_corr)) * exp(-(1-R39_corr) * cosTETA / cos(SAT));
        // Version from MSGProc:
        double TOARAD = 4.92 / (esd*esd)
                      * pow(cosTETA, 0.75)
                      * exp(-(1-R39_corr) * cosTETA)
                      * exp(-(1-R39_corr) / cos(SAT));
        if (R_tot <= R_therm) R_tot = R_therm + 0.0000001;
        if (TOARAD <= R_therm) TOARAD = R_therm + 0.0000001;

        // Original from MMKM2010
        //double REFL = 200 * (R_tot - R_therm) / (TOARAD - R_therm);
        // Version from MSGProc:
        double REFL = 100 * (R_tot - R_therm) / (TOARAD);
        dest[i] = REFL;

        // Normalise outliars
        switch (fpclassify(dest[i]))
        {
            case FP_NAN:
            case FP_SUBNORMAL:
            case FP_ZERO: dest[i] = 0.0; break;
            case FP_INFINITE:
            case FP_NORMAL:
                if (dest[i] < 0.0) dest[i] = 0.0;
                if (dest[i] > 100.0) dest[i] = 100.0;
                break;
        }
    }

    return CE_None;
}

#endif

void ReflectanceDataset::init_rasterband()
{
    switch (channel_id)
    {
        case MSG_SEVIRI_1_5_VIS_0_6:
        case MSG_SEVIRI_1_5_VIS_0_8:
        case MSG_SEVIRI_1_5_IR_1_6:
        case MSG_SEVIRI_1_5_HRV:
            SetBand(1, new SingleChannelReflectanceRasterBand(this, 1));
            return;
        case MSG_SEVIRI_1_5_IR_3_9:
            // if (cal108) delete[] cal108;
            // if (cal134) delete[] cal134;
            throw std::runtime_error("ReflectanceDataset: computing reflectance for channel " + std::to_string(channel_id) + " is not yet implemented");
        default:
            throw std::runtime_error("ReflectanceDataset: computing reflectance for channel " + std::to_string(channel_id) + " is not implemented");
    }
}

}
}
