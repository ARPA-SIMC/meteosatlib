# Makefile.am -- Process this file with automake to produce Makefile.in

noinst_HEADERS =

msat_includedir = $(includedir)/msat
dist_msat_include_HEADERS = \
    config.h \
    iniparser.h \
    Progress.h \
    auto_arr_ptr.h \
    facts.h \
    utils/string.h \
    utils/subprocess.h \
    utils/sys.h \
    utils/term.h \
    utils/testrunner.h \
    utils/tests.h

lib_LTLIBRARIES = libmsat.la

# CURRENT:REVISION:AGE
# - library source changed -> inc REVISION
# - interfaces added/removed/changed -> inc CURRENT, REVISION = 0
# - interfaces added -> inc AGE
# - interfaces removed -> AGE = 0
libmsat_la_CPPFLAGS = -I$(top_srcdir) -I$(top_builddir)
libmsat_la_CXXFLAGS =
libmsat_la_LIBADD =
libmsat_la_LDFLAGS = -version-info 1:0:0

# Common code
libmsat_la_SOURCES = \
    iniparser.c \
    Progress.cpp \
    auto_arr_ptr.cpp \
    facts.cpp \
    utils/string.cc \
    utils/subprocess.cc \
    utils/sys.cc \
    utils/term.cc \
    utils/testrunner.cc \
    utils/tests.cc

if HAVE_GDAL
gdal_includedir = $(msat_includedir)/gdal

dist_gdal_include_HEADERS = \
    gdal/clean_cpl_error.h \
    gdal/clean_gdal_priv.h \
    gdal/const.h \
    gdal/points.h \
    gdal/dataset.h \
    gdal/gdaltranslate.h

libmsat_la_SOURCES += \
    gdal/dataset.cpp \
    gdal/gdaltranslate.cpp

libmsat_la_CPPFLAGS += $(GDAL_CFLAGS)
libmsat_la_LIBADD += $(GDAL_LIBS)
endif

if HRI
hri_includedir = $(msat_includedir)/hri

dist_hri_include_HEADERS = \
    hri/HRI.h \
    hri/HRI_geolocation.h \
    hri/HRI_image.h \
    hri/HRI_machine.h \
    hri/HRI_subframe_identification.h \
    hri/HRI_subframe_interpretation.h \
    hri/HRI_subframe_keyslot.h \
    hri/HRI_subframe_label.h

libmsat_la_SOURCES += \
    hri/HRI.cpp \
    hri/HRI_image.cpp \
    hri/HRI_machine.cpp \
    hri/HRI_subframe_identification.cpp \
    hri/HRI_subframe_interpretation.cpp \
    hri/HRI_subframe_keyslot.cpp \
    hri/HRI_subframe_label.cpp

libmsat_la_CPPFLAGS += $(NETCDF_CFLAGS)
libmsat_la_LIBADD += $(NETCDF_LIBS)
endif

if HRIT
hrit_includedir = $(msat_includedir)/hrit

hrit_include_HEADERS = \
    hrit/MSG_channel.h \
    hrit/MSG_compression.h \
    hrit/MSG_data_CelestialEvents.h \
    hrit/MSG_data_format.h \
    hrit/MSG_data_GeometricProc.h \
    hrit/MSG_data_GeometricQuality.h \
    hrit/MSG_data_gts.h \
    hrit/MSG_data.h \
    hrit/MSG_data_ImageAcquisition.h \
    hrit/MSG_data_ImageDescription.h \
    hrit/MSG_data_image.h \
    hrit/MSG_data_image_mpef.h \
    hrit/MSG_data_ImageProdStats.h \
    hrit/MSG_data_IMPFConfiguration.h \
    hrit/MSG_data_key.h \
    hrit/MSG_data_NavigExtrResult.h \
    hrit/MSG_data_RadiometricProc.h \
    hrit/MSG_data_RadiometricQuality.h \
    hrit/MSG_data_SatelliteStatus.h \
    hrit/MSG_data_SGS_header.h \
    hrit/MSG_data_text.h \
    hrit/MSG_data_TimelinComple.h \
    hrit/MSG_filetype.h \
    hrit/MSG_header_ancillary_text.h \
    hrit/MSG_header_annotation.h \
    hrit/MSG_header.h \
    hrit/MSG_header_image_datafunc.h \
    hrit/MSG_header_image_navig.h \
    hrit/MSG_header_image_struct.h \
    hrit/MSG_header_key.h \
    hrit/MSG_header_segment_id.h \
    hrit/MSG_header_segment_quality.h \
    hrit/MSG_header_timestamp.h \
    hrit/MSG_HRIT.h \
    hrit/MSG_hrit_specdoc.h \
    hrit/MSG_machine.h \
    hrit/MSG_projection.h \
    hrit/MSG_quality.h \
    hrit/MSG_spacecraft.h \
    hrit/MSG_time_cds.h \
    xrit/dataaccess.h \
    xrit/fileaccess.h

libmsat_la_SOURCES += \
    hrit/MSG_channel.cpp \
    hrit/MSG_compression.cpp \
    hrit/MSG_data_CelestialEvents.cpp \
    hrit/MSG_data.cpp \
    hrit/MSG_data_format.cpp \
    hrit/MSG_data_GeometricProc.cpp \
    hrit/MSG_data_GeometricQuality.cpp \
    hrit/MSG_data_gts.cpp \
    hrit/MSG_data_ImageAcquisition.cpp \
    hrit/MSG_data_image.cpp \
    hrit/MSG_data_ImageDescription.cpp \
    hrit/MSG_data_ImageProdStats.cpp \
    hrit/MSG_data_IMPFConfiguration.cpp \
    hrit/MSG_data_key.cpp \
    hrit/MSG_data_NavigExtrResult.cpp \
    hrit/MSG_data_RadiometricProc.cpp \
    hrit/MSG_data_RadiometricQuality.cpp \
    hrit/MSG_data_SatelliteStatus.cpp \
    hrit/MSG_data_SGS_header.cpp \
    hrit/MSG_data_text.cpp \
    hrit/MSG_data_TimelinComple.cpp \
    hrit/MSG_filetype.cpp \
    hrit/MSG_header_ancillary_text.cpp \
    hrit/MSG_header_annotation.cpp \
    hrit/MSG_header.cpp \
    hrit/MSG_header_image_datafunc.cpp \
    hrit/MSG_header_image_navig.cpp \
    hrit/MSG_header_image_struct.cpp \
    hrit/MSG_header_key.cpp \
    hrit/MSG_header_segment_id.cpp \
    hrit/MSG_header_segment_quality.cpp \
    hrit/MSG_header_timestamp.cpp \
    hrit/MSG_HRIT.cpp \
    hrit/MSG_machine.cpp \
    hrit/MSG_quality.cpp \
    hrit/MSG_spacecraft.cpp \
    hrit/MSG_time_cds.cpp \
    xrit/dataaccess.cpp \
    xrit/fileaccess.cpp

if BUNDLED_PDWT
libmsat_la_CPPFLAGS += \
    -I$(top_builddir)/decompress/PublicDecompWT/COMP/Inc \
    -I$(top_builddir)/decompress/PublicDecompWT/COMP/JPEG/Inc \
    -I$(top_builddir)/decompress/PublicDecompWT/COMP/T4/Inc \
    -I$(top_builddir)/decompress/PublicDecompWT/COMP/WT/Inc \
    -I$(top_builddir)/decompress/PublicDecompWT/DISE

libmsat_la_LIBADD += \
    $(top_builddir)/decompress/libpublicdecompwt.la
else
libmsat_la_CPPFLAGS += $(PDWT_CFLAGS)
libmsat_la_LIBADD += $(PDWT_LIBS)
endif

endif

if MSG_NATIVE
msg_native_includedir = $(msat_includedir)/msg-native
msg_native_include_HEADERS = \
    msg-native/MSG_native.h \
    msg-native/MSG_native_packetheader.h \
    msg-native/MSG_native_header.h \
    msg-native/MSG_native_trailer.h \
    msg-native/MSG_native_line.h

libmsat_la_SOURCES += \
    msg-native/MSG_native.cpp \
    msg-native/MSG_native_packetheader.cpp \
    msg-native/MSG_native_header.cpp \
    msg-native/MSG_native_trailer.cpp \
    msg-native/MSG_native_line.cpp
endif

if OMTP_IDS
omtp_ids_includedir = $(msat_includedir)/omtp-ids
omtp_ids_include_HEADERS = \
    omtp-ids/Constants.hh \
    omtp-ids/FileHeader.hh \
    omtp-ids/LineHeader.hh \
    omtp-ids/OpenMTP-IDS.hh \
    omtp-ids/RecordHeader.hh \
    omtp-ids/Record.hh \
    omtp-ids/ScanLine.hh

noinst_HEADERS += \
    omtp-ids/ByteSex.hh \
    omtp-ids/sysdep.h

libmsat_la_SOURCES += \
    omtp-ids/ByteSex.cc \
    omtp-ids/FileHeader.cc \
    omtp-ids/LineHeader.cc \
    omtp-ids/OpenMTP-IDS.cc \
    omtp-ids/Record.cc \
    omtp-ids/RecordHeader.cc \
    omtp-ids/ScanLine.cc
endif

if OPENMTP
openmtp_includedir = $(msat_includedir)/openmtp
openmtp_include_HEADERS = \
    openmtp/OpenMTP_ascii_header.h \
    openmtp/OpenMTP_binary_header.h \
    openmtp/OpenMTP.h \
    openmtp/OpenMTP_image.h \
    openmtp/OpenMTP_image_line.h \
    openmtp/OpenMTP_machine.h

libmsat_la_SOURCES += \
    openmtp/OpenMTP_ascii_header.cpp \
    openmtp/OpenMTP_binary_header.cpp \
    openmtp/OpenMTP.cpp \
    openmtp/OpenMTP_image.cpp \
    openmtp/OpenMTP_image_line.cpp \
    openmtp/OpenMTP_machine.cpp
endif

if THORNSDS_DB1
thornsds_db1_includedir = $(msat_includedir)/thornsds_db1
thornsds_db1_include_HEADERS = \
    thornsds_db1/thornsds_db1.h

libmsat_la_SOURCES += \
    thornsds_db1/thornsds_db1.cpp
endif
