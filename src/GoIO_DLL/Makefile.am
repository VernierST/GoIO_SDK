AM_CXXFLAGS = $(GIO_EXTRA_CFLAGS)
AM_CFLAGS = $(GIO_EXTRA_CFLAGS)

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/GoIO_cpp/ -I$(top_srcdir)/GoIO_cpp/Linux/ -I$(top_srcdir)/GoIO_DLL/ -DTARGET_OS_LINUX -DTARGET_HANDHELD

EXTRA_DIST = GoIO.pc.in 48-ldusb.rules

library_includedir= $(includedir)/GoIO
library_include_HEADERS = GoIO_DLL_interface.h

lib_LTLIBRARIES = libGoIO.la

libGoIO_la_SOURCES = GoIO_DLL_interface.cpp

libGoIO_la_LIBADD= $(top_srcdir)/GoIO_cpp/libGoIOcpp.la $(top_srcdir)/GoIO_cpp/Linux/libGoIOcppLinux.la 

libGoIO_la_LDFLAGS = -version-info 2:13:0

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = GoIO.pc

udevdir = $(sysconfdir)/udev/rules.d
dist_udev_DATA = 48-ldusb.rules
