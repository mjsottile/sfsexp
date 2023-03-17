#!/bin/bash

if [ ! -f ./configure ]; then
  echo "Generate configure first.  See INSTALL."
  exit 1
fi

if [ -f ./Makefile ]; then
  make distclean
fi

./configure --enable-thread-unsafe-memory-management
make
cd tests
./check_leaks.sh
cd ../examples
./check_leaks.sh
cd ..

make distclean
./configure --enable-debug --enable-thread-unsafe-memory-management
make
cd tests
./check_leaks.sh
cd ../examples
./check_leaks.sh
cd ..

make distclean
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
