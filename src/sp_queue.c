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
#include "sp_queue.h"
#include "helpers.h"
#include "sp_errcodes.h"
#include <string.h>

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
	if (capacity > SIZE_MAX / elem_size) {
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

int sp_queue_clear(struct sp_queue *queue, int (*dtor)(void*))
{
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
#endif
	if (dtor != NULL) {
		while (queue->size != 0) {
			int error;
			if ((error = dtor(queue->head))) {
				error(("external dtor function returned %d (non-0)", error));
				return SP_EHANDLER;
			}
			sp_ringbuf_incr(&queue->head, queue->data, queue->capacity, queue->elem_size);
			queue->size--;
		}
	} else {
		queue->size = 0;
		queue->head = queue->tail;
	}
	return 0;
}

int sp_queue_destroy(struct sp_queue *queue, int (*dtor)(void*))
{
	int error;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
#endif
	if ((error = sp_queue_clear(queue, dtor)))
		return error;
	free(queue->data);
	free(queue);
	return 0;
}

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
	if (dest->capacity * dest->elem_size < src->size * src->elem_size) {
		dest->capacity = src->size;
		dest->data = realloc(dest->data, dest->capacity * src->elem_size);
		if (dest->data == NULL) {
			error(("realloc"));
			return SP_ENOMEM;
		}
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
					error(("external cpy function returned %d (non-0)", err));
					return SP_EHANDLER;
				}
				--i;
			}
	}
	return 0;
}

int sp_queue_foreach(struct sp_queue *queue, int (*func)(void*, size_t))
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
	while (p != queue->tail) {
		int err;
		if ((err = func(p, i))) {
			warn(("external func function returned %d (non-0)", err));
			return SP_EHANDLER;
		}
		sp_ringbuf_incr(&p, queue->data, queue->capacity, queue->elem_size);
		++i;
	}
	return 0;
}

int sp_queue_push(struct sp_queue *queue, const void *elem)
{
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (elem == NULL) {
		error(("elem is NULL"));
		return SP_EINVAL;
	}
#endif
	if (sp_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return SP_ERANGE;
	if (sp_ringbuf_fit(&queue->data, queue->size, &queue->capacity, queue->elem_size, &queue->head, &queue->tail))
		return SP_ENOMEM;
	if (queue->size != 0)
		sp_ringbuf_incr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	memcpy(queue->tail, elem, queue->elem_size);
	++queue->size;
	return 0;
}

int sp_queue_pushc(struct sp_queue *queue, char elem)
{
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
#endif
	if (sp_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return SP_ERANGE;
	if (sp_ringbuf_fit(&queue->data, queue->size, &queue->capacity, queue->elem_size, &queue->head, &queue->tail))
		return SP_ENOMEM;
	if (queue->size != 0)
		sp_ringbuf_incr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	*(char*)queue->tail = elem;
	++queue->size;
	return 0;
}

int sp_queue_pushs(struct sp_queue *queue, short elem)
{
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
#endif
	if (sp_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return SP_ERANGE;
	if (sp_ringbuf_fit(&queue->data, queue->size, &queue->capacity, queue->elem_size, &queue->head, &queue->tail))
		return SP_ENOMEM;
	if (queue->size != 0)
		sp_ringbuf_incr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	*(short*)queue->tail = elem;
	++queue->size;
	return 0;
}

int sp_queue_pushi(struct sp_queue *queue, int elem)
{
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
#endif
	if (sp_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return SP_ERANGE;
	if (sp_ringbuf_fit(&queue->data, queue->size, &queue->capacity, queue->elem_size, &queue->head, &queue->tail))
		return SP_ENOMEM;
	if (queue->size != 0)
		sp_ringbuf_incr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	*(int*)queue->tail = elem;
	++queue->size;
	return 0;
}

int sp_queue_pushl(struct sp_queue *queue, long elem)
{
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
#endif
	if (sp_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return SP_ERANGE;
	if (sp_ringbuf_fit(&queue->data, queue->size, &queue->capacity, queue->elem_size, &queue->head, &queue->tail))
		return SP_ENOMEM;
	if (queue->size != 0)
		sp_ringbuf_incr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	*(long*)queue->tail = elem;
	++queue->size;
	return 0;
}

int sp_queue_pushsc(struct sp_queue *queue, signed char elem)
{
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
#endif
	if (sp_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return SP_ERANGE;
	if (sp_ringbuf_fit(&queue->data, queue->size, &queue->capacity, queue->elem_size, &queue->head, &queue->tail))
		return SP_ENOMEM;
	if (queue->size != 0)
		sp_ringbuf_incr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	*(signed char*)queue->tail = elem;
	++queue->size;
	return 0;
}

int sp_queue_pushuc(struct sp_queue *queue, unsigned char elem)
{
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
#endif
	if (sp_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return SP_ERANGE;
	if (sp_ringbuf_fit(&queue->data, queue->size, &queue->capacity, queue->elem_size, &queue->head, &queue->tail))
		return SP_ENOMEM;
	if (queue->size != 0)
		sp_ringbuf_incr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	*(unsigned char*)queue->tail = elem;
	++queue->size;
	return 0;
}

int sp_queue_pushus(struct sp_queue *queue, unsigned short elem)
{
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
#endif
	if (sp_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return SP_ERANGE;
	if (sp_ringbuf_fit(&queue->data, queue->size, &queue->capacity, queue->elem_size, &queue->head, &queue->tail))
		return SP_ENOMEM;
	if (queue->size != 0)
		sp_ringbuf_incr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	*(unsigned short*)queue->tail = elem;
	++queue->size;
	return 0;
}

int sp_queue_pushui(struct sp_queue *queue, unsigned int elem)
{
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
#endif
	if (sp_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return SP_ERANGE;
	if (sp_ringbuf_fit(&queue->data, queue->size, &queue->capacity, queue->elem_size, &queue->head, &queue->tail))
		return SP_ENOMEM;
	if (queue->size != 0)
		sp_ringbuf_incr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	*(unsigned int*)queue->tail = elem;
	++queue->size;
	return 0;
}

int sp_queue_pushul(struct sp_queue *queue, unsigned long elem)
{
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
#endif
	if (sp_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return SP_ERANGE;
	if (sp_ringbuf_fit(&queue->data, queue->size, &queue->capacity, queue->elem_size, &queue->head, &queue->tail))
		return SP_ENOMEM;
	if (queue->size != 0)
		sp_ringbuf_incr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	*(unsigned long*)queue->tail = elem;
	++queue->size;
	return 0;
}

int sp_queue_pushf(struct sp_queue *queue, float elem)
{
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
#endif
	if (sp_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return SP_ERANGE;
	if (sp_ringbuf_fit(&queue->data, queue->size, &queue->capacity, queue->elem_size, &queue->head, &queue->tail))
		return SP_ENOMEM;
	if (queue->size != 0)
		sp_ringbuf_incr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	*(float*)queue->tail = elem;
	++queue->size;
	return 0;
}

int sp_queue_pushd(struct sp_queue *queue, double elem)
{
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
#endif
	if (sp_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return SP_ERANGE;
	if (sp_ringbuf_fit(&queue->data, queue->size, &queue->capacity, queue->elem_size, &queue->head, &queue->tail))
		return SP_ENOMEM;
	if (queue->size != 0)
		sp_ringbuf_incr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	*(double*)queue->tail = elem;
	++queue->size;
	return 0;
}

int sp_queue_pushld(struct sp_queue *queue, long double elem)
{
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
#endif
	if (sp_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return SP_ERANGE;
	if (sp_ringbuf_fit(&queue->data, queue->size, &queue->capacity, queue->elem_size, &queue->head, &queue->tail))
		return SP_ENOMEM;
	if (queue->size != 0)
		sp_ringbuf_incr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	*(long double*)queue->tail = elem;
	++queue->size;
	return 0;
}


int sp_queue_insert(struct sp_queue *queue, size_t idx, const void *elem)
{
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (elem == NULL) {
		error(("elem is NULL"));
		return SP_EINVAL;
	}
	if (idx > queue->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return SP_ERANGE;
	if (sp_ringbuf_fit(&queue->data, queue->size, &queue->capacity, queue->elem_size, &queue->head, &queue->tail))
		return SP_ENOMEM;
	sp_ringbuf_insert(elem, idx, queue->data, &queue->size, queue->capacity, queue->elem_size, &queue->head, &queue->tail);
	return 0;
}

int sp_queue_insertc(struct sp_queue *queue, size_t idx, char elem)
{
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
	if (idx > queue->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return SP_ERANGE;
	if (sp_ringbuf_fit(&queue->data, queue->size, &queue->capacity, queue->elem_size, &queue->head, &queue->tail))
		return SP_ENOMEM;
	sp_ringbuf_insert(&elem, idx, queue->data, &queue->size, queue->capacity, queue->elem_size, &queue->head, &queue->tail);
	return 0;
}

int sp_queue_inserts(struct sp_queue *queue, size_t idx, short elem)
{
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
	if (idx > queue->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return SP_ERANGE;
	if (sp_ringbuf_fit(&queue->data, queue->size, &queue->capacity, queue->elem_size, &queue->head, &queue->tail))
		return SP_ENOMEM;
	sp_ringbuf_insert(&elem, idx, queue->data, &queue->size, queue->capacity, queue->elem_size, &queue->head, &queue->tail);
	return 0;
}

int sp_queue_inserti(struct sp_queue *queue, size_t idx, int elem)
{
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
	if (idx > queue->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return SP_ERANGE;
	if (sp_ringbuf_fit(&queue->data, queue->size, &queue->capacity, queue->elem_size, &queue->head, &queue->tail))
		return SP_ENOMEM;
	sp_ringbuf_insert(&elem, idx, queue->data, &queue->size, queue->capacity, queue->elem_size, &queue->head, &queue->tail);
	return 0;
}

int sp_queue_insertl(struct sp_queue *queue, size_t idx, long elem)
{
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
	if (idx > queue->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return SP_ERANGE;
	if (sp_ringbuf_fit(&queue->data, queue->size, &queue->capacity, queue->elem_size, &queue->head, &queue->tail))
		return SP_ENOMEM;
	sp_ringbuf_insert(&elem, idx, queue->data, &queue->size, queue->capacity, queue->elem_size, &queue->head, &queue->tail);
	return 0;
}

int sp_queue_insertsc(struct sp_queue *queue, size_t idx, signed char elem)
{
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
	if (idx > queue->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return SP_ERANGE;
	if (sp_ringbuf_fit(&queue->data, queue->size, &queue->capacity, queue->elem_size, &queue->head, &queue->tail))
		return SP_ENOMEM;
	sp_ringbuf_insert(&elem, idx, queue->data, &queue->size, queue->capacity, queue->elem_size, &queue->head, &queue->tail);
	return 0;
}

int sp_queue_insertuc(struct sp_queue *queue, size_t idx, unsigned char elem)
{
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
	if (idx > queue->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return SP_ERANGE;
	if (sp_ringbuf_fit(&queue->data, queue->size, &queue->capacity, queue->elem_size, &queue->head, &queue->tail))
		return SP_ENOMEM;
	sp_ringbuf_insert(&elem, idx, queue->data, &queue->size, queue->capacity, queue->elem_size, &queue->head, &queue->tail);
	return 0;
}

int sp_queue_insertus(struct sp_queue *queue, size_t idx, unsigned short elem)
{
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
	if (idx > queue->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return SP_ERANGE;
	if (sp_ringbuf_fit(&queue->data, queue->size, &queue->capacity, queue->elem_size, &queue->head, &queue->tail))
		return SP_ENOMEM;
	sp_ringbuf_insert(&elem, idx, queue->data, &queue->size, queue->capacity, queue->elem_size, &queue->head, &queue->tail);
	return 0;
}

int sp_queue_insertui(struct sp_queue *queue, size_t idx, unsigned int elem)
{
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
	if (idx > queue->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return SP_ERANGE;
	if (sp_ringbuf_fit(&queue->data, queue->size, &queue->capacity, queue->elem_size, &queue->head, &queue->tail))
		return SP_ENOMEM;
	sp_ringbuf_insert(&elem, idx, queue->data, &queue->size, queue->capacity, queue->elem_size, &queue->head, &queue->tail);
	return 0;
}

int sp_queue_insertul(struct sp_queue *queue, size_t idx, unsigned long elem)
{
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
	if (idx > queue->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return SP_ERANGE;
	if (sp_ringbuf_fit(&queue->data, queue->size, &queue->capacity, queue->elem_size, &queue->head, &queue->tail))
		return SP_ENOMEM;
	sp_ringbuf_insert(&elem, idx, queue->data, &queue->size, queue->capacity, queue->elem_size, &queue->head, &queue->tail);
	return 0;
}

int sp_queue_insertf(struct sp_queue *queue, size_t idx, float elem)
{
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
	if (idx > queue->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return SP_ERANGE;
	if (sp_ringbuf_fit(&queue->data, queue->size, &queue->capacity, queue->elem_size, &queue->head, &queue->tail))
		return SP_ENOMEM;
	sp_ringbuf_insert(&elem, idx, queue->data, &queue->size, queue->capacity, queue->elem_size, &queue->head, &queue->tail);
	return 0;
}

int sp_queue_insertd(struct sp_queue *queue, size_t idx, double elem)
{
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
	if (idx > queue->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return SP_ERANGE;
	if (sp_ringbuf_fit(&queue->data, queue->size, &queue->capacity, queue->elem_size, &queue->head, &queue->tail))
		return SP_ENOMEM;
	sp_ringbuf_insert(&elem, idx, queue->data, &queue->size, queue->capacity, queue->elem_size, &queue->head, &queue->tail);
	return 0;
}

int sp_queue_insertld(struct sp_queue *queue, size_t idx, long double elem)
{
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
	if (idx > queue->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return SP_ERANGE;
	if (sp_ringbuf_fit(&queue->data, queue->size, &queue->capacity, queue->elem_size, &queue->head, &queue->tail))
		return SP_ENOMEM;
	sp_ringbuf_insert(&elem, idx, queue->data, &queue->size, queue->capacity, queue->elem_size, &queue->head, &queue->tail);
	return 0;
}


int sp_queue_qinsert(struct sp_queue *queue, size_t idx, const void *elem)
{
	char *p;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (elem == NULL) {
		error(("elem is NULL"));
		return SP_EINVAL;
	}
	if (idx > queue->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return SP_ERANGE;
	if (sp_ringbuf_fit(&queue->data, queue->size, &queue->capacity, queue->elem_size, &queue->head, &queue->tail))
		return SP_ENOMEM;
	if (queue->size != 0)
		sp_ringbuf_incr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	p = sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	memcpy(queue->tail, p, queue->elem_size);
	memcpy(p, elem, queue->elem_size);
	++queue->size;
	return 0;
}

int sp_queue_qinsertc(struct sp_queue *queue, size_t idx, char elem)
{
	char *p;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
	if (idx > queue->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return SP_ERANGE;
	if (sp_ringbuf_fit(&queue->data, queue->size, &queue->capacity, queue->elem_size, &queue->head, &queue->tail))
		return SP_ENOMEM;
	if (queue->size != 0)
		sp_ringbuf_incr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	p = sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	*(char*)queue->tail = *(char*)p;
	*(char*)p = elem;
	++queue->size;
	return 0;
}

int sp_queue_qinserts(struct sp_queue *queue, size_t idx, short elem)
{
	char *p;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
	if (idx > queue->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return SP_ERANGE;
	if (sp_ringbuf_fit(&queue->data, queue->size, &queue->capacity, queue->elem_size, &queue->head, &queue->tail))
		return SP_ENOMEM;
	if (queue->size != 0)
		sp_ringbuf_incr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	p = sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	*(short*)queue->tail = *(short*)p;
	*(short*)p = elem;
	++queue->size;
	return 0;
}

int sp_queue_qinserti(struct sp_queue *queue, size_t idx, int elem)
{
	char *p;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
	if (idx > queue->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return SP_ERANGE;
	if (sp_ringbuf_fit(&queue->data, queue->size, &queue->capacity, queue->elem_size, &queue->head, &queue->tail))
		return SP_ENOMEM;
	if (queue->size != 0)
		sp_ringbuf_incr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	p = sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	*(int*)queue->tail = *(int*)p;
	*(int*)p = elem;
	++queue->size;
	return 0;
}

int sp_queue_qinsertl(struct sp_queue *queue, size_t idx, long elem)
{
	char *p;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
	if (idx > queue->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return SP_ERANGE;
	if (sp_ringbuf_fit(&queue->data, queue->size, &queue->capacity, queue->elem_size, &queue->head, &queue->tail))
		return SP_ENOMEM;
	if (queue->size != 0)
		sp_ringbuf_incr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	p = sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	*(long*)queue->tail = *(long*)p;
	*(long*)p = elem;
	++queue->size;
	return 0;
}

int sp_queue_qinsertsc(struct sp_queue *queue, size_t idx, signed char elem)
{
	char *p;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
	if (idx > queue->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return SP_ERANGE;
	if (sp_ringbuf_fit(&queue->data, queue->size, &queue->capacity, queue->elem_size, &queue->head, &queue->tail))
		return SP_ENOMEM;
	if (queue->size != 0)
		sp_ringbuf_incr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	p = sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	*(signed char*)queue->tail = *(signed char*)p;
	*(signed char*)p = elem;
	++queue->size;
	return 0;
}

int sp_queue_qinsertuc(struct sp_queue *queue, size_t idx, unsigned char elem)
{
	char *p;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
	if (idx > queue->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return SP_ERANGE;
	if (sp_ringbuf_fit(&queue->data, queue->size, &queue->capacity, queue->elem_size, &queue->head, &queue->tail))
		return SP_ENOMEM;
	if (queue->size != 0)
		sp_ringbuf_incr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	p = sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	*(unsigned char*)queue->tail = *(unsigned char*)p;
	*(unsigned char*)p = elem;
	++queue->size;
	return 0;
}

int sp_queue_qinsertus(struct sp_queue *queue, size_t idx, unsigned short elem)
{
	char *p;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
	if (idx > queue->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return SP_ERANGE;
	if (sp_ringbuf_fit(&queue->data, queue->size, &queue->capacity, queue->elem_size, &queue->head, &queue->tail))
		return SP_ENOMEM;
	if (queue->size != 0)
		sp_ringbuf_incr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	p = sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	*(unsigned short*)queue->tail = *(unsigned short*)p;
	*(unsigned short*)p = elem;
	++queue->size;
	return 0;
}

int sp_queue_qinsertui(struct sp_queue *queue, size_t idx, unsigned int elem)
{
	char *p;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
	if (idx > queue->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return SP_ERANGE;
	if (sp_ringbuf_fit(&queue->data, queue->size, &queue->capacity, queue->elem_size, &queue->head, &queue->tail))
		return SP_ENOMEM;
	if (queue->size != 0)
		sp_ringbuf_incr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	p = sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	*(unsigned int*)queue->tail = *(unsigned int*)p;
	*(unsigned int*)p = elem;
	++queue->size;
	return 0;
}

int sp_queue_qinsertul(struct sp_queue *queue, size_t idx, unsigned long elem)
{
	char *p;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
	if (idx > queue->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return SP_ERANGE;
	if (sp_ringbuf_fit(&queue->data, queue->size, &queue->capacity, queue->elem_size, &queue->head, &queue->tail))
		return SP_ENOMEM;
	if (queue->size != 0)
		sp_ringbuf_incr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	p = sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	*(unsigned long*)queue->tail = *(unsigned long*)p;
	*(unsigned long*)p = elem;
	++queue->size;
	return 0;
}

int sp_queue_qinsertf(struct sp_queue *queue, size_t idx, float elem)
{
	char *p;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
	if (idx > queue->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return SP_ERANGE;
	if (sp_ringbuf_fit(&queue->data, queue->size, &queue->capacity, queue->elem_size, &queue->head, &queue->tail))
		return SP_ENOMEM;
	if (queue->size != 0)
		sp_ringbuf_incr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	p = sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	*(float*)queue->tail = *(float*)p;
	*(float*)p = elem;
	++queue->size;
	return 0;
}

int sp_queue_qinsertd(struct sp_queue *queue, size_t idx, double elem)
{
	char *p;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
	if (idx > queue->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return SP_ERANGE;
	if (sp_ringbuf_fit(&queue->data, queue->size, &queue->capacity, queue->elem_size, &queue->head, &queue->tail))
		return SP_ENOMEM;
	if (queue->size != 0)
		sp_ringbuf_incr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	p = sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	*(double*)queue->tail = *(double*)p;
	*(double*)p = elem;
	++queue->size;
	return 0;
}

int sp_queue_qinsertld(struct sp_queue *queue, size_t idx, long double elem)
{
	char *p;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
	if (idx > queue->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return SP_ERANGE;
	if (sp_ringbuf_fit(&queue->data, queue->size, &queue->capacity, queue->elem_size, &queue->head, &queue->tail))
		return SP_ENOMEM;
	if (queue->size != 0)
		sp_ringbuf_incr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	p = sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	*(long double*)queue->tail = *(long double*)p;
	*(long double*)p = elem;
	++queue->size;
	return 0;
}


int sp_queue_peek(const struct sp_queue *queue, void *output)
{
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (output == NULL) {
		error(("output is NULL"));
		return SP_EINVAL;
	}
	if (queue->size == 0) {
		error(("queue is empty"));
		return SP_EILLEGAL;
	}
#endif
	memcpy(output, queue->head, queue->elem_size);
	return 0;
}

char sp_queue_peekc(const struct sp_queue *queue)
{
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return 0;
	}
	if (queue->elem_size != sizeof(char)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(char)));
		return 0;
	}
	if (queue->size == 0) {
		error(("queue is empty"));
		return 0;
	}
#endif
	return *(char*)queue->head;
}

short sp_queue_peeks(const struct sp_queue *queue)
{
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
	if (queue->size == 0) {
		error(("queue is empty"));
		return 0;
	}
#endif
	return *(short*)queue->head;
}

int sp_queue_peeki(const struct sp_queue *queue)
{
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return 0;
	}
	if (queue->elem_size != sizeof(int)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(int)));
		return 0;
	}
	if (queue->size == 0) {
		error(("queue is empty"));
		return 0;
	}
#endif
	return *(int*)queue->head;
}

long sp_queue_peekl(const struct sp_queue *queue)
{
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return 0;
	}
	if (queue->elem_size != sizeof(long)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(long)));
		return 0;
	}
	if (queue->size == 0) {
		error(("queue is empty"));
		return 0;
	}
#endif
	return *(long*)queue->head;
}

signed char sp_queue_peeksc(const struct sp_queue *queue)
{
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return 0;
	}
	if (queue->elem_size != sizeof(signed char)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(signed char)));
		return 0;
	}
	if (queue->size == 0) {
		error(("queue is empty"));
		return 0;
	}
#endif
	return *(signed char*)queue->head;
}

unsigned char sp_queue_peekuc(const struct sp_queue *queue)
{
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return 0;
	}
	if (queue->elem_size != sizeof(unsigned char)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(unsigned char)));
		return 0;
	}
	if (queue->size == 0) {
		error(("queue is empty"));
		return 0;
	}
#endif
	return *(unsigned char*)queue->head;
}

unsigned short sp_queue_peekus(const struct sp_queue *queue)
{
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return 0;
	}
	if (queue->elem_size != sizeof(unsigned short)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(unsigned short)));
		return 0;
	}
	if (queue->size == 0) {
		error(("queue is empty"));
		return 0;
	}
#endif
	return *(unsigned short*)queue->head;
}

unsigned int sp_queue_peekui(const struct sp_queue *queue)
{
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return 0;
	}
	if (queue->elem_size != sizeof(unsigned int)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(unsigned int)));
		return 0;
	}
	if (queue->size == 0) {
		error(("queue is empty"));
		return 0;
	}
#endif
	return *(unsigned int*)queue->head;
}

unsigned long sp_queue_peekul(const struct sp_queue *queue)
{
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return 0;
	}
	if (queue->elem_size != sizeof(unsigned long)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(unsigned long)));
		return 0;
	}
	if (queue->size == 0) {
		error(("queue is empty"));
		return 0;
	}
#endif
	return *(unsigned long*)queue->head;
}

float sp_queue_peekf(const struct sp_queue *queue)
{
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return 0;
	}
	if (queue->elem_size != sizeof(float)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(float)));
		return 0;
	}
	if (queue->size == 0) {
		error(("queue is empty"));
		return 0;
	}
#endif
	return *(float*)queue->head;
}

double sp_queue_peekd(const struct sp_queue *queue)
{
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return 0;
	}
	if (queue->elem_size != sizeof(double)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(double)));
		return 0;
	}
	if (queue->size == 0) {
		error(("queue is empty"));
		return 0;
	}
#endif
	return *(double*)queue->head;
}

long double sp_queue_peekld(const struct sp_queue *queue)
{
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return 0;
	}
	if (queue->elem_size != sizeof(long double)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(long double)));
		return 0;
	}
	if (queue->size == 0) {
		error(("queue is empty"));
		return 0;
	}
#endif
	return *(long double*)queue->head;
}


int sp_queue_pop(struct sp_queue *queue, void *output)
{
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (queue->size == 0) {
		error(("queue is empty"));
		return SP_EILLEGAL;
	}
#endif
	if (output != NULL)
		memcpy(output, queue->head, queue->elem_size);
	if (queue->size != 1)
		sp_ringbuf_incr(&queue->head, queue->data, queue->capacity, queue->elem_size);
	--queue->size;
	return 0;
}

char sp_queue_popc(struct sp_queue *queue)
{
	char ret;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return 0;
	}
	if (queue->size == 0) {
		error(("queue is empty"));
		return 0;
	}
	if (queue->elem_size != sizeof(char)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(char)));
		return 0;
	}
#endif
	ret = *(char*)queue->head;
	if (queue->size != 1)
		sp_ringbuf_incr(&queue->head, queue->data, queue->capacity, queue->elem_size);
	--queue->size;
	return ret;
}

short sp_queue_pops(struct sp_queue *queue)
{
	short ret;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return 0;
	}
	if (queue->size == 0) {
		error(("queue is empty"));
		return 0;
	}
	if (queue->elem_size != sizeof(short)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(short)));
		return 0;
	}
#endif
	ret = *(short*)queue->head;
	if (queue->size != 1)
		sp_ringbuf_incr(&queue->head, queue->data, queue->capacity, queue->elem_size);
	--queue->size;
	return ret;
}

int sp_queue_popi(struct sp_queue *queue)
{
	int ret;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return 0;
	}
	if (queue->size == 0) {
		error(("queue is empty"));
		return 0;
	}
	if (queue->elem_size != sizeof(int)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(int)));
		return 0;
	}
#endif
	ret = *(int*)queue->head;
	if (queue->size != 1)
		sp_ringbuf_incr(&queue->head, queue->data, queue->capacity, queue->elem_size);
	--queue->size;
	return ret;
}

long sp_queue_popl(struct sp_queue *queue)
{
	long ret;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return 0;
	}
	if (queue->size == 0) {
		error(("queue is empty"));
		return 0;
	}
	if (queue->elem_size != sizeof(long)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(long)));
		return 0;
	}
#endif
	ret = *(long*)queue->head;
	if (queue->size != 1)
		sp_ringbuf_incr(&queue->head, queue->data, queue->capacity, queue->elem_size);
	--queue->size;
	return ret;
}

signed char sp_queue_popsc(struct sp_queue *queue)
{
	signed char ret;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return 0;
	}
	if (queue->size == 0) {
		error(("queue is empty"));
		return 0;
	}
	if (queue->elem_size != sizeof(signed char)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(signed char)));
		return 0;
	}
#endif
	ret = *(signed char*)queue->head;
	if (queue->size != 1)
		sp_ringbuf_incr(&queue->head, queue->data, queue->capacity, queue->elem_size);
	--queue->size;
	return ret;
}

unsigned char sp_queue_popuc(struct sp_queue *queue)
{
	unsigned char ret;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return 0;
	}
	if (queue->size == 0) {
		error(("queue is empty"));
		return 0;
	}
	if (queue->elem_size != sizeof(unsigned char)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(unsigned char)));
		return 0;
	}
#endif
	ret = *(unsigned char*)queue->head;
	if (queue->size != 1)
		sp_ringbuf_incr(&queue->head, queue->data, queue->capacity, queue->elem_size);
	--queue->size;
	return ret;
}

unsigned short sp_queue_popus(struct sp_queue *queue)
{
	unsigned short ret;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return 0;
	}
	if (queue->size == 0) {
		error(("queue is empty"));
		return 0;
	}
	if (queue->elem_size != sizeof(unsigned short)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(unsigned short)));
		return 0;
	}
#endif
	ret = *(unsigned short*)queue->head;
	if (queue->size != 1)
		sp_ringbuf_incr(&queue->head, queue->data, queue->capacity, queue->elem_size);
	--queue->size;
	return ret;
}

unsigned int sp_queue_popui(struct sp_queue *queue)
{
	unsigned int ret;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return 0;
	}
	if (queue->size == 0) {
		error(("queue is empty"));
		return 0;
	}
	if (queue->elem_size != sizeof(unsigned int)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(unsigned int)));
		return 0;
	}
#endif
	ret = *(unsigned int*)queue->head;
	if (queue->size != 1)
		sp_ringbuf_incr(&queue->head, queue->data, queue->capacity, queue->elem_size);
	--queue->size;
	return ret;
}

unsigned long sp_queue_popul(struct sp_queue *queue)
{
	unsigned long ret;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return 0;
	}
	if (queue->size == 0) {
		error(("queue is empty"));
		return 0;
	}
	if (queue->elem_size != sizeof(unsigned long)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(unsigned long)));
		return 0;
	}
#endif
	ret = *(unsigned long*)queue->head;
	if (queue->size != 1)
		sp_ringbuf_incr(&queue->head, queue->data, queue->capacity, queue->elem_size);
	--queue->size;
	return ret;
}

float sp_queue_popf(struct sp_queue *queue)
{
	float ret;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return 0;
	}
	if (queue->size == 0) {
		error(("queue is empty"));
		return 0;
	}
	if (queue->elem_size != sizeof(float)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(float)));
		return 0;
	}
#endif
	ret = *(float*)queue->head;
	if (queue->size != 1)
		sp_ringbuf_incr(&queue->head, queue->data, queue->capacity, queue->elem_size);
	--queue->size;
	return ret;
}

double sp_queue_popd(struct sp_queue *queue)
{
	double ret;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return 0;
	}
	if (queue->size == 0) {
		error(("queue is empty"));
		return 0;
	}
	if (queue->elem_size != sizeof(double)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(double)));
		return 0;
	}
#endif
	ret = *(double*)queue->head;
	if (queue->size != 1)
		sp_ringbuf_incr(&queue->head, queue->data, queue->capacity, queue->elem_size);
	--queue->size;
	return ret;
}

long double sp_queue_popld(struct sp_queue *queue)
{
	long double ret;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return 0;
	}
	if (queue->size == 0) {
		error(("queue is empty"));
		return 0;
	}
	if (queue->elem_size != sizeof(long double)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(long double)));
		return 0;
	}
#endif
	ret = *(long double*)queue->head;
	if (queue->size != 1)
		sp_ringbuf_incr(&queue->head, queue->data, queue->capacity, queue->elem_size);
	--queue->size;
	return ret;
}


int sp_queue_remove(struct sp_queue *queue, size_t idx, void *output)
{
	char *p;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	p = sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	if (output != NULL)
		memcpy(output, p, queue->elem_size);
	sp_ringbuf_remove(idx, queue->data, &queue->size, queue->capacity, queue->elem_size, &queue->head, &queue->tail);
	return 0;
}

char sp_queue_removec(struct sp_queue *queue, size_t idx)
{
	char ret;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return 0;
	}
	if (queue->elem_size != sizeof(char)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(char)));
		return 0;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	ret = *(char*)sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	sp_ringbuf_remove(idx, queue->data, &queue->size, queue->capacity, queue->elem_size, &queue->head, &queue->tail);
	return ret;
}

short sp_queue_removes(struct sp_queue *queue, size_t idx)
{
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
	ret = *(short*)sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	sp_ringbuf_remove(idx, queue->data, &queue->size, queue->capacity, queue->elem_size, &queue->head, &queue->tail);
	return ret;
}

int sp_queue_removei(struct sp_queue *queue, size_t idx)
{
	int ret;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return 0;
	}
	if (queue->elem_size != sizeof(int)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(int)));
		return 0;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	ret = *(int*)sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	sp_ringbuf_remove(idx, queue->data, &queue->size, queue->capacity, queue->elem_size, &queue->head, &queue->tail);
	return ret;
}

long sp_queue_removel(struct sp_queue *queue, size_t idx)
{
	long ret;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return 0;
	}
	if (queue->elem_size != sizeof(long)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(long)));
		return 0;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	ret = *(long*)sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	sp_ringbuf_remove(idx, queue->data, &queue->size, queue->capacity, queue->elem_size, &queue->head, &queue->tail);
	return ret;
}

signed char sp_queue_removesc(struct sp_queue *queue, size_t idx)
{
	signed char ret;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return 0;
	}
	if (queue->elem_size != sizeof(signed char)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(signed char)));
		return 0;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	ret = *(signed char*)sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	sp_ringbuf_remove(idx, queue->data, &queue->size, queue->capacity, queue->elem_size, &queue->head, &queue->tail);
	return ret;
}

unsigned char sp_queue_removeuc(struct sp_queue *queue, size_t idx)
{
	unsigned char ret;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return 0;
	}
	if (queue->elem_size != sizeof(unsigned char)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(unsigned char)));
		return 0;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	ret = *(unsigned char*)sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	sp_ringbuf_remove(idx, queue->data, &queue->size, queue->capacity, queue->elem_size, &queue->head, &queue->tail);
	return ret;
}

unsigned short sp_queue_removeus(struct sp_queue *queue, size_t idx)
{
	unsigned short ret;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return 0;
	}
	if (queue->elem_size != sizeof(unsigned short)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(unsigned short)));
		return 0;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	ret = *(unsigned short*)sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	sp_ringbuf_remove(idx, queue->data, &queue->size, queue->capacity, queue->elem_size, &queue->head, &queue->tail);
	return ret;
}

unsigned int sp_queue_removeui(struct sp_queue *queue, size_t idx)
{
	unsigned int ret;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return 0;
	}
	if (queue->elem_size != sizeof(unsigned int)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(unsigned int)));
		return 0;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	ret = *(unsigned int*)sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	sp_ringbuf_remove(idx, queue->data, &queue->size, queue->capacity, queue->elem_size, &queue->head, &queue->tail);
	return ret;
}

unsigned long sp_queue_removeul(struct sp_queue *queue, size_t idx)
{
	unsigned long ret;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return 0;
	}
	if (queue->elem_size != sizeof(unsigned long)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(unsigned long)));
		return 0;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	ret = *(unsigned long*)sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	sp_ringbuf_remove(idx, queue->data, &queue->size, queue->capacity, queue->elem_size, &queue->head, &queue->tail);
	return ret;
}

float sp_queue_removef(struct sp_queue *queue, size_t idx)
{
	float ret;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return 0;
	}
	if (queue->elem_size != sizeof(float)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(float)));
		return 0;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	ret = *(float*)sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	sp_ringbuf_remove(idx, queue->data, &queue->size, queue->capacity, queue->elem_size, &queue->head, &queue->tail);
	return ret;
}

double sp_queue_removed(struct sp_queue *queue, size_t idx)
{
	double ret;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return 0;
	}
	if (queue->elem_size != sizeof(double)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(double)));
		return 0;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	ret = *(double*)sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	sp_ringbuf_remove(idx, queue->data, &queue->size, queue->capacity, queue->elem_size, &queue->head, &queue->tail);
	return ret;
}

long double sp_queue_removeld(struct sp_queue *queue, size_t idx)
{
	long double ret;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return 0;
	}
	if (queue->elem_size != sizeof(long double)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(long double)));
		return 0;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	ret = *(long double*)sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	sp_ringbuf_remove(idx, queue->data, &queue->size, queue->capacity, queue->elem_size, &queue->head, &queue->tail);
	return ret;
}


int sp_queue_qremove(struct sp_queue *queue, size_t idx, void *output)
{
	char *p;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	p = sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	if (output != NULL)
		memcpy(output, p, queue->elem_size);
	memcpy(p, queue->tail, queue->elem_size);
	if (queue->size != 1)
		sp_ringbuf_decr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	--queue->size;
	return 0;
}

char sp_queue_qremovec(struct sp_queue *queue, size_t idx)
{
	char *p;
	char ret;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return 0;
	}
	if (queue->elem_size != sizeof(char)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(char)));
		return 0;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	p = sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	ret = *(char*)p;
	*(char*)p = *(char*)queue->tail;
	if (queue->size != 1)
		sp_ringbuf_decr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	--queue->size;
	return ret;
}

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

int sp_queue_qremovei(struct sp_queue *queue, size_t idx)
{
	char *p;
	int ret;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return 0;
	}
	if (queue->elem_size != sizeof(int)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(int)));
		return 0;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	p = sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	ret = *(int*)p;
	*(int*)p = *(int*)queue->tail;
	if (queue->size != 1)
		sp_ringbuf_decr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	--queue->size;
	return ret;
}

long sp_queue_qremovel(struct sp_queue *queue, size_t idx)
{
	char *p;
	long ret;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return 0;
	}
	if (queue->elem_size != sizeof(long)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(long)));
		return 0;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	p = sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	ret = *(long*)p;
	*(long*)p = *(long*)queue->tail;
	if (queue->size != 1)
		sp_ringbuf_decr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	--queue->size;
	return ret;
}

signed char sp_queue_qremovesc(struct sp_queue *queue, size_t idx)
{
	char *p;
	signed char ret;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return 0;
	}
	if (queue->elem_size != sizeof(signed char)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(signed char)));
		return 0;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	p = sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	ret = *(signed char*)p;
	*(signed char*)p = *(signed char*)queue->tail;
	if (queue->size != 1)
		sp_ringbuf_decr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	--queue->size;
	return ret;
}

unsigned char sp_queue_qremoveuc(struct sp_queue *queue, size_t idx)
{
	char *p;
	unsigned char ret;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return 0;
	}
	if (queue->elem_size != sizeof(unsigned char)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(unsigned char)));
		return 0;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	p = sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	ret = *(unsigned char*)p;
	*(unsigned char*)p = *(unsigned char*)queue->tail;
	if (queue->size != 1)
		sp_ringbuf_decr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	--queue->size;
	return ret;
}

unsigned short sp_queue_qremoveus(struct sp_queue *queue, size_t idx)
{
	char *p;
	unsigned short ret;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return 0;
	}
	if (queue->elem_size != sizeof(unsigned short)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(unsigned short)));
		return 0;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	p = sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	ret = *(unsigned short*)p;
	*(unsigned short*)p = *(unsigned short*)queue->tail;
	if (queue->size != 1)
		sp_ringbuf_decr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	--queue->size;
	return ret;
}

unsigned int sp_queue_qremoveui(struct sp_queue *queue, size_t idx)
{
	char *p;
	unsigned int ret;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return 0;
	}
	if (queue->elem_size != sizeof(unsigned int)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(unsigned int)));
		return 0;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	p = sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	ret = *(unsigned int*)p;
	*(unsigned int*)p = *(unsigned int*)queue->tail;
	if (queue->size != 1)
		sp_ringbuf_decr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	--queue->size;
	return ret;
}

unsigned long sp_queue_qremoveul(struct sp_queue *queue, size_t idx)
{
	char *p;
	unsigned long ret;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return 0;
	}
	if (queue->elem_size != sizeof(unsigned long)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(unsigned long)));
		return 0;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	p = sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	ret = *(unsigned long*)p;
	*(unsigned long*)p = *(unsigned long*)queue->tail;
	if (queue->size != 1)
		sp_ringbuf_decr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	--queue->size;
	return ret;
}

float sp_queue_qremovef(struct sp_queue *queue, size_t idx)
{
	char *p;
	float ret;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return 0;
	}
	if (queue->elem_size != sizeof(float)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(float)));
		return 0;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	p = sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	ret = *(float*)p;
	*(float*)p = *(float*)queue->tail;
	if (queue->size != 1)
		sp_ringbuf_decr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	--queue->size;
	return ret;
}

double sp_queue_qremoved(struct sp_queue *queue, size_t idx)
{
	char *p;
	double ret;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return 0;
	}
	if (queue->elem_size != sizeof(double)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(double)));
		return 0;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	p = sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	ret = *(double*)p;
	*(double*)p = *(double*)queue->tail;
	if (queue->size != 1)
		sp_ringbuf_decr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	--queue->size;
	return ret;
}

long double sp_queue_qremoveld(struct sp_queue *queue, size_t idx)
{
	char *p;
	long double ret;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return 0;
	}
	if (queue->elem_size != sizeof(long double)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(long double)));
		return 0;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	p = sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	ret = *(long double*)p;
	*(long double*)p = *(long double*)queue->tail;
	if (queue->size != 1)
		sp_ringbuf_decr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	--queue->size;
	return ret;
}


int sp_queue_get(const struct sp_queue *queue, size_t idx, void *output)
{
	const void *src;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (output == NULL) {
		error(("output is NULL"));
		return SP_EINVAL;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	src = sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	memcpy(output, src, queue->elem_size);
	return 0;
}

char sp_queue_getc(const struct sp_queue *queue, size_t idx)
{
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return 0;
	}
	if (queue->elem_size != sizeof(char)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(char)));
		return 0;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	return *(char*)sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
}

short sp_queue_gets(const struct sp_queue *queue, size_t idx)
{
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
	return *(short*)sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
}

int sp_queue_geti(const struct sp_queue *queue, size_t idx)
{
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return 0;
	}
	if (queue->elem_size != sizeof(int)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(int)));
		return 0;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	return *(int*)sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
}

long sp_queue_getl(const struct sp_queue *queue, size_t idx)
{
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return 0;
	}
	if (queue->elem_size != sizeof(long)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(long)));
		return 0;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	return *(long*)sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
}

signed char sp_queue_getsc(const struct sp_queue *queue, size_t idx)
{
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return 0;
	}
	if (queue->elem_size != sizeof(signed char)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(signed char)));
		return 0;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	return *(signed char*)sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
}

unsigned char sp_queue_getuc(const struct sp_queue *queue, size_t idx)
{
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return 0;
	}
	if (queue->elem_size != sizeof(unsigned char)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(unsigned char)));
		return 0;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	return *(unsigned char*)sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
}

unsigned short sp_queue_getus(const struct sp_queue *queue, size_t idx)
{
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return 0;
	}
	if (queue->elem_size != sizeof(unsigned short)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(unsigned short)));
		return 0;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	return *(unsigned short*)sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
}

unsigned int sp_queue_getui(const struct sp_queue *queue, size_t idx)
{
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return 0;
	}
	if (queue->elem_size != sizeof(unsigned int)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(unsigned int)));
		return 0;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	return *(unsigned int*)sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
}

unsigned long sp_queue_getul(const struct sp_queue *queue, size_t idx)
{
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return 0;
	}
	if (queue->elem_size != sizeof(unsigned long)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(unsigned long)));
		return 0;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	return *(unsigned long*)sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
}

float sp_queue_getf(const struct sp_queue *queue, size_t idx)
{
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return 0;
	}
	if (queue->elem_size != sizeof(float)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(float)));
		return 0;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	return *(float*)sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
}

double sp_queue_getd(const struct sp_queue *queue, size_t idx)
{
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return 0;
	}
	if (queue->elem_size != sizeof(double)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(double)));
		return 0;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	return *(double*)sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
}

long double sp_queue_getld(const struct sp_queue *queue, size_t idx)
{
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return 0;
	}
	if (queue->elem_size != sizeof(long double)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(long double)));
		return 0;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	return *(long double*)sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
}


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

int sp_queue_setc(struct sp_queue *queue, size_t idx, char val)
{
	char *p;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (queue->elem_size != sizeof(val)) {
		error(("queue->elem_size is incompatible with val type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(val)));
		return SP_EILLEGAL;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	p = sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	*p = val;
	return 0;
}

int sp_queue_sets(struct sp_queue *queue, size_t idx, short val)
{
	char *p;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (queue->elem_size != sizeof(val)) {
		error(("queue->elem_size is incompatible with val type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(val)));
		return SP_EILLEGAL;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	p = sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	*(short*)p = val;
	return 0;
}

int sp_queue_seti(struct sp_queue *queue, size_t idx, int val)
{
	char *p;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (queue->elem_size != sizeof(val)) {
		error(("queue->elem_size is incompatible with val type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(val)));
		return SP_EILLEGAL;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	p = sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	*(int*)p = val;
	return 0;
}

int sp_queue_setl(struct sp_queue *queue, size_t idx, long val)
{
	char *p;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (queue->elem_size != sizeof(val)) {
		error(("queue->elem_size is incompatible with val type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(val)));
		return SP_EILLEGAL;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	p = sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	*(long*)p = val;
	return 0;
}

int sp_queue_setsc(struct sp_queue *queue, size_t idx, signed char val)
{
	char *p;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (queue->elem_size != sizeof(val)) {
		error(("queue->elem_size is incompatible with val type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(val)));
		return SP_EILLEGAL;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	p = sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	*(signed char*)p = val;
	return 0;
}

int sp_queue_setuc(struct sp_queue *queue, size_t idx, unsigned char val)
{
	char *p;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (queue->elem_size != sizeof(val)) {
		error(("queue->elem_size is incompatible with val type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(val)));
		return SP_EILLEGAL;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	p = sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	*(unsigned char*)p = val;
	return 0;
}

int sp_queue_setus(struct sp_queue *queue, size_t idx, unsigned short val)
{
	char *p;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (queue->elem_size != sizeof(val)) {
		error(("queue->elem_size is incompatible with val type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(val)));
		return SP_EILLEGAL;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	p = sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	*(unsigned short*)p = val;
	return 0;
}

int sp_queue_setui(struct sp_queue *queue, size_t idx, unsigned int val)
{
	char *p;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (queue->elem_size != sizeof(val)) {
		error(("queue->elem_size is incompatible with val type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(val)));
		return SP_EILLEGAL;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	p = sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	*(unsigned int*)p = val;
	return 0;
}

int sp_queue_setul(struct sp_queue *queue, size_t idx, unsigned long val)
{
	char *p;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (queue->elem_size != sizeof(val)) {
		error(("queue->elem_size is incompatible with val type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(val)));
		return SP_EILLEGAL;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	p = sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	*(unsigned long*)p = val;
	return 0;
}

int sp_queue_setf(struct sp_queue *queue, size_t idx, float val)
{
	char *p;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (queue->elem_size != sizeof(val)) {
		error(("queue->elem_size is incompatible with val type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(val)));
		return SP_EILLEGAL;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	p = sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	*(float*)p = val;
	return 0;
}

int sp_queue_setd(struct sp_queue *queue, size_t idx, double val)
{
	char *p;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (queue->elem_size != sizeof(val)) {
		error(("queue->elem_size is incompatible with val type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(val)));
		return SP_EILLEGAL;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	p = sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	*(double*)p = val;
	return 0;
}

int sp_queue_setld(struct sp_queue *queue, size_t idx, long double val)
{
	char *p;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (queue->elem_size != sizeof(val)) {
		error(("queue->elem_size is incompatible with val type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(long double)));
		return SP_EILLEGAL;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	p = sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	*(long double*)p = val;
	return 0;
}


int sp_queue_print(struct sp_queue *queue)
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
	for (i = 0; i < queue->size; i++) {
		const void *const elem = sp_ringbuf_get(i, queue->data, queue->capacity, queue->elem_size, queue->head);
		printf("[%lu]\t%p\n", (unsigned long)i, elem);
	}
	return 0;
}

int sp_queue_printc(struct sp_queue *queue)
{
	size_t i;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (queue->elem_size != sizeof(char)) {
		error(("queue->elem_size is incompatible with function type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(char)));
		return SP_EILLEGAL;
	}
#endif
	printf("sp_queue_printc()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)queue->size, (unsigned long)queue->capacity, (unsigned long)queue->elem_size);
	for (i = 0; i < queue->size; i++) {
		const char elem = *(char*)sp_ringbuf_get(i, queue->data, queue->capacity, queue->elem_size, queue->head);
		printf("[%lu]\t%hd\t'%c'\n", (unsigned long)i, elem, elem);
	}
	return 0;
}

int sp_queue_prints(struct sp_queue *queue)
{
	size_t i;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (queue->elem_size != sizeof(short)) {
		error(("queue->elem_size is incompatible with function type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(short)));
		return SP_EILLEGAL;
	}
#endif
	printf("sp_queue_prints()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)queue->size, (unsigned long)queue->capacity, (unsigned long)queue->elem_size);
	for (i = 0; i < queue->size; i++) {
		const short elem = *(short*)sp_ringbuf_get(i, queue->data, queue->capacity, queue->elem_size, queue->head);
		printf("[%lu]\t%hd\n", (unsigned long)i, elem);
	}
	return 0;
}

int sp_queue_printi(struct sp_queue *queue)
{
	size_t i;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (queue->elem_size != sizeof(int)) {
		error(("queue->elem_size is incompatible with function type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(int)));
		return SP_EILLEGAL;
	}
#endif
	printf("sp_queue_printi()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)queue->size, (unsigned long)queue->capacity, (unsigned long)queue->elem_size);
	for (i = 0; i < queue->size; i++) {
		const int elem = *(int*)sp_ringbuf_get(i, queue->data, queue->capacity, queue->elem_size, queue->head);
		printf("[%lu]\t%d\n", (unsigned long)i, elem);
	}
	return 0;
}

int sp_queue_printl(struct sp_queue *queue)
{
	size_t i;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (queue->elem_size != sizeof(long)) {
		error(("queue->elem_size is incompatible with function type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(long)));
		return SP_EILLEGAL;
	}
#endif
	printf("sp_queue_printl()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)queue->size, (unsigned long)queue->capacity, (unsigned long)queue->elem_size);
	for (i = 0; i < queue->size; i++) {
		const long elem = *(long*)sp_ringbuf_get(i, queue->data, queue->capacity, queue->elem_size, queue->head);
		printf("[%lu]\t%ld\n", (unsigned long)i, elem);
	}
	return 0;
}

int sp_queue_printsc(struct sp_queue *queue)
{
	size_t i;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (queue->elem_size != sizeof(signed char)) {
		error(("queue->elem_size is incompatible with function type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(signed char)));
		return SP_EILLEGAL;
	}
#endif
	printf("sp_queue_printsc()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)queue->size, (unsigned long)queue->capacity, (unsigned long)queue->elem_size);
	for (i = 0; i < queue->size; i++) {
		const signed char elem = *(signed char*)sp_ringbuf_get(i, queue->data, queue->capacity, queue->elem_size, queue->head);
		printf("[%lu]\t%hd\t'%c'\n", (unsigned long)i, elem, elem);
	}
	return 0;
}

int sp_queue_printuc(struct sp_queue *queue)
{
	size_t i;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (queue->elem_size != sizeof(unsigned char)) {
		error(("queue->elem_size is incompatible with function type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(unsigned char)));
		return SP_EILLEGAL;
	}
#endif
	printf("sp_queue_printuc()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)queue->size, (unsigned long)queue->capacity, (unsigned long)queue->elem_size);
	for (i = 0; i < queue->size; i++) {
		const unsigned char elem = *(unsigned char*)sp_ringbuf_get(i, queue->data, queue->capacity, queue->elem_size, queue->head);
		printf("[%lu]\t%hd\t'%c'\n", (unsigned long)i, elem, elem);
	}
	return 0;
}

int sp_queue_printus(struct sp_queue *queue)
{
	size_t i;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (queue->elem_size != sizeof(unsigned short)) {
		error(("queue->elem_size is incompatible with function type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(unsigned short)));
		return SP_EILLEGAL;
	}
#endif
	printf("sp_queue_printus()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)queue->size, (unsigned long)queue->capacity, (unsigned long)queue->elem_size);
	for (i = 0; i < queue->size; i++) {
		const unsigned short elem = *(unsigned short*)sp_ringbuf_get(i, queue->data, queue->capacity, queue->elem_size, queue->head);
		printf("[%lu]\t%hu\n", (unsigned long)i, elem);
	}
	return 0;
}

int sp_queue_printui(struct sp_queue *queue)
{
	size_t i;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (queue->elem_size != sizeof(unsigned int)) {
		error(("queue->elem_size is incompatible with function type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(unsigned int)));
		return SP_EILLEGAL;
	}
#endif
	printf("sp_queue_printui()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)queue->size, (unsigned long)queue->capacity, (unsigned long)queue->elem_size);
	for (i = 0; i < queue->size; i++) {
		const unsigned int elem = *(unsigned int*)sp_ringbuf_get(i, queue->data, queue->capacity, queue->elem_size, queue->head);
		printf("[%lu]\t%u\n", (unsigned long)i, elem);
	}
	return 0;
}

int sp_queue_printul(struct sp_queue *queue)
{
	size_t i;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (queue->elem_size != sizeof(unsigned long)) {
		error(("queue->elem_size is incompatible with function type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(unsigned long)));
		return SP_EILLEGAL;
	}
#endif
	printf("sp_queue_printul()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)queue->size, (unsigned long)queue->capacity, (unsigned long)queue->elem_size);
	for (i = 0; i < queue->size; i++) {
		const unsigned long elem = *(unsigned long*)sp_ringbuf_get(i, queue->data, queue->capacity, queue->elem_size, queue->head);
		printf("[%lu]\t%lu\n", (unsigned long)i, elem);
	}
	return 0;
}

int sp_queue_printf(struct sp_queue *queue)
{
	size_t i;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (queue->elem_size != sizeof(float)) {
		error(("queue->elem_size is incompatible with function type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(float)));
		return SP_EILLEGAL;
	}
#endif
	printf("sp_queue_printf()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)queue->size, (unsigned long)queue->capacity, (unsigned long)queue->elem_size);
	for (i = 0; i < queue->size; i++) {
		const float elem = *(float*)sp_ringbuf_get(i, queue->data, queue->capacity, queue->elem_size, queue->head);
		printf("[%lu]\t%g\n", (unsigned long)i, elem);
	}
	return 0;
}

int sp_queue_printd(struct sp_queue *queue)
{
	size_t i;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (queue->elem_size != sizeof(double)) {
		error(("queue->elem_size is incompatible with function type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(double)));
		return SP_EILLEGAL;
	}
#endif
	printf("sp_queue_printd()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)queue->size, (unsigned long)queue->capacity, (unsigned long)queue->elem_size);
	for (i = 0; i < queue->size; i++) {
		const double elem = *(double*)sp_ringbuf_get(i, queue->data, queue->capacity, queue->elem_size, queue->head);
		printf("[%lu]\t%g\n", (unsigned long)i, elem);
	}
	return 0;
}

int sp_queue_printld(struct sp_queue *queue)
{
	size_t i;
#ifdef STAPLE_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return SP_EINVAL;
	}
	if (queue->elem_size != sizeof(long double)) {
		error(("queue->elem_size is incompatible with function type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(long double)));
		return SP_EILLEGAL;
	}
#endif
	printf("sp_queue_printld()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)queue->size, (unsigned long)queue->capacity, (unsigned long)queue->elem_size);
	for (i = 0; i < queue->size; i++) {
		const long double elem = *(long double*)sp_ringbuf_get(i, queue->data, queue->capacity, queue->elem_size, queue->head);
		printf("[%lu]\t%Lg\n", (unsigned long)i, elem);
	}
	return 0;
}
