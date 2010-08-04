# Microsoft Developer Studio Project File - Name="GoIO_DLL" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=GoIO_DLL - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "GoIO_DLL.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "GoIO_DLL.mak" CFG="GoIO_DLL - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "GoIO_DLL - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "GoIO_DLL - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "Perforce Project"
# PROP Scc_LocalPath ".."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "GoIO_DLL - Win32 Release"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GR /GX /O2 /I "..\GoIO_cpp" /I "..\GoIO_cpp\Win32" /I "." /I ".." /I "..\..\GoIO_cpp" /I "..\..\GoIO_cpp\Win32" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_WINDLL" /D "_MBCS" /D "_USRDLL" /D "TARGET_OS_WIN" /D "_GOIO_DLL_SRC" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 /nologo /subsystem:windows /dll /machine:I386

!ELSEIF  "$(CFG)" == "GoIO_DLL - Win32 Debug"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GR /GX /ZI /Od /I "." /I ".." /I "..\..\GoIO_cpp" /I "..\..\GoIO_cpp\Win32" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_WINDLL" /D "_MBCS" /D "_USRDLL" /D "TARGET_OS_WIN" /D "_GOIO_DLL_SRC" /FR /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "GoIO_DLL - Win32 Release"
# Name "GoIO_DLL - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\GoIO_cpp\GCalibrateDataFuncs.cpp
# End Source File
# Begin Source File

SOURCE=..\..\GoIO_cpp\GCyclopsDevice.cpp
# End Source File
# Begin Source File

SOURCE=..\..\GoIO_cpp\GDeviceIO.cpp
# End Source File
# Begin Source File

SOURCE=..\..\GoIO_cpp\GMBLSensor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\GoIO_cpp\GMiniGCDevice.cpp
# End Source File
# Begin Source File

SOURCE=.\GoIO_DLL.cpp
# End Source File
# Begin Source File

SOURCE=.\GoIO_DLL.def
# End Source File
# Begin Source File

SOURCE=.\GoIO_DLL.rc
# End Source File
# Begin Source File

SOURCE=..\GoIO_DLL_interface.cpp
# End Source File
# Begin Source File

SOURCE=..\..\GoIO_cpp\GPortRef.cpp
# End Source File
# Begin Source File

SOURCE=..\..\GoIO_cpp\GSkipBaseDevice.cpp
# End Source File
# Begin Source File

SOURCE=..\..\GoIO_cpp\Win32\GSkipBaseDevice_Win.cpp
# End Source File
# Begin Source File

SOURCE=..\..\GoIO_cpp\GSkipDevice.cpp
# End Source File
# Begin Source File

SOURCE=..\..\GoIO_cpp\GTextUtils.cpp
# End Source File
# Begin Source File

SOURCE=..\..\GoIO_cpp\GThread.cpp
# End Source File
# Begin Source File

SOURCE=..\..\GoIO_cpp\Win32\GThread_Win.cpp
# End Source File
# Begin Source File

SOURCE=..\..\GoIO_cpp\GUSBDirectTempDevice.cpp
# End Source File
# Begin Source File

SOURCE=..\..\GoIO_cpp\GUtils.cpp
# End Source File
# Begin Source File

SOURCE=..\..\GoIO_cpp\Win32\GUtils_Win.cpp
# End Source File
# Begin Source File

SOURCE=..\..\GoIO_cpp\NonSmartSensorDDSRecs.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# End Source File
# Begin Source File

SOURCE=..\..\GoIO_cpp\Win32\WinEnumDevices.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\GoIO_cpp\GCalibrateDataFuncs.h
# End Source File
# Begin Source File

SOURCE=..\..\GoIO_cpp\GCharacters.h
# End Source File
# Begin Source File

SOURCE=..\..\GoIO_cpp\GCyclopsCommExt.h
# End Source File
# Begin Source File

SOURCE=..\..\GoIO_cpp\GCyclopsDevice.h
# End Source File
# Begin Source File

SOURCE=..\..\GoIO_cpp\GDeviceIO.h
# End Source File
# Begin Source File

SOURCE=..\..\GoIO_cpp\GMBLSensor.h
# End Source File
# Begin Source File

SOURCE=..\..\GoIO_cpp\GMiniGCDevice.h
# End Source File
# Begin Source File

SOURCE=.\GoIO_DLL.h
# End Source File
# Begin Source File

SOURCE=..\GoIO_DLL_interface.h
# End Source File
# Begin Source File

SOURCE=..\..\GoIO_cpp\GPlatformDebug.h
# End Source File
# Begin Source File

SOURCE=..\..\GoIO_cpp\GPlatformTypes.h
# End Source File
# Begin Source File

SOURCE=..\..\GoIO_cpp\GPortRef.h
# End Source File
# Begin Source File

SOURCE=..\..\GoIO_cpp\GSensorDDSMem.h
# End Source File
# Begin Source File

SOURCE=..\..\GoIO_cpp\GSkipBaseDevice.h
# End Source File
# Begin Source File

SOURCE=..\..\GoIO_cpp\GSkipComm.h
# End Source File
# Begin Source File

SOURCE=..\..\GoIO_cpp\GSkipCommExt.h
# End Source File
# Begin Source File

SOURCE=..\..\GoIO_cpp\GSkipDevice.h
# End Source File
# Begin Source File

SOURCE=..\..\GoIO_cpp\GSkipErr.h
# End Source File
# Begin Source File

SOURCE=..\..\GoIO_cpp\GStdIncludes.h
# End Source File
# Begin Source File

SOURCE=..\..\GoIO_cpp\GTextUtils.h
# End Source File
# Begin Source File

SOURCE=..\..\GoIO_cpp\GThread.h
# End Source File
# Begin Source File

SOURCE=..\..\GoIO_cpp\GTypes.h
# End Source File
# Begin Source File

SOURCE=..\..\GoIO_cpp\GUSBDirectTempDevice.h
# End Source File
# Begin Source File

SOURCE=..\..\GoIO_cpp\GUtils.h
# End Source File
# Begin Source File

SOURCE=..\..\GoIO_cpp\GVernierUSB.h
# End Source File
# Begin Source File

SOURCE=..\..\GoIO_cpp\NonSmartSensorDDSRecs.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=..\..\GoIO_cpp\Win32\WinEnumDevices.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
