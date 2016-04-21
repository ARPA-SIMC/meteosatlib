# Process this file with automake to produce Makefile.in.

noinst_LTLIBRARIES = libmsatdrv.la
dist_noinst_HEADERS =
libmsatdrv_la_SOURCES =
libmsatdrv_la_CPPFLAGS = -I$(top_srcdir) -I$(top_builddir) -Werror
libmsatdrv_la_CXXFLAGS =
libmsatdrv_la_LIBADD = ../msat/libmsat.la

if HAVE_GRIBAPI
dist_noinst_HEADERS += \
    grib/grib.h \
    grib/utils.h
libmsatdrv_la_CPPFLAGS += $(GDAL_CFLAGS) $(GRIBAPI_CFLAGS) $(MSAT_CFLAGS)
libmsatdrv_la_SOURCES += grib/grib.cpp
libmsatdrv_la_LIBADD += $(GDAL_LIBS) $(GRIBAPI_LIBS) $(MSAT_LIBS)
endif

if HAVE_NETCDF
dist_noinst_HEADERS += \
    netcdf/utils.h \
    netcdf/netcdf.h \
    netcdf/netcdf24.h
libmsatdrv_la_CPPFLAGS += $(GDAL_CFLAGS) $(MSAT_CFLAGS) $(NETCDF_CFLAGS)
libmsatdrv_la_SOURCES += \
    netcdf/utils.cpp \
    netcdf/netcdf.cpp \
    netcdf/netcdf24.cpp
libmsatdrv_la_LIBADD += $(GDAL_LIBS) $(MSAT_LIBS) $(NETCDF_LIBS)
endif

if HRIT
dist_noinst_HEADERS += \
    xrit/xrit.h \
    xrit/dataset.h \
    xrit/rasterband.h \
    xrit/reflectance.h
libmsatdrv_la_CPPFLAGS += $(GDAL_CFLAGS) $(MSAT_CFLAGS)
libmsatdrv_la_SOURCES += \
    xrit/xrit.cpp \
    xrit/dataset.cpp \
    xrit/rasterband.cpp \
    xrit/reflectance.cpp
libmsatdrv_la_LIBADD += $(GDAL_LIBS) $(MSAT_LIBS)
endif

gdalplugindir = $(libdir)/@GDAL_PLUGIN_DIRNAME@
gdalplugin_LTLIBRARIES = gdal_Meteosatlib.la
gdal_Meteosatlib_la_LDFLAGS = -module
gdal_Meteosatlib_la_LIBADD = libmsatdrv.la $(MSAT_LIBS)
gdal_Meteosatlib_la_SOURCES = msatgdalplugin.cpp

noinst_LTLIBRARIES += libmsat2gdal.la
libmsat2gdal_la_LDFLAGS = -module
libmsat2gdal_la_LIBADD = libmsatdrv.la $(MSAT_LIBS)
libmsat2gdal_la_SOURCES = msat2gdal.cpp
