#include "SAFH5Utils.h"
#include <H5Cpp.h>
#include <iostream>

using namespace H5;
using namespace std;

// Table that maps HDF5 product names to channel IDs
struct SAFChannelInfo channel_table[] = {
	{ "AMA_CL",        100,   1,          0,         8 },
	{ "CMa",           101,   1,          0,         8 },
	{ "CMa_DUST",      102,   1,          0,         8 },
	{ "CMa_QUALITY",   103,   1,          0,        16 },
	{ "CMa_TEST",      104,   1,          0,        16 },
	{ "CMa_VOLCANIC",  105,   1,          0,         8 },
	{ "CRR",           106,   1,          0,         8 },
	{ "CRR_DATAFLAG",  107,   1,          0,        16 },
	{ "CRR_QUALITY",   108,   1,          0,        16 },
	{ "CT",            109,   1,          0,         8 },
	{ "CT_PHASE",      110,   1,          0,        16 },
	{ "CT_QUALITY",    111,   1,          0,        16 },
	{ "CTTH_EFFECT",   112,   5,        -50,         8 },
	{ "CTTH_HEIGHT",   113, 200,      -2000,         8 },
	{ "CTTH_PRESS",    114,  25,       -250,         8 },
	{ "CTTH_QUALITY",  115,   1,          0,        16 },
	{ "CTTH_TEMPER",   116,   1,        150,         8 },
	{ "LPW_BL",        117,   0.294118,  -2.35294,   8 },
	{ "LPW_CLEARSKY",  118,   1,          0,        16 },
	{ "LPW_CLOUDY",    119,   1,          0,        16 },
	{ "LPW_CONS_SEA",  120,   1,          0,        16 },
	{ "LPW_HL",        121,   0.067227,  -0.537815,  8 },
	{ "LPW_INT_TPW",   122,   0.588235,  -4.70588,   8 },
	{ "LPW_ML",        123,   0.378151,  -3.02521,   8 },
	{ "LPW_QUALITY",   124,   1,          0,        16 },
	{ "LPW_RAD_RAN",   125,   1,          0,        16 },
	{ "PC_PROB1",      126,  10,          0,         8 },
	{ "PC_PROB2",      127,   1,          0,         8 },
	{ "PC_QUALITY",    128,   1,          0,         8 },
	{ "SAI",           129,  -0.336134,   27.6891,   8 },
	{ "SAI_CLEARSKY",  130,   1,          0,        16 },
	{ "SAI_CLOUDY",    131,   1,          0,        16 },
	{ "SAI_RAD_RAN",   132,   1,          0,        16 },
	{ "TPW",           133,   0.588235,  -4.70588,   8 },
	{ "TPW_CLEARSKY",  134,   1,          0,        16 },
	{ "TPW_CONDIT",    135,   1,          0,        16 },
	{ "TPW_QUALITY",   136,   1,          0,        16 },
};

string readStringAttribute(const H5Object& dataGroup, const std::string& name)
{
  try {
    string res;
    Attribute a = dataGroup.openAttribute(name);
    DataType dt = a.getDataType();
    a.read(dt, res);
    res.resize(dt.getSize());
    return res;
  } catch (...) {
    std::cerr << "File does not have " << name << " attribute..." << std::endl;
    throw;
  }
}

int readIntAttribute(const H5Object& dataGroup, const std::string& name)
{
  try
  {
    DataType intType(H5T_NATIVE_INT, 1);
    int res;
    dataGroup.openAttribute(name).read(intType, &res);
    return res;
  }
  catch ( ... )
  {
    std::cerr << "File does not have " << name << " attribute..." << std::endl;
    throw;
  }
}

float readFloatAttribute(const H5Object& dataGroup, const std::string& name)
{
  try
  {
    DataType doubleType(H5T_NATIVE_FLOAT, 1);
    float res;
    dataGroup.openAttribute(name).read(doubleType, &res);
    return res;
  }
  catch ( ... )
  {
    std::cerr << "File does not have " << name << " attribute..." << std::endl;
    throw;
  }
}

SAFChannelInfo* SAFChannelByName(const std::string& name)
{
	// Sequential scan
	for (int i = 0; i < sizeof(channel_table) / sizeof(SAFChannelInfo); ++i)
		if (name == channel_table[i].name)
			return &channel_table[i];
	return NULL;
}

SAFChannelInfo* SAFChannelByID(int id)
{
	// Sequential scan
	for (int i = 0; i < sizeof(channel_table) / sizeof(SAFChannelInfo); ++i)
		if (id == channel_table[i].channelID)
			return &channel_table[i];
	return NULL;
}

// vim:set ts=2 sw=2:
