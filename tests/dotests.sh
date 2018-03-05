#!/bin/sh

echo "=================================================================="

echo "Generating trees"
perl ./randsexp.pl 200 40000 0.1 > /tmp/SEXP.FAT
perl ./randsexp.pl 200 40000 0.25 > /tmp/SEXP.REGULAR
perl ./randsexp.pl 200 40000 0.4 > /tmp/SEXP.SKINNY

echo "=================================================================="
echo "Running ctorture tests..."

./ctorture -i 10000 < /tmp/SEXP.FAT
./ctorture -i 10000 < /tmp/SEXP.REGULAR
./ctorture -i 10000 < /tmp/SEXP.SKINNY

echo "=================================================================="
echo "Running torture tests..."

./ctorture -t -i 10000 < /tmp/SEXP.FAT
./ctorture -t -i 10000 < /tmp/SEXP.REGULAR
./ctorture -t -i 10000 < /tmp/SEXP.SKINNY

echo "=================================================================="
echo "Running readtests..."

./readtests
echo "=================================================================="
echo "Cleaning up..."
rm -f /tmp/SEXP.FAT /tmp/SEXP.REGULAR /tmp/SEXP.SKINNY
echo "=================================================================="
