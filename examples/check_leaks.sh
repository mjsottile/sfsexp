#!/usr/bin/env bash

# if valgrind isn't present we can't proceed - otherwise
# we get a cascade of command not found failures
if ! command -v valgrind &> /dev/null
then
    echo "cannot continue: valgrind could not be found"
    exit
fi

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

test ./binmode
test ./callbacks
test ./continuations
test ./packunpack
test ./paultest
test ./rcfile
test ./sexpvis
test ./simple_interp
