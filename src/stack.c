#include "stack.h"
#include "helpers.h"
#include "errcodes.h"
#include <string.h>

struct rnd_stack *rnd_stack_create(size_t elem_size, size_t capacity)
{
	struct rnd_stack *ret;

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
	ret->data      = malloc(elem_size * capacity);
	if (ret->data == NULL) {
		error(("malloc"));
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
		if (rnd_foomap(stack->data, stack->size, stack->elem_size, dtor))
			return RND_EHANDLER;
	} else {
		stack->size = 0;
	}
	return 0;
}

int rnd_stack_destroy(struct rnd_stack *stack, int (*dtor)(void*))
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
#endif
	if (dtor != NULL && rnd_foomap(stack->data, stack->size, stack->elem_size, dtor))
		return RND_EHANDLER;
	free(stack->data);
	free(stack);
	return 0;
}

int rnd_stack_copy(struct rnd_stack *dest, const struct rnd_stack *src, void *(*cpy)(const void *))
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
	dest->elem_size = src->elem_size;
	dest->size      = src->size;
	dest->capacity  = src->capacity;
	dest->data      = calloc(dest->capacity, dest->elem_size);
	if (dest->data == NULL) {
		error(("calloc"));
		return RND_ENOMEM;
	}
	if (cpy == NULL) {
		const void *const src_end = (char*)src->data + src->elem_size * src->capacity;
		s = src->data;
		d = dest->data;
		while (s != src_end) {
			memcpy(d, s, src->elem_size);
			s += src->elem_size;
			d += dest->elem_size;
		}
	} else {
		const void *const src_end = (char*)src->data + src->elem_size * src->capacity;
		s = src->data;
		d = dest->data;
		while (s != src_end) {
			const void *const new = cpy(s);
			if (new == NULL) {
				error("external cpy function returned NULL");
				return RND_EHANDLER;
			}
			memcpy(d, new, src->elem_size);
			s += src->elem_size;
			d += dest->elem_size;
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
#endif
	if (rnd_buffit(&stack->data, stack->size * stack->elem_size, &stack->capacity))
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
#endif
	if (rnd_buffit(&stack->data, stack->size * stack->elem_size, &stack->capacity))
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
#endif
	if (rnd_buffit(&stack->data, stack->size * stack->elem_size, &stack->capacity))
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
#endif
	if (rnd_buffit(&stack->data, stack->size * stack->elem_size, &stack->capacity))
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
#endif
	if (rnd_buffit(&stack->data, stack->size * stack->elem_size, &stack->capacity))
		return RND_ENOMEM;
	((long*)stack->data)[stack->size++] = elem;
	return 0;
}

int rnd_stack_pushuc(struct rnd_stack *stack, unsigned char elem)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
#endif
	if (rnd_buffit(&stack->data, stack->size * stack->elem_size, &stack->capacity))
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
#endif
	if (rnd_buffit(&stack->data, stack->size * stack->elem_size, &stack->capacity))
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
#endif
	if (rnd_buffit(&stack->data, stack->size * stack->elem_size, &stack->capacity))
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
#endif
	if (rnd_buffit(&stack->data, stack->size * stack->elem_size, &stack->capacity))
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
#endif
	if (rnd_buffit(&stack->data, stack->size * stack->elem_size, &stack->capacity))
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
#endif
	if (rnd_buffit(&stack->data, stack->size * stack->elem_size, &stack->capacity))
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
#endif
	if (rnd_buffit(&stack->data, stack->size * stack->elem_size, &stack->capacity))
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
	if (idx > stack->size) {
		error(("index out of range"));
		return RND_EORANGE;
	}
#endif
	if (rnd_buffit(&stack->data, stack->size * stack->elem_size, &stack->capacity))
		return RND_ENOMEM;
	p = (char*)stack->data + stack->elem_size * (stack->size - idx);
	memmove(p + stack->elem_size, p, stack->elem_size * idx);
	memcpy(p, elem, stack->elem_size);
	return 0;
}

int rnd_stack_insertc(struct rnd_stack *stack, size_t idx, char elem)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
#endif
}

int rnd_stack_inserts(struct rnd_stack *stack, size_t idx, short elem)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
#endif
}

int rnd_stack_inserti(struct rnd_stack *stack, size_t idx, int elem)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
#endif
}

int rnd_stack_insertl(struct rnd_stack *stack, size_t idx, long elem)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
#endif
}

int rnd_stack_insertuc(struct rnd_stack *stack, size_t idx, unsigned char elem)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
#endif
}

int rnd_stack_insertus(struct rnd_stack *stack, size_t idx, unsigned short elem)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
#endif
}

int rnd_stack_insertui(struct rnd_stack *stack, size_t idx, unsigned int elem)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
#endif
}

int rnd_stack_insertul(struct rnd_stack *stack, size_t idx, unsigned long elem)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
#endif
}

int rnd_stack_insertf(struct rnd_stack *stack, size_t idx, float elem)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
#endif
}

int rnd_stack_insertd(struct rnd_stack *stack, size_t idx, double elem)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
#endif
}

int rnd_stack_insertld(struct rnd_stack *stack, size_t idx, long double elem)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
#endif
}


void *rnd_stack_peek(const struct rnd_stack *stack)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
#endif
}

char rnd_stack_peekc(const struct rnd_stack *stack)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
#endif
}

short rnd_stack_peeks(const struct rnd_stack *stack)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
#endif
}

int rnd_stack_peeki(const struct rnd_stack *stack)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
#endif
}

long rnd_stack_peekl(const struct rnd_stack *stack)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
#endif
}

unsigned char rnd_stack_peekuc(const struct rnd_stack *stack)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
#endif
}

unsigned short rnd_stack_peekus(const struct rnd_stack *stack)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
#endif
}

unsigned int rnd_stack_peekui(const struct rnd_stack *stack)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
#endif
}

unsigned long rnd_stack_peekul(const struct rnd_stack *stack)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
#endif
}

float rnd_stack_peekf(const struct rnd_stack *stack)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
#endif
}

double rnd_stack_peekd(const struct rnd_stack *stack)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
#endif
}

long double rnd_stack_peekld(const struct rnd_stack *stack)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
#endif
}


void *rnd_stack_pop(struct rnd_stack *stack)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
#endif
}

char rnd_stack_popc(struct rnd_stack *stack)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
#endif
}

short rnd_stack_pops(struct rnd_stack *stack)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
#endif
}

int rnd_stack_popi(struct rnd_stack *stack)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
#endif
}

long rnd_stack_popl(struct rnd_stack *stack)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
#endif
}

unsigned char rnd_stack_popuc(struct rnd_stack *stack)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
#endif
}

unsigned short rnd_stack_popus(struct rnd_stack *stack)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
#endif
}

unsigned int rnd_stack_popui(struct rnd_stack *stack)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
#endif
}

unsigned long rnd_stack_popul(struct rnd_stack *stack)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
#endif
}

float rnd_stack_popf(struct rnd_stack *stack)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
#endif
}

double rnd_stack_popd(struct rnd_stack *stack)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
#endif
}

long double rnd_stack_popld(struct rnd_stack *stack)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
#endif
}


void *rnd_stack_remove(struct rnd_stack *stack, size_t idx)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
#endif
}

char rnd_stack_removec(struct rnd_stack *stack, size_t idx)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
#endif
}

short rnd_stack_removes(struct rnd_stack *stack, size_t idx)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
#endif
}

int rnd_stack_removei(struct rnd_stack *stack, size_t idx)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
#endif
}

long rnd_stack_removel(struct rnd_stack *stack, size_t idx)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
#endif
}

unsigned char rnd_stack_removeuc(struct rnd_stack *stack, size_t idx)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
#endif
}

unsigned short rnd_stack_removeus(struct rnd_stack *stack, size_t idx)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
#endif
}

unsigned int rnd_stack_removeui(struct rnd_stack *stack, size_t idx)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
#endif
}

unsigned long rnd_stack_removeul(struct rnd_stack *stack, size_t idx)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
#endif
}

float rnd_stack_removef(struct rnd_stack *stack, size_t idx)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
#endif
}

double rnd_stack_removed(struct rnd_stack *stack, size_t idx)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
#endif
}

long double rnd_stack_removeld(struct rnd_stack *stack, size_t idx)
{
#ifdef RND_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return RND_EINVAL;
	}
#endif
}
