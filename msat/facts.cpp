#include "facts.h"

#if 0
#include <sstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <math.h>
#include <limits.h>
#include <cstdlib>
#include "proj/const.h"
#endif
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
                                // SAF special cases
                                case 106:
                                case 546:                       return "CRR";
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
                                // SAF special cases
                                case 106:
                                case 546:                       return "NUMERIC";
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
                                        // SAF special cases
                                case 100:
                                case 101:
                                case 102:
                                case 103:
                                case 104:
                                case 105:
                                case 106:
                                case 107:
                                case 108:
                                case 109:
                                case 110:
                                case 111:
                                case 112:
                                case 113:
                                case 114:
                                case 115:
                                case 116:
                                case 117:
                                case 118:
                                case 119:
                                case 120:
                                case 121:
                                case 122:
                                case 123:
                                case 124:
                                case 125:
                                case 126:
                                case 127:
                                case 128:
                                case 129:
                                case 130:
                                case 131:
                                case 132:
                                case 133:
                                case 134:
                                case 135:
                                case 136:
                                        return "3";
                        }
                        break;
                }
        }
        return "";
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

double sat_za(float lat, float lon)
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


}
}

// vim:set ts=2 sw=2:
