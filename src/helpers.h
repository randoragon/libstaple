/* This header includes supplementary functions and macros for things like
 * message logging and reporting errors, which are consistently used throughout
 * other source files. This file should be hidden from a library user.
 */
#ifndef RND_HELPERS_H
#define RND_HELPERS_H

#include <stdlib.h>
#include <stdio.h>

#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))

/* RND_DEBUG enables extra debug checks during runtime.
 * RND_QUIET disables all output from the library (if RND_DEBUG is enabled, each
 *           check will produce the appropriate error code, only silently)
 * RND_AUTOABORT causes every warning or error to call exit(1). This can be
 *               useful as implicit error-handling in small simple programs.
 */
#if !defined(RND_QUIET) && !defined(RND_AUTOABORT)
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
#elif !defined(RND_QUIET) && defined(RND_AUTOABORT)
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
#elif defined(RND_QUIET) && !defined(RND_AUTOABORT)
	#define warn(x)  ;
	#define error(x) ;
#else
	#define warn(x)  exit(1)
	#define error(x) exit(1)
#endif

void stderr_printf(const char *fmt, ...);
int rnd_buffit(void **buf, size_t elem_size, size_t size, size_t *capacity);
int rnd_foomap(void *buf, size_t size, size_t elem_size, int (*foo)(void*));
int rnd_size_try_add(size_t size, size_t amount);
int rnd_size_try_sub(size_t size, size_t amount);

#endif /* RND_HELPERS_H */
