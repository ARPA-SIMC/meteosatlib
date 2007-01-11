# METEOSATLIB_DEFS([METEOSATLIB_REQS=meteosatlib])
# ---------------------------------------
AC_DEFUN([METEOSATLIB_DEFS],
[
	dnl Import meteosatlib data
	PKG_CHECK_MODULES(METEOSATLIB,m4_default([$1], meteosatlib))
	AC_SUBST(METEOSATLIB_CFLAGS)
	AC_SUBST(METEOSATLIB_LIBS)
])
