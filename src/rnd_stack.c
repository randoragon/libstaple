#include "rnd_stack.h"
#include "helpers.h"
#include "rnd_errcodes.h"
#include <string.h>

struct rnd_stack *rnd_stack_create(size_t elem_size, size_t capacity)
{
	struct rnd_stack *ret;

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

	return ret;
}

int rnd_stack_clear(struct rnd_stack *stack, int (*dtor)(void*))
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
#endif
	if (dtor != NULL) {
		const void *const end = (char*)stack->data + stack->size * stack->elem_size;
		char *p = stack->data;
		while (p != end) {
			int err;
			if ((err = dtor(p))) {
				error(("external function handler returned %d (non-0)", err));
				return RND_EHANDLER;
			}
			p += stack->elem_size;
		}
	} else {
		stack->size = 0;
	}
	return 0;
}

int rnd_stack_destroy(struct rnd_stack *stack, int (*dtor)(void*))
{
	int error;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
#endif
	if ((error = rnd_stack_clear(stack, dtor)))
		return RND_EHANDLER;
	free(stack->data);
	free(stack);
	return 0;
}

int rnd_stack_copy(struct rnd_stack *dest, const struct rnd_stack *src, int (*cpy)(void*, const void*))
{
	char *s, *d;
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
	if (cpy == NULL) {
		const void *const src_end = (char*)src->data + src->size * src->elem_size;
		s = src->data;
		d = dest->data;
		while (s != src_end) {
			memcpy(d, s, src->elem_size);
			s += src->elem_size;
			d += dest->elem_size;
		}
	} else {
		const void *const src_end = (char*)src->data + src->size * src->elem_size;
		s = src->data;
		d = dest->data;
		while (s != src_end) {
			int err;
			if ((err = cpy(d, s))) {
				error(("external cpy function returned %d (non-0)", err));
				return RND_EHANDLER;
			}
			s += src->elem_size;
			d += dest->elem_size;
		}
	}
	return 0;
}

int rnd_stack_map(struct rnd_stack *stack, int (*func)(void*, size_t))
{
	size_t i;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (func == NULL) {
		error(("func is NULL"));
		return RND_EINVAL;
	}
#endif
	for (i = 0; i < stack->size; i++) {
		void *const p = (char*)stack->data + i * stack->elem_size;
		int err;
		if ((err = func(p, i))) {
			warn(("external func function returned %d (non-0)", err));
			return RND_EHANDLER;
		}
	}
	return 0;
}

int rnd_stack_push(struct rnd_stack *stack, const void *elem)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (elem == NULL) {
		error(("elem is NULL"));
		return RND_EINVAL;
	}
#endif
	if (rnd_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return RND_ERANGE;
	if (rnd_buffit(&stack->data, stack->elem_size, stack->size, &stack->capacity))
		return RND_ENOMEM;
	memcpy((char*)stack->data + stack->size++ * stack->elem_size, elem, stack->elem_size);
	return 0;
}

int rnd_stack_pushc(struct rnd_stack *stack, char elem)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
#endif
	if (rnd_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return RND_ERANGE;
	if (rnd_buffit(&stack->data, stack->elem_size, stack->size, &stack->capacity))
		return RND_ENOMEM;
	((char*)stack->data)[stack->size++] = elem;
	return 0;
}

int rnd_stack_pushs(struct rnd_stack *stack, short elem)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
#endif
	if (rnd_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return RND_ERANGE;
	if (rnd_buffit(&stack->data, stack->elem_size, stack->size, &stack->capacity))
		return RND_ENOMEM;
	((short*)stack->data)[stack->size++] = elem;
	return 0;
}

int rnd_stack_pushi(struct rnd_stack *stack, int elem)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
#endif
	if (rnd_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return RND_ERANGE;
	if (rnd_buffit(&stack->data, stack->elem_size, stack->size, &stack->capacity))
		return RND_ENOMEM;
	((int*)stack->data)[stack->size++] = elem;
	return 0;
}

int rnd_stack_pushl(struct rnd_stack *stack, long elem)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
#endif
	if (rnd_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return RND_ERANGE;
	if (rnd_buffit(&stack->data, stack->elem_size, stack->size, &stack->capacity))
		return RND_ENOMEM;
	((long*)stack->data)[stack->size++] = elem;
	return 0;
}

int rnd_stack_pushsc(struct rnd_stack *stack, signed char elem)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
#endif
	if (rnd_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return RND_ERANGE;
	if (rnd_buffit(&stack->data, stack->elem_size, stack->size, &stack->capacity))
		return RND_ENOMEM;
	((signed char*)stack->data)[stack->size++] = elem;
	return 0;
}

int rnd_stack_pushuc(struct rnd_stack *stack, unsigned char elem)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
#endif
	if (rnd_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return RND_ERANGE;
	if (rnd_buffit(&stack->data, stack->elem_size, stack->size, &stack->capacity))
		return RND_ENOMEM;
	((unsigned char*)stack->data)[stack->size++] = elem;
	return 0;
}

int rnd_stack_pushus(struct rnd_stack *stack, unsigned short elem)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
#endif
	if (rnd_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return RND_ERANGE;
	if (rnd_buffit(&stack->data, stack->elem_size, stack->size, &stack->capacity))
		return RND_ENOMEM;
	((unsigned short*)stack->data)[stack->size++] = elem;
	return 0;
}

int rnd_stack_pushui(struct rnd_stack *stack, unsigned int elem)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
#endif
	if (rnd_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return RND_ERANGE;
	if (rnd_buffit(&stack->data, stack->elem_size, stack->size, &stack->capacity))
		return RND_ENOMEM;
	((unsigned int*)stack->data)[stack->size++] = elem;
	return 0;
}

int rnd_stack_pushul(struct rnd_stack *stack, unsigned long elem)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
#endif
	if (rnd_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return RND_ERANGE;
	if (rnd_buffit(&stack->data, stack->elem_size, stack->size, &stack->capacity))
		return RND_ENOMEM;
	((unsigned long*)stack->data)[stack->size++] = elem;
	return 0;
}

int rnd_stack_pushf(struct rnd_stack *stack, float elem)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
#endif
	if (rnd_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return RND_ERANGE;
	if (rnd_buffit(&stack->data, stack->elem_size, stack->size, &stack->capacity))
		return RND_ENOMEM;
	((float*)stack->data)[stack->size++] = elem;
	return 0;
}

int rnd_stack_pushd(struct rnd_stack *stack, double elem)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
#endif
	if (rnd_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return RND_ERANGE;
	if (rnd_buffit(&stack->data, stack->elem_size, stack->size, &stack->capacity))
		return RND_ENOMEM;
	((double*)stack->data)[stack->size++] = elem;
	return 0;
}

int rnd_stack_pushld(struct rnd_stack *stack, long double elem)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
#endif
	if (rnd_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return RND_ERANGE;
	if (rnd_buffit(&stack->data, stack->elem_size, stack->size, &stack->capacity))
		return RND_ENOMEM;
	((long double*)stack->data)[stack->size++] = elem;
	return 0;
}


int rnd_stack_insert(struct rnd_stack *stack, size_t idx, const void *elem)
{
	char *p;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (elem == NULL) {
		error(("elem is NULL"));
		return RND_EINVAL;
	}
	if (idx > stack->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	if (rnd_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return RND_ERANGE;
	if (rnd_buffit(&stack->data, stack->elem_size, stack->size, &stack->capacity))
		return RND_ENOMEM;
	p = (char*)stack->data + (stack->size - idx) * stack->elem_size;
	memmove(p + stack->elem_size, p, idx * stack->elem_size);
	memcpy(p, elem, stack->elem_size);
	++stack->size;
	return 0;
}

int rnd_stack_insertc(struct rnd_stack *stack, size_t idx, char elem)
{
	char *p;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
	if (idx > stack->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	if (rnd_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return RND_ERANGE;
	if (rnd_buffit(&stack->data, stack->elem_size, stack->size, &stack->capacity))
		return RND_ENOMEM;
	p = (char*)stack->data + (stack->size - idx) * stack->elem_size;
	memmove(p + stack->elem_size, p, idx * stack->elem_size);
	*(char*)p = elem;
	++stack->size;
	return 0;
}

int rnd_stack_inserts(struct rnd_stack *stack, size_t idx, short elem)
{
	char *p;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
	if (idx > stack->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	if (rnd_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return RND_ERANGE;
	if (rnd_buffit(&stack->data, stack->elem_size, stack->size, &stack->capacity))
		return RND_ENOMEM;
	p = (char*)stack->data + (stack->size - idx) * stack->elem_size;
	memmove(p + stack->elem_size, p, idx * stack->elem_size);
	*(short*)p = elem;
	++stack->size;
	return 0;
}

int rnd_stack_inserti(struct rnd_stack *stack, size_t idx, int elem)
{
	char *p;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
	if (idx > stack->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	if (rnd_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return RND_ERANGE;
	if (rnd_buffit(&stack->data, stack->elem_size, stack->size, &stack->capacity))
		return RND_ENOMEM;
	p = (char*)stack->data + (stack->size - idx) * stack->elem_size;
	memmove(p + stack->elem_size, p, idx * stack->elem_size);
	*(int*)p = elem;
	++stack->size;
	return 0;
}

int rnd_stack_insertl(struct rnd_stack *stack, size_t idx, long elem)
{
	char *p;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
	if (idx > stack->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	if (rnd_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return RND_ERANGE;
	if (rnd_buffit(&stack->data, stack->elem_size, stack->size, &stack->capacity))
		return RND_ENOMEM;
	p = (char*)stack->data + (stack->size - idx) * stack->elem_size;
	memmove(p + stack->elem_size, p, idx * stack->elem_size);
	*(long*)p = elem;
	++stack->size;
	return 0;
}

int rnd_stack_insertsc(struct rnd_stack *stack, size_t idx, signed char elem)
{
	char *p;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
	if (idx > stack->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	if (rnd_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return RND_ERANGE;
	if (rnd_buffit(&stack->data, stack->elem_size, stack->size, &stack->capacity))
		return RND_ENOMEM;
	p = (char*)stack->data + (stack->size - idx) * stack->elem_size;
	memmove(p + stack->elem_size, p, idx * stack->elem_size);
	*(signed char*)p = elem;
	++stack->size;
	return 0;
}

int rnd_stack_insertuc(struct rnd_stack *stack, size_t idx, unsigned char elem)
{
	char *p;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
	if (idx > stack->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	if (rnd_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return RND_ERANGE;
	if (rnd_buffit(&stack->data, stack->elem_size, stack->size, &stack->capacity))
		return RND_ENOMEM;
	p = (char*)stack->data + (stack->size - idx) * stack->elem_size;
	memmove(p + stack->elem_size, p, idx * stack->elem_size);
	*(unsigned char*)p = elem;
	++stack->size;
	return 0;
}

int rnd_stack_insertus(struct rnd_stack *stack, size_t idx, unsigned short elem)
{
	char *p;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
	if (idx > stack->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	if (rnd_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return RND_ERANGE;
	if (rnd_buffit(&stack->data, stack->elem_size, stack->size, &stack->capacity))
		return RND_ENOMEM;
	p = (char*)stack->data + (stack->size - idx) * stack->elem_size;
	memmove(p + stack->elem_size, p, idx * stack->elem_size);
	*(unsigned short*)p = elem;
	++stack->size;
	return 0;
}

int rnd_stack_insertui(struct rnd_stack *stack, size_t idx, unsigned int elem)
{
	char *p;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
	if (idx > stack->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	if (rnd_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return RND_ERANGE;
	if (rnd_buffit(&stack->data, stack->elem_size, stack->size, &stack->capacity))
		return RND_ENOMEM;
	p = (char*)stack->data + (stack->size - idx) * stack->elem_size;
	memmove(p + stack->elem_size, p, idx * stack->elem_size);
	*(unsigned int*)p = elem;
	++stack->size;
	return 0;
}

int rnd_stack_insertul(struct rnd_stack *stack, size_t idx, unsigned long elem)
{
	char *p;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
	if (idx > stack->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	if (rnd_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return RND_ERANGE;
	if (rnd_buffit(&stack->data, stack->elem_size, stack->size, &stack->capacity))
		return RND_ENOMEM;
	p = (char*)stack->data + (stack->size - idx) * stack->elem_size;
	memmove(p + stack->elem_size, p, idx * stack->elem_size);
	*(unsigned long*)p = elem;
	++stack->size;
	return 0;
}

int rnd_stack_insertf(struct rnd_stack *stack, size_t idx, float elem)
{
	char *p;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
	if (idx > stack->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	if (rnd_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return RND_ERANGE;
	if (rnd_buffit(&stack->data, stack->elem_size, stack->size, &stack->capacity))
		return RND_ENOMEM;
	p = (char*)stack->data + (stack->size - idx) * stack->elem_size;
	memmove(p + stack->elem_size, p, idx * stack->elem_size);
	*(float*)p = elem;
	++stack->size;
	return 0;
}

int rnd_stack_insertd(struct rnd_stack *stack, size_t idx, double elem)
{
	char *p;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
	if (idx > stack->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	if (rnd_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return RND_ERANGE;
	if (rnd_buffit(&stack->data, stack->elem_size, stack->size, &stack->capacity))
		return RND_ENOMEM;
	p = (char*)stack->data + (stack->size - idx) * stack->elem_size;
	memmove(p + stack->elem_size, p, idx * stack->elem_size);
	*(double*)p = elem;
	++stack->size;
	return 0;
}

int rnd_stack_insertld(struct rnd_stack *stack, size_t idx, long double elem)
{
	char *p;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
	if (idx > stack->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	if (rnd_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return RND_ERANGE;
	if (rnd_buffit(&stack->data, stack->elem_size, stack->size, &stack->capacity))
		return RND_ENOMEM;
	p = (char*)stack->data + (stack->size - idx) * stack->elem_size;
	memmove(p + stack->elem_size, p, idx * stack->elem_size);
	*(long double*)p = elem;
	++stack->size;
	return 0;
}


int rnd_stack_quickinsert(struct rnd_stack *stack, size_t idx, const void *elem)
{
	char *p, *q;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (elem == NULL) {
		error(("elem is NULL"));
		return RND_EINVAL;
	}
	if (idx > stack->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	if (rnd_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return RND_ERANGE;
	if (rnd_buffit(&stack->data, stack->elem_size, stack->size, &stack->capacity))
		return RND_ENOMEM;
	p = (char*)stack->data + (stack->size - idx) * stack->elem_size;
	q = (char*)stack->data + stack->size * stack->elem_size;
	memcpy(q, p, stack->elem_size);
	memcpy(p, elem, stack->elem_size);
	++stack->size;
	return 0;
}

int rnd_stack_quickinsertc(struct rnd_stack *stack, size_t idx, char elem)
{
	char *p, *q;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
	if (idx > stack->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	if (rnd_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return RND_ERANGE;
	if (rnd_buffit(&stack->data, stack->elem_size, stack->size, &stack->capacity))
		return RND_ENOMEM;
	p = (char*)stack->data + (stack->size - idx) * stack->elem_size;
	q = (char*)stack->data + stack->size * stack->elem_size;
	*(char*)q = *(char*)p;
	*(char*)p = elem;
	++stack->size;
	return 0;
}

int rnd_stack_quickinserts(struct rnd_stack *stack, size_t idx, short elem)
{
	char *p, *q;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
	if (idx > stack->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	if (rnd_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return RND_ERANGE;
	if (rnd_buffit(&stack->data, stack->elem_size, stack->size, &stack->capacity))
		return RND_ENOMEM;
	p = (char*)stack->data + (stack->size - idx) * stack->elem_size;
	q = (char*)stack->data + stack->size * stack->elem_size;
	*(short*)q = *(short*)p;
	*(short*)p = elem;
	++stack->size;
	return 0;
}

int rnd_stack_quickinserti(struct rnd_stack *stack, size_t idx, int elem)
{
	char *p, *q;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
	if (idx > stack->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	if (rnd_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return RND_ERANGE;
	if (rnd_buffit(&stack->data, stack->elem_size, stack->size, &stack->capacity))
		return RND_ENOMEM;
	p = (char*)stack->data + (stack->size - idx) * stack->elem_size;
	q = (char*)stack->data + stack->size * stack->elem_size;
	*(int*)q = *(int*)p;
	*(int*)p = elem;
	++stack->size;
	return 0;
}

int rnd_stack_quickinsertl(struct rnd_stack *stack, size_t idx, long elem)
{
	char *p, *q;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
	if (idx > stack->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	if (rnd_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return RND_ERANGE;
	if (rnd_buffit(&stack->data, stack->elem_size, stack->size, &stack->capacity))
		return RND_ENOMEM;
	p = (char*)stack->data + (stack->size - idx) * stack->elem_size;
	q = (char*)stack->data + stack->size * stack->elem_size;
	*(long*)q = *(long*)p;
	*(long*)p = elem;
	++stack->size;
	return 0;
}

int rnd_stack_quickinsertsc(struct rnd_stack *stack, size_t idx, signed char elem)
{
	char *p, *q;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
	if (idx > stack->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	if (rnd_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return RND_ERANGE;
	if (rnd_buffit(&stack->data, stack->elem_size, stack->size, &stack->capacity))
		return RND_ENOMEM;
	p = (char*)stack->data + (stack->size - idx) * stack->elem_size;
	q = (char*)stack->data + stack->size * stack->elem_size;
	*(signed char*)q = *(signed char*)p;
	*(signed char*)p = elem;
	++stack->size;
	return 0;
}

int rnd_stack_quickinsertuc(struct rnd_stack *stack, size_t idx, unsigned char elem)
{
	char *p, *q;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
	if (idx > stack->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	if (rnd_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return RND_ERANGE;
	if (rnd_buffit(&stack->data, stack->elem_size, stack->size, &stack->capacity))
		return RND_ENOMEM;
	p = (char*)stack->data + (stack->size - idx) * stack->elem_size;
	q = (char*)stack->data + stack->size * stack->elem_size;
	*(unsigned char*)q = *(unsigned char*)p;
	*(unsigned char*)p = elem;
	++stack->size;
	return 0;
}

int rnd_stack_quickinsertus(struct rnd_stack *stack, size_t idx, unsigned short elem)
{
	char *p, *q;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
	if (idx > stack->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	if (rnd_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return RND_ERANGE;
	if (rnd_buffit(&stack->data, stack->elem_size, stack->size, &stack->capacity))
		return RND_ENOMEM;
	p = (char*)stack->data + (stack->size - idx) * stack->elem_size;
	q = (char*)stack->data + stack->size * stack->elem_size;
	*(unsigned short*)q = *(unsigned short*)p;
	*(unsigned short*)p = elem;
	++stack->size;
	return 0;
}

int rnd_stack_quickinsertui(struct rnd_stack *stack, size_t idx, unsigned int elem)
{
	char *p, *q;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
	if (idx > stack->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	if (rnd_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return RND_ERANGE;
	if (rnd_buffit(&stack->data, stack->elem_size, stack->size, &stack->capacity))
		return RND_ENOMEM;
	p = (char*)stack->data + (stack->size - idx) * stack->elem_size;
	q = (char*)stack->data + stack->size * stack->elem_size;
	*(unsigned int*)q = *(unsigned int*)p;
	*(unsigned int*)p = elem;
	++stack->size;
	return 0;
}

int rnd_stack_quickinsertul(struct rnd_stack *stack, size_t idx, unsigned long elem)
{
	char *p, *q;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
	if (idx > stack->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	if (rnd_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return RND_ERANGE;
	if (rnd_buffit(&stack->data, stack->elem_size, stack->size, &stack->capacity))
		return RND_ENOMEM;
	p = (char*)stack->data + (stack->size - idx) * stack->elem_size;
	q = (char*)stack->data + stack->size * stack->elem_size;
	*(unsigned long*)q = *(unsigned long*)p;
	*(unsigned long*)p = elem;
	++stack->size;
	return 0;
}

int rnd_stack_quickinsertf(struct rnd_stack *stack, size_t idx, float elem)
{
	char *p, *q;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
	if (idx > stack->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	if (rnd_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return RND_ERANGE;
	if (rnd_buffit(&stack->data, stack->elem_size, stack->size, &stack->capacity))
		return RND_ENOMEM;
	p = (char*)stack->data + (stack->size - idx) * stack->elem_size;
	q = (char*)stack->data + stack->size * stack->elem_size;
	*(float*)q = *(float*)p;
	*(float*)p = elem;
	++stack->size;
	return 0;
}

int rnd_stack_quickinsertd(struct rnd_stack *stack, size_t idx, double elem)
{
	char *p, *q;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
	if (idx > stack->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	if (rnd_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return RND_ERANGE;
	if (rnd_buffit(&stack->data, stack->elem_size, stack->size, &stack->capacity))
		return RND_ENOMEM;
	p = (char*)stack->data + (stack->size - idx) * stack->elem_size;
	q = (char*)stack->data + stack->size * stack->elem_size;
	*(double*)q = *(double*)p;
	*(double*)p = elem;
	++stack->size;
	return 0;
}

int rnd_stack_quickinsertld(struct rnd_stack *stack, size_t idx, long double elem)
{
	char *p, *q;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return RND_EILLEGAL;
	}
	if (idx > stack->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	if (rnd_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return RND_ERANGE;
	if (rnd_buffit(&stack->data, stack->elem_size, stack->size, &stack->capacity))
		return RND_ENOMEM;
	p = (char*)stack->data + (stack->size - idx) * stack->elem_size;
	q = (char*)stack->data + stack->size * stack->elem_size;
	*(long double*)q = *(long double*)p;
	*(long double*)p = elem;
	++stack->size;
	return 0;
}


int rnd_stack_peek(const struct rnd_stack *stack, void *output)
{
	const void *src;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (output == NULL) {
		error(("output is NULL"));
		return RND_EINVAL;
	}
	if (stack->size == 0) {
		error(("stack is empty"));
		return RND_EILLEGAL;
	}
#endif
	src = (char*)stack->data + (stack->size - 1) * stack->elem_size;
	memcpy(output, src, stack->elem_size);
	return 0;
}

char rnd_stack_peekc(const struct rnd_stack *stack)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return 0;
	}
	if (stack->elem_size != sizeof(char)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(char)));
		return 0;
	}
	if (stack->size == 0) {
		error(("stack is empty"));
		return 0;
	}
#endif
	return ((char*)stack->data)[stack->size - 1];
}

short rnd_stack_peeks(const struct rnd_stack *stack)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return 0;
	}
	if (stack->elem_size != sizeof(short)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(short)));
		return 0;
	}
	if (stack->size == 0) {
		error(("stack is empty"));
		return 0;
	}
#endif
	return ((short*)stack->data)[stack->size - 1];
}

int rnd_stack_peeki(const struct rnd_stack *stack)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return 0;
	}
	if (stack->elem_size != sizeof(int)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(int)));
		return 0;
	}
	if (stack->size == 0) {
		error(("stack is empty"));
		return 0;
	}
#endif
	return ((int*)stack->data)[stack->size - 1];
}

long rnd_stack_peekl(const struct rnd_stack *stack)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return 0;
	}
	if (stack->elem_size != sizeof(long)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(long)));
		return 0;
	}
	if (stack->size == 0) {
		error(("stack is empty"));
		return 0;
	}
#endif
	return ((long*)stack->data)[stack->size - 1];
}

signed char rnd_stack_peeksc(const struct rnd_stack *stack)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return 0;
	}
	if (stack->elem_size != sizeof(signed char)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(signed char)));
		return 0;
	}
	if (stack->size == 0) {
		error(("stack is empty"));
		return 0;
	}
#endif
	return ((signed char*)stack->data)[stack->size - 1];
}

unsigned char rnd_stack_peekuc(const struct rnd_stack *stack)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return 0;
	}
	if (stack->elem_size != sizeof(unsigned char)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(unsigned char)));
		return 0;
	}
	if (stack->size == 0) {
		error(("stack is empty"));
		return 0;
	}
#endif
	return ((unsigned char*)stack->data)[stack->size - 1];
}

unsigned short rnd_stack_peekus(const struct rnd_stack *stack)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return 0;
	}
	if (stack->elem_size != sizeof(unsigned short)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(unsigned short)));
		return 0;
	}
	if (stack->size == 0) {
		error(("stack is empty"));
		return 0;
	}
#endif
	return ((unsigned short*)stack->data)[stack->size - 1];
}

unsigned int rnd_stack_peekui(const struct rnd_stack *stack)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return 0;
	}
	if (stack->elem_size != sizeof(unsigned int)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(unsigned int)));
		return 0;
	}
	if (stack->size == 0) {
		error(("stack is empty"));
		return 0;
	}
#endif
	return ((unsigned int*)stack->data)[stack->size - 1];
}

unsigned long rnd_stack_peekul(const struct rnd_stack *stack)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return 0;
	}
	if (stack->elem_size != sizeof(unsigned long)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(unsigned long)));
		return 0;
	}
	if (stack->size == 0) {
		error(("stack is empty"));
		return 0;
	}
#endif
	return ((unsigned long*)stack->data)[stack->size - 1];
}

float rnd_stack_peekf(const struct rnd_stack *stack)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return 0;
	}
	if (stack->elem_size != sizeof(float)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(float)));
		return 0;
	}
	if (stack->size == 0) {
		error(("stack is empty"));
		return 0;
	}
#endif
	return ((float*)stack->data)[stack->size - 1];
}

double rnd_stack_peekd(const struct rnd_stack *stack)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return 0;
	}
	if (stack->elem_size != sizeof(double)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(double)));
		return 0;
	}
	if (stack->size == 0) {
		error(("stack is empty"));
		return 0;
	}
#endif
	return ((double*)stack->data)[stack->size - 1];
}

long double rnd_stack_peekld(const struct rnd_stack *stack)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return 0;
	}
	if (stack->elem_size != sizeof(long double)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(long double)));
		return 0;
	}
	if (stack->size == 0) {
		error(("stack is empty"));
		return 0;
	}
#endif
	return ((long double*)stack->data)[stack->size - 1];
}


int rnd_stack_pop(struct rnd_stack *stack, void *output)
{
	const void *src;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (stack->size == 0) {
		error(("stack is empty"));
		return RND_EILLEGAL;
	}
#endif
	src = (char*)stack->data + (stack->size - 1) * stack->elem_size;
	if (output != NULL)
		memcpy(output, src, stack->elem_size);
	--stack->size;
	return 0;
}

char rnd_stack_popc(struct rnd_stack *stack)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return 0;
	}
	if (stack->size == 0) {
		error(("stack is empty"));
		return 0;
	}
	if (stack->elem_size != sizeof(char)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(char)));
		return 0;
	}
#endif
	return ((char*)stack->data)[--stack->size];
}

short rnd_stack_pops(struct rnd_stack *stack)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return 0;
	}
	if (stack->size == 0) {
		error(("stack is empty"));
		return 0;
	}
	if (stack->elem_size != sizeof(short)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(short)));
		return 0;
	}
#endif
	return ((short*)stack->data)[--stack->size];
}

int rnd_stack_popi(struct rnd_stack *stack)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return 0;
	}
	if (stack->size == 0) {
		error(("stack is empty"));
		return 0;
	}
	if (stack->elem_size != sizeof(int)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(int)));
		return 0;
	}
#endif
	return ((int*)stack->data)[--stack->size];
}

long rnd_stack_popl(struct rnd_stack *stack)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return 0;
	}
	if (stack->size == 0) {
		error(("stack is empty"));
		return 0;
	}
	if (stack->elem_size != sizeof(long)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(long)));
		return 0;
	}
#endif
	return ((long*)stack->data)[--stack->size];
}

signed char rnd_stack_popsc(struct rnd_stack *stack)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return 0;
	}
	if (stack->size == 0) {
		error(("stack is empty"));
		return 0;
	}
	if (stack->elem_size != sizeof(signed char)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(signed char)));
		return 0;
	}
#endif
	return ((signed char*)stack->data)[--stack->size];
}

unsigned char rnd_stack_popuc(struct rnd_stack *stack)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return 0;
	}
	if (stack->size == 0) {
		error(("stack is empty"));
		return 0;
	}
	if (stack->elem_size != sizeof(unsigned char)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(unsigned char)));
		return 0;
	}
#endif
	return ((unsigned char*)stack->data)[--stack->size];
}

unsigned short rnd_stack_popus(struct rnd_stack *stack)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return 0;
	}
	if (stack->size == 0) {
		error(("stack is empty"));
		return 0;
	}
	if (stack->elem_size != sizeof(unsigned short)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(unsigned short)));
		return 0;
	}
#endif
	return ((unsigned short*)stack->data)[--stack->size];
}

unsigned int rnd_stack_popui(struct rnd_stack *stack)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return 0;
	}
	if (stack->size == 0) {
		error(("stack is empty"));
		return 0;
	}
	if (stack->elem_size != sizeof(unsigned int)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(unsigned int)));
		return 0;
	}
#endif
	return ((unsigned int*)stack->data)[--stack->size];
}

unsigned long rnd_stack_popul(struct rnd_stack *stack)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return 0;
	}
	if (stack->size == 0) {
		error(("stack is empty"));
		return 0;
	}
	if (stack->elem_size != sizeof(unsigned long)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(unsigned long)));
		return 0;
	}
#endif
	return ((unsigned long*)stack->data)[--stack->size];
}

float rnd_stack_popf(struct rnd_stack *stack)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return 0;
	}
	if (stack->size == 0) {
		error(("stack is empty"));
		return 0;
	}
	if (stack->elem_size != sizeof(float)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(float)));
		return 0;
	}
#endif
	return ((float*)stack->data)[--stack->size];
}

double rnd_stack_popd(struct rnd_stack *stack)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return 0;
	}
	if (stack->size == 0) {
		error(("stack is empty"));
		return 0;
	}
	if (stack->elem_size != sizeof(double)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(double)));
		return 0;
	}
#endif
	return ((double*)stack->data)[--stack->size];
}

long double rnd_stack_popld(struct rnd_stack *stack)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return 0;
	}
	if (stack->size == 0) {
		error(("stack is empty"));
		return 0;
	}
	if (stack->elem_size != sizeof(long double)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(long double)));
		return 0;
	}
#endif
	return ((long double*)stack->data)[--stack->size];
}


int rnd_stack_remove(struct rnd_stack *stack, size_t idx, void *output)
{
	char *p;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	if (output != NULL)
		memcpy(output, p, stack->elem_size);
	memmove(p, p + stack->elem_size, idx * stack->elem_size);
	--stack->size;
	return 0;
}

char rnd_stack_removec(struct rnd_stack *stack, size_t idx)
{
	char *p;
	char ret;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return 0;
	}
	if (stack->elem_size != sizeof(char)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(char)));
		return 0;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	ret = *(char*)p;
	memmove(p, p + stack->elem_size, idx * stack->elem_size);
	--stack->size;
	return ret;
}

short rnd_stack_removes(struct rnd_stack *stack, size_t idx)
{
	char *p;
	short ret;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return 0;
	}
	if (stack->elem_size != sizeof(short)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(short)));
		return 0;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	ret = *(short*)p;
	memmove(p, p + stack->elem_size, idx * stack->elem_size);
	--stack->size;
	return ret;
}

int rnd_stack_removei(struct rnd_stack *stack, size_t idx)
{
	char *p;
	int ret;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return 0;
	}
	if (stack->elem_size != sizeof(int)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(int)));
		return 0;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	ret = *(int*)p;
	memmove(p, p + stack->elem_size, idx * stack->elem_size);
	--stack->size;
	return ret;
}

long rnd_stack_removel(struct rnd_stack *stack, size_t idx)
{
	char *p;
	long ret;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return 0;
	}
	if (stack->elem_size != sizeof(long)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(long)));
		return 0;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	ret = *(long*)p;
	memmove(p, p + stack->elem_size, idx * stack->elem_size);
	--stack->size;
	return ret;
}

signed char rnd_stack_removesc(struct rnd_stack *stack, size_t idx)
{
	char *p;
	signed char ret;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return 0;
	}
	if (stack->elem_size != sizeof(signed char)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(signed char)));
		return 0;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	ret = *(signed char*)p;
	memmove(p, p + stack->elem_size, idx * stack->elem_size);
	--stack->size;
	return ret;
}

unsigned char rnd_stack_removeuc(struct rnd_stack *stack, size_t idx)
{
	char *p;
	unsigned char ret;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return 0;
	}
	if (stack->elem_size != sizeof(unsigned char)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(unsigned char)));
		return 0;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	ret = *(unsigned char*)p;
	memmove(p, p + stack->elem_size, idx * stack->elem_size);
	--stack->size;
	return ret;
}

unsigned short rnd_stack_removeus(struct rnd_stack *stack, size_t idx)
{
	char *p;
	unsigned short ret;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return 0;
	}
	if (stack->elem_size != sizeof(unsigned short)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(unsigned short)));
		return 0;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	ret = *(unsigned short*)p;
	memmove(p, p + stack->elem_size, idx * stack->elem_size);
	--stack->size;
	return ret;
}

unsigned int rnd_stack_removeui(struct rnd_stack *stack, size_t idx)
{
	char *p;
	unsigned int ret;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return 0;
	}
	if (stack->elem_size != sizeof(unsigned int)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(unsigned int)));
		return 0;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	ret = *(unsigned int*)p;
	memmove(p, p + stack->elem_size, idx * stack->elem_size);
	--stack->size;
	return ret;
}

unsigned long rnd_stack_removeul(struct rnd_stack *stack, size_t idx)
{
	char *p;
	unsigned long ret;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return 0;
	}
	if (stack->elem_size != sizeof(unsigned long)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(unsigned long)));
		return 0;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	ret = *(unsigned long*)p;
	memmove(p, p + stack->elem_size, idx * stack->elem_size);
	--stack->size;
	return ret;
}

float rnd_stack_removef(struct rnd_stack *stack, size_t idx)
{
	char *p;
	float ret;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return 0;
	}
	if (stack->elem_size != sizeof(float)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(float)));
		return 0;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	ret = *(float*)p;
	memmove(p, p + stack->elem_size, idx * stack->elem_size);
	--stack->size;
	return ret;
}

double rnd_stack_removed(struct rnd_stack *stack, size_t idx)
{
	char *p;
	double ret;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return 0;
	}
	if (stack->elem_size != sizeof(double)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(double)));
		return 0;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	ret = *(double*)p;
	memmove(p, p + stack->elem_size, idx * stack->elem_size);
	--stack->size;
	return ret;
}

long double rnd_stack_removeld(struct rnd_stack *stack, size_t idx)
{
	char *p;
	long double ret;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return 0;
	}
	if (stack->elem_size != sizeof(long double)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(long double)));
		return 0;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	ret = *(long double*)p;
	memmove(p, p + stack->elem_size, idx * stack->elem_size);
	--stack->size;
	return ret;
}


int rnd_stack_quickremove(struct rnd_stack *stack, size_t idx, void *output)
{
	char *p, *q;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	q = (char*)stack->data + (stack->size - 1) * stack->elem_size;
	if (output != NULL)
		memcpy(output, p, stack->elem_size);
	memcpy(p, q, stack->elem_size);
	--stack->size;
	return 0;
}

char rnd_stack_quickremovec(struct rnd_stack *stack, size_t idx)
{
	char *p, *q;
	char ret;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return 0;
	}
	if (stack->elem_size != sizeof(char)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(char)));
		return 0;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	q = (char*)stack->data + (stack->size - 1) * stack->elem_size;
	ret = *(char*)p;
	*(char*)p = *(char*)q;
	--stack->size;
	return ret;
}

short rnd_stack_quickremoves(struct rnd_stack *stack, size_t idx)
{
	char *p, *q;
	short ret;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return 0;
	}
	if (stack->elem_size != sizeof(short)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(short)));
		return 0;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	q = (char*)stack->data + (stack->size - 1) * stack->elem_size;
	ret = *(short*)p;
	*(short*)p = *(short*)q;
	--stack->size;
	return ret;
}

int rnd_stack_quickremovei(struct rnd_stack *stack, size_t idx)
{
	char *p, *q;
	int ret;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return 0;
	}
	if (stack->elem_size != sizeof(int)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(int)));
		return 0;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	q = (char*)stack->data + (stack->size - 1) * stack->elem_size;
	ret = *(int*)p;
	*(int*)p = *(int*)q;
	--stack->size;
	return ret;
}

long rnd_stack_quickremovel(struct rnd_stack *stack, size_t idx)
{
	char *p, *q;
	long ret;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return 0;
	}
	if (stack->elem_size != sizeof(long)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(long)));
		return 0;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	q = (char*)stack->data + (stack->size - 1) * stack->elem_size;
	ret = *(long*)p;
	*(long*)p = *(long*)q;
	--stack->size;
	return ret;
}

signed char rnd_stack_quickremovesc(struct rnd_stack *stack, size_t idx)
{
	char *p, *q;
	signed char ret;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return 0;
	}
	if (stack->elem_size != sizeof(signed char)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(signed char)));
		return 0;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	q = (char*)stack->data + (stack->size - 1) * stack->elem_size;
	ret = *(signed char*)p;
	*(signed char*)p = *(signed char*)q;
	--stack->size;
	return ret;
}

unsigned char rnd_stack_quickremoveuc(struct rnd_stack *stack, size_t idx)
{
	char *p, *q;
	unsigned char ret;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return 0;
	}
	if (stack->elem_size != sizeof(unsigned char)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(unsigned char)));
		return 0;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	q = (char*)stack->data + (stack->size - 1) * stack->elem_size;
	ret = *(unsigned char*)p;
	*(unsigned char*)p = *(unsigned char*)q;
	--stack->size;
	return ret;
}

unsigned short rnd_stack_quickremoveus(struct rnd_stack *stack, size_t idx)
{
	char *p, *q;
	unsigned short ret;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return 0;
	}
	if (stack->elem_size != sizeof(unsigned short)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(unsigned short)));
		return 0;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	q = (char*)stack->data + (stack->size - 1) * stack->elem_size;
	ret = *(unsigned short*)p;
	*(unsigned short*)p = *(unsigned short*)q;
	--stack->size;
	return ret;
}

unsigned int rnd_stack_quickremoveui(struct rnd_stack *stack, size_t idx)
{
	char *p, *q;
	unsigned int ret;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return 0;
	}
	if (stack->elem_size != sizeof(unsigned int)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(unsigned int)));
		return 0;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	q = (char*)stack->data + (stack->size - 1) * stack->elem_size;
	ret = *(unsigned int*)p;
	*(unsigned int*)p = *(unsigned int*)q;
	--stack->size;
	return ret;
}

unsigned long rnd_stack_quickremoveul(struct rnd_stack *stack, size_t idx)
{
	char *p, *q;
	unsigned long ret;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return 0;
	}
	if (stack->elem_size != sizeof(unsigned long)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(unsigned long)));
		return 0;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	q = (char*)stack->data + (stack->size - 1) * stack->elem_size;
	ret = *(unsigned long*)p;
	*(unsigned long*)p = *(unsigned long*)q;
	--stack->size;
	return ret;
}

float rnd_stack_quickremovef(struct rnd_stack *stack, size_t idx)
{
	char *p, *q;
	float ret;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return 0;
	}
	if (stack->elem_size != sizeof(float)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(float)));
		return 0;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	q = (char*)stack->data + (stack->size - 1) * stack->elem_size;
	ret = *(float*)p;
	*(float*)p = *(float*)q;
	--stack->size;
	return ret;
}

double rnd_stack_quickremoved(struct rnd_stack *stack, size_t idx)
{
	char *p, *q;
	double ret;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return 0;
	}
	if (stack->elem_size != sizeof(double)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(double)));
		return 0;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	q = (char*)stack->data + (stack->size - 1) * stack->elem_size;
	ret = *(double*)p;
	*(double*)p = *(double*)q;
	--stack->size;
	return ret;
}

long double rnd_stack_quickremoveld(struct rnd_stack *stack, size_t idx)
{
	char *p, *q;
	long double ret;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return 0;
	}
	if (stack->elem_size != sizeof(long double)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(long double)));
		return 0;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	q = (char*)stack->data + (stack->size - 1) * stack->elem_size;
	ret = *(long double*)p;
	*(long double*)p = *(long double*)q;
	--stack->size;
	return ret;
}


int rnd_stack_get(const struct rnd_stack *stack, size_t idx, void *output)
{
	const void *src;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (output == NULL) {
		error(("output is NULL"));
		return RND_EINVAL;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	src = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	memcpy(output, src, stack->elem_size);
	return 0;
}

char rnd_stack_getc(const struct rnd_stack *stack, size_t idx)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return 0;
	}
	if (stack->elem_size != sizeof(char)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(char)));
		return 0;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	return ((char*)stack->data)[stack->size - 1 - idx];
}

short rnd_stack_gets(const struct rnd_stack *stack, size_t idx)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return 0;
	}
	if (stack->elem_size != sizeof(short)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(short)));
		return 0;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	return ((short*)stack->data)[stack->size - 1 - idx];
}

int rnd_stack_geti(const struct rnd_stack *stack, size_t idx)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return 0;
	}
	if (stack->elem_size != sizeof(int)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(int)));
		return 0;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	return ((int*)stack->data)[stack->size - 1 - idx];
}

long rnd_stack_getl(const struct rnd_stack *stack, size_t idx)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return 0;
	}
	if (stack->elem_size != sizeof(long)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(long)));
		return 0;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	return ((long*)stack->data)[stack->size - 1 - idx];
}

signed char rnd_stack_getsc(const struct rnd_stack *stack, size_t idx)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return 0;
	}
	if (stack->elem_size != sizeof(signed char)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(signed char)));
		return 0;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	return ((signed char*)stack->data)[stack->size - 1 - idx];
}

unsigned char rnd_stack_getuc(const struct rnd_stack *stack, size_t idx)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return 0;
	}
	if (stack->elem_size != sizeof(unsigned char)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(unsigned char)));
		return 0;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	return ((unsigned char*)stack->data)[stack->size - 1 - idx];
}

unsigned short rnd_stack_getus(const struct rnd_stack *stack, size_t idx)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return 0;
	}
	if (stack->elem_size != sizeof(unsigned short)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(unsigned short)));
		return 0;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	return ((unsigned short*)stack->data)[stack->size - 1 - idx];
}

unsigned int rnd_stack_getui(const struct rnd_stack *stack, size_t idx)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return 0;
	}
	if (stack->elem_size != sizeof(unsigned int)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(unsigned int)));
		return 0;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	return ((unsigned int*)stack->data)[stack->size - 1 - idx];
}

unsigned long rnd_stack_getul(const struct rnd_stack *stack, size_t idx)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return 0;
	}
	if (stack->elem_size != sizeof(unsigned long)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(unsigned long)));
		return 0;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	return ((unsigned long*)stack->data)[stack->size - 1 - idx];
}

float rnd_stack_getf(const struct rnd_stack *stack, size_t idx)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return 0;
	}
	if (stack->elem_size != sizeof(float)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(float)));
		return 0;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	return ((float*)stack->data)[stack->size - 1 - idx];
}

double rnd_stack_getd(const struct rnd_stack *stack, size_t idx)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return 0;
	}
	if (stack->elem_size != sizeof(double)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(double)));
		return 0;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	return ((double*)stack->data)[stack->size - 1 - idx];
}

long double rnd_stack_getld(const struct rnd_stack *stack, size_t idx)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return 0;
	}
	if (stack->elem_size != sizeof(long double)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(long double)));
		return 0;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	return ((long double*)stack->data)[stack->size - 1 - idx];
}


int rnd_stack_set(struct rnd_stack *stack, size_t idx, void *val)
{
	char *p;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (val == NULL) {
		error(("val is NULL"));
		return RND_EINVAL;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	memcpy(p, val, stack->elem_size);
	return 0;
}

int rnd_stack_setc(struct rnd_stack *stack, size_t idx, char val)
{
	char *p;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (stack->elem_size != sizeof(val)) {
		error(("stack->elem_size is incompatible with val type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(val)));
		return RND_EILLEGAL;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	*p = val;
	return 0;
}

int rnd_stack_sets(struct rnd_stack *stack, size_t idx, short val)
{
	char *p;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (stack->elem_size != sizeof(val)) {
		error(("stack->elem_size is incompatible with val type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(val)));
		return RND_EILLEGAL;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	*(short*)p = val;
	return 0;
}

int rnd_stack_seti(struct rnd_stack *stack, size_t idx, int val)
{
	char *p;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (stack->elem_size != sizeof(val)) {
		error(("stack->elem_size is incompatible with val type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(val)));
		return RND_EILLEGAL;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	*(int*)p = val;
	return 0;
}

int rnd_stack_setl(struct rnd_stack *stack, size_t idx, long val)
{
	char *p;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (stack->elem_size != sizeof(val)) {
		error(("stack->elem_size is incompatible with val type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(val)));
		return RND_EILLEGAL;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	*(long*)p = val;
	return 0;
}

int rnd_stack_setsc(struct rnd_stack *stack, size_t idx, signed char val)
{
	char *p;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (stack->elem_size != sizeof(val)) {
		error(("stack->elem_size is incompatible with val type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(val)));
		return RND_EILLEGAL;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	*(signed char*)p = val;
	return 0;
}

int rnd_stack_setuc(struct rnd_stack *stack, size_t idx, unsigned char val)
{
	char *p;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (stack->elem_size != sizeof(val)) {
		error(("stack->elem_size is incompatible with val type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(val)));
		return RND_EILLEGAL;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	*(unsigned char*)p = val;
	return 0;
}

int rnd_stack_setus(struct rnd_stack *stack, size_t idx, unsigned short val)
{
	char *p;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (stack->elem_size != sizeof(val)) {
		error(("stack->elem_size is incompatible with val type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(val)));
		return RND_EILLEGAL;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	*(unsigned short*)p = val;
	return 0;
}

int rnd_stack_setui(struct rnd_stack *stack, size_t idx, unsigned int val)
{
	char *p;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (stack->elem_size != sizeof(val)) {
		error(("stack->elem_size is incompatible with val type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(val)));
		return RND_EILLEGAL;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	*(unsigned int*)p = val;
	return 0;
}

int rnd_stack_setul(struct rnd_stack *stack, size_t idx, unsigned long val)
{
	char *p;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (stack->elem_size != sizeof(val)) {
		error(("stack->elem_size is incompatible with val type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(val)));
		return RND_EILLEGAL;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	*(unsigned long*)p = val;
	return 0;
}

int rnd_stack_setf(struct rnd_stack *stack, size_t idx, float val)
{
	char *p;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (stack->elem_size != sizeof(val)) {
		error(("stack->elem_size is incompatible with val type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(val)));
		return RND_EILLEGAL;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	*(float*)p = val;
	return 0;
}

int rnd_stack_setd(struct rnd_stack *stack, size_t idx, double val)
{
	char *p;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (stack->elem_size != sizeof(val)) {
		error(("stack->elem_size is incompatible with val type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(val)));
		return RND_EILLEGAL;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	*(double*)p = val;
	return 0;
}

int rnd_stack_setld(struct rnd_stack *stack, size_t idx, long double val)
{
	char *p;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (stack->elem_size != sizeof(val)) {
		error(("stack->elem_size is incompatible with val type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(val)));
		return RND_EILLEGAL;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return RND_EINDEX;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	*(long double*)p = val;
	return 0;
}


int rnd_stack_print(struct rnd_stack *stack)
{
	size_t i;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
#endif
	printf("rnd_stack_print()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)stack->size, (unsigned long)stack->capacity, (unsigned long)stack->elem_size);
	for (i = stack->size; i-- > 0;) {
		const void *const elem = (char*)stack->data + i * stack->elem_size;
		printf("[%lu]\t%p\n", (unsigned long)stack->size - 1 - i, elem);
	}
	return 0;
}

int rnd_stack_printc(struct rnd_stack *stack)
{
	size_t i;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (stack->elem_size != sizeof(char)) {
		error(("stack->elem_size is incompatible with function type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(char)));
		return RND_EILLEGAL;
	}
#endif
	printf("rnd_stack_printc()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)stack->size, (unsigned long)stack->capacity, (unsigned long)stack->elem_size);
	for (i = stack->size; i-- > 0;) {
		const char elem = ((char*)stack->data)[i];
		printf("[%lu]\t%hd\t'%c'\n", (unsigned long)stack->size - 1 - i, elem, elem);
	}
	return 0;
}

int rnd_stack_prints(struct rnd_stack *stack)
{
	size_t i;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (stack->elem_size != sizeof(short)) {
		error(("stack->elem_size is incompatible with function type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(short)));
		return RND_EILLEGAL;
	}
#endif
	printf("rnd_stack_prints()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)stack->size, (unsigned long)stack->capacity, (unsigned long)stack->elem_size);
	for (i = stack->size; i-- > 0;) {
		const short elem = ((short*)stack->data)[i];
		printf("[%lu]\t%hd\n", (unsigned long)stack->size - 1 - i, elem);
	}
	return 0;
}

int rnd_stack_printi(struct rnd_stack *stack)
{
	size_t i;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (stack->elem_size != sizeof(int)) {
		error(("stack->elem_size is incompatible with function type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(int)));
		return RND_EILLEGAL;
	}
#endif
	printf("rnd_stack_printi()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)stack->size, (unsigned long)stack->capacity, (unsigned long)stack->elem_size);
	for (i = stack->size; i-- > 0;) {
		const int elem = ((int*)stack->data)[i];
		printf("[%lu]\t%d\n", (unsigned long)stack->size - 1 - i, elem);
	}
	return 0;
}

int rnd_stack_printl(struct rnd_stack *stack)
{
	size_t i;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (stack->elem_size != sizeof(long)) {
		error(("stack->elem_size is incompatible with function type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(long)));
		return RND_EILLEGAL;
	}
#endif
	printf("rnd_stack_printl()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)stack->size, (unsigned long)stack->capacity, (unsigned long)stack->elem_size);
	for (i = stack->size; i-- > 0;) {
		const long elem = ((long*)stack->data)[i];
		printf("[%lu]\t%ld\n", (unsigned long)stack->size - 1 - i, elem);
	}
	return 0;
}

int rnd_stack_printsc(struct rnd_stack *stack)
{
	size_t i;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (stack->elem_size != sizeof(signed char)) {
		error(("stack->elem_size is incompatible with function type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(signed char)));
		return RND_EILLEGAL;
	}
#endif
	printf("rnd_stack_printsc()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)stack->size, (unsigned long)stack->capacity, (unsigned long)stack->elem_size);
	for (i = stack->size; i-- > 0;) {
		const signed char elem = ((signed char*)stack->data)[i];
		printf("[%lu]\t%hd\t'%c'\n", (unsigned long)stack->size - 1 - i, elem, elem);
	}
	return 0;
}

int rnd_stack_printuc(struct rnd_stack *stack)
{
	size_t i;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (stack->elem_size != sizeof(unsigned char)) {
		error(("stack->elem_size is incompatible with function type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(unsigned char)));
		return RND_EILLEGAL;
	}
#endif
	printf("rnd_stack_printuc()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)stack->size, (unsigned long)stack->capacity, (unsigned long)stack->elem_size);
	for (i = stack->size; i-- > 0;) {
		const unsigned char elem = ((unsigned char*)stack->data)[i];
		printf("[%lu]\t%hd\t'%c'\n", (unsigned long)stack->size - 1 - i, elem, elem);
	}
	return 0;
}

int rnd_stack_printus(struct rnd_stack *stack)
{
	size_t i;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (stack->elem_size != sizeof(unsigned short)) {
		error(("stack->elem_size is incompatible with function type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(unsigned short)));
		return RND_EILLEGAL;
	}
#endif
	printf("rnd_stack_printus()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)stack->size, (unsigned long)stack->capacity, (unsigned long)stack->elem_size);
	for (i = stack->size; i-- > 0;) {
		const unsigned short elem = ((unsigned short*)stack->data)[i];
		printf("[%lu]\t%hu\n", (unsigned long)stack->size - 1 - i, elem);
	}
	return 0;
}

int rnd_stack_printui(struct rnd_stack *stack)
{
	size_t i;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (stack->elem_size != sizeof(unsigned int)) {
		error(("stack->elem_size is incompatible with function type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(unsigned int)));
		return RND_EILLEGAL;
	}
#endif
	printf("rnd_stack_printui()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)stack->size, (unsigned long)stack->capacity, (unsigned long)stack->elem_size);
	for (i = stack->size; i-- > 0;) {
		const unsigned int elem = ((unsigned int*)stack->data)[i];
		printf("[%lu]\t%u\n", (unsigned long)stack->size - 1 - i, elem);
	}
	return 0;
}

int rnd_stack_printul(struct rnd_stack *stack)
{
	size_t i;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (stack->elem_size != sizeof(unsigned long)) {
		error(("stack->elem_size is incompatible with function type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(unsigned long)));
		return RND_EILLEGAL;
	}
#endif
	printf("rnd_stack_printul()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)stack->size, (unsigned long)stack->capacity, (unsigned long)stack->elem_size);
	for (i = stack->size; i-- > 0;) {
		const unsigned long elem = ((unsigned long*)stack->data)[i];
		printf("[%lu]\t%lu\n", (unsigned long)stack->size - 1 - i, elem);
	}
	return 0;
}

int rnd_stack_printf(struct rnd_stack *stack)
{
	size_t i;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (stack->elem_size != sizeof(float)) {
		error(("stack->elem_size is incompatible with function type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(float)));
		return RND_EILLEGAL;
	}
#endif
	printf("rnd_stack_printf()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)stack->size, (unsigned long)stack->capacity, (unsigned long)stack->elem_size);
	for (i = stack->size; i-- > 0;) {
		const float elem = ((float*)stack->data)[i];
		printf("[%lu]\t%g\n", (unsigned long)stack->size - 1 - i, elem);
	}
	return 0;
}

int rnd_stack_printd(struct rnd_stack *stack)
{
	size_t i;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (stack->elem_size != sizeof(double)) {
		error(("stack->elem_size is incompatible with function type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(double)));
		return RND_EILLEGAL;
	}
#endif
	printf("rnd_stack_printd()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)stack->size, (unsigned long)stack->capacity, (unsigned long)stack->elem_size);
	for (i = stack->size; i-- > 0;) {
		const double elem = ((double*)stack->data)[i];
		printf("[%lu]\t%g\n", (unsigned long)stack->size - 1 - i, elem);
	}
	return 0;
}

int rnd_stack_printld(struct rnd_stack *stack)
{
	size_t i;
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
	if (stack->elem_size != sizeof(long double)) {
		error(("stack->elem_size is incompatible with function type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(long double)));
		return RND_EILLEGAL;
	}
#endif
	printf("rnd_stack_printld()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)stack->size, (unsigned long)stack->capacity, (unsigned long)stack->elem_size);
	for (i = stack->size; i-- > 0;) {
		const long double elem = ((long double*)stack->data)[i];
		printf("[%lu]\t%Lg\n", (unsigned long)stack->size - 1 - i, elem);
	}
	return 0;
}
