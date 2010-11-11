GoIO Software Development Kit

The GoIO SDK is intended to provide software developers with the ability to access Vernier Software & Technology Go! devices from their own applications. Supported devices include the Go! Motion detector, the Go! Temp thermometer, the Vernier Mini Gc, and the Go! Link sensor interface, which can connect to a variety of analog sensors.

The heart of the SDK is the GoIO_DLL library.
The API to this library is documented in \GoIO_SDK\redist\include\GoIO_DLL_interface.h. The sequence of function calls into the library is best illustrated by the GoIO_DeviceCheck sample code.

The Windows version of this library is GoIO_DLL.dll. Since GoIO_DLL.dll is a standard Microsoft Windows DLL, you can access the library from a variety of languages including C, C++, Basic, LabView, and Java. This code compiles in Visual Studio 2005. We have also written a .NET wrapper for GoIO_DLL.dll called GoIOdotNET.dll. GoIOdotNET.dll allows .NET based applications to access the Go! devices.

For Mac OS, we provide two sets of libraries: Legacy and Universal. The Legacy libraries (libGoIO_DLL.a, libGoIO_DLL.dylib, and libGoIO_DLL.framework) have their base SDK set to 10.4; the current GoIO SDK no longer supports 10.3.9. Note that these can only be linked to 32 bit executables.

The Universal versions of the library (libGoIOUniversal.a, libGoIOUniversal.dylib, and libGoIOUniversal.framework) are targeted for Mac OS 10.5 and higher, and may be incorporated into 32 or 64 bit executables.

For each set of libraries, we provide 3 different flavors. The ".a" flavor is a static library which gets built directly into your executable at link time. The ".dylib" and ".framework" flavors are loaded dynamically each time your code is loaded for execution. Please note that the .dylib and .framework libraries are expected to be located at very specific places in the file system:

* The ".dylib" libraries have their install directory set to "@executable_path/libGoIO_DLL.dylib", which means they will only work when they are located in the same directory as the executable: e.g. for an application, this would be in the bundle folder: MyApp.app/Contents/MacOS. 

* The ".framework" libraries, likewise, are expected to reside in /Library/Frameworks. 

To change the install location of either, you need to run /usr/bin/install_name_tool on the library executable -- e.g. libGoIOUniversal.dylib or libGoIO_DLL.framework/Versions/A/libGoIO_DLL. For more info, see "man install_name_tool".

The Linux version of the GoIO SDK is packaged separately from the Windows/Mac version. Follow the INSTALL instructions to get started.  

Note that the Go! devices are HID USB devices which use standard device drivers that are automatically preinstalled on Microsoft Windows systems.

====================================================================================================================

The GoIO SDK includes complete source code in C++.

The GoIO SDK is currently distributed with a very permissive BSD style license. See the license.txt file located in the same folder as this readme.txt file.

Basically we encourage everyone to use the SDK, and to freely redistribute the GoIO_DLL library. If the restrictions set out in the license.txt file discourage you from using the SDK, please contact VST at http://www.vernier.com/tech/supportform.html .

VST does not guarantee that the code is bug free, but we try to make it so. If you find any bugs, please report them to http://www.vernier.com/tech/supportform.html .

====================================================================================================================

GoIO_DeviceCheck is a very simple command line sample application that opens the first Go! device that it finds, takes some measurements and printf's the results to STDOUT. It is written in C++(really just C) and its source code runs on Windows, MacOSX, and Linux. Looking at the sample code in \src\GoIO_DeviceCheck\GoIO_DeviceCheck.cpp is the best place to start if you are new to GoIO. Even if you are programming in a different language, eg. Basic or Java, GoIO_DeviceCheck.cpp is probably the first thing that you should look at.

====================================================================================================================

GoIO_Measure is a sample application that is coded to the GoIO_DLL API. The Windows version source code was written in Microsoft Visual C++ 2005.

GoIO_VB_Form_Measure is a Visual Basic .NET application that behaves very similarly to the GoIO_Measure sample application. This application is coded to the managed code API presented by GoIOdotNET.dll and VSTCoreDefsdotNET.dll. These .NET dll's provide a thin wrapper around the unmanaged code in GoIO_DLL.dll.

To run these applications, you need to plug in one or more Go! devices into USB ports, click on the Devices menu, and then click on one of the listed devices.

====================================================================================================================

.NET discussion:

GoIOdotNET.dll provides P/Invoke methods that access the public entry points to the unmanaged code in GoIO_DLL.dll.
VSTCoreDefsdotNET.dll provides some additional structure and constant definitions needed to work with GoIO_DLL. The API's presented by these dll's show up in the .NET object browser in the GoIOdotNET and VSTCoreDefsdotNET namespaces.

The XML documentation files for the GoIOdotNET and VSTCoreDefsdotNET assemblies provide a fair amount of documentation for the GoIO library API's that is accessible in the Object browser, but they are not complete. The most complete reference to the GoIO library API is in GoIO_DLL_interface.h and its associated header files.

The following files need to be distributed with .NET applications that reference GoIOdotNET:
GoIOdotNET.dll
VSTCoreDefsdotNET.dll
GoIO_DLL.dll

We generally recommend that you just place these files in the same folder as the .exe file for your application.

GoIOdotNET.dll and VSTCoreDefsdotNET.dll are pure .NET assemblies that contain no unmanaged code, so the same instance of each file is used by both 32 bit and 64 bit applications. GoIO_DLL.dll is unmanaged code(aka native), so there are separate binary files for 32 bit apps versus 64 bit apps. When you click on Properties->Version in the Windows explorer for GoIO_DLL.dll, the description string is either "GoIO_DLL DLL (x86)" or "GoIO_DLL DLL (x64)". 

If you build your .NET app for the x86 platform, then you should distribute the 32 bit version of GoIO_DLL.dll with your app. If you build your .NET app for the x64 platform, then you should distribute the 64 bit version of GoIO_DLL.dll with your app. If you build your .NET app for the 'Any CPU' platform, then the situation is more complex. Such apps run as 32 bit processes in 32 bit windows, and they run as 64 bit processes in 64 bit windows. This means your installer needs figure out what operating system is being used, and install the matching version of GoIO_DLL.

For maximum simplicity and portability, we recommend that you just build your .NET app for the x86 platform. Note that 32 bit applications(x86) run fine on 64 bit Windows systems.

====================================================================================================================

Release notes:

Version 2.40
Support the +/-30 volt sensor. Fixed bug associated with the +/-10 volt sensor that was introduced in version 2.37.

version 2.39
GoIO_DLL sources generally use int instead of long internally so that code behaves similarly for 32 and 64 bit builds.
Released 64 bit version of GoIO DLL library for the Mac.

Version 2.37
Add sensor map data for non smart sensors so DDS records are descriptive for all known sensors.
GoIO_Sensor_CalibrateData() supports non linear calibrations now, so stainless steel temperature is supported.
The Windows version of GoIO DLL can be built as a 64 bit binary.
.NET support has been added via GoIOdotNET.dll and VSTCoreDefsdotNET.dll.
GoIO_Diags_SetDebugTraceThreshold() allows dynamic control of debug output.

Version 2.30
Tweak DDS memory marshalling logic to avoid memory alignment traps on CPU's that care about such things(eg. ARM).

Version 2.29
Fixed how we specify Mac location name in order to be compatible with Logger Pro.

Version 2.28
Fixed an initialization bug associated with the Vernier Mini Gas Chromatograph(GC) on the Mac.

Version 2.27
Support Vernier Mini Gas Chromatograph(GC).
Add GoIO_DeviceCheck sample code.

Version 2.24
Fixed bug in Mac SDK in which the measurement queue was cleared whenever a STOP command was issued. (RM2846, 20090323)

Version 2.23
Add GoIO_Sensor_SendCmd() and GoIO_Sensor_GetNextResponse() API's. These are advanced API's that should almost never be used. It is generally better to use GoIO_Sensor_SendCmdAndGetResponse() instead.

Version 2.22
Fix a Linux bug that prevented GoLink data collection at rates > 60 hz.

Version 2.20
Fixed a Mac bug which resulted in failure to open a connection to a Go! device connected via a USB hub.

Version 2.19
In order to work around a serious Mac OS 10.4 kernel memory leak (related to HID report writes), I've had to upgrade to HID Interface 122 to get access to the setReport() call. This means we have removed support for 10.2. We now only support 10.3 and higher -- and for best results, we recommend 10.3.9.

Also, upgraded Mac VST_USB to match the head of line logger pro. This includes dynamic HID report-cookie discovery, and better IO failure diagnostics.

Version 2.18
Backed out a workaround for a known Apple memory leak when instantiating a USB plugin interface; the workaround might have been causing some instability, and furthermore was not thread safe. To minimize these leaks, we recommend that Apple clients open and close the device as few times as possible during a session. The Windows code has no known leaks.
Return proper return code from GoIO_Sensor_Unlock().

version 2.17
Allow more than one GoIO_DLL client application to run concurrently and access devices. A given device can only be accessed by a single client.
Fixed more Mac memory leaks.

Version 2.16
Fixed a memory leak on Mac. 
Allow for the fact that the GoLink powers up slowly when opening the device.

Version 2.15
Added support for Intel Macs -- libraries are in Universal Binary format.
Fixed a memory leak on Mac.

version 2.12
More minor tweaks for the Go! Motion DDS record. Include full source for everything now.
Added libGoIO_DLL.framework.

Version 2.11
Make sure GoIO_Sensor_DDSMem_GetSensorNumber() always reports the correct sensor number for Go! Motion.
Fix bad data in the DDS record for Go! Motion.

Version 2.10
Improved performance on slow systems.

Version 2.0:
Added support for Mac OS 10.2 and later.
Added support for Go! Motion.
Added GoIO_GetDLLVersion() API.

version 1.6:
The entry point names to GoIO_DLL.dll were mangled in C++ style in earlier versions. Version 1.6 does not
export mangled names, so the Win32 function GetProcAddress(hModule, "GoIO_Sensor_Open") should succeed now.

