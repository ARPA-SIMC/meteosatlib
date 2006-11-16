#include "Image.h"

#include <sstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <limits>
#include <math.h>
#include <limits.h>
#include <proj/const.h>

// For HRIT satellite IDs
#include <hrit/MSG_spacecraft.h>
// For HRIT channel names
#include <hrit/MSG_channel.h>

using namespace std;

static struct ChannelInfo {
	size_t decimalDigits;
} channelInfo[] = {
	{ 2 },	//  0		FIXME: unverified
	{ 4 },	//  1
	{ 4 },	//  2
	{ 4 },	//  3
	{ 2 },	//  4
	{ 2 },	//  5
	{ 2 },	//  6
	{ 2 },	//  7
	{ 2 },	//  8
	{ 2 },	//  9
	{ 2 },	// 10
	{ 2 },	// 11
	{ 4 },	// 12
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

void Image::crop(size_t x, size_t y, size_t width, size_t height)
{
	data->crop(x, y, width, height);
	x0 += x;
	y0 += y;
#if 0
	Column offset and line offset should not be affected by cropping
	column_offset -= x;
	line_offset -= y;
#endif

	std::stringstream str;
	str << "Cropped subarea " << x << "," << y << " " << width << "x" << height;
	addToHistory(str.str());
}

void Image::cropByCoords(double latmin, double latmax, double lonmin, double lonmax)
{
	size_t x, y, x1, y1;

	// Convert to pixel coordinates
	coordsToPixels(latmin, lonmin, x, y);
	coordsToPixels(latmax, lonmax, x1, y1);

	// Crop using the bounding box for the 2 coordinates
	size_t crop_x = x = x < x1 ? x : x1;
	size_t crop_y = y = y < y1 ? y : y1;
	size_t crop_w = x > x1 ? x-x1 : x1-x;
	size_t crop_h = y > y1 ? y-y1 : y1-y;

	data->crop(crop_x, crop_y, crop_w, crop_h);
	x0 += crop_x;
	y0 += crop_y;

	std::stringstream str;
	str << "Cropped subarea lat " << latmin << "-" << latmax << " lon " << lonmin << "-" << lonmax;
	addToHistory(str.str());
}

void Image::coordsToPixels(double lat, double lon, size_t& x, size_t& y) const
{
	proj::ProjectedPoint p;
	proj->mapToProjected(proj::MapPoint(lat, lon), p);

	//cerr << "  toProjected: " << p.x << "," << p.y << endl;

	int dx = (int)rint((double) column_offset + p.x * column_factor * exp2(-16)) - x0;
	int dy = (int)rint((double) line_offset   + p.y * line_factor   * exp2(-16)) - y0;

  x = dx < 0 ? 0 : (unsigned)dx;
  y = dy < 0 ? 0 : (unsigned)dy;
}

double Image::pixelHSize() const
{
	return (ORBIT_RADIUS - EARTH_RADIUS) * tan( (1.0/column_factor/exp2(-16)) * M_PI / 180 );
}

double Image::pixelVSize() const
{
	return (ORBIT_RADIUS - EARTH_RADIUS) * tan( (1.0/line_factor/exp2(-16)) * M_PI / 180 );
}

int Image::seviriDXFromColumnFactor(int column_factor)
{
#if 0
	double ps = (ORBIT_RADIUS - EARTH_RADIUS) * tan( (1.0/column_factor/exp2(-16)) * PI / 180 );
	return round((2 * asin(EARTH_RADIUS / ORBIT_RADIUS)) / atan(ps / (ORBIT_RADIUS-EARTH_RADIUS)));
#endif

	// This computation has been found by Dr2 Francesca Di Giuseppe and simplified by Enrico Zini
	return (int)round(asin(EARTH_RADIUS / ORBIT_RADIUS) * column_factor * exp2(-15) * 180 / M_PI);
}

int Image::seviriDYFromLineFactor(int line_factor)
{
	// This computation has been found by Dr2 Francesca Di Giuseppe and simplified by Enrico Zini
	return (int)round(asin(EARTH_RADIUS / ORBIT_RADIUS) * line_factor * exp2(-15) * 180 / M_PI);
	//return round((2 * asin(EARTH_RADIUS / ORBIT_RADIUS)) / atan(pixelVSize() / (ORBIT_RADIUS-EARTH_RADIUS)));
}

int Image::columnFactorFromSeviriDX(int seviriDX)
{
	return (int)round((double)seviriDX * M_PI / (asin(EARTH_RADIUS / ORBIT_RADIUS)*exp2(-15)*180));
}

int Image::lineFactorFromSeviriDY(int seviriDY)
{
	return (int)round((double)seviriDY * M_PI / (asin(EARTH_RADIUS / ORBIT_RADIUS)*exp2(-15)*180));
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

// Replace spaces and dots with underscores
static void escape(std::string& str)
{
	for (string::iterator i = str.begin(); i != str.end(); ++i)
		if (*i == ' ' || *i == '.')
			*i = '_';
}

std::string Image::defaultFilename() const
{
	t_enum_MSG_spacecraft sc = (t_enum_MSG_spacecraft)spacecraftIDToHRIT(spacecraft_id);
	// Get the string describing the spacecraft
	std::string spacecraft = MSG_spacecraft_name(sc);

	// Get the string describing the sensor
	std::string sensor;

	// Get the string describing the channel
	std::string channel;
	if (sc == MSG_MSG_1)
	{
		sensor = "Seviri";
		switch (channel_id)
		{
			case MSG_SEVIRI_NO_CHANNEL:		channel = "no-channel"; break;
			case MSG_SEVIRI_1_5_VIS_0_6:	channel = "VIS006"; break;
			case MSG_SEVIRI_1_5_VIS_0_8:	channel = "VIS008"; break;
			case MSG_SEVIRI_1_5_IR_1_6:		channel = "IR_016"; break;
			case MSG_SEVIRI_1_5_IR_3_9:		channel = "IR_039"; break;
			case MSG_SEVIRI_1_5_WV_6_2:		channel = "WV_062"; break;
			case MSG_SEVIRI_1_5_WV_7_3:		channel = "WV_073"; break;
			case MSG_SEVIRI_1_5_IR_8_7:		channel = "IR_087"; break;
			case MSG_SEVIRI_1_5_IR_9_7:		channel = "IR_097"; break;
			case MSG_SEVIRI_1_5_IR_10_8:	channel = "IR_108"; break;
			case MSG_SEVIRI_1_5_IR_12_0:	channel = "IR_120"; break;
			case MSG_SEVIRI_1_5_IR_13_4:	channel = "IR_134"; break;
			case MSG_SEVIRI_1_5_HRV:			channel = "HRV"; break;
			default: channel = "unknown"; break;
		}
	}
	else
	{
		sensor = "unknown";
		channel = "unknown";
	}

	escape(spacecraft);
	escape(sensor);
	escape(channel);

	// Format the date
	char datestring[15];
	snprintf(datestring, 14, "%04d%02d%02d_%02d%02d", year, month, day, hour, minute);

	if (quality == '_')
		return spacecraft + "_" + sensor + "_" + channel + "_channel_" + datestring;
	else
		return string() + quality + "_" + spacecraft + "_" + sensor + "_" + channel + "_channel_" + datestring;
}

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
		cout << " size: " << img->data->columns << "x" << img->data->lines << " factor: " << img->column_factor << "x" << img->line_factor
				 << " origin: " << img->x0 << "," << img->y0 << " centre: " << img->column_offset << "," << img->line_offset
				 << endl;

		cout << " Images: " << endl;

		cout << "  " //<< *i
				 << "\t" << img->data->columns << "x" << img->data->lines << " " << img->data->bpp << "bpp"
						" *" << img->data->slope << "+" << img->data->offset << " decdigits: " << img->decimalDigitsOfScaledValues()
				 << " PSIZE " << img->pixelHSize() << "x" << img->pixelVSize()
				 << " DX " << Image::seviriDXFromColumnFactor(img->column_factor)
				 << " DY " << Image::seviriDYFromLineFactor(img->line_factor)
				 << " CHID " << img->channel_id
				 << " Quality " << img->quality
				 << endl;

		if (withContents)
		{
			cout << "Coord\tUnscaled\tScaled" << endl;
			for (size_t l = 0; l < img->data->lines; ++l)
				for (size_t c = 0; c < img->data->lines; ++c)
					cout << c << "x" << l << '\t' << '\t' << img->data->scaled(c, l) << endl;
		}
  }
};

std::auto_ptr<ImageConsumer> createImageDumper(bool withContents)
{
	return std::auto_ptr<ImageConsumer>(new ImageDumper(withContents));
}

}

// vim:set ts=2 sw=2:
