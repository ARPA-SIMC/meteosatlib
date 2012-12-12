#include "utils.h"
#include <stdexcept>
#include <iostream>

using namespace std;

namespace mh5
{

// File hid_t wrapper

hid_file::hid_file(const std::string& fname, unsigned flags)
{
    name = fname;
    h = H5Fopen(fname.c_str(), flags, H5P_DEFAULT);
    if (h < 0)
        throw Error("cannot open hdf5 file " + fname);
}

hid_file::~hid_file()
{
    if (h >= 0)
        H5Fclose(h);
}

// Group hid_t wrapper

hid_group::hid_group(hid_t h, const std::string& name)
{
    this->h = h;
    this->name = name;
}

hid_group::~hid_group()
{
    if (h >= 0)
        H5Gclose(h);
}

// Dataset hid_t wrapper

hid_dataset::hid_dataset(hid_t h, const std::string& name)
{
    this->h = h;
    this->name = name;
}

hid_dataset::~hid_dataset()
{
    if (h >= 0)
        H5Dclose(h);
}

// Attribute hid_t wrapper

hid_attribute::hid_attribute(hid_t h, const std::string& name)
{
    this->h = h;
    this->name = name;
}

hid_attribute::~hid_attribute()
{
    if (h >= 0)
        H5Aclose(h);
}

// DataType wrapper

DataType::DataType(hid_t h, bool copy)
    : h(-1)
{
    if (copy)
    {
        h = H5Tcopy(h);
        if (h < 0) throw Error("copying datatype");
    }
    this->h = h;
}

DataType::~DataType()
{
    if (h >= 0) H5Tclose(h);
}

DataType::DataType(const DataType& dt)
    : h(dt.h)
{
    DataType* d = const_cast<DataType*>(&dt);
    d->h = -1;
}
DataType& DataType::operator=(const DataType& dt)
{
    h = dt.h;
    DataType* d = const_cast<DataType*>(&dt);
    d->h = -1;
    return *this;
}

size_t DataType::get_size() const
{
    size_t res = H5Tget_size(h);
    if (res == 0)
        throw Error("cannot get size of datatype");
    return res;
}

// Basic implementation for objects based on hid_t

Object::Object() : h(0) {}

Object::Object(hid* h) : h(h) { if (h) h->ref(); }

Object::Object(const Object& f)
{
    h = f.h;
    if (h) h->ref();
}

Object::~Object()
{
    if (h && h->unref())
        delete h;
}

Object& Object::operator=(const Object& f)
{
    if (f.h) f.h->ref();
    if (h && h->unref()) delete h;
    h = f.h;
    return *this;
}

void Object::init(hid* new_h)
{
    if (h && h->unref())
    {
        delete h;
        h = 0;
    }

    h = new_h;
    h->ref();
}

bool Object::has_attr(const std::string& name)
{
    htri_t res = H5Aexists(*h, name.c_str());
    if (res < 0)
        throw Error("checking if attribute " + name + " exists in " + h->name);
    return res > 0;
}

bool Object::has_attr(const std::string& path, const std::string& name)
{
    htri_t res = H5Aexists_by_name(*h, path.c_str(), name.c_str(), H5P_DEFAULT);
    if (res < 0)
        throw Error("checking if attribute " + name + " exists in " + h->name + ":" + path);
    return res > 0;
}

Attribute Object::attr(const std::string& name)
{
    hid_t res = H5Aopen(*h, name.c_str(), H5P_DEFAULT);
    if (res < 0)
        throw Error("accessing attribute " + name + " in " + h->name);
    return Attribute(new hid_attribute(res, name));
}

Attribute Object::attr(const std::string& path, const std::string& name)
{
    hid_t res = H5Aopen_by_name(*h, path.c_str(), name.c_str(), H5P_DEFAULT, H5P_DEFAULT);
    if (res < 0)
        throw Error("accessing attribute " + name + " in " + h->name + ":" + path);
    return Attribute(new hid_attribute(res, name));
}

Group Object::group(const std::string& name)
{
    hid_t g = H5Gopen(*h, name.c_str(), H5P_DEFAULT);
    if (g < 0)
        throw Error("group " + name + " not found in " + h->name);

    return Group(new hid_group(g, h->name + ":" + name));
}

Dataset Object::dataset(const std::string& name)
{
    hid_t g = H5Dopen(*h, name.c_str(), H5P_DEFAULT);
    if (g < 0)
        throw Error("dataset " + name + " not found in " + h->name);

    return Dataset(new hid_dataset(g, h->name + ":" + name));
}

std::string Object::get_objname_by_idx(hsize_t idx) const
{
    // Read length
    ssize_t size = H5Lget_name_by_idx(*h, ".", H5_INDEX_NAME, H5_ITER_INC, idx, NULL, 0, H5P_DEFAULT);
    if (size < 0)
        throw Error("cannot read object name length for an object index in " + h->name);

    // Read name
    char buf[size + 1];
    ssize_t res = H5Lget_name_by_idx(*h, ".", H5_INDEX_NAME, H5_ITER_INC, idx, buf, size + 1, H5P_DEFAULT);
    if (res < 0)
        throw Error("cannot read object name for an object index in " + h->name);

    return string(buf, size);
}


// File access

bool File::is_hdf5(const std::string& fname)
{
    htri_t res = H5Fis_hdf5(fname.c_str());
    if (res < 0)
        throw Error("failed to check if " + fname + " is an HDF5 file");
    return res > 0;
}

void File::open(const std::string& fname, unsigned flags)
{
    init(new hid_file(fname, flags));
}

// Group access

hsize_t Group::get_num_objs() const
{
    hsize_t val;
    herr_t res = H5Gget_num_objs(*h, &val);
    if (res < 0)
        throw Error("cannot read number of objects in " + h->name);
    return val;
}

// Dataset access

DataType Dataset::datatype() const
{
    hid_t res = H5Dget_type(*h);
    if (res < 0)
        throw Error("cannot get data type for dataset " + h->name);
    return DataType(res, false);
}

size_t Dataset::npoints() const
{
    hid_t space = H5Dget_space(*h);
    if (space < 0)
        throw Error("cannot get dataspace for dataset " + h->name);

    hssize_t res = H5Sget_simple_extent_npoints(space);
    if (res == 0)
    {
        H5Sclose(space);
        throw Error("cannot get number of points from dataspace for dataset " + h->name);
    }

    H5Sclose(space);
    return res;
}

void Dataset::read(void* buf, hid_t datatype) const
{
    DataType dt(datatype);
    herr_t res = H5Dread(*h, dt, H5S_ALL, H5S_ALL, H5P_DEFAULT, buf);
    if (res < 0)
        throw Error("cannot read data from dataset " + h->name);
}

// Attribute access

int Attribute::as_int() const
{
    DataType dt(H5T_NATIVE_INT);
    int val;
    herr_t res = H5Aread(*h, dt, &val);
    if (res < 0)
        throw Error("cannot read attribute " + h->name + " as an int");

    return val;
}

float Attribute::as_float() const
{
    DataType dt(H5T_NATIVE_FLOAT);
    float val;
    herr_t res = H5Aread(*h, dt, &val);
    if (res < 0)
        throw Error("cannot read attribute " + h->name + " as a float");

    return val;
}

std::string Attribute::as_string() const
{
    hsize_t size = H5Aget_storage_size(*h);
    if (size == 0)
        throw Error("cannot read storage size for attribute " + h->name);

    hid_t type = H5Aget_type(*h);
    if (type < 0)
        throw Error("cannot read storage type for attribute " + h->name);

    DataType dt(type, false);

    char buf[size];
    herr_t res = H5Aread(*h, dt, buf);
    if (res < 0)
        throw Error("reading attribute " + h->name + " as a string");

    return string(buf, size);
}

}

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

SAFChannelInfo* SAFChannelByName(const std::string& name)
{
	// Sequential scan
	for (size_t i = 0; i < sizeof(channel_table) / sizeof(SAFChannelInfo); ++i)
		if (name == channel_table[i].name)
			return &channel_table[i];
	return NULL;
}

SAFChannelInfo* SAFChannelByID(int id)
{
	// Sequential scan
	for (size_t i = 0; i < sizeof(channel_table) / sizeof(SAFChannelInfo); ++i)
		if (id == channel_table[i].channelID)
			return &channel_table[i];
	return NULL;
}

// vim:set ts=2 sw=2:
