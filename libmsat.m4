# LIBMSAT_DEFS([LIBMSAT_REQS=libmsat])
# ---------------------------------------
AC_DEFUN([LIBMSAT_DEFS],
[
	dnl Import libmsat data
	PKG_CHECK_MODULES(LIBMSAT,m4_default([$1], libmsat))
	AC_SUBST(LIBMSAT_CFLAGS)
	AC_SUBST(LIBMSAT_LIBS)
])
