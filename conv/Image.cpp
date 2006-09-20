#include "Image.h"

#include <sstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <math.h>
#include "parameters.h"

using namespace std;

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


float Image::pixelHSize() const
{
	// This computation has been found by Dr2 Francesca Di Giuseppe
	return (ORBIT_RADIUS - EARTH_RADIUS) * tan( (1.0/column_factor/exp2(-16)) * PI / 180 );
}

float Image::pixelVSize() const
{
	// This computation has been found by Dr2 Francesca Di Giuseppe
	return (ORBIT_RADIUS - EARTH_RADIUS) * tan( (1.0/line_factor/exp2(-16)) * PI / 180 );
}

int Image::seviriDXFromColumnFactor(int column_factor)
{
#if 0
	double ps = (ORBIT_RADIUS - EARTH_RADIUS) * tan( (1.0/column_factor/exp2(-16)) * PI / 180 );
	return round((2 * asin(EARTH_RADIUS / ORBIT_RADIUS)) / atan(ps / (ORBIT_RADIUS-EARTH_RADIUS)));
#endif

	// This computation has been found by Dr2 Francesca Di Giuseppe and simplified by Enrico Zini
	return (int)round(asin(EARTH_RADIUS / ORBIT_RADIUS) * column_factor * exp2(-15) * 180 / PI);
}

int Image::seviriDYFromLineFactor(int line_factor)
{
	// This computation has been found by Dr2 Francesca Di Giuseppe and simplified by Enrico Zini
	return (int)round(asin(EARTH_RADIUS / ORBIT_RADIUS) * line_factor * exp2(-15) * 180 / PI);
	//return round((2 * asin(EARTH_RADIUS / ORBIT_RADIUS)) / atan(pixelVSize() / (ORBIT_RADIUS-EARTH_RADIUS)));
}

int Image::columnFactorFromSeviriDX(int seviriDX)
{
	return (int)round((double)seviriDX * PI / (asin(EARTH_RADIUS / ORBIT_RADIUS)*exp2(-15)*180));
}

int Image::lineFactorFromSeviriDY(int seviriDY)
{
	return (int)round((double)seviriDY * PI / (asin(EARTH_RADIUS / ORBIT_RADIUS)*exp2(-15)*180));
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

//
// ImageData
//

float* ImageData::allScaled() const
{
	float* res = new float[lines * columns];
	for (int y = 0; y < lines; ++y)
		for (int x = 0; x < columns; ++x)
			res[y * columns + x] = scaled(x, y);
	return res;
}

int* ImageData::allUnscaled() const
{
	int* res = new int[lines * columns];
	for (int y = 0; y < lines; ++y)
		for (int x = 0; x < columns; ++x)
			res[y * columns + x] = unscaled(x, y);
	return res;
}

int ImageData::decimalScale() const
{
	int res = -(int)floor(log10(slope));
	if (exp10(res) == slope)
		return res;
	else
		return res + 1;
}


//
// ImageDumper
//

class ImageDumper : public ImageConsumer
{
	bool withContents;

public:
	ImageDumper(bool withContents) : withContents(withContents) {}

	virtual void processImage(const Image& img)
	{
		cout << "Image " << img.datetime() << endl;
		cout << " proj: GEOS(" << img.sublon << ") ch.id: " << img.channel_id << " sp.id: " << img.spacecraft_id << endl;
		cout << " size: " << img.data->columns << "x" << img.data->lines << " factor: " << img.column_factor << "x" << img.line_factor
				 << " offset: " << img.column_offset << "x" << img.line_offset << endl;

		cout << " Images: " << endl;

		cout << "  " //<< *i
				 << "\t" << img.data->columns << "x" << img.data->lines << " " << img.data->bpp << "bpp"
						" *" << img.data->slope << "+" << img.data->offset << " decscale: " << img.data->decimalScale()
				 << " PSIZE " << img.pixelHSize() << "x" << img.pixelVSize()
				 << " DX " << Image::seviriDXFromColumnFactor(img.column_factor)
				 << " DY " << Image::seviriDYFromLineFactor(img.line_factor)
				 << " CHID " << img.channel_id
				 << endl;

		if (withContents)
		{
			cout << "Coord\tUnscaled\tScaled" << endl;
			for (int l = 0; l < img.data->lines; ++l)
				for (int c = 0; c < img.data->lines; ++c)
					cout << c << "x" << l << '\t' << img.data->unscaled(c, l) << '\t' << img.data->scaled(c, l) << endl;
		}
  }
};

std::auto_ptr<ImageConsumer> createImageDumper(bool withContents)
{
	return std::auto_ptr<ImageConsumer>(new ImageDumper(withContents));
}

}

// vim:set ts=2 sw=2:
