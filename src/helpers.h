/* This header includes supplementary functions and macros for things like
 * message logging and reporting errors, which are consistently used throughout
 * other source files. This file should be hidden from the library user.
 */
#ifndef STAPLE_HELPERS_H
#define STAPLE_HELPERS_H

#include <stdlib.h>
#include <stdio.h>

#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))
#ifndef SIZE_MAX
#define SIZE_MAX ((size_t)(-1))
#endif

/* STAPLE_DEBUG enables extra debug checks during runtime.
 * STAPLE_QUIET disables all output from the library (if STAPLE_DEBUG is enabled, each
 *           check will produce the appropriate error code, only silently)
 * STAPLE_ABORT causes every warning or error to call exit(1). This can be
 *               useful as implicit error-handling in small simple programs.
 */
#if !defined(STAPLE_QUIET) && !defined(STAPLE_ABORT)
	#define warn(x) do { \
		fprintf(stderr, "%s:%u: warning: ", __FILE__, __LINE__); \
		stderr_printf x; \
		fputc('\n', stderr); \
	} while (0)
	#define error(x) do { \
		fprintf(stderr, "%s:%u: error: ", __FILE__, __LINE__); \
		stderr_printf x; \
		fputc('\n', stderr); \
	} while (0)
#elif !defined(STAPLE_QUIET) && defined(STAPLE_ABORT)
	#define warn(x) do { \
		fprintf(stderr, "%s:%u: warning: ", __FILE__, __LINE__); \
		stderr_printf x; \
		fputc('\n', stderr); \
		exit(1); \
	} while (0)
	#define error(x) do { \
		fprintf(stderr, "%s:%u: error: ", __FILE__, __LINE__); \
		stderr_printf x; \
		fputc('\n', stderr); \
		exit(1); \
	} while (0)
#elif defined(STAPLE_QUIET) && !defined(STAPLE_ABORT)
	#define warn(x)  ;
	#define error(x) ;
#else
	#define warn(x)  exit(1)
	#define error(x) exit(1)
#endif

void stderr_printf(const char *fmt, ...);
int sp_buf_fit(void **buf, size_t size, size_t *capacity, size_t elem_size);
int sp_ringbuf_fit(void **buf, size_t size, size_t *capacity, size_t elem_size, void **head, void **tail);
int sp_foomap(void *buf, size_t size, size_t elem_size, int (*foo)(void*));
int sp_size_try_add(size_t size, size_t amount);
void  sp_ringbuf_incr(void **ptr, void *buf, size_t capacity, size_t elem_size);
void  sp_ringbuf_decr(void **ptr, void *buf, size_t capacity, size_t elem_size);
void *sp_ringbuf_get(size_t idx, const void *buf, size_t capacity, size_t elem_size, const void *head);
void  sp_ringbuf_insert(const void *elem, size_t idx, void *buf, size_t *size, size_t capacity, size_t elem_size, void **head, void **tail);
void  sp_ringbuf_remove(size_t idx, void *buf, size_t *size, size_t capacity, size_t elem_size, void **head, void **tail);

#endif /* STAPLE_HELPERS_H */
