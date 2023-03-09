/*  Staple - A general-purpose data structure library in pure C89.
 *  Copyright (C) 2021  Randoragon
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation;
 *  version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include "../sp_queue.h"
#include "../internal.h"
#include <string.h>

int sp_queue_eq(const struct sp_queue *queue1, const struct sp_queue *queue2, int (*cmp)(const void*, const void*))
{
#ifdef STAPLE_DEBUG
	if (queue1 == NULL) {
		error(("queue1 is NULL"));
		return 0;
	}
	if (queue2 == NULL) {
		error(("queue2 is NULL"));
		return 0;
	}
#endif
	size_t i = queue1->size;
	void *p = queue1->head,
	     *q = queue2->head;
	if (queue1->elem_size != queue2->elem_size || queue1->size != queue2->size)
		return 0;
	/* Possible area for optimization: when cmp == NULL, don't run memcmp on
	 * an element-by-element basis; instead calculate the largest possible
	 * slices of memory to compare in queue1 and queue2 to minimize the
	 * number of function calls (similar to how sp_stack_eq does it, but
	 * suited for ring buffers). */
	while (i != 0) {
		if (cmp ? cmp(p, q) : memcmp(p, q, queue1->elem_size))
			return 0;
		sp_ringbuf_incr(&p, queue1->data, queue1->capacity, queue1->elem_size);
		sp_ringbuf_incr(&q, queue1->data, queue1->capacity, queue1->elem_size);
		--i;
	}
	return 1;
}
