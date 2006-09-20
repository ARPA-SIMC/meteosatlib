//-----------------------------------------------------------------------------
//
//  File        : GRIB.cpp
//  Description : GRIB File interface implementation
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

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

#include <Ebisuzaki.h>
#include <GRIB.h>

#include <iostream>
#include <sstream>
#include <iomanip>

#define ECMWF_NIGHTMARE_HACK 1

extern long timezone;

// ############################################################################
// GRIB Time implementation
// ############################################################################

GRIB_TIME::GRIB_TIME( )
{
  year = month = day = hour = minute = 0;
  p1 = p2 = p1p2 = naveraged = nmissing = 0;
  timeunit = GRIB_TIMEUNIT_UNKNOWN;
  timerange = GRIB_TIMERANGE_UNKNOWN;
  reftime = "2000-01-01 00:00:00 UTC";
  timestring = "Undefined time";
}

void GRIB_TIME::strref( )
{
  std::stringstream s;
  s << std::setw(4) << std::setfill('0') << year << "-"
    << std::setw(2) << std::setfill('0') << month << "-"
    << std::setw(2) << std::setfill('0') << day << " "
    << std::setw(2) << std::setfill('0') << hour << ":"
    << std::setw(2) << std::setfill('0') << minute << ":00 UTC";
  reftime = s.str( );
  return;
}

void GRIB_TIME::strtime( )
{
  std::stringstream s;
  char *unitstr = timeunitstr( );
  switch (timerange)
  {
    case GRIB_TIMERANGE_FORECAST_AT_REFTIME_PLUS_P1:
      if (p1 == 0)
        s << "Uninitialized analysis product or image for " << reftime;
      else
        s << "Forecast product valid for " << reftime
          << " + " << p1 << " " << unitstr;
      break;
    case GRIB_TIMERANGE_ANALYSIS_AT_REFTIME:
      s << "Initialized analysis product for " << reftime;
      break;
    case GRIB_TIMERANGE_VALID_IN_REFTIME_PLUS_P1_REFTIME_PLUS_P2:
      s << "Product valid between " << reftime
        << " + " << p1 << " " << unitstr << " and" << std::endl
        << "                      " << reftime
        << " + " << p2 << " " << unitstr;
      break;
    case GRIB_TIMERANGE_AVERAGE_IN_REFTIME_PLUS_P1_REFTIME_PLUS_P2:
      s << "Average from " << reftime
        << " + " << p1 << " " << unitstr << " and" << std::endl
        << "             " << reftime << " + " << p2 << " " << unitstr;
      break;
    case GRIB_TIMERANGE_ACCUMULATED_INTERVAL_REFTIME_PLUS_P1_REFTIME_PLUS_P2:
      s << "Accumulated from " << reftime
        << " + " << p1 << " " << unitstr << " and" << std::endl
        << "                 " << reftime << " + " << p2 << " " << unitstr;
      break;
    case GRIB_TIMERANGE_DIFFERENCE_REFTIME_PLUS_P2_REFTIME_PLUS_P1:
      s << "Difference between product at " << reftime
        << " + " << p1 << " " << unitstr << " and" << std::endl
        << "                   product at "
        << reftime << " + " << p2 << " " << unitstr;
      break;
    case GRIB_TIMERANGE_AVERAGE_IN_REFTIME_MINUS_P1_REFTIME_MINUS_P2:
      s << "Average from " << reftime
        << " - " << p1 << " " << unitstr << " and" << std::endl
        << "             " << reftime << " - " << p2 << " " << unitstr;
      break;
    case GRIB_TIMERANGE_AVERAGE_IN_REFTIME_MINUS_P1_REFTIME_PLUS_P2: 
      s << "Average from " << reftime
        << " - " << p1 << " " << unitstr << " and" << std::endl
        << "             " << reftime << " + " << p2 << " " << unitstr;
      break;
    case GRIB_TIMERANGE_VALID_AT_REFTIME_PLUS_P1P2:
      s << "Forecast product valid for " << reftime
        << " + " << p1p2 << " " << unitstr;
      break;
    case GRIB_TIMERANGE_CLIMATOLOGICAL_MEAN_OVER_MULTIPLE_YEARS_FOR_P2:
      s << "Climatological mean of " << naveraged << "/" << nmissing
        << " years from " 
        << reftime << " for " << p2 << " " << unitstr;
      break;
    case GRIB_TIMERANGE_AVERAGE_OVER_FORECAST_OF_PERIOD_P1_REFTIME_PERIOD_P2:
      s << "Average of " << naveraged << "/" << nmissing;
      if (p1 == 0) s << " initialized analyses";
      else s << " forecasts with period " << p1 << " " << unitstr; 
      s << std::endl << "starting at " << reftime << " at intervals of " << p2
        << " " << unitstr;
      break;
    case GRIB_TIMERANGE_ACCUMULATED_OVER_FORECAST_PERIOD_P1_REFTIME_PERIOD_P2:
      s << "Accumulated of " << naveraged << "/" << nmissing;
      if (p1 == 0) s << " initialized analyses";
      else s << " forecasts with period " << p1 << " " << unitstr; 
      s << std::endl << "starting at " << reftime << " at intervals of " << p2
        << " " << unitstr;
      break;
    case GRIB_TIMERANGE_AVERAGE_OVER_FORECAST_OF_PERIOD_P1_AT_INTERVALS_P2:
      s << "Average of " << naveraged << "/" << nmissing
        << " forecasts at " << reftime
        << ", first with period " << p1 << " " << unitstr << std::endl
        << "and the other following at intervals of " << p2 << " " << unitstr; 
      break;
    case GRIB_TIMERANGE_ACCUMULATION_OVER_FORECAST_PERIOD_P1_AT_INTERVALS_P2:
      s << "Accumulation of " << naveraged << "/" << nmissing
        << " forecasts at " << reftime
        << ", first with period " << p1 << " " << unitstr << std::endl
        << "and the other following at intervals of " << p2 << " " << unitstr; 
      break;
    case GRIB_TIMERANGE_AVERAGE_OVER_FORECAST_FIRST_P1_OTHER_P2_REDUCED:
      s << "Average of " << naveraged << "/" << nmissing
        << " forecasts at " << reftime
        << ", first with" << std::endl << "period " << p1 << " " << unitstr
        << " and the other with period reduced by " << p2 << " " << unitstr; 
      break;
    case GRIB_TIMERANGE_VARIANCE_OF_ANALYSES_WITH_REFERENCE_TIME_INTERVALS_P2:
      s << "Temporal variance or covariance of " << naveraged
        << "/" << nmissing
        << " initialized analyses," << std::endl << "starting at "
        << reftime << " with intervals of " << p2 << " " << unitstr;
      break;
    case GRIB_TIMERANGE_STDDEV_OF_FORECASTS_FIRST_P1_OTHER_P2_REDUCED:
      s << "Standard deviation of " << naveraged << "/" << nmissing
        << " forecasts at "
        << reftime << std::endl << "the first with period " << p1
        << " " << unitstr << " and the other following at "
        << p2 << " " << unitstr << " intervals";
      break;
    case GRIB_TIMERANGE_AVERAGE_OVER_ANALYSES_AT_INTERVALS_OF_P2:
      s << "Average of " << naveraged << "/" << nmissing
        << " uninitialized analyses "
        << "starting at " << reftime << "," << std::endl
        << "at intervals of " << p2 << " " << unitstr;
      break;
    case GRIB_TIMERANGE_ACCUMULATION_OVER_ANALYSES_AT_INTERVALS_OF_P2:
      s << "Accumulation of " << naveraged << "/" << nmissing
        << " uninitialized analyses "
        << "starting at " << reftime << "," << std::endl
        << "at intervals of " << p2 << " " << unitstr;
      break;
    case GRIB_TIMERANGE_STDDEV_OF_FORECASTS_RESPECT_TO_AVERAGE_OF_TENDENCY:
      s << "Standard deviation of " << naveraged << "/" << nmissing
        << " forecasts at "
        << reftime << ", with respect" << std::endl
        << "to time average of time tendency." << std::endl
        << "First forecast has period " << p1 << " " << unitstr
        << ", the other follow at intervals of " << p2 << " " << unitstr;
      break;
    case GRIB_TIMERANGE_AVERAGE_OF_DAILY_FORECAST_ACCUMULATIONS:
      s << "Average of " << naveraged << "/" << nmissing
        << " daily forecast accumulation"
        << " for " << reftime << std::endl << "from " << p1 << " " << unitstr
        << " to " << p2 << " " << unitstr << " with 1 day period.";
      break;
    case GRIB_TIMERANGE_AVERAGE_OF_SUCCESSIVE_FORECAST_ACCUMULATIONS:
      s << "Average of " << naveraged << "/" << nmissing
        << " successive forecast accumulation"
        << " for " << reftime << std::endl << "from " << p1 << " " << unitstr
        << " to " << p2 << " " << unitstr << " with " << (p2-p1) << " "
        << unitstr << " period";
      break;
    case GRIB_TIMERANGE_AVERAGE_OF_DAILY_FORECAST_AVERAGES:
      s << "Average of " << naveraged << "/" << nmissing
        << " daily forecast averages"
        << " for " << reftime << std::endl << "from " << p1 << " " << unitstr
        << " to " << p2 << " " << unitstr << " with 1 day period.";
      break;
    case GRIB_TIMERANGE_AVERAGE_OF_SUCCESSIVE_FORECAST_AVERAGES:
      s << "Average of " << naveraged << "/" << nmissing
        << " successive forecast averages"
        << " for " << reftime << std::endl << "from " << p1 << " " << unitstr
        << " to " << p2 << " " << unitstr << " with " << (p2-p1) << " "
        << unitstr << " period";
    default:
      std::cerr << "Unknown time range indicator: " << timerange << std::endl;
      break;
  }
  timestring = s.str( );
  return;
}

void GRIB_TIME::Decode(unsigned char *pds)
{
  year = PDS_Year4(pds);
  month = PDS_Month(pds);
  day = PDS_Day(pds);
  hour = PDS_Hour(pds);
  minute = PDS_Minute(pds);

  timeunit = (t_enum_GRIB_TIMEUNIT) PDS_ForecastTimeUnit(pds);
  p1 = PDS_P1(pds);
  p2 = PDS_P2(pds);
  p1p2 = PDS_P1P2(pds);
  timerange = (t_enum_GRIB_TIMERANGE) PDS_TimeRange(pds);
  naveraged = PDS_NumAve(pds);
  nmissing = PDS_NumMissing(pds);

  strref( );
  strtime( );

  return;
}

unsigned char *GRIB_TIME::Encode( )
{
  static unsigned char time_encoded[13];
  time_encoded[0]  = (year%100);      // pds 13
  time_encoded[1]  = month;           // pds 14
  time_encoded[2]  = day;             // pds 15
  time_encoded[3]  = hour;            // pds 16
  time_encoded[4]  = minute;          // pds 17
  time_encoded[5]  = timeunit;        // pds 18
  time_encoded[6]  = p1;              // pds 19
  time_encoded[7]  = p2;              // pds 20
  time_encoded[8]  = timerange;       // pds 21
  time_encoded[9]  = (naveraged/256); // pds 22
  time_encoded[10] = (naveraged%256); // pds 23
  time_encoded[11] = nmissing;        // pds 24
  time_encoded[12] = (year/100)+1;    // pds 25
  return time_encoded;
}

void GRIB_TIME::set(int year, int month, int day, int hour, int minute,
                    t_enum_GRIB_TIMEUNIT tu, t_enum_GRIB_TIMERANGE tr,
                    int time1, int time2, int nave, int nmiss)
{
  set_referencetime(year, month, day, hour, minute);
  set_time(tu, tr, time1, time2, nave, nmiss);
  return;
}

void GRIB_TIME::set_analysis( )
{
  timeunit = GRIB_TIMEUNIT_HOUR;
  timerange = GRIB_TIMERANGE_ANALYSIS_AT_REFTIME;
  p1 = p2 = p1p2 = naveraged = nmissing = 0;
  strtime( );
}

void GRIB_TIME::set_forecast_hour(int hforecast)
{
  if (hforecast < 0)
  {
    std::cerr << "Negative or zero lenght forecast ????" << std::endl;
    throw;
  }

  timeunit = GRIB_TIMEUNIT_HOUR;
  timerange = GRIB_TIMERANGE_FORECAST_AT_REFTIME_PLUS_P1;
  p1 = hforecast;
  p2 = p1p2 = 0;
  if (hforecast > 255)
  {
    timerange = GRIB_TIMERANGE_VALID_AT_REFTIME_PLUS_P1P2;
    p1p2 = hforecast;
    p1 = p1p2/256;
    p2 = p1p2%256;
  }
  naveraged = nmissing = 0;
  strtime( );
}

void GRIB_TIME::set_referencetime(int year, int month, int day,
                                  int hour, int minute)
{
  this->year = year;
  this->month = month;
  this->day = day;
  this->hour = hour;
  this->minute = minute;
  strref( );
  return;
}

void GRIB_TIME::set_time(t_enum_GRIB_TIMEUNIT tu, t_enum_GRIB_TIMERANGE tr,
                         int time)
{
  this->set_time(tu, tr, time, 0, 0, 0);
  strtime( );
  return;
}

void GRIB_TIME::set_time(t_enum_GRIB_TIMEUNIT tu, t_enum_GRIB_TIMERANGE tr,
                         int time1, int time2)
{
  this->set_time(tu, tr, time1, time2, 0, 0);
  strtime( );
  return;
}

void GRIB_TIME::set_time(t_enum_GRIB_TIMEUNIT tu, t_enum_GRIB_TIMERANGE tr,
                         int time1, int time2, int nave, int nmiss)
{
  timeunit = tu;
  timerange = tr;
  switch (timerange)
  {
    case GRIB_TIMERANGE_FORECAST_AT_REFTIME_PLUS_P1:
      p1 = time1;
      break;
    case GRIB_TIMERANGE_ANALYSIS_AT_REFTIME:
      break;
    case GRIB_TIMERANGE_AVERAGE_IN_REFTIME_PLUS_P1_REFTIME_PLUS_P2:
    case GRIB_TIMERANGE_ACCUMULATED_INTERVAL_REFTIME_PLUS_P1_REFTIME_PLUS_P2:
    case GRIB_TIMERANGE_DIFFERENCE_REFTIME_PLUS_P2_REFTIME_PLUS_P1:
    case GRIB_TIMERANGE_AVERAGE_IN_REFTIME_MINUS_P1_REFTIME_MINUS_P2:
    case GRIB_TIMERANGE_AVERAGE_IN_REFTIME_MINUS_P1_REFTIME_PLUS_P2: 
      p1 = time1;
      p2 = time2;
      break;
    case GRIB_TIMERANGE_VALID_AT_REFTIME_PLUS_P1P2:
      p1 = time1/256;
      p2 = time1%256;
      p1p2 = time1;
      break;
    case GRIB_TIMERANGE_CLIMATOLOGICAL_MEAN_OVER_MULTIPLE_YEARS_FOR_P2:
    case GRIB_TIMERANGE_AVERAGE_OVER_FORECAST_OF_PERIOD_P1_REFTIME_PERIOD_P2:
    case GRIB_TIMERANGE_ACCUMULATED_OVER_FORECAST_PERIOD_P1_REFTIME_PERIOD_P2:
    case GRIB_TIMERANGE_AVERAGE_OVER_FORECAST_OF_PERIOD_P1_AT_INTERVALS_P2:
    case GRIB_TIMERANGE_ACCUMULATION_OVER_FORECAST_PERIOD_P1_AT_INTERVALS_P2:
    case GRIB_TIMERANGE_AVERAGE_OVER_FORECAST_FIRST_P1_OTHER_P2_REDUCED:
    case GRIB_TIMERANGE_STDDEV_OF_FORECASTS_FIRST_P1_OTHER_P2_REDUCED:
    case GRIB_TIMERANGE_STDDEV_OF_FORECASTS_RESPECT_TO_AVERAGE_OF_TENDENCY:
    case GRIB_TIMERANGE_AVERAGE_OF_DAILY_FORECAST_ACCUMULATIONS:
    case GRIB_TIMERANGE_AVERAGE_OF_DAILY_FORECAST_AVERAGES:
    case GRIB_TIMERANGE_AVERAGE_OF_SUCCESSIVE_FORECAST_ACCUMULATIONS:
    case GRIB_TIMERANGE_AVERAGE_OF_SUCCESSIVE_FORECAST_AVERAGES:
      p1 = time1;
      p2 = time2;
      naveraged = nave;
      nmissing = nmiss;
      break;
    case GRIB_TIMERANGE_VARIANCE_OF_ANALYSES_WITH_REFERENCE_TIME_INTERVALS_P2:
    case GRIB_TIMERANGE_AVERAGE_OVER_ANALYSES_AT_INTERVALS_OF_P2:
    case GRIB_TIMERANGE_ACCUMULATION_OVER_ANALYSES_AT_INTERVALS_OF_P2:
      p1 = 0;
      p2 = time2;
      naveraged = nave;
      nmissing = nmiss;
      break;
    default:
      std::cerr << "Unknown time range indicator: " << timerange << std::endl;
      break;
  }
  strtime( );
  return;
}

char *GRIB_TIME::timeunitstr( )
{
  switch (timeunit)
  {
    case GRIB_TIMEUNIT_MINUTE:
      return "minute";
      break;
    case GRIB_TIMEUNIT_HOUR:
      return "hour";
      break;
    case GRIB_TIMEUNIT_DAY:
      return "day";
      break;
    case GRIB_TIMEUNIT_MONTH:
      return "month";
      break;
    case GRIB_TIMEUNIT_YEAR:
      return "year";
      break;
    case GRIB_TIMEUNIT_DECADE:
      return "decade";
      break;
    case GRIB_TIMEUNIT_NORMAL:
      return "normal";
      break;
    case GRIB_TIMEUNIT_CENTURY:
      return "century";
      break;
    case GRIB_TIMEUNIT_HOURS3:
      return "x3 hours";
      break;
    case GRIB_TIMEUNIT_HOURS6:
      return "x6 hours";
      break;
    case GRIB_TIMEUNIT_HOURS12:
      return "x12 hours";
      break;
    case GRIB_TIMEUNIT_SECOND:
      return "second";
      break;
    default:
      return "unknown unit";
      break;
  }
}

time_t GRIB_TIME::ForecastSeconds( )
{
  time_t rincr;
  int dtime = p1;

  if (timerange == GRIB_TIMERANGE_VALID_AT_REFTIME_PLUS_P1P2)
    dtime = p1p2;
  if (timerange > GRIB_TIMERANGE_ANALYSIS_AT_REFTIME &&
      timerange < GRIB_TIMERANGE_AVERAGE_IN_REFTIME_MINUS_P1_REFTIME_MINUS_P2)
    dtime = p2;

  if (timeunit == GRIB_TIMEUNIT_MINUTE)
    rincr = dtime * 60;
  else if (timeunit == GRIB_TIMEUNIT_HOUR)
    rincr = dtime * 3600;
  else if (timeunit == GRIB_TIMEUNIT_DAY)
    rincr = dtime * 86400;
  else if (timeunit == GRIB_TIMEUNIT_MONTH)
    rincr = dtime * 2592000;
  else if (timeunit == GRIB_TIMEUNIT_HOURS3)
    rincr = dtime * 10800;
  else if (timeunit == GRIB_TIMEUNIT_HOURS6)
    rincr = dtime * 21600;
  else if (timeunit == GRIB_TIMEUNIT_HOURS12)
    rincr = dtime * 43200;
  else if (timeunit == GRIB_TIMEUNIT_SECOND)
    rincr = dtime;
  else
    rincr = 0;
  return rincr;
}

std::string GRIB_TIME::Reftime( ) { return reftime; }
std::string GRIB_TIME::TimeString( ) { return timestring; }

std::string GRIB_TIME::Reftime2000( )
{
  std::stringstream s;
  s << "2000-01-01 00:00:00 UTC";
  return s.str( );
}

time_t GRIB_TIME::ForecastSeconds2000( )
{
  struct tm itm;
  time_t ttm;
  const time_t s_epoch_2000 = 946684800;

  memset(&itm, 0, sizeof(struct tm));
  itm.tm_year = year - 1900;
  itm.tm_mon  = month - 1;
  itm.tm_mday = day;
  itm.tm_hour = hour;
  itm.tm_min  = minute;

  tzset( );
  ttm = mktime(&itm) + ForecastSeconds( ) - timezone - s_epoch_2000;
  return ttm;
}

std::string GRIB_TIME::ValidTime( )
{
  struct tm itm, *xtm;
  time_t ttm;

  memset(&itm, 0, sizeof(struct tm));
  itm.tm_year = year - 1900;
  itm.tm_mon  = month - 1;
  itm.tm_mday = day;
  itm.tm_hour = hour;
  itm.tm_min  = minute;

  tzset( );
  ttm = mktime(&itm) + ForecastSeconds( ) - timezone;
  xtm = gmtime(&ttm);
  
  char temp[32];

  strftime(temp, 32, "%Y-%m-%d %H:%M:00 UTC", xtm);
  std::string retval = temp;
  return retval;
}

std::ostream& operator<< ( std::ostream& os, GRIB_TIME &t )
{
  os << t.timestring;
  return os;
}

// ############################################################################
// GRIB Level implementation
// ############################################################################

GRIB_LEVEL::GRIB_LEVEL( )
{
  type = GRIB_LEVEL_UNKNOWN;
  lv1 = 0.0;
  lv2 = 0.0;
}

void GRIB_LEVEL::set(t_enum_GRIB_LEVELS type)
{
  this->set(type, 0.0, 0.0);
  return;
}

void GRIB_LEVEL::set(t_enum_GRIB_LEVELS type, float lev)
{
  this->set(type, lev, 0.0);
  return;
}

void GRIB_LEVEL::set(t_enum_GRIB_LEVELS type, float lev1, float lev2)
{
  lv1 = 0.0;
  lv2 = 0.0;

  this->type = type;
  switch (type)
  {
    case GRIB_LEVEL_SURFACE:
    case GRIB_LEVEL_CLOUD_BASE:
    case GRIB_LEVEL_CLOUD_TOP:
    case GRIB_LEVEL_ISOTHERM_0_DEG:
    case GRIB_LEVEL_ADIABATIC_CONDENSATION_LIFTED_FROM_SURFACE:
    case GRIB_LEVEL_MAXIMUM_WIND:
    case GRIB_LEVEL_TROPOPAUSE:
    case GRIB_LEVEL_NOMINAL_ATMOSPHERE_TOP:
    case GRIB_LEVEL_ENTIRE_ATMOSPHERE:
    case GRIB_LEVEL_ENTIRE_OCEAN:
    case GRIB_LEVEL_MEAN_SEA_LEVEL:
      break;
    case GRIB_LEVEL_ISOBARIC_mb:
    case GRIB_LEVEL_ALTITUDE_ABOVE_MSL_m:
    case GRIB_LEVEL_HEIGHT_ABOVE_GROUND_m:
    case GRIB_LEVEL_HYBRID:
    case GRIB_LEVEL_DEPTH_BELOW_SURFACE_cm:
    case GRIB_LEVEL_ISENTROPIC_K:
    case GRIB_LEVEL_PRESSURE_DIFFERENCE_FROM_GROUND_mb:
    case GRIB_LEVEL_POTENTIAL_VORTICITY_SURFACE_PV_UNITS:
    case GRIB_LEVEL_ISOBARIC_Pa:
    case GRIB_LEVEL_DEPTH_BELOW_SEA_m:
    case GRIB_LEVEL_HEIGHT_ABOVE_GROUND_HIGH_PRECISION_cm:
    case GRIB_LEVEL_ISOTHERMAL_K:
    case GRIB_LEVEL_SIGMA:
    case GRIB_LEVEL_ETA:
      lv1 = lev1;
      break;
    case GRIB_LEVEL_LAYER_HYBRID:
    case GRIB_LEVEL_LAYER_DEPTH_BELOW_SURFACE_cm:
    case GRIB_LEVEL_LAYER_PRESSURE_DIFFERENCE_FROM_GROUND_mb:
    case GRIB_LEVEL_LAYER_ISOBARIC_mb:
    case GRIB_LEVEL_LAYER_ALTITUDE_ABOVE_MSL_m:
    case GRIB_LEVEL_LAYER_HEIGHT_ABOVE_GROUND_m:
    case GRIB_LEVEL_LAYER_SIGMA:
    case GRIB_LEVEL_LAYER_ETA:
    case GRIB_LEVEL_LAYER_ISENTROPIC_K:
    case GRIB_LEVEL_LAYER_ISOBARIC_HIGH_PRECISION_mb:
    case GRIB_LEVEL_LAYER_SIGMA_HIGH_PRECISION:
    case GRIB_LEVEL_LAYER_ISOBARIC_MIXED_PRECISION_mb:
    case GRIB_LEVEL_SATELLITE_METEOSAT7:
    case GRIB_LEVEL_SATELLITE_METEOSAT8:
      lv1 = lev1;
      lv2 = lev2;
      break;
    default:
      break;
  }
  return;
}

int GRIB_LEVEL::Encode( )
{
  int retval = 0;
  switch (type)
  {
    case GRIB_LEVEL_SURFACE:
    case GRIB_LEVEL_CLOUD_BASE:
    case GRIB_LEVEL_CLOUD_TOP:
    case GRIB_LEVEL_ISOTHERM_0_DEG:
    case GRIB_LEVEL_ADIABATIC_CONDENSATION_LIFTED_FROM_SURFACE:
    case GRIB_LEVEL_MAXIMUM_WIND:
    case GRIB_LEVEL_TROPOPAUSE:
    case GRIB_LEVEL_NOMINAL_ATMOSPHERE_TOP:
    case GRIB_LEVEL_ENTIRE_ATMOSPHERE:
    case GRIB_LEVEL_ENTIRE_OCEAN:
    case GRIB_LEVEL_MEAN_SEA_LEVEL:
      break;
    case GRIB_LEVEL_ISOBARIC_mb:
    case GRIB_LEVEL_ALTITUDE_ABOVE_MSL_m:
    case GRIB_LEVEL_HEIGHT_ABOVE_GROUND_m:
    case GRIB_LEVEL_HYBRID:
    case GRIB_LEVEL_DEPTH_BELOW_SURFACE_cm:
    case GRIB_LEVEL_ISENTROPIC_K:
    case GRIB_LEVEL_PRESSURE_DIFFERENCE_FROM_GROUND_mb:
    case GRIB_LEVEL_POTENTIAL_VORTICITY_SURFACE_PV_UNITS:
    case GRIB_LEVEL_ISOBARIC_Pa:
    case GRIB_LEVEL_DEPTH_BELOW_SEA_m:
    case GRIB_LEVEL_HEIGHT_ABOVE_GROUND_HIGH_PRECISION_cm:
      retval = (int) lv1;
      break;
    case GRIB_LEVEL_ISOTHERMAL_K:
      retval = (int) (lv1*100.0);
      break;
    case GRIB_LEVEL_SIGMA:
    case GRIB_LEVEL_ETA:
      retval = (int) (lv1*10000.0);
      break;
    case GRIB_LEVEL_LAYER_HYBRID:
    case GRIB_LEVEL_LAYER_DEPTH_BELOW_SURFACE_cm:
    case GRIB_LEVEL_LAYER_PRESSURE_DIFFERENCE_FROM_GROUND_mb:
      retval = (((int) lv1) << 8) | ((int) lv2);
      break;
    case GRIB_LEVEL_LAYER_ISOBARIC_mb:
      retval = (((int) (lv1/10.0)) << 8) | ((int) (lv2/10.0));
      break;
    case GRIB_LEVEL_LAYER_ALTITUDE_ABOVE_MSL_m:
    case GRIB_LEVEL_LAYER_HEIGHT_ABOVE_GROUND_m:
      retval = (((int) (lv1/100.0)) << 8) | ((int) (lv2/100.0));
      break;
    case GRIB_LEVEL_LAYER_SIGMA:
    case GRIB_LEVEL_LAYER_ETA:
      retval = (((int) (lv1*100.0)) << 8) | ((int) (lv2*100.0));
      break;
    case GRIB_LEVEL_LAYER_ISENTROPIC_K:
      retval = (((int) (475-lv1)) << 8) | ((int) (475-lv2));
      break;
    case GRIB_LEVEL_LAYER_ISOBARIC_HIGH_PRECISION_mb:
      retval = (((int) (1100-lv1)) << 8) | ((int) (1100-lv2));
      break;
    case GRIB_LEVEL_LAYER_SIGMA_HIGH_PRECISION:
      retval = (((int) (1000*lv1)) << 8) | ((int) (1000*lv2));
      break;
    case GRIB_LEVEL_LAYER_ISOBARIC_MIXED_PRECISION_mb:
      retval = (((int) lv1) << 8) | ((int) (1100-lv2));
      break;
    default:
      retval = (((int) lv1) << 8) | ((int) lv2);
      break;
  }
  return retval;
}

void GRIB_LEVEL::Decode(unsigned char *pds)
{
  this->type = (t_enum_GRIB_LEVELS) PDS_KPDS6(pds);

  int lev = PDS_KPDS7(pds);
  int o11 = lev / 256;
  int o12 = lev % 256;

  lv1 = 0.0;
  lv2 = 0.0;

  switch (type)
  {
    case GRIB_LEVEL_SURFACE:
    case GRIB_LEVEL_CLOUD_BASE:
    case GRIB_LEVEL_CLOUD_TOP:
    case GRIB_LEVEL_ISOTHERM_0_DEG:
    case GRIB_LEVEL_ADIABATIC_CONDENSATION_LIFTED_FROM_SURFACE:
    case GRIB_LEVEL_MAXIMUM_WIND:
    case GRIB_LEVEL_TROPOPAUSE:
    case GRIB_LEVEL_NOMINAL_ATMOSPHERE_TOP:
    case GRIB_LEVEL_ENTIRE_ATMOSPHERE:
    case GRIB_LEVEL_ENTIRE_OCEAN:
    case GRIB_LEVEL_MEAN_SEA_LEVEL:
      break;
    case GRIB_LEVEL_ISOBARIC_mb:
    case GRIB_LEVEL_ALTITUDE_ABOVE_MSL_m:
    case GRIB_LEVEL_HEIGHT_ABOVE_GROUND_m:
    case GRIB_LEVEL_HYBRID:
    case GRIB_LEVEL_DEPTH_BELOW_SURFACE_cm:
    case GRIB_LEVEL_ISENTROPIC_K:
    case GRIB_LEVEL_PRESSURE_DIFFERENCE_FROM_GROUND_mb:
    case GRIB_LEVEL_POTENTIAL_VORTICITY_SURFACE_PV_UNITS:
    case GRIB_LEVEL_ISOBARIC_Pa:
    case GRIB_LEVEL_DEPTH_BELOW_SEA_m:
    case GRIB_LEVEL_HEIGHT_ABOVE_GROUND_HIGH_PRECISION_cm:
      lv1 = (float) lev;
      break;
    case GRIB_LEVEL_ISOTHERMAL_K:
      lv1 = (float) (lev/100.0);
      break;
    case GRIB_LEVEL_SIGMA:
    case GRIB_LEVEL_ETA:
      lv1 = (float) (lev/10000.0);
      break;
    case GRIB_LEVEL_LAYER_HYBRID:
    case GRIB_LEVEL_LAYER_DEPTH_BELOW_SURFACE_cm:
    case GRIB_LEVEL_LAYER_PRESSURE_DIFFERENCE_FROM_GROUND_mb:
      lv1 = (float) o11;
      lv2 = (float) o12;
      break;
    case GRIB_LEVEL_LAYER_ISOBARIC_mb:
      lv1 = (float) o11*10.0;
      lv2 = (float) o12*10.0;
      break;
    case GRIB_LEVEL_LAYER_ALTITUDE_ABOVE_MSL_m:
    case GRIB_LEVEL_LAYER_HEIGHT_ABOVE_GROUND_m:
      lv1 = (float) o11*100.0;
      lv2 = (float) o12*100.0;
      break;
    case GRIB_LEVEL_LAYER_SIGMA:
    case GRIB_LEVEL_LAYER_ETA:
      lv1 = (float) o11/100.0;
      lv2 = (float) o12/100.0;
      break;
    case GRIB_LEVEL_LAYER_ISENTROPIC_K:
      lv1 = (float) (475-o11);
      lv2 = (float) (475-o12);
      break;
    case GRIB_LEVEL_LAYER_ISOBARIC_HIGH_PRECISION_mb:
      lv1 = (float) (1100-o11);
      lv2 = (float) (1100-o12);
      break;
    case GRIB_LEVEL_LAYER_SIGMA_HIGH_PRECISION:
      lv1 = (float) o11/1000.0;
      lv2 = (float) o12/1000.0;
      break;
    case GRIB_LEVEL_LAYER_ISOBARIC_MIXED_PRECISION_mb:
      lv1 = (float) o11;
      lv2 = (float) (1100-o12);
      break;
    default:
      break;
  }
  return;
}

void GRIB_LEVEL::set_pressure(float plev)
{
  type = GRIB_LEVEL_ISOBARIC_mb;
  lv1 = plev;
  lv2 = 0.0;
  return;
}

void GRIB_LEVEL::set_height(float hgt)
{
  type = GRIB_LEVEL_HEIGHT_ABOVE_GROUND_m;
  lv1 = hgt;
  lv2 = 0.0;
  return;
}

void GRIB_LEVEL::set_seadepth(float dpt)
{
  type = GRIB_LEVEL_DEPTH_BELOW_SEA_m;
  lv1 = dpt;
  lv2 = 0.0;
  return;
}

void GRIB_LEVEL::set_isentropic(float temp)
{
  type = GRIB_LEVEL_ISENTROPIC_K;
  lv1 = temp;
  lv2 = 0.0;
  return;
}

void GRIB_LEVEL::set_sigma(float sigma)
{
  type = GRIB_LEVEL_SIGMA;
  lv1 = sigma;
  lv2 = 0.0;
  return;
}

void GRIB_LEVEL::set_surface( )
{
  type = GRIB_LEVEL_SURFACE;
  lv1 = 0.0;
  lv2 = 0.0;
  return;
}

std::ostream& operator<< ( std::ostream& os, GRIB_LEVEL &l )
{
  os << l.leveldesc( );
  return os;
}

std::string GRIB_LEVEL::leveldesc( )
{
  std::stringstream s;
  switch (type)
  {
    case GRIB_LEVEL_SURFACE:
      s << "Ground or water surface";
      break;
    case GRIB_LEVEL_CLOUD_BASE:
      s << "Cloud base";
      break;
    case GRIB_LEVEL_CLOUD_TOP:
      s << "Cloud top";
      break;
    case GRIB_LEVEL_ISOTHERM_0_DEG:
      s << "Isotherm level at 0 Celsius";
      break;
    case GRIB_LEVEL_ADIABATIC_CONDENSATION_LIFTED_FROM_SURFACE:
      s << "Level of adiabatic condensation lifted from surface";
      break;
    case GRIB_LEVEL_MAXIMUM_WIND:
      s << "Level of maximum wind";
      break;
    case GRIB_LEVEL_TROPOPAUSE:
      s << "Tropopause level";
      break;
    case GRIB_LEVEL_NOMINAL_ATMOSPHERE_TOP:
      s << "Nominal atmosphere top";
      break;
    case GRIB_LEVEL_MEAN_SEA_LEVEL:
      s << "Mean Sea Level";
      break;
    case GRIB_LEVEL_ENTIRE_ATMOSPHERE:
      s << "Entire atmosphere as single layer";
      break;
    case GRIB_LEVEL_ENTIRE_OCEAN:
      s << "Entire ocean as single layer";
      break;
    case GRIB_LEVEL_ISOBARIC_mb:
      s << lv1 << " mb";
      break;
    case GRIB_LEVEL_ALTITUDE_ABOVE_MSL_m:
      s << lv1 << " m above MSL";
      break;
    case GRIB_LEVEL_HEIGHT_ABOVE_GROUND_m:
      s << lv1 << " m above ground";
      break;
    case GRIB_LEVEL_HYBRID:
      s << lv1 << " hybrid level";
      break;
    case GRIB_LEVEL_DEPTH_BELOW_SURFACE_cm:
      s << lv1 << " cm below surface";
      break;
    case GRIB_LEVEL_ISENTROPIC_K:
      s << lv1 << " K isentropic";
      break;
    case GRIB_LEVEL_PRESSURE_DIFFERENCE_FROM_GROUND_mb:
      s << lv1 << " mb difference from ground";
      break;
    case GRIB_LEVEL_POTENTIAL_VORTICITY_SURFACE_PV_UNITS:
      s << lv1 << " PV units";
      break;
    case GRIB_LEVEL_ISOBARIC_Pa:
      s << lv1 << " Pa";
      break;
    case GRIB_LEVEL_DEPTH_BELOW_SEA_m:
      s << lv1 << " m below sea";
      break;
    case GRIB_LEVEL_HEIGHT_ABOVE_GROUND_HIGH_PRECISION_cm:
      s << lv1 << " cm above ground (high precision)";
      break;
    case GRIB_LEVEL_ISOTHERMAL_K:
      s << lv1 << " K isotherm";
      break;
    case GRIB_LEVEL_SIGMA:
      s << lv1 << " sigma";
      break;
    case GRIB_LEVEL_ETA:
      s << lv1 << " eta";
      break;
    case GRIB_LEVEL_LAYER_ISOBARIC_mb:
      s << lv1 << "-" << lv2 << " mb layer";
      break;
    case GRIB_LEVEL_LAYER_ALTITUDE_ABOVE_MSL_m:
      s << lv1 << "-" << lv2 << " m above sea level layer";
      break;
    case GRIB_LEVEL_LAYER_HEIGHT_ABOVE_GROUND_m:
      s << lv1 << "-" << lv2 << " m height above ground layer";
      break;
    case GRIB_LEVEL_LAYER_SIGMA:
      s << lv1 << "-" << lv2 << " sigma layer";
      break;
    case GRIB_LEVEL_LAYER_ETA:
      s << lv1 << "-" << lv2 << " eta layer";
      break;
    case GRIB_LEVEL_LAYER_ISENTROPIC_K:
      s << lv1 << "-" << lv2 << " K isentropic layer";
      break;
    case GRIB_LEVEL_LAYER_HYBRID:
      s << lv1 << "-" << lv2 << " hybrid layer";
      break;
    case GRIB_LEVEL_LAYER_DEPTH_BELOW_SURFACE_cm:
      s << lv1 << "-" << lv2 << " cm below suface layer";
      break;
    case GRIB_LEVEL_LAYER_PRESSURE_DIFFERENCE_FROM_GROUND_mb:
      s << lv1 << "-" << lv2 << " mb difference from ground layer";
      break;
    case GRIB_LEVEL_LAYER_ISOBARIC_HIGH_PRECISION_mb:
      s << lv1 << "-" << lv2 << " mb high precision layer";
      break;
    case GRIB_LEVEL_LAYER_SIGMA_HIGH_PRECISION:
      s << lv1 << "-" << lv2 << " sigma high precision layer";
      break;
    case GRIB_LEVEL_LAYER_ISOBARIC_MIXED_PRECISION_mb:
      s << lv1 << "-" << lv2 << " mb mixed precision layer";
      break;
    default:
      s << "Unknown level code : " << type << " : " << lv1 << " : " << lv2;
  }
  return s.str( );
}

bool GRIB_LEVEL::is_LevelPressure( )
{
  return (type == GRIB_LEVEL_ISOBARIC_mb);
}

bool GRIB_LEVEL::is_LevelHeight( )
{
  return (type == GRIB_LEVEL_HEIGHT_ABOVE_GROUND_m);
}

bool GRIB_LEVEL::is_Surface( )
{
  return (type == GRIB_LEVEL_SURFACE);
}

bool GRIB_LEVEL::is_MeanSeaLevel( )
{
  return (type == GRIB_LEVEL_MEAN_SEA_LEVEL);
}

// ############################################################################
// GRIB Grid implementation
// ############################################################################

void GRIB_GRID_regular_latlon::set(float lat1, float lat2,
                                   float lon1, float lon2,
                                   float dlat, float dlon)
{
  this->lat1 = lat1;
  this->lon1 = lon1;
  this->lat2 = lat2;
  this->lon2 = lon2;
  this->dlat = dlat;
  this->dlon = dlon;
  return;
}

std::ostream& operator<< ( std::ostream& os, GRIB_GRID_regular_latlon &g )
{
  os << "               Regular latitude/longitude projection" << std::endl
     << "               "
     << "Latitude  " << g.lat1 << " to " << g.lat2
     << " by " << g.dlat << std::endl
     << "               "
     << "Longitude " << g.lon1 << " to " << g.lon2
     << " by " << g.dlon;
  return os;
}

void GRIB_GRID_gaussian_latlon::set(float lat1, float lat2,
                                    float lon1, float lon2,
                                    float dlon, float N)
{
  this->lat1 = lat1;
  this->lon1 = lon1;
  this->lat2 = lat2;
  this->lon2 = lon2;
  this->dlon = dlon;
  this->N    = N;
  return;
}

std::ostream& operator<< ( std::ostream& os, GRIB_GRID_gaussian_latlon &g )
{
  os << "               Gaussian latitude/longitude projection" << std::endl
     << "               Latitude  " << g.lat1 << " to " << g.lat2
     << " " << g.N << " latitude circles" << std::endl
     << "               Longitude " << g.lon1 << " to " << g.lon2
     << " by " << g.dlon;
  return os;
}

void GRIB_GRID_polar_stereographic::set(float lat1, float lon1, float lov,
                                        float dx, float dy, bool pole)
{
  this->lat1 = lat1;
  this->lon1 = lon1;
  this->lov  = lov;
  this->dx   = dx;
  this->dy   = dy;
  this->pole = pole;
  return;
}

std::ostream& operator<< ( std::ostream& os, GRIB_GRID_polar_stereographic &g )
{
  os << "               Polar stereographic projection" << std::endl
     << "               "
     << "First point Latitude " << g.lat1 << ", Longitude " << g.lon1
     << std::endl << "Orientation longitude " << g.lov << std::endl
     << "               "
     << "Dx " << g.dx << " Km, Dy " << g.dy << "Km, "
     << (g.pole ? "North Polar": "South Polar") << std::endl;
  return os;
}

void GRIB_GRID_lambert_conformal::set(float lat1, float lon1, float lov,
                                      float latin1, float latin2,
                                      float latsp, float lonsp,
                                      float dx, float dy,
                                      bool pole, bool bipolar)
{
  this->lat1    = lat1;
  this->lon1    = lon1;
  this->lov     = lov;
  this->latin1  = latin1;
  this->latin2  = latin2;
  this->latsp   = latsp;
  this->lonsp   = lonsp;
  this->dx      = dx;
  this->dy      = dy;
  this->pole    = pole;
  this->bipolar = bipolar;
  return;
}

std::ostream& operator<< ( std::ostream& os, GRIB_GRID_lambert_conformal &g )
{
  os << "               Lambert conformal projection" << std::endl
     << "               "
     << "First point Latitude " << g.lat1 << ", Longitude " << g.lon1
     << std::endl << "Orientation longitude " << g.lov << std::endl
     << "               "
     << "Secant latitude1 " << g.latin1 << ", latitude2 " << g.latin2
     << std::endl << "South Pole Latitude " << g.latsp
     << ", Longitude " << g.lonsp << std::endl
     << "               "
     << "Dx " << g.dx << " Km, Dy " << g.dy << "Km, "
     << (g.pole ? "North Polar": "South Polar") << ", "
     << (g.bipolar ? " bipolar symmetric" : "one center") << std::endl;
  return os;
}

void GRIB_GRID_mercator::set(float lat1, float lon1,
                             float lat2, float lon2,
                             float dx, float dy, float latin)
{
  this->lat1  = lat1;
  this->lon1  = lon1;
  this->lat2  = lat2;
  this->lon2  = lon2;
  this->dx    = dx;
  this->dy    = dy;
  this->latin = latin;
  return;
}

std::ostream& operator<< ( std::ostream& os, GRIB_GRID_mercator &g )
{
  os << "               Mercator projection" << std::endl
     << "               "
     << "Latitude " << g.lat1 << " to " << g.lat2
     << " by " << g.dy << " Km " << std::endl
     << "Longitude " << g.lon1 << " to " << g.lon2
     << "               "
     << " by " << g.dx << " Km " << std::endl
     << "               "
     << "Latin " << g.latin << std::endl;
  return os;
}

void GRIB_GRID_spaceview::set(float lap, float lop, float dx, float dy,
                              float Xp, float Yp, float orient, float Nr,
                              float X0, float Y0)
{
  this->lap    = lap;
  this->lop    = lop;
  this->dx     = dx;
  this->dy     = dy;
  this->Xp     = Xp;
  this->Yp     = Yp;
  this->orient = orient;
  this->Nr     = Nr;
  this->X0     = X0;
  this->Y0     = Y0;
  return;
}

std::ostream& operator<< ( std::ostream& os, GRIB_GRID_spaceview &g )
{
  os << "               Space view projection" << std::endl
     << "               Sub satellite point lat/lon: "
     << g.lap << ", " << g.lop << std::endl
     << "               Apparent diameter of earth X,Y: "
     << g.dx << ", " << g.dy << std::endl
     << "               Sub satellite point X,Y: "
     << g.Xp << ", " << g.Yp << std::endl
     << "               Orient " << g.orient << ", Camera H " << g.Nr
     << std::endl
     << "               Origin of sector image X,Y: " << g.X0 << ", " << g.Y0
     << std::endl;
  return os;
}

void GRIB_GRID_spherical_harmonic::set(int J, int K, int M, int rt, int csm)
{
  this->J = J;
  this->K = K;
  this->M = M;
  representation = (GRIB_GRID_SPECTRAL_REPRESENTATION) rt;
  storage_method = (GRIB_GRID_COEFFICIENT_STORAGE_METHOD) csm;
  return;
}

std::ostream& operator<< ( std::ostream& os, GRIB_GRID_spherical_harmonic &g )
{
  os << "               Spherical harmonic coefficients" << std::endl
     << "               J, K, M : "
     << g.J << ", " << g.K << ", " << g.M << std::endl
     << "               Spectral representation: "
     << ((g.representation == g.SR_associated_legendre_polynomials_first_kind) ?
        "Associated Legendre polynomials of first kind" :
        "Sperical armonics - complex packing")
     << "               Storage Method : "
     << ((g.storage_method == g.CSM_normal) ?  "Normal" : "Complex packing")
     << std::endl;
  return os;
}

GRIB_GRID::GRIB_GRID( )
{
  grid_code = 0;
  type = GRIB_GRID_UNKNOWN;
  is_dirincgiven = true;
  is_earth_spheroid = false;
  is_uv_grid = false;
  is_x_negative = false;
  is_y_negative = false;
  is_fortran = false;
  nxny = nx = ny = 0;
}

void GRIB_GRID::set_size(int nx, int ny)
{
  this->nx = nx;
  this->ny = ny;
  nxny = nx*ny;
  return;
}

void GRIB_GRID::set_number(int code)
{
  grid_code = code;
  return;
}

void GRIB_GRID::set_regular_latlon(float lat1, float lat2,
                                   float lon1, float lon2,
                                   float dlat, float dlon)
{
  grid_code = 255;
  type = GRIB_GRID_REGULAR_LATLON;
  ll.set(lat1, lat2, lon1, lon2, dlat, dlon);
  return;
}

void GRIB_GRID::set_gaussian_latlon(float lat1, float lat2,
                                    float lon1, float lon2,
                                    float dlon, float N)
{
  grid_code = 255;
  type = GRIB_GRID_GAUSSIAN;
  gl.set(lat1, lat2, lon1, lon2, dlon, N);
  return;
}

void GRIB_GRID::set_polar_stereo(float lat1, float lon1, float lov,
                                 float dx, float dy, bool pole)
{
  grid_code = 255;
  type = GRIB_GRID_POLAR_STEREOGRAPHIC;
  ps.set(lat1, lon1, lov, dx, dy, pole);
  return;
}

void GRIB_GRID::set_lambert_conformal(float lat1, float lon1, float lov,
                                      float latin1, float latin2,
                                      float latsp, float lonsp,
                                      float dx, float dy,
                                      bool pole, bool bipolar)
{
  grid_code = 255;
  type = GRIB_GRID_LAMBERT_CONFORMAL;
  lc.set(lat1, lon1, lov, latin1, latin2, latsp, lonsp, dx, dy, pole, bipolar);
  return;
}

void GRIB_GRID::set_mercator(float lat1, float lon1, float lat2, float lon2,
                             float dx, float dy, float latin)
{
  grid_code = 255;
  type = GRIB_GRID_MERCATOR;
  mc.set(lat1, lon1, lat2, lon2, dx, dy, latin);
  return;
}

void GRIB_GRID::set_spaceview(float lap, float lop, float dx, float dy,
                              float Xp, float Yp, float orient, float Nr,
                              float X0, float Y0)
{
  grid_code = 255;
  type = GRIB_GRID_SPACEVIEW;
  sp.set(lap, lop, dx, dy, Xp, Yp, orient, Nr, X0, Y0);
  return;
}

void GRIB_GRID::set_spherical_harmonic(int J, int K, int M, int rt, int csm)
{
  grid_code = 255;
  type = GRIB_GRID_SPHERICAL_HARMONIC_COE;
  sa.set(J, K, M, rt, csm);
  return;
}

void GRIB_GRID::set_earth_spheroid( ) { is_earth_spheroid = true; }
void GRIB_GRID::set_uv_grid( ) { is_uv_grid = true; }
void GRIB_GRID::set_x_negative( ) { is_x_negative = true; }
void GRIB_GRID::set_y_negative( ) { is_y_negative = true; }
void GRIB_GRID::set_fortran_indexing( ) { is_fortran = true; }

std::ostream& operator<< ( std::ostream& os, GRIB_GRID &g )
{
  os << "Size: " << g.nx << " x " << g.ny << " ("
     << g.nxny << ")" << std::endl;
  if (g.grid_code != 0 && g.grid_code != 255)
  {
    os << "          GRID IDENTIFICATION NUMBER " << g.grid_code;
    return os;
  }
  if (! g.is_dirincgiven)
    os << "Direction increment are NOT given!" << std::endl;
  if (g.is_earth_spheroid)
    os << "Earth is spheroid!" << std::endl;
  if (g.is_uv_grid)
    os << "U-V vector components are GRID relative!" << std::endl;
  if (g.is_x_negative)
    os << "X scan mode is reversed (EW)!" << std::endl;
  if (g.is_y_negative)
    os << "Y scan mode is reversed (NS)!" << std::endl;
  if (g.is_fortran)
    os << "Fortran indexing (columnar)!" << std::endl;
  switch (g.type)
  {
    case GRIB_GRID_REGULAR_LATLON:
      os << g.ll;
      break;
    case GRIB_GRID_GAUSSIAN:
      os << g.gl;
      break;
    case GRIB_GRID_POLAR_STEREOGRAPHIC:
      os << g.ps;
      break;
    case GRIB_GRID_LAMBERT_CONFORMAL:
      os << g.lc;
      break;
    case GRIB_GRID_MERCATOR:
      os << g.mc;
      break;
    case GRIB_GRID_SPACEVIEW:
      os << g.sp;
      break;
    case GRIB_GRID_SPHERICAL_HARMONIC_COE:
      os << g.sa;
      break;
    default:
      os << "Unsupported Grid is being used." << std::endl
         << "Code is: " << g.type << std::endl
         << "Grid Descripion is: " << std::endl
         << std::setw(3) << std::setfill('0')
         << (int) g.gds[6] << " "  << (int) g.gds[7] << " "
         << (int) g.gds[8] << " "  << (int) g.gds[9] << " "
         << (int) g.gds[10] << " " << (int) g.gds[11] << " "
         << (int) g.gds[12] << " " << (int) g.gds[13] << " "
         << (int) g.gds[14] << " " << (int) g.gds[15] << " "
         << (int) g.gds[16] << " " << (int) g.gds[17] << " "
         << (int) g.gds[18] << " " << (int) g.gds[19] << " "
         << (int) g.gds[20] << std::endl
         << (int) g.gds[21] << " " << (int) g.gds[22] << " "
         << (int) g.gds[23] << " " << (int) g.gds[24] << " "
         << (int) g.gds[25] << " " << (int) g.gds[26] << " "
         << (int) g.gds[27] << " " << (int) g.gds[28] << " "
         << (int) g.gds[29] << " " << (int) g.gds[30] << " "
         << (int) g.gds[31] << " " << (int) g.gds[32] << " "
         << (int) g.gds[33] << " " << (int) g.gds[34] << " "
         << (int) g.gds[35] << std::endl
         << (int) g.gds[36] << " " << (int) g.gds[37] << " "
         << (int) g.gds[38] << " " << (int) g.gds[39] << " "
         << (int) g.gds[40] << " " << (int) g.gds[41] << " "
         << (int) g.gds[42] << " " << (int) g.gds[43] << std::endl;
  }
  return os;
}

unsigned char *GRIB_GRID::Encode( )
{
  unsigned char *xgds = 0;

  if (grid_code != 255) return 0;

  int mode = 0, scan = 0, pole = 0;
  if (is_dirincgiven)    mode += 128; // bit 1
  if (is_earth_spheroid) mode += 64;  // bit 2
  if (is_uv_grid)        mode += 8;   // bit 5
  if (is_x_negative)     scan += 128; // bit 1
  if (is_y_negative)     scan += 64;  // bit 2
  if (is_fortran)        scan += 32;  // bit 3

  switch (type)
  {
    case GRIB_GRID_REGULAR_LATLON:
      xgds = mk_GDS(NULL, WGRIB_ENCODE_INIT, 32, 0,
                   WGRIB_ENCODE_BYTE,    4,  255,
                   WGRIB_ENCODE_BYTE,    5,  GRIB_GRID_REGULAR_LATLON,
                   WGRIB_ENCODE_2BYTES,  6,  nx,
                   WGRIB_ENCODE_2BYTES,  8,  ny,
                   WGRIB_ENCODE_S3BYTES, 10, (int) (1000.0*(ll.lat1)),
                   WGRIB_ENCODE_S3BYTES, 13, (int) (1000.0*(ll.lon1)),
                   WGRIB_ENCODE_BYTE,    16, mode,
                   WGRIB_ENCODE_S3BYTES, 17, (int) (1000.0*(ll.lat2)),
                   WGRIB_ENCODE_S3BYTES, 20, (int) (1000.0*(ll.lon2)),
                   WGRIB_ENCODE_S2BYTES, 23, (int) (1000.0*(ll.dlon)),
                   WGRIB_ENCODE_S2BYTES, 25, (int) (1000.0*(ll.dlat)),
                   WGRIB_ENCODE_BYTE,    27, scan,   WGRIB_ENCODE_END);
      break;
    case GRIB_GRID_POLAR_STEREOGRAPHIC:
      pole = ps.pole ? 0 : 128;
      xgds = mk_GDS(NULL, WGRIB_ENCODE_INIT, 32, 0,
                   WGRIB_ENCODE_BYTE,    4,  255,
                   WGRIB_ENCODE_BYTE,    5,  GRIB_GRID_POLAR_STEREOGRAPHIC,
                   WGRIB_ENCODE_2BYTES,  6,  nx,
                   WGRIB_ENCODE_2BYTES,  8,  ny,
                   WGRIB_ENCODE_S3BYTES, 10, (int) (1000.0*(ps.lat1)),
                   WGRIB_ENCODE_S3BYTES, 13, (int) (1000.0*(ps.lon1)),
                   WGRIB_ENCODE_BYTE,    16, mode,
                   WGRIB_ENCODE_S3BYTES, 17, (int) (1000.0*(ps.lov)),
                   WGRIB_ENCODE_S3BYTES, 20, (int) (1000.0*(ps.dx)),
                   WGRIB_ENCODE_S3BYTES, 23, (int) (1000.0*(ps.dy)),
                   WGRIB_ENCODE_BYTE,    26, pole,
                   WGRIB_ENCODE_BYTE,    27, scan,   WGRIB_ENCODE_END);
      break;
    case GRIB_GRID_LAMBERT_CONFORMAL:
      pole = 0;
      if      (  lc.pole &&   lc.bipolar) pole = 64;
      else if (! lc.pole && ! lc.bipolar) pole = 128;
      else if (! lc.pole &&   lc.bipolar) pole = 192;
      xgds = mk_GDS(NULL, WGRIB_ENCODE_INIT, 42, 0,
                   WGRIB_ENCODE_BYTE,    4,  255,
                   WGRIB_ENCODE_BYTE,    5,  GRIB_GRID_LAMBERT_CONFORMAL,
                   WGRIB_ENCODE_2BYTES,  6,  nx,
                   WGRIB_ENCODE_2BYTES,  8,  ny,
                   WGRIB_ENCODE_S3BYTES, 10, (int) (1000.0*(lc.lat1)),
                   WGRIB_ENCODE_S3BYTES, 13, (int) (1000.0*(lc.lon1)),
                   WGRIB_ENCODE_BYTE,    16, mode,
                   WGRIB_ENCODE_S3BYTES, 17, (int) (1000.0*(lc.lov)),
                   WGRIB_ENCODE_S3BYTES, 20, (int) (1000.0*(lc.dx)),
                   WGRIB_ENCODE_S3BYTES, 23, (int) (1000.0*(lc.dy)),
                   WGRIB_ENCODE_BYTE,    26, pole,
                   WGRIB_ENCODE_BYTE,    27, scan,
                   WGRIB_ENCODE_S3BYTES, 28, (int) (1000.0*(lc.latin1)),
                   WGRIB_ENCODE_S3BYTES, 31, (int) (1000.0*(lc.latin2)),
                   WGRIB_ENCODE_S3BYTES, 34, (int) (1000.0*(lc.latsp)),
                   WGRIB_ENCODE_S3BYTES, 37, (int) (1000.0*(lc.lonsp)),
                   WGRIB_ENCODE_END);
      break;
    case GRIB_GRID_MERCATOR:
      xgds = mk_GDS(NULL, WGRIB_ENCODE_INIT, 42, 0,
                   WGRIB_ENCODE_BYTE,    4,  255,
                   WGRIB_ENCODE_BYTE,    5,  GRIB_GRID_MERCATOR,
                   WGRIB_ENCODE_2BYTES,  6,  nx,
                   WGRIB_ENCODE_2BYTES,  8,  ny,
                   WGRIB_ENCODE_S3BYTES, 10, (int) (1000.0*(mc.lat1)),
                   WGRIB_ENCODE_S3BYTES, 13, (int) (1000.0*(mc.lon1)),
                   WGRIB_ENCODE_BYTE,    16, mode,
                   WGRIB_ENCODE_S3BYTES, 17, (int) (1000.0*(mc.lat2)),
                   WGRIB_ENCODE_S3BYTES, 20, (int) (1000.0*(mc.lon2)),
                   WGRIB_ENCODE_S3BYTES, 23, (int) (1000.0*(mc.latin)),
                   WGRIB_ENCODE_BYTE,    27, scan,
                   WGRIB_ENCODE_S3BYTES, 28, (int) (1000.0*(mc.dx)),
                   WGRIB_ENCODE_S3BYTES, 31, (int) (1000.0*(mc.dy)),
                   WGRIB_ENCODE_END);
      break;
    case GRIB_GRID_SPACEVIEW:
      xgds = mk_GDS(NULL, WGRIB_ENCODE_INIT, 40, 0,
                   WGRIB_ENCODE_BYTE,    4,  255,
                   WGRIB_ENCODE_BYTE,    5,  GRIB_GRID_SPACEVIEW,
                   WGRIB_ENCODE_2BYTES,  6,  nx,
                   WGRIB_ENCODE_2BYTES,  8,  ny,
                   WGRIB_ENCODE_S3BYTES, 10, (int) (1000.0*(sp.lap)),
                   WGRIB_ENCODE_S3BYTES, 13, (int) (1000.0*(sp.lop)),
                   WGRIB_ENCODE_BYTE,    16, mode,
                   WGRIB_ENCODE_S3BYTES, 17, (int) (1000.0*(sp.dx)),
                   WGRIB_ENCODE_S3BYTES, 20, (int) (1000.0*(sp.dy)),
                   WGRIB_ENCODE_S2BYTES, 23, (int) sp.Xp,
                   WGRIB_ENCODE_S2BYTES, 25, (int) sp.Yp,
                   WGRIB_ENCODE_BYTE,    27, scan,
                   WGRIB_ENCODE_S3BYTES, 28, (int) (1000.0*(sp.orient)),
                   WGRIB_ENCODE_S3BYTES, 31, (int) (1000.0*(sp.Nr)),
                   WGRIB_ENCODE_2BYTES,  34, (int) sp.X0,
                   WGRIB_ENCODE_2BYTES,  36, (int) sp.Y0,
                   WGRIB_ENCODE_END);
      break;
    case GRIB_GRID_SPHERICAL_HARMONIC_COE:
      xgds = mk_GDS(NULL, WGRIB_ENCODE_INIT, 32, 0,
                   WGRIB_ENCODE_BYTE,    4,  255,
                   WGRIB_ENCODE_BYTE,    5,  GRIB_GRID_SPHERICAL_HARMONIC_COE,
                   WGRIB_ENCODE_2BYTES,  6,  sa.J,
                   WGRIB_ENCODE_2BYTES,  8,  sa.K,
                   WGRIB_ENCODE_2BYTES, 10,  sa.M,
                   WGRIB_ENCODE_BYTE,   12,  sa.representation,
                   WGRIB_ENCODE_BYTE,   13,  sa.storage_method,
                   WGRIB_ENCODE_END);
      break;
    default:
      xgds = mk_GDS(NULL, WGRIB_ENCODE_INIT, 32, 0,
                    WGRIB_ENCODE_BYTE,    4,  255,
                    WGRIB_ENCODE_END);
      memcpy(xgds+4, gds+4, 28);
      break;
  }
  return xgds;
}

void GRIB_GRID::Decode(unsigned char *pds,
                       unsigned char *xgds,
                       unsigned char *bms,
                       unsigned char *bds)
{
  // Get grid dimension
  ny = 1;

  if (xgds != NULL)
  {
    if (GDS_LEN(xgds) > 44)
    {
      std::cerr << "Sorry, thinned grids NOT supported." << std::endl;
      throw;
    }

    memcpy(gds, xgds, GDS_LEN(xgds));

    GDS_grid(gds, bds, &nx, &ny, &nxny);
    type = (t_enum_GRIB_GRIDS) GDS_DataType(gds);
    switch (type)
    {
      case GRIB_GRID_REGULAR_LATLON:
        ll.set(0.001*GDS_LatLon_La1(gds), 0.001*GDS_LatLon_La2(gds),
               0.001*GDS_LatLon_Lo1(gds), 0.001*GDS_LatLon_Lo2(gds),
               0.001*GDS_LatLon_dy(gds),  0.001*GDS_LatLon_dx(gds));
        break;
      case GRIB_GRID_GAUSSIAN:
        gl.set(0.001*GDS_LatLon_La1(gds), 0.001*GDS_LatLon_La2(gds),
               0.001*GDS_LatLon_Lo1(gds), 0.001*GDS_LatLon_Lo2(gds),
               0.001*GDS_LatLon_dx(gds),  0.001*GDS_Gaussian_nlat(gds));
        break;
      case GRIB_GRID_POLAR_STEREOGRAPHIC:
        ps.set(0.001*GDS_Polar_La1(gds), 0.001*GDS_Polar_Lo1(gds),
               0.001*GDS_Polar_Lov(gds),
               0.001*GDS_Polar_Dx(gds), 0.001*GDS_Polar_Dy(gds),
               (GDS_Polar_pole(gds) ? true : false));
        break;
      case GRIB_GRID_LAMBERT_CONFORMAL:
        lc.set(0.001*GDS_Lambert_La1(gds), 0.001*GDS_Lambert_Lo1(gds),
               0.001*GDS_Lambert_Lov(gds), 0.001*GDS_Lambert_Latin1(gds),
               0.001*GDS_Lambert_Latin2(gds), 0.001*GDS_Lambert_LatSP(gds),
               0.001*GDS_Lambert_LonSP(gds), 0.001*GDS_Lambert_dx(gds),
               0.001*GDS_Lambert_dy(gds), (GDS_Lambert_NP(gds) ? true : false),
               (((gds[26] & 64) == 0) ? true : false));
        break;
      case GRIB_GRID_MERCATOR:
        mc.set(0.001*GDS_Merc_La1(gds), 0.001*GDS_Merc_Lo1(gds),
               0.001*GDS_Merc_La2(gds), 0.001*GDS_Merc_Lo2(gds),
               0.001*GDS_Merc_dx(gds), 0.001*GDS_Merc_dy(gds),
               0.001*GDS_Merc_Latin(gds));
        break;
      case GRIB_GRID_SPHERICAL_HARMONIC_COE:
        sa.set(GDS_Harmonic_nj(gds), GDS_Harmonic_nk(gds),
               GDS_Harmonic_nm(gds), GDS_Harmonic_type(gds),
               GDS_Harmonic_mode(gds));
        break;
      case GRIB_GRID_SPACEVIEW:
        sp.set(0.001*GDS_Spaceview_lap(gds), 0.001*GDS_Spaceview_lop(gds),
               0.001*GDS_Spaceview_dx(gds), 0.001*GDS_Spaceview_dy(gds),
               GDS_Spaceview_Xp(gds), GDS_Spaceview_Yp(gds),
               0.001*GDS_Spaceview_or(gds), 0.001*GDS_Spaceview_nr(gds),
               GDS_Spaceview_X0(gds), GDS_Spaceview_Y0(gds));
        break;
      default:
        break;
    }

    is_dirincgiven = (gds[16] & 128);
    is_earth_spheroid = (gds[16] & 64);
    is_uv_grid = (gds[16] & 8);
    is_x_negative = (gds[27] & 128);
    is_y_negative = (gds[27] & 64);
    is_fortran = (gds[27] & 32);
  }
  else if (bms != NULL)
    nxny = nx = ((bms[4]<<8) + bms[5]);
  else
  {
    if (BDS_NumBits(bds) == 0)
      nxny = nx = 1;
    else
      nxny = nx = BDS_NValues(bds);
  }

  grid_code = PDS_Grid(pds);

  return;
}

// ############################################################################
// GRIB Field implementation
// ############################################################################

GRIB_FIELD::GRIB_FIELD( )
{
  varcode = GRIB_PARAMETER_UNKNOWN;
  center = GRIB_CENTER_LOCAL;
  subcenter = GRIB_SUBCENTER_LOCAL;
  table = GRIB_TABLE_INTERNATIONAL;
  process = GRIB_PROCESS_LOCAL;
  vals = 0;
  size = 0;
  varname = "Unknown";
  varunit = "Unknown";
  vardesc = "Unknown";
  numbits = 0;
  refvalue = 0.0;
  decimalscale = 0;
  binscale = 0;
  undef_high = 9.9991e20;
  undef_low = 9.9991e20;
  morepds = 0;
  nmorepds = 0;
}

GRIB_FIELD::~GRIB_FIELD( )
{
  if (vals) delete [ ] vals;
  if (morepds) delete [ ] morepds;
}

void GRIB_FIELD::Decode(unsigned char *pds, unsigned char *bds,
                        unsigned char *bms, size_t nxny)
{
  if (size != nxny)
  {
    if (vals) delete [ ] vals;
    vals = new float[nxny];
  }

  if (vals == 0)
    vals = new float[nxny];

  size = nxny;

  center    = PDS_Center(pds);
  subcenter = PDS_Subcenter(pds);
  table     = PDS_Vsn(pds);
  process   = PDS_Model(pds);

  varcode   = PDS_PARAM(pds);

  varname = (Parm_Table(center, subcenter, table, process) + varcode)->name;
  vardesc = (Parm_Table(center, subcenter, table, process) + varcode)->comment;
  size_t indx0 = vardesc.find_first_of('[', 0);
  size_t indx1 = vardesc.find_first_of(']', 0);
  varunit = vardesc.substr(indx0+1,indx1-indx0-1);

  numbits = BDS_NumBits(bds);
  decimalscale = PDS_DecimalScale(pds);
  refvalue = BDS_RefValue(bds);
  binscale = BDS_BinScale(bds);

  double temp = int_power(10.0, - decimalscale);

  BDS_unpack(vals, bds, BMS_bitmap(bms), numbits, size,
             temp * refvalue, temp * int_power(2.0, binscale));
  undef_high = UNDEFINED;
  undef_low = UNDEFINED;

  return;
}

std::string GRIB_FIELD::VarName( ) { return varname; }
std::string GRIB_FIELD::VarUnit( ) { return varunit; }
std::string GRIB_FIELD::VarDescription( ) { return vardesc; }

void GRIB_FIELD::set_table(int c, int s, int t, int p)
{
  center = c;
  subcenter = s;
  table = t;
  process = p;
  return;
}

void GRIB_FIELD::set_scale(int scale)
{
  decimalscale = scale;
  return;
}

void GRIB_FIELD::set_field(int v, float *values, size_t s, float uh, float ul)
{
  varcode = v;
  varname = (Parm_Table(center, subcenter, table, process) + varcode)->name;
  vardesc = (Parm_Table(center, subcenter, table, process) + varcode)->comment;
  size_t indx0 = vardesc.find_first_of('[', 0);
  size_t indx1 = vardesc.find_first_of(']', 0);
  varunit = vardesc.substr(indx0+1,indx1-indx0-1);
  if (size != s || vals == 0)
  {
    size = s;
    if (vals) delete [ ] vals;
    vals = new float[s];
  }
  memcpy(vals, values, s*sizeof(float));
  undef_high = uh;
  undef_low = ul;
  return;
}

void GRIB_FIELD::add_local_def(int noctets, unsigned char *octets)
{
  nmorepds = noctets;
  morepds = new unsigned char [noctets];
  memcpy(morepds, octets, noctets);
  return;
}

// ############################################################################
// GRIB Message implementation
// ############################################################################

GRIB_MESSAGE::GRIB_MESSAGE( )
{
  message   = 0;
  pds       = 0;
  gds       = 0;
  bms       = 0;
  bds       = 0;
  reclen    = 0;
}

GRIB_MESSAGE::~GRIB_MESSAGE( ) { if (message) delete [ ] message; }

int GRIB_MESSAGE::Decode(unsigned char *msg, size_t msgsize)
{
  if (message) delete [ ] message;

  unsigned char *pnt;

  message = new unsigned char[msgsize];
  memcpy(message, msg, msgsize);
  reclen = msgsize;

  pds = message + 8;
  pnt = pds + PDS_LEN(pds);
  if (PDS_HAS_GDS(pds))
  {
    gds = pnt;
    pnt += GDS_LEN(gds);
  }
  if (PDS_HAS_BMS(pds))
  {
    bms = pnt;
    pnt += BMS_LEN(bms);
  }
  bds = pnt;
  pnt += BDS_LEN(bds);

  if ((size_t) (pnt-message+4) != reclen)
  {
    std::cerr << "Message GRIB lenght is inconsistent" << std::endl;
    return -1;
  }

  gtime.Decode(pds);
  level.Decode(pds);
  grid.Decode(pds, gds, bms, bds);
  field.Decode(pds, bds, bms, (size_t) grid.nxny);

  return 0;
}

void GRIB_MESSAGE::Encode( )
{
  static unsigned char header[8]  = {'G', 'R', 'I', 'B', ' ', ' ', ' ', '\1'};
  static unsigned char trailer[4] = {'7', '7', '7', '7'};

  unsigned char *tenc = gtime.Encode( );
  size_t pdslen = 0, gdslen = 0, bmslen = 0, bdslen = 0;

  if (grid.grid_code == 0)
  {
    std::cerr << "Undefined GRID !" << std::endl;
    throw;
  }

  if (level.type == GRIB_LEVEL_UNKNOWN)
  {
    std::cerr << "Undefined LEVEL !" << std::endl;
    throw;
  }

  if (gtime.timeunit == GRIB_TIMEUNIT_UNKNOWN)
  {
    std::cerr << "Undefined TIME !" << std::endl;
    throw;
  }

  if (field.varcode == GRIB_PARAMETER_UNKNOWN)
  {
    std::cerr << "Undefined FIELD !" << std::endl;
    throw;
  }

  pds = mk_PDS(NULL, WGRIB_ENCODE_INIT,
               field.nmorepds ? (40+field.nmorepds) : 28, 0,
               WGRIB_ENCODE_BYTE,     3, field.table,
               WGRIB_ENCODE_BYTE,     4, field.center,
               WGRIB_ENCODE_BYTE,     5, field.process,
               WGRIB_ENCODE_BYTE,     6, grid.grid_code,
               WGRIB_ENCODE_BYTE,     8, field.varcode,
               WGRIB_ENCODE_BYTE,     9, level.type,
               WGRIB_ENCODE_2BYTES,  10, level.Encode( ),
               WGRIB_ENCODE_BYTE,    12, tenc[0],
               WGRIB_ENCODE_BYTE,    13, tenc[1],
               WGRIB_ENCODE_BYTE,    14, tenc[2],
               WGRIB_ENCODE_BYTE,    15, tenc[3],
               WGRIB_ENCODE_BYTE,    16, tenc[4],
               WGRIB_ENCODE_BYTE,    17, tenc[5],
               WGRIB_ENCODE_BYTE,    18, tenc[6],
               WGRIB_ENCODE_BYTE,    19, tenc[7],
               WGRIB_ENCODE_BYTE,    20, tenc[8],
               WGRIB_ENCODE_BYTE,    21, tenc[9],
               WGRIB_ENCODE_BYTE,    22, tenc[10],
               WGRIB_ENCODE_BYTE,    23, tenc[11],
               WGRIB_ENCODE_BYTE,    24, tenc[12],
               WGRIB_ENCODE_BYTE,    25, field.subcenter,
               WGRIB_ENCODE_S2BYTES, 26, field.decimalscale,
               WGRIB_ENCODE_END);
  if (field.nmorepds) memcpy(pds+40, field.morepds, field.nmorepds);

  pdslen = PDS_LEN(pds);

  gds = grid.Encode( );
  if (gds)
  {
    pds[7] |= 128;              // GDS is included
    gdslen = GDS_LEN(gds);
  }

  int mask_nxny = field.size;
  float *tocomp = new float[field.size];
  memcpy(tocomp, field.vals, field.size*sizeof(float));

  // Create bitmask
  bms = mk_BMS(pds, tocomp, &mask_nxny, field.undef_low, field.undef_high);
  if (bms)
    bmslen = BMS_LEN(bms);

  bds = mk_BDS(pds, tocomp, mask_nxny, &bdslen);
  // bdslen = BDS_LEN(bds);

  delete [ ] tocomp;

  reclen = pdslen + gdslen + bmslen + bdslen + 12;

#ifdef ECMWF_NIGHTMARE_HACK
  size_t padbdslen = 0;
  if (reclen > 0x800000)
  {
    size_t newreclen = ((reclen / 120 ) + ((reclen % 120) ? 1 : 0));
    padbdslen = (newreclen * 120) - reclen;
    reclen = newreclen * 120;
    header[4] = (newreclen >> 16) & 255;
    header[5] = (newreclen >>  8) & 255;
    header[6] = (newreclen      ) & 255;
    header[4] = header[4] | 128;
    bds[0] = ((padbdslen + 4) >> 16) & 255;
    bds[1] = ((padbdslen + 4) >>  8) & 255;
    bds[2] = ((padbdslen + 4)      ) & 255;
  }
  else
  {
    header[4] = (reclen >> 16) & 255;
    header[5] = (reclen >>  8) & 255;
    header[6] = (reclen      ) & 255;
    bds[0] = (bdslen >> 16) & 255;
    bds[1] = (bdslen >>  8) & 255;
    bds[2] = (bdslen      ) & 255;
  }
#else
  header[4] = (reclen >> 16) & 255;
  header[5] = (reclen >>  8) & 255;
  header[6] = (reclen      ) & 255;
  bds[0] = (bdslen >> 16) & 255;
  bds[1] = (bdslen >>  8) & 255;
  bds[2] = (bdslen      ) & 255;
#endif

  if (message) delete [ ] message;
  message = new unsigned char[reclen];

  size_t lpos = 0;
  memcpy(message, header, 8*sizeof(unsigned char));
  lpos += 8;
  memcpy(message+lpos, pds, pdslen*sizeof(unsigned char));
  free(pds);
  pds = message+lpos;
  lpos += pdslen;
  if (gds)
  {
    memcpy(message+lpos, gds, gdslen*sizeof(unsigned char));
    free(gds);
    gds = message+lpos;
    lpos += gdslen;
  }
  if (bms)
  {
    memcpy(message+lpos, bms, bmslen*sizeof(unsigned char));
    free(bms);
    bms = message+lpos;
    lpos += bmslen;
  }
  memcpy(message+lpos, bds, bdslen*sizeof(unsigned char));
  free(bds);
  bds = message+lpos;
  lpos += bdslen;
  memcpy(message+lpos, trailer, 4*sizeof(unsigned char));
  lpos += 4;

#ifdef ECMWF_NIGHTMARE_HACK
  if (padbdslen)
    lpos += padbdslen;
#endif
 
  if (reclen != lpos) throw;

  return;
}

void GRIB_MESSAGE::set_grid(GRIB_GRID &g)
{
  grid.grid_code = g.grid_code;
  grid.type = g.type;
  grid.nxny = g.nxny;
  grid.nx = g.nx;
  grid.ny = g.ny;
  grid.is_dirincgiven = g.is_dirincgiven;
  grid.is_earth_spheroid = g.is_earth_spheroid;
  grid.is_uv_grid = g.is_uv_grid;
  grid.is_x_negative = g.is_x_negative;
  grid.is_y_negative = g.is_y_negative;
  grid.is_fortran = g.is_fortran;
  switch (g.type)
  {
    case GRIB_GRID_REGULAR_LATLON:
      grid.ll.lat1 = g.ll.lat1;
      grid.ll.lon1 = g.ll.lon1;
      grid.ll.lat2 = g.ll.lat2;
      grid.ll.lon2 = g.ll.lon2;
      grid.ll.dlat = g.ll.dlat;
      grid.ll.dlon = g.ll.dlon;
      break;
    case GRIB_GRID_GAUSSIAN:
      grid.gl.lat1 = g.gl.lat1;
      grid.gl.lon1 = g.gl.lon1;
      grid.gl.lat2 = g.gl.lat2;
      grid.gl.lon2 = g.gl.lon2;
      grid.gl.dlon = g.gl.dlon;
      grid.gl.N    = g.gl.N;
      break;
    case GRIB_GRID_POLAR_STEREOGRAPHIC:
      grid.ps.lat1 = g.ps.lat1;
      grid.ps.lon1 = g.ps.lon1;
      grid.ps.lov  = g.ps.lov;
      grid.ps.dx   = g.ps.dx;
      grid.ps.dy   = g.ps.dy;
      grid.ps.pole = g.ps.pole;
      break;
    case GRIB_GRID_LAMBERT_CONFORMAL:
      grid.lc.lat1    = g.lc.lat1;
      grid.lc.lon1    = g.lc.lon1;
      grid.lc.lov     = g.lc.lov;
      grid.lc.latin1  = g.lc.latin1;
      grid.lc.latin2  = g.lc.latin2;
      grid.lc.latsp   = g.lc.latsp;
      grid.lc.lonsp   = g.lc.lonsp;
      grid.lc.dx      = g.lc.dx;
      grid.lc.dy      = g.lc.dy;
      grid.lc.pole    = g.lc.pole;
      grid.lc.bipolar = g.lc.bipolar;
      break;
    case GRIB_GRID_MERCATOR:
      grid.mc.lat1  = g.mc.lat1;
      grid.mc.lon1  = g.mc.lon1;
      grid.mc.lat2  = g.mc.lat2;
      grid.mc.lon2  = g.mc.lon2;
      grid.mc.dx    = g.mc.dx;
      grid.mc.dy    = g.mc.dy;
      grid.mc.latin = g.mc.latin;
      break;
    case GRIB_GRID_SPHERICAL_HARMONIC_COE:
      grid.sa.J  = g.sa.J;
      grid.sa.K  = g.sa.K;
      grid.sa.M  = g.sa.M;
	      grid.sa.representation = g.sa.representation;
      grid.sa.storage_method = g.sa.storage_method;
      break;
    case GRIB_GRID_SPACEVIEW:
      grid.sp.lap  = g.sp.lap;
      grid.sp.lop  = g.sp.lop;
      grid.sp.dx  = g.sp.dx;
      grid.sp.dy  = g.sp.dy;
      grid.sp.Xp  = g.sp.Xp;
      grid.sp.Yp  = g.sp.Yp;
      grid.sp.orient  = g.sp.orient;
      grid.sp.Nr  = g.sp.Nr;
      grid.sp.X0  = g.sp.X0;
      grid.sp.Y0  = g.sp.Y0;
      break;
    default:
      break;
  }
  return;
}

void GRIB_MESSAGE::set_time(GRIB_TIME &t)
{
  gtime.reftime = t.reftime;
  gtime.timestring = t.timestring;
  gtime.year = t.year;
  gtime.month = t.month;
  gtime.day = t.day;
  gtime.hour = t.hour;
  gtime.minute = t.minute;
  gtime.p1 = t.p1;
  gtime.p2 = t.p2;
  gtime.p1p2 = t.p1p2;
  gtime.naveraged = t.naveraged;
  gtime.nmissing = t.nmissing;
  gtime.timeunit = t.timeunit;
  gtime.timerange = t.timerange;
  return;
}

void GRIB_MESSAGE::set_level(GRIB_LEVEL &l)
{
  level.type = l.type;
  level.lv1 = l.lv1;
  level.lv2 = l.lv2;
  return;
}

void GRIB_MESSAGE::set_field(GRIB_FIELD &f)
{
  if (field.vals) delete [ ] field.vals;
  field.vals = 0;
  field.size = 0;

  if (f.size)
  {
    field.size = f.size;
    field.vals = new float[f.size];
    memcpy(field.vals, f.vals, f.size*sizeof(float));
  }
  else
    field.vals = 0;

  field.varcode = f.varcode;
  field.varname = f.varname;
  field.varunit = f.varunit;
  field.vardesc = f.vardesc;
  field.numbits = f.numbits;
  field.refvalue = f.refvalue;
  field.decimalscale = f.decimalscale;
  field.binscale = f.binscale;
  field.undef_high = f.undef_high;
  field.undef_low = f.undef_low;
  field.center = f.center;
  field.subcenter = f.subcenter;
  field.table = f.table;
  field.process = f.process;
  if (f.nmorepds)
  {
    field.nmorepds = f.nmorepds;
    field.morepds = new unsigned char[f.nmorepds];
    memcpy(field.morepds, f.morepds, f.nmorepds);
  }
  return;
}

unsigned char *GRIB_MESSAGE::get_pds_values(int pos, int len)
{
  if (len < 0) return 0;
  if (pos < 0 || pos > (PDS_LEN(pds) - len)) return 0;
  unsigned char *tmp = new unsigned char[len];
  memcpy(tmp, pds+pos, len);
  return tmp;
}

size_t GRIB_MESSAGE::get_pds_size( )
{
  return PDS_LEN(pds);
}

// ############################################################################
// GRIB File outer interface
// ############################################################################

GRIB_FILE::GRIB_FILE( )
{
  pos     = 0;
  size    = 0;
  limit   = 0;
  data    = 0;
  fd      = -1;
}

int GRIB_FILE::OpenRead(const std::string& fname)
{
  struct stat finfo;

  if (stat(fname.c_str(), &finfo) < 0)
  {
    std::cerr << "Error opening input file " << fname << std::endl;
    return -1;
  }

  size = finfo.st_size;
  limit = size-4;

  fd = open(fname.c_str(), O_RDONLY);
  if (fd < 0)
  {
    std::cerr << "Error opening input file " << fname << std::endl;
    return -1;
  }

  data = (unsigned char *) mmap(0, size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (data == MAP_FAILED)
  {
    std::cerr << "Error reading input file " << fname << std::endl;
    return -1;
  }

  return 0;
}

int GRIB_FILE::OpenWrite(char *fname)
{
  fd = open(fname, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
  if (fd < 0)
  {
    std::cerr << "Error opening output file " << fname << std::endl;
    return -1;
  }

  return 0;
}

int GRIB_FILE::OpenWrite(std::string fname)
{
  return this->OpenWrite(fname.c_str( ));
}

int GRIB_FILE::Append(char *fname)
{
  fd = open(fname, O_WRONLY|O_CREAT|O_APPEND, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
  if (fd < 0)
  {
    std::cerr << "Error opening output file " << fname << std::endl;
    return -1;
  }

  return 0;
}

int GRIB_FILE::Append(std::string fname)
{
  return this->Append(fname.c_str( ));
}

int GRIB_FILE::Close( )
{
  if (data) munmap(data, size);
  if (fd > 0) close(fd);
  pos     = 0;
  size    = 0;
  limit   = 0;
  data    = 0;
  fd      = -1;
  return 0;
}

GRIB_FILE::~GRIB_FILE( ) { (void) this->Close( ); }

int GRIB_FILE::ReadMessage(GRIB_MESSAGE &message)
{
  size_t reclen = 0;

  if (pos >= limit)
    return 1;

  char *cpnt = (char *) data;

  while (pos < limit)
  {
    if (*(cpnt+pos)   == 'G' &&
        *(cpnt+pos+1) == 'R' &&
        *(cpnt+pos+2) == 'I' &&
        *(cpnt+pos+3) == 'B') break;
    ++pos;
  }

  if (pos >= limit)
    return 1;

  unsigned char *msg = data+pos;

  reclen = (msg[4]<<16) + (msg[5]<<8) + msg[6];

  if (msg[7] != 1)
  {
    std::cerr << "Message GRIB version is not 1: " << (int) msg[7]
              << std::endl;
    return -1;
  }

  pos = pos + reclen;
  return message.Decode(msg, reclen);
}

int GRIB_FILE::WriteMessage(GRIB_MESSAGE &msg)
{
  size_t wbytes;

  msg.Encode( );
  if (msg.message == 0) throw;

  wbytes = write(fd, (void *) msg.message, msg.reclen);
  if (wbytes != msg.reclen)
  {
    std::cerr << "Error writing output message !" << std::endl;
    throw;
  }

  return 0;
}

#ifdef TESTME

int main(int argc, char *argv[])
{
  if (argc < 2) return -1;

  GRIB_FILE gf;
  GRIB_FILE gn;

  int ret = gf.OpenRead(argv[1]);
  if (ret != 0) return -1;

  GRIB_MESSAGE m;

  while ((ret = gf.ReadMessage(m)) == 0)
  {
    std::cout << "Message:  " << m.field.VarDescription( ) << std::endl;
    std::cout << "Grid :    " << m.grid << std::endl
              << "Level:    " << m.level << std::endl
              << "Time :    " << m.gtime << std::endl << std::endl;

  }

  if (ret < 0) return -1;

  ret = gf.Close( );
  if (ret != 0) return -1;

  GRIB_MESSAGE m1;
  GRIB_GRID g;
  GRIB_LEVEL l;
  GRIB_TIME t;
  GRIB_FIELD f; 

  ret = gn.OpenWrite("outtest.grb");
  if (ret != 0) return -1;

  float *ff = new float[65160];
  for (int i = 0; i < 65160; i ++)
    ff[i] = 273.15 + i/360;

  // set time
  t.set_referencetime(2004, 7, 3, 12, 0);
  t.set_forecast_hour(12);

  // set level
  l.set_pressure(1000.0);

  // set grid
  g.set_size(360, 181);
  g.set_regular_latlon(90.0, -90.0, 0.0, -1.0, 1.0, 1.0);

  // set field values
  f.set_field(GRIB_PARAMETER_TMP__, ff, 65160, 9.9991e20, 9.9991e20);

  m1.set_time(t);
  m1.set_grid(g);
  m1.set_level(l);
  m1.set_field(f);

  gn.WriteMessage(m1);

  for (int i = 0; i < 65160; i ++)
    ff[i] = 10000 + i/360;

  f.set_field(GRIB_PARAMETER_PRES_, ff, 65160, 9.9991e20, 9.9991e20);

  m1.set_field(f);

  gn.WriteMessage(m1);

  ret = gn.Close( );
  if (ret != 0) return -1;

  delete [ ] ff;

  std::cout << "Done." << std::endl;
  return 0;
}

#endif
