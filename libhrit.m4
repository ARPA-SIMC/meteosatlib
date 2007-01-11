# LIBHRIT_DEFS([LIBHRIT_REQS=libhrit])
# ---------------------------------------
AC_DEFUN([LIBHRIT_DEFS],
[
	dnl Import libhrit data
	PKG_CHECK_MODULES(LIBHRIT,m4_default([$1], libhrit))
	AC_SUBST(LIBHRIT_CFLAGS)
	AC_SUBST(LIBHRIT_LIBS)
])
