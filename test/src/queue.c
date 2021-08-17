#include <stdlib.h>
#include <time.h>
#include "../../src/rnd.h"
#include "test_struct.h"
#include <criterion/criterion.h>
#include <limits.h>
#include <float.h>
#include <string.h>

/* Make testing for size overflow feasible */
#ifdef SIZE_MAX
#undef SIZE_MAX
#endif
#define SIZE_MAX 65535LU

Test(queue, create)
{
	struct rnd_queue *q;
	q = rnd_queue_create(sizeof(int), 0);
	cr_assert_null(q);
	q = rnd_queue_create(0, 16);
	cr_assert_null(q);
	q = rnd_queue_create(0, 0);
	cr_assert_null(q);
	q = rnd_queue_create(SIZE_MAX, SIZE_MAX);
	cr_assert_null(q);
	q = rnd_queue_create(sizeof(int), 16);
	cr_assert_not_null(q);
	cr_assert_eq(0, rnd_queue_destroy(q, NULL));
}

Test(queue, destroy)
{
	struct rnd_queue *q;
	unsigned i;
	q = rnd_queue_create(sizeof(long double), 1000);
	cr_assert_not_null(q);
	cr_assert_eq(RND_EINVAL, rnd_queue_destroy(NULL, NULL));
	cr_assert_eq(0, rnd_queue_destroy(q, NULL));
	q = rnd_queue_create(sizeof(struct data), 1000);
	for (i = 0; i < 1000; i++) {
		struct data d;
		cr_assert_eq(0, data_init(&d));
		cr_assert_eq(0, rnd_queue_push(q, &d));
	}
	cr_assert_eq(RND_EHANDLER, rnd_queue_destroy(q, data_dtor_bad));
	cr_assert_eq(0, rnd_queue_destroy(q, data_dtor));
}

Test(queue, push)
{
	struct rnd_queue *q;

	{ /* Generic form */
		unsigned i;
		struct data d;
		q = rnd_queue_create(sizeof(struct data), 1000);
		cr_assert_not_null(q);
		data_init(&d);
		cr_assert_eq(RND_EINVAL, rnd_queue_push(q, NULL));
		cr_assert_eq(RND_EINVAL, rnd_queue_push(NULL, &d));
		cr_assert_eq(RND_EINVAL, rnd_queue_push(NULL, NULL));
		cr_assert_eq(q->head, q->data, "%p");
		cr_assert_eq(q->tail, q->data, "%p");
		cr_assert_eq(0, rnd_queue_push(q, &d));
		cr_assert_eq(q->head, q->data, "%p");
		cr_assert_eq(q->tail, q->data, "%p");
		cr_assert_eq(0, rnd_queue_push(q, &d));
		cr_assert_eq(q->head, q->data, "%p");
		cr_assert_eq(q->tail, (char*)q->data + q->elem_size, "%p");
		cr_assert_eq(0, rnd_queue_clear(q, NULL));
		for (i = 0; i < SIZE_MAX / sizeof(struct data); i++) {
			struct data a, b;
			cr_assert_eq(0, data_init(&a));
			cr_assert_eq(0, rnd_queue_push(q, &a));
			cr_assert_eq(0, rnd_queue_get(q, i, &b));
			cr_assert_eq(0, data_cmp(&a, &b));
		}
		cr_assert_eq(RND_ERANGE, rnd_queue_push(q, &d));
		cr_assert_eq(0, data_dtor(&d));
		cr_assert_eq(0, rnd_queue_destroy(q, data_dtor));
	}

	/* Suffixed form
	 * T  - type
	 * F1 - push function
	 * F2 - get function
	 * V  - random value snippet
	 * M  - printf format string
	 */
#define test(T, F1, F2, V, M) do {                                          \
		unsigned i;                                                 \
		q = rnd_queue_create(sizeof(T), 1000);                      \
		cr_assert_not_null(q);                                      \
		cr_assert_eq(RND_EINVAL, F1(NULL, (V)));                    \
		cr_assert_eq(q->head, q->data, "%p");                       \
		cr_assert_eq(q->tail, q->data, "%p");                       \
		cr_assert_eq(0, F1(q, (V)));                                \
		cr_assert_eq(q->head, q->data, "%p");                       \
		cr_assert_eq(q->tail, q->data, "%p");                       \
		cr_assert_eq(0, F1(q, (V)));                                \
		cr_assert_eq(q->head, q->data, "%p");                       \
		cr_assert_eq(q->tail, (char*)q->data + q->elem_size, "%p"); \
		cr_assert_eq(0, rnd_queue_clear(q, NULL));                  \
		for (i = 0; i < SIZE_MAX / sizeof(T); i++) {                \
			T a = (V);                                          \
			cr_assert_eq(0, F1(q, a));                          \
			cr_assert_eq(a, F2(q, i), M);                       \
		}                                                           \
		cr_assert_eq(RND_ERANGE, F1(q, (V)));                       \
		cr_assert_eq(0, rnd_queue_destroy(q, NULL));                \
		q = rnd_queue_create(sizeof(T) + 1, 1);                     \
		cr_assert_not_null(q);                                      \
		cr_assert_eq(RND_EILLEGAL, F1(q, (V)));                     \
		cr_assert_eq(0, rnd_queue_destroy(q, NULL));                \
	} while (0)
	test(char          , rnd_queue_pushc , rnd_queue_getc , IRANGE(CHAR_MIN , CHAR_MAX) , "%hd");
	test(short         , rnd_queue_pushs , rnd_queue_gets , IRANGE(SHRT_MIN , SHRT_MAX) , "%hd");
	test(int           , rnd_queue_pushi , rnd_queue_geti , FRANGE(INT_MIN  , INT_MAX)  , "%d");
	test(long          , rnd_queue_pushl , rnd_queue_getl , FRANGE(LONG_MIN , LONG_MAX) , "%ld");
	test(signed char   , rnd_queue_pushsc, rnd_queue_getsc, IRANGE(SCHAR_MIN, SCHAR_MAX), "%hd");
	test(unsigned char , rnd_queue_pushuc, rnd_queue_getuc, IRANGE(0        , UCHAR_MAX), "%hd");
	test(unsigned short, rnd_queue_pushus, rnd_queue_getus, IRANGE(0        , USHRT_MAX), "%hu");
	test(unsigned int  , rnd_queue_pushui, rnd_queue_getui, FRANGE(0        , UINT_MAX) , "%u");
	test(unsigned long , rnd_queue_pushul, rnd_queue_getul, FRANGE(0        , ULONG_MAX), "%lu");
	test(float         , rnd_queue_pushf , rnd_queue_getf , FRANGE(FLT_MIN  , FLT_MAX)  , "%f");
	test(double        , rnd_queue_pushd , rnd_queue_getd , FRANGE(DBL_MIN  , DBL_MAX)  , "%f");
	test(long double   , rnd_queue_pushld, rnd_queue_getld, FRANGE(LDBL_MIN , LDBL_MAX) , "%Lf");
#undef test
}

Test(queue, peek)
{
	struct rnd_queue *q;

	{ /* Generic form */
		struct data a, b;
		q = rnd_queue_create(sizeof(struct data), 2);
		cr_assert_not_null(q);
		cr_assert_eq(RND_EILLEGAL, rnd_queue_peek(q, &b));
		cr_assert_eq(RND_EINVAL, rnd_queue_peek(q, NULL));
		cr_assert_eq(RND_EINVAL, rnd_queue_peek(NULL, &b));
		cr_assert_eq(RND_EINVAL, rnd_queue_peek(NULL, NULL));
		data_init(&a);
		cr_assert_eq(0, rnd_queue_push(q, &a));
		cr_assert_eq(0, rnd_queue_peek(q, &b));
		cr_assert_eq(0, data_cmp(&a, &b));
		cr_assert_eq(0, rnd_queue_destroy(q, data_dtor));
	}

	/* Suffixed form
	 * T  - type
	 * F1 - peek function
	 * F2 - push function
	 * V  - random value snippet
	 * M  - printf format string
	 */
#define test(T, F1, F2, V, M) do {                           \
		T a = (V), z = 0;                            \
		q = rnd_queue_create(sizeof(T), 2);          \
		cr_assert_not_null(q);                       \
		cr_assert_eq(z, F1(q), M);                   \
		cr_assert_eq(z, F1(NULL), M);                \
		cr_assert_eq(0, F2(q, a));                   \
		cr_assert_eq(a, F1(q), M);                   \
		cr_assert_eq(0, rnd_queue_destroy(q, NULL)); \
		q = rnd_queue_create(sizeof(T) + 1, 1);      \
		cr_assert_not_null(q);                       \
		q->size = 1;                                 \
		cr_assert_eq(z, F1(q), M);                   \
		cr_assert_eq(0, rnd_queue_destroy(q, NULL)); \
	} while (0)
	test(char          , rnd_queue_peekc , rnd_queue_pushc , IRANGE(1, CHAR_MAX) , "%hd");
	test(short         , rnd_queue_peeks , rnd_queue_pushs , IRANGE(1, SHRT_MAX) , "%hd");
	test(int           , rnd_queue_peeki , rnd_queue_pushi , FRANGE(1, INT_MAX)  , "%d");
	test(long          , rnd_queue_peekl , rnd_queue_pushl , FRANGE(1, LONG_MAX) , "%ld");
	test(signed char   , rnd_queue_peeksc, rnd_queue_pushsc, IRANGE(1, SCHAR_MAX), "%hd");
	test(unsigned char , rnd_queue_peekuc, rnd_queue_pushuc, IRANGE(1, UCHAR_MAX), "%hd");
	test(unsigned short, rnd_queue_peekus, rnd_queue_pushus, IRANGE(1, USHRT_MAX), "%hu");
	test(unsigned int  , rnd_queue_peekui, rnd_queue_pushui, FRANGE(1, UINT_MAX) , "%u");
	test(unsigned long , rnd_queue_peekul, rnd_queue_pushul, FRANGE(1, ULONG_MAX), "%lu");
	test(float         , rnd_queue_peekf , rnd_queue_pushf , FRANGE(1, FLT_MAX)  , "%f");
	test(double        , rnd_queue_peekd , rnd_queue_pushd , FRANGE(1, DBL_MAX)  , "%f");
	test(long double   , rnd_queue_peekld, rnd_queue_pushld, FRANGE(1, LDBL_MAX) , "%Lf");
#undef test
}

Test(queue, pop)
{
	struct rnd_queue *q;

	{ /* Generic form */
		struct data a, b;
		q = rnd_queue_create(sizeof(struct data), 2);
		cr_assert_not_null(q);
		cr_assert_eq(RND_EILLEGAL, rnd_queue_pop(q, NULL));
		cr_assert_eq(RND_EINVAL, rnd_queue_pop(NULL, NULL));
		data_init(&a);
		cr_assert_eq(0, rnd_queue_push(q, &a));
		cr_assert_eq(q->head, q->data, "%p");
		cr_assert_eq(q->tail, q->data, "%p");
		cr_assert_eq(0, rnd_queue_pop(q, &b));
		cr_assert_eq(q->head, q->data, "%p");
		cr_assert_eq(q->tail, q->data, "%p");
		cr_assert_eq(0, data_cmp(&a, &b));
		cr_assert_eq(0, rnd_queue_destroy(q, NULL));
		cr_assert_eq(0, data_dtor(&b));
	}

	/* Suffixed form
	 * T  - type
	 * F1 - pop function
	 * F2 - push function
	 * V  - random value snippet
	 * M  - printf format string
	 */
#define test(T, F1, F2, V, M) do {                           \
		T a = (V), z = 0;                            \
		q = rnd_queue_create(sizeof(T), 2);          \
		cr_assert_not_null(q);                       \
		cr_assert_eq(z, F1(q), M);                   \
		cr_assert_eq(z, F1(NULL), M);                \
		cr_assert_eq(0, F2(q, a));                   \
		cr_assert_eq(q->head, q->data, "%p");        \
		cr_assert_eq(q->tail, q->data, "%p");        \
		cr_assert_eq(a, F1(q), M);                   \
		cr_assert_eq(q->head, q->data, "%p");        \
		cr_assert_eq(q->tail, q->data, "%p");        \
		cr_assert_eq(0, rnd_queue_destroy(q, NULL)); \
		q = rnd_queue_create(sizeof(T) + 1, 1);      \
		cr_assert_not_null(q);                       \
		q->size = 1;                                 \
		cr_assert_eq(z, F1(q), M);                   \
		cr_assert_eq(0, rnd_queue_destroy(q, NULL)); \
	} while (0)
	test(char          , rnd_queue_popc , rnd_queue_pushc , IRANGE(1, CHAR_MAX) , "%hd");
	test(short         , rnd_queue_pops , rnd_queue_pushs , IRANGE(1, SHRT_MAX) , "%hd");
	test(int           , rnd_queue_popi , rnd_queue_pushi , FRANGE(1, INT_MAX)  , "%d");
	test(long          , rnd_queue_popl , rnd_queue_pushl , FRANGE(1, LONG_MAX) , "%ld");
	test(signed char   , rnd_queue_popsc, rnd_queue_pushsc, IRANGE(1, SCHAR_MAX), "%hd");
	test(unsigned char , rnd_queue_popuc, rnd_queue_pushuc, IRANGE(1, UCHAR_MAX), "%hd");
	test(unsigned short, rnd_queue_popus, rnd_queue_pushus, IRANGE(1, USHRT_MAX), "%hu");
	test(unsigned int  , rnd_queue_popui, rnd_queue_pushui, FRANGE(1, UINT_MAX) , "%u");
	test(unsigned long , rnd_queue_popul, rnd_queue_pushul, FRANGE(1, ULONG_MAX), "%lu");
	test(float         , rnd_queue_popf , rnd_queue_pushf , FRANGE(1, FLT_MAX)  , "%f");
	test(double        , rnd_queue_popd , rnd_queue_pushd , FRANGE(1, DBL_MAX)  , "%f");
	test(long double   , rnd_queue_popld, rnd_queue_pushld, FRANGE(1, LDBL_MAX) , "%Lf");
#undef test
}

Test(queue, clear)
{
	struct rnd_queue *q;
	unsigned i;
	q = rnd_queue_create(sizeof(long double), 1000);
	cr_assert_not_null(q);
	cr_assert_eq(RND_EINVAL, rnd_queue_clear(NULL, NULL));
	cr_assert_eq(0, rnd_queue_clear(q, NULL));
	cr_assert_eq(0LU, (unsigned long)q->size, "%lu");
	cr_assert_eq(0, rnd_queue_destroy(q, NULL));
	q = rnd_queue_create(sizeof(struct data), 1000);
	for (i = 0; i < 1000; i++) {
		struct data d;
		cr_assert_eq(0, data_init(&d));
		cr_assert_eq(0, rnd_queue_push(q, &d));
	}
	cr_assert_eq(RND_EHANDLER, rnd_queue_clear(q, data_dtor_bad));
	cr_assert_eq(0, rnd_queue_clear(q, data_dtor));
	cr_assert_eq(0LU, (unsigned long)q->size, "%lu");
	cr_assert_eq(0, rnd_queue_destroy(q, NULL));
}

Test(queue, foreach)
{
	struct rnd_queue *q;
	unsigned i;
	q = rnd_queue_create(sizeof(long double), 1000);
	cr_assert_not_null(q);
	cr_assert_eq(RND_EINVAL, rnd_queue_foreach(NULL, data_mutate));
	cr_assert_eq(RND_EINVAL, rnd_queue_foreach(q, NULL));
	cr_assert_eq(RND_EINVAL, rnd_queue_foreach(NULL, NULL));
	cr_assert_eq(0, rnd_queue_destroy(q, NULL));
	q = rnd_queue_create(sizeof(struct data), 1000);
	for (i = 0; i < 1000; i++) {
		struct data d;
		cr_assert_eq(0, data_init(&d));
		cr_assert_eq(0, rnd_queue_push(q, &d));
	}
	cr_assert_eq(RND_EHANDLER, rnd_queue_foreach(q, data_mutate_bad));
	cr_assert_eq(0, rnd_queue_foreach(q, data_mutate));
	cr_assert_eq(0, rnd_queue_foreach(q, data_verify));
	cr_assert_eq(0, rnd_queue_destroy(q, data_dtor));
}

Test(queue, copy)
{
	struct rnd_queue *q, *p;
	unsigned i;
	q = rnd_queue_create(sizeof(int), 1000);
	p = rnd_queue_create(sizeof(int), 333);
	cr_assert_not_null(q);
	cr_assert_eq(RND_EINVAL, rnd_queue_copy(NULL, q, NULL));
	cr_assert_eq(RND_EINVAL, rnd_queue_copy(p, NULL, NULL));
	cr_assert_eq(RND_EINVAL, rnd_queue_copy(NULL, NULL, NULL));
	cr_assert_eq(0, rnd_queue_copy(q, p, NULL));
	cr_assert_eq((unsigned long)p->size, (unsigned long)q->size, "%lu");
	cr_assert_eq((unsigned long)p->elem_size, (unsigned long)q->elem_size, "%lu");
	cr_assert_eq(0, rnd_queue_copy(p, q, NULL));
	cr_assert_eq((unsigned long)p->size, (unsigned long)q->size, "%lu");
	cr_assert_eq((unsigned long)p->elem_size, (unsigned long)q->elem_size, "%lu");
	for (i = 0; i < 1000; i++) {
		cr_assert_eq(0, rnd_queue_pushi(q, FRANGE(INT_MIN, INT_MAX)));
	}
	cr_assert_eq(0, rnd_queue_copy(p, q, NULL));
	cr_assert_eq((unsigned long)p->size, (unsigned long)q->size, "%lu");
	cr_assert_eq((unsigned long)p->elem_size, (unsigned long)q->elem_size, "%lu");
	for (i = 0; i < 1000; i++) {
		int a, b;
		a = rnd_queue_geti(q, i);
		b = rnd_queue_geti(p, i);
		cr_assert_eq(a, b);
	}
	cr_assert_eq(0, rnd_queue_destroy(q, NULL));
	cr_assert_eq(0, rnd_queue_clear(p, NULL));
	q = rnd_queue_create(sizeof(struct data), 1000);
	for (i = 0; i < 1000; i++) {
		struct data d;
		cr_assert_eq(0, data_init(&d));
		cr_assert_eq(0, rnd_queue_push(q, &d));
	}
	cr_assert_eq(RND_EHANDLER, rnd_queue_copy(p, q, data_cpy_bad));
	cr_assert_eq(0, rnd_queue_copy(p, q, data_cpy));
	cr_assert_eq((unsigned long)p->size, (unsigned long)q->size, "%lu");
	cr_assert_eq((unsigned long)p->elem_size, (unsigned long)q->elem_size, "%lu");
	for (i = 0; i < 1000; i++) {
		struct data a, b;
		cr_assert_eq(0, rnd_queue_get(q, i, &a));
		cr_assert_eq(0, rnd_queue_get(p, i, &b));
		cr_assert_eq(0, data_cmp(&a, &b));
	}
	cr_assert_eq(0, rnd_queue_destroy(q, data_dtor));
	cr_assert_eq(0, rnd_queue_destroy(p, data_dtor));
}

Test(queue, insert)
{
	struct rnd_queue *q;
	unsigned i;

	{ /* Generic form */
		int a = 10;
		struct data d[1000];
		q = rnd_queue_create(sizeof(int), 1000);
		cr_assert_not_null(q);
		cr_assert_eq(RND_EINVAL, rnd_queue_insert(q, 0, NULL));
		cr_assert_eq(RND_EINVAL, rnd_queue_insert(NULL, 0, &a));
		cr_assert_eq(RND_EINVAL, rnd_queue_insert(NULL, 0, NULL));
		cr_assert_eq(RND_EINDEX, rnd_queue_insert(q, 1, &a));
		cr_assert_eq(0, rnd_queue_insert(q, 0, &a));
		cr_assert_eq(1LU, (unsigned long)q->size, "%lu");
		cr_assert_eq(10, rnd_queue_geti(q, 0));
		cr_assert_eq(0, rnd_queue_destroy(q, NULL));

		q = rnd_queue_create(sizeof(struct data), 1000);
		cr_assert_not_null(q);
		for (i = 0; i < 1000; i++) {
			struct data a;
			size_t idx = IRANGE(0, i);
			cr_assert_eq(0, data_init(d + i));
			cr_assert_eq(0, rnd_queue_insert(q, idx, d + i));
			cr_assert_eq((unsigned long)i + 1, (unsigned long)q->size, "%lu");
			cr_assert_eq(0, rnd_queue_get(q, idx, &a));
			cr_assert_eq(0, data_cmp(&a, d + i));
		}
		cr_assert_eq(0, rnd_queue_destroy(q, data_dtor));

		q = rnd_queue_create(sizeof(struct data), 10);
		cr_assert_not_null(q);
		for (i = 0; i < 5; i++) {
			cr_assert_eq(0, data_init(d + i));
			cr_assert_eq(0, rnd_queue_insert(q, i, d + i));
		}
		for (i = 0; i < 5; i++) {
			struct data a;
			cr_assert_eq(0, rnd_queue_get(q, i, &a));
			cr_assert_eq(0, data_cmp(&a, d + i));
		}
		cr_assert_eq(0, data_init(d + 5));
		cr_assert_eq(0, rnd_queue_insert(q, 1, d + 5));
		cr_assert_eq(q->head, (char*)q->data + (q->capacity - 1) * q->elem_size, "%p");
		cr_assert_eq(0, data_init(d + 6));
		cr_assert_eq(0, rnd_queue_insert(q, 3, d + 6));
		cr_assert_eq(q->tail, (char*)q->data + 5 * q->elem_size, "%p");
		cr_assert_eq(0, rnd_queue_destroy(q, data_dtor));
	}

	/* Suffixed form
	 * T  - type
	 * F1 - insert function
	 * F2 - get function
	 * V  - random value snippet
	 * M  - printf format string
	 */
#define test(T, F1, F2, V, M) do {                                                              \
		T a = (V);                                                                      \
		T d[1000];                                                                      \
		q = rnd_queue_create(sizeof(T), 1000);                                          \
		cr_assert_not_null(q);                                                          \
		cr_assert_eq(RND_EINVAL, F1(NULL, 0, a));                                       \
		cr_assert_eq(RND_EINDEX, F1(q, 1, a));                                          \
		cr_assert_eq(0, F1(q, 0, a));                                                   \
		cr_assert_eq(1LU, (unsigned long)q->size, "%lu");                               \
		cr_assert_eq(a, F2(q, 0), M);                                                   \
		cr_assert_eq(0, rnd_queue_destroy(q, NULL));                                    \
                                                                                                \
		q = rnd_queue_create(sizeof(T), 1000);                                          \
		cr_assert_not_null(q);                                                          \
		for (i = 0; i < 1000; i++) {                                                    \
			size_t idx = IRANGE(0, i);                                              \
			d[i] = (V);                                                             \
			cr_assert_eq(0, F1(q, idx, d[i]));                                      \
			cr_assert_eq((unsigned long)i + 1, (unsigned long)q->size, "%lu");      \
			cr_assert_eq(d[i], F2(q, idx), M);                                      \
		}                                                                               \
		cr_assert_eq(0, rnd_queue_destroy(q, NULL));                                    \
                                                                                                \
		q = rnd_queue_create(sizeof(T), 10);                                            \
		cr_assert_not_null(q);                                                          \
		for (i = 0; i < 5; i++) {                                                       \
			d[i] = (V);                                                             \
			cr_assert_eq(0, F1(q, i, d[i]));                                        \
		}                                                                               \
		for (i = 0; i < 5; i++) {                                                       \
			cr_assert_eq(d[i], F2(q, i), M);                                        \
		}                                                                               \
		d[5] = (V);                                                                     \
		cr_assert_eq(0, F1(q, 1, d[5]));                                                \
		cr_assert_eq(q->head, (char*)q->data + (q->capacity - 1) * q->elem_size, "%p"); \
		d[6] = (V);                                                                     \
		cr_assert_eq(0, F1(q, 3, d[6]));                                                \
		cr_assert_eq(q->tail, (char*)q->data + 5 * q->elem_size, "%p");                 \
		cr_assert_eq(0, rnd_queue_destroy(q, NULL));                                    \
		q = rnd_queue_create(sizeof(T) + 1, 1000);                                      \
		cr_assert_not_null(q);                                                          \
		cr_assert_eq(RND_EILLEGAL, F1(q, 0, (V)));                                      \
		cr_assert_eq(0, rnd_queue_destroy(q, NULL));                                    \
	} while (0)
	test(char          , rnd_queue_insertc , rnd_queue_getc , IRANGE(1, CHAR_MAX) , "%hd");
	test(short         , rnd_queue_inserts , rnd_queue_gets , IRANGE(1, SHRT_MAX) , "%hd");
	test(int           , rnd_queue_inserti , rnd_queue_geti , FRANGE(1, INT_MAX)  , "%d");
	test(long          , rnd_queue_insertl , rnd_queue_getl , FRANGE(1, LONG_MAX) , "%ld");
	test(signed char   , rnd_queue_insertsc, rnd_queue_getsc, IRANGE(1, SCHAR_MAX), "%hd");
	test(unsigned char , rnd_queue_insertuc, rnd_queue_getuc, IRANGE(1, UCHAR_MAX), "%hd");
	test(unsigned short, rnd_queue_insertus, rnd_queue_getus, IRANGE(1, USHRT_MAX), "%hu");
	test(unsigned int  , rnd_queue_insertui, rnd_queue_getui, FRANGE(1, UINT_MAX) , "%u");
	test(unsigned long , rnd_queue_insertul, rnd_queue_getul, FRANGE(1, ULONG_MAX), "%lu");
	test(float         , rnd_queue_insertf , rnd_queue_getf , FRANGE(1, FLT_MAX)  , "%f");
	test(double        , rnd_queue_insertd , rnd_queue_getd , FRANGE(1, DBL_MAX)  , "%f");
	test(long double   , rnd_queue_insertld, rnd_queue_getld, FRANGE(1, LDBL_MAX) , "%Lf");
#undef test
}

Test(queue, qinsert)
{
	struct rnd_queue *q;
	unsigned i;

	{ /* Generic form */
		int a = 10;
		struct data d[1000];
		q = rnd_queue_create(sizeof(int), 1000);
		cr_assert_not_null(q);
		cr_assert_eq(RND_EINVAL, rnd_queue_qinsert(q, 0, NULL));
		cr_assert_eq(RND_EINVAL, rnd_queue_qinsert(NULL, 0, &a));
		cr_assert_eq(RND_EINVAL, rnd_queue_qinsert(NULL, 0, NULL));
		cr_assert_eq(RND_EINDEX, rnd_queue_qinsert(q, 1, &a));
		cr_assert_eq(0, rnd_queue_qinsert(q, 0, &a));
		cr_assert_eq(1LU, (unsigned long)q->size, "%lu");
		cr_assert_eq(10, rnd_queue_geti(q, 0));
		cr_assert_eq(0, rnd_queue_destroy(q, NULL));

		q = rnd_queue_create(sizeof(struct data), 1000);
		cr_assert_not_null(q);
		for (i = 0; i < 1000; i++) {
			struct data a;
			size_t idx = IRANGE(0, i);
			cr_assert_eq(0, data_init(d + i));
			cr_assert_eq(0, rnd_queue_qinsert(q, idx, d + i));
			cr_assert_eq((unsigned long)i + 1, (unsigned long)q->size, "%lu");
			cr_assert_eq(0, rnd_queue_get(q, idx, &a));
			cr_assert_eq(0, data_cmp(&a, d + i));
		}
		cr_assert_eq(0, rnd_queue_destroy(q, data_dtor));

		q = rnd_queue_create(sizeof(struct data), 10);
		cr_assert_not_null(q);
		for (i = 0; i < 5; i++) {
			cr_assert_eq(0, data_init(d + i));
			cr_assert_eq(0, rnd_queue_qinsert(q, i, d + i));
		}
		for (i = 0; i < 5; i++) {
			struct data a;
			cr_assert_eq(0, rnd_queue_get(q, i, &a));
			cr_assert_eq(0, data_cmp(&a, d + i));
		}
		cr_assert_eq(0, data_init(d + 5));
		cr_assert_eq(0, rnd_queue_qinsert(q, 1, d + 5));
		cr_assert_eq(q->tail, (char*)q->data + 5 * q->elem_size, "%p");
		cr_assert_eq(0, data_init(d + 6));
		cr_assert_eq(0, rnd_queue_qinsert(q, 3, d + 6));
		cr_assert_eq(q->tail, (char*)q->data + 6 * q->elem_size, "%p");
		cr_assert_eq(0, rnd_queue_destroy(q, data_dtor));
	}

	/* Suffixed form
	 * T  - type
	 * F1 - qinsert function
	 * F2 - get function
	 * V  - random value snippet
	 * M  - printf format string
	 */
#define test(T, F1, F2, V, M) do {                                                         \
		T a = (V);                                                                 \
		T d[1000];                                                                 \
		q = rnd_queue_create(sizeof(T), 1000);                                     \
		cr_assert_not_null(q);                                                     \
		cr_assert_eq(RND_EINVAL, F1(NULL, 0, a));                                  \
		cr_assert_eq(RND_EINDEX, F1(q, 1, a));                                     \
		cr_assert_eq(0, F1(q, 0, a));                                              \
		cr_assert_eq(1LU, (unsigned long)q->size, "%lu");                          \
		cr_assert_eq(a, F2(q, 0), M);                                              \
		cr_assert_eq(0, rnd_queue_destroy(q, NULL));                               \
                                                                                           \
		q = rnd_queue_create(sizeof(T), 1000);                                     \
		cr_assert_not_null(q);                                                     \
		for (i = 0; i < 1000; i++) {                                               \
			size_t idx = IRANGE(0, i);                                         \
			d[i] = (V);                                                        \
			cr_assert_eq(0, F1(q, idx, d[i]));                                 \
			cr_assert_eq((unsigned long)i + 1, (unsigned long)q->size, "%lu"); \
			cr_assert_eq(d[i], F2(q, idx), M);                                 \
		}                                                                          \
		cr_assert_eq(0, rnd_queue_destroy(q, NULL));                               \
                                                                                           \
		q = rnd_queue_create(sizeof(T), 10);                                       \
		cr_assert_not_null(q);                                                     \
		for (i = 0; i < 5; i++) {                                                  \
			d[i] = (V);                                                        \
			cr_assert_eq(0, F1(q, i, d[i]));                                   \
		}                                                                          \
		for (i = 0; i < 5; i++) {                                                  \
			cr_assert_eq(d[i], F2(q, i), M);                                   \
		}                                                                          \
		d[5] = (V);                                                                \
		cr_assert_eq(0, F1(q, 1, d[5]));                                           \
		cr_assert_eq(q->tail, (char*)q->data + 5 * q->elem_size, "%p");            \
		d[6] = (V);                                                                \
		cr_assert_eq(0, F1(q, 3, d[6]));                                           \
		cr_assert_eq(q->tail, (char*)q->data + 6 * q->elem_size, "%p");            \
		cr_assert_eq(0, rnd_queue_destroy(q, NULL));                               \
		q = rnd_queue_create(sizeof(T) + 1, 1000);                                 \
		cr_assert_not_null(q);                                                     \
		cr_assert_eq(RND_EILLEGAL, F1(q, 0, (V)));                                 \
		cr_assert_eq(0, rnd_queue_destroy(q, NULL));                               \
	} while (0)
	test(char          , rnd_queue_qinsertc , rnd_queue_getc , IRANGE(1, CHAR_MAX) , "%hd");
	test(short         , rnd_queue_qinserts , rnd_queue_gets , IRANGE(1, SHRT_MAX) , "%hd");
	test(int           , rnd_queue_qinserti , rnd_queue_geti , FRANGE(1, INT_MAX)  , "%d");
	test(long          , rnd_queue_qinsertl , rnd_queue_getl , FRANGE(1, LONG_MAX) , "%ld");
	test(signed char   , rnd_queue_qinsertsc, rnd_queue_getsc, IRANGE(1, SCHAR_MAX), "%hd");
	test(unsigned char , rnd_queue_qinsertuc, rnd_queue_getuc, IRANGE(1, UCHAR_MAX), "%hd");
	test(unsigned short, rnd_queue_qinsertus, rnd_queue_getus, IRANGE(1, USHRT_MAX), "%hu");
	test(unsigned int  , rnd_queue_qinsertui, rnd_queue_getui, FRANGE(1, UINT_MAX) , "%u");
	test(unsigned long , rnd_queue_qinsertul, rnd_queue_getul, FRANGE(1, ULONG_MAX), "%lu");
	test(float         , rnd_queue_qinsertf , rnd_queue_getf , FRANGE(1, FLT_MAX)  , "%f");
	test(double        , rnd_queue_qinsertd , rnd_queue_getd , FRANGE(1, DBL_MAX)  , "%f");
	test(long double   , rnd_queue_qinsertld, rnd_queue_getld, FRANGE(1, LDBL_MAX) , "%Lf");
#undef test
}

Test(queue, remove)
{
	struct rnd_queue *q;
	unsigned i;

	{ /* Generic form */
		struct data a;
		struct data d[100];
		q = rnd_queue_create(sizeof(int), 1000);
		cr_assert_not_null(q);
		cr_assert_eq(RND_EINDEX, rnd_queue_remove(q, 0, &a));
		cr_assert_eq(0, data_init(&a));
		cr_assert_eq(0, rnd_queue_push(q, &a));
		cr_assert_eq(RND_EINVAL, rnd_queue_remove(NULL, 0, &a));
		cr_assert_eq(RND_EINVAL, rnd_queue_remove(NULL, 0, NULL));
		cr_assert_eq(RND_EINDEX, rnd_queue_remove(q, 1, &a));
		cr_assert_eq(q->data, q->head, "%p");
		cr_assert_eq(q->data, q->tail, "%p");
		cr_assert_eq(0, rnd_queue_remove(q, 0, NULL));
		cr_assert_eq(0LU, (unsigned long)q->size, "%lu");
		cr_assert_eq(q->data, q->head, "%p");
		cr_assert_eq(q->data, q->tail, "%p");
		cr_assert_eq(0, rnd_queue_destroy(q, NULL));
		cr_assert_eq(0, data_dtor(&a));

		q = rnd_queue_create(sizeof(struct data), 100);
		cr_assert_not_null(q);
		for (i = 0; i < 100; i++) {
			cr_assert_eq(0, data_init(d + i));
			cr_assert_eq(0, rnd_queue_push(q, d + i));
		}
		for (i = 0; i < 100; i++) {
			struct data a;
			size_t idx = IRANGE(0, q->size - 1), j;
			int found = 0;
			cr_assert_eq(0, rnd_queue_remove(q, idx, &a));
			for (j = 0; j < 100; j++) {
				/* Lazy and slow way to check, but on average it's
				 * enough */
				if (data_cmp(&a, d + j) == 0) {
					found = 1;
					break;
				}
			}
			cr_assert_eq(1, found);
		}
		cr_assert_eq(0, rnd_queue_destroy(q, NULL));

		q = rnd_queue_create(sizeof(struct data), 10);
		cr_assert_not_null(q);
		for (i = 0; i < 8; i++) {
			cr_assert_eq(0, rnd_queue_push(q, d + i));
		}
		cr_assert_eq(0, rnd_queue_remove(q, 3, &a));
		cr_assert_eq(0, data_cmp(d + 3, &a));
		cr_assert_eq(q->head, (char*)q->data + q->elem_size, "%p");
		cr_assert_eq(0, rnd_queue_remove(q, 0, &a));
		cr_assert_eq(0, data_cmp(d, &a));
		cr_assert_eq(q->head, (char*)q->data + 2 * q->elem_size, "%p");
		cr_assert_eq(0, rnd_queue_remove(q, 3, &a));
		cr_assert_eq(0, data_cmp(d + 5, &a));
		cr_assert_eq(q->tail, (char*)q->data + 6 * q->elem_size, "%p");
		cr_assert_eq(0, rnd_queue_destroy(q, NULL));
		for (i = 0; i < 100; i++) {
			cr_assert_eq(0, data_dtor(d + i));
		}
	}

	/* Suffixed form
	 * T  - type
	 * F1 - remove function
	 * F2 - push function
	 * V  - random value snippet
	 * M  - printf format string
	 */
#define test(T, F1, F2, V, M) do {                                                 \
		T a = (V), z = 0;                                                  \
		T d[100];                                                          \
		q = rnd_queue_create(sizeof(T), 1000);                             \
		cr_assert_not_null(q);                                             \
		cr_assert_eq(z, F1(q, 0), M);                                      \
		cr_assert_eq(0, F2(q, a));                                         \
		cr_assert_eq(z, F1(NULL, 0), M);                                   \
		cr_assert_eq(z, F1(q, 1), M);                                      \
		cr_assert_eq(q->data, q->head, "%p");                              \
		cr_assert_eq(q->data, q->tail, "%p");                              \
		cr_assert_eq(a, F1(q, 0), M);                                      \
		cr_assert_eq(0LU, (unsigned long)q->size, "%lu");                  \
		cr_assert_eq(q->data, q->head, "%p");                              \
		cr_assert_eq(q->data, q->tail, "%p");                              \
		cr_assert_eq(0, rnd_queue_destroy(q, NULL));                       \
                                                                                   \
		q = rnd_queue_create(sizeof(T), 100);                              \
		cr_assert_not_null(q);                                             \
		for (i = 0; i < 100; i++) {                                        \
			d[i] = (V);                                                \
			cr_assert_eq(0, F2(q, d[i]));                              \
		}                                                                  \
		for (i = 0; i < 100; i++) {                                        \
			T a;                                                       \
			size_t idx = IRANGE(0, q->size - 1), j;                    \
			int found = 0;                                             \
			cr_assert_neq(0, (a = F1(q, idx)));                        \
			for (j = 0; j < 100; j++) {                                \
				/* Lazy and slow way to check, but on average it's
				 * enough */                                       \
				if (a == d[j]) {                                   \
					found = 1;                                 \
					break;                                     \
				}                                                  \
			}                                                          \
			cr_assert_eq(1, found);                                    \
		}                                                                  \
		cr_assert_eq(0, rnd_queue_destroy(q, NULL));                       \
                                                                                   \
		q = rnd_queue_create(sizeof(T), 10);                               \
		cr_assert_not_null(q);                                             \
		for (i = 0; i < 8; i++) {                                          \
			cr_assert_eq(0, F2(q, d[i]));                              \
		}                                                                  \
		cr_assert_eq(d[3], F1(q, 3), M);                                   \
		cr_assert_eq(q->head, (char*)q->data + q->elem_size, "%p");        \
		cr_assert_eq(d[0], F1(q, 0), M);                                   \
		cr_assert_eq(q->head, (char*)q->data + 2 * q->elem_size, "%p");    \
		cr_assert_eq(d[5], F1(q, 3), M);                                   \
		cr_assert_eq(q->tail, (char*)q->data + 6 * q->elem_size, "%p");    \
		cr_assert_eq(0, rnd_queue_destroy(q, NULL));                       \
		q = rnd_queue_create(sizeof(T) + 1, 1000);                         \
		cr_assert_not_null(q);                                             \
		q->size = 1;                                                       \
		cr_assert_eq(z, F1(q, 0), M);                                      \
		cr_assert_eq(0, rnd_queue_destroy(q, NULL));                       \
	} while (0)
	test(char          , rnd_queue_removec , rnd_queue_pushc , IRANGE(1, CHAR_MAX) , "%hd");
	test(short         , rnd_queue_removes , rnd_queue_pushs , IRANGE(1, SHRT_MAX) , "%hd");
	test(int           , rnd_queue_removei , rnd_queue_pushi , FRANGE(1, INT_MAX)  , "%d");
	test(long          , rnd_queue_removel , rnd_queue_pushl , FRANGE(1, LONG_MAX) , "%ld");
	test(signed char   , rnd_queue_removesc, rnd_queue_pushsc, IRANGE(1, SCHAR_MAX), "%hd");
	test(unsigned char , rnd_queue_removeuc, rnd_queue_pushuc, IRANGE(1, UCHAR_MAX), "%hd");
	test(unsigned short, rnd_queue_removeus, rnd_queue_pushus, IRANGE(1, USHRT_MAX), "%hu");
	test(unsigned int  , rnd_queue_removeui, rnd_queue_pushui, FRANGE(1, UINT_MAX) , "%u");
	test(unsigned long , rnd_queue_removeul, rnd_queue_pushul, FRANGE(1, ULONG_MAX), "%lu");
	test(float         , rnd_queue_removef , rnd_queue_pushf , FRANGE(1, FLT_MAX)  , "%f");
	test(double        , rnd_queue_removed , rnd_queue_pushd , FRANGE(1, DBL_MAX)  , "%f");
	test(long double   , rnd_queue_removeld, rnd_queue_pushld, FRANGE(1, LDBL_MAX) , "%Lf");
#undef test
}

Test(queue, qremove)
{
	struct rnd_queue *q;
	unsigned i;

	{ /* Generic form */
		struct data a;
		struct data d[100];
		q = rnd_queue_create(sizeof(int), 1000);
		cr_assert_not_null(q);
		cr_assert_eq(RND_EINDEX, rnd_queue_qremove(q, 0, &a));
		cr_assert_eq(0, data_init(&a));
		cr_assert_eq(0, rnd_queue_push(q, &a));
		cr_assert_eq(RND_EINVAL, rnd_queue_qremove(NULL, 0, &a));
		cr_assert_eq(RND_EINVAL, rnd_queue_qremove(NULL, 0, NULL));
		cr_assert_eq(RND_EINDEX, rnd_queue_qremove(q, 1, &a));
		cr_assert_eq(q->data, q->head, "%p");
		cr_assert_eq(q->data, q->tail, "%p");
		cr_assert_eq(0, rnd_queue_qremove(q, 0, NULL));
		cr_assert_eq(0LU, (unsigned long)q->size, "%lu");
		cr_assert_eq(q->data, q->head, "%p");
		cr_assert_eq(q->data, q->tail, "%p");
		cr_assert_eq(0, rnd_queue_destroy(q, NULL));
		cr_assert_eq(0, data_dtor(&a));

		q = rnd_queue_create(sizeof(struct data), 100);
		cr_assert_not_null(q);
		for (i = 0; i < 100; i++) {
			cr_assert_eq(0, data_init(d + i));
			cr_assert_eq(0, rnd_queue_push(q, d + i));
		}
		for (i = 0; i < 100; i++) {
			struct data a;
			size_t idx = IRANGE(0, q->size - 1), j;
			int found = 0;
			cr_assert_eq(0, rnd_queue_qremove(q, idx, &a));
			for (j = 0; j < 100; j++) {
				/* Lazy and slow way to check, but on average it's
				 * enough */
				if (data_cmp(&a, d + j) == 0) {
					found = 1;
					break;
				}
			}
			cr_assert_eq(1, found);
		}
		cr_assert_eq(0, rnd_queue_destroy(q, NULL));

		q = rnd_queue_create(sizeof(struct data), 10);
		cr_assert_not_null(q);
		for (i = 0; i < 8; i++) {
			cr_assert_eq(0, rnd_queue_push(q, d + i));
		}
		cr_assert_eq(0, rnd_queue_qremove(q, 3, &a));
		cr_assert_eq(0, data_cmp(d + 3, &a));
		cr_assert_eq(0, rnd_queue_get(q, 3, &a));
		cr_assert_eq(0, data_cmp(d + 7, &a));
		cr_assert_eq(0, rnd_queue_qremove(q, 0, &a));
		cr_assert_eq(0, data_cmp(d, &a));
		cr_assert_eq(0, rnd_queue_get(q, 0, &a));
		cr_assert_eq(0, data_cmp(d + 6, &a));
		cr_assert_eq(0, rnd_queue_qremove(q, 3, &a));
		cr_assert_eq(0, data_cmp(d + 7, &a));
		cr_assert_eq(q->tail, (char*)q->data + 4 * q->elem_size, "%p");
		cr_assert_eq(0, rnd_queue_destroy(q, NULL));
		for (i = 0; i < 100; i++) {
			cr_assert_eq(0, data_dtor(d + i));
		}
	}

	/* Suffixed form
	 * T  - type
	 * F1 - qremove function
	 * F2 - push function
	 * F3 - get function
	 * V  - random value snippet
	 * M  - printf format string
	 */
#define test(T, F1, F2, F3, V, M) do {                                             \
		T a = (V), z = 0;                                                  \
		T d[100];                                                          \
		q = rnd_queue_create(sizeof(T), 1000);                             \
		cr_assert_not_null(q);                                             \
		cr_assert_eq(z, F1(q, 0), M);                                      \
		cr_assert_eq(0, F2(q, a));                                         \
		cr_assert_eq(z, F1(NULL, 0), M);                                   \
		cr_assert_eq(z, F1(q, 1), M);                                      \
		cr_assert_eq(q->data, q->head, "%p");                              \
		cr_assert_eq(q->data, q->tail, "%p");                              \
		cr_assert_eq(a, F1(q, 0), M);                                      \
		cr_assert_eq(0LU, (unsigned long)q->size, "%lu");                  \
		cr_assert_eq(q->data, q->head, "%p");                              \
		cr_assert_eq(q->data, q->tail, "%p");                              \
		cr_assert_eq(0, rnd_queue_destroy(q, NULL));                       \
                                                                                   \
		q = rnd_queue_create(sizeof(T), 100);                              \
		cr_assert_not_null(q);                                             \
		for (i = 0; i < 100; i++) {                                        \
			d[i] = (V);                                                \
			cr_assert_eq(0, F2(q, d[i]));                              \
		}                                                                  \
		for (i = 0; i < 100; i++) {                                        \
			T a;                                                       \
			size_t idx = IRANGE(0, q->size - 1), j;                    \
			int found = 0;                                             \
			cr_assert_neq(0, (a = F1(q, idx)));                        \
			for (j = 0; j < 100; j++) {                                \
				/* Lazy and slow way to check, but on average it's
				 * enough */                                       \
				if (a == d[j]) {                                   \
					found = 1;                                 \
					break;                                     \
				}                                                  \
			}                                                          \
			cr_assert_eq(1, found);                                    \
		}                                                                  \
		cr_assert_eq(0, rnd_queue_destroy(q, NULL));                       \
                                                                                   \
		q = rnd_queue_create(sizeof(T), 10);                               \
		cr_assert_not_null(q);                                             \
		for (i = 0; i < 8; i++) {                                          \
			cr_assert_eq(0, F2(q, d[i]));                              \
		}                                                                  \
		cr_assert_eq(d[3], F1(q, 3), M);                                   \
		cr_assert_eq(d[7], F3(q, 3), M);                                   \
		cr_assert_eq(d[0], F1(q, 0), M);                                   \
		cr_assert_eq(d[6], F3(q, 0), M);                                   \
		cr_assert_eq(d[7], F1(q, 3), M);                                   \
		cr_assert_eq(d[5], F3(q, 3), M);                                   \
		cr_assert_eq(q->tail, (char*)q->data + 4 * q->elem_size, "%p");    \
		cr_assert_eq(0, rnd_queue_destroy(q, NULL));                       \
		q = rnd_queue_create(sizeof(T) + 1, 1000);                         \
		cr_assert_not_null(q);                                             \
		q->size = 1;                                                       \
		cr_assert_eq(z, F1(q, 0), M);                                      \
		cr_assert_eq(0, rnd_queue_destroy(q, NULL));                       \
	} while (0)
	test(char          , rnd_queue_qremovec , rnd_queue_pushc , rnd_queue_getc , IRANGE(1, CHAR_MAX) , "%hd");
	test(short         , rnd_queue_qremoves , rnd_queue_pushs , rnd_queue_gets , IRANGE(1, SHRT_MAX) , "%hd");
	test(int           , rnd_queue_qremovei , rnd_queue_pushi , rnd_queue_geti , FRANGE(1, INT_MAX)  , "%d");
	test(long          , rnd_queue_qremovel , rnd_queue_pushl , rnd_queue_getl , FRANGE(1, LONG_MAX) , "%ld");
	test(signed char   , rnd_queue_qremovesc, rnd_queue_pushsc, rnd_queue_getsc, IRANGE(1, SCHAR_MAX), "%hd");
	test(unsigned char , rnd_queue_qremoveuc, rnd_queue_pushuc, rnd_queue_getuc, IRANGE(1, UCHAR_MAX), "%hd");
	test(unsigned short, rnd_queue_qremoveus, rnd_queue_pushus, rnd_queue_getus, IRANGE(1, USHRT_MAX), "%hu");
	test(unsigned int  , rnd_queue_qremoveui, rnd_queue_pushui, rnd_queue_getui, FRANGE(1, UINT_MAX) , "%u");
	test(unsigned long , rnd_queue_qremoveul, rnd_queue_pushul, rnd_queue_getul, FRANGE(1, ULONG_MAX), "%lu");
	test(float         , rnd_queue_qremovef , rnd_queue_pushf , rnd_queue_getf , FRANGE(1, FLT_MAX)  , "%f");
	test(double        , rnd_queue_qremoved , rnd_queue_pushd , rnd_queue_getd , FRANGE(1, DBL_MAX)  , "%f");
	test(long double   , rnd_queue_qremoveld, rnd_queue_pushld, rnd_queue_getld, FRANGE(1, LDBL_MAX) , "%Lf");
#undef test
}

Test(queue, get)
{
	struct rnd_queue *q;

	{ /* Generic form */
		unsigned i;
		int a = 0;
		struct data d[1000];
		q = rnd_queue_create(sizeof(int), 1000);
		cr_assert_not_null(q);
		cr_assert_eq(RND_EINDEX, rnd_queue_get(q, 0, &a));
		cr_assert_eq(0, rnd_queue_pushi(q, 10));
		cr_assert_eq(RND_EINVAL, rnd_queue_get(NULL, 0, &a));
		cr_assert_eq(RND_EINVAL, rnd_queue_get(q, 0, NULL));
		cr_assert_eq(RND_EINVAL, rnd_queue_get(NULL, 0, NULL));
		cr_assert_eq(RND_EINDEX, rnd_queue_get(q, 1, &a));
		cr_assert_eq(0, rnd_queue_get(q, 0, &a));
		cr_assert_eq(10, a);
		cr_assert_eq(0, rnd_queue_destroy(q, NULL));
		q = rnd_queue_create(sizeof(struct data), 1000);
		cr_assert_not_null(q);
		for (i = 0; i < 1000; i++) {
			cr_assert_eq(0, data_init(d + i));
			cr_assert_eq(0, rnd_queue_push(q, d + i));
		}
		for (i = 0; i < 1000; i++) {
			struct data a;
			cr_assert_eq(0, rnd_queue_get(q, i, &a));
			cr_assert_eq(0, data_cmp(&a, d + i));
		}
		cr_assert_eq(0, rnd_queue_destroy(q, data_dtor));
	}

	/* Suffixed form
	 * T  - type
	 * F1 - get function
	 * F2 - push function
	 * V  - random value snippet
	 * M  - printf format string
	 */
#define test(T, F1, F2, V, M) do {                           \
		unsigned i;                                  \
		T a = (V), z = 0;                            \
		T d[1000];                                   \
		q = rnd_queue_create(sizeof(T), 1000);       \
		cr_assert_not_null(q);                       \
		cr_assert_eq(z, F1(q, 0), M);                \
		cr_assert_eq(0, F2(q, a));                   \
		cr_assert_eq(z, F1(NULL, 0), M);             \
		cr_assert_eq(z, F1(q, 1), M);                \
		cr_assert_eq(a, F1(q, 0), M);                \
		cr_assert_eq(0, rnd_queue_destroy(q, NULL)); \
		q = rnd_queue_create(sizeof(T), 1000);       \
		cr_assert_not_null(q);                       \
		for (i = 0; i < 1000; i++) {                 \
			d[i] = (V);                          \
			cr_assert_eq(0, F2(q, d[i]));        \
		}                                            \
		for (i = 0; i < 1000; i++) {                 \
			cr_assert_eq(d[i], F1(q, i), M);     \
		}                                            \
		cr_assert_eq(0, rnd_queue_destroy(q, NULL)); \
	} while (0)
	test(char          , rnd_queue_getc , rnd_queue_pushc , IRANGE(1, CHAR_MAX) , "%hd");
	test(short         , rnd_queue_gets , rnd_queue_pushs , IRANGE(1, SHRT_MAX) , "%hd");
	test(int           , rnd_queue_geti , rnd_queue_pushi , FRANGE(1, INT_MAX)  , "%d");
	test(long          , rnd_queue_getl , rnd_queue_pushl , FRANGE(1, LONG_MAX) , "%ld");
	test(signed char   , rnd_queue_getsc, rnd_queue_pushsc, IRANGE(1, SCHAR_MAX), "%hd");
	test(unsigned char , rnd_queue_getuc, rnd_queue_pushuc, IRANGE(1, UCHAR_MAX), "%hd");
	test(unsigned short, rnd_queue_getus, rnd_queue_pushus, IRANGE(1, USHRT_MAX), "%hu");
	test(unsigned int  , rnd_queue_getui, rnd_queue_pushui, FRANGE(1, UINT_MAX) , "%u");
	test(unsigned long , rnd_queue_getul, rnd_queue_pushul, FRANGE(1, ULONG_MAX), "%lu");
	test(float         , rnd_queue_getf , rnd_queue_pushf , FRANGE(1, FLT_MAX)  , "%f");
	test(double        , rnd_queue_getd , rnd_queue_pushd , FRANGE(1, DBL_MAX)  , "%f");
	test(long double   , rnd_queue_getld, rnd_queue_pushld, FRANGE(1, LDBL_MAX) , "%Lf");
#undef test
}

Test(queue, set)
{
	struct rnd_queue *q;

	{ /* Generic form */
		unsigned i;
		int a = 1;
		q = rnd_queue_create(sizeof(int), 1000);
		cr_assert_not_null(q);
		cr_assert_eq(RND_EINDEX, rnd_queue_set(q, 0, &a));
		cr_assert_eq(0, rnd_queue_pushi(q, 10));
		cr_assert_eq(RND_EINVAL, rnd_queue_set(NULL, 0, &a));
		cr_assert_eq(RND_EINVAL, rnd_queue_set(q, 0, NULL));
		cr_assert_eq(RND_EINVAL, rnd_queue_set(NULL, 0, NULL));
		cr_assert_eq(RND_EINDEX, rnd_queue_set(q, 1, &a));
		cr_assert_eq(0, rnd_queue_set(q, 0, &a));
		cr_assert_eq(a, rnd_queue_geti(q, 0));
		cr_assert_eq(0, rnd_queue_destroy(q, NULL));
		q = rnd_queue_create(sizeof(struct data), 1000);
		cr_assert_not_null(q);
		for (i = 0; i < 1000; i++) {
			struct data d;
			cr_assert_eq(0, data_init(&d));
			cr_assert_eq(0, rnd_queue_push(q, &d));
		}
		for (i = 0; i < 1000; i++) {
			struct data a, b;
			cr_assert_eq(0, data_init(&b));
			cr_assert_eq(0, rnd_queue_get(q, i, &a));
			cr_assert_eq(0, data_dtor(&a));
			cr_assert_eq(0, rnd_queue_set(q, i, &b));
			cr_assert_eq(0, rnd_queue_get(q, i, &a));
			cr_assert_eq(0, data_cmp(&a, &b));
		}
		cr_assert_eq(0, rnd_queue_destroy(q, data_dtor));
	}

	/* Suffixed form
	 * T  - type
	 * F1 - set function
	 * F2 - push function
	 * F3 - get function
	 * V  - random value snippet
	 * M  - printf format string
	 */
#define test(T, F1, F2, F3, V, M) do {                       \
		unsigned i;                                  \
		T a = (V);                                   \
		q = rnd_queue_create(sizeof(T), 1000);       \
		cr_assert_not_null(q);                       \
		cr_assert_eq(RND_EINDEX, F1(q, 0, a));       \
		cr_assert_eq(0, F2(q, (V)));                 \
		cr_assert_eq(RND_EINVAL, F1(NULL, 0, a));    \
		cr_assert_eq(RND_EINDEX, F1(q, 1, a));       \
		cr_assert_eq(0, F1(q, 0, a));                \
		cr_assert_eq(a, F3(q, 0), M);                \
		cr_assert_eq(0, rnd_queue_clear(q, NULL));   \
		for (i = 0; i < 1000; i++) {                 \
			cr_assert_eq(0, F2(q, (V)));         \
		}                                            \
		for (i = 0; i < 1000; i++) {                 \
			T b = (V);                           \
			cr_assert_eq(0, F1(q, i, b));        \
			cr_assert_eq(b, F3(q, i), M);        \
		}                                            \
		cr_assert_eq(0, rnd_queue_destroy(q, NULL)); \
		q = rnd_queue_create(sizeof(T) + 1, 1000);   \
		cr_assert_not_null(q);                       \
		q->size = 1;                                 \
		cr_assert_eq(RND_EILLEGAL, F1(q, 0, (V)));   \
		cr_assert_eq(0, rnd_queue_destroy(q, NULL)); \
	} while (0)
	test(char          , rnd_queue_setc , rnd_queue_pushc , rnd_queue_getc , IRANGE(1, CHAR_MAX) , "%hd");
	test(short         , rnd_queue_sets , rnd_queue_pushs , rnd_queue_gets , IRANGE(1, SHRT_MAX) , "%hd");
	test(int           , rnd_queue_seti , rnd_queue_pushi , rnd_queue_geti , FRANGE(1, INT_MAX)  , "%d");
	test(long          , rnd_queue_setl , rnd_queue_pushl , rnd_queue_getl , FRANGE(1, LONG_MAX) , "%ld");
	test(signed char   , rnd_queue_setsc, rnd_queue_pushsc, rnd_queue_getsc, IRANGE(1, SCHAR_MAX), "%hd");
	test(unsigned char , rnd_queue_setuc, rnd_queue_pushuc, rnd_queue_getuc, IRANGE(1, UCHAR_MAX), "%hd");
	test(unsigned short, rnd_queue_setus, rnd_queue_pushus, rnd_queue_getus, IRANGE(1, USHRT_MAX), "%hu");
	test(unsigned int  , rnd_queue_setui, rnd_queue_pushui, rnd_queue_getui, FRANGE(1, UINT_MAX) , "%u");
	test(unsigned long , rnd_queue_setul, rnd_queue_pushul, rnd_queue_getul, FRANGE(1, ULONG_MAX), "%lu");
	test(float         , rnd_queue_setf , rnd_queue_pushf , rnd_queue_getf , FRANGE(1, FLT_MAX)  , "%f");
	test(double        , rnd_queue_setd , rnd_queue_pushd , rnd_queue_getd , FRANGE(1, DBL_MAX)  , "%f");
	test(long double   , rnd_queue_setld, rnd_queue_pushld, rnd_queue_getld, FRANGE(1, LDBL_MAX) , "%Lf");
#undef test
}

Test(queue, print)
{
	struct rnd_queue *q;

	{ /* Generic form */
		double a = 4.5, b = -3.14;
		q = rnd_queue_create(sizeof(double), 30);
		cr_assert_eq(0, rnd_queue_push(q, &a));
		cr_assert_eq(0, rnd_queue_push(q, &b));
		cr_assert_eq(RND_EINVAL, rnd_queue_print(NULL));
		cr_assert_eq(0, rnd_queue_print(q));
		cr_assert_eq(0, rnd_queue_destroy(q, NULL));
	}

	/* Suffixed form
	 * T  - type
	 * F1 - push function
	 * F2 - print function
	 * A  - 1st value
	 * B  - 2nd value
	 */
#define test(T, F1, F2, A, B)                                \
	do {                                                 \
		T a = A, b = B;                              \
		q = rnd_queue_create(sizeof(T), 30);         \
		cr_assert_eq(0, F1(q, a));                   \
		cr_assert_eq(0, F1(q, b));                   \
		cr_assert_eq(RND_EINVAL, F2(NULL));          \
		cr_assert_eq(0, F2(q));                      \
		cr_assert_eq(0, rnd_queue_destroy(q, NULL)); \
		q = rnd_queue_create(sizeof(T) + 1, 30);     \
		cr_assert_eq(RND_EILLEGAL, F2(q));           \
		cr_assert_eq(0, rnd_queue_destroy(q, NULL)); \
	} while(0)

	test(char          , rnd_queue_pushc , rnd_queue_printc , 'A', 'B');
	test(short         , rnd_queue_pushs , rnd_queue_prints , SHRT_MIN, SHRT_MAX);
	test(int           , rnd_queue_pushi , rnd_queue_printi , INT_MIN, INT_MAX);
	test(long          , rnd_queue_pushl , rnd_queue_printl , LONG_MIN, LONG_MAX);
	test(signed char   , rnd_queue_pushsc, rnd_queue_printsc, SCHAR_MIN, SCHAR_MAX);
	test(unsigned char , rnd_queue_pushuc, rnd_queue_printuc, 0, UCHAR_MAX);
	test(unsigned short, rnd_queue_pushus, rnd_queue_printus, 0, USHRT_MAX);
	test(unsigned int  , rnd_queue_pushui, rnd_queue_printui, 0, UINT_MAX);
	test(unsigned long , rnd_queue_pushul, rnd_queue_printul, 0, ULONG_MAX);
	test(float         , rnd_queue_pushf , rnd_queue_printf , FLT_MIN, FLT_MAX);
	test(double        , rnd_queue_pushd , rnd_queue_printd , DBL_MIN, DBL_MAX);
	test(long double   , rnd_queue_pushld, rnd_queue_printld, LDBL_MIN, LDBL_MAX);
}

Test(queue, ringbuf_resize)
{
	struct rnd_queue *q;
	unsigned i;
	q = rnd_queue_create(sizeof(int), 5);
	cr_assert_not_null(q);
	for (i = 0; i < 5; i++)
		cr_assert_eq(0, rnd_queue_pushi(q, FRANGE(1, INT_MAX)));
	cr_assert_eq(q->data, q->head, "%p");
	cr_assert_eq((char*)q->data + (q->size - 1) * q->elem_size, q->tail, "%p");
	cr_assert_eq(5LU, (unsigned long)q->capacity, "%lu");
	cr_assert_eq(0, rnd_queue_pushi(q, FRANGE(1, INT_MAX)));
	cr_assert_eq(q->data, q->head, "%p");
	cr_assert_eq((char*)q->data + (q->size - 1) * q->elem_size, q->tail, "%p");
	cr_assert_eq(10LU, (unsigned long)q->capacity, "%lu");
	for (i = 0; i < 3; i++)
		cr_assert_neq(0, rnd_queue_popi(q));
	for (i = 0; i < 7; i++)
		cr_assert_eq(0, rnd_queue_pushi(q, FRANGE(1, INT_MAX)));
	cr_assert_eq((char*)q->data + 3 * q->elem_size, q->head, "%p");
	cr_assert_eq((char*)q->data + 2 * q->elem_size, q->tail, "%p");
	cr_assert_eq(10LU, (unsigned long)q->capacity, "%lu");
	cr_assert_eq(0, rnd_queue_pushi(q, FRANGE(1, INT_MAX)));
	cr_assert_eq((char*)q->data + 3 * q->elem_size, q->head, "%p");
	cr_assert_eq((char*)q->data + 13 * q->elem_size, q->tail, "%p");
	cr_assert_eq(20LU, (unsigned long)q->capacity, "%lu");
	cr_assert_eq(0, rnd_queue_destroy(q, NULL));
}
