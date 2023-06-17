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

int sp_queue_map(struct sp_queue *queue, int (*func)(void*, size_t))
{
	size_t i;
	void *p;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (func == NULL) {
		error(("func is NULL"));
		return SP_EINVAL;
	}
#endif
	p = queue->head;
	i = 0;
	while (i != queue->size) {
		int err;
		if ((err = func(p, i))) {
			error(("callback function func returned %d (non-0)", err));
			return SP_ECALLBK;
		}
		sp_ringbuf_incr(&p, queue->data, queue->capacity, queue->elem_size);
		++i;
	}
	return 0;
}
