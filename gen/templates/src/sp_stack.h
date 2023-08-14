/*H{ STAPLE_STACK_H */
/* The stack module of the staple library. */
/*H}*/

#include <stdlib.h>
#include "sp_errcodes.h"
#include "sp_utils.h"
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
#include <stdint.h>
#endif

struct sp_stack {
	void  *data;
	size_t elem_size;
	size_t size;
	size_t capacity;
};

struct sp_stack *sp_stack_create(size_t elem_size, size_t capacity);
int              sp_stack_clear(struct sp_stack *stack, int (*dtor)(void*));
int              sp_stack_destroy(struct sp_stack *stack, int (*dtor)(void*));
int              sp_stack_eq(const struct sp_stack *stack1, const struct sp_stack *stack2, int (*cmp)(const void*, const void*));
int              sp_stack_copy(struct sp_stack *dest, const struct sp_stack *src, int (*cpy)(void*, const void*));
int              sp_stack_map(struct sp_stack *stack, int (*func)(void*, size_t));

int sp_stack_push(struct sp_stack *stack, const void *elem);
int sp_stack_push$SUFFIX$(struct sp_stack *stack, $TYPE$ elem);
int sp_stack_pushb(struct sp_stack *stack, int elem);
int sp_stack_pushstr(struct sp_stack *stack, const char *elem);
int sp_stack_pushstrn(struct sp_stack *stack, const char *elem, size_t len);

int sp_stack_insert(struct sp_stack *stack, size_t idx, const void *elem);
int sp_stack_insert$SUFFIX$(struct sp_stack *stack, size_t idx, $TYPE$ elem);
int sp_stack_insertstr(struct sp_stack *stack, size_t idx, const char *elem);
int sp_stack_insertstrn(struct sp_stack *stack, size_t idx, const char *elem, size_t len);

int sp_stack_qinsert(struct sp_stack *stack, size_t idx, const void *elem);
int sp_stack_qinsert$SUFFIX$(struct sp_stack *stack, size_t idx, $TYPE$ elem);
int sp_stack_qinsertstr(struct sp_stack *stack, size_t idx, const char *elem);
int sp_stack_qinsertstrn(struct sp_stack *stack, size_t idx, const char *elem, size_t len);

void   *sp_stack_peek(const struct sp_stack *stack);
$TYPE$  sp_stack_peek$SUFFIX$(const struct sp_stack *stack);
int     sp_stack_peekb(const struct sp_stack *stack);
char   *sp_stack_peekstr(const struct sp_stack *stack);

int     sp_stack_pop(struct sp_stack *stack, int (*dtor)(void*));
$TYPE$  sp_stack_pop$SUFFIX$(struct sp_stack *stack);
char   *sp_stack_popstr(struct sp_stack *stack);

int     sp_stack_remove(struct sp_stack *stack, size_t idx, int (*dtor)(void*));
$TYPE$  sp_stack_remove$SUFFIX$(struct sp_stack *stack, size_t idx);
char   *sp_stack_removestr(struct sp_stack *stack, size_t idx);

int     sp_stack_qremove(struct sp_stack *stack, size_t idx, int (*dtor)(void*));
$TYPE$  sp_stack_qremove$SUFFIX$(struct sp_stack *stack, size_t idx);
char   *sp_stack_qremovestr(struct sp_stack *stack, size_t idx);

void   *sp_stack_get(const struct sp_stack *stack, size_t idx);
$TYPE$  sp_stack_get$SUFFIX$(const struct sp_stack *stack, size_t idx);
int     sp_stack_getb(const struct sp_stack *stack, size_t idx);
char   *sp_stack_getstr(const struct sp_stack *stack, size_t idx);

int sp_stack_set(struct sp_stack *stack, size_t idx, void *val);
int sp_stack_set$SUFFIX$(struct sp_stack *stack, size_t idx, $TYPE$ val);
int sp_stack_setstr(struct sp_stack *stack, size_t idx, const char *val);
int sp_stack_setstrn(struct sp_stack *stack, size_t idx, const char *val, size_t len);

int sp_stack_print(const struct sp_stack *stack, int (*func)(const void*));
int sp_stack_print$SUFFIX$(const struct sp_stack *stack);
int sp_stack_printstr(const struct sp_stack *stack);
