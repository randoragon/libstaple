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

int sp_queue_print(const struct sp_queue *queue, int (*func)(const void*))
{
	size_t i;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
#endif
	printf("sp_queue_print()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)queue->size, (unsigned long)queue->capacity, (unsigned long)queue->elem_size);
	if (func == NULL)
		for (i = 0; i < queue->size; i++) {
			const void *const elem = sp_ringbuf_get(i, queue->data, queue->capacity, queue->elem_size, queue->head);
			printf("[%lu]\t%p\n", (unsigned long)i, elem);
		}
	else
		for (i = 0; i < queue->size; i++) {
			const void *const elem = sp_ringbuf_get(i, queue->data, queue->capacity, queue->elem_size, queue->head);
			int err;
			printf("[%lu]\t", (unsigned long)i);
			if ((err = func(elem)) != 0) {
				error(("callback function func returned %d (non-0)", err));
				return SP_ECALLBK;
			}
		}
	return 0;
}
