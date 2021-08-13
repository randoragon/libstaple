#include <stdlib.h>
#include <time.h>
#include "../../src/rnd.h"
#include "test_struct.h"
#include <greatest.h>
#include <limits.h>
#include <float.h>
#include <string.h>

/* Make testing for size overflow feasible */
#define SIZE_MAX 65535LU

GREATEST_MAIN_DEFS();

TEST t_create(void)
{
	struct rnd_queue *q;
	q = rnd_queue_create(sizeof(int), 0);
	ASSERT_EQ(NULL, q);
	q = rnd_queue_create(0, 16);
	ASSERT_EQ(NULL, q);
	q = rnd_queue_create(0, 0);
	ASSERT_EQ(NULL, q);
	q = rnd_queue_create(SIZE_MAX, SIZE_MAX);
	ASSERT_EQ(NULL, q);
	q = rnd_queue_create(sizeof(int), 16);
	ASSERT_NEQ(NULL, q);
	ASSERT_EQ_FMT(0, rnd_queue_destroy(q, NULL), "%d");
	PASS();
}

TEST t_destroy(void)
{
	struct rnd_queue *q;
	unsigned i;
	q = rnd_queue_create(sizeof(long double), 1000);
	ASSERT_NEQ(NULL, q);
	ASSERT_EQ_FMT(RND_EINVAL, rnd_queue_destroy(NULL, NULL), "%d");
	ASSERT_EQ_FMT(0, rnd_queue_destroy(q, NULL), "%d");
	q = rnd_queue_create(sizeof(struct data), 1000);
	for (i = 0; i < 1000; i++) {
		struct data d;
		ASSERT_EQ_FMT(0, data_init(&d), "%d");
		ASSERT_EQ_FMT(0, rnd_queue_push(q, &d), "%d");
	}
	ASSERT_EQ_FMT(RND_EHANDLER, rnd_queue_destroy(q, data_dtor_bad), "%d");
	ASSERT_EQ_FMT(0, rnd_queue_destroy(q, data_dtor), "%d");
	PASS();
}

TEST t_push(void)
{
	struct rnd_queue *q;

	{ /* Generic form */
		unsigned i;
		struct data d;
		q = rnd_queue_create(sizeof(struct data), 1000);
		ASSERT_NEQ(NULL, q);
		data_init(&d);
		ASSERT_EQ_FMT(RND_EINVAL, rnd_queue_push(q, NULL), "%d");
		ASSERT_EQ_FMT(RND_EINVAL, rnd_queue_push(NULL, &d), "%d");
		ASSERT_EQ_FMT(RND_EINVAL, rnd_queue_push(NULL, NULL), "%d");
		ASSERT_EQ_FMT(q->head, q->data, "%p");
		ASSERT_EQ_FMT(q->tail, q->data, "%p");
		ASSERT_EQ_FMT(0, rnd_queue_push(q, &d), "%d");
		ASSERT_EQ_FMT(q->head, q->data, "%p");
		ASSERT_EQ_FMT(q->tail, q->data, "%p");
		ASSERT_EQ_FMT(0, rnd_queue_push(q, &d), "%d");
		ASSERT_EQ_FMT(q->head, q->data, "%p");
		ASSERT_EQ_FMT(q->tail, (char*)q->data + q->elem_size, "%p");
		ASSERT_EQ_FMT(0, rnd_queue_clear(q, NULL), "%d");
		for (i = 0; i < SIZE_MAX / sizeof(struct data); i++) {
			struct data a, b;
			ASSERT_EQ_FMT(0, data_init(&a), "%d");
			ASSERT_EQ_FMT(0, rnd_queue_push(q, &a), "%d");
			ASSERT_EQ_FMT(0, rnd_queue_get(q, i, &b), "%d");
			ASSERT_EQ_FMT(0, data_cmp(&a, &b), "%d");
		}
		ASSERT_EQ_FMT(RND_ERANGE, rnd_queue_push(q, &d), "%d");
		ASSERT_EQ_FMT(0, data_dtor(&d), "%d");
		ASSERT_EQ_FMT(0, rnd_queue_destroy(q, data_dtor), "%d");
	}

	/* Suffixed form
	 * T  - type
	 * F1 - push function
	 * F2 - get function
	 * V  - random value snippet
	 * M  - printf format string
	 */
#define test(T, F1, F2, V, M) do {                                           \
		unsigned i;                                                  \
		q = rnd_queue_create(sizeof(T), 1000);                       \
		ASSERT_NEQ(NULL, q);                                         \
		ASSERT_EQ_FMT(RND_EINVAL, F1(NULL, (V)), "%d");              \
		ASSERT_EQ_FMT(q->head, q->data, "%p");                       \
		ASSERT_EQ_FMT(q->tail, q->data, "%p");                       \
		ASSERT_EQ_FMT(0, F1(q, (V)), "%d");                          \
		ASSERT_EQ_FMT(q->head, q->data, "%p");                       \
		ASSERT_EQ_FMT(q->tail, q->data, "%p");                       \
		ASSERT_EQ_FMT(0, F1(q, (V)), "%d");                          \
		ASSERT_EQ_FMT(q->head, q->data, "%p");                       \
		ASSERT_EQ_FMT(q->tail, (char*)q->data + q->elem_size, "%p"); \
		ASSERT_EQ_FMT(0, rnd_queue_clear(q, NULL), "%d");            \
		for (i = 0; i < SIZE_MAX / sizeof(T); i++) {                 \
			T a = (V);                                           \
			ASSERT_EQ_FMT(0, F1(q, a), "%d");                    \
			ASSERT_EQ_FMT(a, F2(q, i), M);                       \
		}                                                            \
		ASSERT_EQ_FMT(RND_ERANGE, F1(q, (V)), "%d");                 \
		ASSERT_EQ_FMT(0, rnd_queue_destroy(q, NULL), "%d");          \
		q = rnd_queue_create(sizeof(T) + 1, 1);                      \
		ASSERT_NEQ(NULL, q);                                         \
		ASSERT_EQ_FMT(RND_EILLEGAL, F1(q, (V)), "%d");               \
		ASSERT_EQ_FMT(0, rnd_queue_destroy(q, NULL), "%d");          \
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
	PASS();
}

TEST t_peek(void)
{
	struct rnd_queue *q;

	{ /* Generic form */
		struct data a, b;
		q = rnd_queue_create(sizeof(struct data), 2);
		ASSERT_NEQ(NULL, q);
		ASSERT_EQ_FMT(RND_EILLEGAL, rnd_queue_peek(q, &b), "%d");
		ASSERT_EQ_FMT(RND_EINVAL, rnd_queue_peek(q, NULL), "%d");
		ASSERT_EQ_FMT(RND_EINVAL, rnd_queue_peek(NULL, &b), "%d");
		ASSERT_EQ_FMT(RND_EINVAL, rnd_queue_peek(NULL, NULL), "%d");
		data_init(&a);
		ASSERT_EQ_FMT(0, rnd_queue_push(q, &a), "%d");
		ASSERT_EQ_FMT(0, rnd_queue_peek(q, &b), "%d");
		ASSERT_EQ_FMT(0, data_cmp(&a, &b), "%d");
		ASSERT_EQ_FMT(0, rnd_queue_destroy(q, data_dtor), "%d");
	}

	/* Suffixed form
	 * T  - type
	 * F1 - peek function
	 * F2 - push function
	 * V  - random value snippet
	 * M  - printf format string
	 */
#define test(T, F1, F2, V, M) do {                                  \
		T a = (V), z = 0;                                   \
		q = rnd_queue_create(sizeof(T), 2);                 \
		ASSERT_NEQ(NULL, q);                                \
		ASSERT_EQ_FMT(z, F1(q), M);                         \
		ASSERT_EQ_FMT(z, F1(NULL), M);                      \
		ASSERT_EQ_FMT(0, F2(q, a), "%d");                   \
		ASSERT_EQ_FMT(a, F1(q), M);                         \
		ASSERT_EQ_FMT(0, rnd_queue_destroy(q, NULL), "%d"); \
		q = rnd_queue_create(sizeof(T) + 1, 1);             \
		ASSERT_NEQ(NULL, q);                                \
		q->size = 1;                                        \
		ASSERT_EQ_FMT(z, F1(q), M);                         \
		ASSERT_EQ_FMT(0, rnd_queue_destroy(q, NULL), "%d"); \
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
	PASS();
}

TEST t_pop(void)
{
	struct rnd_queue *q;

	{ /* Generic form */
		struct data a, b;
		q = rnd_queue_create(sizeof(struct data), 2);
		ASSERT_NEQ(NULL, q);
		ASSERT_EQ_FMT(RND_EILLEGAL, rnd_queue_pop(q, NULL), "%d");
		ASSERT_EQ_FMT(RND_EINVAL, rnd_queue_pop(NULL, NULL), "%d");
		data_init(&a);
		ASSERT_EQ_FMT(0, rnd_queue_push(q, &a), "%d");
		ASSERT_EQ_FMT(q->head, q->data, "%p");
		ASSERT_EQ_FMT(q->tail, q->data, "%p");
		ASSERT_EQ_FMT(0, rnd_queue_pop(q, &b), "%d");
		ASSERT_EQ_FMT(q->head, q->data, "%p");
		ASSERT_EQ_FMT(q->tail, q->data, "%p");
		ASSERT_EQ_FMT(0, data_cmp(&a, &b), "%d");
		ASSERT_EQ_FMT(0, rnd_queue_destroy(q, NULL), "%d");
		ASSERT_EQ_FMT(0, data_dtor(&b), "%d");
	}

	/* Suffixed form
	 * T  - type
	 * F1 - pop function
	 * F2 - push function
	 * V  - random value snippet
	 * M  - printf format string
	 */
#define test(T, F1, F2, V, M) do {                                  \
		T a = (V), z = 0;                                   \
		q = rnd_queue_create(sizeof(T), 2);                 \
		ASSERT_NEQ(NULL, q);                                \
		ASSERT_EQ_FMT(z, F1(q), M);                         \
		ASSERT_EQ_FMT(z, F1(NULL), M);                      \
		ASSERT_EQ_FMT(0, F2(q, a), "%d");                   \
		ASSERT_EQ_FMT(q->head, q->data, "%p");              \
		ASSERT_EQ_FMT(q->tail, q->data, "%p");              \
		ASSERT_EQ_FMT(a, F1(q), M);                         \
		ASSERT_EQ_FMT(q->head, q->data, "%p");              \
		ASSERT_EQ_FMT(q->tail, q->data, "%p");              \
		ASSERT_EQ_FMT(0, rnd_queue_destroy(q, NULL), "%d"); \
		q = rnd_queue_create(sizeof(T) + 1, 1);             \
		ASSERT_NEQ(NULL, q);                                \
		q->size = 1;                                        \
		ASSERT_EQ_FMT(z, F1(q), M);                         \
		ASSERT_EQ_FMT(0, rnd_queue_destroy(q, NULL), "%d"); \
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
	PASS();
}

TEST t_clear(void)
{
	struct rnd_queue *q;
	unsigned i;
	q = rnd_queue_create(sizeof(long double), 1000);
	ASSERT_NEQ(NULL, q);
	ASSERT_EQ_FMT(RND_EINVAL, rnd_queue_clear(NULL, NULL), "%d");
	ASSERT_EQ_FMT(0, rnd_queue_clear(q, NULL), "%d");
	ASSERT_EQ_FMT(0LU, (unsigned long)q->size, "%lu");
	ASSERT_EQ_FMT(0, rnd_queue_destroy(q, NULL), "%d");
	q = rnd_queue_create(sizeof(struct data), 1000);
	for (i = 0; i < 1000; i++) {
		struct data d;
		ASSERT_EQ_FMT(0, data_init(&d), "%d");
		ASSERT_EQ_FMT(0, rnd_queue_push(q, &d), "%d");
	}
	ASSERT_EQ_FMT(RND_EHANDLER, rnd_queue_clear(q, data_dtor_bad), "%d");
	ASSERT_EQ_FMT(0, rnd_queue_clear(q, data_dtor), "%d");
	ASSERT_EQ_FMT(0LU, (unsigned long)q->size, "%lu");
	ASSERT_EQ_FMT(0, rnd_queue_destroy(q, NULL), "%d");
	PASS();
}

TEST t_map(void)
{
	struct rnd_queue *q;
	unsigned i;
	q = rnd_queue_create(sizeof(long double), 1000);
	ASSERT_NEQ(NULL, q);
	ASSERT_EQ_FMT(RND_EINVAL, rnd_queue_map(NULL, data_mutate), "%d");
	ASSERT_EQ_FMT(RND_EINVAL, rnd_queue_map(q, NULL), "%d");
	ASSERT_EQ_FMT(RND_EINVAL, rnd_queue_map(NULL, NULL), "%d");
	ASSERT_EQ_FMT(0, rnd_queue_destroy(q, NULL), "%d");
	q = rnd_queue_create(sizeof(struct data), 1000);
	for (i = 0; i < 1000; i++) {
		struct data d;
		ASSERT_EQ_FMT(0, data_init(&d), "%d");
		ASSERT_EQ_FMT(0, rnd_queue_push(q, &d), "%d");
	}
	ASSERT_EQ_FMT(RND_EHANDLER, rnd_queue_map(q, data_mutate_bad), "%d");
	ASSERT_EQ_FMT(0, rnd_queue_map(q, data_mutate), "%d");
	ASSERT_EQ_FMT(0, rnd_queue_map(q, data_verify), "%d");
	ASSERT_EQ_FMT(0, rnd_queue_destroy(q, data_dtor), "%d");
	PASS();
}

TEST t_copy(void)
{
	struct rnd_queue *q, *p;
	unsigned i;
	q = rnd_queue_create(sizeof(int), 1000);
	p = rnd_queue_create(sizeof(int), 333);
	ASSERT_NEQ(NULL, q);
	ASSERT_EQ_FMT(RND_EINVAL, rnd_queue_copy(NULL, q, NULL), "%d");
	ASSERT_EQ_FMT(RND_EINVAL, rnd_queue_copy(p, NULL, NULL), "%d");
	ASSERT_EQ_FMT(RND_EINVAL, rnd_queue_copy(NULL, NULL, NULL), "%d");
	ASSERT_EQ_FMT(0, rnd_queue_copy(q, p, NULL), "%d");
	ASSERT_EQ_FMT((unsigned long)p->size, (unsigned long)q->size, "%lu");
	ASSERT_EQ_FMT((unsigned long)p->elem_size, (unsigned long)q->elem_size, "%lu");
	ASSERT_EQ_FMT(0, rnd_queue_copy(p, q, NULL), "%d");
	ASSERT_EQ_FMT((unsigned long)p->size, (unsigned long)q->size, "%lu");
	ASSERT_EQ_FMT((unsigned long)p->elem_size, (unsigned long)q->elem_size, "%lu");
	for (i = 0; i < 1000; i++) {
		ASSERT_EQ_FMT(0, rnd_queue_pushi(q, FRANGE(INT_MIN, INT_MAX)), "%d");
	}
	ASSERT_EQ_FMT(0, rnd_queue_copy(p, q, NULL), "%d");
	ASSERT_EQ_FMT((unsigned long)p->size, (unsigned long)q->size, "%lu");
	ASSERT_EQ_FMT((unsigned long)p->elem_size, (unsigned long)q->elem_size, "%lu");
	for (i = 0; i < 1000; i++) {
		int a, b;
		a = rnd_queue_geti(q, i);
		b = rnd_queue_geti(p, i);
		ASSERT_EQ_FMT(a, b, "%d");
	}
	ASSERT_EQ_FMT(0, rnd_queue_destroy(q, NULL), "%d");
	ASSERT_EQ_FMT(0, rnd_queue_clear(p, NULL), "%d");
	q = rnd_queue_create(sizeof(struct data), 1000);
	for (i = 0; i < 1000; i++) {
		struct data d;
		ASSERT_EQ_FMT(0, data_init(&d), "%d");
		ASSERT_EQ_FMT(0, rnd_queue_push(q, &d), "%d");
	}
	ASSERT_EQ_FMT(RND_EHANDLER, rnd_queue_copy(p, q, data_cpy_bad), "%d");
	ASSERT_EQ_FMT(0, rnd_queue_copy(p, q, data_cpy), "%d");
	ASSERT_EQ_FMT((unsigned long)p->size, (unsigned long)q->size, "%lu");
	ASSERT_EQ_FMT((unsigned long)p->elem_size, (unsigned long)q->elem_size, "%lu");
	for (i = 0; i < 1000; i++) {
		struct data a, b;
		ASSERT_EQ_FMT(0, rnd_queue_get(q, i, &a), "%d");
		ASSERT_EQ_FMT(0, rnd_queue_get(p, i, &b), "%d");
		ASSERT_EQ_FMT(0, data_cmp(&a, &b), "%d");
	}
	ASSERT_EQ_FMT(0, rnd_queue_destroy(q, data_dtor), "%d");
	ASSERT_EQ_FMT(0, rnd_queue_destroy(p, data_dtor), "%d");
	PASS();
}

TEST t_insert(void)
{
	struct rnd_queue *q;
	unsigned i;

	{ /* Generic form */
		int a = 10;
		struct data d[1000];
		q = rnd_queue_create(sizeof(int), 1000);
		ASSERT_NEQ(NULL, q);
		ASSERT_EQ_FMT(RND_EINVAL, rnd_queue_insert(q, 0, NULL), "%d");
		ASSERT_EQ_FMT(RND_EINVAL, rnd_queue_insert(NULL, 0, &a), "%d");
		ASSERT_EQ_FMT(RND_EINVAL, rnd_queue_insert(NULL, 0, NULL), "%d");
		ASSERT_EQ_FMT(RND_EINDEX, rnd_queue_insert(q, 1, &a), "%d");
		ASSERT_EQ_FMT(0, rnd_queue_insert(q, 0, &a), "%d");
		ASSERT_EQ_FMT(1LU, q->size, "%lu");
		ASSERT_EQ_FMT(10, rnd_queue_geti(q, 0), "%d");
		ASSERT_EQ_FMT(0, rnd_queue_destroy(q, NULL), "%d");

		q = rnd_queue_create(sizeof(struct data), 1000);
		ASSERT_NEQ(NULL, q);
		for (i = 0; i < 1000; i++) {
			struct data a;
			size_t idx = IRANGE(0, i);
			ASSERT_EQ_FMT(0, data_init(d + i), "%d");
			ASSERT_EQ_FMT(0, rnd_queue_insert(q, idx, d + i), "%d");
			ASSERT_EQ_FMT((unsigned long)i + 1, (unsigned long)q->size, "%lu");
			ASSERT_EQ_FMT(0, rnd_queue_get(q, idx, &a), "%d");
			ASSERT_EQ_FMT(0, data_cmp(&a, d + i), "%d");
		}
		ASSERT_EQ_FMT(0, rnd_queue_destroy(q, data_dtor), "%d");

		q = rnd_queue_create(sizeof(struct data), 10);
		ASSERT_NEQ(NULL, q);
		for (i = 0; i < 5; i++) {
			ASSERT_EQ_FMT(0, data_init(d + i), "%d");
			ASSERT_EQ_FMT(0, rnd_queue_insert(q, i, d + i), "%d");
		}
		for (i = 0; i < 5; i++) {
			struct data a;
			ASSERT_EQ_FMT(0, rnd_queue_get(q, i, &a), "%d");
			ASSERT_EQ_FMT(0, data_cmp(&a, d + i), "%d");
		}
		ASSERT_EQ_FMT(0, data_init(d + 5), "%d");
		ASSERT_EQ_FMT(0, rnd_queue_insert(q, 1, d + 5), "%d");
		ASSERT_EQ_FMT(q->head, (char*)q->data + (q->capacity - 1) * q->elem_size, "%p");
		ASSERT_EQ_FMT(0, data_init(d + 6), "%d");
		ASSERT_EQ_FMT(0, rnd_queue_insert(q, 3, d + 6), "%d");
		ASSERT_EQ_FMT(q->tail, (char*)q->data + 5 * q->elem_size, "%p");
		ASSERT_EQ_FMT(0, rnd_queue_destroy(q, data_dtor), "%d");
	}

	/* Suffixed form
	 * T  - type
	 * F1 - insert function
	 * F2 - get function
	 * V  - random value snippet
	 * M  - printf format string
	 */
#define test(T, F1, F2, V, M) do {                                                               \
		T a = (V);                                                                       \
		T d[1000];                                                                       \
		q = rnd_queue_create(sizeof(T), 1000);                                           \
		ASSERT_NEQ(NULL, q);                                                             \
		ASSERT_EQ_FMT(RND_EINVAL, F1(NULL, 0, a), "%d");                                 \
		ASSERT_EQ_FMT(RND_EINDEX, F1(q, 1, a), "%d");                                    \
		ASSERT_EQ_FMT(0, F1(q, 0, a), "%d");                                             \
		ASSERT_EQ_FMT(1LU, q->size, "%lu");                                              \
		ASSERT_EQ_FMT(a, F2(q, 0), M);                                                   \
		ASSERT_EQ_FMT(0, rnd_queue_destroy(q, NULL), "%d");                              \
                                                                                                 \
		q = rnd_queue_create(sizeof(T), 1000);                                           \
		ASSERT_NEQ(NULL, q);                                                             \
		for (i = 0; i < 1000; i++) {                                                     \
			size_t idx = IRANGE(0, i);                                               \
			d[i] = (V);                                                              \
			ASSERT_EQ_FMT(0, F1(q, idx, d[i]), "%d");                                \
			ASSERT_EQ_FMT((unsigned long)i + 1, (unsigned long)q->size, "%lu");      \
			ASSERT_EQ_FMT(d[i], F2(q, idx), M);                                      \
		}                                                                                \
		ASSERT_EQ_FMT(0, rnd_queue_destroy(q, NULL), "%d");                              \
                                                                                                 \
		q = rnd_queue_create(sizeof(T), 10);                                             \
		ASSERT_NEQ(NULL, q);                                                             \
		for (i = 0; i < 5; i++) {                                                        \
			d[i] = (V);                                                              \
			ASSERT_EQ_FMT(0, F1(q, i, d[i]), "%d");                                  \
		}                                                                                \
		for (i = 0; i < 5; i++) {                                                        \
			ASSERT_EQ_FMT(d[i], F2(q, i), M);                                        \
		}                                                                                \
		d[5] = (V);                                                                      \
		ASSERT_EQ_FMT(0, F1(q, 1, d[5]), "%d");                                          \
		ASSERT_EQ_FMT(q->head, (char*)q->data + (q->capacity - 1) * q->elem_size, "%p"); \
		d[6] = (V);                                                                      \
		ASSERT_EQ_FMT(0, F1(q, 3, d[6]), "%d");                                          \
		ASSERT_EQ_FMT(q->tail, (char*)q->data + 5 * q->elem_size, "%p");                 \
		ASSERT_EQ_FMT(0, rnd_queue_destroy(q, NULL), "%d");                              \
		q = rnd_queue_create(sizeof(T) + 1, 1000);                                       \
		ASSERT_NEQ(NULL, q);                                                             \
		ASSERT_EQ_FMT(RND_EILLEGAL, F1(q, 0, (V)), "%d");                                \
		ASSERT_EQ_FMT(0, rnd_queue_destroy(q, NULL), "%d");                              \
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
	PASS();
}

TEST t_quickinsert(void)
{
	struct rnd_queue *q;
	unsigned i;

	{ /* Generic form */
		int a = 10;
		struct data d[1000];
		q = rnd_queue_create(sizeof(int), 1000);
		ASSERT_NEQ(NULL, q);
		ASSERT_EQ_FMT(RND_EINVAL, rnd_queue_quickinsert(q, 0, NULL), "%d");
		ASSERT_EQ_FMT(RND_EINVAL, rnd_queue_quickinsert(NULL, 0, &a), "%d");
		ASSERT_EQ_FMT(RND_EINVAL, rnd_queue_quickinsert(NULL, 0, NULL), "%d");
		ASSERT_EQ_FMT(RND_EINDEX, rnd_queue_quickinsert(q, 1, &a), "%d");
		ASSERT_EQ_FMT(0, rnd_queue_quickinsert(q, 0, &a), "%d");
		ASSERT_EQ_FMT(1LU, q->size, "%lu");
		ASSERT_EQ_FMT(10, rnd_queue_geti(q, 0), "%d");
		ASSERT_EQ_FMT(0, rnd_queue_destroy(q, NULL), "%d");

		q = rnd_queue_create(sizeof(struct data), 1000);
		ASSERT_NEQ(NULL, q);
		for (i = 0; i < 1000; i++) {
			struct data a;
			size_t idx = IRANGE(0, i);
			ASSERT_EQ_FMT(0, data_init(d + i), "%d");
			ASSERT_EQ_FMT(0, rnd_queue_quickinsert(q, idx, d + i), "%d");
			ASSERT_EQ_FMT((unsigned long)i + 1, (unsigned long)q->size, "%lu");
			ASSERT_EQ_FMT(0, rnd_queue_get(q, idx, &a), "%d");
			ASSERT_EQ_FMT(0, data_cmp(&a, d + i), "%d");
		}
		ASSERT_EQ_FMT(0, rnd_queue_destroy(q, data_dtor), "%d");

		q = rnd_queue_create(sizeof(struct data), 10);
		ASSERT_NEQ(NULL, q);
		for (i = 0; i < 5; i++) {
			ASSERT_EQ_FMT(0, data_init(d + i), "%d");
			ASSERT_EQ_FMT(0, rnd_queue_quickinsert(q, i, d + i), "%d");
		}
		for (i = 0; i < 5; i++) {
			struct data a;
			ASSERT_EQ_FMT(0, rnd_queue_get(q, i, &a), "%d");
			ASSERT_EQ_FMT(0, data_cmp(&a, d + i), "%d");
		}
		ASSERT_EQ_FMT(0, data_init(d + 5), "%d");
		ASSERT_EQ_FMT(0, rnd_queue_quickinsert(q, 1, d + 5), "%d");
		ASSERT_EQ_FMT(q->tail, (char*)q->data + 5 * q->elem_size, "%p");
		ASSERT_EQ_FMT(0, data_init(d + 6), "%d");
		ASSERT_EQ_FMT(0, rnd_queue_quickinsert(q, 3, d + 6), "%d");
		ASSERT_EQ_FMT(q->tail, (char*)q->data + 6 * q->elem_size, "%p");
		ASSERT_EQ_FMT(0, rnd_queue_destroy(q, data_dtor), "%d");
	}

	/* Suffixed form
	 * T  - type
	 * F1 - quickinsert function
	 * F2 - get function
	 * V  - random value snippet
	 * M  - printf format string
	 */
#define test(T, F1, F2, V, M) do {                                                               \
		T a = (V);                                                                       \
		T d[1000];                                                                       \
		q = rnd_queue_create(sizeof(T), 1000);                                           \
		ASSERT_NEQ(NULL, q);                                                             \
		ASSERT_EQ_FMT(RND_EINVAL, F1(NULL, 0, a), "%d");                                 \
		ASSERT_EQ_FMT(RND_EINDEX, F1(q, 1, a), "%d");                                    \
		ASSERT_EQ_FMT(0, F1(q, 0, a), "%d");                                             \
		ASSERT_EQ_FMT(1LU, q->size, "%lu");                                              \
		ASSERT_EQ_FMT(a, F2(q, 0), M);                                                   \
		ASSERT_EQ_FMT(0, rnd_queue_destroy(q, NULL), "%d");                              \
                                                                                                 \
		q = rnd_queue_create(sizeof(T), 1000);                                           \
		ASSERT_NEQ(NULL, q);                                                             \
		for (i = 0; i < 1000; i++) {                                                     \
			size_t idx = IRANGE(0, i);                                               \
			d[i] = (V);                                                              \
			ASSERT_EQ_FMT(0, F1(q, idx, d[i]), "%d");                                \
			ASSERT_EQ_FMT((unsigned long)i + 1, (unsigned long)q->size, "%lu");      \
			ASSERT_EQ_FMT(d[i], F2(q, idx), M);                                      \
		}                                                                                \
		ASSERT_EQ_FMT(0, rnd_queue_destroy(q, NULL), "%d");                              \
                                                                                                 \
		q = rnd_queue_create(sizeof(T), 10);                                             \
		ASSERT_NEQ(NULL, q);                                                             \
		for (i = 0; i < 5; i++) {                                                        \
			d[i] = (V);                                                              \
			ASSERT_EQ_FMT(0, F1(q, i, d[i]), "%d");                                  \
		}                                                                                \
		for (i = 0; i < 5; i++) {                                                        \
			ASSERT_EQ_FMT(d[i], F2(q, i), M);                                        \
		}                                                                                \
		d[5] = (V);                                                                      \
		ASSERT_EQ_FMT(0, F1(q, 1, d[5]), "%d");                                          \
		ASSERT_EQ_FMT(q->tail, (char*)q->data + 5 * q->elem_size, "%p");                 \
		d[6] = (V);                                                                      \
		ASSERT_EQ_FMT(0, F1(q, 3, d[6]), "%d");                                          \
		ASSERT_EQ_FMT(q->tail, (char*)q->data + 6 * q->elem_size, "%p");                 \
		ASSERT_EQ_FMT(0, rnd_queue_destroy(q, NULL), "%d");                              \
		q = rnd_queue_create(sizeof(T) + 1, 1000);                                       \
		ASSERT_NEQ(NULL, q);                                                             \
		ASSERT_EQ_FMT(RND_EILLEGAL, F1(q, 0, (V)), "%d");                                \
		ASSERT_EQ_FMT(0, rnd_queue_destroy(q, NULL), "%d");                              \
	} while (0)
	test(char          , rnd_queue_quickinsertc , rnd_queue_getc , IRANGE(1, CHAR_MAX) , "%hd");
	test(short         , rnd_queue_quickinserts , rnd_queue_gets , IRANGE(1, SHRT_MAX) , "%hd");
	test(int           , rnd_queue_quickinserti , rnd_queue_geti , FRANGE(1, INT_MAX)  , "%d");
	test(long          , rnd_queue_quickinsertl , rnd_queue_getl , FRANGE(1, LONG_MAX) , "%ld");
	test(signed char   , rnd_queue_quickinsertsc, rnd_queue_getsc, IRANGE(1, SCHAR_MAX), "%hd");
	test(unsigned char , rnd_queue_quickinsertuc, rnd_queue_getuc, IRANGE(1, UCHAR_MAX), "%hd");
	test(unsigned short, rnd_queue_quickinsertus, rnd_queue_getus, IRANGE(1, USHRT_MAX), "%hu");
	test(unsigned int  , rnd_queue_quickinsertui, rnd_queue_getui, FRANGE(1, UINT_MAX) , "%u");
	test(unsigned long , rnd_queue_quickinsertul, rnd_queue_getul, FRANGE(1, ULONG_MAX), "%lu");
	test(float         , rnd_queue_quickinsertf , rnd_queue_getf , FRANGE(1, FLT_MAX)  , "%f");
	test(double        , rnd_queue_quickinsertd , rnd_queue_getd , FRANGE(1, DBL_MAX)  , "%f");
	test(long double   , rnd_queue_quickinsertld, rnd_queue_getld, FRANGE(1, LDBL_MAX) , "%Lf");
#undef test
	PASS();
}

TEST t_remove(void)
{
	struct rnd_queue *q;
	unsigned i;

	{ /* Generic form */
		struct data a;
		struct data d[100];
		q = rnd_queue_create(sizeof(int), 1000);
		ASSERT_NEQ(NULL, q);
		ASSERT_EQ_FMT(RND_EINDEX, rnd_queue_remove(q, 0, &a), "%d");
		ASSERT_EQ_FMT(0, data_init(&a), "%d");
		ASSERT_EQ_FMT(0, rnd_queue_push(q, &a), "%d");
		ASSERT_EQ_FMT(RND_EINVAL, rnd_queue_remove(NULL, 0, &a), "%d");
		ASSERT_EQ_FMT(RND_EINVAL, rnd_queue_remove(NULL, 0, NULL), "%d");
		ASSERT_EQ_FMT(RND_EINDEX, rnd_queue_remove(q, 1, &a), "%d");
		ASSERT_EQ_FMT(q->data, q->head, "%p");
		ASSERT_EQ_FMT(q->data, q->tail, "%p");
		ASSERT_EQ_FMT(0, rnd_queue_remove(q, 0, NULL), "%d");
		ASSERT_EQ_FMT(0LU, q->size, "%lu");
		ASSERT_EQ_FMT(q->data, q->head, "%p");
		ASSERT_EQ_FMT(q->data, q->tail, "%p");
		ASSERT_EQ_FMT(0, rnd_queue_destroy(q, NULL), "%d");
		ASSERT_EQ_FMT(0, data_dtor(&a), "%d");

		q = rnd_queue_create(sizeof(struct data), 100);
		ASSERT_NEQ(NULL, q);
		for (i = 0; i < 100; i++) {
			ASSERT_EQ_FMT(0, data_init(d + i), "%d");
			ASSERT_EQ_FMT(0, rnd_queue_push(q, d + i), "%d");
		}
		for (i = 0; i < 100; i++) {
			struct data a;
			size_t idx = IRANGE(0, q->size - 1), j;
			int found = 0;
			ASSERT_EQ_FMT(0, rnd_queue_remove(q, idx, &a), "%d");
			for (j = 0; j < 100; j++) {
				/* Lazy and slow way to check, but on average it's
				 * enough */
				if (data_cmp(&a, d + j) == 0) {
					found = 1;
					break;
				}
			}
			ASSERT_EQ_FMT(1, found, "%d");
		}
		ASSERT_EQ_FMT(0, rnd_queue_destroy(q, NULL), "%d");

		q = rnd_queue_create(sizeof(struct data), 10);
		ASSERT_NEQ(NULL, q);
		for (i = 0; i < 8; i++) {
			ASSERT_EQ_FMT(0, rnd_queue_push(q, d + i), "%d");
		}
		ASSERT_EQ_FMT(0, rnd_queue_remove(q, 3, &a), "%d");
		ASSERT_EQ_FMT(0, data_cmp(d + 3, &a), "%d");
		ASSERT_EQ_FMT(q->head, (char*)q->data + q->elem_size, "%p");
		ASSERT_EQ_FMT(0, rnd_queue_remove(q, 0, &a), "%d");
		ASSERT_EQ_FMT(0, data_cmp(d, &a), "%d");
		ASSERT_EQ_FMT(q->head, (char*)q->data + 2 * q->elem_size, "%p");
		ASSERT_EQ_FMT(0, rnd_queue_remove(q, 3, &a), "%d");
		ASSERT_EQ_FMT(0, data_cmp(d + 5, &a), "%d");
		ASSERT_EQ_FMT(q->tail, (char*)q->data + 6 * q->elem_size, "%p");
		ASSERT_EQ_FMT(0, rnd_queue_destroy(q, NULL), "%d");
		for (i = 0; i < 100; i++) {
			ASSERT_EQ_FMT(0, data_dtor(d + i), "%d");
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
		ASSERT_NEQ(NULL, q);                                               \
		ASSERT_EQ_FMT(z, F1(q, 0), M);                                     \
		ASSERT_EQ_FMT(0, F2(q, a), "%d");                                  \
		ASSERT_EQ_FMT(z, F1(NULL, 0), M);                                  \
		ASSERT_EQ_FMT(z, F1(q, 1), M);                                     \
		ASSERT_EQ_FMT(q->data, q->head, "%p");                             \
		ASSERT_EQ_FMT(q->data, q->tail, "%p");                             \
		ASSERT_EQ_FMT(a, F1(q, 0), M);                                     \
		ASSERT_EQ_FMT(0LU, q->size, "%lu");                                \
		ASSERT_EQ_FMT(q->data, q->head, "%p");                             \
		ASSERT_EQ_FMT(q->data, q->tail, "%p");                             \
		ASSERT_EQ_FMT(0, rnd_queue_destroy(q, NULL), "%d");                \
                                                                                   \
		q = rnd_queue_create(sizeof(T), 100);                              \
		ASSERT_NEQ(NULL, q);                                               \
		for (i = 0; i < 100; i++) {                                        \
			d[i] = (V);                                                \
			ASSERT_EQ_FMT(0, F2(q, d[i]), "%d");                       \
		}                                                                  \
		for (i = 0; i < 100; i++) {                                        \
			T a;                                                       \
			size_t idx = IRANGE(0, q->size - 1), j;                    \
			int found = 0;                                             \
			ASSERT_NEQ(0, (a = F1(q, idx)));                           \
			for (j = 0; j < 100; j++) {                                \
				/* Lazy and slow way to check, but on average it's
				 * enough */                                       \
				if (a == d[j]) {                                   \
					found = 1;                                 \
					break;                                     \
				}                                                  \
			}                                                          \
			ASSERT_EQ_FMT(1, found, "%d");                             \
		}                                                                  \
		ASSERT_EQ_FMT(0, rnd_queue_destroy(q, NULL), "%d");                \
                                                                                   \
		q = rnd_queue_create(sizeof(T), 10);                               \
		ASSERT_NEQ(NULL, q);                                               \
		for (i = 0; i < 8; i++) {                                          \
			ASSERT_EQ_FMT(0, F2(q, d[i]), "%d");                       \
		}                                                                  \
		ASSERT_EQ_FMT(d[3], F1(q, 3), M);                                  \
		ASSERT_EQ_FMT(q->head, (char*)q->data + q->elem_size, "%p");       \
		ASSERT_EQ_FMT(d[0], F1(q, 0), M);                                  \
		ASSERT_EQ_FMT(q->head, (char*)q->data + 2 * q->elem_size, "%p");   \
		ASSERT_EQ_FMT(d[5], F1(q, 3), M);                                  \
		ASSERT_EQ_FMT(q->tail, (char*)q->data + 6 * q->elem_size, "%p");   \
		ASSERT_EQ_FMT(0, rnd_queue_destroy(q, NULL), "%d");                \
		q = rnd_queue_create(sizeof(T) + 1, 1000);                         \
		ASSERT_NEQ(NULL, q);                                               \
		q->size = 1;                                                       \
		ASSERT_EQ_FMT(z, F1(q, 0), M);                                     \
		ASSERT_EQ_FMT(0, rnd_queue_destroy(q, NULL), "%d");                \
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
	PASS();
}

TEST t_quickremove(void)
{
	struct rnd_queue *q;
	unsigned i;

	{ /* Generic form */
		struct data a;
		struct data d[100];
		q = rnd_queue_create(sizeof(int), 1000);
		ASSERT_NEQ(NULL, q);
		ASSERT_EQ_FMT(RND_EINDEX, rnd_queue_quickremove(q, 0, &a), "%d");
		ASSERT_EQ_FMT(0, data_init(&a), "%d");
		ASSERT_EQ_FMT(0, rnd_queue_push(q, &a), "%d");
		ASSERT_EQ_FMT(RND_EINVAL, rnd_queue_quickremove(NULL, 0, &a), "%d");
		ASSERT_EQ_FMT(RND_EINVAL, rnd_queue_quickremove(NULL, 0, NULL), "%d");
		ASSERT_EQ_FMT(RND_EINDEX, rnd_queue_quickremove(q, 1, &a), "%d");
		ASSERT_EQ_FMT(q->data, q->head, "%p");
		ASSERT_EQ_FMT(q->data, q->tail, "%p");
		ASSERT_EQ_FMT(0, rnd_queue_quickremove(q, 0, NULL), "%d");
		ASSERT_EQ_FMT(0LU, q->size, "%lu");
		ASSERT_EQ_FMT(q->data, q->head, "%p");
		ASSERT_EQ_FMT(q->data, q->tail, "%p");
		ASSERT_EQ_FMT(0, rnd_queue_destroy(q, NULL), "%d");
		ASSERT_EQ_FMT(0, data_dtor(&a), "%d");

		q = rnd_queue_create(sizeof(struct data), 100);
		ASSERT_NEQ(NULL, q);
		for (i = 0; i < 100; i++) {
			ASSERT_EQ_FMT(0, data_init(d + i), "%d");
			ASSERT_EQ_FMT(0, rnd_queue_push(q, d + i), "%d");
		}
		for (i = 0; i < 100; i++) {
			struct data a;
			size_t idx = IRANGE(0, q->size - 1), j;
			int found = 0;
			ASSERT_EQ_FMT(0, rnd_queue_quickremove(q, idx, &a), "%d");
			for (j = 0; j < 100; j++) {
				/* Lazy and slow way to check, but on average it's
				 * enough */
				if (data_cmp(&a, d + j) == 0) {
					found = 1;
					break;
				}
			}
			ASSERT_EQ_FMT(1, found, "%d");
		}
		ASSERT_EQ_FMT(0, rnd_queue_destroy(q, NULL), "%d");

		q = rnd_queue_create(sizeof(struct data), 10);
		ASSERT_NEQ(NULL, q);
		for (i = 0; i < 8; i++) {
			ASSERT_EQ_FMT(0, rnd_queue_push(q, d + i), "%d");
		}
		ASSERT_EQ_FMT(0, rnd_queue_quickremove(q, 3, &a), "%d");
		ASSERT_EQ_FMT(0, data_cmp(d + 3, &a), "%d");
		ASSERT_EQ_FMT(0, rnd_queue_get(q, 3, &a), "%d");
		ASSERT_EQ_FMT(0, data_cmp(d + 7, &a), "%d");
		ASSERT_EQ_FMT(0, rnd_queue_quickremove(q, 0, &a), "%d");
		ASSERT_EQ_FMT(0, data_cmp(d, &a), "%d");
		ASSERT_EQ_FMT(0, rnd_queue_get(q, 0, &a), "%d");
		ASSERT_EQ_FMT(0, data_cmp(d + 6, &a), "%d");
		ASSERT_EQ_FMT(0, rnd_queue_quickremove(q, 3, &a), "%d");
		ASSERT_EQ_FMT(0, data_cmp(d + 7, &a), "%d");
		ASSERT_EQ_FMT(q->tail, (char*)q->data + 4 * q->elem_size, "%p");
		ASSERT_EQ_FMT(0, rnd_queue_destroy(q, NULL), "%d");
		for (i = 0; i < 100; i++) {
			ASSERT_EQ_FMT(0, data_dtor(d + i), "%d");
		}
	}

	/* Suffixed form
	 * T  - type
	 * F1 - remove function
	 * F2 - push function
	 * F3 - get function
	 * V  - random value snippet
	 * M  - printf format string
	 */
#define test(T, F1, F2, F3, V, M) do {                                                 \
		T a = (V), z = 0;                                                  \
		T d[100];                                                          \
		q = rnd_queue_create(sizeof(T), 1000);                             \
		ASSERT_NEQ(NULL, q);                                               \
		ASSERT_EQ_FMT(z, F1(q, 0), M);                                     \
		ASSERT_EQ_FMT(0, F2(q, a), "%d");                                  \
		ASSERT_EQ_FMT(z, F1(NULL, 0), M);                                  \
		ASSERT_EQ_FMT(z, F1(q, 1), M);                                     \
		ASSERT_EQ_FMT(q->data, q->head, "%p");                             \
		ASSERT_EQ_FMT(q->data, q->tail, "%p");                             \
		ASSERT_EQ_FMT(a, F1(q, 0), M);                                     \
		ASSERT_EQ_FMT(0LU, q->size, "%lu");                                \
		ASSERT_EQ_FMT(q->data, q->head, "%p");                             \
		ASSERT_EQ_FMT(q->data, q->tail, "%p");                             \
		ASSERT_EQ_FMT(0, rnd_queue_destroy(q, NULL), "%d");                \
                                                                                   \
		q = rnd_queue_create(sizeof(T), 100);                              \
		ASSERT_NEQ(NULL, q);                                               \
		for (i = 0; i < 100; i++) {                                        \
			d[i] = (V);                                                \
			ASSERT_EQ_FMT(0, F2(q, d[i]), "%d");                       \
		}                                                                  \
		for (i = 0; i < 100; i++) {                                        \
			T a;                                                       \
			size_t idx = IRANGE(0, q->size - 1), j;                    \
			int found = 0;                                             \
			ASSERT_NEQ(0, (a = F1(q, idx)));                           \
			for (j = 0; j < 100; j++) {                                \
				/* Lazy and slow way to check, but on average it's
				 * enough */                                       \
				if (a == d[j]) {                                   \
					found = 1;                                 \
					break;                                     \
				}                                                  \
			}                                                          \
			ASSERT_EQ_FMT(1, found, "%d");                             \
		}                                                                  \
		ASSERT_EQ_FMT(0, rnd_queue_destroy(q, NULL), "%d");                \
                                                                                   \
		q = rnd_queue_create(sizeof(T), 10);                               \
		ASSERT_NEQ(NULL, q);                                               \
		for (i = 0; i < 8; i++) {                                          \
			ASSERT_EQ_FMT(0, F2(q, d[i]), "%d");                       \
		}                                                                  \
		ASSERT_EQ_FMT(d[3], F1(q, 3), M);                                  \
		ASSERT_EQ_FMT(d[7], F3(q, 3), M);                                  \
		ASSERT_EQ_FMT(d[0], F1(q, 0), M);                                  \
		ASSERT_EQ_FMT(d[6], F3(q, 0), M);                                  \
		ASSERT_EQ_FMT(d[7], F1(q, 3), M);                                  \
		ASSERT_EQ_FMT(d[5], F3(q, 3), M);                                  \
		ASSERT_EQ_FMT(q->tail, (char*)q->data + 4 * q->elem_size, "%p");   \
		ASSERT_EQ_FMT(0, rnd_queue_destroy(q, NULL), "%d");                \
		q = rnd_queue_create(sizeof(T) + 1, 1000);                         \
		ASSERT_NEQ(NULL, q);                                               \
		q->size = 1;                                                       \
		ASSERT_EQ_FMT(z, F1(q, 0), M);                                     \
		ASSERT_EQ_FMT(0, rnd_queue_destroy(q, NULL), "%d");                \
	} while (0)
	test(char          , rnd_queue_quickremovec , rnd_queue_pushc , rnd_queue_getc , IRANGE(1, CHAR_MAX) , "%hd");
	test(short         , rnd_queue_quickremoves , rnd_queue_pushs , rnd_queue_gets , IRANGE(1, SHRT_MAX) , "%hd");
	test(int           , rnd_queue_quickremovei , rnd_queue_pushi , rnd_queue_geti , FRANGE(1, INT_MAX)  , "%d");
	test(long          , rnd_queue_quickremovel , rnd_queue_pushl , rnd_queue_getl , FRANGE(1, LONG_MAX) , "%ld");
	test(signed char   , rnd_queue_quickremovesc, rnd_queue_pushsc, rnd_queue_getsc, IRANGE(1, SCHAR_MAX), "%hd");
	test(unsigned char , rnd_queue_quickremoveuc, rnd_queue_pushuc, rnd_queue_getuc, IRANGE(1, UCHAR_MAX), "%hd");
	test(unsigned short, rnd_queue_quickremoveus, rnd_queue_pushus, rnd_queue_getus, IRANGE(1, USHRT_MAX), "%hu");
	test(unsigned int  , rnd_queue_quickremoveui, rnd_queue_pushui, rnd_queue_getui, FRANGE(1, UINT_MAX) , "%u");
	test(unsigned long , rnd_queue_quickremoveul, rnd_queue_pushul, rnd_queue_getul, FRANGE(1, ULONG_MAX), "%lu");
	test(float         , rnd_queue_quickremovef , rnd_queue_pushf , rnd_queue_getf , FRANGE(1, FLT_MAX)  , "%f");
	test(double        , rnd_queue_quickremoved , rnd_queue_pushd , rnd_queue_getd , FRANGE(1, DBL_MAX)  , "%f");
	test(long double   , rnd_queue_quickremoveld, rnd_queue_pushld, rnd_queue_getld, FRANGE(1, LDBL_MAX) , "%Lf");
#undef test
	PASS();
}

TEST t_get(void)
{
	struct rnd_queue *q;

	{ /* Generic form */
		unsigned i;
		int a = 0;
		struct data d[1000];
		q = rnd_queue_create(sizeof(int), 1000);
		ASSERT_NEQ(NULL, q);
		ASSERT_EQ_FMT(RND_EINDEX, rnd_queue_get(q, 0, &a), "%d");
		ASSERT_EQ_FMT(0, rnd_queue_pushi(q, 10), "%d");
		ASSERT_EQ_FMT(RND_EINVAL, rnd_queue_get(NULL, 0, &a), "%d");
		ASSERT_EQ_FMT(RND_EINVAL, rnd_queue_get(q, 0, NULL), "%d");
		ASSERT_EQ_FMT(RND_EINVAL, rnd_queue_get(NULL, 0, NULL), "%d");
		ASSERT_EQ_FMT(RND_EINDEX, rnd_queue_get(q, 1, &a), "%d");
		ASSERT_EQ_FMT(0, rnd_queue_get(q, 0, &a), "%d");
		ASSERT_EQ_FMT(10, a, "%d");
		ASSERT_EQ_FMT(0, rnd_queue_destroy(q, NULL), "%d");
		q = rnd_queue_create(sizeof(struct data), 1000);
		ASSERT_NEQ(NULL, q);
		for (i = 0; i < 1000; i++) {
			ASSERT_EQ_FMT(0, data_init(d + i), "%d");
			ASSERT_EQ_FMT(0, rnd_queue_push(q, d + i), "%d");
		}
		for (i = 0; i < 1000; i++) {
			struct data a;
			ASSERT_EQ_FMT(0, rnd_queue_get(q, i, &a), "%d");
			ASSERT_EQ_FMT(0, data_cmp(&a, d + i), "%d");
		}
		ASSERT_EQ_FMT(0, rnd_queue_destroy(q, data_dtor), "%d");
	}

	/* Suffixed form
	 * T  - type
	 * F1 - get function
	 * F2 - push function
	 * V  - random value snippet
	 * M  - printf format string
	 */
#define test(T, F1, F2, V, M) do {                                  \
		unsigned i;                                         \
		T a = (V), z = 0;                                   \
		T d[1000];                                          \
		q = rnd_queue_create(sizeof(T), 1000);              \
		ASSERT_NEQ(NULL, q);                                \
		ASSERT_EQ_FMT(z, F1(q, 0), M);                      \
		ASSERT_EQ_FMT(0, F2(q, a), "%d");                   \
		ASSERT_EQ_FMT(z, F1(NULL, 0), M);                   \
		ASSERT_EQ_FMT(z, F1(q, 1), M);                      \
		ASSERT_EQ_FMT(a, F1(q, 0), M);                      \
		ASSERT_EQ_FMT(0, rnd_queue_destroy(q, NULL), "%d"); \
		q = rnd_queue_create(sizeof(T), 1000);              \
		ASSERT_NEQ(NULL, q);                                \
		for (i = 0; i < 1000; i++) {                        \
			d[i] = (V);                                 \
			ASSERT_EQ_FMT(0, F2(q, d[i]), "%d");        \
		}                                                   \
		for (i = 0; i < 1000; i++) {                        \
			ASSERT_EQ_FMT(d[i], F1(q, i), M);           \
		}                                                   \
		ASSERT_EQ_FMT(0, rnd_queue_destroy(q, NULL), "%d"); \
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
	PASS();
}

TEST t_set(void)
{
	struct rnd_queue *q;

	{ /* Generic form */
		unsigned i;
		int a = 1;
		q = rnd_queue_create(sizeof(int), 1000);
		ASSERT_NEQ(NULL, q);
		ASSERT_EQ_FMT(RND_EINDEX, rnd_queue_set(q, 0, &a), "%d");
		ASSERT_EQ_FMT(0, rnd_queue_pushi(q, 10), "%d");
		ASSERT_EQ_FMT(RND_EINVAL, rnd_queue_set(NULL, 0, &a), "%d");
		ASSERT_EQ_FMT(RND_EINVAL, rnd_queue_set(q, 0, NULL), "%d");
		ASSERT_EQ_FMT(RND_EINVAL, rnd_queue_set(NULL, 0, NULL), "%d");
		ASSERT_EQ_FMT(RND_EINDEX, rnd_queue_set(q, 1, &a), "%d");
		ASSERT_EQ_FMT(0, rnd_queue_set(q, 0, &a), "%d");
		ASSERT_EQ_FMT(a, rnd_queue_geti(q, 0), "%d");
		ASSERT_EQ_FMT(0, rnd_queue_destroy(q, NULL), "%d");
		q = rnd_queue_create(sizeof(struct data), 1000);
		ASSERT_NEQ(NULL, q);
		for (i = 0; i < 1000; i++) {
			struct data d;
			ASSERT_EQ_FMT(0, data_init(&d), "%d");
			ASSERT_EQ_FMT(0, rnd_queue_push(q, &d), "%d");
		}
		for (i = 0; i < 1000; i++) {
			struct data a, b;
			ASSERT_EQ_FMT(0, data_init(&b), "%d");
			ASSERT_EQ_FMT(0, rnd_queue_get(q, i, &a), "%d");
			ASSERT_EQ_FMT(0, data_dtor(&a), "%d");
			ASSERT_EQ_FMT(0, rnd_queue_set(q, i, &b), "%d");
			ASSERT_EQ_FMT(0, rnd_queue_get(q, i, &a), "%d");
			ASSERT_EQ_FMT(0, data_cmp(&a, &b), "%d");
		}
		ASSERT_EQ_FMT(0, rnd_queue_destroy(q, data_dtor), "%d");
	}

	/* Suffixed form
	 * T  - type
	 * F1 - set function
	 * F2 - push function
	 * F3 - get function
	 * V  - random value snippet
	 * M  - printf format string
	 */
#define test(T, F1, F2, F3, V, M) do {                              \
		unsigned i;                                         \
		T a = (V);                                          \
		q = rnd_queue_create(sizeof(T), 1000);              \
		ASSERT_NEQ(NULL, q);                                \
		ASSERT_EQ_FMT(RND_EINDEX, F1(q, 0, a), "%d");       \
		ASSERT_EQ_FMT(0, F2(q, (V)), "%d");                 \
		ASSERT_EQ_FMT(RND_EINVAL, F1(NULL, 0, a), "%d");    \
		ASSERT_EQ_FMT(RND_EINDEX, F1(q, 1, a), "%d");       \
		ASSERT_EQ_FMT(0, F1(q, 0, a), "%d");                \
		ASSERT_EQ_FMT(a, F3(q, 0), M);                      \
		ASSERT_EQ_FMT(0, rnd_queue_clear(q, NULL), "%d");   \
		for (i = 0; i < 1000; i++) {                        \
			ASSERT_EQ_FMT(0, F2(q, (V)), "%d");         \
		}                                                   \
		for (i = 0; i < 1000; i++) {                        \
			T b = (V);                                  \
			ASSERT_EQ_FMT(0, F1(q, i, b), "%d");        \
			ASSERT_EQ_FMT(b, F3(q, i), M);              \
		}                                                   \
		ASSERT_EQ_FMT(0, rnd_queue_destroy(q, NULL), "%d"); \
		q = rnd_queue_create(sizeof(T) + 1, 1000);          \
		ASSERT_NEQ(NULL, q);                                \
		q->size = 1;                                        \
		ASSERT_EQ_FMT(RND_EILLEGAL, F1(q, 0, (V)), "%d");   \
		ASSERT_EQ_FMT(0, rnd_queue_destroy(q, NULL), "%d"); \
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
	PASS();
}

TEST t_print(void)
{
	struct rnd_queue *q;
	PASS();
}

SUITE(queue) {
	RUN_TEST(t_create);
	RUN_TEST(t_destroy);
	RUN_TEST(t_push);
	RUN_TEST(t_peek);
	RUN_TEST(t_pop);
	RUN_TEST(t_clear);
	RUN_TEST(t_map);
	RUN_TEST(t_copy);
	RUN_TEST(t_insert);
	RUN_TEST(t_quickinsert);
	RUN_TEST(t_remove);
	RUN_TEST(t_quickremove);
	RUN_TEST(t_get);
	RUN_TEST(t_set);
	RUN_TEST(t_print);
}

int main(int argc, char **argv)
{
	GREATEST_MAIN_BEGIN();

	srand(time(NULL));
	RUN_SUITE(queue);

	GREATEST_MAIN_END();
}
