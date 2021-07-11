#include "helpers.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <limits.h>

#ifndef SIZE_MAX
#define SIZE_MAX ((size_t)(-1))
#endif

void stderr_printf(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
}

int rnd_buffit(void **buf, size_t elem_size, size_t size, size_t *capacity)
{
	if (size == *capacity) {
		*capacity *= 2;
		*buf = realloc(*buf, *capacity * elem_size);
		if (*buf == NULL) {
			error(("realloc"));
			return 1;
		}
	}
	return 0;
}

int rnd_foomap(void *buf, size_t elem_size, size_t size, int (*foo)(void*))
{
	const void *const end = (char*)buf + size * elem_size;
	char *p = buf;
	while (p != end) {
		int err;
		if ((err = foo(p))) {
			error(("external function handler returned %d (non-0)", err));
			return 1;
		}
		p += elem_size;
	}
	return 0;
}

int rnd_size_try_add(size_t size, size_t amount)
{
	if (size > SIZE_MAX - amount) {
		error(("size_t overflow detected, unable to increment by %lu", amount));
		return 1;
	}
	return 0;
}
