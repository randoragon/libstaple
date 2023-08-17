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
#include "../sp_errcodes.h"
#include <string.h>

int sp_queue_copy(struct sp_queue *dest, const struct sp_queue *src, int (*cpy)(void*, const void*))
{
#ifdef STAPLE_DEBUG
	if (src == NULL) {
		error(("src is NULL"));
		return SP_EINVAL;
	}
	if (dest == NULL) {
		error(("dest is NULL"));
		return SP_EINVAL;
	}
#endif
	if (DATA_SIZE(dest) < DATA_SIZE(src)) {
		dest->data = realloc(dest->data, DATA_SIZE(src));
		if (dest->data == NULL) {
			error(("realloc"));
			return SP_ENOMEM;
		}
		dest->capacity = src->size;
	}
	dest->elem_size = src->elem_size;
	dest->size      = src->size;
	dest->head      = dest->data;
	dest->tail      = dest->data;
	if (src->size != 0) {
		void  *s = src->head;
		size_t i = src->size;
		sp_ringbuf_decr(&s, src->data, src->capacity, src->elem_size);
		sp_ringbuf_decr(&dest->tail, dest->data, dest->capacity, dest->elem_size);
		if (cpy == NULL)
			while (i != 0) {
				sp_ringbuf_incr(&s, src->data, src->capacity, src->elem_size);
				sp_ringbuf_incr(&dest->tail, dest->data, dest->capacity, dest->elem_size);
				memcpy(dest->tail, s, src->elem_size);
				--i;
			}
		else
			while (i != 0) {
				int err;
				sp_ringbuf_incr(&s, src->data, src->capacity, src->elem_size);
				sp_ringbuf_incr(&dest->tail, dest->data, dest->capacity, dest->elem_size);
				if ((err = cpy(dest->tail, s))) {
					error(("callback function cpy returned %d (non-0)", err));
					return SP_ECALLBK;
				}
				--i;
			}
	}
	return 0;
}
