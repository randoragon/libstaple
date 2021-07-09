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

#ifdef RND_DEBUG
	#ifdef RND_DEBUG_KILL /* calls exit(1) after any debug message */
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
	#else
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
	#endif
#else
	#define warn(x) ;
	#define error(x) ;
#endif

void debug_printf(const char *fmt, ...);
int rnd_buffit(void **buf, size_t size, size_t *capacity);
int rnd_foomap(void *buf, size_t size, size_t elem_size, int (*foo)(void*));

#endif /* RND_HELPERS_H */
