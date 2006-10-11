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

DIST_SUBDIRS = \
    decompress \
    examples \
    conv \
    iniparser \
    hdf5 \
    hri \
    hrit \
    msg-native \
    omtp-ids \
    openmtp \
    thornsds_db1

SUBDIRS = \
    grib \
    images \
    test

bin_PROGRAMS =
lib_LTLIBRARIES =
nobase_include_HEADERS =
noinst_HEADERS =
dist_noinst_DATA =
noinst_PROGRAMS =

if HRI
include $(srcdir)/hri/Automake.rules
endif

if HRIT
include $(srcdir)/decompress/Automake.rules
include $(srcdir)/hrit/Automake.rules
endif

if MSG_NATIVE
include $(srcdir)/msg-native/Automake.rules
endif

if OMTP_IDS
include $(srcdir)/omtp-ids/Automake.rules
endif

if OPENMTP
include $(srcdir)/openmtp/Automake.rules
endif

if THORNSDS_DB1
include $(srcdir)/thornsds_db1/Automake.rules
endif

if HAVE_NETCDF
include $(srcdir)/examples/Automake.rules
endif

if HDF5
include $(srcdir)/hdf5/Automake.rules
endif

include $(srcdir)/iniparser/Automake.rules

.PHONY: dist-rpm
dist-rpm: distcheck
	rpmbuild -ta $(distdir).tar.gz

EXTRA_DIST = \
    $(ac_aux_dir)/autogen.sh \
    $(ac_aux_dir)/bootstrap \
    $(ac_aux_dir)/clobber \
    $(PACKAGE_NAME).spec \
    CREDITS \
    PublicDecompWT_LICENSE

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

## -fin-
