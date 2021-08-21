# Staple - easy data structures in C

Staple is a general-purpose library implementing common data structures in pure
C89 without any external dependencies besides the C standard library.

## Available Modules

- stack
- queue

## Pending Modules

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

### Simplicity

- pure C89
- no magic macros
- no typedefs

### Convenience

- super easy integration with primitive types
- man pages for every module and function
- built-in print functions for every structure

### Flexibility

- can handle generic data behind void pointers
- all data structures have implementations for arbitrary insertion/removal
  get/set functions (if feasible), even if the structure is not optimized for
  it. Some libraries will implement e.g. the stack with only push/peek/pop
  operations and not bother with insertion, removal and lookup at arbitrary
  indices. Staple gives you full control out-of-the-box, because even if in
  99% of cases you will only push/pop from the top of the stack, once in a while
  you need that pinpoint insert, even at the cost of performance.
- no opaque types - you are free to extend the library, if needed

### Reliability

- complete type safety
- unit testing with [criterion](https://github.com/Snaipe/Criterion)
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
first letters of your target type:

suffix | type
---: | :---
none | any (`void*`)
`l` | `long`
`ul` | `unsigned long`
`i` | `int`
`ui` | `unsigned int`
`s` | `short`
`us` | `unsigned short`
`c` | `char`
`sc` | `signed char`
`uc` | `unsigned char`
`f` | `float`
`d` | `double`
`ld` | `long double`

`signed char` is the only explicitly signed type available, due to the fact that
it is not defined by the standard whether or not `char` alone is signed or
unsigned.

This division into generic and suffixed form without using macro functions
introduces a **lot** of redundancy and inflation to the code base and resulting
library size, which is a conscious choice on my part. **This library does not aim
to be small in size**. It prioritizes simplicity, convenience and flexibility.

## Installation

On a Linux system with `make`, simply clone the repo and run the following (if
necessary, as root):

	make install

To uninstall, run

	make uninstall

By default, the files will be copied to `/usr/local/lib`, `/usr/local/include`
and `/usr/local/share/man`. If you want them (un)installed directly at
`/usr` (or somewhere else),
override the `PREFIX` variable:

	make   install PREFIX=/usr
	make uninstall PREFIX=/usr

The Makefile produces two library files, `libstaple.so` and `libstaple.a`. The former
is a dynamically linked library, the latter is static.

## Unit Testing

All test units are stored in `test/src`. If you wish to run the tests, you will
need [criterion](https://github.com/Snaipe/Criterion).

To execute all tests for a single module `XYZ`, run

	make clean test_XYZ

(for example `make clean test_stack`). To execute all tests for all
modules, run

	make clean test

If any test fails on a release commit, please open an issue on GitHub or send me
an email: <randoragongamedev@gmail.com>
