#include "rnd_queue.h"
#include "helpers.h"
#include "rnd_errcodes.h"
#include <string.h>

struct rnd_queue *rnd_queue_create(size_t elem_size, size_t capacity)
{
	struct rnd_queue *ret;

#ifdef RND_DEBUG
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

int rnd_queue_clear(struct rnd_queue *queue, int (*dtor)(void*))
{
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
#endif
	if (dtor != NULL) {
		while (queue->size != 0) {
			int error;
			if ((error = dtor(queue->head))) {
				error(("external dtor function returned %d (non-0)", error));
				return RND_EHANDLER;
			}
			rnd_ringbuf_incr(&queue->head, queue->data, queue->capacity, queue->elem_size);
			queue->size--;
		}
	} else {
		queue->size = 0;
		queue->head = queue->tail;
	}
	return 0;
}

int rnd_queue_destroy(struct rnd_queue *queue, int (*dtor)(void*))
{
	int error;
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
#endif
	if ((error = rnd_queue_clear(queue, dtor)))
		return error;
	free(queue->data);
	free(queue);
	return 0;
}

int rnd_queue_copy(struct rnd_queue *dest, const struct rnd_queue *src, int (*cpy)(void*, const void*))
{
#ifdef RND_DEBUG
	if (src == NULL) {
		error(("src is NULL"));
		return RND_EINVAL;
	}
	if (dest == NULL) {
		error(("dest is NULL"));
		return RND_EINVAL;
	}
#endif
	if (dest->capacity * dest->elem_size < src->size * src->elem_size) {
		dest->capacity = src->size;
		dest->data = realloc(dest->data, dest->capacity * src->elem_size);
		if (dest->data == NULL) {
			error(("realloc"));
			return RND_ENOMEM;
		}
	}
	dest->elem_size = src->elem_size;
	dest->size      = src->size;
	dest->head      = dest->data;
	dest->tail      = dest->data;
	if (src->size != 0) {
		void  *s = src->head;
		size_t i = src->size;
		rnd_ringbuf_decr(&s, src->data, src->capacity, src->elem_size);
		rnd_ringbuf_decr(&dest->tail, dest->data, dest->capacity, dest->elem_size);
		if (cpy == NULL)
			while (i != 0) {
				rnd_ringbuf_incr(&s, src->data, src->capacity, src->elem_size);
				rnd_ringbuf_incr(&dest->tail, dest->data, dest->capacity, dest->elem_size);
				memcpy(dest->tail, s, src->elem_size);
				--i;
			}
		else
			while (i != 0) {
				int err;
				rnd_ringbuf_incr(&s, src->data, src->capacity, src->elem_size);
				rnd_ringbuf_incr(&dest->tail, dest->data, dest->capacity, dest->elem_size);
				if ((err = cpy(dest->tail, s))) {
					error(("external cpy function returned %d (non-0)", err));
					return RND_EHANDLER;
				}
				--i;
			}
	}
	return 0;
}

int rnd_queue_map(struct rnd_queue *queue, int (*func)(void*, size_t))
{
	size_t i;
	void *p;
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (func == NULL) {
		error(("func is NULL"));
		return RND_EINVAL;
	}
#endif
	p = queue->head;
	i = 0;
	while (p != queue->tail) {
		int err;
		if ((err = func(p, i))) {
			warn(("external func function returned %d (non-0)", err));
			return RND_EHANDLER;
		}
		rnd_ringbuf_incr(&p, queue->data, queue->capacity, queue->elem_size);
		++i;
	}
	return 0;
}

int rnd_queue_push(struct rnd_queue *queue, const void *elem)
{
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (elem == NULL) {
		error(("elem is NULL"));
		return RND_EINVAL;
	}
#endif
	if (rnd_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return RND_ERANGE;
	if (rnd_ringbuffit(&queue->data, queue->elem_size, queue->size, &queue->capacity, &queue->head, &queue->tail))
		return RND_ENOMEM;
	if (queue->size != 0)
		rnd_ringbuf_incr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	memcpy(queue->tail, elem, queue->elem_size);
	++queue->size;
	return 0;
}

int rnd_queue_pushc(struct rnd_queue *queue, char elem)
{
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
#endif
	if (rnd_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return RND_ERANGE;
	if (rnd_ringbuffit(&queue->data, queue->elem_size, queue->size, &queue->capacity, &queue->head, &queue->tail))
		return RND_ENOMEM;
	if (queue->size != 0)
		rnd_ringbuf_incr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	*(char*)queue->tail = elem;
	++queue->size;
	return 0;
}

int rnd_queue_pushs(struct rnd_queue *queue, short elem)
{
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
#endif
	if (rnd_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return RND_ERANGE;
	if (rnd_ringbuffit(&queue->data, queue->elem_size, queue->size, &queue->capacity, &queue->head, &queue->tail))
		return RND_ENOMEM;
	if (queue->size != 0)
		rnd_ringbuf_incr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	*(short*)queue->tail = elem;
	++queue->size;
	return 0;
}

int rnd_queue_pushi(struct rnd_queue *queue, int elem)
{
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
#endif
	if (rnd_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return RND_ERANGE;
	if (rnd_ringbuffit(&queue->data, queue->elem_size, queue->size, &queue->capacity, &queue->head, &queue->tail))
		return RND_ENOMEM;
	if (queue->size != 0)
		rnd_ringbuf_incr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	*(int*)queue->tail = elem;
	++queue->size;
	return 0;
}

int rnd_queue_pushl(struct rnd_queue *queue, long elem)
{
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
#endif
	if (rnd_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return RND_ERANGE;
	if (rnd_ringbuffit(&queue->data, queue->elem_size, queue->size, &queue->capacity, &queue->head, &queue->tail))
		return RND_ENOMEM;
	if (queue->size != 0)
		rnd_ringbuf_incr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	*(long*)queue->tail = elem;
	++queue->size;
	return 0;
}

int rnd_queue_pushsc(struct rnd_queue *queue, signed char elem)
{
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
#endif
	if (rnd_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return RND_ERANGE;
	if (rnd_ringbuffit(&queue->data, queue->elem_size, queue->size, &queue->capacity, &queue->head, &queue->tail))
		return RND_ENOMEM;
	if (queue->size != 0)
		rnd_ringbuf_incr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	*(signed char*)queue->tail = elem;
	++queue->size;
	return 0;
}

int rnd_queue_pushuc(struct rnd_queue *queue, unsigned char elem)
{
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
#endif
	if (rnd_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return RND_ERANGE;
	if (rnd_ringbuffit(&queue->data, queue->elem_size, queue->size, &queue->capacity, &queue->head, &queue->tail))
		return RND_ENOMEM;
	if (queue->size != 0)
		rnd_ringbuf_incr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	*(unsigned char*)queue->tail = elem;
	++queue->size;
	return 0;
}

int rnd_queue_pushus(struct rnd_queue *queue, unsigned short elem)
{
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
#endif
	if (rnd_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return RND_ERANGE;
	if (rnd_ringbuffit(&queue->data, queue->elem_size, queue->size, &queue->capacity, &queue->head, &queue->tail))
		return RND_ENOMEM;
	if (queue->size != 0)
		rnd_ringbuf_incr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	*(unsigned short*)queue->tail = elem;
	++queue->size;
	return 0;
}

int rnd_queue_pushui(struct rnd_queue *queue, unsigned int elem)
{
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
#endif
	if (rnd_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return RND_ERANGE;
	if (rnd_ringbuffit(&queue->data, queue->elem_size, queue->size, &queue->capacity, &queue->head, &queue->tail))
		return RND_ENOMEM;
	if (queue->size != 0)
		rnd_ringbuf_incr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	*(unsigned int*)queue->tail = elem;
	++queue->size;
	return 0;
}

int rnd_queue_pushul(struct rnd_queue *queue, unsigned long elem)
{
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
#endif
	if (rnd_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return RND_ERANGE;
	if (rnd_ringbuffit(&queue->data, queue->elem_size, queue->size, &queue->capacity, &queue->head, &queue->tail))
		return RND_ENOMEM;
	if (queue->size != 0)
		rnd_ringbuf_incr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	*(unsigned long*)queue->tail = elem;
	++queue->size;
	return 0;
}

int rnd_queue_pushf(struct rnd_queue *queue, float elem)
{
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
#endif
	if (rnd_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return RND_ERANGE;
	if (rnd_ringbuffit(&queue->data, queue->elem_size, queue->size, &queue->capacity, &queue->head, &queue->tail))
		return RND_ENOMEM;
	if (queue->size != 0)
		rnd_ringbuf_incr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	*(float*)queue->tail = elem;
	++queue->size;
	return 0;
}

int rnd_queue_pushd(struct rnd_queue *queue, double elem)
{
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
#endif
	if (rnd_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return RND_ERANGE;
	if (rnd_ringbuffit(&queue->data, queue->elem_size, queue->size, &queue->capacity, &queue->head, &queue->tail))
		return RND_ENOMEM;
	if (queue->size != 0)
		rnd_ringbuf_incr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	*(double*)queue->tail = elem;
	++queue->size;
	return 0;
}

int rnd_queue_pushld(struct rnd_queue *queue, long double elem)
{
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
#endif
	if (rnd_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return RND_ERANGE;
	if (rnd_ringbuffit(&queue->data, queue->elem_size, queue->size, &queue->capacity, &queue->head, &queue->tail))
		return RND_ENOMEM;
	if (queue->size != 0)
		rnd_ringbuf_incr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	*(long double*)queue->tail = elem;
	++queue->size;
	return 0;
}


int rnd_queue_insert(struct rnd_queue *queue, size_t idx, const void *elem)
{
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (elem == NULL) {
		error(("elem is NULL"));
		return RND_EINVAL;
	}
	if (idx > queue->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	if (rnd_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return RND_ERANGE;
	if (rnd_ringbuffit(&queue->data, queue->elem_size, queue->size, &queue->capacity, &queue->head, &queue->tail))
		return RND_ENOMEM;
	rnd_ringbuf_insert(elem, idx, queue->data, &queue->size, queue->elem_size, queue->capacity, &queue->head, &queue->tail);
	return 0;
}

int rnd_queue_insertc(struct rnd_queue *queue, size_t idx, char elem)
{
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
	if (idx > queue->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	if (rnd_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return RND_ERANGE;
	if (rnd_ringbuffit(&queue->data, queue->elem_size, queue->size, &queue->capacity, &queue->head, &queue->tail))
		return RND_ENOMEM;
	rnd_ringbuf_insert(&elem, idx, queue->data, &queue->size, queue->elem_size, queue->capacity, &queue->head, &queue->tail);
	return 0;
}

int rnd_queue_inserts(struct rnd_queue *queue, size_t idx, short elem)
{
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
	if (idx > queue->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	if (rnd_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return RND_ERANGE;
	if (rnd_ringbuffit(&queue->data, queue->elem_size, queue->size, &queue->capacity, &queue->head, &queue->tail))
		return RND_ENOMEM;
	rnd_ringbuf_insert(&elem, idx, queue->data, &queue->size, queue->elem_size, queue->capacity, &queue->head, &queue->tail);
	return 0;
}

int rnd_queue_inserti(struct rnd_queue *queue, size_t idx, int elem)
{
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
	if (idx > queue->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	if (rnd_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return RND_ERANGE;
	if (rnd_ringbuffit(&queue->data, queue->elem_size, queue->size, &queue->capacity, &queue->head, &queue->tail))
		return RND_ENOMEM;
	rnd_ringbuf_insert(&elem, idx, queue->data, &queue->size, queue->elem_size, queue->capacity, &queue->head, &queue->tail);
	return 0;
}

int rnd_queue_insertl(struct rnd_queue *queue, size_t idx, long elem)
{
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
	if (idx > queue->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	if (rnd_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return RND_ERANGE;
	if (rnd_ringbuffit(&queue->data, queue->elem_size, queue->size, &queue->capacity, &queue->head, &queue->tail))
		return RND_ENOMEM;
	rnd_ringbuf_insert(&elem, idx, queue->data, &queue->size, queue->elem_size, queue->capacity, &queue->head, &queue->tail);
	return 0;
}

int rnd_queue_insertsc(struct rnd_queue *queue, size_t idx, signed char elem)
{
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
	if (idx > queue->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	if (rnd_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return RND_ERANGE;
	if (rnd_ringbuffit(&queue->data, queue->elem_size, queue->size, &queue->capacity, &queue->head, &queue->tail))
		return RND_ENOMEM;
	rnd_ringbuf_insert(&elem, idx, queue->data, &queue->size, queue->elem_size, queue->capacity, &queue->head, &queue->tail);
	return 0;
}

int rnd_queue_insertuc(struct rnd_queue *queue, size_t idx, unsigned char elem)
{
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
	if (idx > queue->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	if (rnd_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return RND_ERANGE;
	if (rnd_ringbuffit(&queue->data, queue->elem_size, queue->size, &queue->capacity, &queue->head, &queue->tail))
		return RND_ENOMEM;
	rnd_ringbuf_insert(&elem, idx, queue->data, &queue->size, queue->elem_size, queue->capacity, &queue->head, &queue->tail);
	return 0;
}

int rnd_queue_insertus(struct rnd_queue *queue, size_t idx, unsigned short elem)
{
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
	if (idx > queue->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	if (rnd_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return RND_ERANGE;
	if (rnd_ringbuffit(&queue->data, queue->elem_size, queue->size, &queue->capacity, &queue->head, &queue->tail))
		return RND_ENOMEM;
	rnd_ringbuf_insert(&elem, idx, queue->data, &queue->size, queue->elem_size, queue->capacity, &queue->head, &queue->tail);
	return 0;
}

int rnd_queue_insertui(struct rnd_queue *queue, size_t idx, unsigned int elem)
{
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
	if (idx > queue->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	if (rnd_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return RND_ERANGE;
	if (rnd_ringbuffit(&queue->data, queue->elem_size, queue->size, &queue->capacity, &queue->head, &queue->tail))
		return RND_ENOMEM;
	rnd_ringbuf_insert(&elem, idx, queue->data, &queue->size, queue->elem_size, queue->capacity, &queue->head, &queue->tail);
	return 0;
}

int rnd_queue_insertul(struct rnd_queue *queue, size_t idx, unsigned long elem)
{
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
	if (idx > queue->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	if (rnd_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return RND_ERANGE;
	if (rnd_ringbuffit(&queue->data, queue->elem_size, queue->size, &queue->capacity, &queue->head, &queue->tail))
		return RND_ENOMEM;
	rnd_ringbuf_insert(&elem, idx, queue->data, &queue->size, queue->elem_size, queue->capacity, &queue->head, &queue->tail);
	return 0;
}

int rnd_queue_insertf(struct rnd_queue *queue, size_t idx, float elem)
{
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
	if (idx > queue->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	if (rnd_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return RND_ERANGE;
	if (rnd_ringbuffit(&queue->data, queue->elem_size, queue->size, &queue->capacity, &queue->head, &queue->tail))
		return RND_ENOMEM;
	rnd_ringbuf_insert(&elem, idx, queue->data, &queue->size, queue->elem_size, queue->capacity, &queue->head, &queue->tail);
	return 0;
}

int rnd_queue_insertd(struct rnd_queue *queue, size_t idx, double elem)
{
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
	if (idx > queue->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	if (rnd_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return RND_ERANGE;
	if (rnd_ringbuffit(&queue->data, queue->elem_size, queue->size, &queue->capacity, &queue->head, &queue->tail))
		return RND_ENOMEM;
	rnd_ringbuf_insert(&elem, idx, queue->data, &queue->size, queue->elem_size, queue->capacity, &queue->head, &queue->tail);
	return 0;
}

int rnd_queue_insertld(struct rnd_queue *queue, size_t idx, long double elem)
{
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
	if (idx > queue->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	if (rnd_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return RND_ERANGE;
	if (rnd_ringbuffit(&queue->data, queue->elem_size, queue->size, &queue->capacity, &queue->head, &queue->tail))
		return RND_ENOMEM;
	rnd_ringbuf_insert(&elem, idx, queue->data, &queue->size, queue->elem_size, queue->capacity, &queue->head, &queue->tail);
	return 0;
}


int rnd_queue_quickinsert(struct rnd_queue *queue, size_t idx, const void *elem)
{
	char *p;
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (elem == NULL) {
		error(("elem is NULL"));
		return RND_EINVAL;
	}
	if (idx > queue->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	if (rnd_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return RND_ERANGE;
	if (rnd_buffit(&queue->data, queue->elem_size, queue->size, &queue->capacity))
		return RND_ENOMEM;
	if (queue->size != 0)
		rnd_ringbuf_incr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	p = rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
	memcpy(queue->tail, p, queue->elem_size);
	memcpy(p, elem, queue->elem_size);
	++queue->size;
	return 0;
}

int rnd_queue_quickinsertc(struct rnd_queue *queue, size_t idx, char elem)
{
	char *p;
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
	if (idx > queue->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	if (rnd_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return RND_ERANGE;
	if (rnd_buffit(&queue->data, queue->elem_size, queue->size, &queue->capacity))
		return RND_ENOMEM;
	if (queue->size != 0)
		rnd_ringbuf_incr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	p = rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
	*(char*)queue->tail = *(char*)p;
	*(char*)p = elem;
	++queue->size;
	return 0;
}

int rnd_queue_quickinserts(struct rnd_queue *queue, size_t idx, short elem)
{
	char *p;
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
	if (idx > queue->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	if (rnd_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return RND_ERANGE;
	if (rnd_buffit(&queue->data, queue->elem_size, queue->size, &queue->capacity))
		return RND_ENOMEM;
	if (queue->size != 0)
		rnd_ringbuf_incr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	p = rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
	*(short*)queue->tail = *(short*)p;
	*(short*)p = elem;
	++queue->size;
	return 0;
}

int rnd_queue_quickinserti(struct rnd_queue *queue, size_t idx, int elem)
{
	char *p;
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
	if (idx > queue->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	if (rnd_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return RND_ERANGE;
	if (rnd_buffit(&queue->data, queue->elem_size, queue->size, &queue->capacity))
		return RND_ENOMEM;
	if (queue->size != 0)
		rnd_ringbuf_incr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	p = rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
	*(int*)queue->tail = *(int*)p;
	*(int*)p = elem;
	++queue->size;
	return 0;
}

int rnd_queue_quickinsertl(struct rnd_queue *queue, size_t idx, long elem)
{
	char *p;
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
	if (idx > queue->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	if (rnd_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return RND_ERANGE;
	if (rnd_buffit(&queue->data, queue->elem_size, queue->size, &queue->capacity))
		return RND_ENOMEM;
	if (queue->size != 0)
		rnd_ringbuf_incr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	p = rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
	*(long*)queue->tail = *(long*)p;
	*(long*)p = elem;
	++queue->size;
	return 0;
}

int rnd_queue_quickinsertsc(struct rnd_queue *queue, size_t idx, signed char elem)
{
	char *p;
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
	if (idx > queue->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	if (rnd_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return RND_ERANGE;
	if (rnd_buffit(&queue->data, queue->elem_size, queue->size, &queue->capacity))
		return RND_ENOMEM;
	if (queue->size != 0)
		rnd_ringbuf_incr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	p = rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
	*(signed char*)queue->tail = *(signed char*)p;
	*(signed char*)p = elem;
	++queue->size;
	return 0;
}

int rnd_queue_quickinsertuc(struct rnd_queue *queue, size_t idx, unsigned char elem)
{
	char *p;
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
	if (idx > queue->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	if (rnd_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return RND_ERANGE;
	if (rnd_buffit(&queue->data, queue->elem_size, queue->size, &queue->capacity))
		return RND_ENOMEM;
	if (queue->size != 0)
		rnd_ringbuf_incr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	p = rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
	*(unsigned char*)queue->tail = *(unsigned char*)p;
	*(unsigned char*)p = elem;
	++queue->size;
	return 0;
}

int rnd_queue_quickinsertus(struct rnd_queue *queue, size_t idx, unsigned short elem)
{
	char *p;
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
	if (idx > queue->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	if (rnd_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return RND_ERANGE;
	if (rnd_buffit(&queue->data, queue->elem_size, queue->size, &queue->capacity))
		return RND_ENOMEM;
	if (queue->size != 0)
		rnd_ringbuf_incr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	p = rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
	*(unsigned short*)queue->tail = *(unsigned short*)p;
	*(unsigned short*)p = elem;
	++queue->size;
	return 0;
}

int rnd_queue_quickinsertui(struct rnd_queue *queue, size_t idx, unsigned int elem)
{
	char *p;
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
	if (idx > queue->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	if (rnd_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return RND_ERANGE;
	if (rnd_buffit(&queue->data, queue->elem_size, queue->size, &queue->capacity))
		return RND_ENOMEM;
	if (queue->size != 0)
		rnd_ringbuf_incr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	p = rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
	*(unsigned int*)queue->tail = *(unsigned int*)p;
	*(unsigned int*)p = elem;
	++queue->size;
	return 0;
}

int rnd_queue_quickinsertul(struct rnd_queue *queue, size_t idx, unsigned long elem)
{
	char *p;
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
	if (idx > queue->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	if (rnd_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return RND_ERANGE;
	if (rnd_buffit(&queue->data, queue->elem_size, queue->size, &queue->capacity))
		return RND_ENOMEM;
	if (queue->size != 0)
		rnd_ringbuf_incr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	p = rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
	*(unsigned long*)queue->tail = *(unsigned long*)p;
	*(unsigned long*)p = elem;
	++queue->size;
	return 0;
}

int rnd_queue_quickinsertf(struct rnd_queue *queue, size_t idx, float elem)
{
	char *p;
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
	if (idx > queue->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	if (rnd_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return RND_ERANGE;
	if (rnd_buffit(&queue->data, queue->elem_size, queue->size, &queue->capacity))
		return RND_ENOMEM;
	if (queue->size != 0)
		rnd_ringbuf_incr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	p = rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
	*(float*)queue->tail = *(float*)p;
	*(float*)p = elem;
	++queue->size;
	return 0;
}

int rnd_queue_quickinsertd(struct rnd_queue *queue, size_t idx, double elem)
{
	char *p;
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
	if (idx > queue->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	if (rnd_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return RND_ERANGE;
	if (rnd_buffit(&queue->data, queue->elem_size, queue->size, &queue->capacity))
		return RND_ENOMEM;
	if (queue->size != 0)
		rnd_ringbuf_incr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	p = rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
	*(double*)queue->tail = *(double*)p;
	*(double*)p = elem;
	++queue->size;
	return 0;
}

int rnd_queue_quickinsertld(struct rnd_queue *queue, size_t idx, long double elem)
{
	char *p;
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (queue->elem_size != sizeof(elem)) {
		error(("queue->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
	if (idx > queue->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	if (rnd_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return RND_ERANGE;
	if (rnd_buffit(&queue->data, queue->elem_size, queue->size, &queue->capacity))
		return RND_ENOMEM;
	if (queue->size != 0)
		rnd_ringbuf_incr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	p = rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
	*(long double*)queue->tail = *(long double*)p;
	*(long double*)p = elem;
	++queue->size;
	return 0;
}


int rnd_queue_peek(const struct rnd_queue *queue, void *output)
{
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (output == NULL) {
		error(("output is NULL"));
		return RND_EINVAL;
	}
	if (queue->size == 0) {
		error(("queue is empty"));
		return RND_EILLEGAL;
	}
#endif
	memcpy(output, queue->head, queue->elem_size);
	return 0;
}

char rnd_queue_peekc(const struct rnd_queue *queue)
{
#ifdef RND_DEBUG
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

short rnd_queue_peeks(const struct rnd_queue *queue)
{
#ifdef RND_DEBUG
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

int rnd_queue_peeki(const struct rnd_queue *queue)
{
#ifdef RND_DEBUG
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

long rnd_queue_peekl(const struct rnd_queue *queue)
{
#ifdef RND_DEBUG
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

signed char rnd_queue_peeksc(const struct rnd_queue *queue)
{
#ifdef RND_DEBUG
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

unsigned char rnd_queue_peekuc(const struct rnd_queue *queue)
{
#ifdef RND_DEBUG
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

unsigned short rnd_queue_peekus(const struct rnd_queue *queue)
{
#ifdef RND_DEBUG
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

unsigned int rnd_queue_peekui(const struct rnd_queue *queue)
{
#ifdef RND_DEBUG
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

unsigned long rnd_queue_peekul(const struct rnd_queue *queue)
{
#ifdef RND_DEBUG
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

float rnd_queue_peekf(const struct rnd_queue *queue)
{
#ifdef RND_DEBUG
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

double rnd_queue_peekd(const struct rnd_queue *queue)
{
#ifdef RND_DEBUG
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

long double rnd_queue_peekld(const struct rnd_queue *queue)
{
#ifdef RND_DEBUG
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


int rnd_queue_pop(struct rnd_queue *queue, void *output)
{
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (queue->size == 0) {
		error(("queue is empty"));
		return RND_EILLEGAL;
	}
#endif
	if (output != NULL)
		memcpy(output, queue->head, queue->elem_size);
	if (queue->size != 1)
		rnd_ringbuf_incr(&queue->head, queue->data, queue->capacity, queue->elem_size);
	--queue->size;
	return 0;
}

char rnd_queue_popc(struct rnd_queue *queue)
{
	char ret;
#ifdef RND_DEBUG
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
		rnd_ringbuf_incr(&queue->head, queue->data, queue->capacity, queue->elem_size);
	--queue->size;
	return ret;
}

short rnd_queue_pops(struct rnd_queue *queue)
{
	short ret;
#ifdef RND_DEBUG
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
		rnd_ringbuf_incr(&queue->head, queue->data, queue->capacity, queue->elem_size);
	--queue->size;
	return ret;
}

int rnd_queue_popi(struct rnd_queue *queue)
{
	int ret;
#ifdef RND_DEBUG
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
		rnd_ringbuf_incr(&queue->head, queue->data, queue->capacity, queue->elem_size);
	--queue->size;
	return ret;
}

long rnd_queue_popl(struct rnd_queue *queue)
{
	long ret;
#ifdef RND_DEBUG
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
		rnd_ringbuf_incr(&queue->head, queue->data, queue->capacity, queue->elem_size);
	--queue->size;
	return ret;
}

signed char rnd_queue_popsc(struct rnd_queue *queue)
{
	signed char ret;
#ifdef RND_DEBUG
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
		rnd_ringbuf_incr(&queue->head, queue->data, queue->capacity, queue->elem_size);
	--queue->size;
	return ret;
}

unsigned char rnd_queue_popuc(struct rnd_queue *queue)
{
	unsigned char ret;
#ifdef RND_DEBUG
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
		rnd_ringbuf_incr(&queue->head, queue->data, queue->capacity, queue->elem_size);
	--queue->size;
	return ret;
}

unsigned short rnd_queue_popus(struct rnd_queue *queue)
{
	unsigned short ret;
#ifdef RND_DEBUG
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
		rnd_ringbuf_incr(&queue->head, queue->data, queue->capacity, queue->elem_size);
	--queue->size;
	return ret;
}

unsigned int rnd_queue_popui(struct rnd_queue *queue)
{
	unsigned int ret;
#ifdef RND_DEBUG
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
		rnd_ringbuf_incr(&queue->head, queue->data, queue->capacity, queue->elem_size);
	--queue->size;
	return ret;
}

unsigned long rnd_queue_popul(struct rnd_queue *queue)
{
	unsigned long ret;
#ifdef RND_DEBUG
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
		rnd_ringbuf_incr(&queue->head, queue->data, queue->capacity, queue->elem_size);
	--queue->size;
	return ret;
}

float rnd_queue_popf(struct rnd_queue *queue)
{
	float ret;
#ifdef RND_DEBUG
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
		rnd_ringbuf_incr(&queue->head, queue->data, queue->capacity, queue->elem_size);
	--queue->size;
	return ret;
}

double rnd_queue_popd(struct rnd_queue *queue)
{
	double ret;
#ifdef RND_DEBUG
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
		rnd_ringbuf_incr(&queue->head, queue->data, queue->capacity, queue->elem_size);
	--queue->size;
	return ret;
}

long double rnd_queue_popld(struct rnd_queue *queue)
{
	long double ret;
#ifdef RND_DEBUG
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
		rnd_ringbuf_incr(&queue->head, queue->data, queue->capacity, queue->elem_size);
	--queue->size;
	return ret;
}


int rnd_queue_remove(struct rnd_queue *queue, size_t idx, void *output)
{
	char *p;
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	p = rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
	if (output != NULL)
		memcpy(output, p, queue->elem_size);
	rnd_ringbuf_remove(idx, queue->data, &queue->size, queue->elem_size, queue->capacity, &queue->head, &queue->tail);
	return 0;
}

char rnd_queue_removec(struct rnd_queue *queue, size_t idx)
{
	char ret;
#ifdef RND_DEBUG
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
	ret = *(char*)rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
	rnd_ringbuf_remove(idx, queue->data, &queue->size, queue->elem_size, queue->capacity, &queue->head, &queue->tail);
	return ret;
}

short rnd_queue_removes(struct rnd_queue *queue, size_t idx)
{
	short ret;
#ifdef RND_DEBUG
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
	ret = *(short*)rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
	rnd_ringbuf_remove(idx, queue->data, &queue->size, queue->elem_size, queue->capacity, &queue->head, &queue->tail);
	return ret;
}

int rnd_queue_removei(struct rnd_queue *queue, size_t idx)
{
	int ret;
#ifdef RND_DEBUG
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
	ret = *(int*)rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
	rnd_ringbuf_remove(idx, queue->data, &queue->size, queue->elem_size, queue->capacity, &queue->head, &queue->tail);
	return ret;
}

long rnd_queue_removel(struct rnd_queue *queue, size_t idx)
{
	long ret;
#ifdef RND_DEBUG
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
	ret = *(long*)rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
	rnd_ringbuf_remove(idx, queue->data, &queue->size, queue->elem_size, queue->capacity, &queue->head, &queue->tail);
	return ret;
}

signed char rnd_queue_removesc(struct rnd_queue *queue, size_t idx)
{
	signed char ret;
#ifdef RND_DEBUG
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
	ret = *(signed char*)rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
	rnd_ringbuf_remove(idx, queue->data, &queue->size, queue->elem_size, queue->capacity, &queue->head, &queue->tail);
	return ret;
}

unsigned char rnd_queue_removeuc(struct rnd_queue *queue, size_t idx)
{
	unsigned char ret;
#ifdef RND_DEBUG
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
	ret = *(unsigned char*)rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
	rnd_ringbuf_remove(idx, queue->data, &queue->size, queue->elem_size, queue->capacity, &queue->head, &queue->tail);
	return ret;
}

unsigned short rnd_queue_removeus(struct rnd_queue *queue, size_t idx)
{
	unsigned short ret;
#ifdef RND_DEBUG
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
	ret = *(unsigned short*)rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
	rnd_ringbuf_remove(idx, queue->data, &queue->size, queue->elem_size, queue->capacity, &queue->head, &queue->tail);
	return ret;
}

unsigned int rnd_queue_removeui(struct rnd_queue *queue, size_t idx)
{
	unsigned int ret;
#ifdef RND_DEBUG
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
	ret = *(unsigned int*)rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
	rnd_ringbuf_remove(idx, queue->data, &queue->size, queue->elem_size, queue->capacity, &queue->head, &queue->tail);
	return ret;
}

unsigned long rnd_queue_removeul(struct rnd_queue *queue, size_t idx)
{
	unsigned long ret;
#ifdef RND_DEBUG
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
	ret = *(unsigned long*)rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
	rnd_ringbuf_remove(idx, queue->data, &queue->size, queue->elem_size, queue->capacity, &queue->head, &queue->tail);
	return ret;
}

float rnd_queue_removef(struct rnd_queue *queue, size_t idx)
{
	float ret;
#ifdef RND_DEBUG
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
	ret = *(float*)rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
	rnd_ringbuf_remove(idx, queue->data, &queue->size, queue->elem_size, queue->capacity, &queue->head, &queue->tail);
	return ret;
}

double rnd_queue_removed(struct rnd_queue *queue, size_t idx)
{
	double ret;
#ifdef RND_DEBUG
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
	ret = *(double*)rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
	rnd_ringbuf_remove(idx, queue->data, &queue->size, queue->elem_size, queue->capacity, &queue->head, &queue->tail);
	return ret;
}

long double rnd_queue_removeld(struct rnd_queue *queue, size_t idx)
{
	long double ret;
#ifdef RND_DEBUG
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
	ret = *(long double*)rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
	rnd_ringbuf_remove(idx, queue->data, &queue->size, queue->elem_size, queue->capacity, &queue->head, &queue->tail);
	return ret;
}


int rnd_queue_quickremove(struct rnd_queue *queue, size_t idx, void *output)
{
	char *p;
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	p = rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
	if (output != NULL)
		memcpy(output, p, queue->elem_size);
	memcpy(p, queue->tail, queue->elem_size);
	if (queue->size != 1)
		rnd_ringbuf_decr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	--queue->size;
	return 0;
}

char rnd_queue_quickremovec(struct rnd_queue *queue, size_t idx)
{
	char *p;
	char ret;
#ifdef RND_DEBUG
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
	p = rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
	ret = *(char*)p;
	*(char*)p = *(char*)queue->tail;
	if (queue->size != 1)
		rnd_ringbuf_decr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	--queue->size;
	return ret;
}

short rnd_queue_quickremoves(struct rnd_queue *queue, size_t idx)
{
	char *p;
	short ret;
#ifdef RND_DEBUG
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
	p = rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
	ret = *(short*)p;
	*(short*)p = *(short*)queue->tail;
	if (queue->size != 1)
		rnd_ringbuf_decr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	--queue->size;
	return ret;
}

int rnd_queue_quickremovei(struct rnd_queue *queue, size_t idx)
{
	char *p;
	int ret;
#ifdef RND_DEBUG
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
	p = rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
	ret = *(int*)p;
	*(int*)p = *(int*)queue->tail;
	if (queue->size != 1)
		rnd_ringbuf_decr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	--queue->size;
	return ret;
}

long rnd_queue_quickremovel(struct rnd_queue *queue, size_t idx)
{
	char *p;
	long ret;
#ifdef RND_DEBUG
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
	p = rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
	ret = *(long*)p;
	*(long*)p = *(long*)queue->tail;
	if (queue->size != 1)
		rnd_ringbuf_decr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	--queue->size;
	return ret;
}

signed char rnd_queue_quickremovesc(struct rnd_queue *queue, size_t idx)
{
	char *p;
	signed char ret;
#ifdef RND_DEBUG
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
	p = rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
	ret = *(signed char*)p;
	*(signed char*)p = *(signed char*)queue->tail;
	if (queue->size != 1)
		rnd_ringbuf_decr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	--queue->size;
	return ret;
}

unsigned char rnd_queue_quickremoveuc(struct rnd_queue *queue, size_t idx)
{
	char *p;
	unsigned char ret;
#ifdef RND_DEBUG
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
	p = rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
	ret = *(unsigned char*)p;
	*(unsigned char*)p = *(unsigned char*)queue->tail;
	if (queue->size != 1)
		rnd_ringbuf_decr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	--queue->size;
	return ret;
}

unsigned short rnd_queue_quickremoveus(struct rnd_queue *queue, size_t idx)
{
	char *p;
	unsigned short ret;
#ifdef RND_DEBUG
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
	p = rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
	ret = *(unsigned short*)p;
	*(unsigned short*)p = *(unsigned short*)queue->tail;
	if (queue->size != 1)
		rnd_ringbuf_decr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	--queue->size;
	return ret;
}

unsigned int rnd_queue_quickremoveui(struct rnd_queue *queue, size_t idx)
{
	char *p;
	unsigned int ret;
#ifdef RND_DEBUG
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
	p = rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
	ret = *(unsigned int*)p;
	*(unsigned int*)p = *(unsigned int*)queue->tail;
	if (queue->size != 1)
		rnd_ringbuf_decr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	--queue->size;
	return ret;
}

unsigned long rnd_queue_quickremoveul(struct rnd_queue *queue, size_t idx)
{
	char *p;
	unsigned long ret;
#ifdef RND_DEBUG
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
	p = rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
	ret = *(unsigned long*)p;
	*(unsigned long*)p = *(unsigned long*)queue->tail;
	if (queue->size != 1)
		rnd_ringbuf_decr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	--queue->size;
	return ret;
}

float rnd_queue_quickremovef(struct rnd_queue *queue, size_t idx)
{
	char *p;
	float ret;
#ifdef RND_DEBUG
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
	p = rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
	ret = *(float*)p;
	*(float*)p = *(float*)queue->tail;
	if (queue->size != 1)
		rnd_ringbuf_decr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	--queue->size;
	return ret;
}

double rnd_queue_quickremoved(struct rnd_queue *queue, size_t idx)
{
	char *p;
	double ret;
#ifdef RND_DEBUG
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
	p = rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
	ret = *(double*)p;
	*(double*)p = *(double*)queue->tail;
	if (queue->size != 1)
		rnd_ringbuf_decr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	--queue->size;
	return ret;
}

long double rnd_queue_quickremoveld(struct rnd_queue *queue, size_t idx)
{
	char *p;
	long double ret;
#ifdef RND_DEBUG
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
	p = rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
	ret = *(long double*)p;
	*(long double*)p = *(long double*)queue->tail;
	if (queue->size != 1)
		rnd_ringbuf_decr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	--queue->size;
	return ret;
}


int rnd_queue_get(const struct rnd_queue *queue, size_t idx, void *output)
{
	const void *src;
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (output == NULL) {
		error(("output is NULL"));
		return RND_EINVAL;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	src = rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
	memcpy(output, src, queue->elem_size);
	return 0;
}

char rnd_queue_getc(const struct rnd_queue *queue, size_t idx)
{
#ifdef RND_DEBUG
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
	return *(char*)rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
}

short rnd_queue_gets(const struct rnd_queue *queue, size_t idx)
{
#ifdef RND_DEBUG
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
	return *(short*)rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
}

int rnd_queue_geti(const struct rnd_queue *queue, size_t idx)
{
#ifdef RND_DEBUG
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
	return *(int*)rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
}

long rnd_queue_getl(const struct rnd_queue *queue, size_t idx)
{
#ifdef RND_DEBUG
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
	return *(long*)rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
}

signed char rnd_queue_getsc(const struct rnd_queue *queue, size_t idx)
{
#ifdef RND_DEBUG
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
	return *(signed char*)rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
}

unsigned char rnd_queue_getuc(const struct rnd_queue *queue, size_t idx)
{
#ifdef RND_DEBUG
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
	return *(unsigned char*)rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
}

unsigned short rnd_queue_getus(const struct rnd_queue *queue, size_t idx)
{
#ifdef RND_DEBUG
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
	return *(unsigned short*)rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
}

unsigned int rnd_queue_getui(const struct rnd_queue *queue, size_t idx)
{
#ifdef RND_DEBUG
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
	return *(unsigned int*)rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
}

unsigned long rnd_queue_getul(const struct rnd_queue *queue, size_t idx)
{
#ifdef RND_DEBUG
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
	return *(unsigned long*)rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
}

float rnd_queue_getf(const struct rnd_queue *queue, size_t idx)
{
#ifdef RND_DEBUG
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
	return *(float*)rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
}

double rnd_queue_getd(const struct rnd_queue *queue, size_t idx)
{
#ifdef RND_DEBUG
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
	return *(double*)rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
}

long double rnd_queue_getld(const struct rnd_queue *queue, size_t idx)
{
#ifdef RND_DEBUG
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
	return *(long double*)rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
}


int rnd_queue_set(struct rnd_queue *queue, size_t idx, void *val)
{
	char *p;
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (val == NULL) {
		error(("val is NULL"));
		return RND_EINVAL;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	p = rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
	memcpy(p, val, queue->elem_size);
	return 0;
}

int rnd_queue_setc(struct rnd_queue *queue, size_t idx, char val)
{
	char *p;
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (queue->elem_size != sizeof(val)) {
		error(("queue->elem_size is incompatible with val type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(val)));
		return RND_EILLEGAL;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	p = rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
	*p = val;
	return 0;
}

int rnd_queue_sets(struct rnd_queue *queue, size_t idx, short val)
{
	char *p;
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (queue->elem_size != sizeof(val)) {
		error(("queue->elem_size is incompatible with val type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(val)));
		return RND_EILLEGAL;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	p = rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
	*(short*)p = val;
	return 0;
}

int rnd_queue_seti(struct rnd_queue *queue, size_t idx, int val)
{
	char *p;
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (queue->elem_size != sizeof(val)) {
		error(("queue->elem_size is incompatible with val type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(val)));
		return RND_EILLEGAL;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	p = rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
	*(int*)p = val;
	return 0;
}

int rnd_queue_setl(struct rnd_queue *queue, size_t idx, long val)
{
	char *p;
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (queue->elem_size != sizeof(val)) {
		error(("queue->elem_size is incompatible with val type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(val)));
		return RND_EILLEGAL;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	p = rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
	*(long*)p = val;
	return 0;
}

int rnd_queue_setsc(struct rnd_queue *queue, size_t idx, signed char val)
{
	char *p;
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (queue->elem_size != sizeof(val)) {
		error(("queue->elem_size is incompatible with val type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(val)));
		return RND_EILLEGAL;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	p = rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
	*(signed char*)p = val;
	return 0;
}

int rnd_queue_setuc(struct rnd_queue *queue, size_t idx, unsigned char val)
{
	char *p;
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (queue->elem_size != sizeof(val)) {
		error(("queue->elem_size is incompatible with val type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(val)));
		return RND_EILLEGAL;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	p = rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
	*(unsigned char*)p = val;
	return 0;
}

int rnd_queue_setus(struct rnd_queue *queue, size_t idx, unsigned short val)
{
	char *p;
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (queue->elem_size != sizeof(val)) {
		error(("queue->elem_size is incompatible with val type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(val)));
		return RND_EILLEGAL;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	p = rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
	*(unsigned short*)p = val;
	return 0;
}

int rnd_queue_setui(struct rnd_queue *queue, size_t idx, unsigned int val)
{
	char *p;
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (queue->elem_size != sizeof(val)) {
		error(("queue->elem_size is incompatible with val type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(val)));
		return RND_EILLEGAL;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	p = rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
	*(unsigned int*)p = val;
	return 0;
}

int rnd_queue_setul(struct rnd_queue *queue, size_t idx, unsigned long val)
{
	char *p;
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (queue->elem_size != sizeof(val)) {
		error(("queue->elem_size is incompatible with val type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(val)));
		return RND_EILLEGAL;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	p = rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
	*(unsigned long*)p = val;
	return 0;
}

int rnd_queue_setf(struct rnd_queue *queue, size_t idx, float val)
{
	char *p;
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (queue->elem_size != sizeof(val)) {
		error(("queue->elem_size is incompatible with val type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(val)));
		return RND_EILLEGAL;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	p = rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
	*(float*)p = val;
	return 0;
}

int rnd_queue_setd(struct rnd_queue *queue, size_t idx, double val)
{
	char *p;
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (queue->elem_size != sizeof(val)) {
		error(("queue->elem_size is incompatible with val type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(val)));
		return RND_EILLEGAL;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	p = rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
	*(double*)p = val;
	return 0;
}

int rnd_queue_setld(struct rnd_queue *queue, size_t idx, long double val)
{
	char *p;
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (queue->elem_size != sizeof(val)) {
		error(("queue->elem_size is incompatible with val type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(long double)));
		return RND_EILLEGAL;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	p = rnd_ringbuf_get(idx, queue->data, queue->elem_size, queue->capacity, queue->head);
	*(long double*)p = val;
	return 0;
}


int rnd_queue_print(struct rnd_queue *queue)
{
	size_t i;
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
#endif
	printf("rnd_queue_print()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)queue->size, (unsigned long)queue->capacity, (unsigned long)queue->elem_size);
	for (i = 0; i < queue->size; i++) {
		const void *const elem = rnd_ringbuf_get(i, queue->data, queue->elem_size, queue->capacity, queue->head);
		printf("[%lu]\t%p\n", (unsigned long)i, elem);
	}
	return 0;
}

int rnd_queue_printc(struct rnd_queue *queue)
{
	size_t i;
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (queue->elem_size != sizeof(char)) {
		error(("queue->elem_size is incompatible with function type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(char)));
		return RND_EILLEGAL;
	}
#endif
	printf("rnd_queue_printc()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)queue->size, (unsigned long)queue->capacity, (unsigned long)queue->elem_size);
	for (i = 0; i < queue->size; i++) {
		const char elem = *(char*)rnd_ringbuf_get(i, queue->data, queue->elem_size, queue->capacity, queue->head);
		printf("[%lu]\t%hd\t'%c'\n", (unsigned long)i, elem, elem);
	}
	return 0;
}

int rnd_queue_prints(struct rnd_queue *queue)
{
	size_t i;
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (queue->elem_size != sizeof(short)) {
		error(("queue->elem_size is incompatible with function type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(short)));
		return RND_EILLEGAL;
	}
#endif
	printf("rnd_queue_prints()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)queue->size, (unsigned long)queue->capacity, (unsigned long)queue->elem_size);
	for (i = 0; i < queue->size; i++) {
		const short elem = *(short*)rnd_ringbuf_get(i, queue->data, queue->elem_size, queue->capacity, queue->head);
		printf("[%lu]\t%hd\n", (unsigned long)i, elem);
	}
	return 0;
}

int rnd_queue_printi(struct rnd_queue *queue)
{
	size_t i;
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (queue->elem_size != sizeof(int)) {
		error(("queue->elem_size is incompatible with function type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(int)));
		return RND_EILLEGAL;
	}
#endif
	printf("rnd_queue_printi()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)queue->size, (unsigned long)queue->capacity, (unsigned long)queue->elem_size);
	for (i = 0; i < queue->size; i++) {
		const int elem = *(int*)rnd_ringbuf_get(i, queue->data, queue->elem_size, queue->capacity, queue->head);
		printf("[%lu]\t%d\n", (unsigned long)i, elem);
	}
	return 0;
}

int rnd_queue_printl(struct rnd_queue *queue)
{
	size_t i;
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (queue->elem_size != sizeof(long)) {
		error(("queue->elem_size is incompatible with function type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(long)));
		return RND_EILLEGAL;
	}
#endif
	printf("rnd_queue_printl()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)queue->size, (unsigned long)queue->capacity, (unsigned long)queue->elem_size);
	for (i = 0; i < queue->size; i++) {
		const long elem = *(long*)rnd_ringbuf_get(i, queue->data, queue->elem_size, queue->capacity, queue->head);
		printf("[%lu]\t%ld\n", (unsigned long)i, elem);
	}
	return 0;
}

int rnd_queue_printsc(struct rnd_queue *queue)
{
	size_t i;
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (queue->elem_size != sizeof(signed char)) {
		error(("queue->elem_size is incompatible with function type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(signed char)));
		return RND_EILLEGAL;
	}
#endif
	printf("rnd_queue_printsc()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)queue->size, (unsigned long)queue->capacity, (unsigned long)queue->elem_size);
	for (i = 0; i < queue->size; i++) {
		const signed char elem = *(signed char*)rnd_ringbuf_get(i, queue->data, queue->elem_size, queue->capacity, queue->head);
		printf("[%lu]\t%hd\t'%c'\n", (unsigned long)i, elem, elem);
	}
	return 0;
}

int rnd_queue_printuc(struct rnd_queue *queue)
{
	size_t i;
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (queue->elem_size != sizeof(unsigned char)) {
		error(("queue->elem_size is incompatible with function type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(unsigned char)));
		return RND_EILLEGAL;
	}
#endif
	printf("rnd_queue_printuc()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)queue->size, (unsigned long)queue->capacity, (unsigned long)queue->elem_size);
	for (i = 0; i < queue->size; i++) {
		const unsigned char elem = *(unsigned char*)rnd_ringbuf_get(i, queue->data, queue->elem_size, queue->capacity, queue->head);
		printf("[%lu]\t%hd\t'%c'\n", (unsigned long)i, elem, elem);
	}
	return 0;
}

int rnd_queue_printus(struct rnd_queue *queue)
{
	size_t i;
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (queue->elem_size != sizeof(unsigned short)) {
		error(("queue->elem_size is incompatible with function type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(unsigned short)));
		return RND_EILLEGAL;
	}
#endif
	printf("rnd_queue_printus()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)queue->size, (unsigned long)queue->capacity, (unsigned long)queue->elem_size);
	for (i = 0; i < queue->size; i++) {
		const unsigned short elem = *(unsigned short*)rnd_ringbuf_get(i, queue->data, queue->elem_size, queue->capacity, queue->head);
		printf("[%lu]\t%hu\n", (unsigned long)i, elem);
	}
	return 0;
}

int rnd_queue_printui(struct rnd_queue *queue)
{
	size_t i;
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (queue->elem_size != sizeof(unsigned int)) {
		error(("queue->elem_size is incompatible with function type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(unsigned int)));
		return RND_EILLEGAL;
	}
#endif
	printf("rnd_queue_printui()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)queue->size, (unsigned long)queue->capacity, (unsigned long)queue->elem_size);
	for (i = 0; i < queue->size; i++) {
		const unsigned int elem = *(unsigned int*)rnd_ringbuf_get(i, queue->data, queue->elem_size, queue->capacity, queue->head);
		printf("[%lu]\t%u\n", (unsigned long)i, elem);
	}
	return 0;
}

int rnd_queue_printul(struct rnd_queue *queue)
{
	size_t i;
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (queue->elem_size != sizeof(unsigned long)) {
		error(("queue->elem_size is incompatible with function type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(unsigned long)));
		return RND_EILLEGAL;
	}
#endif
	printf("rnd_queue_printul()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)queue->size, (unsigned long)queue->capacity, (unsigned long)queue->elem_size);
	for (i = 0; i < queue->size; i++) {
		const unsigned long elem = *(unsigned long*)rnd_ringbuf_get(i, queue->data, queue->elem_size, queue->capacity, queue->head);
		printf("[%lu]\t%lu\n", (unsigned long)i, elem);
	}
	return 0;
}

int rnd_queue_printf(struct rnd_queue *queue)
{
	size_t i;
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (queue->elem_size != sizeof(float)) {
		error(("queue->elem_size is incompatible with function type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(float)));
		return RND_EILLEGAL;
	}
#endif
	printf("rnd_queue_printf()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)queue->size, (unsigned long)queue->capacity, (unsigned long)queue->elem_size);
	for (i = 0; i < queue->size; i++) {
		const float elem = *(float*)rnd_ringbuf_get(i, queue->data, queue->elem_size, queue->capacity, queue->head);
		printf("[%lu]\t%g\n", (unsigned long)i, elem);
	}
	return 0;
}

int rnd_queue_printd(struct rnd_queue *queue)
{
	size_t i;
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (queue->elem_size != sizeof(double)) {
		error(("queue->elem_size is incompatible with function type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(double)));
		return RND_EILLEGAL;
	}
#endif
	printf("rnd_queue_printd()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)queue->size, (unsigned long)queue->capacity, (unsigned long)queue->elem_size);
	for (i = 0; i < queue->size; i++) {
		const double elem = *(double*)rnd_ringbuf_get(i, queue->data, queue->elem_size, queue->capacity, queue->head);
		printf("[%lu]\t%g\n", (unsigned long)i, elem);
	}
	return 0;
}

int rnd_queue_printld(struct rnd_queue *queue)
{
	size_t i;
#ifdef RND_DEBUG
	if (queue == NULL) {
		error(("queue is NULL"));
		return RND_EINVAL;
	}
	if (queue->elem_size != sizeof(long double)) {
		error(("queue->elem_size is incompatible with function type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(long double)));
		return RND_EILLEGAL;
	}
#endif
	printf("rnd_queue_printld()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)queue->size, (unsigned long)queue->capacity, (unsigned long)queue->elem_size);
	for (i = 0; i < queue->size; i++) {
		const long double elem = *(long double*)rnd_ringbuf_get(i, queue->data, queue->elem_size, queue->capacity, queue->head);
		printf("[%lu]\t%Lg\n", (unsigned long)i, elem);
	}
	return 0;
}
