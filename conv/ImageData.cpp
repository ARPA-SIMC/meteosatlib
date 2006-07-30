#include "ImageData.h"

#include <sstream>
#include <iomanip>
#include <math.h>
#include "parameters.h"

using namespace std;

float ImageData::pixelSize() const
{
	return (ORBIT_RADIUS - EARTH_RADIUS) * tan( (1.0/column_factor/exp2(-16)) * PI / 180 );
}

float ImageData::seviriDX() const
{
	return round((2 * asin(EARTH_RADIUS / ORBIT_RADIUS)) / atan(pixelSize() / (ORBIT_RADIUS-EARTH_RADIUS)));
}

float ImageData::seviriDY() const
{
	return seviriDX();
}

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

std::string ImageData::datetime() const
{
	stringstream res;
	res << setfill('0') << setw(4) << year << "-" << setw(2) << month << "-" << setw(2) << day
			<< " " << setw(2) << hour << ":" << setw(2) << minute;
	return res.str();
}

time_t ImageData::forecastSeconds2000() const
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

// vim:set ts=2 sw=2:
