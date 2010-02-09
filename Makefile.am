## Makefile.am -- Process this file with automake to produce Makefile.in
## $Id$
## Copyright (C) 2005 Deneys S. Maartens <dsm@tlabs.ac.za>
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

ACLOCAL_AMFLAGS = -I m4

DIST_SUBDIRS = \
    decompress \
    examples \
    iniparser \
    hri \
    hrit \
    msg-native \
    omtp-ids \
    openmtp \
    thornsds_db1 \
    grib \
    msat \
    images \
    tools \
    tests

## using SUBDIRS is a no-no

## variables updated by Automake.include files
dist_noinst_DATA =
noinst_HEADERS =
man_MANS =
bin_PROGRAMS =
check_PROGRAMS =
noinst_PROGRAMS =
dist_check_SCRIPTS =
lib_LTLIBRARIES =
noinst_LTLIBRARIES =
TESTS =
EXTRA_DIST =

if HRIT
include decompress/Automake.rules
endif

if HAVE_NETCDF
include examples/Automake.rules
endif

include grib/Automake.rules

if HRI
include hri/Automake.rules
endif

if HRIT
include hrit/Automake.rules
endif

include images/Automake.rules
include iniparser/Automake.rules

if MSG_NATIVE
include msg-native/Automake.rules
endif

include msat/Automake.rules

if OMTP_IDS
include omtp-ids/Automake.rules
endif

if OPENMTP
include openmtp/Automake.rules
endif

include tests/Automake.rules

if THORNSDS_DB1
include thornsds_db1/Automake.rules
endif

include tools/Automake.rules

.PHONY: dist-rpm
dist-rpm: distcheck
	rpmbuild -ta $(distdir).tar.gz

EXTRA_DIST += \
    $(ac_aux_dir)/autogen.sh \
    $(ac_aux_dir)/clobber \
    $(PACKAGE_NAME).spec \
    CREDITS \
    PublicDecompWT_LICENSE \
    libmsat.m4 \
    libmsat.pc.in \
    libhrit.m4 \
    libhrit.pc.in \
    libopenmtp.pc.in

AUX_DIST = \
    $(ac_aux_dir)/compile \
    $(ac_aux_dir)/config.guess \
    $(ac_aux_dir)/config.sub \
    $(ac_aux_dir)/depcomp \
    $(ac_aux_dir)/install-sh \
    $(ac_aux_dir)/ltmain.sh \
    $(ac_aux_dir)/missing \
    $(ac_aux_dir)/mkinstalldirs

DISTCLEANFILES = \
    stamp-h \
    stamp-h[0-9]*

MAINTAINERCLEANFILES = \
    aclocal.m4 \
    config.h.in \
    configure \
    Makefile.in \
    $(AUX_DIST)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libmsat.pc

m4dir = $(datadir)/aclocal
m4_DATA = libmsat.m4

if HRIT
pkgconfig_DATA += libhrit.pc
m4_DATA += libhrit.m4
endif

if OPENMTP
pkgconfig_DATA += libopenmtp.pc
endif

## -fin-
