## Makefile.am -- Process this file with automake to produce Makefile.in
## $Id$
## Copyright (C) 2006 Enrico Zini <enrico@enricozini.org>
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2, or (at your option)
## any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software Foundation,
## Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
##

msatincludedir=$(includedir)/msat

nobase_dist_msatinclude_HEADERS = \
    proj/const.h \
    proj/Geos.h \
    proj/Mercator.h \
    proj/Points.h \
    proj/Polar.h \
    proj/Projection.h \
    Image.h \
    ExportGRIB.h \
    ExportNetCDF.h \
    ExportNetCDF24.h \
    ExportImage.h \
    ImportGRIB.h \
    ImportSAFH5.h \
    ImportNetCDF.h \
    ImportNetCDF24.h \
    ImportXRIT.h \
    NetCDFUtils.h \
    SAFH5Utils.h \
    Progress.h

noinst_LTLIBRARIES = libmsat.la
libmsat_la_SOURCES = \
    proj/Polar.cpp \
    proj/Geos.cpp \
    proj/Mercator.cpp \
    Image.cpp \
    ImportGRIB.cpp \
    ExportGRIB.cpp \
    Progress.cpp
libmsat_la_LIBADD = \
    ../grib/libgrib.la

if HRIT
libmsat_la_SOURCES += ImportXRIT.cpp
libmsat_la_LIBADD += ../hrit/libhrit.la
endif

if HAVE_NETCDF
libmsat_la_SOURCES += ImportNetCDF.cpp ExportNetCDF.cpp ImportNetCDF24.cpp ExportNetCDF24.cpp
libmsat_la_LIBADD += -lnetcdf_c++ -lnetcdf
endif

if HDF5
libmsat_la_SOURCES += SAFH5Utils.cpp ImportSAFH5.cpp
libmsat_la_LIBADD += -lhdf5_cpp
endif

if HAVE_MAGICK
libmsat_la_SOURCES += ExportImage.cpp
libmsat_la_LIBADD += -lMagick++
endif

MAINTAINERCLEANFILES = \
    Makefile.in

## -fin-
## vim:syn=automake:
