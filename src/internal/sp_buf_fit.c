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
			error(("realloc"));
			return 1;
		}
	}
	return 0;
}
