#!/bin/bash

function test {
    valgrind --leak-check=full --show-reachable=yes -q "$@" > /dev/null
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

