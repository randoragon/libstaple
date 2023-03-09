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
#include "../internal.h"
#include <stdlib.h>
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
