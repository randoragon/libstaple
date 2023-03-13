# Staple - easy data structures in C

![build status](https://github.com/Randoragon/libstaple/actions/workflows/main.yaml/badge.svg)

Staple is a general-purpose library implementing common data structures in pure
C without any external dependencies besides the C standard library.

## Available Modules

- stack
- queue

## Pending Modules

The list might grow/shrink and implementations need not follow in any particular
order. I work on these (among other things) in my spare time, so expect either
slow development, or sudden bursts of productivity separated by hiatuses.

- deque,
- bitarray,
- hashmap,
- linked list,
- 2D/3D matrix,
- priority queue,
- avl tree,
- rbtree

## Quick Example

Here's a program that creates a stack of integers, pushes a few numbers, prints
the top of the stack, then pops and returns it.

```c
#include <stdio.h>
#include <staple.h>

int main(void)
{
	struct sp_stack *s;
	int ret;
	s = sp_stack_create(sizeof(int), 10);

	sp_stack_pushi(s, 10);
	sp_stack_pushi(s, 4);
	sp_stack_pushi(s, 8);
	printf("top of the stack is %d\n", sp_stack_peeki(s));
	ret = sp_stack_popi(s);

	sp_stack_destroy(s, NULL);
	return ret;
}
```

## Library Design Overview

### Compatibility

- works with any version of C starting from C89
- fully compliant, compiler-agnostic

### Simplicity

- no magic macros
- no typedefs

### Convenience

- super easy integration with primitive types
- man pages for every module and function (integrates nicely with vim's `K`
  keybinding)
- built-in print functions for every structure

### Flexibility

- can handle generic data behind void pointers
- all data structures have implementations for arbitrary insertion/removal
  get/set functions (if feasible), even if the structure is not optimized for
  it. Some libraries will implement e.g. the stack with only push/peek/pop
  operations and not bother with insertion, removal and lookup at arbitrary
  indices. Staple gives you full control out-of-the-box, because even if in 99%
  of cases you will only push/pop from the top of the stack, once in a while you
  need that pinpoint insert, even at the cost of performance.
- no opaque types - you are free to extend the library, if needed

### Reliability

- complete type safety
- unit testing with [libcheck](https://libcheck.github.io/check/index.html)
- runtime errors reported on stderr (can be disabled) and with return codes

## Handling Generic Data vs. Primitives

Every function that operates on structure elements (e.g. push, pop, insert,
etc.) is available in 2 forms:

- **generic form** for `void*` - allows storing any data (pass-by-reference)
- **suffixed form** - a faster way to work with primitive types only
  (pass-by-value)

Examples:

```c
sp_stack_push(s, &data); /* reads from a (void*) */
sp_stack_pushi(s, -6);   /* pass-by-value (int) */
sp_stack_pushus(s, 3);   /* pass-by-value (unsigned short) */
```

Here's a full table of suffixes, although all you need to remember is to use the
first letters of your target type (except for strings):

suffix | type
---: | :---
none | any (`void*`)
`c`, `sc`, `uc` | `char`, `signed char`, `unsigned char`
`s`, `us` | `short`, `unsigned short`
`i`, `ui` | `int`, `unsigned int`
`l`, `ul` | `long`, `unsigned long`
`f`, `d`, `ld` | `float`, `double`, `long double`
`str`, `strn` | string (`char*`), substring (`char*`)

`signed char` is the only explicitly signed type available, due to the fact that
it is not defined by the standard whether or not `char` alone is signed or
unsigned.

Additionally, these C99 type suffixes are available when compiling the library
in C99 (default):

suffix | type
---: | :---
`b` | `_Bool`
`ll`, `ull` | `long long`, `unsigned long long`
`i8`, `u8` | `int8_t`, `uint8_t`
`i16`, `u16` | `int16_t`, `uint16_t`
`i32`, `u32` | `int32_t`, `uint32_t`
`i64`, `u64` | `int64_t`, `uint64_t`

This division into generic and suffixed form without using macro functions
introduces a **lot** of redundancy and inflation to the code base and resulting
library size, which is a conscious choice on my part. **This library does not
aim to be small in size**. It prioritizes simplicity, convenience and
flexibility.

## Installation

AUR: [libstaple](https://aur.archlinux.org/packages/libstaple/)

On a Linux system with `make`, simply clone the repo and run the following (if
necessary, as root):

	make clean install

To uninstall, run

	make uninstall

By default, the files will be copied to `/usr/local/lib`, `/usr/local/include`
and `/usr/local/share/man`. If you want them (un)installed directly at `/usr`
(or somewhere else), override the `PREFIX` variable:

	make   install PREFIX=/usr
	make uninstall PREFIX=/usr

The Makefile produces two library files, `libstaple.so` and `libstaple.a`. The
former is a dynamically linked library, the latter is static.

The library can be compiled with various optional modes (refer to the
**libstaple**(7) man page for descriptions of each):

	make clean install DEBUG=1 QUIET=1 ABORT=1

### Compiling with C89 only

If your target platform does not have a C99 compiler, the library can be built
with just "ANSI" C89:

	make STDC=c89 clean install

This option is provided for compatibility purposes only - the library will
behave exactly the same, except the C99 functionality will be missing.

## Unit Testing

All test units are stored in `test/src`. If you wish to run the tests, you will
need [libcheck](https://github.com/libcheck/check) and optionally
[valgrind](https://valgrind.org/). By default, the tests are run with valgrind
to check for memory-related errors. Valgrind can be disabled by adding
`VALGRIND=` argument to any make command, e.g. `make VALGRIND= test`.

Testing is divided into modules. To execute tests for a single module "`XYZ`", run

	make test_XYZ

(for example `make test_stack`). You can also run tests for all the
modules with

	make test

**NOTE:** Running tests requires that Staple was built with debug mode enabled
and abort mode disabled. The Makefile is not smart enough to know when that
isn't the case, but the test executables will notice and exit with an error
automatically. If this happens, simply force the library to recompile with `make
clean` and try running the tests again.

Test executables and object files can be removed with

	make test_clean

(this is probably not useful unless you are writing tests).

### Running tests in Docker

The supplied `Dockerfile` can be used to automatically run tests in a Docker
container. This includes all the libcheck tests and possibly more - for example
it ensures the library is compiler-agnostic by testing Staple built with a few
different C compilers (see `test/docker-entrypoint.sh` for more details).

To build the Dockerfile and execute all tests in a container, run

	docker build -t staple .
	docker run staple

---

If you notice any test failing on a release commit, please open an issue on
GitHub or send me an email: <randoragongamedev@gmail.com>
