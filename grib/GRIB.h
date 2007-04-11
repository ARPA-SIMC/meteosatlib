//-----------------------------------------------------------------------------
//
//  File        : GRIB.h
//  Description : Grib file version1 interface
//  Project     : LAMMA 2004
//  Author      : Graziano Giuliani (LaMMA Regione Toscana)
//  References  : http://www.wmo.ch/web/www/WDM/Guides/Guide-binary-2.html
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//-----------------------------------------------------------------------------

#include <string>
#include <cstdio>

/// Coded GRIB centers
typedef enum t_enum_GRIB_CENTER {
  GRIB_CENTER_UNKNOWN = -1,
  GRIB_CENTER_MELB    = 1,  ///< Melbourne
  GRIB_CENTER_MOSCOW  = 4,  ///< Moscow
  GRIB_CENTER_NMC     = 7,  ///< National Weather Service NCEP
  GRIB_CENTER_NWSTG   = 8,  ///< National Weather Service NWSTG
  GRIB_CENTER_JMA     = 34, ///< Japanese Meteo Agency
  GRIB_CENTER_BEIJING = 38, ///< Chinese Meteo
  GRIB_CENTER_BSA     = 46, ///< Brasilian Space Agency
  GRIB_CENTER_NHC     = 52, ///< National Hurricane Center
  GRIB_CENTER_CMC     = 54, ///< Canadian Meteo
  GRIB_CENTER_GWC     = 57, ///< USAF Global Weather Service
  GRIB_CENTER_FNOC    = 58, ///< US Navy - Fleet Numerical Oceanographic Center
  GRIB_CENTER_NOAA    = 59, ///< NOAA Forecast Systems Lab
  GRIB_CENTER_NCAR    = 60, ///< NCAR
  GRIB_CENTER_UKMO    = 74, ///< UK Met Office
  GRIB_CENTER_DWD     = 78, ///< Germany
  GRIB_CENTER_ROME    = 80, ///< Italy
  GRIB_CENTER_FWS     = 85, ///< France
  GRIB_CENTER_ESA     = 97, ///< European Space Agency
  GRIB_CENTER_KNMI    = 94, ///< Denmark
  GRIB_CENTER_ECMWF   = 98, ///< European Center for Medium-Range Weather Frcst.
  GRIB_CENTER_LOCAL   = 128 ///< Uncoded center up to 254
} t_enum_GRIB_CENTER;

/// Coded GRIB subcenter. Really depends on center
typedef enum t_enum_GRIB_SUBCENTER {
  GRIB_SUBCENTER_UNKNOWN = -1,
  GRIB_SUBCENTER_LOCAL   = 128
} t_enum_GRIB_SUBCENTER;

/// Coded GRIB tables. Apart from table 2 (International) all are local (?)
typedef enum t_enum_GRIB_TABLE {
  GRIB_TABLE_UNKNOWN       = -1,
  GRIB_TABLE_INTERNATIONAL = 2,
  GRIB_TABLE_LOCAL         = 128
} t_enum_GRIB_TABLE;

/// Generating process. Usually center local use.
typedef enum t_enum_GRIB_PROCESS {
  GRIB_PROCESS_UNKNOWN = -1,
  GRIB_PROCESS_LOCAL   = 128
} t_enum_GRIB_PROCESS;

/// Parameters as in table 2 for international use
typedef enum t_enum_GRIB_PARAMETER_TABLE_VERSION2 {
  GRIB_PARAMETER_UNKNOWN  = -1,
  GRIB_PARAMETER_RESERVED = 0,
  GRIB_PARAMETER_PRES_    = 1,   ///< Pressure, Pa
  GRIB_PARAMETER_PRMSL    = 2,   ///< Pressure reduced to MSL, Pa
  GRIB_PARAMETER_PTEND    = 3,   ///< Pressure tendency, Pa/s
  GRIB_PARAMETER_PVORT    = 4,   ///< Potential vorticity, Km^2/kg/s
  GRIB_PARAMETER_ICAHT    = 5,   ///< ICAO Stand. Atmosphere Reference Height, m
  GRIB_PARAMETER_GP___    = 6,   ///< Geopotential, m^2/s^2
  GRIB_PARAMETER_HGT__    = 7,   ///< Geopotential height, gpm
  GRIB_PARAMETER_DIST_    = 8,   ///< Geometric height, m
  GRIB_PARAMETER_HSTDV    = 9,   ///< Standard deviation of height, m
  GRIB_PARAMETER_TOZNE    = 10,  ///< Total ozone, Dobson
  GRIB_PARAMETER_TMP__    = 11,  ///< Temperature, K
  GRIB_PARAMETER_VTMP_    = 12,  ///< Virtual temperature, K
  GRIB_PARAMETER_POT__    = 13,  ///< Potential temperature, K
  GRIB_PARAMETER_EPOT_    = 14,  ///< Equivalent potential temperature, K
  GRIB_PARAMETER_T_MAX    = 15,  ///< Maximum temperature, K
  GRIB_PARAMETER_T_MIN    = 16,  ///< Minimum temperature, K
  GRIB_PARAMETER_DPT__    = 17,  ///< Dew point temperature, K
  GRIB_PARAMETER_DEPR_    = 18,  ///< Dew point depression, K
  GRIB_PARAMETER_LAPR_    = 19,  ///< Lapse rate, K/m
  GRIB_PARAMETER_VIS__    = 20,  ///< Visibility, m
  GRIB_PARAMETER_RDSP1    = 21,  ///< Radar Spectra (1)
  GRIB_PARAMETER_RDSP2    = 22,  ///< Radar Spectra (2)
  GRIB_PARAMETER_RDSP3    = 23,  ///< Radar Spectra (3)
  GRIB_PARAMETER_PLI__    = 24,  ///< Parcel lifted index (to 500 hPa), K
  GRIB_PARAMETER_TMP_A    = 25,  ///< Temperature anomaly, K
  GRIB_PARAMETER_PRESA    = 26,  ///< Pressure anomaly, Pa
  GRIB_PARAMETER_GP_A_    = 27,  ///< Geopotential height anomaly, gpm
  GRIB_PARAMETER_WVSP1    = 28,  ///< Wave Spectra (1)
  GRIB_PARAMETER_WVSP2    = 29,  ///< Wave Spectra (2)
  GRIB_PARAMETER_WVSP3    = 30,  ///< Wave Spectra (3)
  GRIB_PARAMETER_WDIR_    = 31,  ///< Wind direction (from which blowing), deg
  GRIB_PARAMETER_WIND_    = 32,  ///< Wind speed, m/s
  GRIB_PARAMETER_U_GRD    = 33,  ///< U-component of wind, m/s
  GRIB_PARAMETER_V_GRD    = 34,  ///< V-component of wind, m/s
  GRIB_PARAMETER_STRM_    = 35,  ///< Stream function, m^2/s
  GRIB_PARAMETER_V_POT    = 36,  ///< Velocity potential, m^2/s
  GRIB_PARAMETER_MNTSF    = 37,  ///< Montgomery stream function, m^2/s^2
  GRIB_PARAMETER_SGCVV    = 38,  ///< Sigma coordinate vertical velocity, 1/s
  GRIB_PARAMETER_V_VEL    = 39,  ///< Vertical velocity (pressure), Pa/s
  GRIB_PARAMETER_DZDT_    = 40,  ///< Vertical velocity (geometric), m/s
  GRIB_PARAMETER_ABS_V    = 41,  ///< Absolute vorticity, 1/s
  GRIB_PARAMETER_ABS_D    = 42,  ///< Absolute divergence, 1/s
  GRIB_PARAMETER_REL_V    = 43,  ///< Relative vorticity, 1/s
  GRIB_PARAMETER_REL_D    = 44,  ///< Relative divergence, 1/s
  GRIB_PARAMETER_VUCSH    = 45,  ///< Vertical u-component shear, 1/s
  GRIB_PARAMETER_VVCSH    = 46,  ///< Vertical v-component shear, 1/s
  GRIB_PARAMETER_DIR_C    = 47,  ///< Direction of current, deg
  GRIB_PARAMETER_SP_C_    = 48,  ///< Speed of current, m/s
  GRIB_PARAMETER_UOGRD    = 49,  ///< U-component of current, m/s
  GRIB_PARAMETER_VOGRD    = 50,  ///< V-component of current, m/s
  GRIB_PARAMETER_SPF_H    = 51,  ///< Specific humidity, kg/kg
  GRIB_PARAMETER_R_H__    = 52,  ///< Relative humidity, %
  GRIB_PARAMETER_MIXR_    = 53,  ///< Humidity mixing ratio, kg/kg
  GRIB_PARAMETER_P_WAT    = 54,  ///< Precipitable water, kg/m^2
  GRIB_PARAMETER_VAPP_    = 55,  ///< Vapor pressure, Pa
  GRIB_PARAMETER_SAT_D    = 56,  ///< Saturation deficit, Pa
  GRIB_PARAMETER_EVP__    = 57,  ///< Evaporation, kg/m^2
  GRIB_PARAMETER_C_ICE    = 58,  ///< Cloud Ice, kg/m^2
  GRIB_PARAMETER_PRATE    = 59,  ///< Precipitation rate, kg/m^2/s
  GRIB_PARAMETER_TSTM_    = 60,  ///< Thunderstorm probability, %
  GRIB_PARAMETER_A_PCP    = 61,  ///< Total precipitation, kg/m^2
  GRIB_PARAMETER_NCPCP    = 62,  ///< Large scale precip. (non-conv), kg/m^2
  GRIB_PARAMETER_ACPCP    = 63,  ///< Convective precipitation, kg/m^2
  GRIB_PARAMETER_SRWEQ    = 64,  ///< Snowfall rate water equivalent, kg/m^2/s
  GRIB_PARAMETER_WEASD    = 65,  ///< Water equiv. of accum. snow depth, kg/m^2
  GRIB_PARAMETER_SNO_D    = 66,  ///< Snow depth, m
  GRIB_PARAMETER_MIXHT    = 67,  ///< Mixed layer depth, m
  GRIB_PARAMETER_TTHDP    = 68,  ///< Transient thermocline depth, m
  GRIB_PARAMETER_MTHD_    = 69,  ///< Main thermocline depth, m
  GRIB_PARAMETER_MTH_A    = 70,  ///< Main thermocline anomaly, m
  GRIB_PARAMETER_T_CDC    = 71,  ///< Total cloud cover, %
  GRIB_PARAMETER_CDCON    = 72,  ///< Convective cloud cover, %
  GRIB_PARAMETER_L_CDC    = 73,  ///< Low cloud cover, %
  GRIB_PARAMETER_M_CDC    = 74,  ///< Medium cloud cover, %
  GRIB_PARAMETER_H_CDC    = 75,  ///< High cloud cover, %
  GRIB_PARAMETER_C_WAT    = 76,  ///< Cloud water, kg/m^2
  GRIB_PARAMETER_BLI__    = 77,  ///< Best lifted index (to 500 hPa), K
  GRIB_PARAMETER_SNO_C    = 78,  ///< Convective snow, kg/m^2
  GRIB_PARAMETER_SNO_L    = 79,  ///< Large scale snow, kg/m^2
  GRIB_PARAMETER_WTMP_    = 80,  ///< Water Temperature, K
  GRIB_PARAMETER_LAND_    = 81,  ///< Land cover (land=1, sea=0), proportion
  GRIB_PARAMETER_DSL_M    = 82,  ///< Deviation of sea level from mean, m
  GRIB_PARAMETER_SFC_R    = 83,  ///< Surface roughness, m
  GRIB_PARAMETER_ALBDO    = 84,  ///< Albedo, %
  GRIB_PARAMETER_TSOIL    = 85,  ///< Soil temperature, K
  GRIB_PARAMETER_SOILM    = 86,  ///< Soil moisture content, kg/m^2
  GRIB_PARAMETER_VEG__    = 87,  ///< Vegetation, %
  GRIB_PARAMETER_SALTY    = 88,  ///< Salinity, kg/kg
  GRIB_PARAMETER_DEN__    = 89,  ///< Densitym kg/m^3
  GRIB_PARAMETER_WATR_    = 90,  ///< Water runoff, kg/m^2
  GRIB_PARAMETER_ICE_C    = 91,  ///< Ice cover (ice=1, no ice=0), proportion
  GRIB_PARAMETER_ICETK    = 92,  ///< Ice thickness, m
  GRIB_PARAMETER_DICED    = 93,  ///< Direction of ice drift, deg
  GRIB_PARAMETER_SICED    = 94,  ///< Speed of ice drift, m/s
  GRIB_PARAMETER_U_ICE    = 95,  ///< U-component of ice drift, m/s
  GRIB_PARAMETER_V_ICE    = 96,  ///< V-component of ice drift, m/s
  GRIB_PARAMETER_ICE_G    = 97,  ///< Ice growth rate, m/s
  GRIB_PARAMETER_ICE_D    = 98,  ///< Ice divergence, 1/s
  GRIB_PARAMETER_SNO_M    = 99,  ///< Snow melt, kg/m^2
  GRIB_PARAMETER_HTSGW    = 100, ///< Significant height wind, waves, swell, m
  GRIB_PARAMETER_WVDIR    = 101, ///< Direction of wind waves (from which), deg
  GRIB_PARAMETER_WVHGT    = 102, ///< Significant height of wind waves, m
  GRIB_PARAMETER_WVPER    = 103, ///< Mean period of wind waves, s
  GRIB_PARAMETER_SWDIR    = 104, ///< Direction of swell waves, deg
  GRIB_PARAMETER_SWELL    = 105, ///< Significant height of swell waves,m
  GRIB_PARAMETER_SWPER    = 106, ///< Mean period of swell waves, s
  GRIB_PARAMETER_DIRPW    = 107, ///< Primary wave direction, deg
  GRIB_PARAMETER_PERPW    = 108, ///< Primary wave mean period, s
  GRIB_PARAMETER_DIRSW    = 109, ///< Secondary wave direction, deg
  GRIB_PARAMETER_PERSW    = 110, ///< Secondary wave mean period, s
  GRIB_PARAMETER_NSWRS    = 111, ///< Net short-wave radiation flx (surf), W/m^2
  GRIB_PARAMETER_NLWRS    = 112, ///< Net long wave radiation flux (surf), W/m^2
  GRIB_PARAMETER_NSWRT    = 113, ///< Net short-wave radiation flux (top), W/m^2
  GRIB_PARAMETER_NLWRT    = 114, ///< Net long wave radiation flux (top), W/m^2
  GRIB_PARAMETER_LWAVR    = 115, ///< Long wave radiation flux, W/m^2
  GRIB_PARAMETER_SWAVR    = 116, ///< Short wave radiation flux, W/m^2
  GRIB_PARAMETER_G_RAD    = 117, ///< Global radiation flux, W/m^2
  GRIB_PARAMETER_BRTMP    = 118, ///< Brightness temperature, K
  GRIB_PARAMETER_LWRAD    = 119, ///< Radiance (wave number), W/m/sr
  GRIB_PARAMETER_SWRAD    = 120, ///< Radiance (wave length), W/m^3/sr
  GRIB_PARAMETER_LHTFL    = 121, ///< Latent heat net flu, W/m^2
  GRIB_PARAMETER_SHTFL    = 122, ///< Sensible heat net flux, W/m^2
  GRIB_PARAMETER_BLYDP    = 123, ///< Boundary layer dissipation, W/m^2
  GRIB_PARAMETER_U_FLX    = 124, ///< Momentum flux, U component, N/m^2
  GRIB_PARAMETER_V_FLX    = 125, ///< Momentum flux, V component, N/m^2
  GRIB_PARAMETER_WMIXE    = 126, ///< Wind mixing energy, J
  GRIB_PARAMETER_IMG_D    = 127, ///< Image data
  GRIB_PARAMETER_LOCALUSE = 128, ///< Reserved for local use up to 254
  GRIB_PARAMETER_MISSING  = 255  ///< Missing parameter definition
} t_enum_GRIB_PARAMETER_TABLE_VERSION2;

/// Time units
typedef enum t_enum_GRIB_TIMEUNIT {
  GRIB_TIMEUNIT_UNKNOWN = -1,  ///< software internal use
  GRIB_TIMEUNIT_MINUTE  = 0,   ///< minute
  GRIB_TIMEUNIT_HOUR    = 1,   ///< hour
  GRIB_TIMEUNIT_DAY     = 2,   ///< day
  GRIB_TIMEUNIT_MONTH   = 3,   ///< month
  GRIB_TIMEUNIT_YEAR    = 4,   ///< year
  GRIB_TIMEUNIT_DECADE  = 5,   ///< 10 years
  GRIB_TIMEUNIT_NORMAL  = 6,   ///< 30 years
  GRIB_TIMEUNIT_CENTURY = 7,   ///< century
  GRIB_TIMEUNIT_HOURS3  = 10,  ///< 3 hours
  GRIB_TIMEUNIT_HOURS6  = 11,  ///< 6 hours
  GRIB_TIMEUNIT_HOURS12 = 12,  ///< 12 hours
  GRIB_TIMEUNIT_SECOND  = 254  ///< seconds
} t_enum_GRIB_TIMEUNIT;

/// Time ranges
typedef enum t_enum_GRIB_TIMERANGE {
  GRIB_TIMERANGE_UNKNOWN                                                  = -1,
  GRIB_TIMERANGE_FORECAST_AT_REFTIME_PLUS_P1                              = 0,
  GRIB_TIMERANGE_ANALYSIS_AT_REFTIME                                      = 1,
  GRIB_TIMERANGE_VALID_IN_REFTIME_PLUS_P1_REFTIME_PLUS_P2                 = 2,
  GRIB_TIMERANGE_AVERAGE_IN_REFTIME_PLUS_P1_REFTIME_PLUS_P2               = 3,
  GRIB_TIMERANGE_ACCUMULATED_INTERVAL_REFTIME_PLUS_P1_REFTIME_PLUS_P2     = 4,
  GRIB_TIMERANGE_DIFFERENCE_REFTIME_PLUS_P2_REFTIME_PLUS_P1               = 5,
  GRIB_TIMERANGE_AVERAGE_IN_REFTIME_MINUS_P1_REFTIME_MINUS_P2             = 6,
  GRIB_TIMERANGE_AVERAGE_IN_REFTIME_MINUS_P1_REFTIME_PLUS_P2              = 7,
  GRIB_TIMERANGE_VALID_AT_REFTIME_PLUS_P1P2                               = 10,
  GRIB_TIMERANGE_CLIMATOLOGICAL_MEAN_OVER_MULTIPLE_YEARS_FOR_P2           = 51,
  GRIB_TIMERANGE_AVERAGE_OVER_FORECAST_OF_PERIOD_P1_REFTIME_PERIOD_P2     = 113,
  GRIB_TIMERANGE_ACCUMULATED_OVER_FORECAST_PERIOD_P1_REFTIME_PERIOD_P2    = 114,
  GRIB_TIMERANGE_AVERAGE_OVER_FORECAST_OF_PERIOD_P1_AT_INTERVALS_P2       = 115,
  GRIB_TIMERANGE_ACCUMULATION_OVER_FORECAST_PERIOD_P1_AT_INTERVALS_P2     = 116,
  GRIB_TIMERANGE_AVERAGE_OVER_FORECAST_FIRST_P1_OTHER_P2_REDUCED          = 117,
  GRIB_TIMERANGE_VARIANCE_OF_ANALYSES_WITH_REFERENCE_TIME_INTERVALS_P2    = 118,
  GRIB_TIMERANGE_STDDEV_OF_FORECASTS_FIRST_P1_OTHER_P2_REDUCED            = 119,
  GRIB_TIMERANGE_AVERAGE_OVER_ANALYSES_AT_INTERVALS_OF_P2                 = 123,
  GRIB_TIMERANGE_ACCUMULATION_OVER_ANALYSES_AT_INTERVALS_OF_P2            = 124,
  GRIB_TIMERANGE_STDDEV_OF_FORECASTS_RESPECT_TO_AVERAGE_OF_TENDENCY       = 125,
  GRIB_TIMERANGE_AVERAGE_OF_DAILY_FORECAST_ACCUMULATIONS                  = 128,
  GRIB_TIMERANGE_AVERAGE_OF_SUCCESSIVE_FORECAST_ACCUMULATIONS             = 129,
  GRIB_TIMERANGE_AVERAGE_OF_DAILY_FORECAST_AVERAGES                       = 130,
  GRIB_TIMERANGE_AVERAGE_OF_SUCCESSIVE_FORECAST_AVERAGES                  = 131
} t_enum_GRIB_TIMERANGE;

/// Implemented grid definitions
typedef enum t_enum_GRIB_GRIDS {
  GRIB_GRID_UNKNOWN                = -1,
  GRIB_GRID_REGULAR_LATLON         = 0,
  GRIB_GRID_MERCATOR               = 1,
  GRIB_GRID_GNOMONIC               = 2,
  GRIB_GRID_LAMBERT_CONFORMAL      = 3,
  GRIB_GRID_GAUSSIAN               = 4,
  GRIB_GRID_POLAR_STEREOGRAPHIC    = 5,
  GRIB_GRID_UTM                    = 6,
  GRIB_GRID_SIMPLE_POLYCONIC       = 7,
  GRIB_GRID_ALBERS_EQUAL_AREA      = 8,
  GRIB_GRID_MILLER_CYLINDRICAL     = 9,
  GRIB_GRID_ROTATED_LATLON         = 10,
  GRIB_GRID_OBLIQUE_LAMBERT        = 13,
  GRIB_GRID_ROTATED_GAUSSIAN       = 14,
  GRIB_GRID_STRETCHED_LATLON       = 20,
  GRIB_GRID_STRETCHED_GAUSSIAN     = 24,
  GRIB_GRID_STRETCHED_ROT_LATLON   = 30,
  GRIB_GRID_STRETCHED_ROT_GAUSSIAN = 34,
  GRIB_GRID_SPHERICAL_HARMONIC_COE = 50,
  GRIB_GRID_ROTATED_SPHER_HARM_COE = 60,
  GRIB_GRID_STRETCH_SPHER_HARM_COE = 70,
  GRIB_GRID_STRETCHED_ROT_SP_H_COE = 80,
  GRIB_GRID_SPACEVIEW              = 90
} t_enum_GRIB_GRIDS;

/// Level codes
typedef enum t_enum_GRIB_LEVELS {
  GRIB_LEVEL_UNKNOWN = -1,
  GRIB_LEVEL_RESERVED = 0,
  GRIB_LEVEL_SURFACE,
  GRIB_LEVEL_CLOUD_BASE,
  GRIB_LEVEL_CLOUD_TOP,
  GRIB_LEVEL_ISOTHERM_0_DEG,
  GRIB_LEVEL_ADIABATIC_CONDENSATION_LIFTED_FROM_SURFACE,
  GRIB_LEVEL_MAXIMUM_WIND,
  GRIB_LEVEL_TROPOPAUSE,
  GRIB_LEVEL_NOMINAL_ATMOSPHERE_TOP,
  GRIB_LEVEL_SEA_BOTTOM,
  GRIB_LEVEL_ISOTHERMAL_K = 20,
  GRIB_LEVEL_SATELLITE_METEOSAT7 = 54,
  GRIB_LEVEL_SATELLITE_METEOSAT8 = 55,
  GRIB_LEVEL_SATELLITE_METEOSAT9 = 56,
  GRIB_LEVEL_ISOBARIC_mb = 100,
  GRIB_LEVEL_LAYER_ISOBARIC_mb,
  GRIB_LEVEL_MEAN_SEA_LEVEL,
  GRIB_LEVEL_ALTITUDE_ABOVE_MSL_m,
  GRIB_LEVEL_LAYER_ALTITUDE_ABOVE_MSL_m,
  GRIB_LEVEL_HEIGHT_ABOVE_GROUND_m,
  GRIB_LEVEL_LAYER_HEIGHT_ABOVE_GROUND_m,
  GRIB_LEVEL_SIGMA,
  GRIB_LEVEL_LAYER_SIGMA,
  GRIB_LEVEL_HYBRID,
  GRIB_LEVEL_LAYER_HYBRID,
  GRIB_LEVEL_DEPTH_BELOW_SURFACE_cm,
  GRIB_LEVEL_LAYER_DEPTH_BELOW_SURFACE_cm,
  GRIB_LEVEL_ISENTROPIC_K,
  GRIB_LEVEL_LAYER_ISENTROPIC_K,
  GRIB_LEVEL_PRESSURE_DIFFERENCE_FROM_GROUND_mb,
  GRIB_LEVEL_LAYER_PRESSURE_DIFFERENCE_FROM_GROUND_mb,
  GRIB_LEVEL_POTENTIAL_VORTICITY_SURFACE_PV_UNITS,
  GRIB_LEVEL_ETA,
  GRIB_LEVEL_LAYER_ETA,
  GRIB_LEVEL_LAYER_ISOBARIC_HIGH_PRECISION_mb,
  GRIB_LEVEL_HEIGHT_ABOVE_GROUND_HIGH_PRECISION_cm,
  GRIB_LEVEL_ISOBARIC_Pa,
  GRIB_LEVEL_LAYER_SIGMA_HIGH_PRECISION,
  GRIB_LEVEL_LAYER_ISOBARIC_MIXED_PRECISION_mb,
  GRIB_LEVEL_DEPTH_BELOW_SEA_m = 160,
  GRIB_LEVEL_ENTIRE_ATMOSPHERE = 200,
  GRIB_LEVEL_ENTIRE_OCEAN,
  GRIB_LEVEL_SPECIAL = 204
} t_enum_GRIB_LEVELS;

/// Local Use for MSG (Meteosat8) satellite PDS extension
typedef struct
{
  unsigned short spc;
  unsigned char chn;
  unsigned char pad;
  float sublon;
  int npix;
  int nlin;
  int cfac;
  int lfac;
  int coff;
  int loff;
  float sh;
  float cal_offset;
  float cal_slope;
} GRIB_MSG_PDS;

// ############################################################################
// Interface to time definition
// ############################################################################

///
/// @brief Interface to time description as stored in GRIB version 1 files
///
/// All string times are encoded as in
///
/// <pre>
///                 YYYY-MM-DD hh:mm:ss TZD
///                                         
///          ISO8601 date   ISO8601 time  time zone designator (UTC)
/// </pre>
///
class GRIB_TIME {
  public:
    /// Constructor
    GRIB_TIME( );

    /// @brief Decode time from pds. Not to be used.
    /// @param pds: The encoded pds
    void Decode(unsigned char *pds);

    /// Encode time to 13 characters. Not to be used.
    unsigned char *Encode( );

    /// @brief Set time. Complete interface.
    /// @param year: year 4 digit
    /// @param month: month 0-12
    /// @param day: day 1-31
    /// @param hour: hour 0-23
    /// @param minute: minute 0-59
    /// @param tu: time unit as in t_enum_GRIB_TIMEUNIT
    /// @param tr: time range as in t_enum_GRIB_TIMERANGE
    /// @param time1: first time (time from reference or interval start)
    /// @param time2: interval end (set 0 if not interval)
    /// @param nave: number of averaged/accumulated
    /// @param nmiss: number of missing in average/accumulation 
    void set(int year, int month, int day, int hour, int minute,
             t_enum_GRIB_TIMEUNIT tu, t_enum_GRIB_TIMERANGE tr,
             int time1, int time2, int nave, int nmiss);

    /// @brief Set reference time
    /// @param year: year 4 digit
    /// @param month: month 0-12
    /// @param day: day 1-31
    /// @param hour: hour 0-23
    /// @param minute: minute 0-59
    void set_referencetime(int year, int month, int day, int hour, int minute);

    /// Set analysis at reference time
    void set_analysis( );

    /// @brief Set forecast at reference time + hforecast hours
    /// @param hforecast: number of hours of forecast from reference time
    void set_forecast_hour(int hforecast);

    /// Obtain reference time
    std::string Reftime( );

    /// Obtain reference time as "2000-01-01 00:00:00 UTC"
    std::string Reftime2000( );

    /// Obtain current time as a full descriptive string
    std::string TimeString( );

    /// \brief Obtain current time as "2000-01-01 00:00:00 UTC".
    /// Note this is menaingful only for analysis and forecasts.
    std::string ValidTime( );

    /// Obtain forecast seconds from reference time
    time_t ForecastSeconds( );

    /// Obtain forecast seconds from 2000-01-01 00:00:00 UTC
    time_t ForecastSeconds2000( );

    /// Used timeunit
    t_enum_GRIB_TIMEUNIT timeunit;

    /// Used timerange
    t_enum_GRIB_TIMERANGE timerange;

    /// overloaded output to stream operator
    friend std::ostream& operator<< ( std::ostream& os, GRIB_TIME &t );

    /// reference reference time string
    std::string reftime;

    /// reference time string
    std::string timestring;

    /// Local storage for year
    int year;
    /// Local storage for month
    int month;
    /// Local storage for day
    int day;
    /// Local storage for hour
    int hour;
    /// Local storage for minute
    int minute;
    /// Local sorage for encoded time
    int p1;
    /// Local sorage for encoded time
    int p2;
    /// Local sorage for encoded time
    int p1p2;
    /// Local sorage for encoded time
    int naveraged, nmissing;

  /// Undocumented interfaces
  private:
    void set_time(t_enum_GRIB_TIMEUNIT tu, t_enum_GRIB_TIMERANGE tr, int time);
    void set_time(t_enum_GRIB_TIMEUNIT tu, t_enum_GRIB_TIMERANGE tr,
                  int time1, int time2);
    void set_time(t_enum_GRIB_TIMEUNIT tu, t_enum_GRIB_TIMERANGE tr,
                  int time1, int time2, int nave, int nmiss);
    char *timeunitstr( );
    void strref( );
    void strtime( );
};

// ############################################################################
// Interface to level definition
// ############################################################################

///
/// @brief Interface to level description as stored in GRIB version 1 files
///
class GRIB_LEVEL {
  public:
    /// Constructor
    GRIB_LEVEL( );

    /// @brief Decode level from pds. Not to be used.
    /// @param pds: Encoded pds from message
    void Decode(unsigned char *pds);

    /// Encode level to single integer value. Not to be used.
    int Encode( );

    /// level type
    t_enum_GRIB_LEVELS type;

    /// level value
    float lv1;
    /// level value (for layers)
    float lv2;

    /// Level is pressure?
    bool is_LevelPressure( );
    /// Level is height ?
    bool is_LevelHeight( );
    /// Level is surface ?
    bool is_Surface( );
    /// Level is MSL ?
    bool is_MeanSeaLevel( );

    /// @brief Set level as pressure level at specified pressure in mb
    /// @param plev: Pressure value in mb
    void set_pressure(float plev);

    /// @brief Set level as height level at specified height in m
    /// @param hgt: Height above ground value in m
    void set_height(float hgt);

    /// @brief Set level as sea depth level at specified depth in m
    /// @param dpt: Sea level depth in m
    void set_seadepth(float dpt);

    /// @brief Set level as isentropic level at specified temperature in K
    /// @param temp: Isentropic temperature in K
    void set_isentropic(float temp);

    /// @brief Set level as sigma level at specified value
    /// @param sigma: sigma value
    void set_sigma(float sigma);

    /// Set level as surface
    void set_surface( );

    /// @brief Set Level. Complete interface.
    /// @param type: type of level as in t_enum_GRIB_LEVELS
    /// @param lev1: Level or first level if layer
    /// @param lev2: Second level if layer
    void set(t_enum_GRIB_LEVELS type, float lev1, float lev2);

    /// Overloaded output to stream
    friend std::ostream& operator<< ( std::ostream& os, GRIB_LEVEL &l );

  private:
    void set(t_enum_GRIB_LEVELS type);
    void set(t_enum_GRIB_LEVELS type, float lev);
    std::string leveldesc( );
};

// ############################################################################
// Grid definitions
// ############################################################################

///
/// @brief Regular lat/lon grid parameters
///
class GRIB_GRID_regular_latlon {
  public:
    /// @brief Set grid parameters
    /// @param lat1: Latitude of first point
    /// @param lat2: Latitude of last point
    /// @param lon1: Longitude of first point
    /// @param lon2: Longitude of last point
    /// @param dlat: Latitude directional increment
    /// @param dlon: Longitude directional increment
    void set(float lat1, float lat2,
             float lon1, float lon2,
             float dlat, float dlon);

    float lat1;  ///< Latitude of first point
    float lat2;  ///< Latitude of last point
    float lon1;  ///< Longitude of first point
    float lon2;  ///< Longitude of last point
    float dlat;  ///< Latitude directional increment
    float dlon;  ///< Longitudinal directional increment

    /// Overloaded output to stream
    friend std::ostream& operator<< ( std::ostream& os,
                                      GRIB_GRID_regular_latlon &g );
};

///
/// @brief Gaussian latitude/longitude grid
///
class GRIB_GRID_gaussian_latlon {
  public:
    /// @brief Set grid parameters
    /// @param lat1: Latitude of first point
    /// @param lon1: Longitude of first point
    /// @param lat2: Latitude of last point
    /// @param lon2: Longitude of last point
    /// @param dlon: Longitude directional increment
    /// @param N: number of latitude circles between a pole and the equator
    void set(float lat1, float lat2,
             float lon1, float lon2,
             float dlon, float N);

    float lat1;  ///< Latitude of first point
    float lat2;  ///< Latitude of last point
    float lon1;  ///< Longitude of first point
    float lon2;  ///< Longitude of last point
    float dlon;  ///< Longitudinal directional increment
    float N;     ///< number of latitude circles between a pole and the equator

    /// Overloaded output to stream
    friend std::ostream& operator<< ( std::ostream& os,
                                      GRIB_GRID_gaussian_latlon &g );
};

///
/// @brief Polar Stereographic grid parameters
///
class GRIB_GRID_polar_stereographic {
  public:
    /// @brief Set grid parameters
    /// @param lat1: Latitude of first point
    /// @param lon1: Longitude of first point
    /// @param lov: Grid orientation: longitude parallel to y-axis
    /// @param dx: WE resolution in Km
    /// @param dy: SN resolution in Km
    /// @param pole: is North Pole on projection plan ?
    void set(float lat1, float lon1, float lov,
             float dx, float dy, bool pole);

    float lat1; ///< Latitude of first point
    float lon1; ///< Longitude of first point
    float lov;  ///< Grid orientation: longitude parallel to y-axis
    float dx;   ///< WE resolution in Km
    float dy;   ///< SN resolution in Km
    bool pole;  ///< is North Pole on projection plan

    /// Overloaded output to stream
    friend std::ostream& operator<< ( std::ostream& os,
                                      GRIB_GRID_polar_stereographic &g );
};

///
/// @brief Lambert conformal grid parameters
///
class GRIB_GRID_lambert_conformal {
  public:
    /// @brief Set grid parameters
    /// @param lat1: Latitude of first point
    /// @param lon1: Longitude of first point
    /// @param lov: Grid orientation: longitude parallel to y-axis
    /// @param latin1: First secant latitude
    /// @param latin2: Second secant latitude (= first if tangent)
    /// @param latsp: Latitude of south pole
    /// @param lonsp: Longitude of south pole
    /// @param dx: WE resolution in Km
    /// @param dy: SN resolution in Km
    /// @param pole: is North Pole on projection plan ?
    /// @param bipolar: is projection bipolar and symmetric ?
    void set(float lat1, float lon1, float lov,
             float latin1, float latin2,
             float latsp, float lonsp,
             float dx, float dy,
             bool pole, bool bipolar);

    float lat1;    ///< Latitude of first grid point
    float lon1;    ///< Longitude of first grid point
    float lov;     ///< Grid orientation: longitude parallel to y-axis
    float latin1;  ///< First secant latitude
    float latin2;  ///< Second secant latitude (= first if tangent)
    float latsp;   ///< Latitude of south pole
    float lonsp;   ///< Longitude of south pole
    float dx;      ///< WE resolution in Km
    float dy;      ///< SN resolution in Km
    bool pole;     ///< is North Pole on projection plan
    bool bipolar;  ///< is projection bipolar and symmetric

    /// Overloaded output to stream
    friend std::ostream& operator<< ( std::ostream& os,
                                      GRIB_GRID_lambert_conformal &g );
};

///
/// @brief Mercator grid parameters
///
class GRIB_GRID_mercator {
  public:
    /// @brief Set grid parameters
    /// @param lat1: Latitude of first point
    /// @param lon1: Longitude of first point
    /// @param lat2: Latitude of last point
    /// @param lon2: Longitude of last point
    /// @param dx: WE resolution in Km
    /// @param dy: SN resolution in Km
    /// @param latin: Latitude of intersection with earth
    void set(float lat1, float lon1,
             float lat2, float lon2,
             float dx, float dy, float latin);

    float lat1;   ///< Latitude of first grid point
    float lon1;   ///< Longitude of first grid point
    float lat2;   ///< Latitude of last grid point
    float lon2;   ///< Longitude of last grid point
    float dx;     ///< WE resolution in Km
    float dy;     ///< SN resolution in Km
    float latin;  ///< Latitude of intersection with earth

    /// Overloaded output to stream
    friend std::ostream& operator<< ( std::ostream& os,
                                      GRIB_GRID_mercator &g );
};

///
/// @brief Space View grid (meteorological satellites)
///
class GRIB_GRID_spaceview {
  public:
    /// @brief Set grid parameters
    /// @param lap: Latitude of sub-satellite point
    /// @param lop: Longitude of sub-satellite point
    /// @param dx: Apparent diameter of earth in grid lenght, X direction
    /// @param dy: Apparent diameter of earth in grid lenght, Y direction
    /// @param Xp: X-coordinate of sub-satellite point
    /// @param Yp: Y-coordinate of sub-satellite point
    /// @param orient: Grid orientation in mdeg
    /// @param Nr: Camera altitude from earth center in ER units
    /// @param X0: X coordinate of origin of sector image.
    /// @param Y0: Y coordinate of origin of sector image.
    void set(float lap, float lop, int dx, int dy,
             float Xp, float Yp, float orient, float Nr, float X0, float Y0);

    float lap;     ///< Latitude of sub-satellite point
    float lop;     ///< Longitude of sub-satellite point
    int dx;      ///< Apparent diameter of earth in grid lenght, X direction
    int dy;      ///< Apparent diameter of earth in grid lenght, Y direction
    float Xp;      ///< X-coordinate of sub-satellite point
    float Yp;      ///< Y-coordinate of sub-satellite point
    float orient;  ///< Grid orientation in mdeg
    float Nr;      ///< Camera altitude from earth center in ER units
    float X0;      ///< X coordinate of origin of sector image.
    float Y0;      ///< Y coordinate of origin of sector image.

    /// Overloaded output to stream
    friend std::ostream& operator<< ( std::ostream& os,
                                      GRIB_GRID_spaceview &g );
};

///
///  @brief Spherical Armonic coefficients
///
class GRIB_GRID_spherical_harmonic {
  public:
    /// @brief Set grid parameters
    /// @param J: J pentagonal resolution parameter
    /// @param K: K pentagonal resolution parameter
    /// @param M: M pentagonal resolution parameter
    /// @param rt: Representation type
    /// @param csm: Coefficient storage method
    void set(int J, int K, int M, int rt, int csm);

    /// Representation type
    enum GRIB_GRID_SPECTRAL_REPRESENTATION
    {
      SR_associated_legendre_polynomials_first_kind = 1,
      SR_spherical_harmonics_complex_packing        = 2
    } representation;

    /// Coefficient storage method
    enum GRIB_GRID_COEFFICIENT_STORAGE_METHOD
    {
      CSM_normal  = 1,
      CSM_complex = 2
    } storage_method;

    /// Pentagonal resolution parameter (truncation NJ)
    int J;
    /// Pentagonal resolution parameter (truncation NK)
    int K;
    /// Pentagonal resolution parameter (truncation NM)
    int M;

    /// Overloaded output to stream
    friend std::ostream& operator<< ( std::ostream& os,
                                      GRIB_GRID_spherical_harmonic &g );
};

// ############################################################################
// Interface to grid definitions
// ############################################################################

///
/// @brief Interface to grid as defined in GRIB version 1 files
///
class GRIB_GRID {
  public:
    /// Constructor
    GRIB_GRID( );

    /// @brief Decode grid definition from all message parts. Do not use.
    /// @param pds: Encoded pds section of GRIB
    /// @param gds: Encoded gds section of GRIB
    /// @param bms: Encoded bms section of GRIB
    /// @param bds: Encoded bds section of GRIB
    void Decode(unsigned char *pds, unsigned char *gds,
                unsigned char *bms, unsigned char *bds);

    /// Encode grid into a gds section. Do not use.
    unsigned char *Encode( );

    /// @brief Set grid dimensions.
    /// @param nx: WE direction
    /// @param ny: SN direction
    void set_size(int nx, int ny);

    /// @brief Set grid number if standard grid is used. See GRIB documentation
    /// @param code: pre-defined or site specific grid code nmber
    void set_number(int code);

    /// @brief Set parameters of a regular latitude/longitude grid
    /// @param lat1: Latitude of first point
    /// @param lat2: Latitude of last point
    /// @param lon1: Longitude of first point
    /// @param lon2: Longitude of last point
    /// @param dlat: Latitude directional increment
    /// @param dlon: Longitude directional increment
    void set_regular_latlon(float lat1, float lat2,
                            float lon1, float lon2,
                            float dlat, float dlon);

    /// @brief Set parameters of a Gaussian latitude/longitude grid
    /// @param lat1: Latitude of first point
    /// @param lon1: Longitude of first point
    /// @param lat2: Latitude of last point
    /// @param lon2: Longitude of last point
    /// @param dlon: Longitude directional increment
    /// @param N: number of latitude circles between a pole and the equator
    void set_gaussian_latlon(float lat1, float lon1,
                             float lat2, float lon2,
                             float dlon, float N);

    /// @brief Set parameters for a polar stereographic grid
    /// @param lat1: Latitude of first point
    /// @param lon1: Longitude of first point
    /// @param lov: Grid orientation: longitude parallel to y-axis
    /// @param dx: WE resolution in Km
    /// @param dy: SN resolution in Km
    /// @param pole: is North Pole on projection plan ?
    void set_polar_stereo(float lat1, float lon1, float lov,
                          float dx, float dy, bool pole);

    /// @brief Set parameters for a Lambert conformal grid
    /// @param lat1: Latitude of first point
    /// @param lon1: Longitude of first point
    /// @param lov: Grid orientation: longitude parallel to y-axis
    /// @param latin1: First secant latitude
    /// @param latin2: Second secant latitude (= first if tangent)
    /// @param latsp: Latitude of south pole
    /// @param lonsp: Longitude of south pole
    /// @param dx: WE resolution in Km
    /// @param dy: SN resolution in Km
    /// @param pole: is North Pole on projection plan ?
    /// @param bipolar: is projection bipolar and symmetric ?
    void set_lambert_conformal(float lat1, float lon1, float lov,
                               float latin1, float latin2,
                               float latsp, float lonsp,
                               float dx, float dy,
                               bool pole, bool bipolar);

    /// @brief Set parameters for a Mercator grid.
    /// @param lat1: Latitude of first point
    /// @param lon1: Longitude of first point
    /// @param lat2: Latitude of last point
    /// @param lon2: Longitude of last point
    /// @param dx: WE resolution in Km
    /// @param dy: SN resolution in Km
    /// @param latin: Latitude of intersection with earth
    void set_mercator(float lat1, float lon1, float lat2, float lon2,
                      float dx, float dy, float latin);

    /// @brief Set space view projection
    /// @param lap: Latitude of sub-satellite point
    /// @param lop: Longitude of sub-satellite point
    /// @param dx: Apparent diameter of earth in grid lenght, X direction
    /// @param dy: Apparent diameter of earth in grid lenght, Y direction
    /// @param Xp: X-coordinate of sub-satellite point
    /// @param Yp: Y-coordinate of sub-satellite point
    /// @param orient: Grid orientation in mdeg
    /// @param Nr: Camera altitude from earth center in ER units
    /// @param X0: X coordinate of origin of sector image.
    /// @param Y0: Y coordinate of origin of sector image.
    void set_spaceview(float lap, float lop, int dx, int dy,
                       float Xp, float Yp, float orient, float Nr,
                       float X0, float Y0);

    /// @brief Set spherical harmonics coefficients rapresentation
    /// @param J: J pentagonal resolution parameter
    /// @param K: K pentagonal resolution parameter
    /// @param M: M pentagonal resolution parameter
    /// @param rt: Representation type
    /// @param csm: Coefficient storage method
    void set_spherical_harmonic(int J, int K, int M, int rt, int csm);
    
    /// Set earth sphere/spheroid flag
    void set_earth_spheroid( );

    /// Set flag for wind on SN-WE or grid reference.
    void set_uv_grid( );

    /// Reverse the order of X axis
    void set_x_negative( );

    /// Reverse the order of Y axis
    void set_y_negative( );

    /// Fortran indexing: columnar order of grid parameter values
    void set_fortran_indexing( );

    /// Supported grids
    union {
      GRIB_GRID_regular_latlon ll;
      GRIB_GRID_polar_stereographic ps;
      GRIB_GRID_lambert_conformal lc;
      GRIB_GRID_gaussian_latlon gl;
      GRIB_GRID_mercator mc;
      GRIB_GRID_spherical_harmonic sa;
      GRIB_GRID_spaceview sp;
    };

    /// Type of the grid
    t_enum_GRIB_GRIDS type;

    /// @brief Code if grid is defined without gds
    /// Use standard defined grid number as in
    /// <a href="http://www.wmo.ch/web/www/WDM/Guides/Guide-binary-2.html">
    /// GRIB documentation</a>
    int grid_code;

    /// Directional increments are given or not
    bool is_dirincgiven;

    /// Earth assumed sperical or spheroid IAU 1965
    bool is_earth_spheroid;

    /// U-V components of vectors are grid or sphere
    bool is_uv_grid;

    /// Point scan in -i (East-West)
    bool is_x_negative;

    /// Point scan in -j (North-South)
    bool is_y_negative;

    /// Fortran indexing (column scan first)
    bool is_fortran;

    /// Overloaded output to stream
    friend std::ostream& operator<< ( std::ostream& os, GRIB_GRID &g );

    /// Grid size as in grid definition
    long nxny;
    /// Grid size as in grid definition
    int nx;
    /// Grid size as in grid definition
    int ny;

    /// Local storage for gds.
    unsigned char gds[44];
};

// ############################################################################
// Interface to GRIB field
// ############################################################################

///
/// @brief GRIB Field stored/to be stored im message
///
class GRIB_FIELD {
  public:
    /// Constructor
    GRIB_FIELD( );

    /// Destructor
    ~GRIB_FIELD( );

    /// @brief Decode grib parameter stored in message. Do not use.
    /// @param pds: Encoded pds section of GRIB
    /// @param bds: Encoded bds section of GRIB
    /// @param bms: Encoded bms section of GRIB
    /// @param nxny: expexcted field size from grid definition
    void Decode(unsigned char *pds, unsigned char *bds,
                unsigned char *bms, size_t nxny);

    /// @brief Set overall grib identification
    /// @param center: Originating center code
    /// @param subcenter: Originating subcenter code
    /// @param table: Parameter table used in codification
    /// @param process: Process generating parameter (model, satellite, etc)
    void set_table(int center, int subcenter, int table, int process);

    /// @brief Set field from a float values array
    /// @param varcode: Parameter code
    /// @param values: Allocated array filled with parameter values
    /// @param size: Size of the previous array
    /// @param undef_high: High mark for undefined value in encoding
    /// @param undef_low: Low mark for undefined value in encoding
    void set_field(int varcode, float *values, size_t size,
                   float undef_high, float undef_low);

    /// @brief Set field from a float values array, avoiding copying the array by taking ownership of its memory allocation
    /// @param varcode: Parameter code
    /// @param values: Allocated array filled with parameter values
    /// @param size: Size of the previous array
    /// @param undef_high: High mark for undefined value in encoding
    /// @param undef_low: Low mark for undefined value in encoding
    void set_field_nocopy(int varcode, float *values, size_t size,
                   float undef_high, float undef_low);

    /// @brief Add local definitions (PDS reserved originating center)
    /// @param noctets: Number of bytes
    /// @param octets: Local definition
    void add_local_def(int noctets, unsigned char *octets);
    
    /// @brief Set decimal scaling. Stored values will be: sv = val * 10^scale
    /// @param scale: decimal scaling in encode
    void set_scale(int scale);

    /// Originating center
    int center;
    /// Originating subcenter for center
    int subcenter;
    /// center/subcenter grib table used
    int table;
    /// process originating data stored in GRIB (model, satellite, etc.)
    int process;

    /// variable code
    int varcode;

    /// parameter storage size
    size_t size;
    /// parameter values
    float *vals;

    /// variable name from some hardcoded center/subcenter/process/table 
    std::string VarName( );
    /// variable units from some hardcoded center/subcenter/process/table 
    std::string VarUnit( );
    /// variable description from some hardcoded center/subcenter/process/table 
    std::string VarDescription( );

    /// undefined value used in defining bitmap for stored parameter. HI mark
    float undef_high;
    /// undefined value used in defining bitmap for stored parameter. LOW mark
    float undef_low;

    /// parameter name
    std::string varname;
    /// parameter unit
    std::string varunit;
    /// parameter description
    std::string vardesc;

    /// used in decoding for soring packing info
    int numbits;
    /// used in decoding for soring packing info
    float refvalue;
    /// used in decoding/encoding for soring packing info
    int decimalscale;
    /// used in decoding for soring packing info
    int binscale;

    /// PDS reserved octets
    int nmorepds;
    unsigned char *morepds;
};

// ############################################################################
// Interface to GRIB message
// ############################################################################

///
/// @brief Message structure: a grid at single level and single time
///
class GRIB_MESSAGE {
  public:
    /// Constructor
    GRIB_MESSAGE( );

    /// Destructor
    ~GRIB_MESSAGE( );

    /// @brief Decode a message. Not to be used.
    /// @param message: encoded message from input buffer
    /// @param reclen: Size in bytes of the previous array
    int Decode(unsigned char *message, size_t reclen);

    /// Encode a message. Not to be used.
    void Encode( );

    /// @brief Set the grid
    /// @param grid: grid to be encoded
    void set_grid(GRIB_GRID &grid);

    /// @grib Set the grid (using pre-defined grid).
    /// @param code: grid code. See GRIB1 documentation
    void set_grid(int code);

    /// @brief Set time
    /// @param gtime: time to be encoded
    void set_time(GRIB_TIME &gtime);

    /// @brief Set vertical level of grid
    /// @param level: level to be encoded
    void set_level(GRIB_LEVEL &level);

    /// @brief Set the field to be stored.
    /// @param field: field parameter to be encoded
    void set_field(GRIB_FIELD &field);

    /// Overall message record lenght
    size_t reclen;

    /// Encoded message
    unsigned char *message;

    /// The grid (local storage. Filled on set and on ReadMessage)
    GRIB_GRID grid;
    /// The level (local storage. Filled on set and on ReadMessage)
    GRIB_LEVEL level;
    /// The time (local storage. Filled on set and on ReadMessage)
    GRIB_TIME gtime;
    /// The field (local storage. Filled on set and on ReadMessage)
    GRIB_FIELD field;

    /// @brief Extract a portion from the PDS from a non-standard position
    /// @param pos: position in the PDS (start is zero)
    /// @param len: len in bytes 
    unsigned char *get_pds_values(int pos, int len);

    /// @brief Extract PDS lenght
    size_t get_pds_size( );

  private:
    unsigned char *pds;
    unsigned char *gds;
    unsigned char *bms;
    unsigned char *bds;
};

// ############################################################################
// Overall GRIB FILE interface
// ############################################################################

///
/// @brief GRIB level 1 file
///
class GRIB_FILE {
  public:
    /// Constructor
    GRIB_FILE();

    /// Destructor
    ~GRIB_FILE();

    /// @brief Open grib file for read
    /// param fname: file name with full path
    int OpenRead(const std::string& fname);

    /// @brief Open grib file for write. File is truncated
    /// param fname: file name with full path
    int OpenWrite(const std::string& fname);

    // Note from Enrico: OpenRead(char*) and OpenWrite(char*) were deleted
    // because their string counterparts to recursively call themselves

    /// @brief Open grib file for append
    /// param fname: file name with full path
    int Append(char *fname);

    /// @brief Open grib file for append
    /// param fname: file name with full path
    int Append(std::string fname);

    /// @brief Read message from file
    /// @param message: Output decoded message
    int ReadMessage(GRIB_MESSAGE &message);

    /// @brief Write message to file
    /// @param message: Input decoded message
    int WriteMessage(GRIB_MESSAGE &message);

    /// Close previously opened file
    int Close( );

  private:
    int fd;    // input stream pointer
    FILE *fp;  // output stream pointer

    size_t size;  // input total file size
    size_t pos;   // position in file size
    size_t limit; // last message start

    unsigned char *data; // actual input data
};

// ############################################################################
