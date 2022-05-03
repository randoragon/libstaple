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
#include "sp_stack.h"
#include "helpers.h"
#include "sp_errcodes.h"
#include <string.h>

struct sp_stack *sp_stack_create(size_t elem_size, size_t capacity)
{
	struct sp_stack *ret;

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

	return ret;
}

int sp_stack_clear(struct sp_stack *stack, int (*dtor)(void*))
{
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
#endif
	if (dtor != NULL) {
		const void *const end = (char*)stack->data + stack->size * stack->elem_size;
		char *p = stack->data;
		while (p != end) {
			int err;
			if ((err = dtor(p))) {
				error(("external function handler returned %d (non-0)", err));
				return SP_EHANDLER;
			}
			p += stack->elem_size;
		}
	}
	stack->size = 0;
	return 0;
}

int sp_stack_destroy(struct sp_stack *stack, int (*dtor)(void*))
{
	int error;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
#endif
	if ((error = sp_stack_clear(stack, dtor)))
		return SP_EHANDLER;
	free(stack->data);
	free(stack);
	return 0;
}

int sp_stack_copy(struct sp_stack *dest, const struct sp_stack *src, int (*cpy)(void*, const void*))
{
	char *s, *d;
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
				return SP_EHANDLER;
			}
			s += src->elem_size;
			d += dest->elem_size;
		}
	}
	return 0;
}

int sp_stack_foreach(struct sp_stack *stack, int (*func)(void*, size_t))
{
	size_t i;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (func == NULL) {
		error(("func is NULL"));
		return SP_EINVAL;
	}
#endif
	for (i = 0; i < stack->size; i++) {
		void *const p = (char*)stack->data + i * stack->elem_size;
		int err;
		if ((err = func(p, i))) {
			warn(("external func function returned %d (non-0)", err));
			return SP_EHANDLER;
		}
	}
	return 0;
}

int sp_stack_push(struct sp_stack *stack, const void *elem)
{
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (elem == NULL) {
		error(("elem is NULL"));
		return SP_EINVAL;
	}
#endif
	if (sp_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return SP_ERANGE;
	if (sp_buf_fit(&stack->data, stack->size, &stack->capacity, stack->elem_size))
		return SP_ENOMEM;
	memcpy((char*)stack->data + stack->size++ * stack->elem_size, elem, stack->elem_size);
	return 0;
}

int sp_stack_pushc(struct sp_stack *stack, char elem)
{
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
#endif
	if (sp_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return SP_ERANGE;
	if (sp_buf_fit(&stack->data, stack->size, &stack->capacity, stack->elem_size))
		return SP_ENOMEM;
	((char*)stack->data)[stack->size++] = elem;
	return 0;
}

int sp_stack_pushs(struct sp_stack *stack, short elem)
{
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
#endif
	if (sp_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return SP_ERANGE;
	if (sp_buf_fit(&stack->data, stack->size, &stack->capacity, stack->elem_size))
		return SP_ENOMEM;
	((short*)stack->data)[stack->size++] = elem;
	return 0;
}

int sp_stack_pushi(struct sp_stack *stack, int elem)
{
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
#endif
	if (sp_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return SP_ERANGE;
	if (sp_buf_fit(&stack->data, stack->size, &stack->capacity, stack->elem_size))
		return SP_ENOMEM;
	((int*)stack->data)[stack->size++] = elem;
	return 0;
}

int sp_stack_pushl(struct sp_stack *stack, long elem)
{
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
#endif
	if (sp_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return SP_ERANGE;
	if (sp_buf_fit(&stack->data, stack->size, &stack->capacity, stack->elem_size))
		return SP_ENOMEM;
	((long*)stack->data)[stack->size++] = elem;
	return 0;
}

int sp_stack_pushsc(struct sp_stack *stack, signed char elem)
{
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
#endif
	if (sp_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return SP_ERANGE;
	if (sp_buf_fit(&stack->data, stack->size, &stack->capacity, stack->elem_size))
		return SP_ENOMEM;
	((signed char*)stack->data)[stack->size++] = elem;
	return 0;
}

int sp_stack_pushuc(struct sp_stack *stack, unsigned char elem)
{
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
#endif
	if (sp_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return SP_ERANGE;
	if (sp_buf_fit(&stack->data, stack->size, &stack->capacity, stack->elem_size))
		return SP_ENOMEM;
	((unsigned char*)stack->data)[stack->size++] = elem;
	return 0;
}

int sp_stack_pushus(struct sp_stack *stack, unsigned short elem)
{
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
#endif
	if (sp_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return SP_ERANGE;
	if (sp_buf_fit(&stack->data, stack->size, &stack->capacity, stack->elem_size))
		return SP_ENOMEM;
	((unsigned short*)stack->data)[stack->size++] = elem;
	return 0;
}

int sp_stack_pushui(struct sp_stack *stack, unsigned int elem)
{
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
#endif
	if (sp_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return SP_ERANGE;
	if (sp_buf_fit(&stack->data, stack->size, &stack->capacity, stack->elem_size))
		return SP_ENOMEM;
	((unsigned int*)stack->data)[stack->size++] = elem;
	return 0;
}

int sp_stack_pushul(struct sp_stack *stack, unsigned long elem)
{
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
#endif
	if (sp_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return SP_ERANGE;
	if (sp_buf_fit(&stack->data, stack->size, &stack->capacity, stack->elem_size))
		return SP_ENOMEM;
	((unsigned long*)stack->data)[stack->size++] = elem;
	return 0;
}

int sp_stack_pushf(struct sp_stack *stack, float elem)
{
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
#endif
	if (sp_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return SP_ERANGE;
	if (sp_buf_fit(&stack->data, stack->size, &stack->capacity, stack->elem_size))
		return SP_ENOMEM;
	((float*)stack->data)[stack->size++] = elem;
	return 0;
}

int sp_stack_pushd(struct sp_stack *stack, double elem)
{
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
#endif
	if (sp_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return SP_ERANGE;
	if (sp_buf_fit(&stack->data, stack->size, &stack->capacity, stack->elem_size))
		return SP_ENOMEM;
	((double*)stack->data)[stack->size++] = elem;
	return 0;
}

int sp_stack_pushld(struct sp_stack *stack, long double elem)
{
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
#endif
	if (sp_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return SP_ERANGE;
	if (sp_buf_fit(&stack->data, stack->size, &stack->capacity, stack->elem_size))
		return SP_ENOMEM;
	((long double*)stack->data)[stack->size++] = elem;
	return 0;
}

int sp_stack_pushstr(struct sp_stack *stack, const char *elem)
{
	char *buf;
	size_t len;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (elem == NULL) {
		error(("elem is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
#endif
	if (sp_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return SP_ERANGE;
	if (sp_buf_fit(&stack->data, stack->size, &stack->capacity, stack->elem_size))
		return SP_ENOMEM;
	len = sp_strnlen(elem, SIZE_MAX);
	if (sp_size_try_add(len, 1))
		return SP_ERANGE;
	buf = malloc((len + 1) * sizeof(*elem));
	if (buf == NULL) {
		error(("malloc"));
		return SP_ENOMEM;
	}
	memcpy(buf, elem, len * sizeof(*elem));
	buf[len] = '\0';
	((char**)stack->data)[stack->size++] = buf;
	return 0;
}

int sp_stack_pushstrn(struct sp_stack *stack, const char *elem, size_t len)
{
	char *buf;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (elem == NULL) {
		error(("elem is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
#endif
	if (sp_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return SP_ERANGE;
	if (sp_buf_fit(&stack->data, stack->size, &stack->capacity, stack->elem_size))
		return SP_ENOMEM;
	if (sp_size_try_add(len, 1))
		return SP_ERANGE;
	buf = malloc((len + 1) * sizeof(*elem));
	if (buf == NULL) {
		error(("malloc"));
		return SP_ENOMEM;
	}
	memcpy(buf, elem, len * sizeof(*elem));
	buf[len] = '\0';
	((char**)stack->data)[stack->size++] = buf;
	return 0;
}

int sp_stack_insert(struct sp_stack *stack, size_t idx, const void *elem)
{
	char *p;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (elem == NULL) {
		error(("elem is NULL"));
		return SP_EINVAL;
	}
	if (idx > stack->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return SP_ERANGE;
	if (sp_buf_fit(&stack->data, stack->size, &stack->capacity, stack->elem_size))
		return SP_ENOMEM;
	p = (char*)stack->data + (stack->size - idx) * stack->elem_size;
	memmove(p + stack->elem_size, p, idx * stack->elem_size);
	memcpy(p, elem, stack->elem_size);
	++stack->size;
	return 0;
}

int sp_stack_insertc(struct sp_stack *stack, size_t idx, char elem)
{
	char *p;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
	if (idx > stack->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return SP_ERANGE;
	if (sp_buf_fit(&stack->data, stack->size, &stack->capacity, stack->elem_size))
		return SP_ENOMEM;
	p = (char*)stack->data + (stack->size - idx) * stack->elem_size;
	memmove(p + stack->elem_size, p, idx * stack->elem_size);
	*(char*)p = elem;
	++stack->size;
	return 0;
}

int sp_stack_inserts(struct sp_stack *stack, size_t idx, short elem)
{
	char *p;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
	if (idx > stack->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return SP_ERANGE;
	if (sp_buf_fit(&stack->data, stack->size, &stack->capacity, stack->elem_size))
		return SP_ENOMEM;
	p = (char*)stack->data + (stack->size - idx) * stack->elem_size;
	memmove(p + stack->elem_size, p, idx * stack->elem_size);
	*(short*)p = elem;
	++stack->size;
	return 0;
}

int sp_stack_inserti(struct sp_stack *stack, size_t idx, int elem)
{
	char *p;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
	if (idx > stack->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return SP_ERANGE;
	if (sp_buf_fit(&stack->data, stack->size, &stack->capacity, stack->elem_size))
		return SP_ENOMEM;
	p = (char*)stack->data + (stack->size - idx) * stack->elem_size;
	memmove(p + stack->elem_size, p, idx * stack->elem_size);
	*(int*)p = elem;
	++stack->size;
	return 0;
}

int sp_stack_insertl(struct sp_stack *stack, size_t idx, long elem)
{
	char *p;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
	if (idx > stack->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return SP_ERANGE;
	if (sp_buf_fit(&stack->data, stack->size, &stack->capacity, stack->elem_size))
		return SP_ENOMEM;
	p = (char*)stack->data + (stack->size - idx) * stack->elem_size;
	memmove(p + stack->elem_size, p, idx * stack->elem_size);
	*(long*)p = elem;
	++stack->size;
	return 0;
}

int sp_stack_insertsc(struct sp_stack *stack, size_t idx, signed char elem)
{
	char *p;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
	if (idx > stack->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return SP_ERANGE;
	if (sp_buf_fit(&stack->data, stack->size, &stack->capacity, stack->elem_size))
		return SP_ENOMEM;
	p = (char*)stack->data + (stack->size - idx) * stack->elem_size;
	memmove(p + stack->elem_size, p, idx * stack->elem_size);
	*(signed char*)p = elem;
	++stack->size;
	return 0;
}

int sp_stack_insertuc(struct sp_stack *stack, size_t idx, unsigned char elem)
{
	char *p;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
	if (idx > stack->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return SP_ERANGE;
	if (sp_buf_fit(&stack->data, stack->size, &stack->capacity, stack->elem_size))
		return SP_ENOMEM;
	p = (char*)stack->data + (stack->size - idx) * stack->elem_size;
	memmove(p + stack->elem_size, p, idx * stack->elem_size);
	*(unsigned char*)p = elem;
	++stack->size;
	return 0;
}

int sp_stack_insertus(struct sp_stack *stack, size_t idx, unsigned short elem)
{
	char *p;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
	if (idx > stack->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return SP_ERANGE;
	if (sp_buf_fit(&stack->data, stack->size, &stack->capacity, stack->elem_size))
		return SP_ENOMEM;
	p = (char*)stack->data + (stack->size - idx) * stack->elem_size;
	memmove(p + stack->elem_size, p, idx * stack->elem_size);
	*(unsigned short*)p = elem;
	++stack->size;
	return 0;
}

int sp_stack_insertui(struct sp_stack *stack, size_t idx, unsigned int elem)
{
	char *p;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
	if (idx > stack->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return SP_ERANGE;
	if (sp_buf_fit(&stack->data, stack->size, &stack->capacity, stack->elem_size))
		return SP_ENOMEM;
	p = (char*)stack->data + (stack->size - idx) * stack->elem_size;
	memmove(p + stack->elem_size, p, idx * stack->elem_size);
	*(unsigned int*)p = elem;
	++stack->size;
	return 0;
}

int sp_stack_insertul(struct sp_stack *stack, size_t idx, unsigned long elem)
{
	char *p;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
	if (idx > stack->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return SP_ERANGE;
	if (sp_buf_fit(&stack->data, stack->size, &stack->capacity, stack->elem_size))
		return SP_ENOMEM;
	p = (char*)stack->data + (stack->size - idx) * stack->elem_size;
	memmove(p + stack->elem_size, p, idx * stack->elem_size);
	*(unsigned long*)p = elem;
	++stack->size;
	return 0;
}

int sp_stack_insertf(struct sp_stack *stack, size_t idx, float elem)
{
	char *p;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
	if (idx > stack->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return SP_ERANGE;
	if (sp_buf_fit(&stack->data, stack->size, &stack->capacity, stack->elem_size))
		return SP_ENOMEM;
	p = (char*)stack->data + (stack->size - idx) * stack->elem_size;
	memmove(p + stack->elem_size, p, idx * stack->elem_size);
	*(float*)p = elem;
	++stack->size;
	return 0;
}

int sp_stack_insertd(struct sp_stack *stack, size_t idx, double elem)
{
	char *p;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
	if (idx > stack->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return SP_ERANGE;
	if (sp_buf_fit(&stack->data, stack->size, &stack->capacity, stack->elem_size))
		return SP_ENOMEM;
	p = (char*)stack->data + (stack->size - idx) * stack->elem_size;
	memmove(p + stack->elem_size, p, idx * stack->elem_size);
	*(double*)p = elem;
	++stack->size;
	return 0;
}

int sp_stack_insertld(struct sp_stack *stack, size_t idx, long double elem)
{
	char *p;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
	if (idx > stack->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return SP_ERANGE;
	if (sp_buf_fit(&stack->data, stack->size, &stack->capacity, stack->elem_size))
		return SP_ENOMEM;
	p = (char*)stack->data + (stack->size - idx) * stack->elem_size;
	memmove(p + stack->elem_size, p, idx * stack->elem_size);
	*(long double*)p = elem;
	++stack->size;
	return 0;
}

int sp_stack_insertstr(struct sp_stack *stack, size_t idx, const char *elem)
{
	char *p;
	char *buf;
	size_t len;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (elem == NULL) {
		error(("elem is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
	if (idx > stack->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return SP_ERANGE;
	if (sp_buf_fit(&stack->data, stack->size, &stack->capacity, stack->elem_size))
		return SP_ENOMEM;
	len = sp_strnlen(elem, SIZE_MAX);
	if (sp_size_try_add(len, 1))
		return SP_ERANGE;
	buf = malloc((len + 1) * sizeof(*elem));
	if (buf == NULL) {
		error(("malloc"));
		return SP_ENOMEM;
	}
	memcpy(buf, elem, len * sizeof(*elem));
	buf[len] = '\0';
	p = (char*)stack->data + (stack->size - idx) * stack->elem_size;
	memmove(p + stack->elem_size, p, idx * stack->elem_size);
	*(char**)p = buf;
	++stack->size;
	return 0;
}

int sp_stack_insertstrn(struct sp_stack *stack, size_t idx, const char *elem, size_t len)
{
	char *p;
	char *buf;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (elem == NULL) {
		error(("elem is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
	if (idx > stack->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return SP_ERANGE;
	if (sp_buf_fit(&stack->data, stack->size, &stack->capacity, stack->elem_size))
		return SP_ENOMEM;
	if (sp_size_try_add(len, 1))
		return SP_ERANGE;
	buf = malloc((len + 1) * sizeof(*elem));
	if (buf == NULL) {
		error(("malloc"));
		return SP_ENOMEM;
	}
	memcpy(buf, elem, len * sizeof(*elem));
	buf[len] = '\0';
	p = (char*)stack->data + (stack->size - idx) * stack->elem_size;
	memmove(p + stack->elem_size, p, idx * stack->elem_size);
	*(char**)p = buf;
	++stack->size;
	return 0;
}


int sp_stack_qinsert(struct sp_stack *stack, size_t idx, const void *elem)
{
	char *p, *q;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (elem == NULL) {
		error(("elem is NULL"));
		return SP_EINVAL;
	}
	if (idx > stack->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return SP_ERANGE;
	if (sp_buf_fit(&stack->data, stack->size, &stack->capacity, stack->elem_size))
		return SP_ENOMEM;
	p = (char*)stack->data + (stack->size - idx) * stack->elem_size;
	q = (char*)stack->data + stack->size * stack->elem_size;
	memcpy(q, p, stack->elem_size);
	memcpy(p, elem, stack->elem_size);
	++stack->size;
	return 0;
}

int sp_stack_qinsertc(struct sp_stack *stack, size_t idx, char elem)
{
	char *p, *q;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
	if (idx > stack->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return SP_ERANGE;
	if (sp_buf_fit(&stack->data, stack->size, &stack->capacity, stack->elem_size))
		return SP_ENOMEM;
	p = (char*)stack->data + (stack->size - idx) * stack->elem_size;
	q = (char*)stack->data + stack->size * stack->elem_size;
	*(char*)q = *(char*)p;
	*(char*)p = elem;
	++stack->size;
	return 0;
}

int sp_stack_qinserts(struct sp_stack *stack, size_t idx, short elem)
{
	char *p, *q;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
	if (idx > stack->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return SP_ERANGE;
	if (sp_buf_fit(&stack->data, stack->size, &stack->capacity, stack->elem_size))
		return SP_ENOMEM;
	p = (char*)stack->data + (stack->size - idx) * stack->elem_size;
	q = (char*)stack->data + stack->size * stack->elem_size;
	*(short*)q = *(short*)p;
	*(short*)p = elem;
	++stack->size;
	return 0;
}

int sp_stack_qinserti(struct sp_stack *stack, size_t idx, int elem)
{
	char *p, *q;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
	if (idx > stack->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return SP_ERANGE;
	if (sp_buf_fit(&stack->data, stack->size, &stack->capacity, stack->elem_size))
		return SP_ENOMEM;
	p = (char*)stack->data + (stack->size - idx) * stack->elem_size;
	q = (char*)stack->data + stack->size * stack->elem_size;
	*(int*)q = *(int*)p;
	*(int*)p = elem;
	++stack->size;
	return 0;
}

int sp_stack_qinsertl(struct sp_stack *stack, size_t idx, long elem)
{
	char *p, *q;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
	if (idx > stack->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return SP_ERANGE;
	if (sp_buf_fit(&stack->data, stack->size, &stack->capacity, stack->elem_size))
		return SP_ENOMEM;
	p = (char*)stack->data + (stack->size - idx) * stack->elem_size;
	q = (char*)stack->data + stack->size * stack->elem_size;
	*(long*)q = *(long*)p;
	*(long*)p = elem;
	++stack->size;
	return 0;
}

int sp_stack_qinsertsc(struct sp_stack *stack, size_t idx, signed char elem)
{
	char *p, *q;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
	if (idx > stack->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return SP_ERANGE;
	if (sp_buf_fit(&stack->data, stack->size, &stack->capacity, stack->elem_size))
		return SP_ENOMEM;
	p = (char*)stack->data + (stack->size - idx) * stack->elem_size;
	q = (char*)stack->data + stack->size * stack->elem_size;
	*(signed char*)q = *(signed char*)p;
	*(signed char*)p = elem;
	++stack->size;
	return 0;
}

int sp_stack_qinsertuc(struct sp_stack *stack, size_t idx, unsigned char elem)
{
	char *p, *q;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
	if (idx > stack->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return SP_ERANGE;
	if (sp_buf_fit(&stack->data, stack->size, &stack->capacity, stack->elem_size))
		return SP_ENOMEM;
	p = (char*)stack->data + (stack->size - idx) * stack->elem_size;
	q = (char*)stack->data + stack->size * stack->elem_size;
	*(unsigned char*)q = *(unsigned char*)p;
	*(unsigned char*)p = elem;
	++stack->size;
	return 0;
}

int sp_stack_qinsertus(struct sp_stack *stack, size_t idx, unsigned short elem)
{
	char *p, *q;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
	if (idx > stack->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return SP_ERANGE;
	if (sp_buf_fit(&stack->data, stack->size, &stack->capacity, stack->elem_size))
		return SP_ENOMEM;
	p = (char*)stack->data + (stack->size - idx) * stack->elem_size;
	q = (char*)stack->data + stack->size * stack->elem_size;
	*(unsigned short*)q = *(unsigned short*)p;
	*(unsigned short*)p = elem;
	++stack->size;
	return 0;
}

int sp_stack_qinsertui(struct sp_stack *stack, size_t idx, unsigned int elem)
{
	char *p, *q;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
	if (idx > stack->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return SP_ERANGE;
	if (sp_buf_fit(&stack->data, stack->size, &stack->capacity, stack->elem_size))
		return SP_ENOMEM;
	p = (char*)stack->data + (stack->size - idx) * stack->elem_size;
	q = (char*)stack->data + stack->size * stack->elem_size;
	*(unsigned int*)q = *(unsigned int*)p;
	*(unsigned int*)p = elem;
	++stack->size;
	return 0;
}

int sp_stack_qinsertul(struct sp_stack *stack, size_t idx, unsigned long elem)
{
	char *p, *q;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
	if (idx > stack->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return SP_ERANGE;
	if (sp_buf_fit(&stack->data, stack->size, &stack->capacity, stack->elem_size))
		return SP_ENOMEM;
	p = (char*)stack->data + (stack->size - idx) * stack->elem_size;
	q = (char*)stack->data + stack->size * stack->elem_size;
	*(unsigned long*)q = *(unsigned long*)p;
	*(unsigned long*)p = elem;
	++stack->size;
	return 0;
}

int sp_stack_qinsertf(struct sp_stack *stack, size_t idx, float elem)
{
	char *p, *q;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
	if (idx > stack->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return SP_ERANGE;
	if (sp_buf_fit(&stack->data, stack->size, &stack->capacity, stack->elem_size))
		return SP_ENOMEM;
	p = (char*)stack->data + (stack->size - idx) * stack->elem_size;
	q = (char*)stack->data + stack->size * stack->elem_size;
	*(float*)q = *(float*)p;
	*(float*)p = elem;
	++stack->size;
	return 0;
}

int sp_stack_qinsertd(struct sp_stack *stack, size_t idx, double elem)
{
	char *p, *q;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
	if (idx > stack->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return SP_ERANGE;
	if (sp_buf_fit(&stack->data, stack->size, &stack->capacity, stack->elem_size))
		return SP_ENOMEM;
	p = (char*)stack->data + (stack->size - idx) * stack->elem_size;
	q = (char*)stack->data + stack->size * stack->elem_size;
	*(double*)q = *(double*)p;
	*(double*)p = elem;
	++stack->size;
	return 0;
}

int sp_stack_qinsertld(struct sp_stack *stack, size_t idx, long double elem)
{
	char *p, *q;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
	if (idx > stack->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return SP_ERANGE;
	if (sp_buf_fit(&stack->data, stack->size, &stack->capacity, stack->elem_size))
		return SP_ENOMEM;
	p = (char*)stack->data + (stack->size - idx) * stack->elem_size;
	q = (char*)stack->data + stack->size * stack->elem_size;
	*(long double*)q = *(long double*)p;
	*(long double*)p = elem;
	++stack->size;
	return 0;
}

int sp_stack_qinsertstr(struct sp_stack *stack, size_t idx, const char *elem)
{
	char *p, *q;
	char *buf;
	size_t len;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (elem == NULL) {
		error(("elem is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
	if (idx > stack->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return SP_ERANGE;
	if (sp_buf_fit(&stack->data, stack->size, &stack->capacity, stack->elem_size))
		return SP_ENOMEM;
	len = sp_strnlen(elem, SIZE_MAX);
	if (sp_size_try_add(len, 1))
		return SP_ERANGE;
	buf = malloc((len + 1) * sizeof(*elem));
	if (buf == NULL) {
		error(("malloc"));
		return SP_ENOMEM;
	}
	memcpy(buf, elem, len * sizeof(*elem));
	buf[len] = '\0';
	p = (char*)stack->data + (stack->size - idx) * stack->elem_size;
	q = (char*)stack->data + stack->size * stack->elem_size;
	*(char**)q = *(char**)p;
	*(char**)p = buf;
	++stack->size;
	return 0;
}

int sp_stack_qinsertstrn(struct sp_stack *stack, size_t idx, const char *elem, size_t len)
{
	char *p, *q;
	char *buf;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (elem == NULL) {
		error(("elem is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
	if (idx > stack->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return SP_ERANGE;
	if (sp_buf_fit(&stack->data, stack->size, &stack->capacity, stack->elem_size))
		return SP_ENOMEM;
	if (sp_size_try_add(len, 1))
		return SP_ERANGE;
	buf = malloc((len + 1) * sizeof(*elem));
	if (buf == NULL) {
		error(("malloc"));
		return SP_ENOMEM;
	}
	memcpy(buf, elem, len * sizeof(*elem));
	buf[len] = '\0';
	p = (char*)stack->data + (stack->size - idx) * stack->elem_size;
	q = (char*)stack->data + stack->size * stack->elem_size;
	*(char**)q = *(char**)p;
	*(char**)p = buf;
	++stack->size;
	return 0;
}


int sp_stack_peek(const struct sp_stack *stack, void *output)
{
	const void *src;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (output == NULL) {
		error(("output is NULL"));
		return SP_EINVAL;
	}
	if (stack->size == 0) {
		error(("stack is empty"));
		return SP_EILLEGAL;
	}
#endif
	src = (char*)stack->data + (stack->size - 1) * stack->elem_size;
	memcpy(output, src, stack->elem_size);
	return 0;
}

char sp_stack_peekc(const struct sp_stack *stack)
{
#ifdef STAPLE_DEBUG
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

short sp_stack_peeks(const struct sp_stack *stack)
{
#ifdef STAPLE_DEBUG
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

int sp_stack_peeki(const struct sp_stack *stack)
{
#ifdef STAPLE_DEBUG
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

long sp_stack_peekl(const struct sp_stack *stack)
{
#ifdef STAPLE_DEBUG
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

signed char sp_stack_peeksc(const struct sp_stack *stack)
{
#ifdef STAPLE_DEBUG
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

unsigned char sp_stack_peekuc(const struct sp_stack *stack)
{
#ifdef STAPLE_DEBUG
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

unsigned short sp_stack_peekus(const struct sp_stack *stack)
{
#ifdef STAPLE_DEBUG
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

unsigned int sp_stack_peekui(const struct sp_stack *stack)
{
#ifdef STAPLE_DEBUG
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

unsigned long sp_stack_peekul(const struct sp_stack *stack)
{
#ifdef STAPLE_DEBUG
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

float sp_stack_peekf(const struct sp_stack *stack)
{
#ifdef STAPLE_DEBUG
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

double sp_stack_peekd(const struct sp_stack *stack)
{
#ifdef STAPLE_DEBUG
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

long double sp_stack_peekld(const struct sp_stack *stack)
{
#ifdef STAPLE_DEBUG
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

char *sp_stack_peekstr(const struct sp_stack *stack)
{
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return NULL;
	}
	if (stack->elem_size != sizeof(char*)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(char*)));
		return NULL;
	}
	if (stack->size == 0) {
		error(("stack is empty"));
		return NULL;
	}
#endif
	return ((char**)stack->data)[stack->size - 1];
}


int sp_stack_pop(struct sp_stack *stack, void *output)
{
	const void *src;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->size == 0) {
		error(("stack is empty"));
		return SP_EILLEGAL;
	}
#endif
	src = (char*)stack->data + (stack->size - 1) * stack->elem_size;
	if (output != NULL)
		memcpy(output, src, stack->elem_size);
	--stack->size;
	return 0;
}

char sp_stack_popc(struct sp_stack *stack)
{
#ifdef STAPLE_DEBUG
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

short sp_stack_pops(struct sp_stack *stack)
{
#ifdef STAPLE_DEBUG
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

int sp_stack_popi(struct sp_stack *stack)
{
#ifdef STAPLE_DEBUG
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

long sp_stack_popl(struct sp_stack *stack)
{
#ifdef STAPLE_DEBUG
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

signed char sp_stack_popsc(struct sp_stack *stack)
{
#ifdef STAPLE_DEBUG
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

unsigned char sp_stack_popuc(struct sp_stack *stack)
{
#ifdef STAPLE_DEBUG
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

unsigned short sp_stack_popus(struct sp_stack *stack)
{
#ifdef STAPLE_DEBUG
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

unsigned int sp_stack_popui(struct sp_stack *stack)
{
#ifdef STAPLE_DEBUG
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

unsigned long sp_stack_popul(struct sp_stack *stack)
{
#ifdef STAPLE_DEBUG
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

float sp_stack_popf(struct sp_stack *stack)
{
#ifdef STAPLE_DEBUG
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

double sp_stack_popd(struct sp_stack *stack)
{
#ifdef STAPLE_DEBUG
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

long double sp_stack_popld(struct sp_stack *stack)
{
#ifdef STAPLE_DEBUG
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

char *sp_stack_popstr(struct sp_stack *stack)
{
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return NULL;
	}
	if (stack->size == 0) {
		error(("stack is empty"));
		return NULL;
	}
	if (stack->elem_size != sizeof(char*)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(char*)));
		return NULL;
	}
#endif
	return ((char**)stack->data)[--stack->size];
}


int sp_stack_remove(struct sp_stack *stack, size_t idx, void *output)
{
	char *p;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	if (output != NULL)
		memcpy(output, p, stack->elem_size);
	memmove(p, p + stack->elem_size, idx * stack->elem_size);
	--stack->size;
	return 0;
}

char sp_stack_removec(struct sp_stack *stack, size_t idx)
{
	char *p;
	char ret;
#ifdef STAPLE_DEBUG
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

short sp_stack_removes(struct sp_stack *stack, size_t idx)
{
	char *p;
	short ret;
#ifdef STAPLE_DEBUG
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

int sp_stack_removei(struct sp_stack *stack, size_t idx)
{
	char *p;
	int ret;
#ifdef STAPLE_DEBUG
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

long sp_stack_removel(struct sp_stack *stack, size_t idx)
{
	char *p;
	long ret;
#ifdef STAPLE_DEBUG
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

signed char sp_stack_removesc(struct sp_stack *stack, size_t idx)
{
	char *p;
	signed char ret;
#ifdef STAPLE_DEBUG
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

unsigned char sp_stack_removeuc(struct sp_stack *stack, size_t idx)
{
	char *p;
	unsigned char ret;
#ifdef STAPLE_DEBUG
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

unsigned short sp_stack_removeus(struct sp_stack *stack, size_t idx)
{
	char *p;
	unsigned short ret;
#ifdef STAPLE_DEBUG
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

unsigned int sp_stack_removeui(struct sp_stack *stack, size_t idx)
{
	char *p;
	unsigned int ret;
#ifdef STAPLE_DEBUG
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

unsigned long sp_stack_removeul(struct sp_stack *stack, size_t idx)
{
	char *p;
	unsigned long ret;
#ifdef STAPLE_DEBUG
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

float sp_stack_removef(struct sp_stack *stack, size_t idx)
{
	char *p;
	float ret;
#ifdef STAPLE_DEBUG
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

double sp_stack_removed(struct sp_stack *stack, size_t idx)
{
	char *p;
	double ret;
#ifdef STAPLE_DEBUG
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

long double sp_stack_removeld(struct sp_stack *stack, size_t idx)
{
	char *p;
	long double ret;
#ifdef STAPLE_DEBUG
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

char *sp_stack_removestr(struct sp_stack *stack, size_t idx)
{
	char *p;
	char *ret;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return NULL;
	}
	if (stack->elem_size != sizeof(char*)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(char*)));
		return NULL;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return NULL;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	ret = *(char**)p;
	memmove(p, p + stack->elem_size, idx * stack->elem_size);
	--stack->size;
	return ret;
}


int sp_stack_qremove(struct sp_stack *stack, size_t idx, void *output)
{
	char *p, *q;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return SP_EINDEX;
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

char sp_stack_qremovec(struct sp_stack *stack, size_t idx)
{
	char *p, *q;
	char ret;
#ifdef STAPLE_DEBUG
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

short sp_stack_qremoves(struct sp_stack *stack, size_t idx)
{
	char *p, *q;
	short ret;
#ifdef STAPLE_DEBUG
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

int sp_stack_qremovei(struct sp_stack *stack, size_t idx)
{
	char *p, *q;
	int ret;
#ifdef STAPLE_DEBUG
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

long sp_stack_qremovel(struct sp_stack *stack, size_t idx)
{
	char *p, *q;
	long ret;
#ifdef STAPLE_DEBUG
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

signed char sp_stack_qremovesc(struct sp_stack *stack, size_t idx)
{
	char *p, *q;
	signed char ret;
#ifdef STAPLE_DEBUG
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

unsigned char sp_stack_qremoveuc(struct sp_stack *stack, size_t idx)
{
	char *p, *q;
	unsigned char ret;
#ifdef STAPLE_DEBUG
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

unsigned short sp_stack_qremoveus(struct sp_stack *stack, size_t idx)
{
	char *p, *q;
	unsigned short ret;
#ifdef STAPLE_DEBUG
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

unsigned int sp_stack_qremoveui(struct sp_stack *stack, size_t idx)
{
	char *p, *q;
	unsigned int ret;
#ifdef STAPLE_DEBUG
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

unsigned long sp_stack_qremoveul(struct sp_stack *stack, size_t idx)
{
	char *p, *q;
	unsigned long ret;
#ifdef STAPLE_DEBUG
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

float sp_stack_qremovef(struct sp_stack *stack, size_t idx)
{
	char *p, *q;
	float ret;
#ifdef STAPLE_DEBUG
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

double sp_stack_qremoved(struct sp_stack *stack, size_t idx)
{
	char *p, *q;
	double ret;
#ifdef STAPLE_DEBUG
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

long double sp_stack_qremoveld(struct sp_stack *stack, size_t idx)
{
	char *p, *q;
	long double ret;
#ifdef STAPLE_DEBUG
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

char *sp_stack_qremovestr(struct sp_stack *stack, size_t idx)
{
	char *p, *q;
	char *ret;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return NULL;
	}
	if (stack->elem_size != sizeof(char*)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(char*)));
		return NULL;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return NULL;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	q = (char*)stack->data + (stack->size - 1) * stack->elem_size;
	ret = *(char**)p;
	*(char**)p = *(char**)q;
	--stack->size;
	return ret;
}


int sp_stack_get(const struct sp_stack *stack, size_t idx, void *output)
{
	const void *src;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (output == NULL) {
		error(("output is NULL"));
		return SP_EINVAL;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	src = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	memcpy(output, src, stack->elem_size);
	return 0;
}

char sp_stack_getc(const struct sp_stack *stack, size_t idx)
{
#ifdef STAPLE_DEBUG
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

short sp_stack_gets(const struct sp_stack *stack, size_t idx)
{
#ifdef STAPLE_DEBUG
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

int sp_stack_geti(const struct sp_stack *stack, size_t idx)
{
#ifdef STAPLE_DEBUG
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

long sp_stack_getl(const struct sp_stack *stack, size_t idx)
{
#ifdef STAPLE_DEBUG
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

signed char sp_stack_getsc(const struct sp_stack *stack, size_t idx)
{
#ifdef STAPLE_DEBUG
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

unsigned char sp_stack_getuc(const struct sp_stack *stack, size_t idx)
{
#ifdef STAPLE_DEBUG
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

unsigned short sp_stack_getus(const struct sp_stack *stack, size_t idx)
{
#ifdef STAPLE_DEBUG
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

unsigned int sp_stack_getui(const struct sp_stack *stack, size_t idx)
{
#ifdef STAPLE_DEBUG
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

unsigned long sp_stack_getul(const struct sp_stack *stack, size_t idx)
{
#ifdef STAPLE_DEBUG
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

float sp_stack_getf(const struct sp_stack *stack, size_t idx)
{
#ifdef STAPLE_DEBUG
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

double sp_stack_getd(const struct sp_stack *stack, size_t idx)
{
#ifdef STAPLE_DEBUG
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

long double sp_stack_getld(const struct sp_stack *stack, size_t idx)
{
#ifdef STAPLE_DEBUG
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

char *sp_stack_getstr(const struct sp_stack *stack, size_t idx)
{
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return NULL;
	}
	if (stack->elem_size != sizeof(char*)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(char*)));
		return NULL;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return NULL;
	}
#endif
	return ((char**)stack->data)[stack->size - 1 - idx];
}


int sp_stack_set(struct sp_stack *stack, size_t idx, void *val)
{
	char *p;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (val == NULL) {
		error(("val is NULL"));
		return SP_EINVAL;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	memcpy(p, val, stack->elem_size);
	return 0;
}

int sp_stack_setc(struct sp_stack *stack, size_t idx, char val)
{
	char *p;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(val)) {
		error(("stack->elem_size is incompatible with val type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(val)));
		return SP_EILLEGAL;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	*p = val;
	return 0;
}

int sp_stack_sets(struct sp_stack *stack, size_t idx, short val)
{
	char *p;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(val)) {
		error(("stack->elem_size is incompatible with val type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(val)));
		return SP_EILLEGAL;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	*(short*)p = val;
	return 0;
}

int sp_stack_seti(struct sp_stack *stack, size_t idx, int val)
{
	char *p;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(val)) {
		error(("stack->elem_size is incompatible with val type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(val)));
		return SP_EILLEGAL;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	*(int*)p = val;
	return 0;
}

int sp_stack_setl(struct sp_stack *stack, size_t idx, long val)
{
	char *p;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(val)) {
		error(("stack->elem_size is incompatible with val type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(val)));
		return SP_EILLEGAL;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	*(long*)p = val;
	return 0;
}

int sp_stack_setsc(struct sp_stack *stack, size_t idx, signed char val)
{
	char *p;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(val)) {
		error(("stack->elem_size is incompatible with val type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(val)));
		return SP_EILLEGAL;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	*(signed char*)p = val;
	return 0;
}

int sp_stack_setuc(struct sp_stack *stack, size_t idx, unsigned char val)
{
	char *p;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(val)) {
		error(("stack->elem_size is incompatible with val type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(val)));
		return SP_EILLEGAL;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	*(unsigned char*)p = val;
	return 0;
}

int sp_stack_setus(struct sp_stack *stack, size_t idx, unsigned short val)
{
	char *p;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(val)) {
		error(("stack->elem_size is incompatible with val type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(val)));
		return SP_EILLEGAL;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	*(unsigned short*)p = val;
	return 0;
}

int sp_stack_setui(struct sp_stack *stack, size_t idx, unsigned int val)
{
	char *p;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(val)) {
		error(("stack->elem_size is incompatible with val type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(val)));
		return SP_EILLEGAL;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	*(unsigned int*)p = val;
	return 0;
}

int sp_stack_setul(struct sp_stack *stack, size_t idx, unsigned long val)
{
	char *p;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(val)) {
		error(("stack->elem_size is incompatible with val type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(val)));
		return SP_EILLEGAL;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	*(unsigned long*)p = val;
	return 0;
}

int sp_stack_setf(struct sp_stack *stack, size_t idx, float val)
{
	char *p;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(val)) {
		error(("stack->elem_size is incompatible with val type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(val)));
		return SP_EILLEGAL;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	*(float*)p = val;
	return 0;
}

int sp_stack_setd(struct sp_stack *stack, size_t idx, double val)
{
	char *p;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(val)) {
		error(("stack->elem_size is incompatible with val type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(val)));
		return SP_EILLEGAL;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	*(double*)p = val;
	return 0;
}

int sp_stack_setld(struct sp_stack *stack, size_t idx, long double val)
{
	char *p;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(val)) {
		error(("stack->elem_size is incompatible with val type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(val)));
		return SP_EILLEGAL;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	*(long double*)p = val;
	return 0;
}

int sp_stack_setstr(struct sp_stack *stack, size_t idx, const char *val)
{
	char *p;
	char *buf;
	size_t len;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (val == NULL) {
		error(("val is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(val)) {
		error(("stack->elem_size is incompatible with val type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(val)));
		return SP_EILLEGAL;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	free(*(char**)p);
	len = sp_strnlen(val, SIZE_MAX);
	if (sp_size_try_add(len, 1))
		return SP_ERANGE;
	buf = malloc((len + 1) * sizeof(*val));
	if (buf == NULL) {
		error(("malloc"));
		return SP_ENOMEM;
	}
	memcpy(buf, val, len * sizeof(*val));
	buf[len] = '\0';
	*(char**)p = buf;
	return 0;
}

int sp_stack_setstrn(struct sp_stack *stack, size_t idx, const char *val, size_t len)
{
	char *p;
	char *buf;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (val == NULL) {
		error(("val is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(val)) {
		error(("stack->elem_size is incompatible with val type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(val)));
		return SP_EILLEGAL;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	free(*(char**)p);
	if (sp_size_try_add(len, 1))
		return SP_ERANGE;
	buf = malloc((len + 1) * sizeof(*val));
	if (buf == NULL) {
		error(("malloc"));
		return SP_ENOMEM;
	}
	memcpy(buf, val, len * sizeof(*val));
	buf[len] = '\0';
	*(char**)p = buf;
	return 0;
}


int sp_stack_print(const struct sp_stack *stack)
{
	size_t i;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
#endif
	printf("sp_stack_print()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)stack->size, (unsigned long)stack->capacity, (unsigned long)stack->elem_size);
	for (i = stack->size; i-- > 0;) {
		const void *const elem = (char*)stack->data + i * stack->elem_size;
		printf("[%lu]\t%p\n", (unsigned long)stack->size - 1 - i, elem);
	}
	return 0;
}

int sp_stack_printc(const struct sp_stack *stack)
{
	size_t i;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(char)) {
		error(("stack->elem_size is incompatible with function type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(char)));
		return SP_EILLEGAL;
	}
#endif
	printf("sp_stack_printc()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)stack->size, (unsigned long)stack->capacity, (unsigned long)stack->elem_size);
	for (i = stack->size; i-- > 0;) {
		const char elem = ((char*)stack->data)[i];
		printf("[%lu]\t%hd\t'%c'\n", (unsigned long)stack->size - 1 - i, elem, elem);
	}
	return 0;
}

int sp_stack_prints(const struct sp_stack *stack)
{
	size_t i;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(short)) {
		error(("stack->elem_size is incompatible with function type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(short)));
		return SP_EILLEGAL;
	}
#endif
	printf("sp_stack_prints()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)stack->size, (unsigned long)stack->capacity, (unsigned long)stack->elem_size);
	for (i = stack->size; i-- > 0;) {
		const short elem = ((short*)stack->data)[i];
		printf("[%lu]\t%hd\n", (unsigned long)stack->size - 1 - i, elem);
	}
	return 0;
}

int sp_stack_printi(const struct sp_stack *stack)
{
	size_t i;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(int)) {
		error(("stack->elem_size is incompatible with function type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(int)));
		return SP_EILLEGAL;
	}
#endif
	printf("sp_stack_printi()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)stack->size, (unsigned long)stack->capacity, (unsigned long)stack->elem_size);
	for (i = stack->size; i-- > 0;) {
		const int elem = ((int*)stack->data)[i];
		printf("[%lu]\t%d\n", (unsigned long)stack->size - 1 - i, elem);
	}
	return 0;
}

int sp_stack_printl(const struct sp_stack *stack)
{
	size_t i;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(long)) {
		error(("stack->elem_size is incompatible with function type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(long)));
		return SP_EILLEGAL;
	}
#endif
	printf("sp_stack_printl()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)stack->size, (unsigned long)stack->capacity, (unsigned long)stack->elem_size);
	for (i = stack->size; i-- > 0;) {
		const long elem = ((long*)stack->data)[i];
		printf("[%lu]\t%ld\n", (unsigned long)stack->size - 1 - i, elem);
	}
	return 0;
}

int sp_stack_printsc(const struct sp_stack *stack)
{
	size_t i;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(signed char)) {
		error(("stack->elem_size is incompatible with function type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(signed char)));
		return SP_EILLEGAL;
	}
#endif
	printf("sp_stack_printsc()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)stack->size, (unsigned long)stack->capacity, (unsigned long)stack->elem_size);
	for (i = stack->size; i-- > 0;) {
		const signed char elem = ((signed char*)stack->data)[i];
		printf("[%lu]\t%hd\t'%c'\n", (unsigned long)stack->size - 1 - i, elem, elem);
	}
	return 0;
}

int sp_stack_printuc(const struct sp_stack *stack)
{
	size_t i;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(unsigned char)) {
		error(("stack->elem_size is incompatible with function type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(unsigned char)));
		return SP_EILLEGAL;
	}
#endif
	printf("sp_stack_printuc()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)stack->size, (unsigned long)stack->capacity, (unsigned long)stack->elem_size);
	for (i = stack->size; i-- > 0;) {
		const unsigned char elem = ((unsigned char*)stack->data)[i];
		printf("[%lu]\t%hd\t'%c'\n", (unsigned long)stack->size - 1 - i, elem, elem);
	}
	return 0;
}

int sp_stack_printus(const struct sp_stack *stack)
{
	size_t i;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(unsigned short)) {
		error(("stack->elem_size is incompatible with function type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(unsigned short)));
		return SP_EILLEGAL;
	}
#endif
	printf("sp_stack_printus()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)stack->size, (unsigned long)stack->capacity, (unsigned long)stack->elem_size);
	for (i = stack->size; i-- > 0;) {
		const unsigned short elem = ((unsigned short*)stack->data)[i];
		printf("[%lu]\t%hu\n", (unsigned long)stack->size - 1 - i, elem);
	}
	return 0;
}

int sp_stack_printui(const struct sp_stack *stack)
{
	size_t i;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(unsigned int)) {
		error(("stack->elem_size is incompatible with function type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(unsigned int)));
		return SP_EILLEGAL;
	}
#endif
	printf("sp_stack_printui()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)stack->size, (unsigned long)stack->capacity, (unsigned long)stack->elem_size);
	for (i = stack->size; i-- > 0;) {
		const unsigned int elem = ((unsigned int*)stack->data)[i];
		printf("[%lu]\t%u\n", (unsigned long)stack->size - 1 - i, elem);
	}
	return 0;
}

int sp_stack_printul(const struct sp_stack *stack)
{
	size_t i;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(unsigned long)) {
		error(("stack->elem_size is incompatible with function type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(unsigned long)));
		return SP_EILLEGAL;
	}
#endif
	printf("sp_stack_printul()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)stack->size, (unsigned long)stack->capacity, (unsigned long)stack->elem_size);
	for (i = stack->size; i-- > 0;) {
		const unsigned long elem = ((unsigned long*)stack->data)[i];
		printf("[%lu]\t%lu\n", (unsigned long)stack->size - 1 - i, elem);
	}
	return 0;
}

int sp_stack_printf(const struct sp_stack *stack)
{
	size_t i;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(float)) {
		error(("stack->elem_size is incompatible with function type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(float)));
		return SP_EILLEGAL;
	}
#endif
	printf("sp_stack_printf()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)stack->size, (unsigned long)stack->capacity, (unsigned long)stack->elem_size);
	for (i = stack->size; i-- > 0;) {
		const float elem = ((float*)stack->data)[i];
		printf("[%lu]\t%g\n", (unsigned long)stack->size - 1 - i, elem);
	}
	return 0;
}

int sp_stack_printd(const struct sp_stack *stack)
{
	size_t i;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(double)) {
		error(("stack->elem_size is incompatible with function type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(double)));
		return SP_EILLEGAL;
	}
#endif
	printf("sp_stack_printd()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)stack->size, (unsigned long)stack->capacity, (unsigned long)stack->elem_size);
	for (i = stack->size; i-- > 0;) {
		const double elem = ((double*)stack->data)[i];
		printf("[%lu]\t%g\n", (unsigned long)stack->size - 1 - i, elem);
	}
	return 0;
}

int sp_stack_printld(const struct sp_stack *stack)
{
	size_t i;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(long double)) {
		error(("stack->elem_size is incompatible with function type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(long double)));
		return SP_EILLEGAL;
	}
#endif
	printf("sp_stack_printld()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)stack->size, (unsigned long)stack->capacity, (unsigned long)stack->elem_size);
	for (i = stack->size; i-- > 0;) {
		const long double elem = ((long double*)stack->data)[i];
		printf("[%lu]\t%Lg\n", (unsigned long)stack->size - 1 - i, elem);
	}
	return 0;
}

int sp_stack_printstr(const struct sp_stack *stack)
{
	size_t i;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(char*)) {
		error(("stack->elem_size is incompatible with function type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(char*)));
		return SP_EILLEGAL;
	}
#endif
	printf("sp_stack_printstr()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)stack->size, (unsigned long)stack->capacity, (unsigned long)stack->elem_size);
	for (i = stack->size; i-- > 0;) {
		const char *elem = ((char**)stack->data)[i];
		printf("[%lu]\t%s\n", (unsigned long)stack->size - 1 - i, elem);
	}
	return 0;
}
