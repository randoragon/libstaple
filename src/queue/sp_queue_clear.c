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
#include "../helpers.h"
#include "../sp_errcodes.h"

int sp_queue_clear(struct sp_queue *queue, int (*dtor)(void*))
{
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
#endif
	if (dtor != NULL)
		while (queue->size != 0) {
			int err;
			if ((err = dtor(queue->head))) {
				error(("external function dtor returned %d (non-0)", err));
				return SP_EHANDLER;
			}
			sp_ringbuf_incr(&queue->head, queue->data, queue->capacity, queue->elem_size);
			queue->size--;
		}
	queue->size = 0;
	queue->head = queue->tail;
	return 0;
}