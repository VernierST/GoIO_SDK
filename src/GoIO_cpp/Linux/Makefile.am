AM_CXXFLAGS = $(GIO_EXTRA_CFLAGS)
AM_CFLAGS = $(GIO_EXTRA_CFLAGS)

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/GoIO_cpp/ -I$(top_srcdir)/GoIO_cpp/Linux/ -I$(top_srcdir)/GoIO_DLL/ -DTARGET_OS_LINUX 

noinst_LTLIBRARIES = libGoIOcppLinux.la

EXTRA_DIST = libGoIOcppLinux.la

libGoIOcppLinux_la_SOURCES  = \
	GThread_Linux.cpp \
	GTextUtils_Linux.cpp \
	GSkipBaseDevice_Linux.cpp \
	GSkipBaseDevice_Linux_libusb.cpp \
	GUtils_Linux.cpp \
	stdafx.h
