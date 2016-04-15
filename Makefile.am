# Makefile.am -- Process this file with automake to produce Makefile.in

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
