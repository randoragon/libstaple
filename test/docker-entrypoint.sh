#!/bin/sh

# This script is the entrypoint for the Dockerfile.

run_tests () {
    echo ">>> build&test with $1..."

    # For some reason valgrind crashes inside the docker image, disable it.
    # Hopefully I can find a fix for this later, memory safety is important.
    make LUA=lua5.3 CC="$1" VALGRIND= clean test_clean test
}

run_tests gcc
run_tests clang
