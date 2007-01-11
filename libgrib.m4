# LIBGRIB_DEFS([LIBGRIB_REQS=libgrib])
# ---------------------------------------
AC_DEFUN([LIBGRIB_DEFS],
[
	dnl Import libgrib data
	PKG_CHECK_MODULES(LIBGRIB,m4_default([$1], libgrib))
	AC_SUBST(LIBGRIB_CFLAGS)
	AC_SUBST(LIBGRIB_LIBS)
])
