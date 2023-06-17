#!/bin/sh
# This script is the entrypoint for the Dockerfile.

# If `make generate` introduces any changes, it means that the repo versions of
# src/ and man/ are out of sync with gen/. This can be caused by unwittingly
# editing src/ or man/ directly instead of editing their templates, or by
# editing the templates and forgetting to regenerate the source files.
# Either way it is bad and deserves a job failure.
tmp="$(mktemp -d current.XXXXX)"
cp -r -- src man "$tmp"
make LUA=lua5.3 generate
diff -r -q "$tmp/src" src || { echo 'error: src not in sync with gen' >&2; exit 1; }
diff -r -q "$tmp/man" man || { echo 'error: man not in sync with gen' >&2; exit 2; }
rm -rf -- "$tmp"

# Run all tests on Staple built with a particular compiler
run_tests () {
    echo
    echo ">>> build&test with $1..."

    # For some reason valgrind crashes inside the docker image, disable it.
    # Hopefully I can find a fix for this later, memory safety is important.
    make -j$(nproc) LUA=lua5.3 CC="$1" VALGRIND= clean test_clean test
}
run_tests gcc
run_tests clang
