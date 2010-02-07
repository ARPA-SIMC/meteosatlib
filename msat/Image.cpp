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
