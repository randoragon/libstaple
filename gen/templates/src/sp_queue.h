/*H{ STAPLE_QUEUE_H */
/* The queue module of the staple library. */
/*H}*/

#include <stdlib.h>
#include "sp_errcodes.h"
#include "sp_utils.h"
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
#include <stdint.h>
#endif

struct sp_queue {
	void *data;
	void *head;
	void *tail;
	size_t elem_size;
	size_t size;
	size_t capacity;
};

struct sp_queue *sp_queue_create(size_t elem_size, size_t capacity);
int              sp_queue_clear(struct sp_queue *queue, int (*dtor)(void*));
int              sp_queue_destroy(struct sp_queue *queue, int (*dtor)(void*));
int              sp_queue_eq(const struct sp_queue *queue1, const struct sp_queue *queue2, int (*cmp)(const void*, const void*));
int              sp_queue_copy(struct sp_queue *dest, const struct sp_queue *src, int (*cpy)(void*, const void*));
int              sp_queue_map(struct sp_queue *queue, int (*func)(void*, size_t));

int sp_queue_push(struct sp_queue *queue, const void *elem);
int sp_queue_push$SUFFIX$(struct sp_queue *queue, $TYPE$ elem);
int sp_queue_pushstr(struct sp_queue *queue, const char *elem);
int sp_queue_pushstrn(struct sp_queue *queue, const char *elem, size_t len);

int sp_queue_insert(struct sp_queue *queue, size_t idx, const void *elem);
int sp_queue_insert$SUFFIX$(struct sp_queue *queue, size_t idx, $TYPE$ elem);
int sp_queue_insertstr(struct sp_queue *queue, size_t idx, const char *elem);
int sp_queue_insertstrn(struct sp_queue *queue, size_t idx, const char *elem, size_t len);

int sp_queue_qinsert(struct sp_queue *queue, size_t idx, const void *elem);
int sp_queue_qinsert$SUFFIX$(struct sp_queue *queue, size_t idx, $TYPE$ elem);
int sp_queue_qinsertstr(struct sp_queue *queue, size_t idx, const char *elem);
int sp_queue_qinsertstrn(struct sp_queue *queue, size_t idx, const char *elem, size_t len);

void          *sp_queue_peek(const struct sp_queue *queue);
$TYPE$         sp_queue_peek$SUFFIX$(const struct sp_queue *queue);
char          *sp_queue_peekstr(const struct sp_queue *queue);

int            sp_queue_pop(struct sp_queue *queue, int (*dtor)(void*));
$TYPE$         sp_queue_pop$SUFFIX$(struct sp_queue *queue);
char          *sp_queue_popstr(struct sp_queue *queue);

int            sp_queue_remove(struct sp_queue *queue, size_t idx, int (*dtor)(void*));
$TYPE$         sp_queue_remove$SUFFIX$(struct sp_queue *queue, size_t idx);
char          *sp_queue_removestr(struct sp_queue *queue, size_t idx);

int            sp_queue_qremove(struct sp_queue *queue, size_t idx, int (*dtor)(void*));
$TYPE$         sp_queue_qremove$SUFFIX$(struct sp_queue *queue, size_t idx);
char          *sp_queue_qremovestr(struct sp_queue *queue, size_t idx);

void          *sp_queue_get(const struct sp_queue *queue, size_t idx);
$TYPE$         sp_queue_get$SUFFIX$(const struct sp_queue *queue, size_t idx);
char          *sp_queue_getstr(const struct sp_queue *queue, size_t idx);

int sp_queue_set(struct sp_queue *queue, size_t idx, void *val);
int sp_queue_set$SUFFIX$(struct sp_queue *queue, size_t idx, $TYPE$ val);
int sp_queue_setstr(struct sp_queue *queue, size_t idx, const char *val);
int sp_queue_setstrn(struct sp_queue *queue, size_t idx, const char *val, size_t len);

int sp_queue_print(const struct sp_queue *queue, int (*func)(const void*));
int sp_queue_print$SUFFIX$(const struct sp_queue *queue);
int sp_queue_printstr(const struct sp_queue *queue);
