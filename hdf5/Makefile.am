## Makefile.am -- Process this file with automake to produce Makefile.in
## Copyright (C) 2005 Enrico Zini <enrico@enricozini.org>
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

if HDF5

bin_PROGRAMS = \
    SAFH52Grib

SAFH52Grib_SOURCES = \
    SAFH52Grib.cpp
SAFH52Grib_LDADD = \
    ../conv/libconv.la

endif

MAINTAINERCLEANFILES = \
    Makefile.in

## -fin-
## vim:syn=automake:
