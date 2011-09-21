# configure.ac -- Process this file with autoconf to produce a configure script.
# $Id$
# Copyright (C) 2004, 2005, 2006 Deneys S. Maartens <dsm@tlabs.ac.za>
# Modified in 2006--2011 by Enrico Zini <enrico@enricozini.org>
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

AC_INIT([meteosatlib], [0.6.2], [enrico@enricozini.org, giuliani@lamma.rete.toscana.it])

AC_REVISION([$Revision$])

AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_MACRO_DIR([m4])
AC_SUBST([ac_aux_dir])

AM_INIT_AUTOMAKE([subdir-objects])

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
], [
    if test "$enable_hrit" == "yes"; then
        enable_hrit="no"
        AC_MSG_WARN("*** hrit support disabled for lack of libmagick++")
    fi
    if test "$enable_thornsds_db1" == "yes"; then
        enable_thornsds_db1="no"
        AC_MSG_WARN("*** thornsds_db1 support disabled for lack of libmagick++")
    fi
])
PKG_CHECK_MODULES(MAGICKPP,ImageMagick++)

dnl NetCDF3 (which does not use pkg-config)
dnl AC_CHECK_NETCDF([have_libnetcdf=yes],[have_libnetcdf=no])
PKG_CHECK_MODULES(NETCDF,[netcdf],[have_libnetcdf=yes],[have_libnetcdf=no])

dnl --------------------------------------------------------------------
dnl Checks for header files.

if test "x$have_libnetcdf" == "xyes"; then
    AC_DEFINE([HAVE_NETCDF], 1, [NetCDF functions are available])
    AC_DEFINE([MSAT_HAVE_NETCDF], 1, [NetCDF functions are available])
    NETCDF_LIBS="$NETCDF_LIBS -lnetcdf_c++"
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
	AC_DEFINE([MSAT_HAVE_HRIT], 1, [XRIT functions are available])
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

AM_CONDITIONAL([HRI], [test x"$enable_hri" = x"yes"])
AM_CONDITIONAL([HRIT], [test x"$enable_hrit" = x"yes"])
AM_CONDITIONAL([MSG_NATIVE], [test x"$enable_msg_native" = x"yes"])
AM_CONDITIONAL([OMTP_IDS], [test x"$enable_omtp_ids" = x"yes"])
AM_CONDITIONAL([OPENMTP], [test x"$enable_openmtp" = x"yes"])
AM_CONDITIONAL([THORNSDS_DB1], [test x"$enable_thornsds_db1" = x"yes"])
AM_CONDITIONAL([HAVE_NETCDF], [test x"$have_libnetcdf" = x"yes"])
AM_CONDITIONAL([HAVE_MAGICK], [test x"$have_libmagick" = x"yes"])

dnl --------------------------------------------------------------------
dnl Extra defines

dnl Extra warnings are good
CXXFLAGS="-Wall $CXXFLAGS"

dnl --------------------------------------------------------------------
dnl Substitution variables for the .pc.in files

if test x"$enable_hrit" = x"yes"; then
	PC_MSAT_REQS="$PC_MSAT_REQS libhrit"
	PC_MSAT_LIBS="$PC_MSAT_LIBS -lhrit"
fi

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
    libmsat.pc
    libhrit.pc
    libopenmtp.pc
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
===================================================])

## -fin-
