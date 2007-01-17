#include "Image.h"

#include <sstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <limits>
#include <math.h>
#include <limits.h>
#include "proj/const.h"

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

//	cerr << "  coordsToPixels: " << lat << "," << lon << " -> " << p.x << "," << p.y << endl;

	x = (int)rint((double)p.x * column_res) + (column_offset - x0);
	y = (int)rint((double)p.y * line_res) + (line_offset   - y0);

//	cerr << "    to pixels: " << dx << "," << dy << endl;
}

void Image::pixelsToCoords(int x, int y, double& lat, double& lon) const
{
	proj::ProjectedPoint pp;
	pp.x = (double)(x - (column_offset - x0)) / column_res;
	pp.y = (double)(y - (line_offset   - y0)) / line_res;
	proj::MapPoint p;
	proj->projectedToMap(pp, p);
	lat = p.lat;
	lon = p.lon;
}

double Image::pixelHSize() const
{
	return (ORBIT_RADIUS - EARTH_RADIUS) * tan( (1.0/column_res) * M_PI / 180 );
}

double Image::pixelVSize() const
{
	return (ORBIT_RADIUS - EARTH_RADIUS) * tan( (1.0/line_res) * M_PI / 180 );
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

// Reimplemented here to be able to link without libhrit in case it's disabled
std::string Image::spacecraftName(int hritID)
{
  switch (hritID)
  {
    case MSG_NO_SPACECRAFT: return "Non Spacecraft";
    case MSG_METOP_1: return "METOP1";
    case MSG_METOP_2: return "METOP2";
    case MSG_METOP_3: return "METOP3";
    case MSG_METEOSAT_3: return "METEOSAT3";
    case MSG_METEOSAT_4: return "METEOSAT4";
    case MSG_METEOSAT_5: return "METEOSAT5";
    case MSG_METEOSAT_6: return "METEOSAT6";
    case MSG_MTP_1: return "MTP1";
    case MSG_MTP_2: return "MTP2";
    case MSG_MSG_1: return "MSG1";
    case MSG_MSG_2: return "MSG2";
    case MSG_MSG_3: return "MSG3";
    case MSG_MSG_4: return "MSG4";
    case MSG_NOAA_12: return "NOAA12";
    case MSG_NOAA_13: return "NOAA13";
    case MSG_NOAA_14: return "NOAA14";
    case MSG_NOAA_15: return "NOAA15";
    case MSG_GOES_7: return "GOES7";
    case MSG_GOES_8: return "GOES8";
    case MSG_GOES_9: return "GOES9";
    case MSG_GOES_10: return "GOES10";
    case MSG_GOES_11: return "GOES11";
    case MSG_GOES_12: return "GOES12";
    case MSG_GOMS_1: return "GOMS1";
    case MSG_GOMS_2: return "GOMS2";
    case MSG_GOMS_3: return "GOMS3";
    case MSG_GMS_4: return "GMS4";
    case MSG_GMS_5: return "GMS5";
    case MSG_GMS_6: return "GMS6";
    case MSG_MTSAT_1: return "MTSAT1";
    case MSG_MTSAT_2: return "MTSAT2";
  }
	return "unknown";
}

// Reimplemented here to be able to link without libhrit in case it's disabled
std::string Image::sensorName(int hritSpacecraftID)
{
	switch (hritSpacecraftID)
	{
		case MSG_MSG_1: return "Seviri";
		default: return "unknown";
	}
}

// Reimplemented here to be able to link without libhrit in case it's disabled
std::string Image::channelName(int hritSpacecraftID, int channelID)
{
	if (hritSpacecraftID == MSG_MSG_1)
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
		}
	}
	return "unknown";
}

std::string Image::defaultFilename() const
{
	t_enum_MSG_spacecraft sc = (t_enum_MSG_spacecraft)spacecraftIDToHRIT(spacecraft_id);
	// Get the string describing the spacecraft
	std::string spacecraft = spacecraftName(sc);

	// Get the string describing the sensor
	std::string sensor = sensorName(sc);

	// Get the string describing the channel
	std::string channel = channelName(sc, channel_id);

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
		//cout << "  ptc map " << x << "," << y << " -> " << lat << "," << lon << " -> " << nx << "," << ny << endl;
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
	 *    -Farsi dire l'area di destinazione
	 */
	// Compute projected bounding box
	proj::ProjectedBox pbox;
	res->proj->mapToProjected(box, pbox);
	double px0, py0, pw, ph;
#if 0
	{
		proj::MapBox bigbox(proj::MapPoint(90,-90), proj::MapPoint(90, 90), proj::MapPoint(-90,-90), proj::MapPoint(-90, 90));
		proj::ProjectedBox bigpbox;
		this->proj->mapToProjected(bigbox, bigpbox);
		bigpbox.boundingBox(px0, py0, pw, ph);
		cerr << "big bbox pre " << px0 << "," << py0 << " " << pw << "x" << ph << endl;

		res->proj->mapToProjected(bigbox, bigpbox);
		bigpbox.boundingBox(px0, py0, pw, ph);
		cerr << "big bbox post" << px0 << "," << py0 << " " << pw << "x" << ph << endl;

		proj::ProjectedPoint tpoint;
		res->proj->mapToProjected(proj::MapPoint(0, -90), tpoint);
		cerr << "left " << tpoint.x << "," << tpoint.y << endl;
		res->proj->mapToProjected(proj::MapPoint(90, 0), tpoint);
		cerr << "top " << tpoint.x << "," << tpoint.y << endl;
	}
#endif
	pbox.boundingBox(px0, py0, pw, ph);

	//cerr << "bbox " << px0 << "," << py0 << " " << pw << "x" << ph << endl;

	// Compute output pixel space
	//res->x0 = (int)rint(px0 * width / projw);  // fixme
	//res->x0 = (int)rint(px0 * height / projh); // fixme
	res->column_res = width / pw;
	res->line_res   = height / ph;
	res->x0 = (int)rint(px0 * res->column_res);
	res->y0 = (int)rint(py0 * res->line_res);
	res->column_offset = 0;
	res->line_offset = 0;
	if (res->x0 < 0) { res->column_offset = -res->x0; res->x0 = 0; }
	if (res->y0 < 0) { res->line_offset = -res->y0; res->y0 = 0; }

	//cerr << "orig x0: " << x0 << " y0 " << y0 << " coff " << column_offset << " loff " << line_offset << " cres " << column_res << " lres " << line_res << endl;
	//cerr << "orig x0: " << res->x0 << " y0 " << res->y0 << " coff " << res->column_offset << " loff " << res->line_offset << " cres " << res->column_res << " lres " << res->line_res << endl;

	/*
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

	res->quality = quality;
	res->history = history;
	res->addToHistory("reprojected");

	res->data = data->createReprojected(width, height, ReprojectMapper(*this, *res));

	// TODO: fill in res->data with data from this image

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
				 << " origin: " << img->x0 << "," << img->y0 << " centre: " << img->column_offset << "," << img->line_offset
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
