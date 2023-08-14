/*H{ STAPLE_INTERNAL_H */
/* This header includes supplementary functions and macros for things like
 * message logging and reporting errors, which are consistently used throughout
 * other source files. This file should be hidden from the library user.
 */
/*H}*/

#include "sp_utils.h"
#include <stdlib.h>
#include <stdio.h>

#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))
#ifndef SP_SIZE_MAX
#define SP_SIZE_MAX ((size_t)(-1))
#endif

/* Computes the minimum number of bytes necessary to store X bits. */
#define BITS_TO_BYTES(X) ((X) / SP_BYTE_SIZE + ((X) % SP_BYTE_SIZE ? 1 : 0))
/* Rounds a number of bits up to the nearest full byte. */
#define ROUND_UP_TO_BYTE(X) ((X) + ((X) % SP_BYTE_SIZE ? SP_BYTE_SIZE - (X) % SP_BYTE_SIZE : 0))

/* These macros are only used for printf format strings and casts */
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
#define SP_SIZE_FMT "%zu"
#define SP_SIZE_T size_t
#else
#define SP_SIZE_FMT "%lu"
#define SP_SIZE_T unsigned long
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

/* Misc functions */
size_t sp_strnlen(const char *s, size_t maxlen);
void   stderr_printf(const char *fmt, ...);
int    sp_foomap(void *buf, size_t size, size_t elem_size, int (*foo)(void*));
int    sp_size_try_add(size_t size, size_t amount);
int    sp_buf_fit(void **buf, size_t size, size_t *capacity, size_t elem_size);

/* Boolean buffers */
int  sp_boolbuf_fit(void **buf, size_t size, size_t *capacity);
int  sp_boolbuf_get(size_t idx, const void *buf);
void sp_boolbuf_set(size_t idx, int val, void *buf);

/* Ring buffers */
int   sp_ringbuf_fit(void **buf, size_t size, size_t *capacity, size_t elem_size, void **head, void **tail);
void  sp_ringbuf_incr(void **ptr, const void *buf, size_t capacity, size_t elem_size);
void  sp_ringbuf_decr(void **ptr, const void *buf, size_t capacity, size_t elem_size);
void *sp_ringbuf_get(size_t idx, const void *buf, size_t capacity, size_t elem_size, const void *head);
void  sp_ringbuf_insert(const void *elem, size_t idx, void *buf, size_t *size, size_t capacity, size_t elem_size, void **head, void **tail);
void  sp_ringbuf_remove(size_t idx, void *buf, size_t *size, size_t capacity, size_t elem_size, void **head, void **tail);
