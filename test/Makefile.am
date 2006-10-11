## Makefile.am -- Process this file with automake to produce Makefile.in
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

check_PROGRAMS = msatlib_test
TESTS = $(check_PROGRAMS)

noinst_PROGRAMS = $(test_bin)

noinst_HEADERS = \
	tut.h \
	tut_reporter.h \
	tut_restartable.h

msatlib_test_CPPFLAGS = \
	-I$(top_srcdir)/conv \
	-I$(top_srcdir)/grib
msatlib_test_LDADD = \
	-lhdf5_cpp \
	../hrit/libhrit.la \
	../grib/libgrib.la \
	../conv/libconv.la \
	-lnetcdf_c++ \
	-lnetcdf
msatlib_test_SOURCES = \
	tut-main.cpp \
	test-utils.cpp \
	test-importxrit.cpp \
	test-importgrib.cpp \
	test-importsafh5.cpp \
	test-importnetcdf.cpp \
	test-image.cpp \
	../hdf5/Utils.cpp

MAINTAINERCLEANFILES = \
    Makefile.in

## vim:syn=automake:
## -fin-
