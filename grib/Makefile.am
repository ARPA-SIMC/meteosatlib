## Makefile.am -- Process this file with automake to produce Makefile.in
## $Id$
## Copyright (C) 2005 Deneys S. Maartens <dsm@tlabs.ac.za>
## Modified 2006 Enrico Zini <enrico@enricozini.org>
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

gribincludedir=$(includedir)/grib

dist_gribinclude_HEADERS = \
    Ebisuzaki.h \
    GRIB.h

dist_noinst_DATA = \
    gribtab

lib_LTLIBRARIES = libgrib.la
libgrib_la_SOURCES = Ebisuzaki.cpp GRIB.cpp

MAINTAINERCLEANFILES = \
    Makefile.in

## -fin-
## vim:syn=automake:
