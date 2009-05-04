CPP_LINUX_SRC = Linux/

SUBDIRS = Linux

AM_CXXFLAGS = $(GIO_EXTRA_CFLAGS)
AM_CFLAGS = $(GIO_EXTRA_CFLAGS)

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/GoIO_cpp/ -I$(top_srcdir)/GoIO_cpp/Linux/ -I$(top_srcdir)/GoIO_DLL/ 

library_includedir= $(includedir)/GoIO
library_include_HEADERS = GSensorDDSMem.h GSkipCommExt.h GVernierUSB.h GMiniGCDDSMem.h

noinst_LTLIBRARIES = libGoIOcpp.la

EXTRA_DIST = libGoIOcpp.la

libGoIOcpp_la_SOURCES = GThread.cpp GTextUtils.cpp GUtils.cpp GUSBDirectTempDevice.cpp GSkipDevice.cpp GSkipBaseDevice.cpp GPortRef.cpp GMBLSensor.cpp GDeviceIO.cpp GCyclopsDevice.cpp GCharacters.h GDeviceIO.h GPlatformTypes.h  GSkipBaseDevice.h GSkipDevice.h   GTextUtils.h  GUSBDirectTempDevice.h GCyclopsCommExt.h  GMBLSensor.h      GPortRef.h        GSkipCommExt.h     GSkipErr.h      GThread.h     GUtils.h GCyclopsDevice.h   GPlatformDebug.h  GSensorDDSMem.h GSkipComm.h        GStdIncludes.h  GTypes.h      GVernierUSB.h


