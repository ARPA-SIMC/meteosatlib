#include "facts.h"
#include <stdexcept>
#include <limits>

// For HRIT satellite IDs
#include "hrit/MSG_spacecraft.h"
// For HRIT channel names
#include "hrit/MSG_channel.h"

using namespace std;

static struct ChannelInfo {
        size_t decimalDigits;
        double packedMissing;
        double scaledMissing;
} channelInfo[] = {
        { 2, 0, 0 },            //  0           FIXME: unverified
        { 4, 0, -1.02691 },     //  1
        { 4, 0, 0 },            //  2
        { 4, 0, 0 },            //  3
        { 2, 0, 0 },            //  4
        { 2, 0, 0 },            //  5
        { 2, 0, 0 },            //  6
        { 2, 0, 0 },            //  7
        { 2, 0, 0 },            //  8
        { 2, 0, 0 },            //  9
        { 2, 0, 0 },            // 10
        { 2, 0, 0 },            // 11
        { 4, 0, -1.63196 },     // 12
};

static const size_t channel_info_size = sizeof(channelInfo) / sizeof(struct ChannelInfo);

namespace msat {
namespace facts {

// THIS is the way to get to ColumnDirGridStep and LineDirGridStep
double pixelHSizeFromCFAC(double cfac)
{
        int test_cfac = round(cfac * exp2(16));
        //fprintf(stderr, "Band::pixelHSizeFromCFAC cheat %d", test_cfac);
        switch (test_cfac)
        {
                // Handle known values
                case 13642337: return METEOSAT_PIXELSIZE_X;
                case -13642337: return -METEOSAT_PIXELSIZE_X;
                case 40927000: return METEOSAT_PIXELSIZE_X_HRV;
                case -40927000: return -METEOSAT_PIXELSIZE_X_HRV;
                default: 
                        return (ORBIT_RADIUS - EARTH_RADIUS) * tan(M_PI / 180.0 / cfac) * 1000.0;
        }
}
double pixelVSizeFromLFAC(double lfac)
{
        int test_lfac = round(lfac * exp2(16));
        switch (test_lfac)
        {
                // Handle known values
                case 13642337: return METEOSAT_PIXELSIZE_Y;
                case -13642337: return -METEOSAT_PIXELSIZE_Y;
                case 40927000: return METEOSAT_PIXELSIZE_Y_HRV;
                case -40927000: return -METEOSAT_PIXELSIZE_Y_HRV;
                default:
                        return (ORBIT_RADIUS - EARTH_RADIUS) * tan(M_PI / 180.0 / lfac) * 1000.0;
        }
}

double CFACFromPixelHSize(double psx)
{
        // Handle known values
        //fprintf(stderr, "Band::CFACFromPixelHSize cheat %f\n", psx - METEOSAT_PIXELSIZE_X);
        if (fabs(psx - METEOSAT_PIXELSIZE_X) < 0.001) return 13642337*exp2(-16);
        if (fabs(psx - -METEOSAT_PIXELSIZE_X) < 0.001) return -13642337*exp2(-16);
        if (fabs(psx - METEOSAT_PIXELSIZE_X_HRV) < 0.001) return 40927000*exp2(-16);
        if (fabs(psx - -METEOSAT_PIXELSIZE_X_HRV) < 0.001) return -40927000*exp2(-16);
        return M_PI / atan(psx / ((ORBIT_RADIUS - EARTH_RADIUS) * 1000.0)) / 180.0;
}

double LFACFromPixelVSize(double psy)
{
        if (fabs(psy - METEOSAT_PIXELSIZE_Y) < 0.001) return 13642337*exp2(-16);
        if (fabs(psy - -METEOSAT_PIXELSIZE_Y) < 0.001) return -13642337*exp2(-16);
        if (fabs(psy - METEOSAT_PIXELSIZE_Y_HRV) < 0.001) return 40927000*exp2(-16);
        if (fabs(psy - -METEOSAT_PIXELSIZE_Y_HRV) < 0.001) return -40927000*exp2(-16);
        return M_PI / atan(psy / ((ORBIT_RADIUS - EARTH_RADIUS) * 1000.0)) / 180.0;
}

int seviriDXFromColumnRes(double column_res)
{
#if 0
        double ps = (ORBIT_RADIUS - EARTH_RADIUS) * tan( (1.0/column_factor/exp2(-16)) * PI / 180 );
        return round((2 * asin(EARTH_RADIUS / ORBIT_RADIUS)) / atan(ps / (ORBIT_RADIUS-EARTH_RADIUS)));
#endif

        // This computation has been found by Dr2 Francesca Di Giuseppe and simplified by Enrico Zini
        return (int)round(asin(EARTH_RADIUS / ORBIT_RADIUS) * column_res * 360 / M_PI);
}

int seviriDYFromLineRes(double line_res)
{
        // This computation has been found by Dr2 Francesca Di Giuseppe and simplified by Enrico Zini
        return (int)round(asin(EARTH_RADIUS / ORBIT_RADIUS) * line_res * 360 / M_PI);
        //return round((2 * asin(EARTH_RADIUS / ORBIT_RADIUS)) / atan(pixelVSize() / (ORBIT_RADIUS-EARTH_RADIUS)));
}

double columnResFromSeviriDX(int seviriDX)
{
        return (double)seviriDX * M_PI / (asin(EARTH_RADIUS / ORBIT_RADIUS)*360);
}

double lineResFromSeviriDY(int seviriDY)
{
        return (double)seviriDY * M_PI / (asin(EARTH_RADIUS / ORBIT_RADIUS)*360);
}

int seviriDXFromCFAC(double column_res)
{
#if 0
        double ps = (ORBIT_RADIUS - EARTH_RADIUS) * tan( (1.0/column_factor/exp2(-16)) * PI / 180 );
        return round((2 * asin(EARTH_RADIUS / ORBIT_RADIUS)) / atan(ps / (ORBIT_RADIUS-EARTH_RADIUS)));
#endif

        // This computation has been found by Dr2 Francesca Di Giuseppe and simplified by Enrico Zini
        return (int)round(asin(EARTH_RADIUS / ORBIT_RADIUS) * column_res * 360 / M_PI);
}

int seviriDYFromLFAC(double line_res)
{
        // This computation has been found by Dr2 Francesca Di Giuseppe and simplified by Enrico Zini
        return (int)round(asin(EARTH_RADIUS / ORBIT_RADIUS) * line_res * 360 / M_PI);
        //return round((2 * asin(EARTH_RADIUS / ORBIT_RADIUS)) / atan(pixelVSize() / (ORBIT_RADIUS-EARTH_RADIUS)));
}

double CFACFromSeviriDX(int seviriDX)
{
        return (double)seviriDX * M_PI / (asin(EARTH_RADIUS / ORBIT_RADIUS)*360);
}

double LFACFromSeviriDY(int seviriDY)
{
        return (double)seviriDY * M_PI / (asin(EARTH_RADIUS / ORBIT_RADIUS)*360);
}

int seviriDXFromPixelHSize(double psx)
{
        // Handle well-known values
        if (fabs(psx - METEOSAT_PIXELSIZE_X) < 0.001)
                return 3622;
        else
                return seviriDXFromCFAC(CFACFromPixelHSize(psx));
}

int seviriDYFromPixelVSize(double psy)
{
        // Handle well-known values
        if (fabs(psy - METEOSAT_PIXELSIZE_Y) < 0.001)
                return 3622;
        else
                return seviriDYFromLFAC(LFACFromPixelVSize(psy));
}

double pixelHSizeFromSeviriDX(int dx)
{
        switch (dx)
        {
                // Handle well-known values
                case 3608: return METEOSAT_PIXELSIZE_X;
                case 3622: return METEOSAT_PIXELSIZE_X;
                default:
                        return pixelHSizeFromCFAC(CFACFromSeviriDX(dx));
        }
}

double pixelVSizeFromSeviriDY(int dy)
{
        switch (dy)
        {
                // Handle well-known values
                case 3608: return METEOSAT_PIXELSIZE_Y;
                case 3622: return METEOSAT_PIXELSIZE_Y;
                default:
                        return pixelVSizeFromLFAC(LFACFromSeviriDY(dy));
        }
}

int spacecraftIDFromHRIT(int id)
{
        switch (id)
        {
                case MSG_NO_SPACECRAFT: return 1023;
                case MSG_METOP_1      : return 3;
                case MSG_METOP_2      : return 4;
                case MSG_METOP_3      : return 5;
                case MSG_METEOSAT_3   : return 50;
                case MSG_METEOSAT_4   : return 51;
                case MSG_METEOSAT_5   : return 52;
                case MSG_METEOSAT_6   : return 53;
        //      case MSG_MTP_1        : return 3;
        //      case MSG_MTP_2        : return 4;
                case MSG_MSG_1        : return 55;
                case MSG_MSG_2        : return 56;
                case MSG_MSG_3        : return 57;
                case MSG_MSG_4        : return 70;
                case MSG_NOAA_12      : return 204;
        //      case MSG_NOAA_13      : return ;
                case MSG_NOAA_14      : return 205;
                case MSG_NOAA_15      : return 206;
                case MSG_GOES_7       : return 251;
                case MSG_GOES_8       : return 252;
                case MSG_GOES_9       : return 253;
                case MSG_GOES_10      : return 254;
                case MSG_GOES_11      : return 255;
                case MSG_GOES_12      : return 256;
                case MSG_GOMS_1       : return 310;
                case MSG_GOMS_2       : return 311;
        //      case MSG_GOMS_3       : return ;
                case MSG_GMS_4        : return 151;
                case MSG_GMS_5        : return 152;
        //      case MSG_GMS_6        : return ;
                case MSG_MTSAT_1      : return 58;
                case MSG_MTSAT_2      : return 59;
                default:                return 1023;
        }
}

int spacecraftIDToHRIT(int id)
{
        switch (id)
        {
                case 1023: return MSG_NO_SPACECRAFT;
                case 3:    return MSG_METOP_1;
                case 4:    return MSG_METOP_2;
                case 5:    return MSG_METOP_3;
                case 50:   return MSG_METEOSAT_3;
                case 51:   return MSG_METEOSAT_4;
                case 52:   return MSG_METEOSAT_5;
                case 53:   return MSG_METEOSAT_6;
        //      case 3:    return MSG_MTP_1;
        //      case 4:    return MSG_MTP_2;
                case 55:   return MSG_MSG_1;
                case 56:   return MSG_MSG_2;
                case 57:   return MSG_MSG_3;
                case 70:   return MSG_MSG_4;
                case 204:  return MSG_NOAA_12;
                case 205:  return MSG_NOAA_14;
                case 206:  return MSG_NOAA_15;
                case 251:  return MSG_GOES_7;
                case 252:  return MSG_GOES_8;
                case 253:  return MSG_GOES_9;
                case 254:  return MSG_GOES_10;
                case 255:  return MSG_GOES_11;
                case 256:  return MSG_GOES_12;
                case 310:  return MSG_GOMS_1;
                case 311:  return MSG_GOMS_2;
                case 151:  return MSG_GMS_4;
                case 152:  return MSG_GMS_5;
                case 58:   return MSG_MTSAT_1;
                case 59:   return MSG_MTSAT_2;
                default:   return MSG_NO_SPACECRAFT;
        }
}

// Reimplemented here to be able to link without libhrit in case it's disabled
const char* spacecraftName(int spacecraft_id)
{
        switch (spacecraft_id)
        {
                case 3: return "METOP1";
                case 4: return "METOP2";
                case 5: return "METOP3";
                case 50: return "METEOSAT3";
                case 51: return "METEOSAT4";
                case 52: return "METEOSAT5";
                case 53: return "METEOSAT6";
                case 54: return "METEOSAT7";
                case 55: return "MSG1";
                case 56: return "MSG2";
                case 57: return "MSG3";
                case 58: return "MTSAT1";
                case 59: return "MTSAT2";
                case 70: return "MSG4";
                case 150: return "GMS3";
                case 151: return "GMS4";
                case 152: return "GMS5";
                case 204: return "NOAA12";
                case 205: return "NOAA14";
                case 206: return "NOAA15";
                case 251: return "GOES7";
                case 252: return "GOES8";
                case 253: return "GOES9";
                case 254: return "GOES10";
                case 255: return "GOES11";
                case 256: return "GOES12";
                case 310: return "GOMS1";
                case 311: return "GOMS2";
                case 999: return "Non Spacecraft";
                case 1023: return "Non Spacecraft";
                default:  return "unknown";
        }
}

int spacecraftID(const std::string& name)
{
        if (name == "METOP1")       return    3;
        if (name == "METOP2")       return    4;
        if (name == "METOP3")       return    5;
        if (name == "METEOSAT3") return  50;
        if (name == "METEOSAT4") return  51;
        if (name == "METEOSAT5") return  52;
        if (name == "METEOSAT6") return  53;
        if (name == "METEOSAT7") return  54;
        if (name == "MSG1")         return   55;
        if (name == "MSG2")         return   56;
        if (name == "MSG3")         return   57;
        if (name == "MTSAT1")       return   58;
        if (name == "MTSAT2")       return   59;
        if (name == "MSG4")         return   70;
        if (name == "GMS3")         return  150;
        if (name == "GMS4")         return  151;
        if (name == "GMS5")         return  152;
        if (name == "NOAA12")       return  204;
        if (name == "NOAA14")       return  205;
        if (name == "NOAA15")       return  206;
        if (name == "GOES7")        return  251;
        if (name == "GOES8")        return  252;
        if (name == "GOES9")        return  253;
        if (name == "GOES10")       return  254;
        if (name == "GOES11")       return  255;
        if (name == "GOES12")       return  256;
        if (name == "GOMS1")        return  310;
        if (name == "GOMS2")        return  311;
        return -1;
}

// Reimplemented here to be able to link without libhrit in case it's disabled
const char* sensorName(int spacecraftID)
{
        switch (spacecraftID)
        {
                case 55: return "Seviri";
                case 56: return "Seviri";
                case 57: return "Seviri";
                case 70: return "Seviri";
                default: return "unknown";
        }
}

// Reimplemented here to be able to link without libhrit in case it's disabled
const char* channelName(int spacecraftID, int channelID)
{
        switch (spacecraftID)
        {
                case 55:
                case 56:
                case 57:
                case 70:
                {
                        switch (channelID)
                        {
                                case MSG_SEVIRI_NO_CHANNEL:     return "no-channel";
                                case MSG_SEVIRI_1_5_VIS_0_6:    return "VIS006";
                                case MSG_SEVIRI_1_5_VIS_0_8:    return "VIS008";
                                case MSG_SEVIRI_1_5_IR_1_6:     return "IR_016";
                                case MSG_SEVIRI_1_5_IR_3_9:     return "IR_039";
                                case MSG_SEVIRI_1_5_WV_6_2:     return "WV_062";
                                case MSG_SEVIRI_1_5_WV_7_3:     return "WV_073";
                                case MSG_SEVIRI_1_5_IR_8_7:     return "IR_087";
                                case MSG_SEVIRI_1_5_IR_9_7:     return "IR_097";
                                case MSG_SEVIRI_1_5_IR_10_8:    return "IR_108";
                                case MSG_SEVIRI_1_5_IR_12_0:    return "IR_120";
                                case MSG_SEVIRI_1_5_IR_13_4:    return "IR_134";
                                case MSG_SEVIRI_1_5_HRV:        return "HRV";
                        }
                        break;
                }
        }
        return "unknown";
}

const char* channelUnit(int spacecraftID, int channelID)
{
        switch (spacecraftID)
        {
                case 55:
                case 56:
                case 57:
                case 70:
                {
                        switch (channelID)
                        {
                                case MSG_SEVIRI_NO_CHANNEL:     return "unknown";
                                case MSG_SEVIRI_1_5_VIS_0_6:    return "mW m^-2 sr^-1 (cm^-1)^-1";
                                case MSG_SEVIRI_1_5_VIS_0_8:    return "mW m^-2 sr^-1 (cm^-1)^-1";
                                case MSG_SEVIRI_1_5_IR_1_6:     return "mW m^-2 sr^-1 (cm^-1)^-1";
                                case MSG_SEVIRI_1_5_IR_3_9:     return "K";
                                case MSG_SEVIRI_1_5_WV_6_2:     return "K";
                                case MSG_SEVIRI_1_5_WV_7_3:     return "K";
                                case MSG_SEVIRI_1_5_IR_8_7:     return "K";
                                case MSG_SEVIRI_1_5_IR_9_7:     return "K";
                                case MSG_SEVIRI_1_5_IR_10_8:    return "K";
                                case MSG_SEVIRI_1_5_IR_12_0:    return "K";
                                case MSG_SEVIRI_1_5_IR_13_4:    return "K";
                                case MSG_SEVIRI_1_5_HRV:        return "mW m^-2 sr^-1 (cm^-1)^-1";
                        }
                        break;
                }
        }
        return "unknown";
}

const char* channelLevel(int spacecraftID, int channelID)
{
        switch (spacecraftID)
        {
                case 55:
                case 56:
                case 57:
                case 70:
                {
                        switch (channelID)
                        {
                                case MSG_SEVIRI_NO_CHANNEL:
                                case MSG_SEVIRI_1_5_VIS_0_6:
                                case MSG_SEVIRI_1_5_VIS_0_8:
                                case MSG_SEVIRI_1_5_IR_1_6:
                                case MSG_SEVIRI_1_5_IR_3_9:
                                case MSG_SEVIRI_1_5_WV_6_2:
                                case MSG_SEVIRI_1_5_WV_7_3:
                                case MSG_SEVIRI_1_5_IR_8_7:
                                case MSG_SEVIRI_1_5_IR_9_7:
                                case MSG_SEVIRI_1_5_IR_10_8:
                                case MSG_SEVIRI_1_5_IR_12_0:
                                case MSG_SEVIRI_1_5_IR_13_4:
                                case MSG_SEVIRI_1_5_HRV:
                                        return "1.5";
                        }
                        break;
                }
        }
        return "";
}

double channel_central_wavelength(int spacecraftID, int channelID)
{
    switch (spacecraftID)
    {
        case 55:
        case 56:
        case 57:
        case 70:
            switch (channelID)
            {
                case MSG_SEVIRI_1_5_VIS_0_6:    return  0.6;
                case MSG_SEVIRI_1_5_VIS_0_8:    return  0.8;
                case MSG_SEVIRI_1_5_IR_1_6:     return  1.6;
                case MSG_SEVIRI_1_5_IR_3_9:     return  3.9;
                case MSG_SEVIRI_1_5_WV_6_2:     return  6.2;
                case MSG_SEVIRI_1_5_WV_7_3:     return  7.3;
                case MSG_SEVIRI_1_5_IR_8_7:     return  8.7;
                case MSG_SEVIRI_1_5_IR_9_7:     return  9.7;
                case MSG_SEVIRI_1_5_IR_10_8:    return 10.8;
                case MSG_SEVIRI_1_5_IR_12_0:    return 12.0;
                case MSG_SEVIRI_1_5_IR_13_4:    return 13.4;
                case MSG_SEVIRI_1_5_HRV:        return  0.7;
            }
            break;
    }
    throw std::runtime_error("central wavelength unknown for satellite " + to_string(spacecraftID) + " and channel " + to_string(channelID));
}

double channel_central_wave_number(int spacecraftID, int channelID)
{
    double wavelength = channel_central_wavelength(spacecraftID, channelID);
    // Inverse of the wavelength in meters
    return 1.0 / (wavelength * 0.000001);
}

int channel_from_central_wavelength(int spacecraftID, double wavelength)
{
    switch (spacecraftID)
    {
        case 55:
        case 56:
        case 57:
        case 70:
            break;
        default:
            throw std::runtime_error("only satellite IDs from 55, 56 and 57 are supported (got: " + to_string(spacecraftID) + ")");
    }

    int val = round(wavelength * 10.0);
    switch (val)
    {
        case   6: return MSG_SEVIRI_1_5_VIS_0_6;
        case   8: return MSG_SEVIRI_1_5_VIS_0_8;
        case  16: return MSG_SEVIRI_1_5_IR_1_6;
        case  39: return MSG_SEVIRI_1_5_IR_3_9;
        case  62: return MSG_SEVIRI_1_5_WV_6_2;
        case  73: return MSG_SEVIRI_1_5_WV_7_3;
        case  87: return MSG_SEVIRI_1_5_IR_8_7;
        case  97: return MSG_SEVIRI_1_5_IR_9_7;
        case 108: return MSG_SEVIRI_1_5_IR_10_8;
        case 120: return MSG_SEVIRI_1_5_IR_12_0;
        case 134: return MSG_SEVIRI_1_5_IR_13_4;
        case   7: return MSG_SEVIRI_1_5_HRV;
    }

    throw std::runtime_error("unknown central wavelength " + to_string(val));
}

int channel_from_central_wave_number(int spacecraftID, double wave_number)
{
    double wavelength = (1 / wave_number) * 1000000;
    return channel_from_central_wavelength(spacecraftID, wavelength);
}

double defaultPackedMissing(int channel_id)
{
        if (channel_id >= 0 && (size_t)channel_id < channel_info_size)
                // When it's a known channel, se can use our internal table
                return channelInfo[channel_id].packedMissing;
        else
                // Otherwise, default on something 
                return std::numeric_limits<double>::quiet_NaN();
}

double defaultScaledMissing(int channel_id)
{
        if (channel_id >= 0 && (size_t)channel_id < channel_info_size)
                // When it's a known channel, se can use our internal table
                return channelInfo[channel_id].scaledMissing;
        else
                // Otherwise, default on something 
                return std::numeric_limits<double>::quiet_NaN();
}

int significantDigitsForChannel(int channel)
{
        if (channel >= 0 && (size_t)channel < channel_info_size)
                // When it's a known channel, se can use our internal table
                return channelInfo[channel].decimalDigits;
        return 0;
}

double sat_za(double lat, double lon)
{
    // Convert to radians
    double rlat = lat * M_PI / 180.0;
    double rlon = lon * M_PI / 180.0;

    // http://www.spaceacademy.net.au/watch/track/locgsat.htm
    // g is the angle at the center of earth between the observer and the
    //   satellite
    double cosg = cos(rlon) * cos(rlat);
    // R is the radius of earth
    // h is the hight of the satellite above the equator
    // e is the angle between the satellite and the horizon at the observer
    //   location
    // tan(e) = [(R+h)cos(g) - R] / [(R+h)sin(g)]
    //   simplifying (R+h) it becomes
    // tan(e) = (cos(g) - R/(R+h)) / sin(g)
    //   replacing  sin(g) = sqrt(1-cos²(g))  it becomes
    // tan(e) = (cos(g) - R/(R+h)) / sqrt(1-cos²(g))

    // R / (R + h)
    const double rrh = EARTH_RADIUS / ORBIT_RADIUS;

    // Elevation
    double ele = atan((cosg - rrh) / sqrt(1 - cosg*cosg));

    // Zenith angle
    return M_PI/2 - ele;
}

int jday(int yr, int month, int day)
{
  bool leap;
  double j = 0.0;

  if (((yr%4) == 0 && (yr%100) != 0) || (yr%400) == 0)
    leap = true;
  else
    leap = false;
  
  if (month == 1) j = 0.0;
  if (month == 2) j = 31.0;
  if (month == 3) j = 59.0;
  if (month == 4) j = 90.0;
  if (month == 5) j = 120.0;
  if (month == 6) j = 151.0;
  if (month == 7) j = 181.0;
  if (month == 8) j = 212.0;
  if (month == 9) j = 243.0;
  if (month == 10) j = 273.0;
  if (month == 11) j = 304.0;
  if (month == 12) j = 334.0;
  if (month > 2 && leap) j = j + 1.0;

  j = j + day;
  return j;
}


// From MSG_data_RadiometricProc.cpp
double cos_sol_za(int yr, int month, int day, int hour, int minute,
           double lat, double lon)
{
  double hourz = (double)hour + ((double)minute) / 60.0;
  double jd = jday(yr, month, day);
  double zenith;

  zenith = cos_sol_za(jd, hourz, lat, lon);

  return zenith;
}

double cos_sol_za(int jday, int hour, int minute, double dlat, double dlon)
{
  double hourz = (double)hour + ((double)minute) / 60.0;
  return cos_sol_za(jday, hourz, dlat, dlon);
}

double cos_sol_za(int jday, double hour, double dlat, double dlon)
{
  // http://en.wikipedia.org/wiki/Solar_zenith_angle
  double coz;

  const double sinob = 0.3978;
  // Days per year
  const double dpy = 365.242;
  // Degrees per hour, speed of earth rotation
  const double dph = 15.0;
  // From degrees to radians
  const double rpd = M_PI/180.0;
  // From radians to degrees
  const double dpr = 1.0/rpd;
  // Angle in earth orbit in radians, 0 = the beginning of the year
  double dang = 2.0*M_PI*(double)(jday-1)/dpy;
  double homp = 12.0 + 0.123570*sin(dang) - 0.004289*cos(dang) +
                0.153809*sin(2.0*dang) + 0.060783*cos(2.0*dang);
  // Hour angle in the local solar time (degrees)
  double hang = dph* (hour-homp) + dlon;
  double ang = 279.9348*rpd + dang;
  double sigma = (ang*dpr+0.4087*sin(ang)+1.8724*cos(ang)-
                 0.0182*sin(2.0*ang)+0.0083*cos(2.0*ang))*rpd;
  // Sin of sun declination
  double sindlt = sinob*sin(sigma);
  // Cos of sun declination
  double cosdlt = sqrt(1.0-sindlt*sindlt);

  coz = sindlt*sin(rpd*dlat) + cosdlt*cos(rpd*dlat)*cos(rpd*hang);

  return coz;
}


}
}

// vim:set ts=2 sw=2:
