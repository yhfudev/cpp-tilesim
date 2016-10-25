#!/bin/sh

./autoclean.sh

rm -f configure

rm -f Makefile.in

rm -f config.guess
rm -f config.sub
rm -f install-sh
rm -f missing
rm -f depcomp

if [ 0 = 1 ]; then
autoscan
else
#cd pflib && ./autogen.sh && cd ..

touch NEWS
touch README
touch AUTHORS
touch ChangeLog
touch config.h.in

cd src/libAntTweakBar/; ./autogen.sh; make distclean; cd -

#libtoolize --copy --force
aclocal
automake -a -c
autoconf

# if you use --enable-debug option, you may use gprof to get the run time for each of the function.
#./configure --enable-debug --enable-dbrecord
#./configure --enable-debug
#./configure --disable-threedimention
#make clean
#make

# Compile for win32:
#mkdir w32; cd w32
#PATH=$PATH:/opt/mingw/usr/bin/ ../configure --host=i686-pc-mingw32 --prefix=/opt/mingw/usr/i686-pc-mingw32/
#PATH=$PATH:/opt/mingw/usr/bin/ make

fi
