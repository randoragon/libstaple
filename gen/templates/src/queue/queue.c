#include "../sp_queue.h"
#include "../internal.h"

/*F{*/
struct sp_queue *sp_queue_create(size_t elem_size, size_t capacity)
{
	struct sp_queue *ret;

#ifdef STAPLE_DEBUG
	/*. C_ERR_ELEM_SIZE_ZERO */
	/*. C_ERR_CAPACITY_ZERO */
#endif
	if (capacity > SP_SIZE_MAX / elem_size) {
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
	ret->capacity  = capacity;
	ret->data      = malloc(capacity * elem_size);
	if (ret->data == NULL) {
		/*. C_ERRMSG_MALLOC */
		free(ret);
		return NULL;
	}
	ret->head = ret->tail = ret->data;

	return ret;
}
/*F}*/

/*F{*/
#include "../sp_errcodes.h"
int sp_queue_clear(struct sp_queue *queue, int (*dtor)(void*))
{
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR queue SP_EINVAL */
#endif
	if (dtor != NULL)
		while (queue->size != 0) {
			int err;
			if ((err = dtor(queue->head))) {
				/*. C_ERRMSG_HANDLER_NON_ZERO dtor err */
				return SP_EHANDLER;
			}
			sp_ringbuf_incr(&queue->head, queue->data, queue->capacity, queue->elem_size);
			queue->size--;
		}
	queue->size = 0;
	queue->head = queue->tail;
	return 0;
}
/*F}*/

/*F{*/
#include "../sp_errcodes.h"
int sp_queue_destroy(struct sp_queue *queue, int (*dtor)(void*))
{
	int error;
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR queue SP_EINVAL */
#endif
	if ((error = sp_queue_clear(queue, dtor)))
		return error;
	free(queue->data);
	free(queue);
	return 0;
}
/*F}*/

/*F{*/
#include "../sp_errcodes.h"
#include <string.h>
int sp_queue_copy(struct sp_queue *dest, const struct sp_queue *src, int (*cpy)(void*, const void*))
{
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
					/*. C_ERRMSG_HANDLER_NON_ZERO cpy err */
					return SP_EHANDLER;
				}
				--i;
			}
	}
	return 0;
}
/*F}*/

/*F{*/
#include "../sp_errcodes.h"
int sp_queue_foreach(struct sp_queue *queue, int (*func)(void*, size_t))
{
	size_t i;
	void *p;
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR queue SP_EINVAL */
	/*. C_ERR_NULLPTR func SP_EINVAL */
#endif
	p = queue->head;
	i = 0;
	while (i != queue->size) {
		int err;
		if ((err = func(p, i))) {
			/*. C_ERRMSG_HANDLER_NON_ZERO func err */
			return SP_EHANDLER;
		}
		sp_ringbuf_incr(&p, queue->data, queue->capacity, queue->elem_size);
		++i;
	}
	return 0;
}
/*F}*/

/*F{*/
#include "../sp_errcodes.h"
#include <string.h>
int sp_queue_push(struct sp_queue *queue, const void *elem)
{
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR queue SP_EINVAL */
	/*. C_ERR_NULLPTR elem SP_EINVAL */
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
/*F}*/

/*F{*/
#include "../sp_errcodes.h"
int sp_queue_push$SUFFIX$(struct sp_queue *queue, $TYPE$ elem)
{
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR queue SP_EINVAL */
	/*. C_ERR_INCOMPAT_ELEM_TYPE queue elem SP_EILLEGAL */
#endif
	if (sp_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return SP_ERANGE;
	if (sp_ringbuf_fit(&queue->data, queue->size, &queue->capacity, queue->elem_size, &queue->head, &queue->tail))
		return SP_ENOMEM;
	if (queue->size != 0)
		sp_ringbuf_incr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	*($TYPE$*)queue->tail = elem;
	++queue->size;
	return 0;
}
/*F}*/

/*F{*/
#include "../sp_errcodes.h"
#include <string.h>
int sp_queue_pushstr(struct sp_queue *queue, const char *elem)
{
	char *buf;
	size_t len;
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR queue SP_EINVAL */
	/*. C_ERR_NULLPTR elem SP_EINVAL */
	/*. C_ERR_INCOMPAT_ELEM_TYPE queue elem SP_EILLEGAL */
#endif
	if (sp_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return SP_ERANGE;
	if (sp_ringbuf_fit(&queue->data, queue->size, &queue->capacity, queue->elem_size, &queue->head, &queue->tail))
		return SP_ENOMEM;
	len = sp_strnlen(elem, SP_SIZE_MAX);
	if (sp_size_try_add(len, 1))
		return SP_ERANGE;
	if (queue->size != 0)
		sp_ringbuf_incr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	buf = malloc((len + 1) * sizeof(*elem));
	if (buf == NULL) {
		/*. C_ERRMSG_MALLOC */
		return SP_ENOMEM;
	}
	memcpy(buf, elem, len * sizeof(*elem));
	buf[len] = '\0';
	*(char**)queue->tail = buf;
	++queue->size;
	return 0;
}
/*F}*/

/*F{*/
#include "../sp_errcodes.h"
#include <string.h>
int sp_queue_pushstrn(struct sp_queue *queue, const char *elem, size_t len)
{
	char *buf;
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR queue SP_EINVAL */
	/*. C_ERR_NULLPTR elem SP_EINVAL */
	/*. C_ERR_INCOMPAT_ELEM_TYPE queue elem SP_EILLEGAL */
#endif
	if (sp_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return SP_ERANGE;
	if (sp_ringbuf_fit(&queue->data, queue->size, &queue->capacity, queue->elem_size, &queue->head, &queue->tail))
		return SP_ENOMEM;
	if (sp_size_try_add(len, 1))
		return SP_ERANGE;
	if (queue->size != 0)
		sp_ringbuf_incr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	buf = malloc((len + 1) * sizeof(*elem));
	if (buf == NULL) {
		/*. C_ERRMSG_MALLOC */
		return SP_ENOMEM;
	}
	memcpy(buf, elem, len * sizeof(*elem));
	buf[len] = '\0';
	*(char**)queue->tail = buf;
	++queue->size;
	return 0;
}
/*F}*/

/*F{*/
#include "../sp_errcodes.h"
#include <string.h>
int sp_queue_insert(struct sp_queue *queue, size_t idx, const void *elem)
{
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR queue SP_EINVAL */
	/*. C_ERR_NULLPTR elem SP_EINVAL */
	if (idx > queue->size) {
		/*. C_ERRMSG_INDEX_OUT_OF_RANGE */
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
/*F}*/

/*F{*/
#include "../sp_errcodes.h"
#include <string.h>
int sp_queue_insert$SUFFIX$(struct sp_queue *queue, size_t idx, $TYPE$ elem)
{
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR queue SP_EINVAL */
	/*. C_ERR_INCOMPAT_ELEM_TYPE queue elem SP_EILLEGAL */
	if (idx > queue->size) {
		/*. C_ERRMSG_INDEX_OUT_OF_RANGE */
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
/*F}*/

/*F{*/
#include "../sp_errcodes.h"
#include <string.h>
int sp_queue_insertstr(struct sp_queue *queue, size_t idx, const char *elem)
{
	char *buf;
	size_t len;
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR queue SP_EINVAL */
	/*. C_ERR_NULLPTR elem SP_EINVAL */
	/*. C_ERR_INCOMPAT_ELEM_TYPE queue elem SP_EILLEGAL */
	if (idx > queue->size) {
		/*. C_ERRMSG_INDEX_OUT_OF_RANGE */
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return SP_ERANGE;
	if (sp_ringbuf_fit(&queue->data, queue->size, &queue->capacity, queue->elem_size, &queue->head, &queue->tail))
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
	sp_ringbuf_insert(&buf, idx, queue->data, &queue->size, queue->capacity, queue->elem_size, &queue->head, &queue->tail);
	return 0;
}
/*F}*/

/*F{*/
#include "../sp_errcodes.h"
#include <string.h>
int sp_queue_insertstrn(struct sp_queue *queue, size_t idx, const char *elem, size_t len)
{
	char *buf;
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR queue SP_EINVAL */
	/*. C_ERR_NULLPTR elem SP_EINVAL */
	/*. C_ERR_INCOMPAT_ELEM_TYPE queue elem SP_EILLEGAL */
	if (idx > queue->size) {
		/*. C_ERRMSG_INDEX_OUT_OF_RANGE */
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return SP_ERANGE;
	if (sp_ringbuf_fit(&queue->data, queue->size, &queue->capacity, queue->elem_size, &queue->head, &queue->tail))
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
	sp_ringbuf_insert(&buf, idx, queue->data, &queue->size, queue->capacity, queue->elem_size, &queue->head, &queue->tail);
	return 0;
}
/*F}*/


/*F{*/
#include "../sp_errcodes.h"
#include <string.h>
int sp_queue_qinsert(struct sp_queue *queue, size_t idx, const void *elem)
{
	char *p;
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR queue SP_EINVAL */
	/*. C_ERR_NULLPTR elem SP_EINVAL */
	if (idx > queue->size) {
		/*. C_ERRMSG_INDEX_OUT_OF_RANGE */
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
/*F}*/

/*F{*/
#include "../sp_errcodes.h"
#include <string.h>
int sp_queue_qinsert$SUFFIX$(struct sp_queue *queue, size_t idx, $TYPE$ elem)
{
	char *p;
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR queue SP_EINVAL */
	/*. C_ERR_INCOMPAT_ELEM_TYPE queue elem SP_EILLEGAL */
	if (idx > queue->size) {
		/*. C_ERRMSG_INDEX_OUT_OF_RANGE */
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
	*($TYPE$*)queue->tail = *($TYPE$*)p;
	*($TYPE$*)p = elem;
	++queue->size;
	return 0;
}
/*F}*/

/*F{*/
#include "../sp_errcodes.h"
#include <string.h>
int sp_queue_qinsertstr(struct sp_queue *queue, size_t idx, const char *elem)
{
	char *p;
	char *buf;
	size_t len;
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR queue SP_EINVAL */
	/*. C_ERR_NULLPTR elem SP_EINVAL */
	/*. C_ERR_INCOMPAT_ELEM_TYPE queue elem SP_EILLEGAL */
	if (idx > queue->size) {
		/*. C_ERRMSG_INDEX_OUT_OF_RANGE */
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return SP_ERANGE;
	if (sp_ringbuf_fit(&queue->data, queue->size, &queue->capacity, queue->elem_size, &queue->head, &queue->tail))
		return SP_ENOMEM;
	len = sp_strnlen(elem, SP_SIZE_MAX);
	if (sp_size_try_add(len, 1))
		return SP_ERANGE;
	if (queue->size != 0)
		sp_ringbuf_incr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	buf = malloc((len + 1) * sizeof(*elem));
	if (buf == NULL) {
		/*. C_ERRMSG_MALLOC */
		return SP_ENOMEM;
	}
	memcpy(buf, elem, len * sizeof(*elem));
	buf[len] = '\0';
	p = sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	*(char**)queue->tail = *(char**)p;
	*(char**)p = buf;
	++queue->size;
	return 0;
}
/*F}*/

/*F{*/
#include "../sp_errcodes.h"
#include <string.h>
int sp_queue_qinsertstrn(struct sp_queue *queue, size_t idx, const char *elem, size_t len)
{
	char *p;
	char *buf;
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR queue SP_EINVAL */
	/*. C_ERR_NULLPTR elem SP_EINVAL */
	/*. C_ERR_INCOMPAT_ELEM_TYPE queue elem SP_EILLEGAL */
	if (idx > queue->size) {
		/*. C_ERRMSG_INDEX_OUT_OF_RANGE */
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(queue->size * queue->elem_size, queue->elem_size))
		return SP_ERANGE;
	if (sp_ringbuf_fit(&queue->data, queue->size, &queue->capacity, queue->elem_size, &queue->head, &queue->tail))
		return SP_ENOMEM;
	if (sp_size_try_add(len, 1))
		return SP_ERANGE;
	if (queue->size != 0)
		sp_ringbuf_incr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	buf = malloc((len + 1) * sizeof(*elem));
	if (buf == NULL) {
		/*. C_ERRMSG_MALLOC */
		return SP_ENOMEM;
	}
	memcpy(buf, elem, len * sizeof(*elem));
	buf[len] = '\0';
	p = sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	*(char**)queue->tail = *(char**)p;
	*(char**)p = buf;
	++queue->size;
	return 0;
}
/*F}*/


/*F{*/
void *sp_queue_peek(const struct sp_queue *queue)
{
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR queue NULL */
	if (queue->size == 0) {
		/*. C_ERRMSG_IS_EMPTY queue */
		return NULL;
	}
#endif
	return queue->head;
}
/*F}*/

/*F{*/
$TYPE$ sp_queue_peek$SUFFIX$(const struct sp_queue *queue)
{
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR queue 0 */
	/*. C_ERR_INCOMPAT_ELEM_TYPE queue $TYPE$ 0 */
	if (queue->size == 0) {
		/*. C_ERRMSG_IS_EMPTY queue */
		return 0;
	}
#endif
	return *($TYPE$*)queue->head;
}
/*F}*/

/*F{*/
char *sp_queue_peekstr(const struct sp_queue *queue)
{
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR queue NULL */
	/*. C_ERR_INCOMPAT_ELEM_TYPE queue char* NULL */
	if (queue->size == 0) {
		/*. C_ERRMSG_IS_EMPTY queue */
		return NULL;
	}
#endif
	return *(char**)queue->head;
}
/*F}*/


/*F{*/
#include "../sp_errcodes.h"
#include <string.h>
int sp_queue_pop(struct sp_queue *queue, void *output)
{
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR queue SP_EINVAL */
	if (queue->size == 0) {
		/*. C_ERRMSG_IS_EMPTY queue */
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
/*F}*/

/*F{*/
$TYPE$ sp_queue_pop$SUFFIX$(struct sp_queue *queue)
{
	$TYPE$ ret;
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR queue 0 */
	if (queue->size == 0) {
		/*. C_ERRMSG_IS_EMPTY queue */
		return 0;
	}
	/*. C_ERR_INCOMPAT_ELEM_TYPE queue $TYPE$ 0 */
#endif
	ret = *($TYPE$*)queue->head;
	if (queue->size != 1)
		sp_ringbuf_incr(&queue->head, queue->data, queue->capacity, queue->elem_size);
	--queue->size;
	return ret;
}
/*F}*/

/*F{*/
char *sp_queue_popstr(struct sp_queue *queue)
{
	char *ret;
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR queue NULL */
	/*. C_ERR_INCOMPAT_ELEM_TYPE queue char* NULL */
	if (queue->size == 0) {
		/*. C_ERRMSG_IS_EMPTY queue */
		return NULL;
	}
#endif
	ret = *(char**)queue->head;
	if (queue->size != 1)
		sp_ringbuf_incr(&queue->head, queue->data, queue->capacity, queue->elem_size);
	--queue->size;
	return ret;
}
/*F}*/


/*F{*/
#include "../sp_errcodes.h"
#include <string.h>
int sp_queue_remove(struct sp_queue *queue, size_t idx, void *output)
{
	char *p;
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR queue SP_EINVAL */
	if (idx >= queue->size) {
		/*. C_ERRMSG_INDEX_OUT_OF_RANGE */
		return SP_EINDEX;
	}
#endif
	p = sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	if (output != NULL)
		memcpy(output, p, queue->elem_size);
	sp_ringbuf_remove(idx, queue->data, &queue->size, queue->capacity, queue->elem_size, &queue->head, &queue->tail);
	return 0;
}
/*F}*/

/*F{*/
$TYPE$ sp_queue_remove$SUFFIX$(struct sp_queue *queue, size_t idx)
{
	$TYPE$ ret;
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR queue 0 */
	/*. C_ERR_INCOMPAT_ELEM_TYPE queue $TYPE$ 0 */
	if (idx >= queue->size) {
		/*. C_ERRMSG_INDEX_OUT_OF_RANGE */
		return 0;
	}
#endif
	ret = *($TYPE$*)sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	sp_ringbuf_remove(idx, queue->data, &queue->size, queue->capacity, queue->elem_size, &queue->head, &queue->tail);
	return ret;
}
/*F}*/

/*F{*/
char *sp_queue_removestr(struct sp_queue *queue, size_t idx)
{
	char *p;
	char *ret;
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR queue NULL */
	/*. C_ERR_INCOMPAT_ELEM_TYPE queue char* NULL */
	if (idx >= queue->size) {
		/*. C_ERRMSG_INDEX_OUT_OF_RANGE */
		return NULL;
	}
#endif
	p = sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	ret = *(char**)p;
	sp_ringbuf_remove(idx, queue->data, &queue->size, queue->capacity, queue->elem_size, &queue->head, &queue->tail);
	return ret;
}
/*F}*/


/*F{*/
#include "../sp_errcodes.h"
#include <string.h>
int sp_queue_qremove(struct sp_queue *queue, size_t idx, void *output)
{
	char *p;
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR queue SP_EINVAL */
	if (idx >= queue->size) {
		/*. C_ERRMSG_INDEX_OUT_OF_RANGE */
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
/*F}*/

/*F{*/
$TYPE$ sp_queue_qremove$SUFFIX$(struct sp_queue *queue, size_t idx)
{
	char *p;
	$TYPE$ ret;
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR queue 0 */
	/*. C_ERR_INCOMPAT_ELEM_TYPE queue $TYPE$ 0 */
	if (idx >= queue->size) {
		/*. C_ERRMSG_INDEX_OUT_OF_RANGE */
		return 0;
	}
#endif
	p = sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	ret = *($TYPE$*)p;
	*($TYPE$*)p = *($TYPE$*)queue->tail;
	if (queue->size != 1)
		sp_ringbuf_decr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	--queue->size;
	return ret;
}
/*F}*/

/*F{*/
char *sp_queue_qremovestr(struct sp_queue *queue, size_t idx)
{
	char *p;
	char *ret;
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR queue NULL */
	/*. C_ERR_INCOMPAT_ELEM_TYPE queue char* NULL */
	if (idx >= queue->size) {
		/*. C_ERRMSG_INDEX_OUT_OF_RANGE */
		return NULL;
	}
#endif
	p = sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	ret = *(char**)p;
	*(char**)p = *(char**)queue->tail;
	if (queue->size != 1)
		sp_ringbuf_decr(&queue->tail, queue->data, queue->capacity, queue->elem_size);
	--queue->size;
	return ret;
}
/*F}*/


/*F{*/
void *sp_queue_get(const struct sp_queue *queue, size_t idx)
{
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR queue NULL */
	if (idx >= queue->size) {
		/*. C_ERRMSG_INDEX_OUT_OF_RANGE */
		return NULL;
	}
#endif
	return sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
}
/*F}*/

/*F{*/
$TYPE$ sp_queue_get$SUFFIX$(const struct sp_queue *queue, size_t idx)
{
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR queue 0 */
	/*. C_ERR_INCOMPAT_ELEM_TYPE queue $TYPE$ 0 */
	if (idx >= queue->size) {
		/*. C_ERRMSG_INDEX_OUT_OF_RANGE */
		return 0;
	}
#endif
	return *($TYPE$*)sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
}
/*F}*/

/*F{*/
char *sp_queue_getstr(const struct sp_queue *queue, size_t idx)
{
	void *src;
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR queue NULL */
	/*. C_ERR_INCOMPAT_ELEM_TYPE queue char* NULL */
	if (idx >= queue->size) {
		/*. C_ERRMSG_INDEX_OUT_OF_RANGE */
		return NULL;
	}
#endif
	src = sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	return *(char**)src;
}
/*F}*/


/*F{*/
#include "../sp_errcodes.h"
#include <string.h>
int sp_queue_set(struct sp_queue *queue, size_t idx, void *val)
{
	char *p;
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR queue SP_EINVAL */
	/*. C_ERR_NULLPTR val SP_EINVAL */
	if (idx >= queue->size) {
		/*. C_ERRMSG_INDEX_OUT_OF_RANGE */
		return SP_EINDEX;
	}
#endif
	p = sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	memcpy(p, val, queue->elem_size);
	return 0;
}
/*F}*/

/*F{*/
#include "../sp_errcodes.h"
int sp_queue_set$SUFFIX$(struct sp_queue *queue, size_t idx, $TYPE$ val)
{
	char *p;
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR queue SP_EINVAL */
	/*. C_ERR_INCOMPAT_ELEM_TYPE queue val SP_EILLEGAL */
	if (idx >= queue->size) {
		/*. C_ERRMSG_INDEX_OUT_OF_RANGE */
		return SP_EINDEX;
	}
#endif
	p = sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
	*($TYPE$*)p = val;
	return 0;
}
/*F}*/

/*F{*/
#include "../sp_errcodes.h"
#include <string.h>
int sp_queue_setstr(struct sp_queue *queue, size_t idx, const char *val)
{
	char *p;
	char *buf;
	size_t len;
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR queue SP_EINVAL */
	/*. C_ERR_NULLPTR val SP_EINVAL */
	/*. C_ERR_INCOMPAT_ELEM_TYPE queue val SP_EILLEGAL */
	if (idx >= queue->size) {
		/*. C_ERRMSG_INDEX_OUT_OF_RANGE */
		return SP_EINDEX;
	}
#endif
	p = sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
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
int sp_queue_setstrn(struct sp_queue *queue, size_t idx, const char *val, size_t len)
{
	char *p;
	char *buf;
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR queue SP_EINVAL */
	/*. C_ERR_NULLPTR val SP_EINVAL */
	/*. C_ERR_INCOMPAT_ELEM_TYPE queue val SP_EILLEGAL */
	if (idx >= queue->size) {
		/*. C_ERRMSG_INDEX_OUT_OF_RANGE */
		return SP_EINDEX;
	}
#endif
	p = sp_ringbuf_get(idx, queue->data, queue->capacity, queue->elem_size, queue->head);
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
int sp_queue_print(const struct sp_queue *queue)
{
	size_t i;
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR queue SP_EINVAL */
#endif
	printf("sp_queue_print()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)queue->size, (unsigned long)queue->capacity, (unsigned long)queue->elem_size);
	for (i = 0; i < queue->size; i++) {
		const void *const elem = sp_ringbuf_get(i, queue->data, queue->capacity, queue->elem_size, queue->head);
		printf("[%lu]\t%p\n", (unsigned long)i, elem);
	}
	return 0;
}
/*F}*/

/*F{*/
#include "../sp_errcodes.h"
int sp_queue_print$SUFFIX$(const struct sp_queue *queue)
{
	size_t i;
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR queue SP_EINVAL */
	/*. C_ERR_INCOMPAT_ELEM_TYPE queue $TYPE$ SP_EILLEGAL */
#endif
	printf("sp_queue_print$SUFFIX$()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)queue->size, (unsigned long)queue->capacity, (unsigned long)queue->elem_size);
	for (i = 0; i < queue->size; i++) {
		const $TYPE$ elem = *($TYPE$*)sp_ringbuf_get(i, queue->data, queue->capacity, queue->elem_size, queue->head);
		printf("[%lu]\t"$FMT_STR$"\n", (unsigned long)i, $FMT_ARGS$);
	}
	return 0;
}
/*F}*/

/*F{*/
#include "../sp_errcodes.h"
int sp_queue_printstr(const struct sp_queue *queue)
{
	size_t i;
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR queue SP_EINVAL */
	/*. C_ERR_INCOMPAT_ELEM_TYPE queue char* SP_EILLEGAL */
#endif
	printf("sp_queue_printstr()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)queue->size, (unsigned long)queue->capacity, (unsigned long)queue->elem_size);
	for (i = 0; i < queue->size; i++) {
		const char *const elem = *(char**)sp_ringbuf_get(i, queue->data, queue->capacity, queue->elem_size, queue->head);
		printf("[%lu]\t%s\n", (unsigned long)i, elem);
	}
	return 0;
}
/*F}*/
