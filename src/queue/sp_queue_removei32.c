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
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
#include "../sp_queue.h"
#include "../internal.h"
#include <stdint.h>
#include <inttypes.h>

int32_t sp_queue_removei32(struct sp_queue *queue, size_t idx)
{
	int32_t ret;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return 0;
	}
	if (queue->elem_size != sizeof(int32_t)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(int32_t)));
		return 0;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	ret = *(int32_t*)sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	sp_ringbuf_remove(idx, queue->data, &queue->size, queue->capacity, queue->elem_size, &queue->head, &queue->tail);
	return ret;
}

#else
typedef int prevent_empty_translation_unit;
#endif