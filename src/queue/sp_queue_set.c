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

int sp_queue_set(struct sp_queue *queue, size_t idx, void *val)
{
	char *p;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (val == NULL) {
		error(("val is NULL"));
		return SP_EINVAL;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	p = sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	memcpy(p, val, queue->elem_size);
	return 0;
}
