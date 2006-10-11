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

noinst_HEADERS = \
    Image.h \
    ExportGRIB.h \
    ExportNetCDF.h \
    ImportGRIB.h \
    ImportSAFH5.h \
    ImportNetCDF.h \
    ImportXRIT.h \
    SAFH5Utils.h

noinst_LTLIBRARIES = libconv.la
libconv_la_SOURCES = \
    Image.cpp \
    ImportGRIB.cpp \
    ExportGRIB.cpp
libconv_la_LIBADD = \
    ../grib/libgrib.la

if HRIT
libconv_la_SOURCES += ImportXRIT.cpp
libconv_la_LIBADD += ../hrit/libhrit.la
endif

if HAVE_NETCDF
libconv_la_SOURCES += ImportNetCDF.cpp ExportNetCDF.cpp
libconv_la_LIBADD += -lnetcdf_c++ -lnetcdf
endif

if HDF5
libconv_la_SOURCES += SAFH5Utils.cpp ImportSAFH5.cpp
libconv_la_LIBADD += -lhdf5_cpp
endif

MAINTAINERCLEANFILES = \
    Makefile.in

## -fin-
## vim:syn=automake:
