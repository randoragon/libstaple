#include "rnd_queue.h"
#include "helpers.h"
#include "rnd_errcodes.h"
#include <string.h>

struct rnd_queue *rnd_queue_create(size_t elem_size, size_t capacity)
{
	struct rnd_queue *ret;

	if (capacity > SIZE_MAX / elem_size) {
		error(("size_t overflow detected, maximum size exceeded"));
		return NULL;
	}

	ret = malloc(sizeof(*ret));
	if (ret == NULL) {
		error(("malloc"));
		return NULL;
	}

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
			if (queue->head == queue->data + queue->elem_size * (queue->capacity - 1))
				queue->head = queue->data;
			else
				queue->head += queue->elem_size;
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
	if (cpy == NULL) {
		char *s = src->head;
		while (s != src->tail) {
			memcpy(dest->tail, s, src->elem_size);
			if (s == src->data + (src->size - 1) * src->elem_size) {
				s = src->data;
			} else {
				s += src->elem_size;
			}
			if (dest->tail == dest->data + (dest->size - 1) * dest->elem_size) {
				dest->tail = dest->data;
			} else {
				dest->tail += dest->elem_size;
			}
		}
	} else {
		char *s = src->head;
		while (s != src->tail) {
			int err;
			if ((err = cpy(dest->tail, s))) {
				error(("external cpy function returned %d (non-0)", err));
				return RND_EHANDLER;
			}
			if (s == src->data + (src->size - 1) * src->elem_size) {
				s = src->data;
			} else {
				s += src->elem_size;
			}
			if (dest->tail == dest->data + (dest->size - 1) * dest->elem_size) {
				dest->tail = dest->data;
			} else {
				dest->tail += dest->elem_size;
			}
		}
	}
	return 0;
}

int rnd_queue_map(struct rnd_queue *queue, int (*func)(void*, size_t))
{
	size_t i;
	char *p;
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
		if (p == (char*)queue->data + (queue->size - 1) * queue->elem_size)
			p = queue->data;
		else
			p += queue->elem_size;
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
	if (rnd_buffit(&queue->data, queue->elem_size, queue->size, &queue->capacity))
		return RND_ENOMEM;
	memcpy((char*)queue->data + queue->size++ * queue->elem_size, elem, queue->elem_size);
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
	if (rnd_buffit(&queue->data, queue->elem_size, queue->size, &queue->capacity))
		return RND_ENOMEM;
	((char*)queue->data)[queue->size++] = elem;
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
	if (rnd_buffit(&queue->data, queue->elem_size, queue->size, &queue->capacity))
		return RND_ENOMEM;
	((short*)queue->data)[queue->size++] = elem;
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
	if (rnd_buffit(&queue->data, queue->elem_size, queue->size, &queue->capacity))
		return RND_ENOMEM;
	((int*)queue->data)[queue->size++] = elem;
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
	if (rnd_buffit(&queue->data, queue->elem_size, queue->size, &queue->capacity))
		return RND_ENOMEM;
	((long*)queue->data)[queue->size++] = elem;
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
	if (rnd_buffit(&queue->data, queue->elem_size, queue->size, &queue->capacity))
		return RND_ENOMEM;
	((signed char*)queue->data)[queue->size++] = elem;
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
	if (rnd_buffit(&queue->data, queue->elem_size, queue->size, &queue->capacity))
		return RND_ENOMEM;
	((unsigned char*)queue->data)[queue->size++] = elem;
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
	if (rnd_buffit(&queue->data, queue->elem_size, queue->size, &queue->capacity))
		return RND_ENOMEM;
	((unsigned short*)queue->data)[queue->size++] = elem;
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
	if (rnd_buffit(&queue->data, queue->elem_size, queue->size, &queue->capacity))
		return RND_ENOMEM;
	((unsigned int*)queue->data)[queue->size++] = elem;
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
	if (rnd_buffit(&queue->data, queue->elem_size, queue->size, &queue->capacity))
		return RND_ENOMEM;
	((unsigned long*)queue->data)[queue->size++] = elem;
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
	if (rnd_buffit(&queue->data, queue->elem_size, queue->size, &queue->capacity))
		return RND_ENOMEM;
	((float*)queue->data)[queue->size++] = elem;
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
	if (rnd_buffit(&queue->data, queue->elem_size, queue->size, &queue->capacity))
		return RND_ENOMEM;
	((double*)queue->data)[queue->size++] = elem;
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
	if (rnd_buffit(&queue->data, queue->elem_size, queue->size, &queue->capacity))
		return RND_ENOMEM;
	((long double*)queue->data)[queue->size++] = elem;
	return 0;
}


int rnd_queue_insert(struct rnd_queue *queue, size_t idx, const void *elem)
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
	p = (char*)queue->data + (queue->size - idx) * queue->elem_size;
	memmove(p + queue->elem_size, p, idx * queue->elem_size);
	memcpy(p, elem, queue->elem_size);
	++queue->size;
	return 0;
}

int rnd_queue_insertc(struct rnd_queue *queue, size_t idx, char elem)
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
	p = (char*)queue->data + (queue->size - idx) * queue->elem_size;
	memmove(p + queue->elem_size, p, idx * queue->elem_size);
	*(char*)p = elem;
	++queue->size;
	return 0;
}

int rnd_queue_inserts(struct rnd_queue *queue, size_t idx, short elem)
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
	p = (char*)queue->data + (queue->size - idx) * queue->elem_size;
	memmove(p + queue->elem_size, p, idx * queue->elem_size);
	*(short*)p = elem;
	++queue->size;
	return 0;
}

int rnd_queue_inserti(struct rnd_queue *queue, size_t idx, int elem)
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
	p = (char*)queue->data + (queue->size - idx) * queue->elem_size;
	memmove(p + queue->elem_size, p, idx * queue->elem_size);
	*(int*)p = elem;
	++queue->size;
	return 0;
}

int rnd_queue_insertl(struct rnd_queue *queue, size_t idx, long elem)
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
	p = (char*)queue->data + (queue->size - idx) * queue->elem_size;
	memmove(p + queue->elem_size, p, idx * queue->elem_size);
	*(long*)p = elem;
	++queue->size;
	return 0;
}

int rnd_queue_insertsc(struct rnd_queue *queue, size_t idx, signed char elem)
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
	if (idx > queue->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	if (rnd_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return RND_ERANGE;
	if (rnd_buffit(&queue->data, queue->elem_size, queue->size, &queue->capacity))
		return RND_ENOMEM;
	p = (char*)queue->data + (queue->size - idx) * queue->elem_size;
	memmove(p + queue->elem_size, p, idx * queue->elem_size);
	*(signed char*)p = elem;
	++queue->size;
	return 0;
}

int rnd_queue_insertuc(struct rnd_queue *queue, size_t idx, unsigned char elem)
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
	p = (char*)queue->data + (queue->size - idx) * queue->elem_size;
	memmove(p + queue->elem_size, p, idx * queue->elem_size);
	*(unsigned char*)p = elem;
	++queue->size;
	return 0;
}

int rnd_queue_insertus(struct rnd_queue *queue, size_t idx, unsigned short elem)
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
	if (idx > queue->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	if (rnd_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return RND_ERANGE;
	if (rnd_buffit(&queue->data, queue->elem_size, queue->size, &queue->capacity))
		return RND_ENOMEM;
	p = (char*)queue->data + (queue->size - idx) * queue->elem_size;
	memmove(p + queue->elem_size, p, idx * queue->elem_size);
	*(unsigned short*)p = elem;
	++queue->size;
	return 0;
}

int rnd_queue_insertui(struct rnd_queue *queue, size_t idx, unsigned int elem)
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
	p = (char*)queue->data + (queue->size - idx) * queue->elem_size;
	memmove(p + queue->elem_size, p, idx * queue->elem_size);
	*(unsigned int*)p = elem;
	++queue->size;
	return 0;
}

int rnd_queue_insertul(struct rnd_queue *queue, size_t idx, unsigned long elem)
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
	p = (char*)queue->data + (queue->size - idx) * queue->elem_size;
	memmove(p + queue->elem_size, p, idx * queue->elem_size);
	*(unsigned long*)p = elem;
	++queue->size;
	return 0;
}

int rnd_queue_insertf(struct rnd_queue *queue, size_t idx, float elem)
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
	p = (char*)queue->data + (queue->size - idx) * queue->elem_size;
	memmove(p + queue->elem_size, p, idx * queue->elem_size);
	*(float*)p = elem;
	++queue->size;
	return 0;
}

int rnd_queue_insertd(struct rnd_queue *queue, size_t idx, double elem)
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
	p = (char*)queue->data + (queue->size - idx) * queue->elem_size;
	memmove(p + queue->elem_size, p, idx * queue->elem_size);
	*(double*)p = elem;
	++queue->size;
	return 0;
}

int rnd_queue_insertld(struct rnd_queue *queue, size_t idx, long double elem)
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
	p = (char*)queue->data + (queue->size - idx) * queue->elem_size;
	memmove(p + queue->elem_size, p, idx * queue->elem_size);
	*(long double*)p = elem;
	++queue->size;
	return 0;
}


int rnd_queue_quickinsert(struct rnd_queue *queue, size_t idx, const void *elem)
{
	char *p, *q;
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
	p = (char*)queue->data + (queue->size - idx) * queue->elem_size;
	q = (char*)queue->data + queue->size * queue->elem_size;
	memcpy(q, p, queue->elem_size);
	memcpy(p, elem, queue->elem_size);
	++queue->size;
	return 0;
}

int rnd_queue_quickinsertc(struct rnd_queue *queue, size_t idx, char elem)
{
	char *p, *q;
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
	p = (char*)queue->data + (queue->size - idx) * queue->elem_size;
	q = (char*)queue->data + queue->size * queue->elem_size;
	*(char*)q = *(char*)p;
	*(char*)p = elem;
	++queue->size;
	return 0;
}

int rnd_queue_quickinserts(struct rnd_queue *queue, size_t idx, short elem)
{
	char *p, *q;
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
	p = (char*)queue->data + (queue->size - idx) * queue->elem_size;
	q = (char*)queue->data + queue->size * queue->elem_size;
	*(short*)q = *(short*)p;
	*(short*)p = elem;
	++queue->size;
	return 0;
}

int rnd_queue_quickinserti(struct rnd_queue *queue, size_t idx, int elem)
{
	char *p, *q;
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
	p = (char*)queue->data + (queue->size - idx) * queue->elem_size;
	q = (char*)queue->data + queue->size * queue->elem_size;
	*(int*)q = *(int*)p;
	*(int*)p = elem;
	++queue->size;
	return 0;
}

int rnd_queue_quickinsertl(struct rnd_queue *queue, size_t idx, long elem)
{
	char *p, *q;
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
	p = (char*)queue->data + (queue->size - idx) * queue->elem_size;
	q = (char*)queue->data + queue->size * queue->elem_size;
	*(long*)q = *(long*)p;
	*(long*)p = elem;
	++queue->size;
	return 0;
}

int rnd_queue_quickinsertsc(struct rnd_queue *queue, size_t idx, signed char elem)
{
	char *p, *q;
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
	p = (char*)queue->data + (queue->size - idx) * queue->elem_size;
	q = (char*)queue->data + queue->size * queue->elem_size;
	*(signed char*)q = *(signed char*)p;
	*(signed char*)p = elem;
	++queue->size;
	return 0;
}

int rnd_queue_quickinsertuc(struct rnd_queue *queue, size_t idx, unsigned char elem)
{
	char *p, *q;
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
	p = (char*)queue->data + (queue->size - idx) * queue->elem_size;
	q = (char*)queue->data + queue->size * queue->elem_size;
	*(unsigned char*)q = *(unsigned char*)p;
	*(unsigned char*)p = elem;
	++queue->size;
	return 0;
}

int rnd_queue_quickinsertus(struct rnd_queue *queue, size_t idx, unsigned short elem)
{
	char *p, *q;
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
	p = (char*)queue->data + (queue->size - idx) * queue->elem_size;
	q = (char*)queue->data + queue->size * queue->elem_size;
	*(unsigned short*)q = *(unsigned short*)p;
	*(unsigned short*)p = elem;
	++queue->size;
	return 0;
}

int rnd_queue_quickinsertui(struct rnd_queue *queue, size_t idx, unsigned int elem)
{
	char *p, *q;
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
	p = (char*)queue->data + (queue->size - idx) * queue->elem_size;
	q = (char*)queue->data + queue->size * queue->elem_size;
	*(unsigned int*)q = *(unsigned int*)p;
	*(unsigned int*)p = elem;
	++queue->size;
	return 0;
}

int rnd_queue_quickinsertul(struct rnd_queue *queue, size_t idx, unsigned long elem)
{
	char *p, *q;
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
	p = (char*)queue->data + (queue->size - idx) * queue->elem_size;
	q = (char*)queue->data + queue->size * queue->elem_size;
	*(unsigned long*)q = *(unsigned long*)p;
	*(unsigned long*)p = elem;
	++queue->size;
	return 0;
}

int rnd_queue_quickinsertf(struct rnd_queue *queue, size_t idx, float elem)
{
	char *p, *q;
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
	p = (char*)queue->data + (queue->size - idx) * queue->elem_size;
	q = (char*)queue->data + queue->size * queue->elem_size;
	*(float*)q = *(float*)p;
	*(float*)p = elem;
	++queue->size;
	return 0;
}

int rnd_queue_quickinsertd(struct rnd_queue *queue, size_t idx, double elem)
{
	char *p, *q;
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
	p = (char*)queue->data + (queue->size - idx) * queue->elem_size;
	q = (char*)queue->data + queue->size * queue->elem_size;
	*(double*)q = *(double*)p;
	*(double*)p = elem;
	++queue->size;
	return 0;
}

int rnd_queue_quickinsertld(struct rnd_queue *queue, size_t idx, long double elem)
{
	char *p, *q;
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
	p = (char*)queue->data + (queue->size - idx) * queue->elem_size;
	q = (char*)queue->data + queue->size * queue->elem_size;
	*(long double*)q = *(long double*)p;
	*(long double*)p = elem;
	++queue->size;
	return 0;
}


int rnd_queue_peek(const struct rnd_queue *queue, void *output)
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
	if (queue->size == 0) {
		error(("queue is empty"));
		return RND_EILLEGAL;
	}
#endif
	src = (char*)queue->data + (queue->size - 1) * queue->elem_size;
	memcpy(output, src, queue->elem_size);
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
	return ((char*)queue->data)[queue->size - 1];
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
	return ((short*)queue->data)[queue->size - 1];
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
	return ((int*)queue->data)[queue->size - 1];
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
	return ((long*)queue->data)[queue->size - 1];
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
	return ((signed char*)queue->data)[queue->size - 1];
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
	return ((unsigned char*)queue->data)[queue->size - 1];
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
	return ((unsigned short*)queue->data)[queue->size - 1];
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
	return ((unsigned int*)queue->data)[queue->size - 1];
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
	return ((unsigned long*)queue->data)[queue->size - 1];
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
	return ((float*)queue->data)[queue->size - 1];
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
	return ((double*)queue->data)[queue->size - 1];
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
	return ((long double*)queue->data)[queue->size - 1];
}


int rnd_queue_pop(struct rnd_queue *queue, void *output)
{
	const void *src;
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
	src = (char*)queue->data + (queue->size - 1) * queue->elem_size;
	if (output != NULL)
		memcpy(output, src, queue->elem_size);
	--queue->size;
	return 0;
}

char rnd_queue_popc(struct rnd_queue *queue)
{
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
	return ((char*)queue->data)[--queue->size];
}

short rnd_queue_pops(struct rnd_queue *queue)
{
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
	return ((short*)queue->data)[--queue->size];
}

int rnd_queue_popi(struct rnd_queue *queue)
{
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
	return ((int*)queue->data)[--queue->size];
}

long rnd_queue_popl(struct rnd_queue *queue)
{
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
	return ((long*)queue->data)[--queue->size];
}

signed char rnd_queue_popsc(struct rnd_queue *queue)
{
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
	return ((signed char*)queue->data)[--queue->size];
}

unsigned char rnd_queue_popuc(struct rnd_queue *queue)
{
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
	return ((unsigned char*)queue->data)[--queue->size];
}

unsigned short rnd_queue_popus(struct rnd_queue *queue)
{
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
	return ((unsigned short*)queue->data)[--queue->size];
}

unsigned int rnd_queue_popui(struct rnd_queue *queue)
{
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
	return ((unsigned int*)queue->data)[--queue->size];
}

unsigned long rnd_queue_popul(struct rnd_queue *queue)
{
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
	return ((unsigned long*)queue->data)[--queue->size];
}

float rnd_queue_popf(struct rnd_queue *queue)
{
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
	return ((float*)queue->data)[--queue->size];
}

double rnd_queue_popd(struct rnd_queue *queue)
{
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
	return ((double*)queue->data)[--queue->size];
}

long double rnd_queue_popld(struct rnd_queue *queue)
{
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
	return ((long double*)queue->data)[--queue->size];
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
	p = (char*)queue->data + (queue->size - 1 - idx) * queue->elem_size;
	if (output != NULL)
		memcpy(output, p, queue->elem_size);
	memmove(p, p + queue->elem_size, idx * queue->elem_size);
	--queue->size;
	return 0;
}

char rnd_queue_removec(struct rnd_queue *queue, size_t idx)
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
	p = (char*)queue->data + (queue->size - 1 - idx) * queue->elem_size;
	ret = *(char*)p;
	memmove(p, p + queue->elem_size, idx * queue->elem_size);
	--queue->size;
	return ret;
}

short rnd_queue_removes(struct rnd_queue *queue, size_t idx)
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
#endif
	if (idx >= queue->size) {
		error(("index out of range"));
		return 0;
	}
	p = (char*)queue->data + (queue->size - 1 - idx) * queue->elem_size;
	ret = *(short*)p;
	memmove(p, p + queue->elem_size, idx * queue->elem_size);
	--queue->size;
	return ret;
}

int rnd_queue_removei(struct rnd_queue *queue, size_t idx)
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
	p = (char*)queue->data + (queue->size - 1 - idx) * queue->elem_size;
	ret = *(int*)p;
	memmove(p, p + queue->elem_size, idx * queue->elem_size);
	--queue->size;
	return ret;
}

long rnd_queue_removel(struct rnd_queue *queue, size_t idx)
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
	if (idx >= queue->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	p = (char*)queue->data + (queue->size - 1 - idx) * queue->elem_size;
	ret = *(long*)p;
	memmove(p, p + queue->elem_size, idx * queue->elem_size);
	--queue->size;
	return ret;
}

signed char rnd_queue_removesc(struct rnd_queue *queue, size_t idx)
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
	p = (char*)queue->data + (queue->size - 1 - idx) * queue->elem_size;
	ret = *(signed char*)p;
	memmove(p, p + queue->elem_size, idx * queue->elem_size);
	--queue->size;
	return ret;
}

unsigned char rnd_queue_removeuc(struct rnd_queue *queue, size_t idx)
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
	p = (char*)queue->data + (queue->size - 1 - idx) * queue->elem_size;
	ret = *(unsigned char*)p;
	memmove(p, p + queue->elem_size, idx * queue->elem_size);
	--queue->size;
	return ret;
}

unsigned short rnd_queue_removeus(struct rnd_queue *queue, size_t idx)
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
	p = (char*)queue->data + (queue->size - 1 - idx) * queue->elem_size;
	ret = *(unsigned short*)p;
	memmove(p, p + queue->elem_size, idx * queue->elem_size);
	--queue->size;
	return ret;
}

unsigned int rnd_queue_removeui(struct rnd_queue *queue, size_t idx)
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
	p = (char*)queue->data + (queue->size - 1 - idx) * queue->elem_size;
	ret = *(unsigned int*)p;
	memmove(p, p + queue->elem_size, idx * queue->elem_size);
	--queue->size;
	return ret;
}

unsigned long rnd_queue_removeul(struct rnd_queue *queue, size_t idx)
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
	p = (char*)queue->data + (queue->size - 1 - idx) * queue->elem_size;
	ret = *(unsigned long*)p;
	memmove(p, p + queue->elem_size, idx * queue->elem_size);
	--queue->size;
	return ret;
}

float rnd_queue_removef(struct rnd_queue *queue, size_t idx)
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
	p = (char*)queue->data + (queue->size - 1 - idx) * queue->elem_size;
	ret = *(float*)p;
	memmove(p, p + queue->elem_size, idx * queue->elem_size);
	--queue->size;
	return ret;
}

double rnd_queue_removed(struct rnd_queue *queue, size_t idx)
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
	p = (char*)queue->data + (queue->size - 1 - idx) * queue->elem_size;
	ret = *(double*)p;
	memmove(p, p + queue->elem_size, idx * queue->elem_size);
	--queue->size;
	return ret;
}

long double rnd_queue_removeld(struct rnd_queue *queue, size_t idx)
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
	p = (char*)queue->data + (queue->size - 1 - idx) * queue->elem_size;
	ret = *(long double*)p;
	memmove(p, p + queue->elem_size, idx * queue->elem_size);
	--queue->size;
	return ret;
}


int rnd_queue_quickremove(struct rnd_queue *queue, size_t idx, void *output)
{
	char *p, *q;
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
	p = (char*)queue->data + (queue->size - 1 - idx) * queue->elem_size;
	q = (char*)queue->data + (queue->size - 1) * queue->elem_size;
	if (output != NULL)
		memcpy(output, p, queue->elem_size);
	memcpy(p, q, queue->elem_size);
	--queue->size;
	return 0;
}

char rnd_queue_quickremovec(struct rnd_queue *queue, size_t idx)
{
	char *p, *q;
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
	p = (char*)queue->data + (queue->size - 1 - idx) * queue->elem_size;
	q = (char*)queue->data + (queue->size - 1) * queue->elem_size;
	ret = *(char*)p;
	*(char*)p = *(char*)q;
	--queue->size;
	return ret;
}

short rnd_queue_quickremoves(struct rnd_queue *queue, size_t idx)
{
	char *p, *q;
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
#endif
	if (idx >= queue->size) {
		error(("index out of range"));
		return 0;
	}
	p = (char*)queue->data + (queue->size - 1 - idx) * queue->elem_size;
	q = (char*)queue->data + (queue->size - 1) * queue->elem_size;
	ret = *(short*)p;
	*(short*)p = *(short*)q;
	--queue->size;
	return ret;
}

int rnd_queue_quickremovei(struct rnd_queue *queue, size_t idx)
{
	char *p, *q;
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
	p = (char*)queue->data + (queue->size - 1 - idx) * queue->elem_size;
	q = (char*)queue->data + (queue->size - 1) * queue->elem_size;
	ret = *(int*)p;
	*(int*)p = *(int*)q;
	--queue->size;
	return ret;
}

long rnd_queue_quickremovel(struct rnd_queue *queue, size_t idx)
{
	char *p, *q;
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
	if (idx >= queue->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	p = (char*)queue->data + (queue->size - 1 - idx) * queue->elem_size;
	q = (char*)queue->data + (queue->size - 1) * queue->elem_size;
	ret = *(long*)p;
	*(long*)p = *(long*)q;
	--queue->size;
	return ret;
}

signed char rnd_queue_quickremovesc(struct rnd_queue *queue, size_t idx)
{
	char *p, *q;
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
	p = (char*)queue->data + (queue->size - 1 - idx) * queue->elem_size;
	q = (char*)queue->data + (queue->size - 1) * queue->elem_size;
	ret = *(signed char*)p;
	*(signed char*)p = *(signed char*)q;
	--queue->size;
	return ret;
}

unsigned char rnd_queue_quickremoveuc(struct rnd_queue *queue, size_t idx)
{
	char *p, *q;
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
	p = (char*)queue->data + (queue->size - 1 - idx) * queue->elem_size;
	q = (char*)queue->data + (queue->size - 1) * queue->elem_size;
	ret = *(unsigned char*)p;
	*(unsigned char*)p = *(unsigned char*)q;
	--queue->size;
	return ret;
}

unsigned short rnd_queue_quickremoveus(struct rnd_queue *queue, size_t idx)
{
	char *p, *q;
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
	p = (char*)queue->data + (queue->size - 1 - idx) * queue->elem_size;
	q = (char*)queue->data + (queue->size - 1) * queue->elem_size;
	ret = *(unsigned short*)p;
	*(unsigned short*)p = *(unsigned short*)q;
	--queue->size;
	return ret;
}

unsigned int rnd_queue_quickremoveui(struct rnd_queue *queue, size_t idx)
{
	char *p, *q;
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
	p = (char*)queue->data + (queue->size - 1 - idx) * queue->elem_size;
	q = (char*)queue->data + (queue->size - 1) * queue->elem_size;
	ret = *(unsigned int*)p;
	*(unsigned int*)p = *(unsigned int*)q;
	--queue->size;
	return ret;
}

unsigned long rnd_queue_quickremoveul(struct rnd_queue *queue, size_t idx)
{
	char *p, *q;
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
	p = (char*)queue->data + (queue->size - 1 - idx) * queue->elem_size;
	q = (char*)queue->data + (queue->size - 1) * queue->elem_size;
	ret = *(unsigned long*)p;
	*(unsigned long*)p = *(unsigned long*)q;
	--queue->size;
	return ret;
}

float rnd_queue_quickremovef(struct rnd_queue *queue, size_t idx)
{
	char *p, *q;
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
	p = (char*)queue->data + (queue->size - 1 - idx) * queue->elem_size;
	q = (char*)queue->data + (queue->size - 1) * queue->elem_size;
	ret = *(float*)p;
	*(float*)p = *(float*)q;
	--queue->size;
	return ret;
}

double rnd_queue_quickremoved(struct rnd_queue *queue, size_t idx)
{
	char *p, *q;
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
	p = (char*)queue->data + (queue->size - 1 - idx) * queue->elem_size;
	q = (char*)queue->data + (queue->size - 1) * queue->elem_size;
	ret = *(double*)p;
	*(double*)p = *(double*)q;
	--queue->size;
	return ret;
}

long double rnd_queue_quickremoveld(struct rnd_queue *queue, size_t idx)
{
	char *p, *q;
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
	p = (char*)queue->data + (queue->size - 1 - idx) * queue->elem_size;
	q = (char*)queue->data + (queue->size - 1) * queue->elem_size;
	ret = *(long double*)p;
	*(long double*)p = *(long double*)q;
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
	src = (char*)queue->data + (queue->size - 1 - idx) * queue->elem_size;
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
	return ((char*)queue->data)[queue->size - 1 - idx];
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
	return ((short*)queue->data)[queue->size - 1 - idx];
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
	return ((int*)queue->data)[queue->size - 1 - idx];
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
	return ((long*)queue->data)[queue->size - 1 - idx];
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
	return ((signed char*)queue->data)[queue->size - 1 - idx];
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
	return ((unsigned char*)queue->data)[queue->size - 1 - idx];
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
	return ((unsigned short*)queue->data)[queue->size - 1 - idx];
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
	return ((unsigned int*)queue->data)[queue->size - 1 - idx];
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
	return ((unsigned long*)queue->data)[queue->size - 1 - idx];
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
	return ((float*)queue->data)[queue->size - 1 - idx];
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
	return ((double*)queue->data)[queue->size - 1 - idx];
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
	return ((long double*)queue->data)[queue->size - 1 - idx];
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
	p = (char*)queue->data + (queue->size - 1 - idx) * queue->elem_size;
	memcpy(val, p, queue->elem_size);
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
	p = (char*)queue->data + (queue->size - 1 - idx) * queue->elem_size;
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
	p = (char*)queue->data + (queue->size - 1 - idx) * queue->elem_size;
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
	p = (char*)queue->data + (queue->size - 1 - idx) * queue->elem_size;
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
	p = (char*)queue->data + (queue->size - 1 - idx) * queue->elem_size;
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
	if (idx >= queue->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	p = (char*)queue->data + (queue->size - 1 - idx) * queue->elem_size;
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
	p = (char*)queue->data + (queue->size - 1 - idx) * queue->elem_size;
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
	if (idx >= queue->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	p = (char*)queue->data + (queue->size - 1 - idx) * queue->elem_size;
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
	p = (char*)queue->data + (queue->size - 1 - idx) * queue->elem_size;
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
	p = (char*)queue->data + (queue->size - 1 - idx) * queue->elem_size;
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
	p = (char*)queue->data + (queue->size - 1 - idx) * queue->elem_size;
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
	p = (char*)queue->data + (queue->size - 1 - idx) * queue->elem_size;
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
					(unsigned long)queue->elem_size, sizeof(val)));
		return RND_EILLEGAL;
	}
	if (idx >= queue->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	p = (char*)queue->data + (queue->size - 1 - idx) * queue->elem_size;
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
	for (i = queue->size; i-- > 0;) {
		const void *const elem = (char*)queue->data + i * queue->elem_size;
		printf("[%lu]\t%p\n", (unsigned long)queue->size - 1 - i, elem);
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
		error(("queue->elem_size is incompatible with val type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(val)));
		return RND_EILLEGAL;
	}
#endif
	printf("rnd_queue_printc()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)queue->size, (unsigned long)queue->capacity, (unsigned long)queue->elem_size);
	for (i = queue->size; i-- > 0;) {
		const char elem = ((char*)queue->data)[i];
		printf("[%lu]\t%hd\t'%c'\n", (unsigned long)queue->size - 1 - i, elem, elem);
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
		error(("queue->elem_size is incompatible with val type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(val)));
		return RND_EILLEGAL;
	}
#endif
	printf("rnd_queue_prints()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)queue->size, (unsigned long)queue->capacity, (unsigned long)queue->elem_size);
	for (i = queue->size; i-- > 0;) {
		const short elem = ((short*)queue->data)[i];
		printf("[%lu]\t%hd\n", (unsigned long)queue->size - 1 - i, elem);
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
		error(("queue->elem_size is incompatible with val type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(val)));
		return RND_EILLEGAL;
	}
#endif
	printf("rnd_queue_printi()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)queue->size, (unsigned long)queue->capacity, (unsigned long)queue->elem_size);
	for (i = queue->size; i-- > 0;) {
		const int elem = ((int*)queue->data)[i];
		printf("[%lu]\t%d\n", (unsigned long)queue->size - 1 - i, elem);
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
		error(("queue->elem_size is incompatible with val type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(val)));
		return RND_EILLEGAL;
	}
#endif
	printf("rnd_queue_printl()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)queue->size, (unsigned long)queue->capacity, (unsigned long)queue->elem_size);
	for (i = queue->size; i-- > 0;) {
		const long elem = ((long*)queue->data)[i];
		printf("[%lu]\t%ld\n", (unsigned long)queue->size - 1 - i, elem);
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
		error(("queue->elem_size is incompatible with val type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(val)));
		return RND_EILLEGAL;
	}
#endif
	printf("rnd_queue_printsc()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)queue->size, (unsigned long)queue->capacity, (unsigned long)queue->elem_size);
	for (i = queue->size; i-- > 0;) {
		const signed char elem = ((signed char*)queue->data)[i];
		printf("[%lu]\t%hd\t'%c'\n", (unsigned long)queue->size - 1 - i, elem, elem);
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
		error(("queue->elem_size is incompatible with val type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(val)));
		return RND_EILLEGAL;
	}
#endif
	printf("rnd_queue_printuc()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)queue->size, (unsigned long)queue->capacity, (unsigned long)queue->elem_size);
	for (i = queue->size; i-- > 0;) {
		const unsigned char elem = ((unsigned char*)queue->data)[i];
		printf("[%lu]\t%hd\t'%c'\n", (unsigned long)queue->size - 1 - i, elem, elem);
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
		error(("queue->elem_size is incompatible with val type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(val)));
		return RND_EILLEGAL;
	}
#endif
	printf("rnd_queue_printus()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)queue->size, (unsigned long)queue->capacity, (unsigned long)queue->elem_size);
	for (i = queue->size; i-- > 0;) {
		const unsigned short elem = ((unsigned short*)queue->data)[i];
		printf("[%lu]\t%hu\n", (unsigned long)queue->size - 1 - i, elem);
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
		error(("queue->elem_size is incompatible with val type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(val)));
		return RND_EILLEGAL;
	}
#endif
	printf("rnd_queue_printui()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)queue->size, (unsigned long)queue->capacity, (unsigned long)queue->elem_size);
	for (i = queue->size; i-- > 0;) {
		const unsigned int elem = ((unsigned int*)queue->data)[i];
		printf("[%lu]\t%u\n", (unsigned long)queue->size - 1 - i, elem);
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
		error(("queue->elem_size is incompatible with val type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(val)));
		return RND_EILLEGAL;
	}
#endif
	printf("rnd_queue_printul()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)queue->size, (unsigned long)queue->capacity, (unsigned long)queue->elem_size);
	for (i = queue->size; i-- > 0;) {
		const unsigned long elem = ((unsigned long*)queue->data)[i];
		printf("[%lu]\t%lu\n", (unsigned long)queue->size - 1 - i, elem);
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
		error(("queue->elem_size is incompatible with val type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(val)));
		return RND_EILLEGAL;
	}
#endif
	printf("rnd_queue_printf()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)queue->size, (unsigned long)queue->capacity, (unsigned long)queue->elem_size);
	for (i = queue->size; i-- > 0;) {
		const float elem = ((float*)queue->data)[i];
		printf("[%lu]\t%g\n", (unsigned long)queue->size - 1 - i, elem);
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
		error(("queue->elem_size is incompatible with val type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(val)));
		return RND_EILLEGAL;
	}
#endif
	printf("rnd_queue_printd()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)queue->size, (unsigned long)queue->capacity, (unsigned long)queue->elem_size);
	for (i = queue->size; i-- > 0;) {
		const double elem = ((double*)queue->data)[i];
		printf("[%lu]\t%g\n", (unsigned long)queue->size - 1 - i, elem);
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
		error(("queue->elem_size is incompatible with val type (%lu != %lu)",
					(unsigned long)queue->elem_size, sizeof(val)));
		return RND_EILLEGAL;
	}
#endif
	printf("rnd_queue_printld()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)queue->size, (unsigned long)queue->capacity, (unsigned long)queue->elem_size);
	for (i = queue->size; i-- > 0;) {
		const long double elem = ((long double*)queue->data)[i];
		printf("[%lu]\t%Lg\n", (unsigned long)queue->size - 1 - i, elem);
	}
	return 0;
}
