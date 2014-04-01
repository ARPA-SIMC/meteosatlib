# Process this file with automake to produce Makefile.in.
#
# Copyright (C) 2005, 2006  Deneys S. Maartens  <dsm@tlabs.ac.za>
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

bin_PROGRAMS = msat

dist_bin_SCRIPTS = msat-view

INCLUDES = -I$(top_srcdir) -I$(top_builddir) $(GDAL_CFLAGS) $(MAGICKPP_CFLAGS) $(MSAT_CFLAGS)

msat_LDADD = $(GDAL_LIBS) $(MAGICKPP_LIBS) $(MSAT_LIBS) ../msat/libmsat.la
msat_SOURCES = msat.cpp

if HAVE_MAGICK
msat_SOURCES += image.cpp
endif

man_MANS = msat.1 msat-view.1

EXTRA_DIST = msat.1 msat-view.1 regenerate-msat-manpage image.h msat2gdal

# HRI
if HRI
if HAVE_NETCDF
bin_PROGRAMS += hri/HRI2NetCDF
hri_HRI2NetCDF_LDFLAGS = $(NETCDF_LIBS)
hri_HRI2NetCDF_LDADD = ../msat/libmsat.la
hri_HRI2NetCDF_CPPFLAGS = $(NETCDF_CFLAGS)
hri_HRI2NetCDF_SOURCES = hri/HRI2NetCDF.cpp
endif
endif

# HRIT
if HRIT

bin_PROGRAMS += hrit/xritdump
hrit_xritdump_LDADD = ../msat/libmsat.la
hrit_xritdump_SOURCES = hrit/xritdump.cpp

if HAVE_NETCDF
bin_PROGRAMS += hrit/XRIT2NetCDF
hrit_XRIT2NetCDF_CPPFLAGS = $(NETCDF_CFLAGS)
hrit_XRIT2NetCDF_LDADD = $(NETCDF_LIBS) ../msat/libmsat.la
hrit_XRIT2NetCDF_SOURCES = hrit/XRIT2NetCDF.cpp
endif

if HAVE_MAGICK
bin_PROGRAMS += hrit/XRIT2Image
hrit_XRIT2Image_CPPFLAGS = $(MAGICKPP_CFLAGS)
hrit_XRIT2Image_LDADD = $(MAGICKPP_LIBS) ../msat/libmsat.la
hrit_XRIT2Image_SOURCES = hrit/XRIT2Image.cpp
endif

endif

# MSG_NATIVE
if MSG_NATIVE
if HRIT

bin_PROGRAMS += msg-native/nativedump
msg_native_nativedump_LDADD = ../msat/libmsat.la
msg_native_nativedump_SOURCES = msg-native/nativedump.cpp

if HAVE_MAGICK
bin_PROGRAMS += msg-native/native2Image
msg_native_native2Image_CXXFLAGS = $(MAGICKPP_CFLAGS)
msg_native_native2Image_LDADD = $(MAGICKPP_LIBS) ../msat/libmsat.la
msg_native_native2Image_SOURCES = msg-native/native2Image.cpp
endif

endif
endif

# OMTP IDS
if OMTP_IDS
bin_PROGRAMS += \
    omtp-ids/OpenMTP-IDS_debug \
    omtp-ids/OpenMTP-IDS_sector \
    omtp-ids/OpenMTP-IDS_to_pgm \
    omtp-ids/OpenMTP-IDS_write

omtp_ids_OpenMTP_IDS_debug_LDADD = ../msat/libmsat.la
omtp_ids_OpenMTP_IDS_debug_SOURCES = omtp-ids/OpenMTP-IDS_debug.cc

omtp_ids_OpenMTP_IDS_sector_LDADD = ../msat/libmsat.la
omtp_ids_OpenMTP_IDS_sector_SOURCES = omtp-ids/OpenMTP-IDS_sector.cc

omtp_ids_OpenMTP_IDS_to_pgm_LDADD = ../msat/libmsat.la
omtp_ids_OpenMTP_IDS_to_pgm_SOURCES = omtp-ids/OpenMTP-IDS_to_pgm.cc

omtp_ids_OpenMTP_IDS_write_LDADD = ../msat/libmsat.la
omtp_ids_OpenMTP_IDS_write_SOURCES = omtp-ids/OpenMTP-IDS_write.cc
endif

# openmtp
if OPENMTP

bin_PROGRAMS += openmtp/OpenMTP_info
openmtp_OpenMTP_info_LDADD = ../msat/libmsat.la
openmtp_OpenMTP_info_SOURCES = openmtp/OpenMTP_info.cc

if HAVE_NETCDF
bin_PROGRAMS += openmtp/OpenMTP_to_NetCDF
openmtp_OpenMTP_to_NetCDF_LDADD = ../msat/libmsat.la $(NETCDF_LIBS)
openmtp_OpenMTP_to_NetCDF_CPPFLAGS = $(NETCDF_CFLAGS)
openmtp_OpenMTP_to_NetCDF_SOURCES = openmtp/OpenMTP_to_NetCDF.cpp
endif

endif

# thornsds_db1
if THORNSDS_DB1
bin_PROGRAMS += thornsds_db1/db1_to_netcdf
thornsds_db1_db1_to_netcdf_LDADD = ../msat/libmsat.la $(NETCDF_LIBS)
thornsds_db1_db1_to_netcdf_CPPFLAGS = $(NETCDF_CFLAGS)
thornsds_db1_db1_to_netcdf_SOURCES = thornsds_db1/db1_to_netcdf.cpp

if HAVE_MAGICK
bin_PROGRAMS += thornsds_db1/db1_to_image
thornsds_db1_db1_to_image_CPPFLAGS = $(MAGICKPP_CFLAGS)
thornsds_db1_db1_to_image_LDADD = ../msat/libmsat.la $(MAGICKPP_LIBS)
thornsds_db1_db1_to_image_SOURCES = thornsds_db1/db1_to_image.cpp
endif
endif
