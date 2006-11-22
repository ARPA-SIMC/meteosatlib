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

bin_PROGRAMS = msat interpolate
man_MANS = 

noinst_PROGRAMS = msat

msat_SOURCES = msat.cpp
msat_LDADD = ../msat/libmsat.la 

interpolate_SOURCES = interpolate.cpp
interpolate_LDADD = ../msat/libmsat.la 

if DBALLE
bin_PROGRAMS += dbasat
man_MANS += dbasat.1

dbasat_SOURCES = dbasat.cpp
dbasat_LDADD = ../msat/libmsat.la $(LIBDBALLE_LIBS)
dbasat_INCLUDES = $(LIBDBALLE_CFLAGS)

dbasat.1: dbasat
	./$< help manpage > $@ || rm $@
endif

MAINTAINERCLEANFILES = \
    Makefile.in

## -fin-
## vim:syn=automake:
