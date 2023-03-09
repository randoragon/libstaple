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
