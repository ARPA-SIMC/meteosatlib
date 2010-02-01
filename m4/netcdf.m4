AC_DEFUN([AC_CHECK_NETCDF],
[
dnl Look for NetCDF3 (which does not use pkg-config)

dnl Look for the headers
AC_CHECK_HEADER([netcdfcpp.h], [have_netcdf=yes], [have_netcdf=no])

dnl Look for the library
if test $have_netcdf = yes; then
    AC_CHECK_LIB([netcdf], [ncopts], [have_netcdf=yes], [have_netcdf=no])
fi

dnl Check that the library has what we need
if test $have_netcdf = yes; then
    saved_LIBS="$LIBS"
    LIBS="$LIBS -lnetcdf_c++ -lnetcdf"
    AC_MSG_CHECKING([for NcFile in -lnetcdf_c++])
    AC_LINK_IFELSE(
        [AC_LANG_PROGRAM(
            [#include <netcdfcpp.h>],
            [NcFile::NcFile nc("example.nc")])],
        [
            AC_MSG_RESULT([yes])
            have_netcdf=yes
        ],
        [
            AC_MSG_RESULT([no])
            have_netcdf=no
        ]
    )
    LIBS="$saved_LIBS"
fi

dnl Define what is needed
if test $have_netcdf = yes; then
    if test -d /usr/include/netcdf-3 ; then
        NETCDF_CFLAGS="-I/usr/include/netcdf-3"
    fi
    if test -d /usr/lib/netcdf-3 ; then
        NETCDF_LIBS="-L/usr/lib/netcdf-3"
    fi
    m4_default([$1],[])
else
    m4_default([$2],[AC_MSG_ERROR([NetCDF not found])])
fi
])
