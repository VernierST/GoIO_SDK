GoIO Software Development Kit

The GoIO SDK is intended to provide software developers with the ability to access Vernier Software & Technology Go! devices from their own applications. Supported devices include the Go! Motion detector, the Go! Temp thermometer, the Vernier Mini Gc, and the Go! Link sensor interface, which can connect to a variety of analog sensors.

The heart of the SDK is the GoIO_DLL library.
The API to this library is documented in \GoIO_SDK\redist\include\GoIO_DLL_interface.h. The sequence of function calls into the library is best illustrated by the GoIO_DeviceCheck sample code.

The Windows version of this library is GoIO_DLL.dll. Since GoIO_DLL.dll is a standard Microsoft Windows DLL, you can access the library from a variety of languages including C, C++, Basic, LabView, and Java.

The Apple version of the GoIO_DLL library comes in two different flavors: libGoIO_DLL.dylib and libGoIO_DLL.framework. Both flavors of the library implement exactly the same API. They are just packaged differently as a convenience to users. These libraries can also be accessed from a variety of languages. GoIO_DLL requires Mac OS 10.3.9 or later. Note that libGoIO_DLL.framework is stored in \GoIO_SDK\redist\GoIO_DLL\MacOSX\libGoIO_DLL.framework.zip . You will have to unzip it before you can use it.

libGoIO_DLL.dylib has the install directory set to "@executable_path/libGoIO_DLL.dylib", which means it will only work if it is located in the same directory as the executable: e.g. for an application, this would be in the bundle folder: MyApp.app/Contents/MacOS. The libGoIO_DLL.framework, likewise, is expected to reside in /Library/Frameworks. To change the install location, you need to run /usr/bin/install_name_tool on the library executable -- libGoIO_DLL.dylib or libGoIO_DLL.framework/Versions/A/libGoIO_DLL. For more info, see "man install_name_tool".

The Linux version of the GoIO library just contains the source and one test program to test the library functions, for now.  Follow the INSTALL instructions to get started.  

====================================================================================================================

The SDK includes complete source code in C++. You are free to use this code however you like. All the software included in this SDK was written by Vernier Software & Technology(VST), and VST places absolutely no legal restrictions on using the software. You can copy the code, modify it, embed it in your own products, etc.

VST does not guarantee that the code is bug free, but we try to make it so. If you find any bugs, please report them to http://www.vernier.com/tech/supportform.html .

VST would prefer that you not modify GoIO_DLL.dll, libGoIO_DLL.dylib, or libGoIO_DLL.framework. If you do modify any of them, please give your modified version a different name, so we can easily distinguish your version from the VST version.

====================================================================================================================

GoIO_DeviceCheck is a very simple command line sample application that opens the first Go device that it finds, takes some measurements and printf's the results to STDOUT. It is written in C++(really just C) and its source code runs on Windows, MacOSX, and Linux. Looking at the sample code in \src\GoIO_DeviceCheck\GoIO_DeviceCheck.cpp is the best place to start if you are new to GoIO. Even if you are programming in a different language, eg. Basic or Java, GoIO_DeviceCheck.cpp is probably the first thing that you should look at.

====================================================================================================================

GoIO_console2 is a sample application that is coded to the GoIO_DLL API. The Windows version source code was written in Microsoft Visual C++ version 6.0. The Apple version is a Cocoa application written in Objective C++ using the Xcode IDE.

To run this application, you need to plug in one or more Go! devices into USB ports, click on the GoIO_console2 Devices menu, and then click on one of the listed devices.

Note that the Go! devices are HID USB devices which use standard device drivers that are automatically preinstalled on Windows ME, Win2000, and XP systems. The HID USB device drivers are present on Windows 98 installation CD's but they are not always preinstalled. You may be prompted to insert a Windows 98 installation CD when you plug in a Go! device for the first time.

====================================================================================================================

GoIO_console is a sample application that basically acts as a low level USB HID packet logger for the Go devices. Because it logs almost all the data communication packets sent between the host computer and the Go devices, it is a useful diagnostic application when testing new devices and device drivers. This application is not coded to the GoIO_DLL API, so we do not recommend that you use it as a template for your applications.

Currently, we only provide a Windows version of GoIO_console. It was written in Microsoft Visual C++ version 6.0.

====================================================================================================================

Release notes:

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

