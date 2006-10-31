#include "Image.h"

#include <sstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <math.h>
#include <limits.h>
#include <proj/const.h>

// For HRIT satellite IDs
#include <hrit/MSG_spacecraft.h>

using namespace std;

static struct ChannelInfo {
	size_t decimalDigits;
} channelInfo[] = {
	{ 2 },	//  0		FIXME: unverified
	{ 2 },	//  1		FIXME: unverified
	{ 2 },	//  2		FIXME: unverified
	{ 2 },	//  3		FIXME: unverified
	{ 2 },	//  4		FIXME: unverified
	{ 2 },	//  5		FIXME: unverified
	{ 2 },	//  6		FIXME: unverified
	{ 2 },	//  7		FIXME: unverified
	{ 2 },	//  8		FIXME: unverified
	{ 2 },	//  9		FIXME: unverified
	{ 2 },	// 10		FIXME: unverified
	{ 2 },	// 11		FIXME: unverified
	{ 2 },	// 12		FIXME: unverified
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

void Image::crop(int x, int y, int width, int height)
{
	data->crop(x, y, width, height);
	column_offset += x;
	line_offset += y;
}

void Image::coordsToPixels(double lat, double lon, int& x, int& y) const
{
  double column, line;

	proj::ProjectedPoint p;
	proj->mapToProjected(proj::MapPoint(lat, lon), p);

  x = rint((double) column_offset + p.x * column_factor / (1 << 16) - 1.0);
  y = rint((double) line_offset   + p.y * line_factor   / (1 << 16) - 1.0);
}

double Image::pixelHSize() const
{
	// This computation has been found by Dr2 Francesca Di Giuseppe
	return (ORBIT_RADIUS - EARTH_RADIUS) * tan( (1.0/column_factor/exp2(-16)) * M_PI / 180 );
}

double Image::pixelVSize() const
{
	// This computation has been found by Dr2 Francesca Di Giuseppe
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
	time_t ttm;

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
	if (data->scalesToInt)
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
		// When the original value was a float value, we can read the number of
		// significant digits from a table indexed by channels

		if (channel_id >= channel_info_size)
		{
			stringstream str;
			str << "no information found for channel " << channel_id;
			throw std::runtime_error(str.str());
		}
		return channelInfo[channel_id].decimalDigits;
	}
}

//
// ImageData
//

ImageData::ImageData() : columns(0), lines(0), slope(1), offset(0), bpp(0),
	              scalesToInt(false), missingValue(FP_NAN) {}

float* ImageData::allScaled() const
{
	float* res = new float[lines * columns];
	for (int y = 0; y < lines; ++y)
		for (int x = 0; x < columns; ++x)
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
				 << " offset: " << img->column_offset << "x" << img->line_offset << endl;

		cout << " Images: " << endl;

		cout << "  " //<< *i
				 << "\t" << img->data->columns << "x" << img->data->lines << " " << img->data->bpp << "bpp"
						" *" << img->data->slope << "+" << img->data->offset << " decdigits: " << img->decimalDigitsOfScaledValues()
				 << " PSIZE " << img->pixelHSize() << "x" << img->pixelVSize()
				 << " DX " << Image::seviriDXFromColumnFactor(img->column_factor)
				 << " DY " << Image::seviriDYFromLineFactor(img->line_factor)
				 << " CHID " << img->channel_id
				 << endl;

		if (withContents)
		{
			cout << "Coord\tUnscaled\tScaled" << endl;
			for (int l = 0; l < img->data->lines; ++l)
				for (int c = 0; c < img->data->lines; ++c)
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
