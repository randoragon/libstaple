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