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

struct sp_queue *sp_queue_create(size_t elem_size, size_t capacity)
{
	struct sp_queue *ret;

#ifdef STAPLE_DEBUG
	if (elem_size == 0) {
		error(("elem_size cannot be 0"));
		return NULL;
	}
	if (capacity == 0) {
		error(("capacity cannot be 0"));
		return NULL;
	}
#endif
	if (capacity > SP_SIZE_MAX / elem_size) {
		error(("size_t overflow detected, maximum size exceeded"));
		return NULL;
	}

	ret = malloc(sizeof(*ret));
	if (ret == NULL) {
		error(("malloc"));
		return NULL;
	}

	ret->elem_size = elem_size;
	ret->size      = 0;
	ret->capacity  = capacity;
	ret->data      = malloc(capacity * elem_size);
	if (ret->data == NULL) {
		error(("malloc"));
		free(ret);
		return NULL;
	}
	ret->head = ret->tail = ret->data;

	return ret;
}
