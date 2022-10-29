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

unsigned long long sp_queue_popull(struct sp_queue *queue)
{
	unsigned long long ret;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return 0;
	}
	if (queue->size == 0) {
		error(("queue is empty"));
		return 0;
	}
	if (queue->elem_size != sizeof(unsigned long long)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(unsigned long long)));
		return 0;
	}
#endif
	ret = *(unsigned long long*)queue->head;
	if (queue->size != 1)
		sp_ringbuf_incr(&queue->head, queue->data, queue->capacity, queue->elem_size);
	--queue->size;
	return ret;
}

#else
typedef int prevent_empty_translation_unit;
#endif