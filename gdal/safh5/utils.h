#ifndef MSAT_GDAL_SAFH5_UTILS_H
#define MSAT_GDAL_SAFH5_UTILS_H

#include <string>
#include <hdf5.h>

namespace mh5 {
/*
 * There already is a C++ interface for HDF5, but it seems to be too much of a
 * moving target at the moment to be able to work with it. So I'll have to roll
 * my own, simple wrappers.
 */

class Group;
class Attribute;
class Dataset;

// http://www.hdfgroup.org/HDF5/doc/UG/13_ErrorHandling.html

class Error : public std::exception
{
protected:
    std::string msg;

public:
    Error(const std::string& msg) : msg(msg) {}
    virtual ~Error() throw () {}
    virtual const char* what() const throw() { return msg.c_str(); }
};

/// Wrap a hid_t, with reference counting
struct hid
{
    hid_t h;
    mutable int _ref;
    std::string name;

    hid() : h(-1), _ref(0) {};
    virtual ~hid() {}

    void ref() const { ++_ref; }
    bool unref() const { return (--_ref) == 0; }

    operator hid_t() { return h; }

private:
    // Disallow copying
    hid(const hid&);
    hid& operator=(const hid&);
};

struct hid_file : public hid
{
    hid_file(const std::string& fname, unsigned flags);
    virtual ~hid_file();
};

struct hid_group : public hid
{
    hid_group(hid_t h, const std::string& name);
    virtual ~hid_group();
};

struct hid_dataset : public hid
{
    hid_dataset(hid_t h, const std::string& name);
    virtual ~hid_dataset();
};

struct hid_attribute : public hid
{
    hid_attribute(hid_t h, const std::string& name);
    virtual ~hid_attribute();
};

class DataType
{
protected:
    hid_t h;

public:
    DataType(hid_t h, bool copy=true);
    ~DataType();

    operator hid_t() { return h; }

    // unique_ptr-type copy semantics
    DataType(const DataType& dt);
    DataType& operator=(const DataType&);

    size_t get_size() const;
};

class Object
{
protected:
    hid* h;

public:
    Object();
    Object(hid* h);
    Object(const Object& f);
    ~Object();
    Object& operator=(const Object& f);

    void init(hid* h);

    bool has_attr(const std::string& name);
    bool has_attr(const std::string& path, const std::string& name);

    Attribute attr(const std::string& name);
    Attribute attr(const std::string& path, const std::string& name);

    Group group(const std::string& name);
    Dataset dataset(const std::string& name);

    std::string get_objname_by_idx(hsize_t idx) const;
};

class Group : public Object
{
public:
    Group() : Object() {}
    Group(hid* h) : Object(h) {}
    Group(const Group& g) : Object(g) {}
    Group& operator=(const Group& g) { Object::operator=(g); return *this; }

    hsize_t get_num_objs() const;
};

class Dataset : public Object
{
public:
    Dataset() : Object() {}
    Dataset(hid* h) : Object(h) {}
    Dataset(const Dataset& d) : Object(d) {}
    Dataset& operator=(const Dataset& d) { Object::operator=(d); return *this; }

    DataType datatype() const;

    size_t npoints() const;

    void read(uint8_t* buf) const { return read(buf, H5T_NATIVE_UCHAR); }
    void read(uint16_t* buf) const { return read(buf, H5T_NATIVE_USHORT); }
    void read(uint32_t* buf) const { return read(buf, H5T_NATIVE_ULONG); }
    void read(void* buf, hid_t datatype) const;
};

class Attribute : public Object
{
public:
    Attribute() : Object() {}
    Attribute(hid* h) : Object(h) {}
    Attribute(const Attribute& a) : Object(a) {}
    Attribute& operator=(const Attribute& a) { Object::operator=(a); return *this; }

    int as_int() const;
    float as_float() const;
    std::string as_string() const;
};

class File : public Object
{
public:
    File() : Object() {}
    File(hid* h) : Object(h) {}
    File(const File& f) : Object(f) {}
    File& operator=(const File& f) { Object::operator=(f); return *this; }

    std::string filename() const;

    void open(const std::string& fname, unsigned flags);

    static bool is_hdf5(const std::string& fname);
};

}

struct SAFChannelInfo
{
	const char* name;
	int channelID;
	float slope;
	float offset;
	size_t bpp;
};

SAFChannelInfo* SAFChannelByName(const std::string& name);
SAFChannelInfo* SAFChannelByID(int id);

#endif
