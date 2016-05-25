# Process this file with automake to produce Makefile.in.

check_PROGRAMS = msat_test

dist_check_SCRIPTS = runtest

TESTS_ENVIRONMENT = $(srcdir)/runtest

check-local:
	for test in $(check_PROGRAMS); do \
		$(TESTS_ENVIRONMENT) $$test ; \
	done

dist_noinst_HEADERS = \
    gdal/utils.h

msat_test_CPPFLAGS = \
    -I$(top_srcdir) -I$(top_builddir) \
    -DDATA_DIR=\"`pwd`/$(top_srcdir)/tests/data\" \
    -DWORK_DIR=\"`pwd`/$(top_builddir)/tests/work/\" \
    $(GDAL_CFLAGS) $(MSAT_CFLAGS) $(NETCDF_CFLAGS)
msat_test_LDADD = ../msat/libmsat.la
msat_test_LDFLAGS =

msat_test_SOURCES = \
    msat/test-facts.cpp \
    tests-main.cc

if HRIT
msat_test_SOURCES += \
    msat/test-fileaccess.cpp \
    msat/test-dataaccess.cpp
endif

if HAVE_GDAL
msat_test_SOURCES += \
    gdal/utils.cc \
    gdal/test-georef-grib.cpp \
    gdal/test-georef-netcdf.cpp \
    gdal/test-georef-xrit.cpp \
    gdal/test-georef-xrithrv.cpp \
    gdal/test-importgrib.cpp \
    gdal/test-importnetcdf.cpp \
    gdal/test-importxrit.cpp \
    gdal/test-importxrit-rss.cpp \
    gdal/test-importxrit-nonlinear.cpp \
    gdal/test-importxrithrv.cpp \
    gdal/test-importxrit-rsshrv.cpp \
    gdal/test-xrit-reflectance.cpp \
    gdal/test-xrit-solar-za.cpp

msat_test_LDFLAGS += $(GDAL_LIBS) $(NETCDF_LIBS)
endif

EXTRA_DIST = \
    data/H-000-MSG1__-MSG1________-_________-EPI______-200611130800-__ \
    data/H-000-MSG1__-MSG1________-_________-EPI______-200611141200-__ \
    data/H-000-MSG1__-MSG1________-HRV______-000018___-200611141200-C_ \
    data/H-000-MSG1__-MSG1________-IR_039___-000001___-200611130800-C_ \
    data/H-000-MSG1__-MSG1________-_________-PRO______-200611130800-__ \
    data/H-000-MSG1__-MSG1________-_________-PRO______-200611141200-__ \
    data/H-000-MSG2__-MSG2________-_________-EPI______-200807150900-__ \
    data/H-000-MSG2__-MSG2________-_________-PRO______-200807150900-__ \
    data/H-000-MSG2__-MSG2________-VIS006___-000001___-200807150900-C_ \
    data/H-000-MSG2__-MSG2________-_________-PRO______-201001191200-__ \
    data/H-000-MSG2__-MSG2________-_________-EPI______-201001191200-__ \
    data/H-000-MSG2__-MSG2________-IR_039___-000008___-201001191200-C_ \
    data/H-000-MSG2__-MSG2________-IR_108___-000008___-201001191200-C_ \
    data/H-000-MSG2__-MSG2________-IR_134___-000008___-201001191200-C_ \
    data/MSG_Seviri_1_5_Infrared_10_8_channel_20051219_1415.nc \
    data/MSG_Seviri_1_5_Infrared_9_7_channel_20060426_1945.grb \
    data/rss/H-000-MSG2__-MSG2_RSS____-_________-PRO______-201604281230-__ \
    data/rss/H-000-MSG2__-MSG2_RSS____-HRV______-000024___-201604281230-C_ \
    data/rss/H-000-MSG2__-MSG2_RSS____-IR_039___-000008___-201604281230-C_ \
    data/rss/H-000-MSG2__-MSG2_RSS____-VIS006___-000008___-201604281230-C_ \
    data/rss/H-000-MSG2__-MSG2_RSS____-_________-EPI______-201604281230-__
