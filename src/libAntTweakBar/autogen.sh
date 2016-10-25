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

aclocal
automake -a -c
autoconf

#./configure --enable-debug --enable-dbrecord
#./configure --enable-debug
#./configure
# if you use --enable-debug option, you may use gprof to get the run time for each of the function.

make clean
make

fi
