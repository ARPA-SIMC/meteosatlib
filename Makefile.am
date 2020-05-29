# Makefile.am -- Process this file with automake to produce Makefile.in

ACLOCAL_AMFLAGS = -I m4

SUBDIRS =
if HRIT
if BUNDLED_PDWT
SUBDIRS += decompress
endif
endif
SUBDIRS += msat
if HAVE_GDAL
SUBDIRS += gdal
endif
SUBDIRS += tools examples tests

EXTRA_DIST = \
    config/autogen.sh \
    config/clobber \
    AUTHORS \
    LICENSE \
    ChangeLog \
    INSTALL \
    README.md \
    m4/netcdf.m4 \
    libmsat.pc.in \
    run-local

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libmsat.pc
