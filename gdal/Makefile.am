# Process this file with automake to produce Makefile.in.
#
# Copyright (C) 2010--2012 Enrico Zini <enrico@enricozini.org>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

noinst_LTLIBRARIES = libmsatdrv.la
dist_noinst_HEADERS =
libmsatdrv_la_SOURCES =
libmsatdrv_la_CPPFLAGS = -I$(top_srcdir) -I$(top_builddir)
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

if HAVE_HDF5
dist_noinst_HEADERS += \
    safh5/utils.h \
    safh5/safh5.h
libmsatdrv_la_CPPFLAGS += $(HDF5_CFLAGS) $(GDAL_CFLAGS) $(MSAT_CFLAGS)
libmsatdrv_la_SOURCES += \
    safh5/utils.cpp \
    safh5/safh5.cpp
libmsatdrv_la_LIBADD += $(HDF5_LIBS) $(GDAL_LIBS) $(MSAT_LIBS)
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
