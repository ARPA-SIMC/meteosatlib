#include "Image.h"
#include "facts.h"

#include <gdal/ogr_spatialref.h>

#include <sstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <limits>
#include <math.h>
#include <limits.h>
#include <cstdlib>

// For HRIT satellite IDs
#include <hrit/MSG_spacecraft.h>
// For HRIT channel names
#include <hrit/MSG_channel.h>

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
	// When it's a known channel, se can use our internal table
	int res = facts::significantDigitsForChannel(channel_id);
	if (res != 0) return res;

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
		stringstream str;
		str << "no information found for channel " << channel_id;
		throw std::runtime_error(str.str());
	}
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
