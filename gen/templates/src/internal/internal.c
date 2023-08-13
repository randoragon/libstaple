#include "../internal.h"
#include <stdlib.h>

/*F{*/
#include <stdio.h>
#include <stdarg.h>
void stderr_printf(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
}
/*F}*/

/*F{*/
#include <limits.h>
int sp_buf_fit(void **buf, size_t size, size_t *capacity, size_t elem_size)
{
	if (size == *capacity) {
		if (!sp_size_try_add(*capacity, *capacity)) {
			*capacity *= 2;
		} else if (size < SP_SIZE_MAX / elem_size) {
			*capacity = SP_SIZE_MAX / elem_size;
		} else {
			error(("size_t overflow detected, stack size limit reached"));
			return 2;
		}
		*buf = realloc(*buf, *capacity * elem_size);
		if (*buf == NULL) {
			/*. C_ERRMSG_REALLOC */
			return 1;
		}
	}
	return 0;
}
/*F}*/

/*F{*/
#include <limits.h>
int sp_boolbuf_fit(void **buf, size_t size, size_t *capacity)
{
	if (size == *capacity) {
		if (!sp_size_try_add(*capacity, *capacity)) {
			*capacity *= 2;
		} else if (size < SP_SIZE_MAX) {
			*capacity = SP_SIZE_MAX;
		} else {
			error(("size_t overflow detected, stack size limit reached"));
			return 2;
		}
		*buf = realloc(*buf, BITS_TO_BYTES(*capacity));
		if (*buf == NULL) {
			/*. C_ERRMSG_REALLOC */
			return 1;
		}
	}
	return 0;
}
/*F}*/

/*F{*/
#include <string.h>
/* Same as sp_buf_fit, but for ring buffers.
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
 * Return values are identical to sp_buf_fit.
 */
int sp_ringbuf_fit(void **buf, size_t size, size_t *capacity, size_t elem_size, void **head, void **tail)
{
	if (size == *capacity) {
		const size_t head_offset = *(char**)head - *(char**)buf;
		void *dest;

		int error;
		if ((error = sp_buf_fit(buf, size, capacity, elem_size)))
			return error;

		*head = (char*)(*buf) + head_offset;
		*tail = (char*)(*head) + (size - 1) * elem_size;
		dest  = (char*)(*buf) + size * elem_size;
		memcpy(dest, *buf, head_offset);
	}
	return 0;
}
/*F}*/

/*F{*/
int sp_foomap(void *buf, size_t size, size_t elem_size, int (*foo)(void*))
{
	const void *const end = (char*)buf + size * elem_size;
	char *p = buf;
	while (p != end) {
		int err;
		if ((err = foo(p))) {
			/*. C_ERRMSG_CALLBACK_NON_ZERO foo err */
			return 1;
		}
		p += elem_size;
	}
	return 0;
}
/*F}*/

/*F{*/
#include <limits.h>
int sp_size_try_add(size_t size, size_t amount)
{
	if (size > SP_SIZE_MAX - amount) {
		error(("size_t overflow detected, unable to increment by %lu", (unsigned long)amount));
		return 1;
	}
	return 0;
}
/*F}*/

/*F{*/
void sp_ringbuf_incr(void **ptr, void *buf, size_t capacity, size_t elem_size)
{
	if (*ptr == (char*)buf + (capacity - 1) * elem_size)
		*ptr = buf;
	else
		*ptr = (char*)(*ptr) + elem_size;
}
/*F}*/

/*F{*/
void sp_ringbuf_decr(void **ptr, void *buf, size_t capacity, size_t elem_size)
{
	if (*ptr == buf)
		*ptr = (char*)buf + (capacity - 1) * elem_size;
	else
		*ptr = (char*)(*ptr) - elem_size;
}
/*F}*/

/*F{*/
/* Return address of nth element in a ring buffer. The obvious way to find a
 * specific index is by looping, but this function calculates it faster.
 * index 0 gives head, index size-1 gives tail.
 */
void *sp_ringbuf_get(size_t idx, const void *buf, size_t capacity, size_t elem_size, const void *head)
{
	const char *const buf_end = (char*)buf + (capacity - 1) * elem_size;
	const size_t no_elements_ahead = 1 + (buf_end - (char*)head) / elem_size;
	if (idx < no_elements_ahead)
		return (char*)head + idx * elem_size;
	else
		return (char*)buf + (idx - no_elements_ahead) * elem_size;
}
/*F}*/

/*F{*/
#include <string.h>
/* Insert an element into a ring buffer. The buffer must already have sufficient
 * capacity. Indexing starts from left to right, valid index values are in range
 * <0;size>. */
void sp_ringbuf_insert(const void *elem, size_t idx, void *buf, size_t *size, size_t capacity, size_t elem_size, void **head, void **tail)
{
	void *s, *d;
	size_t i;
	/* The new element splits the original buffer into 2 sub-buffers. Since
	 * the entire buffer is circular, we can choose which of the 2
	 * sub-buffers to shift away to make room. We pick the smaller one to
	 * minimize the number of shifts.
	 */
	if (*size != 0) {
		if (idx < *size / 2) {
			s = *head;
			sp_ringbuf_decr(head, buf, capacity, elem_size);
			d = *head;
			i = 0;
			while (i != idx) {
				memcpy(d, s, elem_size);
				d = s;
				sp_ringbuf_incr(&s, buf, capacity, elem_size);
				++i;
			}
		} else {
			s = *tail;
			sp_ringbuf_incr(tail, buf, capacity, elem_size);
			d = *tail;
			i = *size;
			while (i != idx) {
				memcpy(d, s, elem_size);
				d = s;
				sp_ringbuf_decr(&s, buf, capacity, elem_size);
				--i;
			}
		}
	} else {
		d = *tail;
	}
	memcpy(d, elem, elem_size);
	++(*size);
}
/*F}*/

/*F{*/
#include <string.h>
/* Remove an element from a ring buffer. Indexing starts from left to right,
 * valid index values are in range <0;size>.
 */
void sp_ringbuf_remove(size_t idx, void *buf, size_t *size, size_t capacity, size_t elem_size, void **head, void **tail)
{
	void *s, *d;
	/* The gap from the removed element splits the original buffer into 2
	 * sub-buffers.  Since the entire buffer is circular, we can choose
	 * which of the 2 sub-buffers to shift to cover the gap. We pick the
	 * smaller one to minimize the number of shifts.
	 */
	if (*size != 1) {
		if (idx < *size / 2) {
			s = d = sp_ringbuf_get(idx, buf, capacity, elem_size, *head);
			sp_ringbuf_decr(&s, buf, capacity, elem_size);
			while (idx != 0) {
				memcpy(d, s, elem_size);
				d = s;
				sp_ringbuf_decr(&s, buf, capacity, elem_size);
				--idx;
			}
			sp_ringbuf_incr(head, buf, capacity, elem_size);
		} else {
			s = d = sp_ringbuf_get(idx, buf, capacity, elem_size, *head);
			sp_ringbuf_incr(&s, buf, capacity, elem_size);
			while (idx != *size - 1) {
				memcpy(d, s, elem_size);
				d = s;
				sp_ringbuf_incr(&s, buf, capacity, elem_size);
				++idx;
			}
			sp_ringbuf_decr(tail, buf, capacity, elem_size);
		}
	}
	--(*size);
}
/*F}*/

/*F{*/
size_t sp_strnlen(const char *s, size_t maxlen)
{
	size_t i = 0;
	for (; i < maxlen && s[i] != '\0'; i++);
	return i;
}
/*F}*/
