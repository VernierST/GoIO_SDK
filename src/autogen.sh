#! /bin/sh


touch  NEWS README AUTHORS ChangeLog

unset AUTOMAKE
for am in automake-1.7 automake-1.8 automake-1.9 automake; do
	which $am > /dev/null || continue
	ver=`$am --version | head -n 1 | sed -e s/^[^0-9]*//`
	verint=`echo $ver | sed -e s/[^0-9]//g`
	if test $verint -ge 190; then
		AUTOMAKE=$am
		break
	fi
done
test -z $AUTOMAKE && {
	echo "Automake version 1.9.0 is required to build this package"
	exit 1
}

autoreconf -v --install || exit 1
./configure --enable-maintainer-mode "$@"
