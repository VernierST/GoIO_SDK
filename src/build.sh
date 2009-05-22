touch NEWS README AUTHORS ChangeLog
./autogen.sh --prefix=/usr --sysconfdir=/etc
make
make distcheck
sudo make install

