echo aclocal
aclocal
echo libtoolize
libtoolize --force
echo autoconf
autoconf
echo automake
automake -a
touch NEWS README AUTHORS ChangeLog
automake -a
./configure --prefix=/usr --sysconfdir=/etc
make
make distcheck
sudo make install