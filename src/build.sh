touch NEWS README AUTHORS ChangeLog
./autogen.sh --prefix=/usr --sysconfdir=/etc --enable-libusb
make
#make distcheck DISTCHECK_CONFIGURE_FLAGS='--enable-libusb'
make dist
sudo make install

