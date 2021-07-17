# the rnd library

`rnd` is a general-purpose library implementing common data structures in pure
ANSI C without any external dependencies besides the C standard library.

## Supported Data Structures

- stack

## Pending Implementations

- array,
- queue,
- hashmap,
- linked list,
- 2D/3D matrix,
- priority queue,
- avl tree, rbtree

## Installation

On a Linux system with `make`, simply clone the repo and run the following (if
necessary, as root):

	make install

To uninstall, run

	make uninstall

By default, the files will be copied to `/usr/local/lib`, `/usr/local/include`
and `/usr/local/share/man/man3`. If you want them (un)installed directly at
`/usr` (or somewhere else),
override the `PREFIX` variable:

	make   install PREFIX=/usr
	make uninstall PREFIX=/usr

## Library Design

1. The library should be as flexible as reasonably possible, while remaining
   very intuitive to the user.
2. No typedefs, because explicit `struct` notation is preferred for clarity (the
   user will add their own typedefs if they want).
3. Documentation as man pages, because it's a format Unix developers are
   familiar with, doesn't require heavy machinery to work with, and has
   extremely fast and offline access time.
4. Comments in code are kept to a minimum, because good code speaks for itself,
   and long explanations in comments are hard to maintain and cripple
   readability.
5. Library can be compiled for release and debug code separately, release code
   does less error checking and therefore is faster.
6. Sometimes error checking is sacrificed in favor of convenience, but only in
   cases where errors can only result from library misuse (e.g. popping from an
   empty stack).

### Abstractions Vs. Primitives

Every major function for inserting, removing, getting and setting elements is
available in 2 forms:

- **abstract form** with `void*` - allows you to store any structure
- **suffixed form** for all major primitive types

Example:

- `rnd_stack_push()` - reads from a `void*`, generic
- `rnd_stack_pushi()` - pass immediate `int` value
- `rnd_stack_pushus()` - pass immediate `unsigned short` value

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

This division into abstract and suffixed form introduces a **lot** of redundancy
and inflation to the code base, but I believe it's worthwhile to spend 3 times
longer writing a library that you're going to use for a long time, even if it
constitutes only a small speed improvement.
