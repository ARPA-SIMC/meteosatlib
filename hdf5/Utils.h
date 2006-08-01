#ifndef UTILS_H
#define UTILS_H

#include <string>

namespace H5 {
class H5Object;
}

struct SAFChannelInfo
{
	const char* name;
	int channelID;
	float slope;
	float offset;
	int bpp;
};

std::string readStringAttribute(const H5::H5Object& dataGroup, const std::string& name);
int readIntAttribute(const H5::H5Object& dataGroup, const std::string& name);
float readFloatAttribute(const H5::H5Object& dataGroup, const std::string& name);
SAFChannelInfo* SAFChannelByName(const std::string& name);
SAFChannelInfo* SAFChannelByID(int id);

#endif