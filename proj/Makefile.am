## Makefile.am -- Process this file with automake to produce Makefile.in
## $Id: /local/meteosatlib/conv/Makefile.am 1898 2006-10-12T17:29:20.474497Z enrico  $
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
    const.h \
    Points.h \
    Projection.h \
    Geos.h \
    Mercator.h \
    Polar.h

noinst_LTLIBRARIES = libproj.la
libproj_la_SOURCES = \
    Geos.cpp \
    Mercator.cpp \
    Polar.cpp

MAINTAINERCLEANFILES = \
    Makefile.in

## -fin-
## vim:syn=automake:
