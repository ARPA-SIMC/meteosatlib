#ifndef MSAT_GRIB_UTILS_H
#define MSAT_GRIB_UTILS_H

#include <grib_api.h>
#include "msat/gdal/clean_cpl_error.h"
#include <string>
#include <cerrno>

// Define this if you want to create a foile /tmp/trace-gribapi with a trace of
// all grib_api operations
#define TRACE_GRIBAPI

#ifdef TRACE_GRIBAPI
#include <cstdio>
#endif

#define RUN_OR_RETURN(expr) do { \
    CPLErr res = (expr); \
    if (res != CE_None) return res; \
} while (0)

namespace msat {
namespace grib {

struct griberror
{
    griberror() {}
};

static inline void checked(int error, const char* context = NULL, const char* func = NULL)
{
    if (error != GRIB_SUCCESS)
    {
        if (context and func)
            CPLError(CE_Failure, CPLE_AppDefined, "%s %s: %s", func, context, grib_get_error_message(error));
        else if (context)
            CPLError(CE_Failure, CPLE_AppDefined, "%s: %s", context, grib_get_error_message(error));
        else
            CPLError(CE_Failure, CPLE_AppDefined, "%s", grib_get_error_message(error));
        throw griberror();
    }
}

// Little abstraction layer on top of grib_api
struct Grib
{
#ifdef TRACE_GRIBAPI
    FILE* trace = nullptr;
    #define trace(...) do {\
            fprintf(trace, "GH %p: ", static_cast<const void*>(gh)); \
            fprintf(trace, __VA_ARGS__); \
            fprintf(trace, "\n"); \
            fflush(trace); \
        } while (0)
#else
    #define trace(...) do {} while(0)
#endif

    grib_handle* gh = nullptr;
    FILE* fp = nullptr;

    Grib()
    {
#ifdef TRACE_GRIBAPI
        trace = fopen("/tmp/trace-gribapi", "a+");
#endif
    }
    Grib(grib_handle* gh) : gh(gh)
    {
#ifdef TRACE_GRIBAPI
        trace = fopen("/tmp/trace-gribapi", "a+");
#endif
    }
    Grib(const Grib&) = delete;
    Grib(Grib&& o)
        : gh(o.gh), fp(o.fp)
    {
        o.gh = nullptr;
        o.fp = nullptr;
#ifdef TRACE_GRIBAPI
        trace = o.trace;
        o.trace = nullptr;
#endif
    }
    ~Grib() {
        if (trace) trace("close");
        if (gh) grib_handle_delete(gh);
#ifdef TRACE_GRIBAPI
        if (trace) fclose(trace);
#endif
        if (fp) fclose(fp);
    }

    Grib& operator=(const Grib&) = delete;
    Grib& operator=(Grib&& o)
    {
        if (this == &o) return *this;
        if (gh) grib_handle_delete(gh);
        gh = o.gh;
        o.gh = nullptr;
        if (fp) fclose(fp);
        fp = o.fp;
        o.fp = nullptr;
#ifdef TRACE_GRIBAPI
        if (trace) fclose(trace);
        trace = o.trace;
        o.trace = nullptr;
#endif
        return *this;
    }

    grib_handle* release_handle()
    {
        grib_handle* res = gh;
        trace("release");
        gh = 0;
        return res;
    }

    CPLErr new_from_samples(grib_context* c, const char* name)
    {
        gh = grib_handle_new_from_samples(c, name);
        trace("h = grib_handle_new_from_samples(%p, \"%s\"); /* %p */", static_cast<const void*>(c), name, static_cast<const void*>(gh));
        if (gh == NULL)
        {
            CPLError(CE_Failure, CPLE_AppDefined, "Cannot create handle from samples %s", name);
            return CE_Failure;
        }
        return CE_None;
    }

#if 0
    CPLErr new_from_template(grib_context* c, const char* name)
    {
        gh = grib_handle_new_from_template(c, name);
        trace("h = grib_handle_new_from_template(%p, \"%s\"); /* %p */", c, name, gh);
        if (gh == NULL)
        {
            CPLError(CE_Failure, CPLE_AppDefined, "Cannot create handle from template %s", name);
            return CE_Failure;
        }
        return CE_None;
    }
#endif

    CPLErr new_from_file(grib_context* c, const char* name)
    {
        fp = fopen(name, "rb");
        if (!fp)
        {
            CPLError(CE_Failure, CPLE_OpenFailed, "%s cannot be opened: %s", name, strerror(errno));
            return CE_Failure;
        }
        int err;
        gh = grib_handle_new_from_file(0, fp, &err);
        trace("h = grib_handle_new_from_file(%p, f, &err); /* %p, %d (%s), f = %p open to %s */", static_cast<const void*>(c), static_cast<const void*>(gh), err, grib_get_error_message(err), static_cast<const void*>(fp), name);
        if (gh == NULL)
        {
            CPLError(CE_Failure, CPLE_AppDefined, "%s looks like a GRIB, but grib_api says %s",
                name, grib_get_error_message(err));
            return CE_Failure;
        }
        return CE_None;
    }

    long get_long(const char* key)
    {
        long res;
        int err = grib_get_long(gh, key, &res);
        trace("GRIB_CHECK(grib_get_long(h, \"%s\", &lval), %d); /* -> %ld */", key, err, res);
        checked(err, key, "get_long");
        return res;
    }

    bool get_long_ifexists(const char* key, long* res)
    {
        int err = grib_get_long(gh, key, res);
        trace("GRIB_CHECK(grib_get_long(h, \"%s\", &lval), %d); /* -> %ld */", key, err, *res);
        if (err == GRIB_SUCCESS)
            return true;
        if (err == GRIB_NOT_FOUND)
            return false;
        CPLError(CE_Failure, CPLE_AppDefined, "get_long_ifexists %s: %s", key, grib_get_error_message(err));
        throw griberror();
    }

    long get_long_oneof(const char* key, ...)
    {
        va_list ap;
        long res;
        int err = GRIB_NOT_FOUND;

        for (va_start(ap, key); key != NULL; key = va_arg(ap, const char*))
        {
            err = grib_get_long(gh, key, &res);
            trace("GRIB_CHECK(grib_get_long(h, \"%s\", &dval), %d); /* -> %ld */", key, err, res);
            if (err == GRIB_SUCCESS)
                break;
        }

        va_end(ap);
        checked(err, key, "get_long_oneof");
        return res;
    }

    int get_long_unchecked(const char* key, long* res)
    {
        int err = grib_get_long(gh, key, res);
        trace("GRIB_CHECK(grib_get_long(h, \"%s\", &lval), %d); /* -> %ld (unchecked) */", key, err, *res);
        return err;
    }

    void get_long_array(const char* key, long* values, size_t* length)
    {
        int err = grib_get_long_array(gh, key, values, length);
        trace("GRIB_CHECK(grib_get_long_array(h, \"%s\", values, len), %d); /* len: %zd */", key, err, *length);
        checked(err, key, "get_long_array");
    }

    double get_double(const char* key)
    {
        double res;
        int err = grib_get_double(gh, key, &res);
        trace("GRIB_CHECK(grib_get_double(h, \"%s\", &dval), %d); /* -> %f */", key, err, res);
        checked(err, key, "get_double");
        return res;
    }

    double get_double_oneof(const char* key, ...)
    {
        va_list ap;
        double res;
        int err = GRIB_NOT_FOUND;

        for (va_start(ap, key); key != NULL; key = va_arg(ap, const char*))
        {
            err = grib_get_double(gh, key, &res);
            trace("GRIB_CHECK(grib_get_double(h, \"%s\", &dval), %d); /* -> %f */", key, err, res);
            if (err == GRIB_SUCCESS)
                break;
        }

        va_end(ap);
        checked(err, key, "get_double_oneof");
        return res;
    }

    void get_double_array(const char* key, double* values, size_t* length)
    {
        int err = grib_get_double_array(gh, key, values, length);
        trace("get_double_array(\"%s\": %zd) -> %d", key, *length, err);
        checked(err, key, "get_double_array");
    }
    
    size_t set_string(const char* key, const std::string& str)
    {
        size_t size = str.size() + 1;
        int err = grib_set_string(gh, key, str.c_str(), &size);
        trace("set_string(\"%s\" <- %s: %zd) -> %d", key, str.c_str(), size, err);
        checked(err, key, "set_string");
        return size;
    }

    void set_long(const char* key, long val)
    {
        int err = grib_set_long(gh, key, val);
        trace("GRIB_CHECK(grib_set_long(h, \"%s\", %ld), %d);", key, val, err);
        checked(err, key, "set_long");
    }

    void set_long_oneof(const char* key, long val, ...)
    {
        va_list ap;
        int err = GRIB_NOT_FOUND;
            
        for (va_start(ap, val); key != NULL; key = va_arg(ap, const char*), val = va_arg(ap, long))
        {
            err = grib_set_long(gh, key, val);
            trace("GRIB_CHECK(grib_set_long(h, \"%s\", %ld), %d);", key, val, err);
            if (err == GRIB_SUCCESS)
                break;
        }

        va_end(ap);
        checked(err, key, "set_long_oneof");
    }

    int set_long_unchecked(const char* key, long val)
    {
        int err = grib_set_long(gh, key, val);
        trace("GRIB_CHECK(grib_set_long(h, \"%s\", %ld), %d); /* unchecked */", key, val, err);
        return err;
    }

    void set_double(const char* key, double val)
    {
        int err = grib_set_double(gh, key, val);
        trace("GRIB_CHECK(grib_set_double(h, \"%s\", %f), %d);", key, val, err);
        checked(err, key, "set_double");
    }

    void set_double_array(const char* key, double* values, size_t len)
    {
        int err = grib_set_double_array(gh, key, values, len);
        trace("set_double_array(\"%s\" <- %zd) -> %d", key, len, err);
        for (size_t i = 0; i < len; i += (len/5 == 0 ? 1 : len/5))
            trace("  set_double_array val %zd: %f", i, values[i]);
        checked(err, key, "set_double_array");
    }

    CPLErr write(const std::string& filename)
    {
        //fprintf(stderr, "WRITEGRIB TO %s\n", filename.c_str());
        const void* buffer;
        size_t size;

        /* Get the coded message in a buffer */
        int res = grib_get_message(gh, &buffer, &size);
        if (res != GRIB_SUCCESS)
        {
            CPLError(CE_Failure, CPLE_AppDefined, "cannot encode to buffer: %s", grib_get_error_message(res));
            return CE_Failure;
        }
        trace("encoded to %zd bytes", size);

        FILE* out = fopen(filename.c_str(), "w");
        if (out == NULL)
        {
            CPLError(CE_Failure, CPLE_OpenFailed, "cannot open file %s for writing", filename.c_str());
            return CE_Failure;
        }

        /* write the buffer in a file*/
        if (fwrite(buffer, 1, size, out) != size) 
        {
            fclose(out);
            CPLError(CE_Failure, CPLE_FileIO, "cannot write to file %s", filename.c_str());
            return CE_Failure;
        }
        trace("written to file %s", filename.c_str());

        fclose(out);
        trace("flushed");
        return CE_None;
    }

    /**
     * Format the time as YYYY-MM-DD HH:MM:SS
     *
     * The buffer must be at least 20 bytes long.
     */
    void formatTime(char* buf)
    {
        long edition = get_long("editionNumber");
        long year, month, day, hour, min, sec;

        switch (edition)
        {
            case 1: {
                long lce = get_long("centuryOfReferenceTimeOfData");
                long lye = get_long("yearOfCentury");
                year = (lce - 1) * 100 + lye;
                break;
            }
            case 2: {
                year = get_long("year");
                break;
            }
            default:
                CPLError(CE_Failure, CPLE_AppDefined, "unsupported grib edition %ld when reading image time", edition);
                throw griberror();
        }
        month = get_long("month");
        day = get_long("day");
        hour = get_long("hour");
        min = get_long("minute");
        sec = get_long("second");
        snprintf(buf, 20, "%04ld-%02ld-%02ld %02ld:%02ld:%02ld",
                year, month, day, hour, min, sec);
    }

    // Set the GRIB time from the given datetime string
    void setTime(const char* buf)
    {
        long edition = get_long("editionNumber");

        int year, month, day, hour, minute, second;
        if (sscanf(buf, "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second) != 6)
        {
            CPLError(CE_Failure, CPLE_AppDefined, "cannot parse time \"%s\"", buf);
            throw griberror();
        }

        switch (edition)
        {
            case 1:
                set_long("centuryOfReferenceTimeOfData", (year / 100) + 1);
                set_long("yearOfCentury", year % 100);
                break;
            case 2:
                set_long("year", year);
                break;
            default:
                CPLError(CE_Failure, CPLE_AppDefined, "unsupported grib edition %ld when setting image time", edition);
                throw griberror();
        }
        set_long("month", month);
        set_long("day", day);
        set_long("hour", hour);
        set_long("minute", minute);
        set_long_unchecked("second", second);
    }
};

#if 0
template<typename T>
static inline void grib_read_values(Grib& grib, T* buf, size_t* length)
{
    throw std::runtime_error("Reading this data type is not supported");
}
template<>
inline void grib_read_values<long>(Grib& grib, long* buf, size_t* length)
{
    grib.get_long_array("values", buf, length);
}
template<>
inline void grib_read_values<float>(Grib& grib, float* buf, size_t* length)
{
    // FIXME: ugly!
    double* buf1 = new double[*length];
    try {
        grib.get_double_array("values", buf1, length);
        for (size_t i = 0; i < *length; ++i)
            buf[i] = buf1[i];
    } catch (...) {
        delete[] buf1;
        throw;
    }
    delete[] buf1;
}
template<>
inline void grib_read_values<double>(Grib& grib, double* buf, size_t* length)
{
    grib.get_double_array("values", buf, length);
}
#endif

}
}

#undef trace
#endif
