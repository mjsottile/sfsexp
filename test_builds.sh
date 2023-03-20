#!/usr/bin/env bash

if [ ! -f ./configure ]; then
  echo "Generate configure first.  See INSTALL."
  exit 1
fi

if [ -f ./Makefile ]; then
  make distclean
fi

./configure
make
cd tests
./check_leaks.sh
cd ../examples
./check_leaks.sh
cd ..

make distclean
./configure --enable-debug
make
cd tests
./check_leaks.sh
cd ../examples
./check_leaks.sh
cd ..

make distclean
CFLAGS=-D_NO_MEMORY_MANAGEMENT_ ./configure
make
cd tests
./check_leaks.sh
cd ../examples
./check_leaks.sh
cd ..

make distclean
CFLAGS=-D_NO_MEMORY_MANAGEMENT_ ./configure --enable-debug 
make
cd tests
./check_leaks.sh
cd ../examples
./check_leaks.sh
cd ..
