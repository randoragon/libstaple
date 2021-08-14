#!/bin/sh

# Generates suffixed variants of a man page, which link back to the generic
# variant.
#
# Usage:    ./gen-suffixed.sh DIR
# Example:  ./gen-suffixed.sh stack
#
# The script assumes the following directory structure:
#
#       stack/
#           suffixed/
#               rnd_stack_pushc.3
#               rnd_stack_pushs.3
#               ... (all suffixed form man pages)
#           rnd_stack_push.3
#           rnd_stack_pop.3
#           ... (all generic form man pages)

[ ! -d "$1" ] && echo 'output directory not found' >&2 && exit 1
mkdir -p "$1/suffixed"

for f in "$1"/*.[0-9]; do
    [ ! -r "$f" ] && continue

    fname="$(basename -- "$f")"
    man_name="${fname%.*}"
    man_section="${fname##*.}"
    reference=".so man${man_section}/$fname"
    suffixes='c s i l sc uc us ui ul f d ld'

    for suffix in $suffixes; do
        echo "$reference" > "$1/suffixed/${man_name}${suffix}.${man_section}"
    done
done
