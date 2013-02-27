# configure.ac -- Process this file with autoconf to produce a configure script.
#
# Copyright (C) 2004, 2005, 2006 Deneys S. Maartens <dsm@tlabs.ac.za>
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

AC_INIT([meteosatlib], [1.2], [enrico@enricozini.org])
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_AUX_DIR([config])
AM_INIT_AUTOMAKE([foreign subdir-objects nostdinc])
AC_CONFIG_MACRO_DIR([m4])

dnl create a config.h file (Automake will add -DHAVE_CONFIG_H)
AM_CONFIG_HEADER([config.h msat/config.h])

dnl --------------------------------------------------------------------
dnl Compile time options.

AC_ARG_ENABLE([hri],
    [AS_HELP_STRING(
        [--disable-hri],
        [disable hri support])],
    [],
    [enable_hri="yes"])
AC_ARG_ENABLE([hrit],
    [AS_HELP_STRING(
        [--disable-hrit],
        [disable hrit support])],
    [],
    [enable_hrit="yes"])
AC_ARG_ENABLE([thornsds_db1],
    [AS_HELP_STRING(
        [--disable-thornsds_db1],
        [disable thornsds_db1 support])],
    [],
    [enable_thornsds_db1="yes"])
AC_ARG_ENABLE([msg-native],
    [AS_HELP_STRING(
        [--disable-msg-native],
        [disable msg-native support])],
    [],
    [enable_msg_native="yes"])
AC_ARG_ENABLE([omtp-ids],
    [AS_HELP_STRING(
        [--disable-omtp-ids],
        [disable omtp-ids support])],
    [],
    [enable_omtp_ids="yes"])
AC_ARG_ENABLE([openmtp],
    [AS_HELP_STRING(
        [--disable-openmtp],
        [disable openmtp support])],
    [],
    [enable_openmtp="yes"])

dnl --------------------------------------------------------------------
dnl Checks for programs.

AC_PROG_CXX
AM_PROG_CC_C_O
AC_PROG_LIBTOOL
AC_LANG_CPLUSPLUS

dnl --------------------------------------------------------------------
dnl Checks for libraries.

PKG_PROG_PKG_CONFIG()

dnl ImageMagick++
PKG_CHECK_EXISTS(ImageMagick++,[
    have_libmagick=yes
    AC_DEFINE([HAVE_MAGICKPP], 1, [Magick++ functions are available])
    AC_DEFINE([MSAT_HAVE_MAGICKPP], 1, [Magick++ functions are available])
    PKG_CHECK_MODULES(MAGICKPP,ImageMagick++)
], [
    have_libmagick=no
    if test "$enable_hrit" == "yes" -o "$enable_thornsds_db1" == "yes"; then
        AC_MSG_WARN("*** libmagick++ not found: some features will be disabled")
    fi
])

dnl NetCDF3 (which does not use pkg-config, or not reliably)
AC_CHECK_NETCDF([have_libnetcdf=yes],[have_libnetcdf=no])
if test "x$have_libnetcdf" == "xyes"; then
    AC_DEFINE([HAVE_NETCDF], 1, [NetCDF functions are available])
    AC_DEFINE([MSAT_HAVE_NETCDF], 1, [NetCDF functions are available])
    AC_SUBST(NETCDF_CFLAGS)
    AC_SUBST(NETCDF_LIBS)
else
    if test "$enable_hri" == "yes"; then
        enable_hri="no"
        AC_MSG_WARN("*** hri support disabled")
    fi
    if test "$enable_hrit" == "yes"; then
        enable_hrit="no"
        AC_MSG_WARN("*** hrit support disabled")
    fi
    if test "$enable_thornsds_db1" == "yes"; then
        enable_thornsds_db1="no"
        AC_MSG_WARN("*** thornsds_db1 support disabled")
    fi
fi

dnl GDAL (which does not use pkg-config)
AC_PATH_PROG([GDAL_CONFIG], [gdal-config], [NONE])
if test "x$GDAL_CONFIG" == "xNONE"; then
    have_gdal="no"
    AC_MSG_WARN("*** GDAL support disabled")
else
    have_gdal="yes"
    AC_DEFINE([HAVE_GDAL], 1, [GDAL functions are available])
    AC_DEFINE([MSAT_HAVE_GDAL], 1, [GDAL functions are available])
    AC_SUBST(GDAL_CONFIG)
    GDAL_CFLAGS=`$GDAL_CONFIG --cflags`
    GDAL_LIBS=`$GDAL_CONFIG --libs`
    AC_SUBST(GDAL_CFLAGS)
    AC_SUBST(GDAL_LIBS)
    echo "GPD $GDAL_PLUGIN_DIRNAME"
    if test -z "$GDAL_PLUGIN_DIRNAME"; then
        GDAL_PLUGIN_DIRNAME=gdalplugins/$(gdal-config --version | sed -re ['s/([0-9]+\.[0-9]+).+/\1/'])
    fi
    echo "GPD $GDAL_PLUGIN_DIRNAME"
    AC_MSG_NOTICE([GDAL plugin directory is $GDAL_PLUGIN_DIRNAME])
    AC_SUBST(GDAL_PLUGIN_DIRNAME)
fi

dnl grib_api (which does not use pkg-config)
have_gribapi=yes
AC_CHECK_LIB([grib_api], [grib_get_api_version], [true], [have_gribapi=no], [-ljasper])
AC_CHECK_HEADER([grib_api.h], [true], [have_gribapi=no])
if test x"$have_gribapi" = xyes
then
	AC_DEFINE([HAVE_GRIBAPI], 1, [grib_api functions are available])
	GRIBAPI_CFLAGS=
	GRIBAPI_LIBS="-lgrib_api -ljasper"
	AC_SUBST(GRIBAPI_CFLAGS)
	AC_SUBST(GRIBAPI_LIBS)
else
	have_gribapi="no"
	AC_MSG_WARN("*** GRIB support disabled")
fi
enable_grib=$have_gribapi

dnl HDF5 (which does not use pkg-config)
have_hdf5=yes
AC_CHECK_LIB([hdf5], [H5Fis_hdf5], [], [have_hdf5=no])
AC_CHECK_HEADER([hdf5.h], [], [have_hdf5=no])
if test x"$enable_hdf5" = x"no"; then
	AC_MSG_WARN("*** hdf5 support disabled")
else
	AC_DEFINE([HAVE_HDF5], 1, [HDF5 functions are available])
	HDF5_CFLAGS=""
	HDF5_LIBS="-lhdf5"
	AC_SUBST(HDF5_CFLAGS)
	AC_SUBST(HDF5_LIBS)
fi
enable_safh5=$have_hdf5


dnl --------------------------------------------------------------------
dnl Checks for typedefs, structures, and compiler characteristics.

dnl --------------------------------------------------------------------
dnl Checks for library functions.

dnl --------------------------------------------------------------------
dnl Conditional stuff.

if test x"$enable_hri" == x"yes"; then
    if ! test x"$have_libnetcdf" = x"yes"; then
        enable_hri="no"
        AC_MSG_WARN("*** hri support disabled")
    else
        if ! test x"$have_libnetcdf" = x"yes"; then
            enable_hri="no"
            AC_MSG_WARN("*** hri support disabled")
        fi
    fi
fi

if test x"$enable_hrit" == x"yes"; then
    if ! test x"$have_libnetcdf" = x"yes" ; then
        enable_hrit="no"
        AC_MSG_WARN("*** hrit support disabled")
    fi
    if test x"$enable_hrit" == x"yes"; then
        make_hrit=hrit/Subdir.mk
        make_decompress=decompress/Subdir.mk
	AC_DEFINE([HAVE_HRIT], 1, [XRIT functions are available])
	AC_DEFINE([MSAT_HAVE_HRIT], 1, [XRIT functions are available])
    fi
fi

if test x"$enable_thornsds_db1" == x"yes"; then
    if ! test x"$have_libnetcdf" = x"yes" ; then
        enable_thornsds_db1="no"
        AC_MSG_WARN("*** thornsds_db1 support disabled")
    fi
    if test x"$enable_thornsds_db1" == x"yes"; then
        make_thornsds_db1=thornsds_db1/Subdir.mk
        make_decompress=decompress/Subdir.mk
    fi
fi

AM_CONDITIONAL([HRI], [test x"$enable_hri" = x"yes"])
AM_CONDITIONAL([HRIT], [test x"$enable_hrit" = x"yes"])
AM_CONDITIONAL([MSG_NATIVE], [test x"$enable_msg_native" = x"yes"])
AM_CONDITIONAL([OMTP_IDS], [test x"$enable_omtp_ids" = x"yes"])
AM_CONDITIONAL([OPENMTP], [test x"$enable_openmtp" = x"yes"])
AM_CONDITIONAL([THORNSDS_DB1], [test x"$enable_thornsds_db1" = x"yes"])
AM_CONDITIONAL([HAVE_NETCDF], [test x"$have_libnetcdf" = x"yes"])
AM_CONDITIONAL([HAVE_MAGICK], [test x"$have_libmagick" = x"yes"])
AM_CONDITIONAL([HAVE_GRIBAPI], [test x"$have_gribapi" = x"yes"])
AM_CONDITIONAL([HAVE_GDAL], [test x"$have_gdal" = x"yes"])
AM_CONDITIONAL([HAVE_HDF5], [test x"$have_hdf5" = x"yes"])
AM_CONDITIONAL([OLD_GRIB], [test x"$enable_grib" = x"yes"])

AC_SUBST(enable_hri)
AC_SUBST(enable_hrit)
AC_SUBST(enable_msg_native)
AC_SUBST(enable_omtp_ids)
AC_SUBST(enable_openmtp)
AC_SUBST(enable_thornsds_db1)
AC_SUBST(enable_grib)
AC_SUBST(enable_safh5)

dnl --------------------------------------------------------------------
dnl Extra defines

dnl Extra warnings are good
CXXFLAGS="-Wall $CXXFLAGS"

dnl --------------------------------------------------------------------
dnl Substitution variables for the .pc.in files

if test x"$have_libnetcdf" = x"yes"; then
	# FIXME: there seems to be no way to tell users of the .pc.in that they
	# need the netcdf libraries, as netcdf does not package .pc files.  Are
	# there better ways of handling this?
	PC_MSAT_LIBS="$PC_MSAT_LIBS -lnetcdf_c++ -lnetcdf"
fi

if test x"$have_libmagick" = x"yes"; then
	PC_MSAT_REQS="$PC_MSAT_REQS ImageMagick++"
fi

AC_SUBST(PC_MSAT_REQS)
AC_SUBST(PC_MSAT_LIBS)

dnl Default institution information required when generating NetCDF
if test "x$MSAT_DEFAULT_INSTITUTION" == "x"; then
    MSAT_DEFAULT_INSTITUTION="ARPA-SIM"
fi
AC_DEFINE_UNQUOTED([MSAT_DEFAULT_INSTITUTION], ["$MSAT_DEFAULT_INSTITUTION"], [Default institution name])
AC_MSG_NOTICE([Default institution is $MSAT_DEFAULT_INSTITUTION])


dnl --------------------------------------------------------------------
dnl Create output files.

AC_CONFIG_FILES([
    Makefile
    decompress/Makefile
    msat/Makefile
    gdal/Makefile
    tools/Makefile
    examples/Makefile
    tests/Makefile
    libmsat.pc
    meteosatlib.spec
])
AC_OUTPUT

AC_MSG_NOTICE([
===================================================
$PACKAGE_NAME-$PACKAGE_VERSION configuration:
AS_HELP_STRING([hri:], [$enable_hri])
AS_HELP_STRING([hrit:], [$enable_hrit])
AS_HELP_STRING([msg-native:], [$enable_msg_native])
AS_HELP_STRING([omtp-ids:], [$enable_omtp_ids])
AS_HELP_STRING([openmtp:], [$enable_openmtp])
AS_HELP_STRING([thornsds_db1:], [$enable_thornsds_db1])
AS_HELP_STRING([magick++:], [$have_libmagick])
AS_HELP_STRING([GDAL:], [$have_gdal])
AS_HELP_STRING([grib_api:], [$have_gribapi])
AS_HELP_STRING([hdf5:], [$have_hdf5])
===================================================])
