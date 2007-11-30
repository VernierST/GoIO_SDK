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
./configure --prefix=/usr
make
make distcheck
sudo make install