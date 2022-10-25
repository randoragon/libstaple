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

short sp_queue_qremoves(struct sp_queue *queue, size_t idx)
{
	char *p;
	short ret;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return 0;
	}
	if (queue->elem_size != sizeof(short)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(short)));
		return 0;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	p = sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	ret = *(short*)p;
	*(short*)p = *(short*)queue->tail;
	if (queue->size != 1)
		sp_ringbuf_decr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	--queue->size;
	return ret;
}