To prepare your machine to build the GoIO library, you must install some packages first.
On a clean ubuntu 12 machine, you must do the following at the command line:
	sudo apt-get install aptitude
	sudo aptitude install build-essential
	sudo aptitude install automake1.9
	sudo aptitude install dpkg-dev
	sudo aptitude install libtool
	sudo aptitude install libusb-1.0-0-dev

To build and install the GoIO library, invoke build.sh in the root folder of the GoIO SDK.
This is basically just a standard invocation of autogen.sh, except that the --enable-libusb option is specified.
For desktop Linux builds, the --enable-libusb option is necessary.

After building and installing the GoIO library, you can invoke /GoIO_DeviceCheck/build.sh to build the GoIO_DeviceCheck application.

Further information describing the GoIO library may be found in readme.txt.
Note that the Linux version of the SDK does not include a redist folder, but you can find
GoIO_DLL_interface.h in the GoIO_DLL subfolder.
