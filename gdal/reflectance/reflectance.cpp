#include "reflectance.h"
#include "pixeltolatlon.h"
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

        add_info(ds, "ReflectanceDataset");

        sources[id - 1] = rb;
        if (take_ownership)
            owned_datasets.insert(ds);
    }
}


// cos(80deg)
#define cos80 0.173648178

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


SingleChannelReflectanceRasterBand::SingleChannelReflectanceRasterBand(ReflectanceDataset* ds, int idx)
    : ReflectanceRasterBand(ds, idx)
{
    source_rb = ds->sources[ds->channel_id - 1];
    if (!source_rb)
        throw std::runtime_error("SingleChannelReflectanceRasterBand: GDALRasterBand not found for channel " + std::to_string(ds->channel_id) + " metadata");

    add_info(source_rb, "SingleChannelReflectanceRasterBand");

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
    std::vector<double> raw(nBlockXSize * nBlockYSize);
    if (source_rb->RasterIO(GF_Read, xblock * nBlockXSize, yblock * nBlockYSize, nBlockXSize, nBlockYSize, raw.data(), nBlockXSize, nBlockYSize, GDT_Float64, 0, 0, nullptr) == CE_Failure)
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

Reflectance39RasterBand::Reflectance39RasterBand(ReflectanceDataset* ds, int idx)
    : ReflectanceRasterBand(ds, idx)
{
    source_ir039 = ds->sources[MSG_SEVIRI_1_5_IR_3_9 - 1];
    source_ir108 = ds->sources[MSG_SEVIRI_1_5_IR_10_8 - 1];
    source_ir134 = ds->sources[MSG_SEVIRI_1_5_IR_13_4 - 1];

    if (!source_ir039)
        throw std::runtime_error("Reflectance39RasterBand: GDALRasterBand not found for channel " + std::to_string(MSG_SEVIRI_1_5_IR_3_9) + " metadata");
    if (!source_ir108)
        throw std::runtime_error("Reflectance39RasterBand: GDALRasterBand not found for channel " + std::to_string(MSG_SEVIRI_1_5_IR_10_8) + " metadata");
    if (!source_ir134)
        throw std::runtime_error("Reflectance39RasterBand: GDALRasterBand not found for channel " + std::to_string(MSG_SEVIRI_1_5_IR_13_4) + " metadata");

    add_info(source_ir039, "Reflectance39RasterBand");

    // Pre-cache slope and offset for the three channels
    int success;

    ir039_slope = source_ir039->GetScale(&success);
    if (!success) throw std::runtime_error("Reflectance39RasterBand: source raster band for channel IR 3.9 has no meaningful Scale information");
    ir039_offset = source_ir039->GetOffset(&success);
    if (!success) throw std::runtime_error("Reflectance39RasterBand: source raster band hfor channel IR 3.9 as no meaningful Offset information");

    ir108_slope = source_ir108->GetScale(&success);
    if (!success) throw std::runtime_error("Reflectance39RasterBand: source raster band for channel IR 10.8 has no meaningful Scale information");
    ir108_offset = source_ir108->GetOffset(&success);
    if (!success) throw std::runtime_error("Reflectance39RasterBand: source raster band hfor channel IR 10.8 as no meaningful Offset information");

    ir134_slope = source_ir134->GetScale(&success);
    if (!success) throw std::runtime_error("Reflectance39RasterBand: source raster band for channel IR 13.4 has no meaningful Scale information");
    ir134_offset = source_ir134->GetOffset(&success);
    if (!success) throw std::runtime_error("Reflectance39RasterBand: source raster band hfor channel IR 13.4 as no meaningful Offset information");
}

Reflectance39RasterBand::~Reflectance39RasterBand()
{
}

CPLErr Reflectance39RasterBand::IReadBlock(int xblock, int yblock, void *buf)
{
    // Read the IR 3.9 data
    std::vector<double> raw039(nBlockXSize * nBlockYSize);
    if (source_ir039->RasterIO(GF_Read, xblock * nBlockXSize, yblock * nBlockYSize, nBlockXSize, nBlockYSize, raw039.data(), nBlockXSize, nBlockYSize, GDT_Float64, 0, 0, nullptr) == CE_Failure)
        return CE_Failure;

    // Read the IR_10.8 channel
    std::vector<double> raw108(nBlockXSize * nBlockYSize);
    if (source_ir108->RasterIO(GF_Read, xblock * nBlockXSize, yblock * nBlockYSize, nBlockXSize, nBlockYSize, raw108.data(), nBlockXSize, nBlockYSize, GDT_Float64, 0, 0, nullptr) == CE_Failure)
        return CE_Failure;

    // Read the IR_13.4 channel
    std::vector<double> raw134(nBlockXSize * nBlockYSize);
    if (source_ir134->RasterIO(GF_Read, xblock * nBlockXSize, yblock * nBlockYSize, nBlockXSize, nBlockYSize, raw134.data(), nBlockXSize, nBlockYSize, GDT_Float64, 0, 0, nullptr) == CE_Failure)
        return CE_Failure;

    // Precompute pixel georeferentiation
    std::vector<double> lats(nBlockXSize * nBlockYSize);
    std::vector<double> lons(nBlockXSize * nBlockYSize);
    p2ll->compute(xblock * nBlockXSize, yblock * nBlockYSize, nBlockXSize, nBlockYSize, lats.data(), lons.data());

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
        double BT039 = raw039[i] * ir039_slope + ir039_offset;
        double BT108 = raw108[i] * ir108_slope + ir108_offset;
        double BT134 = raw134[i] * ir134_slope + ir134_offset;

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
            SetBand(1, new Reflectance39RasterBand(this, 1));
            return;
        default:
            throw std::runtime_error("ReflectanceDataset: computing reflectance for channel " + std::to_string(channel_id) + " is not implemented");
    }
}

CPLErr msat_reflectance_ir039(
        void **papoSources, int nSources, void *pData, int nXSize, int nYSize,
        GDALDataType eSrcType, GDALDataType eBufType,
        int nPixelSpace, int nLineSpace)
{
    if (nSources != 6)
    {
        CPLError(CE_Failure, CPLE_AppDefined, "Computing IR 3.9 reflectance needs 5 source raster bands (%d found)", nSources);
        return CE_Failure;
    }
    if (eSrcType != GDT_Float64)
    {
        CPLError(CE_Failure, CPLE_AppDefined, "Computing IR 3.9 reflectance, source type is not GDT_Float64");
        return CE_Failure;
    }

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

    double* ir039 = (double*)papoSources[0];
    double* jday = (double*)papoSources[1];
    double* sat_za = (double*)papoSources[2];
    double* cos_sol_za = (double*)papoSources[3];
    double* ir108 = (double*)papoSources[4];
    double* ir134 = (double*)papoSources[5];

    const double c1 = 0.0000119104;
    const double c2 = 1.43877;
    const double Vc = 2569.094;
    const double A = 0.9959;
    const double B = 3.471;
    double esd = 1.0 - 0.0167 * cos( 2.0 * M_PI * ((int)jday[0] - 3) / 365.0);

    for (int line = 0; line < nYSize; ++line)
        for (int col = 0; col < nXSize; ++col)
        {
            unsigned idx = line * nXSize + col;

            // We can compute radiance from counts straight away
            //double R_tot = (raw039[i] * rad_slope) + rad_offset;
            // But we use the Brightness Temperature instead, so we can apply CO2
            // correction
            double BT039 = ir039[idx];
            double BT108 = ir108[idx];
            double BT134 = ir134[idx];
            double cosTETA = cos_sol_za[idx];
            // Use cos(80°) as lower bound, to avoid division by zero
            if (cosTETA < cos80) cosTETA = cos80;
            double SAT = sat_za[idx];

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

            // Normalise outliars
            switch (fpclassify(REFL))
            {
                case FP_NAN:
                case FP_SUBNORMAL:
                case FP_ZERO: REFL = 0.0; break;
                case FP_INFINITE:
                case FP_NORMAL:
                    if (REFL < 0.0) REFL = 0.0;
                    if (REFL > 100.0) REFL = 100.0;
                    break;
            }

            GDALCopyWords(&REFL, GDT_Float64, 0,
                    ((GByte *)pData) + nLineSpace * line + col * nPixelSpace,
                    eBufType, nPixelSpace, 1);
        }

    return CE_None;
}

}
}
