/*
 * Copyright (C) 2012--2013  ARPA-SIM <urpsim@smr.arpa.emr.it>
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

#include "reflectance.h"
#include "dataset.h"
#include "rasterband.h"
#include <msat/auto_arr_ptr.h>
#include <msat/gdal/const.h>
#include <msat/facts.h>
#include <ogr_spatialref.h>
#include <msat/hrit/MSG_data_RadiometricProc.h>
#include <string>
#include <stdexcept>
#include <stdint.h>

using namespace std;

namespace msat {
namespace xrit {

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

BaseReflectanceRasterBand::BaseReflectanceRasterBand(XRITDataset* ds, int idx)
    : XRITRasterBand(ds, idx), p2ll(0)
{
}

BaseReflectanceRasterBand::~BaseReflectanceRasterBand()
{
    if (p2ll) delete p2ll;
}

bool BaseReflectanceRasterBand::init(MSG_data& PRO_data, MSG_data& EPI_data, MSG_header& header)
{
    if (!XRITRasterBand::init(PRO_data, EPI_data, header))
        return false;

    eDataType = GDT_Float32;

    // Day time
    int ye, mo, da, ho, mi;
    if (sscanf(xds->fa.timing.c_str(), "%04d%02d%02d%02d%02d",
            &ye, &mo, &da, &ho, &mi) != 5)
        throw std::runtime_error("cannot parse file time");

    jday = msat::facts::jday(ye, mo, da);
    daytime = (double)ho + ((double)mi) / 60.0;

    rad_slope  = PRO_data.prologue->radiometric_proc.ImageCalibration[channel_id-1].Cal_Slope;
    rad_offset = PRO_data.prologue->radiometric_proc.ImageCalibration[channel_id-1].Cal_Offset;

    p2ll = new PixelToLatlon(poDS);

    return true;
}

const char* BaseReflectanceRasterBand::GetUnitType()
{
    return "%";
}

double BaseReflectanceRasterBand::GetOffset(int* pbSuccess)
{
    if (pbSuccess) *pbSuccess = TRUE;
    return 0.0;
}

double BaseReflectanceRasterBand::GetScale(int* pbSuccess)
{
    if (pbSuccess) *pbSuccess = TRUE;
    return 1.0;
}

double BaseReflectanceRasterBand::GetNoDataValue(int* pbSuccess)
{
    if (pbSuccess) *pbSuccess = TRUE;
    return 0.0;
}

SZARasterBand::SZARasterBand(XRITDataset* ds, int idx)
    : BaseReflectanceRasterBand(ds, idx)
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
    if (!BaseReflectanceRasterBand::init(PRO_data, EPI_data, header))
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


ReflectanceRasterBand::ReflectanceRasterBand(XRITDataset* ds, int idx)
    : BaseReflectanceRasterBand(ds, idx)
{
}

ReflectanceRasterBand::~ReflectanceRasterBand()
{
}

bool ReflectanceRasterBand::init(MSG_data& PRO_data, MSG_data& EPI_data, MSG_header& header)
{
    if (!BaseReflectanceRasterBand::init(PRO_data, EPI_data, header))
        return false;

    double esd = 1.0 - 0.0167 * cos( 2.0 * M_PI * (jday - 3) / 365.0);
    switch (channel_id)
    {
        case MSG_SEVIRI_1_5_VIS_0_6: tr = 20.76 / (esd*esd); break;
        case MSG_SEVIRI_1_5_VIS_0_8: tr = 23.24 / (esd*esd); break;
        case MSG_SEVIRI_1_5_IR_1_6:  tr = 19.85 / (esd*esd); break;
        case MSG_SEVIRI_1_5_HRV:     tr = 25.11 / (esd*esd); break;
        default:
            return false;
    }

    return true;
}

    /*
double cozena(double day, double hour, double dlat, double dlon)
{
  double coz;

  const double sinob = 0.3978;
  const double dpy = 365.242;
  const double dph = 15.0;
  
  double rpd = M_PI/180.0;

  double dpr = 1.0/rpd;
  double dang = 2.0*M_PI*(day-1.0)/dpy;
  double homp = 12.0 + 0.123570*sin(dang) - 0.004289*cos(dang) +
                0.153809*sin(2.0*dang) + 0.060783*cos(2.0*dang);
  double hang = dph* (hour-homp) - dlon;
  double ang = 279.9348*rpd + dang;
  double sigma = (ang*dpr+0.4087*sin(ang)+1.8724*cos(ang)-
                 0.0182*sin(2.0*ang)+0.0083*cos(2.0*ang))*rpd;
  double sindlt = sinob*sin(sigma);
  double cosdlt = sqrt(1.0-sindlt*sindlt);

  coz = sindlt*sin(rpd*dlat) + cosdlt*cos(rpd*dlat)*cos(rpd*hang);

  return coz;
}

double scan2zen(double scan, double satheight)
{
  const double rearth = 6357.0;
  double temp = (rearth + satheight)*(sin(scan)/rearth);

  if (temp > 1.0) temp = 1.0;
  return asin(temp);
}

*/

CPLErr ReflectanceRasterBand::IReadBlock(int xblock, int yblock, void *buf)
{
    // Read the raw data
    auto_arr_ptr<uint16_t> raw(nBlockXSize * nBlockYSize);
    if (XRITRasterBand::IReadBlock(xblock, yblock, raw.get()) == CE_Failure)
        return CE_Failure;

    // Precompute pixel georeferentiation
    auto_arr_ptr<double> lats(nBlockXSize * nBlockYSize);
    auto_arr_ptr<double> lons(nBlockXSize * nBlockYSize);
    p2ll->compute(xblock * nBlockXSize, yblock * nBlockYSize, nBlockXSize, nBlockYSize, lats.get(), lons.get());

    // Compute reflectances
    float* dest = (float*) buf;
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

Reflectance39RasterBand::Reflectance39RasterBand(XRITDataset* ds, int idx)
    : BaseReflectanceRasterBand(ds, idx), cal108(0), cal134(0)
{
}

Reflectance39RasterBand::~Reflectance39RasterBand()
{
    if (cal108) delete[] cal108;
    if (cal134) delete[] cal134;
}

bool Reflectance39RasterBand::init(MSG_data& PRO_data, MSG_data& EPI_data, MSG_header& header)
{
    if (!BaseReflectanceRasterBand::init(PRO_data, EPI_data, header))
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

}
}
