#include "Image.h"
#include "gdalutils.h"

#include <sstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <limits>
#include <math.h>
#include <limits.h>
#include "proj/const.h"

#include <gdal/gdal_priv.h>

// For HRIT satellite IDs
#include <hrit/MSG_spacecraft.h>
// For HRIT channel names
#include <hrit/MSG_channel.h>

using namespace std;

static struct ChannelInfo {
	size_t decimalDigits;
	double packedMissing;
	double scaledMissing;
} channelInfo[] = {
	{ 2, 0, 0 },	//  0		FIXME: unverified
	{ 4, 0, -1.02691 },	//  1
	{ 4, 0, 0 },	//  2
	{ 4, 0, 0 },	//  3
	{ 2, 0, 0 },	//  4
	{ 2, 0, 0 },	//  5
	{ 2, 0, 0 },	//  6
	{ 2, 0, 0 },	//  7
	{ 2, 0, 0 },	//  8
	{ 2, 0, 0 },	//  9
	{ 2, 0, 0 },	// 10
	{ 2, 0, 0 },	// 11
	{ 4, 0, -1.63196 },	// 12
};

static const size_t channel_info_size = sizeof(channelInfo) / sizeof(struct ChannelInfo);

namespace msat {

//
// Image
//

Image::~Image() { /* if (data) delete data; */ }

bool Band::scalesToInt()
{
	switch (GetRasterDataType())
	{
		case GDT_Byte:
		case GDT_UInt16:
		case GDT_Int16:
		case GDT_UInt32:
		case GDT_Int32:
			return true;
		default:
			return false;
	}
}

void Image::setQualityFromPathname(const std::string& pathname)
{
	size_t pos = pathname.rfind('/');
	if (pos == string::npos)
		pos = 1;
	else
		pos += 1;
	if (pos < pathname.size() && pathname[pos] == '_')
		quality = pathname[pos-1];
}

#if 0
void Image::addToHistory(const std::string& event)
{
	if (history.empty())
		history = event;
	else
		history += ", " + event;
}
#endif

std::string Image::historyPlusEvent(const std::string& event)
{
	string history = getHistory();
	if (history.empty())
		return event;
	else
		return history + ", " + event;
}

void Image::setTime(int year, int month, int day, int hour, int minute, int second)
{
	throw std::runtime_error("setTime called on read-only image");
}

void Image::setSpacecraft(int id)
{
	throw std::runtime_error("setSpacecraft called on read-only image");
}

void Image::getMsatGeotransform(int& xs, int& ys, double& psx, double& psy)
{
	double gt[6];
	GetGeoTransform(gt);
	
	psx = gt[1];
	psy = -gt[5];

	xs = lrint(-gt[0] / psx);
	ys = lrint(gt[3] / psy);
}

void Image::setMsatGeotransform(int xs, int ys, double psx, double psy)
{
	double gt[6];

	gt[0] = -(xs) * psx;
	gt[3] = (ys) * psy;
	gt[1] = psx;
	gt[5] = -psy;
	gt[2] = 0.0;
	gt[4] = 0.0;

	SetGeoTransform(gt);
}

void Band::setName(const std::string& name)
{
	throw std::runtime_error("setName called on read-only raster band");
}

void Band::setPreferredBpp(int bpp)
{
	// Do nothing by default
}

void Band::setChannel(int id)
{
	throw std::runtime_error("setChannel called on read-only raster band");
}

// THIS is the way to get to ColumnDirGridStep and LineDirGridStep
double Band::pixelHSizeFromCFAC(double cfac)
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
double Band::pixelVSizeFromLFAC(double lfac)
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
double Band::CFACFromPixelHSize(double psx)
{
	// Handle known values
	//fprintf(stderr, "Band::CFACFromPixelHSize cheat %f\n", psx - METEOSAT_PIXELSIZE_X);
	if (fabs(psx - METEOSAT_PIXELSIZE_X) < 0.001) return 13642337*exp2(-16);
	if (fabs(psx - -METEOSAT_PIXELSIZE_X) < 0.001) return -13642337*exp2(-16);
	if (fabs(psx - METEOSAT_PIXELSIZE_X_HRV) < 0.001) return 40927000*exp2(-16);
	if (fabs(psx - -METEOSAT_PIXELSIZE_X_HRV) < 0.001) return -40927000*exp2(-16);
	return M_PI / atan(psx / ((ORBIT_RADIUS - EARTH_RADIUS) * 1000.0)) / 180.0;
}

double Band::LFACFromPixelVSize(double psy)
{
	if (fabs(psy - METEOSAT_PIXELSIZE_Y) < 0.001) return 13642337*exp2(-16);
	if (fabs(psy - -METEOSAT_PIXELSIZE_Y) < 0.001) return -13642337*exp2(-16);
	if (fabs(psy - METEOSAT_PIXELSIZE_Y_HRV) < 0.001) return 40927000*exp2(-16);
	if (fabs(psy - -METEOSAT_PIXELSIZE_Y_HRV) < 0.001) return -40927000*exp2(-16);
	return M_PI / atan(psy / ((ORBIT_RADIUS - EARTH_RADIUS) * 1000.0)) / 180.0;
}

int Band::seviriDXFromCFAC(double column_res)
{
#if 0
	double ps = (ORBIT_RADIUS - EARTH_RADIUS) * tan( (1.0/column_factor/exp2(-16)) * PI / 180 );
	return round((2 * asin(EARTH_RADIUS / ORBIT_RADIUS)) / atan(ps / (ORBIT_RADIUS-EARTH_RADIUS)));
#endif

	// This computation has been found by Dr2 Francesca Di Giuseppe and simplified by Enrico Zini
	return (int)round(asin(EARTH_RADIUS / ORBIT_RADIUS) * column_res * 360 / M_PI);
}

int Band::seviriDYFromLFAC(double line_res)
{
	// This computation has been found by Dr2 Francesca Di Giuseppe and simplified by Enrico Zini
	return (int)round(asin(EARTH_RADIUS / ORBIT_RADIUS) * line_res * 360 / M_PI);
	//return round((2 * asin(EARTH_RADIUS / ORBIT_RADIUS)) / atan(pixelVSize() / (ORBIT_RADIUS-EARTH_RADIUS)));
}

double Band::CFACFromSeviriDX(int seviriDX)
{
	return (double)seviriDX * M_PI / (asin(EARTH_RADIUS / ORBIT_RADIUS)*360);
}

double Band::LFACFromSeviriDY(int seviriDY)
{
	return (double)seviriDY * M_PI / (asin(EARTH_RADIUS / ORBIT_RADIUS)*360);
}

int Band::seviriDXFromPixelHSize(double psx)
{
	// Handle well-known values
	if (fabs(psx - METEOSAT_PIXELSIZE_X) < 0.001)
		return 3622;
	else
		return seviriDXFromCFAC(CFACFromPixelHSize(psx));
}

int Band::seviriDYFromPixelVSize(double psy)
{
	// Handle well-known values
	if (fabs(psy - METEOSAT_PIXELSIZE_Y) < 0.001)
		return 3622;
	else
		return seviriDYFromLFAC(LFACFromPixelVSize(psy));
}

double Band::pixelHSizeFromSeviriDX(int dx)
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

double Band::pixelVSizeFromSeviriDY(int dy)
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

int Image::spacecraftIDFromHRIT(int id)
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
//  	case MSG_MTP_1        : return 3;
//  	case MSG_MTP_2        : return 4;
  	case MSG_MSG_1        : return 55;
  	case MSG_MSG_2        : return 56;
  	case MSG_MSG_3        : return 57;
  	case MSG_MSG_4        : return 70;
  	case MSG_NOAA_12      : return 204;
//  	case MSG_NOAA_13      : return ;
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
//  	case MSG_GOMS_3       : return ;
  	case MSG_GMS_4        : return 151;
  	case MSG_GMS_5        : return 152;
//  	case MSG_GMS_6        : return ;
  	case MSG_MTSAT_1      : return 58;
  	case MSG_MTSAT_2      : return 59;
		default:								return 1023;
	}
}

int Image::spacecraftIDToHRIT(int id)
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
//  	case 3:    return MSG_MTP_1;
//  	case 4:    return MSG_MTP_2;
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
		default:	 return MSG_NO_SPACECRAFT;
	}
}

std::string Image::datetime()
{
	stringstream res;
	int year, month, day, hour, minute, second;
	getTime(year, month, day, hour, minute, second);

	res << setfill('0') << setw(4) << year << "-" << setw(2) << month << "-" << setw(2) << day
			<< " " << setw(2) << hour << ":" << setw(2) << minute;
	return res.str();
}

int Band::decimalDigitsOfScaledValues()
{
	return decimalDigitsOfScaledValues(GetScale());
}

int Band::decimalDigitsOfScaledValues(double scale)
{
	int channel_id = getChannel();
	if (channel_id >= 0 && (size_t)channel_id < channel_info_size)
		// When it's a known channel, se can use our internal table
		return channelInfo[channel_id].decimalDigits;
	else if (scalesToInt())
	{
		// When the original value was an integer value, we can compute the log10
		// of the scaling factor, add 1 if the scaling factor is not a direct power
		// of 10 and use the result as the count of decimal digits

		int res = -(int)round(log10(scale));
		if (exp10(-res) == scale)
			return res;
		else
			return res + 1;
	} else {
		stringstream str;
		str << "no information found for channel " << channel_id;
		throw std::runtime_error(str.str());
	}
}

double Band::scaled(int column, int line)
{
	double res;
	gdalChecked(RasterIO(GF_Read, column, line, 1, 1, &res, 1, 1, gdalType<double>(), 0, 0));
	return res * GetScale() + GetOffset();
}

double Band::scaledNoDataValue()
{
	return GetNoDataValue() * GetScale() + GetOffset();
}

double Band::defaultPackedMissing(int channel_id)
{
	if (channel_id >= 0 && (size_t)channel_id < channel_info_size)
		// When it's a known channel, se can use our internal table
		return channelInfo[channel_id].packedMissing;
	else
		// Otherwise, default on something 
		return std::numeric_limits<double>::quiet_NaN();
}

double Band::defaultScaledMissing(int channel_id)
{
	if (channel_id >= 0 && (size_t)channel_id < channel_info_size)
		// When it's a known channel, se can use our internal table
		return channelInfo[channel_id].scaledMissing;
	else
		// Otherwise, default on something 
		return std::numeric_limits<double>::quiet_NaN();
}

// Reimplemented here to be able to link without libhrit in case it's disabled
std::string Image::spacecraftName(int spacecraft_id)
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
		default:
			stringstream str;
			str << "unknown" << spacecraft_id;
			return str.str();
  }
}

int Image::spacecraftID(const std::string& name)
{
    if (name == "METOP1")	return    3;
    if (name == "METOP2")	return    4;
    if (name == "METOP3")	return    5;
    if (name == "METEOSAT3") return  50;
    if (name == "METEOSAT4") return  51;
    if (name == "METEOSAT5") return  52;
    if (name == "METEOSAT6") return  53;
    if (name == "METEOSAT7") return  54;
    if (name == "MSG1")		return   55;
    if (name == "MSG2")		return   56;
    if (name == "MSG3")		return   57;
    if (name == "MTSAT1")	return   58;
    if (name == "MTSAT2")	return   59;
    if (name == "MSG4")		return   70;
    if (name == "GMS3")		return  150;
    if (name == "GMS4")		return  151;
    if (name == "GMS5")		return  152;
    if (name == "NOAA12")	return  204;
    if (name == "NOAA14")	return  205;
    if (name == "NOAA15")	return  206;
    if (name == "GOES7")	return  251;
    if (name == "GOES8")	return  252;
    if (name == "GOES9")	return  253;
    if (name == "GOES10")	return  254;
    if (name == "GOES11")	return  255;
    if (name == "GOES12")	return  256;
    if (name == "GOMS1")	return  310;
    if (name == "GOMS2")	return  311;
	throw std::runtime_error("Unknown spacecraft name " + name);
}

// Reimplemented here to be able to link without libhrit in case it's disabled
std::string Image::sensorName(int spacecraftID)
{
	switch (spacecraftID)
	{
		case 55: return "Seviri";
		case 56: return "Seviri";
		default:
			stringstream str;
			str << "unknown" << spacecraftID;
			return str.str();
	}
}

// Reimplemented here to be able to link without libhrit in case it's disabled
std::string Image::channelName(int spacecraftID, int channelID)
{
	switch (spacecraftID)
	{
		case 54: // "METEOSAT7"
			switch (channelID)
			{
				case 1: return "VIS";
				case 2: return "IR";
				case 3: return "WV";
			}
			break;
		case 55:
		case 56:
		{
			switch (channelID)
			{
				case MSG_SEVIRI_NO_CHANNEL:		return "no-channel";
				case MSG_SEVIRI_1_5_VIS_0_6:	return "VIS006";
				case MSG_SEVIRI_1_5_VIS_0_8:	return "VIS008";
				case MSG_SEVIRI_1_5_IR_1_6:		return "IR_016";
				case MSG_SEVIRI_1_5_IR_3_9:		return "IR_039";
				case MSG_SEVIRI_1_5_WV_6_2:		return "WV_062";
				case MSG_SEVIRI_1_5_WV_7_3:		return "WV_073";
				case MSG_SEVIRI_1_5_IR_8_7:		return "IR_087";
				case MSG_SEVIRI_1_5_IR_9_7:		return "IR_097";
				case MSG_SEVIRI_1_5_IR_10_8:	return "IR_108";
				case MSG_SEVIRI_1_5_IR_12_0:	return "IR_120";
				case MSG_SEVIRI_1_5_IR_13_4:	return "IR_134";
				case MSG_SEVIRI_1_5_HRV:			return "HRV";
				// SAF special cases
                case 100: return "AMA_CL";
                case 101: return "CMa";
                case 102: return "CMa_DUST";
                case 103: return "CMa_QUALITY";
                case 104: return "CMa_TEST";
                case 105: return "CMa_VOLCANIC";
                case 106: return "CRR";
                case 107: return "CRR_DATAFLAG";
                case 108: return "CRR_QUALITY";
                case 109: return "CT";
                case 110: return "CT_PHASE";
                case 111: return "CT_QUALITY";
                case 112: return "CTTH_EFFECT";
                case 113: return "CTTH_HEIGHT";
                case 114: return "CTTH_PRESS";
                case 115: return "CTTH_QUALITY";
                case 116: return "CTTH_TEMPER";
                case 117: return "LPW_BL";
                case 118: return "LPW_CLEARSKY";
                case 119: return "LPW_CLOUDY";
                case 120: return "LPW_CONS_SEA";
                case 121: return "LPW_HL";
                case 122: return "LPW_INT_TPW";
                case 123: return "LPW_ML";
                case 124: return "LPW_QUALITY";
                case 125: return "LPW_RAD_RAN";
                case 126: return "PC_PROB1";
                case 127: return "PC_PROB2";
                case 128: return "PC_QUALITY";
                case 129: return "SAI";
                case 130: return "SAI_CLEARSKY";
                case 131: return "SAI_CLOUDY";
                case 132: return "SAI_RAD_RAN";
                case 133: return "TPW";
                case 134: return "TPW_CLEARSKY";
                case 135: return "TPW_CONDIT";
                case 136: return "TPW_QUALITY";
			}
			break;
		}
	}
	stringstream str;
	str << "unknown" << spacecraftID << "_" << channelID;
	return str.str();
}

const char* Image::channelUnit(int spacecraftID, int channelID)
{
	switch (spacecraftID)
	{
		case 55:
		case 56:
		{
			switch (channelID)
			{
				case MSG_SEVIRI_NO_CHANNEL:		return "unknown";
				case MSG_SEVIRI_1_5_VIS_0_6:	return "mW m^-2 sr^-1 (cm^-1)^-1";
				case MSG_SEVIRI_1_5_VIS_0_8:	return "mW m^-2 sr^-1 (cm^-1)^-1";
				case MSG_SEVIRI_1_5_IR_1_6:		return "mW m^-2 sr^-1 (cm^-1)^-1";
				case MSG_SEVIRI_1_5_IR_3_9:		return "K";
				case MSG_SEVIRI_1_5_WV_6_2:		return "K";
				case MSG_SEVIRI_1_5_WV_7_3:		return "K";
				case MSG_SEVIRI_1_5_IR_8_7:		return "K";
				case MSG_SEVIRI_1_5_IR_9_7:		return "K";
				case MSG_SEVIRI_1_5_IR_10_8:	return "K";
				case MSG_SEVIRI_1_5_IR_12_0:	return "K";
				case MSG_SEVIRI_1_5_IR_13_4:	return "K";
				case MSG_SEVIRI_1_5_HRV:		return "mW m^-2 sr^-1 (cm^-1)^-1";
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
					return "NUMERIC";
			}
			break;
		}
	}
	return "unknown";
}

std::string Image::channelLevel(int spacecraftID, int channelID)
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

template<typename T>
static void copyData(GDALRasterBand& src, GDALRasterBand& dst, int sx0, int sy0, int ssx, int ssy, int tsx, int tsy, GDALDataType type)
{
	T* buf = new T[tsx*tsy];
	src.RasterIO(GF_Read, sx0, sy0, ssx, ssy, (void*)buf, tsx, tsy, type, 0, 0);
	dst.RasterIO(GF_Write, 0, 0, tsx, tsy, (void*)buf, tsx, tsy, type, 0, 0);
	delete[] buf;
}

void recode(
		GDALDataset& img, int ridx,
		const std::string& fileName, GDALDriver* driver, GDALDataType type,
		const std::string& opt1, const std::string& opt2, const std::string& opt3)
{
	using namespace msat;
	proj::ImageBox all(proj::ImagePoint(0, 0), proj::ImagePoint(img.GetRasterXSize(), img.GetRasterYSize()));
	return recode(img, ridx,
			all, img.GetRasterXSize(), img.GetRasterYSize(),
			fileName, driver, type,
			opt1, opt2, opt3);
}

void recode(
		GDALDataset& img, int ridx,
		const msat::proj::ImageBox& cropArea,
		const std::string& fileName, GDALDriver* driver, GDALDataType type,
		const std::string& opt1, const std::string& opt2, const std::string& opt3)
{
	int sx = cropArea.bottomRight.column - cropArea.topLeft.column;
	int sy = cropArea.bottomRight.line - cropArea.topLeft.line;
	return recode(img, ridx,
			cropArea, sx, sy,
			fileName, driver, type,
			opt1, opt2, opt3);
}

void recode(
		GDALDataset& img, int ridx,
		int sx, int sy,
		const std::string& fileName, GDALDriver* driver, GDALDataType type,
		const std::string& opt1, const std::string& opt2, const std::string& opt3)
{
	using namespace msat;
	proj::ImageBox all(proj::ImagePoint(0, 0), proj::ImagePoint(img.GetRasterXSize(), img.GetRasterYSize()));
	return recode(img, ridx,
			all, sx, sy,
			fileName, driver, type,
			opt1, opt2, opt3);
}

void recode(
		GDALDataset& img, int ridx,
		const msat::proj::ImageBox& cropArea, int sx, int sy,
		const std::string& fileName, GDALDriver* driver, GDALDataType type,
		const std::string& opt1, const std::string& opt2, const std::string& opt3)
{
	char** options = 0;
	if (!opt1.empty()) options = CSLAddString(options, opt1.c_str());
	if (!opt2.empty()) options = CSLAddString(options, opt2.c_str());
	if (!opt3.empty()) options = CSLAddString(options, opt3.c_str());

	int sx0 = cropArea.topLeft.column;
	int sy0 = cropArea.topLeft.line;
	int ssx = cropArea.bottomRight.column - cropArea.topLeft.column;
	int ssy = cropArea.bottomRight.line - cropArea.topLeft.line;
	int tsx = sx;
	int tsy = sy;

	auto_ptr<GDALDataset> ds(
			driver->Create(fileName.c_str(),
				tsx, tsy, 1, type, options));

	CSLDestroy(options);

	if (ds.get() == 0)
		throw std::runtime_error("Failed to create dataset");

	ds->SetProjection(img.GetProjectionRef());

	double gt[6];
	img.GetGeoTransform(gt);
	//fprintf(stderr, "Orig geot x0 %f y0 %f\n", gt[0] / gt[1], gt[3] / gt[5]);
	if (sx0 != 0 || sy0 != 0 || ssx != tsx || ssy != tsy)
	{
		// Adjust matrix when we crop
		double gt1 = gt[1] * (double)ssx / (double)tsx;
		double gt5 = gt[5] * (double)ssy / (double)tsy;

		gt[0] = (gt[0] / gt[1] + sx0) * gt1;
		gt[3] = (gt[3] / gt[5] + sy0) * gt5;

		gt[1] = gt1;
		gt[5] = gt5;
#if 0
		int xs, ys;
		double rx, ry;
		img.getMsatGeoTransform(xs, ys, xr, yr);
		ds->setMsatGeoTransform(xs - sx0, ys - sy0, rx * (double)ssx / (double)tsx, ry * (double)ssy / (double)tsy);
#endif
	}
	ds->SetGeoTransform(gt);


	msat::Image* simg = dynamic_cast<msat::Image*>(&img);
	msat::Image* timg = dynamic_cast<msat::Image*>(ds.get());
	if (simg && timg)
	{
		int year, month, day, hour, minute, second;
		simg->getTime(year, month, day, hour, minute, second);
		timg->setTime(year, month, day, hour, minute, second);

		timg->setSpacecraft(simg->getSpacecraft());
	}

	GDALRasterBand* srb = img.GetRasterBand(ridx);
	GDALRasterBand* trb = ds->GetRasterBand(1);

	//fprintf(stderr, "RB from %d to %d\n", srb->GetRasterDataType(), trb->GetRasterDataType());

	trb->SetOffset(srb->GetOffset());
	trb->SetScale(srb->GetScale());
	trb->SetNoDataValue(srb->GetNoDataValue());

	msat::Band* sband = dynamic_cast<msat::Band*>(srb);
	msat::Band* tband = dynamic_cast<msat::Band*>(trb);
	if (srb && trb)
	{
		tband->setChannel(sband->getChannel());
		// This does not solve problems, as xRIT declares 11 BPP but
		// that's before calibration, which can be nonlinear
		//tband->setPreferredBpp(sband->getOriginalBpp());
	}

	switch (type)
	{
		case GDT_Byte: copyData<unsigned char>(*srb, *trb, sx0, sy0, ssx, ssy, tsx, tsy, type); break;
		case GDT_UInt16: copyData<uint16_t>(*srb, *trb, sx0, sy0, ssx, ssy, tsx, tsy, type); break;
		case GDT_Int16: copyData<int16_t>(*srb, *trb, sx0, sy0, ssx, ssy, tsx, tsy, type); break;
		case GDT_UInt32: copyData<uint32_t>(*srb, *trb, sx0, sy0, ssx, ssy, tsx, tsy, type); break;
		case GDT_Int32: copyData<int32_t>(*srb, *trb, sx0, sy0, ssx, ssy, tsx, tsy, type); break;
		case GDT_Float32: copyData<float>(*srb, *trb, sx0, sy0, ssx, ssy, tsx, tsy, type); break;
		case GDT_Float64: copyData<double>(*srb, *trb, sx0, sy0, ssx, ssy, tsx, tsy, type); break;
		default:
			throw std::runtime_error("unsupported GDAL type");
	}
	/* destructor is called here, which should flush and close the file */
}

//
// ImageVector
// 

ImageVector::~ImageVector()
{
	for (iterator i = begin(); i != end(); ++i)
		delete *i;
}

//
// ImageDumper
//

class ImageDumper : public ImageConsumer
{
	bool withContents;

public:
	ImageDumper(bool withContents) : withContents(withContents) {}

	void processBand(GDALRasterBand* band)
	{
		cout << "  "
			 << cout.precision(15)
			 << band->GetXSize() << "x" << band->GetYSize()
			 <<	" *" << band->GetScale() << "+" << band->GetOffset()
			 << "  " << bppOfGDALDataType(band->GetRasterDataType()) << "bpp"
			 << " unit: " << band->GetUnitType()
			 << endl;
		cout << "Default filename: " << defaultFilename(*band) << endl;

		if (Band* b = dynamic_cast<Band*>(band))
		{
			cout << "  " << b->getName() << " "
				 << " ch.id: " << b->getChannel()
				 << " decdigits: " << b->decimalDigitsOfScaledValues()
				 << endl;
		}

		double offset = band->GetOffset();
		double scale = band->GetScale();
		if (withContents)
		{
			GdalBuffer<double> data(band->GetXSize(), band->GetYSize());
			data.read(*band);
			cout << "Coord\tUnscaled\tScaled" << endl;
			for (int l = 0; l < data.sy; ++l)
				for (int c = 0; c < data.sx; ++c)
					cout << c << "x" << l << '\t' << data.get(c, l) << '\t' << data.get(c, l) * scale + offset << endl;
		} else {
			int count = 10;
			if (band->GetYSize() < 10)
				count = band->GetYSize();
			GdalBuffer<double> data(count, 1);
			data.read(*band);
			cout << "First " << count << " data values:" << endl;
			cout << "Coord\tUnscaled\tScaled" << endl;
			for (int i = 0; i < count; ++i)
				cout << i << "x" << 0 << '\t' << data.data[i] << '\t' << data.data[i] * scale + offset << endl;
		}
	}

	virtual void processImage(GDALDataset& img)
	{
		cout << "Dataset: " << endl;
		cout << " size: " << img.GetRasterXSize() << "x" << img.GetRasterYSize()
			 << " proj: " << img.GetProjectionRef()
			 << endl;

		double geoTransform[6];
		img.GetGeoTransform(geoTransform);
		cout << " geotransform matrix: " << endl
			 << cout.precision(15)
			 << "  " << geoTransform[0] << ", " << geoTransform[1] << ", " << geoTransform[2] << "; "
			 << "  " << geoTransform[3] << ", " << geoTransform[4] << ", " << geoTransform[5]
			 << endl;

		if (char **md = img.GetMetadata())
		{
			cout << "Metadata []:" << endl;
			for (char** s = md; md && *s; ++s)
				cout << *s << endl;
		}
		if (char** md = img.GetMetadata("SUBDATASETS"))
		{
			cout << "Metadata [SUBDATASETS]:" << endl;
			for (char** s = md; md && *s; ++s)
				cout << *s << endl;
		}
		if (char** md = img.GetMetadata("IMAGE_STRUCTURE"))
		{
			cout << "Metadata [IMAGE_STRUCTURE]:" << endl;
			for (char** s = md; md && *s; ++s)
				cout << *s << endl;
		}
		if (char** md = img.GetMetadata("MSAT"))
		{
			cout << "Metadata [MSAT]:" << endl;
			for (char** s = md; md && *s; ++s)
				cout << *s << endl;
		}

		GeoReferencer georef(&img);
		double lat, lon;
		georef.pixelToLatlon(0, 0, lat, lon);
		cout << "Coordinates of the top left pixel " << lat << "," << lon << endl;
		georef.pixelToLatlon(img.GetRasterXSize(), img.GetRasterYSize(), lat, lon);
		cout << "Coordinates of the bottom right pixel " << lat << "," << lon << endl;
		cout << "Default filename: " << defaultFilename(img) << endl;

		if (Image* i = dynamic_cast<Image*>(&img))
		{
			
			cout << " "
				 << i->datetime()
				 << " sp.id: " << i->getSpacecraft()
				 << " quality " << i->quality
				 << endl;
			cout << "History: " << i->getHistory() << endl;
		}

		for (int i = 1; i <= img.GetRasterCount(); ++i)
		{
			cout << " Band " << i << "/" << img.GetRasterCount() << ": " << endl;
			processBand(img.GetRasterBand(i));
		}
	}
};

std::auto_ptr<ImageConsumer> createImageDumper(bool withContents)
{
	return std::auto_ptr<ImageConsumer>(new ImageDumper(withContents));
}

void escapeSpacesAndDots(std::string& str)
{
	for (string::iterator i = str.begin(); i != str.end(); ++i)
		if (*i == ' ' || *i == '.')
			*i = '_';
}

std::string defaultFilename(GDALDataset& img)
{
	if (img.GetRasterCount() == 1)
		return defaultFilename(*img.GetRasterBand(1));

	if (Image* i = dynamic_cast<Image*>(&img))
	{
		// Get the string describing the spacecraft
		std::string spacecraft = Image::spacecraftName(i->getSpacecraft());

		// Get the string describing the sensor
		std::string sensor = Image::sensorName(i->getSpacecraft());

		// See if we have a common level in all the raster bands
		Band* b = dynamic_cast<Band*>(i->GetRasterBand(1));
		std::string level = Image::channelLevel(i->getSpacecraft(), b->getChannel());
		for (int j = 2; j <= img.GetRasterCount(); ++j)
		{
			b = dynamic_cast<Band*>(i->GetRasterBand(j));
			string l = Image::channelLevel(i->getSpacecraft(), b->getChannel());
			if (l != level)
			{
				level.clear();
				break;
			}
		}

		escapeSpacesAndDots(spacecraft);
		escapeSpacesAndDots(sensor);

		// Format the date
		int year, month, day, hour, minute, second;
		i->getTime(year, month, day, hour, minute, second);
		char datestring[15];
		snprintf(datestring, 14, "%04d%02d%02d_%02d%02d", year, month, day, hour, minute);

		string res;
		if (i->quality != '_')
			res = string() + i->quality + "_";
		res += spacecraft + "_" + sensor + "_";
		if (!level.empty())
			res += level + "_";
		res += datestring;
		return res;
	} else {
		return "pleaseimplementme";
	}
}

std::string defaultFilename(GDALRasterBand& band)
{
	if (Band* b = dynamic_cast<Band*>(&band))
	{
		Image& img = b->img();
		// Get the string describing the spacecraft
		std::string spacecraft = Image::spacecraftName(img.getSpacecraft());

		// Get the string describing the sensor
		std::string sensor = Image::sensorName(img.getSpacecraft());

		// Get the string describing the channel
		std::string channel = Image::channelName(img.getSpacecraft(), b->getChannel());

		// Get the string with the channel data level
		std::string level = Image::channelLevel(img.getSpacecraft(), b->getChannel());

		escapeSpacesAndDots(spacecraft);
		escapeSpacesAndDots(sensor);
		escapeSpacesAndDots(channel);
		escapeSpacesAndDots(level);

		// Format the date
		int year, month, day, hour, minute, second;
		img.getTime(year, month, day, hour, minute, second);
		char datestring[15];
		snprintf(datestring, 14, "%04d%02d%02d_%02d%02d", year, month, day, hour, minute);

		string res;
		if (img.quality != '_')
			res = string() + img.quality + "_";
		res += spacecraft + "_" + sensor + "_";
		if (!level.empty())
			res += level + "_";
		res += channel + "_" + datestring;
		return res;
	} else {
		return "pleaseimplementme";
	}
}

std::string defaultShortName(GDALDataset& img)
{
	if (img.GetRasterCount() == 1)
		return defaultShortName(*img.GetRasterBand(1));

	if (Image* i = dynamic_cast<Image*>(&img))
	{
		return "";
	} else {
		return "pleaseimplementme";
	}
}

std::string defaultShortName(GDALRasterBand& band)
{
	if (Band* b = dynamic_cast<Band*>(&band))
	{
		Image& img = b->img();
		string channelstring = Image::channelName(img.getSpacecraft(), b->getChannel());
		escapeSpacesAndDots(channelstring);
		return channelstring;
	} else {
		return "pleaseimplementme";
	}
}

int getBPP(GDALRasterBand& rb)
{
	if (Band* b = dynamic_cast<Band*>(&rb))
	{
		return b->getOriginalBpp();
	} else {
		return bppOfGDALDataType(rb.GetRasterDataType());
	}
}

time_t forecastSeconds2000(int year, int month, int day, int hour, int minute, int second)
{
	const time_t s_epoch_2000 = 946684800;
	struct tm itm;

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

void ImageTransformation::updateImgArea(GeoReferencer& georef, const proj::MapBox& cropGeoArea)
{
	using namespace proj;

	if (!cropGeoArea.isNonZero()) return;

	// Remap geoArea into pixels
	double latmin, latmax, lonmin, lonmax;
	cropGeoArea.boundingBox(latmin, latmax, lonmin, lonmax);

	// Convert to pixel coordinates
	//double px, py;
	int x, y, x1, y1;

	//cerr << "LT " << latmin << ", " << lonmin << endl;
	//georef.latlonToProjected(latmin, lonmin, px, py);
	//cerr << "LTProj " << latmin << ", " << lonmin << " -> " << px << ", " << py << endl;
	//georef.projectedToPixel(px, py, x, y);
	//cerr << "LTPix " << px << ", " << py << " -> " << x << ", " << y << endl;
	georef.latlonToPixel(latmin, lonmin, x, y);
	//cerr << "LTP " << latmax << ", " << lonmax << " -> " << x1 << ", " << y1 << endl;
	georef.latlonToPixel(latmax, lonmax, x1, y1);
	//cerr << "LTP " << latmax << ", " << lonmax << " -> " << x1 << ", " << y1 << endl;

	if (x < 0) x = 0;
	if (y < 0) y = 0;
	if (x1 < 0) x1 = 0;
	if (y1 < 0) y1 = 0;

	// Crop using the bounding box for the 2 coordinates
	size_t imgx = (unsigned)(x < x1 ? x : x1);
	size_t imgy = (unsigned)(y < y1 ? y : y1);
	size_t imgw = (unsigned)(x > x1 ? x-x1 : x1-x);
	size_t imgh = (unsigned)(y > y1 ? y-y1 : y1-y);

	// Recreate the area
	cropImgArea = ImageBox(ImagePoint(imgx, imgy), ImagePoint(imgx+imgw, imgy+imgh));

	//cerr << "geo-crop area: (" << imgx << ", " << imgy << ") + (" << imgw << ", " << imgh << ")" << endl;
}

}

// vim:set ts=2 sw=2:
