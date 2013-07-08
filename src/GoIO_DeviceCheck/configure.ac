AC_PREREQ(2.53)
AC_INIT(GoIO_DeviceCheck, 0.1, http://www.vernier.com/)
AM_INIT_AUTOMAKE()
AC_CONFIG_SRCDIR(GoIO_DeviceCheck.cpp)

AC_PROG_CC
AC_PROG_CXX

PKG_CHECK_MODULES(GOIO, GoIO)

AC_SUBST(GOIO_CFLAGS)
AC_SUBST(GOIO_LIBS)

AC_CONFIG_FILES([
		Makefile
])

AC_OUTPUT

echo " "
AC_MSG_RESULT([Configured to install in: ${prefix}])

