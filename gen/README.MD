# Code Generation

This directory contains lua code and templates used to automatically generate
the C source code and man pages for the entire Staple library. Code generation
is employed to circumvent the pitfalls of managing a highly redundant code base
by hand.

This is pretty much developer-only territory, as the end-user does not need to
know or care about this -- the repository already contains the generated files.

## Why generate code?

Generating code saves us implementing all the different suffixed function
variants, and instead we write one template based on which they are generated.
Not only is this less error-prone and makes it easy to make changes, it also
automatically saves each function as a separate translation unit, resulting in a
"maximally fragmented" library file. This is good, because later if you
statically link your program with Staple, only the object files corresponding to
the used functions are embedded into the binary, minimizing its file size.

## luaparser

The entire generator is written in pure lua 5.3 (compatible with lua 5.4). The
code is a bit lengthy to read and not the easiest to understand. However, it
does the job and there shouldn't be a reason to change it, unless new
functionality is needed. I would like to document it more in the future, but for
now my priorities lie elsewhere.

## Relevant files

### `generate.lua`

The main runner script invoked by the Makefile; it contains a table defining
every (except `str`,`strn` and `b`, which are implemented separately) suffixed
variant to generate. It should be easy to tweak even without me describing its
syntax, because the existing configuration serves as a good example.

### `pathconf.lua`

Contains hard-coded paths to every template that `generate.lua` should parse.
The reason this is necessary to hard-code is that lua has no built-in facility
for listing files, and I didn't want to rely on an external library like
LuaFileSystem. \
This file must be manually updated and there are no mechanisms to remind you to
do that -- it will just generate less code.

### `luaparser/generate_man.lua`

Defines a `MAN_SORT_ORDER` table, which defines the sort order for SEE ALSO
sections. \
This must also be manually updated, however it is equipped with a
warning system that should report if any item is missing.
