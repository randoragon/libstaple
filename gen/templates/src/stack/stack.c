#include "../sp_stack.h"
#include "../internal.h"

/*F{*/
#include "../sp_utils.h"
struct sp_stack *sp_stack_create(size_t elem_size, size_t capacity)
{
	struct sp_stack *ret;

#ifdef STAPLE_DEBUG
	/*. C_ERR_CAPACITY_ZERO */
#endif
	if (elem_size != SP_SIZEOF_BOOL && capacity > SP_SIZE_MAX / elem_size) {
		/*. C_ERRMSG_SIZE_T_OVERFLOW */
		return NULL;
	}

	ret = malloc(sizeof(*ret));
	if (ret == NULL) {
		/*. C_ERRMSG_MALLOC */
		return NULL;
	}

	ret->elem_size = elem_size;
	ret->size      = 0;
	if (elem_size == SP_SIZEOF_BOOL) {
		ret->capacity = ROUND_UP_TO_BYTE(capacity);
		ret->data     = malloc(ret->capacity / SP_BYTE_SIZE);
	} else {
		ret->capacity = capacity;
		ret->data     = malloc(capacity * elem_size);
	}
	if (ret->data == NULL) {
		/*. C_ERRMSG_MALLOC */
		free(ret);
		return NULL;
	}

	return ret;
}
/*F}*/

/*F{*/
#include "../sp_errcodes.h"
int sp_stack_clear(struct sp_stack *stack, int (*dtor)(void*))
{
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR stack SP_EINVAL */
	/*. C_ERR_BOOL_DTOR stack SP_EILLEGAL */
#endif
	if (dtor != NULL) {
		const void *const end = (char*)stack->data + stack->size * stack->elem_size;
		char *p = stack->data;
		while (p != end) {
			int err;
			if ((err = dtor(p))) {
				/*. C_ERRMSG_CALLBACK_NON_ZERO dtor err */
				return SP_ECALLBK;
			}
			p += stack->elem_size;
		}
	}
	stack->size = 0;
	return 0;
}
/*F}*/

/*F{*/
int sp_stack_destroy(struct sp_stack *stack, int (*dtor)(void*))
{
	int error;
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR stack SP_EINVAL */
	/*. C_ERR_BOOL_DTOR stack SP_EILLEGAL */
#endif
	if ((error = sp_stack_clear(stack, dtor)))
		return error;
	free(stack->data);
	free(stack);
	return 0;
}
/*F}*/

/*F{*/
#include <string.h>
int sp_stack_eq(const struct sp_stack *stack1, const struct sp_stack *stack2, int (*cmp)(const void*, const void*))
{
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR stack1 0 */
	/*. C_ERR_NULLPTR stack2 0 */
#endif
	if (stack1->elem_size != stack2->elem_size || stack1->size != stack2->size)
		return 0;
	if (cmp) {
		size_t i;
		for (i = 0; i < stack1->size; i++) {
			const void *const p = (char*)stack1->data + i * stack1->elem_size,
			           *const q = (char*)stack2->data + i * stack2->elem_size;
			if (cmp(p, q))
				return 0;
		}
		return 1;
	}
	return !memcmp(stack1->data, stack2->data, stack1->elem_size * stack1->size);
}
/*F}*/

/*F{*/
#include "../sp_errcodes.h"
#include <string.h>
int sp_stack_copy(struct sp_stack *dest, const struct sp_stack *src, int (*cpy)(void*, const void*))
{
	char *s, *d;
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR src SP_EINVAL */
	/*. C_ERR_NULLPTR dest SP_EINVAL */
#endif
	if (dest->capacity * dest->elem_size < src->size * src->elem_size) {
		dest->capacity = src->size;
		dest->data = realloc(dest->data, dest->capacity * src->elem_size);
		if (dest->data == NULL) {
			/*. C_ERRMSG_REALLOC */
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
				/*. C_ERRMSG_CALLBACK_NON_ZERO cpy err */
				return SP_ECALLBK;
			}
			s += src->elem_size;
			d += dest->elem_size;
		}
	}
	return 0;
}
/*F}*/

/*F{*/
#include "../sp_errcodes.h"
int sp_stack_map(struct sp_stack *stack, int (*func)(void*, size_t))
{
	size_t i;
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR stack SP_EINVAL */
	/*. C_ERR_NULLPTR func SP_EINVAL */
#endif
	for (i = 0; i < stack->size; i++) {
		void *const p = (char*)stack->data + i * stack->elem_size;
		int err;
		if ((err = func(p, i))) {
			/*. C_ERRMSG_CALLBACK_NON_ZERO func err */
			return SP_ECALLBK;
		}
	}
	return 0;
}
/*F}*/

/*F{*/
#include "../sp_errcodes.h"
#include <string.h>
int sp_stack_push(struct sp_stack *stack, const void *elem)
{
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR stack SP_EINVAL */
	/*. C_ERR_NULLPTR elem SP_EINVAL */
#endif
	if (sp_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return SP_ERANGE;
	if (sp_buf_fit(&stack->data, stack->size, &stack->capacity, stack->elem_size))
		return SP_ENOMEM;
	memcpy((char*)stack->data + stack->size++ * stack->elem_size, elem, stack->elem_size);
	return 0;
}
/*F}*/

/*F{*/
#include "../sp_errcodes.h"
int sp_stack_push$SUFFIX$(struct sp_stack *stack, $TYPE$ elem)
{
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR stack SP_EINVAL */
	/*. C_ERR_INCOMPAT_ELEM_TYPE stack sizeof(elem) SP_EILLEGAL */
#endif
	if (sp_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return SP_ERANGE;
	if (sp_buf_fit(&stack->data, stack->size, &stack->capacity, stack->elem_size))
		return SP_ENOMEM;
	(($TYPE$*)stack->data)[stack->size++] = elem;
	return 0;
}
/*F}*/

/*F{*/
#include "../sp_utils.h"
#include "../sp_errcodes.h"
int sp_stack_pushb(struct sp_stack *stack, int elem)
{
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR stack SP_EINVAL */
	/*. C_ERR_INCOMPAT_ELEM_TYPE stack SP_SIZEOF_BOOL SP_EILLEGAL */
#endif
	if (stack->size % SP_BYTE_SIZE == 0) {
		if (sp_size_try_add(stack->size, 1))
			return SP_ERANGE;
		if (sp_boolbuf_fit(&stack->data, stack->size, &stack->capacity))
			return SP_ENOMEM;
	}
	sp_boolbuf_set(stack->size++, elem, stack->data);
	return 0;
}
/*F}*/

/*F{*/
#include "../sp_errcodes.h"
#include <string.h>
int sp_stack_pushstr(struct sp_stack *stack, const char *elem)
{
	char *buf;
	size_t len;
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR stack SP_EINVAL */
	/*. C_ERR_NULLPTR elem SP_EINVAL */
	/*. C_ERR_INCOMPAT_ELEM_TYPE stack sizeof(elem) SP_EILLEGAL */
#endif
	if (sp_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return SP_ERANGE;
	if (sp_buf_fit(&stack->data, stack->size, &stack->capacity, stack->elem_size))
		return SP_ENOMEM;
	len = sp_strnlen(elem, SP_SIZE_MAX);
	if (sp_size_try_add(len, 1))
		return SP_ERANGE;
	buf = malloc((len + 1) * sizeof(*elem));
	if (buf == NULL) {
		/*. C_ERRMSG_MALLOC */
		return SP_ENOMEM;
	}
	memcpy(buf, elem, len * sizeof(*elem));
	buf[len] = '\0';
	((char**)stack->data)[stack->size++] = buf;
	return 0;
}
/*F}*/

/*F{*/
#include "../sp_errcodes.h"
#include <string.h>
int sp_stack_pushstrn(struct sp_stack *stack, const char *elem, size_t len)
{
	char *buf;
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR stack SP_EINVAL */
	/*. C_ERR_NULLPTR elem SP_EINVAL */
	/*. C_ERR_INCOMPAT_ELEM_TYPE stack sizeof(elem) SP_EILLEGAL */
#endif
	if (sp_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return SP_ERANGE;
	if (sp_buf_fit(&stack->data, stack->size, &stack->capacity, stack->elem_size))
		return SP_ENOMEM;
	if (sp_size_try_add(len, 1))
		return SP_ERANGE;
	buf = malloc((len + 1) * sizeof(*elem));
	if (buf == NULL) {
		/*. C_ERRMSG_MALLOC */
		return SP_ENOMEM;
	}
	memcpy(buf, elem, len * sizeof(*elem));
	buf[len] = '\0';
	((char**)stack->data)[stack->size++] = buf;
	return 0;
}
/*F}*/


/*F{*/
#include "../sp_errcodes.h"
#include <string.h>
int sp_stack_insert(struct sp_stack *stack, size_t idx, const void *elem)
{
	char *p;
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR stack SP_EINVAL */
	/*. C_ERR_NULLPTR elem SP_EINVAL */
	if (idx > stack->size) {
		/*. C_ERRMSG_INDEX_OUT_OF_RANGE */
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
/*F}*/

/*F{*/
#include "../sp_errcodes.h"
#include <string.h>
int sp_stack_insert$SUFFIX$(struct sp_stack *stack, size_t idx, $TYPE$ elem)
{
	char *p;
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR stack SP_EINVAL */
	/*. C_ERR_INCOMPAT_ELEM_TYPE stack sizeof(elem) SP_EILLEGAL */
	if (idx > stack->size) {
		/*. C_ERRMSG_INDEX_OUT_OF_RANGE */
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return SP_ERANGE;
	if (sp_buf_fit(&stack->data, stack->size, &stack->capacity, stack->elem_size))
		return SP_ENOMEM;
	p = (char*)stack->data + (stack->size - idx) * stack->elem_size;
	memmove(p + stack->elem_size, p, idx * stack->elem_size);
	*($TYPE$*)p = elem;
	++stack->size;
	return 0;
}
/*F}*/

/*F{*/
#include "../sp_errcodes.h"
#include <string.h>
int sp_stack_insertstr(struct sp_stack *stack, size_t idx, const char *elem)
{
	char *p;
	char *buf;
	size_t len;
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR stack SP_EINVAL */
	/*. C_ERR_NULLPTR elem SP_EINVAL */
	/*. C_ERR_INCOMPAT_ELEM_TYPE stack sizeof(elem) SP_EILLEGAL */
	if (idx > stack->size) {
		/*. C_ERRMSG_INDEX_OUT_OF_RANGE */
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return SP_ERANGE;
	if (sp_buf_fit(&stack->data, stack->size, &stack->capacity, stack->elem_size))
		return SP_ENOMEM;
	len = sp_strnlen(elem, SP_SIZE_MAX);
	if (sp_size_try_add(len, 1))
		return SP_ERANGE;
	buf = malloc((len + 1) * sizeof(*elem));
	if (buf == NULL) {
		/*. C_ERRMSG_MALLOC */
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
/*F}*/

/*F{*/
#include "../sp_errcodes.h"
#include <string.h>
int sp_stack_insertstrn(struct sp_stack *stack, size_t idx, const char *elem, size_t len)
{
	char *p;
	char *buf;
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR stack SP_EINVAL */
	/*. C_ERR_NULLPTR elem SP_EINVAL */
	/*. C_ERR_INCOMPAT_ELEM_TYPE stack sizeof(elem) SP_EILLEGAL */
	if (idx > stack->size) {
		/*. C_ERRMSG_INDEX_OUT_OF_RANGE */
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
		/*. C_ERRMSG_MALLOC */
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
/*F}*/


/*F{*/
#include "../sp_errcodes.h"
#include <string.h>
int sp_stack_qinsert(struct sp_stack *stack, size_t idx, const void *elem)
{
	char *p, *q;
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR stack SP_EINVAL */
	/*. C_ERR_NULLPTR elem SP_EINVAL */
	if (idx > stack->size) {
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
/*F}*/

/*F{*/
#include "../sp_errcodes.h"
int sp_stack_qinsert$SUFFIX$(struct sp_stack *stack, size_t idx, $TYPE$ elem)
{
	char *p, *q;
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR stack SP_EINVAL */
	/*. C_ERR_INCOMPAT_ELEM_TYPE stack sizeof(elem) SP_EILLEGAL */
	if (idx > stack->size) {
		/*. C_ERRMSG_INDEX_OUT_OF_RANGE */
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return SP_ERANGE;
	if (sp_buf_fit(&stack->data, stack->size, &stack->capacity, stack->elem_size))
		return SP_ENOMEM;
	p = (char*)stack->data + (stack->size - idx) * stack->elem_size;
	q = (char*)stack->data + stack->size * stack->elem_size;
	*($TYPE$*)q = *($TYPE$*)p;
	*($TYPE$*)p = elem;
	++stack->size;
	return 0;
}
/*F}*/

/*F{*/
#include "../sp_errcodes.h"
#include <string.h>
int sp_stack_qinsertstr(struct sp_stack *stack, size_t idx, const char *elem)
{
	char *p, *q;
	char *buf;
	size_t len;
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR stack SP_EINVAL */
	/*. C_ERR_NULLPTR elem SP_EINVAL */
	/*. C_ERR_INCOMPAT_ELEM_TYPE stack sizeof(elem) SP_EILLEGAL */
	if (idx > stack->size) {
		/*. C_ERRMSG_INDEX_OUT_OF_RANGE */
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(stack->size * stack->elem_size, stack->elem_size))
		return SP_ERANGE;
	if (sp_buf_fit(&stack->data, stack->size, &stack->capacity, stack->elem_size))
		return SP_ENOMEM;
	len = sp_strnlen(elem, SP_SIZE_MAX);
	if (sp_size_try_add(len, 1))
		return SP_ERANGE;
	buf = malloc((len + 1) * sizeof(*elem));
	if (buf == NULL) {
		/*. C_ERRMSG_MALLOC */
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
/*F}*/

/*F{*/
#include "../sp_errcodes.h"
#include <string.h>
int sp_stack_qinsertstrn(struct sp_stack *stack, size_t idx, const char *elem, size_t len)
{
	char *p, *q;
	char *buf;
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR stack SP_EINVAL */
	/*. C_ERR_NULLPTR elem SP_EINVAL */
	/*. C_ERR_INCOMPAT_ELEM_TYPE stack sizeof(elem) SP_EILLEGAL */
	if (idx > stack->size) {
		/*. C_ERRMSG_INDEX_OUT_OF_RANGE */
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
		/*. C_ERRMSG_MALLOC */
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
/*F}*/


/*F{*/
void *sp_stack_peek(const struct sp_stack *stack)
{
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR stack NULL */
	if (stack->size == 0) {
		/*. C_ERRMSG_IS_EMPTY stack */
		return NULL;
	}
#endif
	return (char*)stack->data + (stack->size - 1) * stack->elem_size;
}
/*F}*/

/*F{*/
$TYPE$ sp_stack_peek$SUFFIX$(const struct sp_stack *stack)
{
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR stack 0 */
	/*. C_ERR_INCOMPAT_ELEM_TYPE stack sizeof($TYPE$) 0 */
	if (stack->size == 0) {
		/*. C_ERRMSG_IS_EMPTY stack */
		return 0;
	}
#endif
	return (($TYPE$*)stack->data)[stack->size - 1];
}
/*F}*/

/*F{*/
int sp_stack_peekb(const struct sp_stack *stack)
{
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR stack 0 */
	/*. C_ERR_INCOMPAT_ELEM_TYPE stack SP_SIZEOF_BOOL 0 */
	if (stack->size == 0) {
		/*. C_ERRMSG_IS_EMPTY stack */
		return 0;
	}
#endif
	return sp_boolbuf_get(stack->size - 1, stack->data);
}
/*F}*/

/*F{*/
char *sp_stack_peekstr(const struct sp_stack *stack)
{
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR stack NULL */
	/*. C_ERR_INCOMPAT_ELEM_TYPE stack sizeof(char*) NULL */
	if (stack->size == 0) {
		/*. C_ERRMSG_IS_EMPTY stack */
		return NULL;
	}
#endif
	return ((char**)stack->data)[stack->size - 1];
}
/*F}*/


/*F{*/
#include "../sp_errcodes.h"
#include <string.h>
int sp_stack_pop(struct sp_stack *stack, int (*dtor)(void*))
{
	void *p;
	int err;
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR stack SP_EINVAL */
	if (stack->size == 0) {
		/*. C_ERRMSG_IS_EMPTY stack */
		return SP_EILLEGAL;
	}
#endif
	p = (char*)stack->data + (stack->size - 1) * stack->elem_size;
	if (dtor != NULL && (err = dtor(p))) {
		/*. C_ERRMSG_CALLBACK_NON_ZERO dtor err */
		return SP_ECALLBK;
	}
	--stack->size;
	return 0;
}
/*F}*/

/*F{*/
$TYPE$ sp_stack_pop$SUFFIX$(struct sp_stack *stack)
{
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR stack 0 */
	if (stack->size == 0) {
		/*. C_ERRMSG_IS_EMPTY stack */
		return 0;
	}
	/*. C_ERR_INCOMPAT_ELEM_TYPE stack sizeof($TYPE$) 0 */
#endif
	return (($TYPE$*)stack->data)[--stack->size];
}
/*F}*/

/*F{*/
char *sp_stack_popstr(struct sp_stack *stack)
{
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR stack NULL */
	if (stack->size == 0) {
		/*. C_ERRMSG_IS_EMPTY stack */
		return NULL;
	}
	/*. C_ERR_INCOMPAT_ELEM_TYPE stack sizeof(char*) NULL */
#endif
	return ((char**)stack->data)[--stack->size];
}
/*F}*/


/*F{*/
#include "../sp_errcodes.h"
#include <string.h>
int sp_stack_remove(struct sp_stack *stack, size_t idx, int (*dtor)(void*))
{
	char *p;
	int err;
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR stack SP_EINVAL */
	if (idx >= stack->size) {
		/*. C_ERRMSG_INDEX_OUT_OF_RANGE */
		return SP_EINDEX;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	if (dtor != NULL && (err = dtor(p))) {
		/*. C_ERRMSG_CALLBACK_NON_ZERO dtor err */
		return SP_ECALLBK;
	}
	memmove(p, p + stack->elem_size, idx * stack->elem_size);
	--stack->size;
	return 0;
}
/*F}*/

/*F{*/
#include <string.h>
$TYPE$ sp_stack_remove$SUFFIX$(struct sp_stack *stack, size_t idx)
{
	char *p;
	$TYPE$ ret;
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR stack 0 */
	/*. C_ERR_INCOMPAT_ELEM_TYPE stack sizeof($TYPE$) 0 */
	if (idx >= stack->size) {
		/*. C_ERRMSG_INDEX_OUT_OF_RANGE */
		return 0;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	ret = *($TYPE$*)p;
	memmove(p, p + stack->elem_size, idx * stack->elem_size);
	--stack->size;
	return ret;
}
/*F}*/

/*F{*/
#include <string.h>
char *sp_stack_removestr(struct sp_stack *stack, size_t idx)
{
	char *p;
	char *ret;
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR stack NULL */
	/*. C_ERR_INCOMPAT_ELEM_TYPE stack sizeof(char*) NULL */
	if (idx >= stack->size) {
		/*. C_ERRMSG_INDEX_OUT_OF_RANGE */
		return NULL;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	ret = *(char**)p;
	memmove(p, p + stack->elem_size, idx * stack->elem_size);
	--stack->size;
	return ret;
}
/*F}*/


/*F{*/
#include "../sp_errcodes.h"
#include <string.h>
int sp_stack_qremove(struct sp_stack *stack, size_t idx, int (*dtor)(void*))
{
	char *p, *q;
	int err;
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR stack SP_EINVAL */
	if (idx >= stack->size) {
		/*. C_ERRMSG_INDEX_OUT_OF_RANGE */
		return SP_EINDEX;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	if (dtor != NULL && (err = dtor(p))) {
		/*. C_ERRMSG_CALLBACK_NON_ZERO dtor err */
		return SP_ECALLBK;
	}
	q = (char*)stack->data + (stack->size - 1) * stack->elem_size;
	memcpy(p, q, stack->elem_size);
	--stack->size;
	return 0;
}
/*F}*/

/*F{*/
$TYPE$ sp_stack_qremove$SUFFIX$(struct sp_stack *stack, size_t idx)
{
	char *p, *q;
	$TYPE$ ret;
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR stack 0 */
	/*. C_ERR_INCOMPAT_ELEM_TYPE stack sizeof($TYPE$) 0 */
	if (idx >= stack->size) {
		/*. C_ERRMSG_INDEX_OUT_OF_RANGE */
		return 0;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	q = (char*)stack->data + (stack->size - 1) * stack->elem_size;
	ret = *($TYPE$*)p;
	*($TYPE$*)p = *($TYPE$*)q;
	--stack->size;
	return ret;
}
/*F}*/

/*F{*/
char *sp_stack_qremovestr(struct sp_stack *stack, size_t idx)
{
	char *p, *q;
	char *ret;
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR stack NULL */
	/*. C_ERR_INCOMPAT_ELEM_TYPE stack sizeof(char*) NULL */
	if (idx >= stack->size) {
		/*. C_ERRMSG_INDEX_OUT_OF_RANGE */
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
/*F}*/


/*F{*/
void *sp_stack_get(const struct sp_stack *stack, size_t idx)
{
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR stack NULL */
	if (idx >= stack->size) {
		/*. C_ERRMSG_INDEX_OUT_OF_RANGE */
		return NULL;
	}
#endif
	return (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
}
/*F}*/

/*F{*/
$TYPE$ sp_stack_get$SUFFIX$(const struct sp_stack *stack, size_t idx)
{
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR stack 0 */
	/*. C_ERR_INCOMPAT_ELEM_TYPE stack sizeof($TYPE$) 0 */
	if (idx >= stack->size) {
		/*. C_ERRMSG_INDEX_OUT_OF_RANGE */
		return 0;
	}
#endif
	return (($TYPE$*)stack->data)[stack->size - 1 - idx];
}
/*F}*/

/*F{*/
int sp_stack_getb(const struct sp_stack *stack, size_t idx)
{
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR stack 0 */
	/*. C_ERR_INCOMPAT_ELEM_TYPE stack SP_SIZEOF_BOOL 0 */
	if (idx >= stack->size) {
		/*. C_ERRMSG_INDEX_OUT_OF_RANGE */
		return 0;
	}
#endif
	return sp_boolbuf_get(stack->size - 1 - idx, stack->data);
}
/*F}*/

/*F{*/
char *sp_stack_getstr(const struct sp_stack *stack, size_t idx)
{
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR stack NULL */
	/*. C_ERR_INCOMPAT_ELEM_TYPE stack sizeof(char*) NULL */
	if (idx >= stack->size) {
		/*. C_ERRMSG_INDEX_OUT_OF_RANGE */
		return NULL;
	}
#endif
	return ((char**)stack->data)[stack->size - 1 - idx];
}
/*F}*/


/*F{*/
#include "../sp_errcodes.h"
#include <string.h>
int sp_stack_set(struct sp_stack *stack, size_t idx, void *val)
{
	char *p;
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR stack SP_EINVAL */
	/*. C_ERR_NULLPTR val SP_EINVAL */
	if (idx >= stack->size) {
		/*. C_ERRMSG_INDEX_OUT_OF_RANGE */
		return SP_EINDEX;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	memcpy(p, val, stack->elem_size);
	return 0;
}
/*F}*/

/*F{*/
#include "../sp_errcodes.h"
int sp_stack_set$SUFFIX$(struct sp_stack *stack, size_t idx, $TYPE$ val)
{
	char *p;
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR stack SP_EINVAL */
	/*. C_ERR_INCOMPAT_ELEM_TYPE stack sizeof(val) SP_EILLEGAL */
	if (idx >= stack->size) {
		/*. C_ERRMSG_INDEX_OUT_OF_RANGE */
		return SP_EINDEX;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	*($TYPE$*)p = val;
	return 0;
}
/*F}*/

/*F{*/
#include "../sp_errcodes.h"
int sp_stack_setb(struct sp_stack *stack, size_t idx, int val)
{
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR stack SP_EINVAL */
	/*. C_ERR_INCOMPAT_ELEM_TYPE stack SP_SIZEOF_BOOL SP_EILLEGAL */
	if (idx >= stack->size) {
		/*. C_ERRMSG_INDEX_OUT_OF_RANGE */
		return SP_EINDEX;
	}
#endif
	sp_boolbuf_set(stack->size - 1 - idx, val, stack->data);
	return 0;
}
/*F}*/

/*F{*/
#include "../sp_errcodes.h"
#include <string.h>
int sp_stack_setstr(struct sp_stack *stack, size_t idx, const char *val)
{
	char *p;
	char *buf;
	size_t len;
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR stack SP_EINVAL */
	/*. C_ERR_NULLPTR val SP_EINVAL */
	/*. C_ERR_INCOMPAT_ELEM_TYPE stack sizeof(val) SP_EILLEGAL */
	if (idx >= stack->size) {
		/*. C_ERRMSG_INDEX_OUT_OF_RANGE */
		return SP_EINDEX;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	free(*(char**)p);
	len = sp_strnlen(val, SP_SIZE_MAX);
	if (sp_size_try_add(len, 1))
		return SP_ERANGE;
	buf = malloc((len + 1) * sizeof(*val));
	if (buf == NULL) {
		/*. C_ERRMSG_MALLOC */
		return SP_ENOMEM;
	}
	memcpy(buf, val, len * sizeof(*val));
	buf[len] = '\0';
	*(char**)p = buf;
	return 0;
}
/*F}*/

/*F{*/
#include "../sp_errcodes.h"
#include <string.h>
int sp_stack_setstrn(struct sp_stack *stack, size_t idx, const char *val, size_t len)
{
	char *p;
	char *buf;
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR stack SP_EINVAL */
	/*. C_ERR_NULLPTR val SP_EINVAL */
	/*. C_ERR_INCOMPAT_ELEM_TYPE stack sizeof(val) SP_EILLEGAL */
	if (idx >= stack->size) {
		/*. C_ERRMSG_INDEX_OUT_OF_RANGE */
		return SP_EINDEX;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	free(*(char**)p);
	if (sp_size_try_add(len, 1))
		return SP_ERANGE;
	buf = malloc((len + 1) * sizeof(*val));
	if (buf == NULL) {
		/*. C_ERRMSG_MALLOC */
		return SP_ENOMEM;
	}
	memcpy(buf, val, len * sizeof(*val));
	buf[len] = '\0';
	*(char**)p = buf;
	return 0;
}
/*F}*/


/*F{*/
#include "../sp_errcodes.h"
int sp_stack_print(const struct sp_stack *stack, int (*func)(const void*))
{
	size_t i;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
#endif
	printf("sp_stack_print()\nsize/capacity: "SP_SIZE_FMT"/"SP_SIZE_FMT", elem_size: "SP_SIZE_FMT"\n",
		(SP_SIZE_T)stack->size, (SP_SIZE_T)stack->capacity, (SP_SIZE_T)stack->elem_size);
	if (func == NULL)
		for (i = stack->size; i-- > 0;) {
			const void *const elem = (char*)stack->data + i * stack->elem_size;
			printf("["SP_SIZE_FMT"]\t%p\n", (SP_SIZE_T)stack->size - 1 - i, elem);
		}
	else
		for (i = stack->size; i-- > 0;) {
			const void *const elem = (char*)stack->data + i * stack->elem_size;
			int err;
			printf("["SP_SIZE_FMT"]\t", (SP_SIZE_T)stack->size - 1 - i);
			if ((err = func(elem)) != 0) {
				/*. C_ERRMSG_CALLBACK_NON_ZERO func err */
				return SP_ECALLBK;
			}
		}
	return 0;
}
/*F}*/

/*F{*/
#include "../sp_errcodes.h"
int sp_stack_print$SUFFIX$(const struct sp_stack *stack)
{
	size_t i;
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR stack SP_EINVAL */
	/*. C_ERR_INCOMPAT_ELEM_TYPE stack sizeof($TYPE$) SP_EILLEGAL */
#endif
	printf("sp_stack_print$SUFFIX$()\nsize/capacity: "SP_SIZE_FMT"/"SP_SIZE_FMT", elem_size: "SP_SIZE_FMT"\n",
		(SP_SIZE_T)stack->size, (SP_SIZE_T)stack->capacity, (SP_SIZE_T)stack->elem_size);
	for (i = stack->size; i-- > 0;) {
		const $TYPE$ elem = (($TYPE$*)stack->data)[i];
		printf("["SP_SIZE_FMT"]\t"$FMT_STR$"\n", (SP_SIZE_T)stack->size - 1 - i, $FMT_ARGS$);
	}
	return 0;
}
/*F}*/

/*F{*/
#include "../sp_errcodes.h"
int sp_stack_printstr(const struct sp_stack *stack)
{
	size_t i;
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR stack SP_EINVAL */
	/*. C_ERR_INCOMPAT_ELEM_TYPE stack sizeof(char*) SP_EILLEGAL */
#endif
	printf("sp_stack_printstr()\nsize/capacity: "SP_SIZE_FMT"/"SP_SIZE_FMT", elem_size: "SP_SIZE_FMT"\n",
		(SP_SIZE_T)stack->size, (SP_SIZE_T)stack->capacity, (SP_SIZE_T)stack->elem_size);
	for (i = stack->size; i-- > 0;) {
		const char *elem = ((char**)stack->data)[i];
		printf("["SP_SIZE_FMT"]\t%s\n", (SP_SIZE_T)stack->size - 1 - i, elem);
	}
	return 0;
}
/*F}*/
