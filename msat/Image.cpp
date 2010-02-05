#include "Image.h"

#include <gdal/ogr_spatialref.h>

#include <sstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <limits>
#include <math.h>
#include <limits.h>
#include <cstdlib>
#include "proj/const.h"

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

Image::~Image() { if (data) delete data; }

void Image::setData(ImageData* data)
{
	if (this->data) delete this->data;
	this->data = data;
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

void Image::addToHistory(const std::string& event)
{
	if (history.empty())
		history = event;
	else
		history += ", " + event;
}

std::string Image::historyPlusEvent(const std::string& event) const
{
	if (history.empty())
		return event;
	else
		return history + ", " + event;
}

void Image::crop(const proj::ImageBox& area)
{
	int x, y, width, height;
	area.boundingBox(x, y, width, height);
	data->crop(x, y, width, height);
	x0 += x;
	y0 += y;
#if 0
	Column offset and line offset should not be affected by cropping
	column_offset -= x;
	line_offset -= y;
#endif

	std::stringstream str;
//	str << "Cropped subarea " << x << "," << y << " " << width << "x" << height;
	addToHistory(str.str());
}

void Image::cropByCoords(const proj::MapBox& area)
{
	double latmin, latmax, lonmin, lonmax;
	area.boundingBox(latmin, latmax, lonmin, lonmax);

	// Convert to pixel coordinates
	int x, y, x1, y1;
	coordsToPixels(latmin, lonmin, x, y);
	coordsToPixels(latmax, lonmax, x1, y1);

	if (x < 0) x = 0;
	if (y < 0) y = 0;
	if (x1 < 0) x1 = 0;
	if (y1 < 0) y1 = 0;

	// Crop using the bounding box for the 2 coordinates
	size_t crop_x = (unsigned)(x < x1 ? x : x1);
	size_t crop_y = (unsigned)(y < y1 ? y : y1);
	size_t crop_w = (unsigned)(x > x1 ? x-x1 : x1-x);
	size_t crop_h = (unsigned)(y > y1 ? y-y1 : y1-y);

//	cerr << "CROP lat " << latmin << "-" << latmax << " lon " << lonmin << "-" << lonmax << endl;
//	cerr << "  converted to " << x << "," << y << " " << x1 << "," << y1 << endl;
//	cerr << "  bounding box " << crop_x << "," << crop_y << " " << crop_w << "x" << crop_h << endl;

	data->crop(crop_x, crop_y, crop_w, crop_h);
	x0 += crop_x;
	y0 += crop_y;

	std::stringstream str;
	str << "Cropped subarea lat " << latmin << "-" << latmax << " lon " << lonmin << "-" << lonmax;
	addToHistory(str.str());
}

void Image::coordsToPixels(double lat, double lon, int& x, int& y) const
{
	proj::ProjectedPoint p;
	proj->mapToProjected(proj::MapPoint(lat, lon), p);

	//cerr << "  coordsToPixels: " << lat << "," << lon << " -> " << p.x << "," << p.y << endl;

	x = (int)rint((double)p.x * column_res) + (column_offset - x0);
	y = (int)rint((double)p.y * line_res) + (line_offset   - y0);

	//cerr << "    to pixels: " << x << "," << y << endl;
}

void Image::pixelsToCoords(int x, int y, double& lat, double& lon) const
{
	proj::ProjectedPoint pp;
	pp.x = (double)(x - (column_offset - x0)) / column_res;
	pp.y = (double)(y - (line_offset   - y0)) / line_res;
	//cerr << "PTC " << x << "," << y << " -> " << pp.x << "," << pp.y << endl;
	proj::MapPoint p;
	proj->projectedToMap(pp, p);
	lat = p.lat;
	lon = p.lon;
	//cerr << "PTC " << pp.x << "," << pp.y << " -> " << lat << "," << lon << endl;
}

double Image::pixelHSize() const
{
	return (ORBIT_RADIUS - EARTH_RADIUS) * tan( (1.0/column_res) * M_PI / 180 );
}

double Image::pixelVSize() const
{
	return (ORBIT_RADIUS - EARTH_RADIUS) * tan( (1.0/line_res) * M_PI / 180 );
}

// THIS is the way to get to ColumnDirGridStep and LineDirGridStep
double Image::pixelHSizeFromCFAC(double cfac)
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
double Image::pixelVSizeFromLFAC(double lfac)
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

double Image::CFACFromPixelHSize(double psx)
{
	// Handle known values
	//fprintf(stderr, "Band::CFACFromPixelHSize cheat %f\n", psx - METEOSAT_PIXELSIZE_X);
	if (fabs(psx - METEOSAT_PIXELSIZE_X) < 0.001) return 13642337*exp2(-16);
	if (fabs(psx - -METEOSAT_PIXELSIZE_X) < 0.001) return -13642337*exp2(-16);
	if (fabs(psx - METEOSAT_PIXELSIZE_X_HRV) < 0.001) return 40927000*exp2(-16);
	if (fabs(psx - -METEOSAT_PIXELSIZE_X_HRV) < 0.001) return -40927000*exp2(-16);
	return M_PI / atan(psx / ((ORBIT_RADIUS - EARTH_RADIUS) * 1000.0)) / 180.0;
}

double Image::LFACFromPixelVSize(double psy)
{
	if (fabs(psy - METEOSAT_PIXELSIZE_Y) < 0.001) return 13642337*exp2(-16);
	if (fabs(psy - -METEOSAT_PIXELSIZE_Y) < 0.001) return -13642337*exp2(-16);
	if (fabs(psy - METEOSAT_PIXELSIZE_Y_HRV) < 0.001) return 40927000*exp2(-16);
	if (fabs(psy - -METEOSAT_PIXELSIZE_Y_HRV) < 0.001) return -40927000*exp2(-16);
	return M_PI / atan(psy / ((ORBIT_RADIUS - EARTH_RADIUS) * 1000.0)) / 180.0;
}

int Image::seviriDXFromColumnRes(double column_res)
{
#if 0
	double ps = (ORBIT_RADIUS - EARTH_RADIUS) * tan( (1.0/column_factor/exp2(-16)) * PI / 180 );
	return round((2 * asin(EARTH_RADIUS / ORBIT_RADIUS)) / atan(ps / (ORBIT_RADIUS-EARTH_RADIUS)));
#endif

	// This computation has been found by Dr2 Francesca Di Giuseppe and simplified by Enrico Zini
	return (int)round(asin(EARTH_RADIUS / ORBIT_RADIUS) * column_res * 360 / M_PI);
}

int Image::seviriDYFromLineRes(double line_res)
{
	// This computation has been found by Dr2 Francesca Di Giuseppe and simplified by Enrico Zini
	return (int)round(asin(EARTH_RADIUS / ORBIT_RADIUS) * line_res * 360 / M_PI);
	//return round((2 * asin(EARTH_RADIUS / ORBIT_RADIUS)) / atan(pixelVSize() / (ORBIT_RADIUS-EARTH_RADIUS)));
}

double Image::columnResFromSeviriDX(int seviriDX)
{
	return (double)seviriDX * M_PI / (asin(EARTH_RADIUS / ORBIT_RADIUS)*360);
}

double Image::lineResFromSeviriDY(int seviriDY)
{
	return (double)seviriDY * M_PI / (asin(EARTH_RADIUS / ORBIT_RADIUS)*360);
}

int Image::seviriDXFromCFAC(double column_res)
{
#if 0
	double ps = (ORBIT_RADIUS - EARTH_RADIUS) * tan( (1.0/column_factor/exp2(-16)) * PI / 180 );
	return round((2 * asin(EARTH_RADIUS / ORBIT_RADIUS)) / atan(ps / (ORBIT_RADIUS-EARTH_RADIUS)));
#endif

	// This computation has been found by Dr2 Francesca Di Giuseppe and simplified by Enrico Zini
	return (int)round(asin(EARTH_RADIUS / ORBIT_RADIUS) * column_res * 360 / M_PI);
}

int Image::seviriDYFromLFAC(double line_res)
{
	// This computation has been found by Dr2 Francesca Di Giuseppe and simplified by Enrico Zini
	return (int)round(asin(EARTH_RADIUS / ORBIT_RADIUS) * line_res * 360 / M_PI);
	//return round((2 * asin(EARTH_RADIUS / ORBIT_RADIUS)) / atan(pixelVSize() / (ORBIT_RADIUS-EARTH_RADIUS)));
}

double Image::CFACFromSeviriDX(int seviriDX)
{
	return (double)seviriDX * M_PI / (asin(EARTH_RADIUS / ORBIT_RADIUS)*360);
}

double Image::LFACFromSeviriDY(int seviriDY)
{
	return (double)seviriDY * M_PI / (asin(EARTH_RADIUS / ORBIT_RADIUS)*360);
}

int Image::seviriDXFromPixelHSize(double psx)
{
	// Handle well-known values
	if (fabs(psx - METEOSAT_PIXELSIZE_X) < 0.001)
		return 3622;
	else
		return seviriDXFromCFAC(CFACFromPixelHSize(psx));
}

int Image::seviriDYFromPixelVSize(double psy)
{
	// Handle well-known values
	if (fabs(psy - METEOSAT_PIXELSIZE_Y) < 0.001)
		return 3622;
	else
		return seviriDYFromLFAC(LFACFromPixelVSize(psy));
}

double Image::pixelHSizeFromSeviriDX(int dx)
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

double Image::pixelVSizeFromSeviriDY(int dy)
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

std::string Image::datetime() const
{
	stringstream res;
	res << setfill('0') << setw(4) << year << "-" << setw(2) << month << "-" << setw(2) << day
			<< " " << setw(2) << hour << ":" << setw(2) << minute;
	return res.str();
}

time_t Image::forecastSeconds2000() const
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

int Image::decimalDigitsOfScaledValues() const
{
	if (channel_id >= 0 && (size_t)channel_id < channel_info_size)
		// When it's a known channel, se can use our internal table
		return channelInfo[channel_id].decimalDigits;
	else if (data->scalesToInt)
	{
		// When the original value was an integer value, we can compute the log10
		// of the scaling factor, add 1 if the scaling factor is not a direct power
		// of 10 and use the result as the count of decimal digits

		int res = -(int)round(log10(data->slope));
		if (exp10(-res) == data->slope)
			return res;
		else
			return res + 1;
	} else {
		stringstream str;
		str << "no information found for channel " << channel_id;
		throw std::runtime_error(str.str());
	}
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
				case 106:
				case 546:											return "CRR";
			}
			break;
		}
	}
	stringstream str;
	str << "unknown" << spacecraftID << "_" << channelID;
	return str.str();
}

std::string Image::channelUnit(int spacecraftID, int channelID)
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
				case MSG_SEVIRI_1_5_HRV:			return "mW m^-2 sr^-1 (cm^-1)^-1";
				// SAF special cases
				case 106:
				case 546:											return "NUMERIC";
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

std::string Image::spaceviewWKT(double sublon)
{
	// Also add GDAL projection (see the msg driver they have)
	// Taken from GDAL's msgdataset
	OGRSpatialReference osr;
	osr.SetGEOS(sublon, ORBIT_RADIUS_FOR_GDAL, 0, 0);
	//osr.SetWellKnownGeogCS("WGS84"); // Temporary line to satisfy ERDAS (otherwise the ellips is "unnamed"). Eventually this should become the custom a and b ellips (CGMS).

	// The following are 3 different try-outs for also setting the ellips a and b parameters.
	// We leave them out for now however because this does not work. In gdalwarp, when choosing some
	// specific target SRS, the result is an error message:
	// 
	// ERROR 1: geocentric transformation missing z or ellps
	// ERROR 1: GDALWarperOperation::ComputeSourceWindow() failed because
	// the pfnTransformer failed.
	// 
	// I can't explain the reason for the message at this time (could be a problem in the way the SRS is set here,
	// but also a bug in Proj.4 or GDAL.
	osr.SetGeogCS( NULL, NULL, NULL, 6378169, 295.488065897, NULL, 0, NULL, 0 );

	/*
	   oSRS.SetGeogCS( "unnamed ellipse", "unknown", "unnamed", 6378169, 295.488065897, "Greenwich", 0.0);

	   if( oSRS.importFromProj4("+proj=geos +h=35785831 +a=6378169 +b=6356583.8") == OGRERR_NONE )
	   {
	   oSRS.exportToWkt( &(poDS->pszProjection) );
	   }
	   */

	char* projstring;
	osr.exportToWkt(&projstring);
	string res = projstring;
	OGRFree(projstring);
	return res;
}

double Image::defaultPackedMissing(int channel_id)
{
	if (channel_id >= 0 && (size_t)channel_id < channel_info_size)
		// When it's a known channel, se can use our internal table
		return channelInfo[channel_id].packedMissing;
	else
		// Otherwise, default on something 
		return std::numeric_limits<double>::quiet_NaN();
}

double Image::defaultScaledMissing(int channel_id)
{
	if (channel_id >= 0 && (size_t)channel_id < channel_info_size)
		// When it's a known channel, se can use our internal table
		return channelInfo[channel_id].scaledMissing;
	else
		// Otherwise, default on something 
		return std::numeric_limits<double>::quiet_NaN();
}

std::auto_ptr<Image> Image::rescaled(size_t width, size_t height) const
{
	std::auto_ptr<Image> res(new Image());

	res->year = year;
	res->month = month;
	res->day = day;
	res->hour = hour;
	res->minute = minute;
	res->proj.reset(proj->clone());
	res->channel_id = channel_id;
	res->spacecraft_id = spacecraft_id;

	// Compute output pixel space
	res->column_res = column_res * width / data->columns;
	res->line_res   = line_res * height / data->lines;
	res->x0 = x0 * width / data->columns;
	res->y0 = y0 * height / data->lines;
	res->column_offset = column_offset * width / data->columns;
	res->line_offset = line_offset * height / data->lines;

	res->quality = quality;
	res->history = history;
	res->defaultFilename = defaultFilename;
	res->shortName = shortName;
	res->unit = unit;
	std::stringstream str;
	str << "scaled to " << width << "x" << height;
	res->addToHistory(str.str());

	res->data = data->createResampled(width, height);

	return res;
}

#ifdef EXPERIMENTAL_REPROJECTION
struct ReprojectMapper : public Image::PixelMapper
{
	const Image& src;
	const Image& dst;
	ReprojectMapper(const Image& src, const Image& dst) : src(src), dst(dst) {}
	virtual ~ReprojectMapper() {}
	virtual void operator()(size_t x, size_t y, int& nx, int& ny) const
	{
		double lat, lon;
		dst.pixelsToCoords(x, y, lat, lon);
		src.coordsToPixels(lat, lon, nx, ny);
		// cerr << "  ptc map " << x << "," << y << " -> " << lat << "," << lon << " -> " << nx << "," << ny << endl;
	}
};

std::auto_ptr<Image> Image::reproject(size_t width, size_t height, std::auto_ptr<proj::Projection> proj, const proj::MapBox& box) const
{
	std::auto_ptr<Image> res(new Image());

	res->year = year;
	res->month = month;
	res->day = day;
	res->hour = hour;
	res->minute = minute;
	res->proj = proj;
	res->channel_id = channel_id;
	res->spacecraft_id = spacecraft_id;

	/* From http://lists.maptools.org/pipermail/gdal-dev/2006-June/009157.html
	 *  1) Read in the image into a memory buffer (input space)
	 *  2) Get the current map projection information (Projection, datum, Upper left corner projection parameters, size of pixel in X and Y)
	 *  3) Frame your output space
	 *     - Calculate the output space upper left and lower right values
	 *        - Lat, Lon, projection x/y, output space pixel size
	 *  4) Create a memory buffer for the output space
	 *  5) For each pixel in the output space
	 *  6) Find the projection x/y using the pixel size and offset of the pixel from the upper left corner
	 *  7) Convert output space projection x/y to lat, lon using GCTP (inverse projection)
	 *  8) Convert lat, lon to input space projection x/y using forward projection routine in GCTP
	 *  9) Convert projection x/y into line, sample (row, column)
	 * 10) Resample DN values in input space near the calculated input space line, sample
	 * 11) Place resampled DN value into the output space at current pixel location (line, sample)
	 * 12) Get next output pixel location (back to step 5)
	 * 13) Write the output space buffer to a file
	 */

	// Compute projected bounding box
	proj::ProjectedBox pbox;
	res->proj->mapToProjected(box, pbox);
	double px0, py0, pw, ph;
	pbox.boundingBox(px0, py0, pw, ph);

	// Compute output pixel space
	res->column_res = width / pw;
	res->line_res   = height / ph;
	res->x0 = (int)rint(px0 * res->column_res);
	res->y0 = (int)rint(py0 * res->line_res);
	res->column_offset = 0;
	res->line_offset = 0;
	if (res->x0 < 0) { res->column_offset = -res->x0; res->x0 = 0; }
	if (res->y0 < 0) { res->line_offset = -res->y0; res->y0 = 0; }

	res->quality = quality;
	res->history = history;
	res->defaultFilename = defaultFilename;
	res->shortName = shortName;
	res->unit = unit;
	res->addToHistory("reprojected to " + res->proj->format());

	res->data = data->createReprojected(width, height, ReprojectMapper(*this, *res));

	return res;
}
#endif

//
// ImageData
//

ImageData::ImageData() : columns(0), lines(0), slope(1), offset(0), bpp(0),
	              scalesToInt(false), missingValue(std::numeric_limits<float>::max()) {}

float* ImageData::allScaled() const
{
	float* res = new float[lines * columns];
	for (size_t y = 0; y < lines; ++y)
		for (size_t x = 0; x < columns; ++x)
			res[y * columns + x] = scaled(x, y);
	return res;
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

	virtual void processImage(std::auto_ptr<Image> img)
	{
		cout << "Image " << img->datetime() << endl;
		cout << " proj: " << img->proj->format() << " ch.id: " << img->channel_id << " sp.id: " << img->spacecraft_id << endl;
		cout << " size: " << img->data->columns << "x" << img->data->lines << " resolution: " << img->column_res << "x" << img->line_res
				 << " x0: " << img->x0 << ", y0: " << img->y0 << " COFF: " << img->column_offset << ", LOFF: " << img->line_offset
				 << endl;

		cout << " Images: " << endl;

		cout << "  " //<< *i
				 << "\t" << img->data->columns << "x" << img->data->lines << " " << img->data->bpp << "bpp"
						" *" << img->data->slope << "+" << img->data->offset << " decdigits: " << img->decimalDigitsOfScaledValues()
				 << " PSIZE " << img->pixelHSize() << "x" << img->pixelVSize()
				 << " DX " << img->seviriDX()
				 << " DY " << img->seviriDY()
				 << " CHID " << img->channel_id
				 << " Quality " << img->quality
				 << endl;

		cout << "History: " << img->history << endl;
		cout << "Short name: " << img->shortName << " unit: " << img->unit << " default filename: " << img->defaultFilename << endl;

		double lat, lon;
		img->pixelsToCoords(0, 0, lat, lon);
		cout << "Coordinates of the top left pixel " << lat << "," << lon << endl;
		img->pixelsToCoords(img->data->columns, img->data->lines, lat, lon);
		cout << "Coordinates of the bottom right pixel " << lat << "," << lon << endl;

		if (withContents)
		{
			cout << "Coord\tUnscaled\tScaled" << endl;
			for (size_t l = 0; l < img->data->lines; ++l)
				for (size_t c = 0; c < img->data->columns; ++c)
				{
					cout << c << "x" << l << '\t';
					if (img->data->scalesToInt)
						cout << img->data->scaledToInt(c, l);
					else
						cout << "--";
					cout << '\t' << img->data->scaled(c, l) << endl;
				}
		} else {
			cout << "First 10 data values:" << endl;
			cout << "Coord\tUnscaled\tScaled" << endl;
			int todo = 10;
			for (size_t l = 0; todo && l < img->data->lines; ++l)
				for (size_t c = 0; todo && c < img->data->columns; ++c, --todo)
				{
					cout << c << "x" << l << '\t';
					if (img->data->scalesToInt)
						cout << img->data->scaledToInt(c, l);
					else
						cout << "--";
					cout << '\t' << img->data->scaled(c, l) << endl;
				}

		}
  }
};

std::auto_ptr<ImageConsumer> createImageDumper(bool withContents)
{
	return std::auto_ptr<ImageConsumer>(new ImageDumper(withContents));
}

}

// vim:set ts=2 sw=2:
