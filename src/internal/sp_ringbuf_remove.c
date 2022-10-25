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