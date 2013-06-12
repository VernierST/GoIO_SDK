# Microsoft Developer Studio Project File - Name="GoIO_console" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=GoIO_console - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "GoIO_console.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "GoIO_console.mak" CFG="GoIO_console - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "GoIO_console - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "GoIO_console - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "Perforce Project"
# PROP Scc_LocalPath "..\.."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "GoIO_console - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GR /GX /O2 /I "..\..\GoIO_cpp" /I "..\..\GoIO_cpp\Win32" /I "." /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "TARGET_OS_WIN" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "GoIO_console - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GR /GX /ZI /Od /I "..\..\GoIO_cpp" /I "..\..\GoIO_cpp\Win32" /I "." /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "TARGET_OS_WIN" /FR /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "GoIO_console - Win32 Release"
# Name "GoIO_console - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
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

SOURCE=.\GoIO_console.cpp
# End Source File
# Begin Source File

SOURCE=.\GoIO_console.rc
# End Source File
# Begin Source File

SOURCE=.\GoIO_consoleDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\GoIO_consoleView.cpp
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

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\SetMeasPeriodDlg.cpp
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

SOURCE=..\..\GoIO_cpp\GMiniGCDDSMem.h
# End Source File
# Begin Source File

SOURCE=.\GoIO_console.h
# End Source File
# Begin Source File

SOURCE=.\GoIO_consoleDoc.h
# End Source File
# Begin Source File

SOURCE=.\GoIO_consoleView.h
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

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\SetMeasPeriodDlg.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=..\..\GoIO_cpp\Win32\WinEnumDevices.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\GoIO_console.ico
# End Source File
# Begin Source File

SOURCE=.\res\GoIO_console.rc2
# End Source File
# Begin Source File

SOURCE=.\res\GoIO_consoleDoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
