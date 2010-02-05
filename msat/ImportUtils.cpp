#include "ImportUtils.h"
#include "Image.h"
#include <cstdio>

using namespace std;

namespace msat {
namespace util {

void escapeSpacesAndDots(std::string& str)
{
	for (string::iterator i = str.begin(); i != str.end(); ++i)
		if (*i == ' ' || *i == '.')
			*i = '_';
}

std::string satelliteSingleImageFilename(const Image& img)
{
	// Get the string describing the spacecraft
	std::string spacecraft = Image::spacecraftName(img.spacecraft_id);

	// Get the string describing the sensor
	std::string sensor = Image::sensorName(img.spacecraft_id);

	// Get the string describing the channel
	std::string channel = Image::channelName(img.spacecraft_id, img.channel_id);

	escapeSpacesAndDots(spacecraft);
	escapeSpacesAndDots(sensor);
	escapeSpacesAndDots(channel);

	// Format the date
	char datestring[15];
	snprintf(datestring, 14, "%04d%02d%02d_%02d%02d", img.year, img.month, img.day, img.hour, img.minute);

	if (img.quality == '_')
		return spacecraft + "_" + sensor + "_" + channel + "_channel_" + datestring;
	else
		return string() + img.quality + "_" + spacecraft + "_" + sensor + "_" + channel + "_channel_" + datestring;
}

std::string satelliteSingleImageShortName(const Image& img)
{
	string channelstring = Image::channelName(img.spacecraft_id, img.channel_id);
	escapeSpacesAndDots(channelstring);
	return channelstring;
}

}
}

// vim:set ts=2 sw=2:
