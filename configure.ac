# configure.ac -- Process this file with autoconf to produce a configure script.
# $Id$
# Copyright (C) 2004, 2005, 2006 Deneys S. Maartens <dsm@tlabs.ac.za>
# Modified in 2006, 2007 by Enrico Zini <enrico@enricozini.org>
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

m4_pattern_allow

AC_INIT([meteosatlib], [0.5], [enrico@enricozini.org, giuliani@lamma.rete.toscana.it])

AC_REVISION([$Revision$])

AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_AUX_DIR([config])
AC_SUBST([ac_aux_dir])

AM_INIT_AUTOMAKE([subdir-objects])

dnl create a config.h file (Automake will add -DHAVE_CONFIG_H)
AM_CONFIG_HEADER([config.h])

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
AC_ARG_ENABLE([hdf5],
    [AS_HELP_STRING(
        [--disable-hdf5],
        [disable hdf5 support])],
    [],
    [enable_hdf5="yes"])

dnl --------------------------------------------------------------------
dnl Checks for programs.

AC_PROG_CXX
AM_PROG_CC_C_O
AC_PROG_LIBTOOL
AC_LANG_CPLUSPLUS

dnl --------------------------------------------------------------------
dnl Checks for libraries.

if test -d /usr/include/netcdf-3 ; then
    CPPFLAGS="$CPPFLAGS -I/usr/include/netcdf-3"
fi
if test -d /usr/lib/netcdf-3 ; then
    LDFLAGS="$LDFLAGS -L/usr/lib/netcdf-3"
fi

AC_CHECK_LIB([netcdf], [ncopts], [have_libnetcdf=yes])

saved_LIBS="$LIBS"
LIBS="$LIBS -lnetcdf_c++ -lnetcdf"
AC_MSG_CHECKING([for NcFile in -lnetcdf_c++])
AC_LINK_IFELSE(
    [AC_LANG_PROGRAM(
        [#include <netcdfcpp.h>],
        [NcFile::NcFile nc("example.nc")])],
    [AC_MSG_RESULT([yes])
    have_libnetcdf_c__=yes],
    [AC_MSG_RESULT([no])])
LIBS="$savedLIBS"

AC_CHECK_LIB([Magick++], [ReadImage], [have_libmagick=yes])

AC_CHECK_LIB([hdf5], [H5_init_library], [have_libhdf5=yes])
dnl AC_CHECK_LIB([hdf5_cpp], [Exception], [have_libhdf5cpp=yes])

dnl --------------------------------------------------------------------
dnl Checks for header files.

AC_CHECK_HEADER([Magick++.h],
    [
     	AC_DEFINE([HAVE_MAGICKPP], 1, [Magick++ functions are available])
    ],
    [
    if test "$enable_hrit" == "yes"; then
        enable_hrit="no"
        AC_MSG_WARN("*** hrit support disabled")
    fi
    if test "$enable_thornsds_db1" == "yes"; then
        enable_thornsds_db1="no"
        AC_MSG_WARN("*** thornsds_db1 support disabled")
    fi
])

AC_CHECK_HEADER([netcdfcpp.h],
    [
     	AC_DEFINE([HAVE_NETCDF], 1, [NetCDF functions are available])
    ],
    [
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
])

AC_CHECK_HEADER([H5Cpp.h],
    [],
    [
    if test "$enable_hdf5" == "yes"; then
        enable_hdf5="no"
        AC_MSG_WARN("*** hdf5 support disabled")
    fi
])

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
        if ! test x"$have_libnetcdf_c__" = x"yes"; then
            enable_hri="no"
            AC_MSG_WARN("*** hri support disabled")
        fi
    fi
fi

if test x"$enable_hrit" == x"yes"; then
    if test x"$have_libmagick" = x"yes"; then
        if ! test x"$have_libnetcdf" = x"yes" ; then
            enable_hrit="no"
            AC_MSG_WARN("*** hrit support disabled")
        fi
    else
        enable_hrit="no"
        AC_MSG_WARN("*** hrit support disabled")
    fi
    if test x"$enable_hrit" == x"yes"; then
        make_hrit=hrit/Subdir.mk
        make_decompress=decompress/Subdir.mk
	AC_DEFINE([HAVE_HRIT], 1, [XRIT functions are available])
    fi
fi

if test x"$enable_thornsds_db1" == x"yes"; then
    if test x"$have_libmagick" = x"yes"; then
        if ! test x"$have_libnetcdf" = x"yes" ; then
            enable_thornsds_db1="no"
            AC_MSG_WARN("*** thornsds_db1 support disabled")
        fi
    else
        enable_thornsds_db1="no"
        AC_MSG_WARN("*** thornsds_db1 support disabled")
    fi
    if test x"$enable_thornsds_db1" == x"yes"; then
        make_thornsds_db1=thornsds_db1/Subdir.mk
        make_decompress=decompress/Subdir.mk
    fi
fi

if test x"$enable_hdf5" == x"yes"; then
    if ! test x"$have_libhdf5" = x"yes"; then
        enable_hdf5="no"
        AC_MSG_WARN("*** hdf5 support disabled")
    else
	AC_DEFINE([HAVE_HDF5], 1, [HDF5 functions are available])
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
AM_CONDITIONAL([HDF5], [test x"$enable_hdf5" = x"yes"])

dnl --------------------------------------------------------------------
dnl Extra defines

dnl Extra warnings are good
CXXFLAGS="-Wall $CXXFLAGS"

dnl --------------------------------------------------------------------
dnl Create output files.

AC_CONFIG_FILES([
    Makefile
    decompress/Makefile
    examples/Makefile
    msat/Makefile
    grib/Makefile
    iniparser/Makefile
    images/Makefile
    hri/Makefile
    hri/examples/Makefile
    hrit/Makefile
    hrit/examples/Makefile
    msg-native/Makefile
    msg-native/examples/Makefile
    omtp-ids/Makefile
    omtp-ids/examples/Makefile
    openmtp/Makefile
    openmtp/examples/Makefile
    thornsds_db1/Makefile
    thornsds_db1/examples/Makefile
    tests/Makefile
    tools/Makefile
    meteosatlib.spec
    meteosatlib.pc
])
AC_OUTPUT

AC_MSG_NOTICE([
===================================================
$PACKAGE_NAME-$PACKAGE_VERSION configuration:
AS_HELP_STRING([hdf5:], [$enable_hdf5])
AS_HELP_STRING([hri:], [$enable_hri])
AS_HELP_STRING([hrit:], [$enable_hrit])
AS_HELP_STRING([msg-native:], [$enable_msg_native])
AS_HELP_STRING([omtp-ids:], [$enable_omtp_ids])
AS_HELP_STRING([openmtp:], [$enable_openmtp])
AS_HELP_STRING([thornsds_db1:], [$enable_thornsds_db1])
===================================================])

## -fin-
