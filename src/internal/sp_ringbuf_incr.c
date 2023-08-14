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

void sp_ringbuf_incr(void **ptr, const void *buf, size_t capacity, size_t elem_size)
{
	if (*ptr == (char*)buf + (capacity - 1) * elem_size)
		/* Discard const for semantic reasons - we're not writing into
		 * ptr, so it's safe, but the user might want to, which is why
		 * ptr itself cannot be const. TL;DR please the compiler. */
		*ptr = (void*)buf;
	else
		*ptr = (char*)(*ptr) + elem_size;
}
