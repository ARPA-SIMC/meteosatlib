# Makefile.am -- Process this file with automake to produce Makefile.in
#
# Copyright (C) 2005 Deneys S. Maartens <dsm@tlabs.ac.za>
# Copyright (C) 2006--2012  Enrico Zini <enrico@enricozini.org>
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

ACLOCAL_AMFLAGS = -I m4

SUBDIRS =
if HRIT
SUBDIRS += decompress
endif
SUBDIRS += msat
if HAVE_GDAL
SUBDIRS += gdal
endif
SUBDIRS += tools examples tests

.PHONY: dist-rpm
dist-rpm: distcheck
	rpmbuild -ta $(distdir).tar.gz

EXTRA_DIST = \
    config/autogen.sh \
    config/clobber \
    $(PACKAGE_NAME).spec \
    AUTHORS \
    COPYING \
    ChangeLog \
    INSTALL \
    PublicDecompWT_LICENSE \
    README \
    m4/netcdf.m4 \
    libmsat.pc.in \
    meteosatlib.spec.in \
    run-local

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libmsat.pc
