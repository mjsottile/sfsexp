#!/usr/bin/env bash

function test {
    libtool --mode=execute valgrind --leak-check=full --show-reachable=yes -q "$@" > /dev/null
    status=$?
    if [ $status -ne 0 ]; then
        echo "error with $1"
    else
        echo "no error with $1"
    fi
    return $status
}

test ./ctest

## note: need to keep iteration count for ctorture low here
## due to performance overhead of valgrind.
perl ./randsexp.pl 200 40000 0.4 > /tmp/SEXP.SKINNY
test ./ctorture -i 10 -f /tmp/SEXP.SKINNY
rm -f /tmp/SEXP.SKINNY

test ./error_codes
test ./partial
test ./read_and_dump
test ./readtests
