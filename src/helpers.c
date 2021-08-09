#include "helpers.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <limits.h>
#include <string.h>

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
		if (!rnd_size_try_add(*capacity, *capacity)) {
			*capacity *= 2;
		} else if (size < SIZE_MAX / elem_size) {
			*capacity = SIZE_MAX / elem_size;
		} else {
			error(("size_t overflow detected, stack size limit reached"));
			return 2;
		}
		*buf = realloc(*buf, *capacity * elem_size);
		if (*buf == NULL) {
			error(("realloc"));
			return 1;
		}
	}
	return 0;
}

/* Same as rnd_buffit, but for ring buffers.
 * Example (numbers denote order of insertion):
 * 	(1)	4 5 1 2 3
 * 	we start with a queue of capacity 5. Suppose we want to insert 6. First,
 * 	the buffer is enlarged:
 * 	(2)	4 5 1 2 3 _ _ _ _ _
 * 	the above ring buffer would break if a new element were to be inserted
 * 	after 5! To fix this, we shift the "tail part":
 * 	(3)	_ _ 1 2 3 4 5 _ _ _
 * 	then, the ring buffer can be used as normal:
 * 	(4)	_ _ 1 2 3 4 5 6 _ _
 * Return values are identical to rnd_buffit.
 */
int rnd_ringbuffit(void **buf, size_t elem_size, size_t size, size_t *capacity, void **head, void **tail)
{
	if (size == *capacity) {
		const size_t head_offset = *(char*)head - *(char*)buf;
		void *dest;

		int error;
		if ((error = rnd_buffit(buf, elem_size, size, capacity)))
			return error;

		*head = (char*)(*buf) + head_offset;
		*tail = (char*)(*head) + (size - 1) * elem_size;
		dest  = (char*)(*buf) + size * elem_size;
		memcpy(dest, buf, head_offset);
	}
	return 0;
}

int rnd_foomap(void *buf, size_t size, size_t elem_size, int (*foo)(void*))
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
		error(("size_t overflow detected, unable to increment by %lu", (unsigned long)amount));
		return 1;
	}
	return 0;
}

void rnd_ringbuf_incr(void **ptr, void *buf, size_t size, size_t elem_size)
{
	if (*ptr == (char*)buf + (size - 1) * elem_size)
		*ptr = buf;
	else
		*ptr = (char*)ptr + elem_size;
}
