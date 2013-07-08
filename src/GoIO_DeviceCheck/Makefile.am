
AM_CFLAGS   = -g -Wall

#For desktop build:
AM_CPPFLAGS = -DTARGET_OS_LINUX $(GOIO_CFLAGS)

#For LabQuest device build:
#AM_CPPFLAGS = -DTARGET_OS_LINUX -DTARGET_PLATFORM_LABQUEST $(GOIO_CFLAGS)

bin_PROGRAMS = GoIO_DeviceCheck

GoIO_DeviceCheck_SOURCES = 	GoIO_DeviceCheck.cpp

GoIO_DeviceCheck_LDADD   = $(GOIO_LIBS)

