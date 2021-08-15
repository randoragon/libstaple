#include <stdlib.h>
#include <time.h>
#include "../../src/rnd.h"
#include "test_struct.h"
#include <greatest.h>
#include <limits.h>
#include <float.h>

/* Make testing for size overflow feasible */
#ifdef SIZE_MAX
#undef SIZE_MAX
#endif
#define SIZE_MAX 65535LU

GREATEST_MAIN_DEFS();

TEST t_create(void)
{
	struct rnd_stack *s;
	s = rnd_stack_create(sizeof(int), 0);
	ASSERT_EQ(NULL, s);
	s = rnd_stack_create(0, 16);
	ASSERT_EQ(NULL, s);
	s = rnd_stack_create(0, 0);
	ASSERT_EQ(NULL, s);
	s = rnd_stack_create(SIZE_MAX, SIZE_MAX);
	ASSERT_EQ(NULL, s);
	s = rnd_stack_create(sizeof(int), 16);
	ASSERT_NEQ(NULL, s);
	ASSERT_EQ_FMT(0, rnd_stack_destroy(s, NULL), "%d");
	PASS();
}

TEST t_destroy(void)
{
	struct rnd_stack *s;
	unsigned i;
	s = rnd_stack_create(sizeof(long double), 1000);
	ASSERT_NEQ(NULL, s);
	ASSERT_EQ_FMT(RND_EINVAL, rnd_stack_destroy(NULL, NULL), "%d");
	ASSERT_EQ_FMT(0, rnd_stack_destroy(s, NULL), "%d");
	s = rnd_stack_create(sizeof(struct data), 1000);
	for (i = 0; i < 1000; i++) {
		struct data d;
		ASSERT_EQ_FMT(0, data_init(&d), "%d");
		ASSERT_EQ_FMT(0, rnd_stack_push(s, &d), "%d");
	}
	ASSERT_EQ_FMT(RND_EHANDLER, rnd_stack_destroy(s, data_dtor_bad), "%d");
	ASSERT_EQ_FMT(0, rnd_stack_destroy(s, data_dtor), "%d");
	PASS();
}

TEST t_push(void)
{
	struct rnd_stack *s;

	{ /* Generic form */
		unsigned i;
		struct data d;
		s = rnd_stack_create(sizeof(struct data), 1000);
		ASSERT_NEQ(NULL, s);
		data_init(&d);
		ASSERT_EQ_FMT(RND_EINVAL, rnd_stack_push(s, NULL), "%d");
		ASSERT_EQ_FMT(RND_EINVAL, rnd_stack_push(NULL, &d), "%d");
		ASSERT_EQ_FMT(RND_EINVAL, rnd_stack_push(NULL, NULL), "%d");
		ASSERT_EQ_FMT(0LU, (unsigned long)s->size, "%lu");
		ASSERT_EQ_FMT(0, rnd_stack_push(s, &d), "%d");
		ASSERT_EQ_FMT(1LU, (unsigned long)s->size, "%lu");
		ASSERT_EQ_FMT(0, rnd_stack_push(s, &d), "%d");
		ASSERT_EQ_FMT(2LU, (unsigned long)s->size, "%lu");
		ASSERT_EQ_FMT(0, rnd_stack_clear(s, NULL), "%d");
		for (i = 0; i < SIZE_MAX / sizeof(struct data); i++) {
			struct data a, b;
			ASSERT_EQ_FMT(0, data_init(&a), "%d");
			ASSERT_EQ_FMT(0, rnd_stack_push(s, &a), "%d");
			ASSERT_EQ_FMT(0, rnd_stack_get(s, 0, &b), "%d");
			ASSERT_EQ_FMT(0, data_cmp(&a, &b), "%d");
		}
		ASSERT_EQ_FMT(RND_ERANGE, rnd_stack_push(s, &d), "%d");
		ASSERT_EQ_FMT(0, data_dtor(&d), "%d");
		ASSERT_EQ_FMT(0, rnd_stack_destroy(s, data_dtor), "%d");
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
		s = rnd_stack_create(sizeof(T), 1000);                       \
		ASSERT_NEQ(NULL, s);                                         \
		ASSERT_EQ_FMT(RND_EINVAL, F1(NULL, (V)), "%d");              \
		ASSERT_EQ_FMT(0LU, (unsigned long)s->size, "%lu");           \
		ASSERT_EQ_FMT(0, F1(s, (V)), "%d");                          \
		ASSERT_EQ_FMT(1LU, (unsigned long)s->size, "%lu");           \
		ASSERT_EQ_FMT(0, F1(s, (V)), "%d");                          \
		ASSERT_EQ_FMT(2LU, (unsigned long)s->size, "%lu");           \
		ASSERT_EQ_FMT(0, rnd_stack_clear(s, NULL), "%d");            \
		for (i = 0; i < SIZE_MAX / sizeof(T); i++) {                 \
			T a = (V);                                           \
			ASSERT_EQ_FMT(0, F1(s, a), "%d");                    \
			ASSERT_EQ_FMT(a, F2(s, 0), M);                       \
		}                                                            \
		ASSERT_EQ_FMT(RND_ERANGE, F1(s, (V)), "%d");                 \
		ASSERT_EQ_FMT(0, rnd_stack_destroy(s, NULL), "%d");          \
		s = rnd_stack_create(sizeof(T) + 1, 1);                      \
		ASSERT_NEQ(NULL, s);                                         \
		ASSERT_EQ_FMT(RND_EILLEGAL, F1(s, (V)), "%d");               \
		ASSERT_EQ_FMT(0, rnd_stack_destroy(s, NULL), "%d");          \
	} while (0)
	test(char          , rnd_stack_pushc , rnd_stack_getc , IRANGE(CHAR_MIN , CHAR_MAX) , "%hd");
	test(short         , rnd_stack_pushs , rnd_stack_gets , IRANGE(SHRT_MIN , SHRT_MAX) , "%hd");
	test(int           , rnd_stack_pushi , rnd_stack_geti , FRANGE(INT_MIN  , INT_MAX)  , "%d");
	test(long          , rnd_stack_pushl , rnd_stack_getl , FRANGE(LONG_MIN , LONG_MAX) , "%ld");
	test(signed char   , rnd_stack_pushsc, rnd_stack_getsc, IRANGE(SCHAR_MIN, SCHAR_MAX), "%hd");
	test(unsigned char , rnd_stack_pushuc, rnd_stack_getuc, IRANGE(0        , UCHAR_MAX), "%hd");
	test(unsigned short, rnd_stack_pushus, rnd_stack_getus, IRANGE(0        , USHRT_MAX), "%hu");
	test(unsigned int  , rnd_stack_pushui, rnd_stack_getui, FRANGE(0        , UINT_MAX) , "%u");
	test(unsigned long , rnd_stack_pushul, rnd_stack_getul, FRANGE(0        , ULONG_MAX), "%lu");
	test(float         , rnd_stack_pushf , rnd_stack_getf , FRANGE(FLT_MIN  , FLT_MAX)  , "%f");
	test(double        , rnd_stack_pushd , rnd_stack_getd , FRANGE(DBL_MIN  , DBL_MAX)  , "%f");
	test(long double   , rnd_stack_pushld, rnd_stack_getld, FRANGE(LDBL_MIN , LDBL_MAX) , "%Lf");
#undef test
	PASS();
}

TEST t_peek(void)
{
	struct rnd_stack *s;

	{ /* Generic form */
		struct data a, b;
		s = rnd_stack_create(sizeof(struct data), 2);
		ASSERT_NEQ(NULL, s);
		ASSERT_EQ_FMT(RND_EILLEGAL, rnd_stack_peek(s, &b), "%d");
		ASSERT_EQ_FMT(RND_EINVAL, rnd_stack_peek(s, NULL), "%d");
		ASSERT_EQ_FMT(RND_EINVAL, rnd_stack_peek(NULL, &b), "%d");
		ASSERT_EQ_FMT(RND_EINVAL, rnd_stack_peek(NULL, NULL), "%d");
		data_init(&a);
		ASSERT_EQ_FMT(0, rnd_stack_push(s, &a), "%d");
		ASSERT_EQ_FMT(0, rnd_stack_peek(s, &b), "%d");
		ASSERT_EQ_FMT(0, data_cmp(&a, &b), "%d");
		ASSERT_EQ_FMT(0, rnd_stack_destroy(s, data_dtor), "%d");
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
		s = rnd_stack_create(sizeof(T), 2);                 \
		ASSERT_NEQ(NULL, s);                                \
		ASSERT_EQ_FMT(z, F1(s), M);                         \
		ASSERT_EQ_FMT(z, F1(NULL), M);                      \
		ASSERT_EQ_FMT(0, F2(s, a), "%d");                   \
		ASSERT_EQ_FMT(a, F1(s), M);                         \
		ASSERT_EQ_FMT(0, rnd_stack_destroy(s, NULL), "%d"); \
		s = rnd_stack_create(sizeof(T) + 1, 1);             \
		ASSERT_NEQ(NULL, s);                                \
		s->size = 1;                                        \
		ASSERT_EQ_FMT(z, F1(s), M);                         \
		ASSERT_EQ_FMT(0, rnd_stack_destroy(s, NULL), "%d"); \
	} while (0)
	test(char          , rnd_stack_peekc , rnd_stack_pushc , IRANGE(1, CHAR_MAX) , "%hd");
	test(short         , rnd_stack_peeks , rnd_stack_pushs , IRANGE(1, SHRT_MAX) , "%hd");
	test(int           , rnd_stack_peeki , rnd_stack_pushi , FRANGE(1, INT_MAX)  , "%d");
	test(long          , rnd_stack_peekl , rnd_stack_pushl , FRANGE(1, LONG_MAX) , "%ld");
	test(signed char   , rnd_stack_peeksc, rnd_stack_pushsc, IRANGE(1, SCHAR_MAX), "%hd");
	test(unsigned char , rnd_stack_peekuc, rnd_stack_pushuc, IRANGE(1, UCHAR_MAX), "%hd");
	test(unsigned short, rnd_stack_peekus, rnd_stack_pushus, IRANGE(1, USHRT_MAX), "%hu");
	test(unsigned int  , rnd_stack_peekui, rnd_stack_pushui, FRANGE(1, UINT_MAX) , "%u");
	test(unsigned long , rnd_stack_peekul, rnd_stack_pushul, FRANGE(1, ULONG_MAX), "%lu");
	test(float         , rnd_stack_peekf , rnd_stack_pushf , FRANGE(1, FLT_MAX)  , "%f");
	test(double        , rnd_stack_peekd , rnd_stack_pushd , FRANGE(1, DBL_MAX)  , "%f");
	test(long double   , rnd_stack_peekld, rnd_stack_pushld, FRANGE(1, LDBL_MAX) , "%Lf");
#undef test
	PASS();
}

TEST t_pop(void)
{
	struct rnd_stack *s;

	{ /* Generic form */
		struct data a, b;
		s = rnd_stack_create(sizeof(struct data), 2);
		ASSERT_NEQ(NULL, s);
		ASSERT_EQ_FMT(RND_EILLEGAL, rnd_stack_pop(s, NULL), "%d");
		ASSERT_EQ_FMT(RND_EINVAL, rnd_stack_pop(NULL, NULL), "%d");
		data_init(&a);
		ASSERT_EQ_FMT(0, rnd_stack_push(s, &a), "%d");
		ASSERT_EQ_FMT(1LU, (unsigned long)s->size, "%lu");
		ASSERT_EQ_FMT(0, rnd_stack_pop(s, &b), "%d");
		ASSERT_EQ_FMT(0LU, (unsigned long)s->size, "%lu");
		ASSERT_EQ_FMT(0, data_cmp(&a, &b), "%d");
		ASSERT_EQ_FMT(0, rnd_stack_destroy(s, NULL), "%d");
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
		s = rnd_stack_create(sizeof(T), 2);                 \
		ASSERT_NEQ(NULL, s);                                \
		ASSERT_EQ_FMT(z, F1(s), M);                         \
		ASSERT_EQ_FMT(z, F1(NULL), M);                      \
		ASSERT_EQ_FMT(0, F2(s, a), "%d");                   \
		ASSERT_EQ_FMT(1LU, (unsigned long)s->size, "%lu");  \
		ASSERT_EQ_FMT(a, F1(s), M);                         \
		ASSERT_EQ_FMT(0LU, (unsigned long)s->size, "%lu");  \
		ASSERT_EQ_FMT(0, rnd_stack_destroy(s, NULL), "%d"); \
		s = rnd_stack_create(sizeof(T) + 1, 1);             \
		ASSERT_NEQ(NULL, s);                                \
		s->size = 1;                                        \
		ASSERT_EQ_FMT(z, F1(s), M);                         \
		ASSERT_EQ_FMT(0, rnd_stack_destroy(s, NULL), "%d"); \
	} while (0)
	test(char          , rnd_stack_popc , rnd_stack_pushc , IRANGE(1, CHAR_MAX) , "%hd");
	test(short         , rnd_stack_pops , rnd_stack_pushs , IRANGE(1, SHRT_MAX) , "%hd");
	test(int           , rnd_stack_popi , rnd_stack_pushi , FRANGE(1, INT_MAX)  , "%d");
	test(long          , rnd_stack_popl , rnd_stack_pushl , FRANGE(1, LONG_MAX) , "%ld");
	test(signed char   , rnd_stack_popsc, rnd_stack_pushsc, IRANGE(1, SCHAR_MAX), "%hd");
	test(unsigned char , rnd_stack_popuc, rnd_stack_pushuc, IRANGE(1, UCHAR_MAX), "%hd");
	test(unsigned short, rnd_stack_popus, rnd_stack_pushus, IRANGE(1, USHRT_MAX), "%hu");
	test(unsigned int  , rnd_stack_popui, rnd_stack_pushui, FRANGE(1, UINT_MAX) , "%u");
	test(unsigned long , rnd_stack_popul, rnd_stack_pushul, FRANGE(1, ULONG_MAX), "%lu");
	test(float         , rnd_stack_popf , rnd_stack_pushf , FRANGE(1, FLT_MAX)  , "%f");
	test(double        , rnd_stack_popd , rnd_stack_pushd , FRANGE(1, DBL_MAX)  , "%f");
	test(long double   , rnd_stack_popld, rnd_stack_pushld, FRANGE(1, LDBL_MAX) , "%Lf");
#undef test
	PASS();
}

TEST t_clear(void)
{
	struct rnd_stack *s;
	unsigned i;
	s = rnd_stack_create(sizeof(long double), 1000);
	ASSERT_NEQ(NULL, s);
	ASSERT_EQ_FMT(RND_EINVAL, rnd_stack_clear(NULL, NULL), "%d");
	ASSERT_EQ_FMT(0, rnd_stack_clear(s, NULL), "%d");
	ASSERT_EQ_FMT(0LU, (unsigned long)s->size, "%lu");
	ASSERT_EQ_FMT(0, rnd_stack_destroy(s, NULL), "%d");
	s = rnd_stack_create(sizeof(struct data), 1000);
	for (i = 0; i < 1000; i++) {
		struct data d;
		ASSERT_EQ_FMT(0, data_init(&d), "%d");
		ASSERT_EQ_FMT(0, rnd_stack_push(s, &d), "%d");
	}
	ASSERT_EQ_FMT(RND_EHANDLER, rnd_stack_clear(s, data_dtor_bad), "%d");
	ASSERT_EQ_FMT(0, rnd_stack_clear(s, data_dtor), "%d");
	ASSERT_EQ_FMT(0LU, (unsigned long)s->size, "%lu");
	ASSERT_EQ_FMT(0, rnd_stack_destroy(s, NULL), "%d");
	PASS();
}

TEST t_foreach(void)
{
	struct rnd_stack *s;
	unsigned i;
	s = rnd_stack_create(sizeof(long double), 1000);
	ASSERT_NEQ(NULL, s);
	ASSERT_EQ_FMT(RND_EINVAL, rnd_stack_foreach(NULL, data_mutate), "%d");
	ASSERT_EQ_FMT(RND_EINVAL, rnd_stack_foreach(s, NULL), "%d");
	ASSERT_EQ_FMT(RND_EINVAL, rnd_stack_foreach(NULL, NULL), "%d");
	ASSERT_EQ_FMT(0, rnd_stack_destroy(s, NULL), "%d");
	s = rnd_stack_create(sizeof(struct data), 1000);
	for (i = 0; i < 1000; i++) {
		struct data d;
		ASSERT_EQ_FMT(0, data_init(&d), "%d");
		ASSERT_EQ_FMT(0, rnd_stack_push(s, &d), "%d");
	}
	ASSERT_EQ_FMT(RND_EHANDLER, rnd_stack_foreach(s, data_mutate_bad), "%d");
	ASSERT_EQ_FMT(0, rnd_stack_foreach(s, data_mutate), "%d");
	ASSERT_EQ_FMT(0, rnd_stack_foreach(s, data_verify), "%d");
	ASSERT_EQ_FMT(0, rnd_stack_destroy(s, data_dtor), "%d");
	PASS();
}

TEST t_copy(void)
{
	struct rnd_stack *s, *p;
	unsigned i;
	s = rnd_stack_create(sizeof(int), 1000);
	p = rnd_stack_create(sizeof(int), 333);
	ASSERT_NEQ(NULL, s);
	ASSERT_EQ_FMT(RND_EINVAL, rnd_stack_copy(NULL, s, NULL), "%d");
	ASSERT_EQ_FMT(RND_EINVAL, rnd_stack_copy(p, NULL, NULL), "%d");
	ASSERT_EQ_FMT(RND_EINVAL, rnd_stack_copy(NULL, NULL, NULL), "%d");
	ASSERT_EQ_FMT(0, rnd_stack_copy(s, p, NULL), "%d");
	ASSERT_EQ_FMT((unsigned long)p->size, (unsigned long)s->size, "%lu");
	ASSERT_EQ_FMT((unsigned long)p->elem_size, (unsigned long)s->elem_size, "%lu");
	ASSERT_EQ_FMT(0, rnd_stack_copy(p, s, NULL), "%d");
	ASSERT_EQ_FMT((unsigned long)p->size, (unsigned long)s->size, "%lu");
	ASSERT_EQ_FMT((unsigned long)p->elem_size, (unsigned long)s->elem_size, "%lu");
	for (i = 0; i < 1000; i++) {
		ASSERT_EQ_FMT(0, rnd_stack_pushi(s, FRANGE(INT_MIN, INT_MAX)), "%d");
	}
	ASSERT_EQ_FMT(0, rnd_stack_copy(p, s, NULL), "%d");
	ASSERT_EQ_FMT((unsigned long)p->size, (unsigned long)s->size, "%lu");
	ASSERT_EQ_FMT((unsigned long)p->elem_size, (unsigned long)s->elem_size, "%lu");
	for (i = 0; i < 1000; i++) {
		int a, b;
		a = rnd_stack_geti(s, i);
		b = rnd_stack_geti(p, i);
		ASSERT_EQ_FMT(a, b, "%d");
	}
	ASSERT_EQ_FMT(0, rnd_stack_destroy(s, NULL), "%d");
	ASSERT_EQ_FMT(0, rnd_stack_clear(p, NULL), "%d");
	s = rnd_stack_create(sizeof(struct data), 1000);
	for (i = 0; i < 1000; i++) {
		struct data d;
		ASSERT_EQ_FMT(0, data_init(&d), "%d");
		ASSERT_EQ_FMT(0, rnd_stack_push(s, &d), "%d");
	}
	ASSERT_EQ_FMT(RND_EHANDLER, rnd_stack_copy(p, s, data_cpy_bad), "%d");
	ASSERT_EQ_FMT(0, rnd_stack_copy(p, s, data_cpy), "%d");
	ASSERT_EQ_FMT((unsigned long)p->size, (unsigned long)s->size, "%lu");
	ASSERT_EQ_FMT((unsigned long)p->elem_size, (unsigned long)s->elem_size, "%lu");
	for (i = 0; i < 1000; i++) {
		struct data a, b;
		ASSERT_EQ_FMT(0, rnd_stack_get(s, i, &a), "%d");
		ASSERT_EQ_FMT(0, rnd_stack_get(p, i, &b), "%d");
		ASSERT_EQ_FMT(0, data_cmp(&a, &b), "%d");
	}
	ASSERT_EQ_FMT(0, rnd_stack_destroy(s, data_dtor), "%d");
	ASSERT_EQ_FMT(0, rnd_stack_destroy(p, data_dtor), "%d");
	PASS();
}

TEST t_insert(void)
{
	struct rnd_stack *s;
	unsigned i;

	{ /* Generic form */
		int a = 10;
		struct data d[1000];
		s = rnd_stack_create(sizeof(int), 1000);
		ASSERT_NEQ(NULL, s);
		ASSERT_EQ_FMT(RND_EINVAL, rnd_stack_insert(s, 0, NULL), "%d");
		ASSERT_EQ_FMT(RND_EINVAL, rnd_stack_insert(NULL, 0, &a), "%d");
		ASSERT_EQ_FMT(RND_EINVAL, rnd_stack_insert(NULL, 0, NULL), "%d");
		ASSERT_EQ_FMT(RND_EINDEX, rnd_stack_insert(s, 1, &a), "%d");
		ASSERT_EQ_FMT(0, rnd_stack_insert(s, 0, &a), "%d");
		ASSERT_EQ_FMT(1LU, (unsigned long)s->size, "%lu");
		ASSERT_EQ_FMT(10, rnd_stack_geti(s, 0), "%d");
		ASSERT_EQ_FMT(0, rnd_stack_destroy(s, NULL), "%d");

		s = rnd_stack_create(sizeof(struct data), 1000);
		ASSERT_NEQ(NULL, s);
		for (i = 0; i < 1000; i++) {
			struct data a;
			size_t idx = IRANGE(0, i);
			ASSERT_EQ_FMT(0, data_init(d + i), "%d");
			ASSERT_EQ_FMT(0, rnd_stack_insert(s, idx, d + i), "%d");
			ASSERT_EQ_FMT((unsigned long)i + 1, (unsigned long)s->size, "%lu");
			ASSERT_EQ_FMT(0, rnd_stack_get(s, idx, &a), "%d");
			ASSERT_EQ_FMT(0, data_cmp(&a, d + i), "%d");
		}
		ASSERT_EQ_FMT(0, rnd_stack_destroy(s, data_dtor), "%d");

		s = rnd_stack_create(sizeof(struct data), 10);
		ASSERT_NEQ(NULL, s);
		for (i = 0; i < 5; i++) {
			ASSERT_EQ_FMT(0, data_init(d + i), "%d");
			ASSERT_EQ_FMT(0, rnd_stack_insert(s, i, d + i), "%d");
		}
		for (i = 0; i < 5; i++) {
			struct data a;
			ASSERT_EQ_FMT(0, rnd_stack_get(s, i, &a), "%d");
			ASSERT_EQ_FMT(0, data_cmp(&a, d + i), "%d");
		}
		{
			struct data a;
			ASSERT_EQ_FMT(0, data_init(d + 5), "%d");
			ASSERT_EQ_FMT(0, rnd_stack_insert(s, 1, d + 5), "%d");
			ASSERT_EQ_FMT(0, rnd_stack_peek(s, &a), "%d");
			ASSERT_EQ_FMT(0, data_cmp(&a, d), "%d");
			ASSERT_EQ_FMT(0, data_init(d + 6), "%d");
			ASSERT_EQ_FMT(0, rnd_stack_insert(s, 3, d + 6), "%d");
			ASSERT_EQ_FMT(0, rnd_stack_peek(s, &a), "%d");
			ASSERT_EQ_FMT(0, data_cmp(&a, d), "%d");
			ASSERT_EQ_FMT(7LU, (unsigned long)s->size, "%lu");
			ASSERT_EQ_FMT(0, rnd_stack_destroy(s, data_dtor), "%d");
		}
	}

	/* Suffixed form
	 * T  - type
	 * F1 - insert function
	 * F2 - get function
	 * V  - random value snippet
	 * M  - printf format string
	 */
#define test(T, F1, F2, V, M) do {                                                          \
		T a = (V);                                                                  \
		T d[1000];                                                                  \
		s = rnd_stack_create(sizeof(T), 1000);                                      \
		ASSERT_NEQ(NULL, s);                                                        \
		ASSERT_EQ_FMT(RND_EINVAL, F1(NULL, 0, a), "%d");                            \
		ASSERT_EQ_FMT(RND_EINDEX, F1(s, 1, a), "%d");                               \
		ASSERT_EQ_FMT(0, F1(s, 0, a), "%d");                                        \
		ASSERT_EQ_FMT(1LU, (unsigned long)s->size, "%lu");                          \
		ASSERT_EQ_FMT(a, F2(s, 0), M);                                              \
		ASSERT_EQ_FMT(0, rnd_stack_destroy(s, NULL), "%d");                         \
                                                                                            \
		s = rnd_stack_create(sizeof(T), 1000);                                      \
		ASSERT_NEQ(NULL, s);                                                        \
		for (i = 0; i < 1000; i++) {                                                \
			size_t idx = IRANGE(0, i);                                          \
			d[i] = (V);                                                         \
			ASSERT_EQ_FMT(0, F1(s, idx, d[i]), "%d");                           \
			ASSERT_EQ_FMT((unsigned long)i + 1, (unsigned long)s->size, "%lu"); \
			ASSERT_EQ_FMT(d[i], F2(s, idx), M);                                 \
		}                                                                           \
		ASSERT_EQ_FMT(0, rnd_stack_destroy(s, NULL), "%d");                         \
                                                                                            \
		s = rnd_stack_create(sizeof(T), 10);                                        \
		ASSERT_NEQ(NULL, s);                                                        \
		for (i = 0; i < 5; i++) {                                                   \
			d[i] = (V);                                                         \
			ASSERT_EQ_FMT(0, F1(s, i, d[i]), "%d");                             \
		}                                                                           \
		for (i = 0; i < 5; i++) {                                                   \
			ASSERT_EQ_FMT(d[i], F2(s, i), M);                                   \
		}                                                                           \
		d[5] = (V);                                                                 \
		ASSERT_EQ_FMT(0, F1(s, 1, d[5]), "%d");                                     \
		ASSERT_EQ_FMT(d[0], F2(s, 0), M);                                           \
		d[6] = (V);                                                                 \
		ASSERT_EQ_FMT(0, F1(s, 3, d[6]), "%d");                                     \
		ASSERT_EQ_FMT(d[0], F2(s, 0), M);                                           \
		ASSERT_EQ_FMT(0, rnd_stack_destroy(s, NULL), "%d");                         \
		s = rnd_stack_create(sizeof(T) + 1, 1000);                                  \
		ASSERT_NEQ(NULL, s);                                                        \
		ASSERT_EQ_FMT(RND_EILLEGAL, F1(s, 0, (V)), "%d");                           \
		ASSERT_EQ_FMT(0, rnd_stack_destroy(s, NULL), "%d");                         \
	} while (0)
	test(char          , rnd_stack_insertc , rnd_stack_getc , IRANGE(1, CHAR_MAX) , "%hd");
	test(short         , rnd_stack_inserts , rnd_stack_gets , IRANGE(1, SHRT_MAX) , "%hd");
	test(int           , rnd_stack_inserti , rnd_stack_geti , FRANGE(1, INT_MAX)  , "%d");
	test(long          , rnd_stack_insertl , rnd_stack_getl , FRANGE(1, LONG_MAX) , "%ld");
	test(signed char   , rnd_stack_insertsc, rnd_stack_getsc, IRANGE(1, SCHAR_MAX), "%hd");
	test(unsigned char , rnd_stack_insertuc, rnd_stack_getuc, IRANGE(1, UCHAR_MAX), "%hd");
	test(unsigned short, rnd_stack_insertus, rnd_stack_getus, IRANGE(1, USHRT_MAX), "%hu");
	test(unsigned int  , rnd_stack_insertui, rnd_stack_getui, FRANGE(1, UINT_MAX) , "%u");
	test(unsigned long , rnd_stack_insertul, rnd_stack_getul, FRANGE(1, ULONG_MAX), "%lu");
	test(float         , rnd_stack_insertf , rnd_stack_getf , FRANGE(1, FLT_MAX)  , "%f");
	test(double        , rnd_stack_insertd , rnd_stack_getd , FRANGE(1, DBL_MAX)  , "%f");
	test(long double   , rnd_stack_insertld, rnd_stack_getld, FRANGE(1, LDBL_MAX) , "%Lf");
#undef test
	PASS();
}

TEST t_qinsert(void)
{
	struct rnd_stack *s;
	unsigned i;

	{ /* Generic form */
		int a = 10;
		struct data d[1000];
		s = rnd_stack_create(sizeof(int), 1000);
		ASSERT_NEQ(NULL, s);
		ASSERT_EQ_FMT(RND_EINVAL, rnd_stack_qinsert(s, 0, NULL), "%d");
		ASSERT_EQ_FMT(RND_EINVAL, rnd_stack_qinsert(NULL, 0, &a), "%d");
		ASSERT_EQ_FMT(RND_EINVAL, rnd_stack_qinsert(NULL, 0, NULL), "%d");
		ASSERT_EQ_FMT(RND_EINDEX, rnd_stack_qinsert(s, 1, &a), "%d");
		ASSERT_EQ_FMT(0, rnd_stack_qinsert(s, 0, &a), "%d");
		ASSERT_EQ_FMT(1LU, (unsigned long)s->size, "%lu");
		ASSERT_EQ_FMT(10, rnd_stack_geti(s, 0), "%d");
		ASSERT_EQ_FMT(0, rnd_stack_destroy(s, NULL), "%d");

		s = rnd_stack_create(sizeof(struct data), 1000);
		ASSERT_NEQ(NULL, s);
		for (i = 0; i < 1000; i++) {
			struct data a;
			size_t idx = IRANGE(0, i);
			ASSERT_EQ_FMT(0, data_init(d + i), "%d");
			ASSERT_EQ_FMT(0, rnd_stack_qinsert(s, idx, d + i), "%d");
			ASSERT_EQ_FMT((unsigned long)i + 1, (unsigned long)s->size, "%lu");
			ASSERT_EQ_FMT(0, rnd_stack_get(s, idx, &a), "%d");
			ASSERT_EQ_FMT(0, data_cmp(&a, d + i), "%d");
		}
		ASSERT_EQ_FMT(0, rnd_stack_destroy(s, data_dtor), "%d");

		s = rnd_stack_create(sizeof(struct data), 10);
		ASSERT_NEQ(NULL, s);
		for (i = 0; i < 5; i++) {
			ASSERT_EQ_FMT(0, data_init(d + i), "%d");
			ASSERT_EQ_FMT(0, rnd_stack_qinsert(s, i, d + i), "%d");
		}
		for (i = 0; i < 5; i++) {
			struct data a;
			ASSERT_EQ_FMT(0, rnd_stack_get(s, i, &a), "%d");
			ASSERT_EQ_FMT(0, data_cmp(&a, d + (8 - i) % 5), "%d");
		}
		{
			struct data a;
			ASSERT_EQ_FMT(0, data_init(d + 5), "%d");
			ASSERT_EQ_FMT(0, rnd_stack_qinsert(s, 1, d + 5), "%d");
			ASSERT_EQ_FMT(0, rnd_stack_peek(s, &a), "%d");
			ASSERT_EQ_FMT(0, data_cmp(&a, d + 3), "%d");
			ASSERT_EQ_FMT(0, data_init(d + 6), "%d");
			ASSERT_EQ_FMT(0, rnd_stack_qinsert(s, 3, d + 6), "%d");
			ASSERT_EQ_FMT(0, rnd_stack_peek(s, &a), "%d");
			ASSERT_EQ_FMT(0, data_cmp(&a, d + 2), "%d");
			ASSERT_EQ_FMT(7LU, (unsigned long)s->size, "%lu");
			ASSERT_EQ_FMT(0, rnd_stack_destroy(s, data_dtor), "%d");
		}
	}

	/* Suffixed form
	 * T  - type
	 * F1 - qinsert function
	 * F2 - get function
	 * V  - random value snippet
	 * M  - printf format string
	 */
#define test(T, F1, F2, V, M) do {                                                               \
		T a = (V);                                                                       \
		T d[1000];                                                                       \
		s = rnd_stack_create(sizeof(T), 1000);                                           \
		ASSERT_NEQ(NULL, s);                                                             \
		ASSERT_EQ_FMT(RND_EINVAL, F1(NULL, 0, a), "%d");                                 \
		ASSERT_EQ_FMT(RND_EINDEX, F1(s, 1, a), "%d");                                    \
		ASSERT_EQ_FMT(0, F1(s, 0, a), "%d");                                             \
		ASSERT_EQ_FMT(1LU, (unsigned long)s->size, "%lu");                               \
		ASSERT_EQ_FMT(a, F2(s, 0), M);                                                   \
		ASSERT_EQ_FMT(0, rnd_stack_destroy(s, NULL), "%d");                              \
                                                                                                 \
		s = rnd_stack_create(sizeof(T), 1000);                                           \
		ASSERT_NEQ(NULL, s);                                                             \
		for (i = 0; i < 1000; i++) {                                                     \
			size_t idx = IRANGE(0, i);                                               \
			d[i] = (V);                                                              \
			ASSERT_EQ_FMT(0, F1(s, idx, d[i]), "%d");                                \
			ASSERT_EQ_FMT((unsigned long)i + 1, (unsigned long)s->size, "%lu");      \
			ASSERT_EQ_FMT(d[i], F2(s, idx), M);                                      \
		}                                                                                \
		ASSERT_EQ_FMT(0, rnd_stack_destroy(s, NULL), "%d");                              \
                                                                                                 \
		s = rnd_stack_create(sizeof(T), 10);                                             \
		ASSERT_NEQ(NULL, s);                                                             \
		for (i = 0; i < 5; i++) {                                                        \
			d[i] = (V);                                                              \
			ASSERT_EQ_FMT(0, F1(s, i, d[i]), "%d");                                  \
		}                                                                                \
		for (i = 0; i < 5; i++) {                                                        \
			ASSERT_EQ_FMT(d[(8 - i) % 5], F2(s, i), M);                              \
		}                                                                                \
		d[5] = (V);                                                                      \
		ASSERT_EQ_FMT(0, F1(s, 1, d[5]), "%d");                                          \
		ASSERT_EQ_FMT(d[3], F2(s, 0), M);                                                \
		d[6] = (V);                                                                      \
		ASSERT_EQ_FMT(0, F1(s, 3, d[6]), "%d");                                          \
		ASSERT_EQ_FMT(d[2], F2(s, 0), M);                                                \
		ASSERT_EQ_FMT(0, rnd_stack_destroy(s, NULL), "%d");                              \
		s = rnd_stack_create(sizeof(T) + 1, 1000);                                       \
		ASSERT_NEQ(NULL, s);                                                             \
		ASSERT_EQ_FMT(RND_EILLEGAL, F1(s, 0, (V)), "%d");                                \
		ASSERT_EQ_FMT(0, rnd_stack_destroy(s, NULL), "%d");                              \
	} while (0)
	test(char          , rnd_stack_qinsertc , rnd_stack_getc , IRANGE(1, CHAR_MAX) , "%hd");
	test(short         , rnd_stack_qinserts , rnd_stack_gets , IRANGE(1, SHRT_MAX) , "%hd");
	test(int           , rnd_stack_qinserti , rnd_stack_geti , FRANGE(1, INT_MAX)  , "%d");
	test(long          , rnd_stack_qinsertl , rnd_stack_getl , FRANGE(1, LONG_MAX) , "%ld");
	test(signed char   , rnd_stack_qinsertsc, rnd_stack_getsc, IRANGE(1, SCHAR_MAX), "%hd");
	test(unsigned char , rnd_stack_qinsertuc, rnd_stack_getuc, IRANGE(1, UCHAR_MAX), "%hd");
	test(unsigned short, rnd_stack_qinsertus, rnd_stack_getus, IRANGE(1, USHRT_MAX), "%hu");
	test(unsigned int  , rnd_stack_qinsertui, rnd_stack_getui, FRANGE(1, UINT_MAX) , "%u");
	test(unsigned long , rnd_stack_qinsertul, rnd_stack_getul, FRANGE(1, ULONG_MAX), "%lu");
	test(float         , rnd_stack_qinsertf , rnd_stack_getf , FRANGE(1, FLT_MAX)  , "%f");
	test(double        , rnd_stack_qinsertd , rnd_stack_getd , FRANGE(1, DBL_MAX)  , "%f");
	test(long double   , rnd_stack_qinsertld, rnd_stack_getld, FRANGE(1, LDBL_MAX) , "%Lf");
#undef test
	PASS();
}

TEST t_remove(void)
{
	struct rnd_stack *s;
	unsigned i;

	{ /* Generic form */
		struct data a;
		struct data d[100];
		s = rnd_stack_create(sizeof(int), 1000);
		ASSERT_NEQ(NULL, s);
		ASSERT_EQ_FMT(RND_EINDEX, rnd_stack_remove(s, 0, &a), "%d");
		ASSERT_EQ_FMT(0, data_init(&a), "%d");
		ASSERT_EQ_FMT(0, rnd_stack_push(s, &a), "%d");
		ASSERT_EQ_FMT(RND_EINVAL, rnd_stack_remove(NULL, 0, &a), "%d");
		ASSERT_EQ_FMT(RND_EINVAL, rnd_stack_remove(NULL, 0, NULL), "%d");
		ASSERT_EQ_FMT(RND_EINDEX, rnd_stack_remove(s, 1, &a), "%d");
		ASSERT_EQ_FMT(1LU, (unsigned long)s->size, "%lu");
		ASSERT_EQ_FMT(0, rnd_stack_remove(s, 0, NULL), "%d");
		ASSERT_EQ_FMT(0LU, (unsigned long)s->size, "%lu");
		ASSERT_EQ_FMT(0, rnd_stack_destroy(s, NULL), "%d");
		ASSERT_EQ_FMT(0, data_dtor(&a), "%d");

		s = rnd_stack_create(sizeof(struct data), 100);
		ASSERT_NEQ(NULL, s);
		for (i = 0; i < 100; i++) {
			ASSERT_EQ_FMT(0, data_init(d + i), "%d");
			ASSERT_EQ_FMT(0, rnd_stack_push(s, d + i), "%d");
		}
		for (i = 0; i < 100; i++) {
			struct data a;
			size_t idx = IRANGE(0, s->size - 1), j;
			int found = 0;
			ASSERT_EQ_FMT(0, rnd_stack_remove(s, idx, &a), "%d");
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
		ASSERT_EQ_FMT(0, rnd_stack_destroy(s, NULL), "%d");

		s = rnd_stack_create(sizeof(struct data), 10);
		ASSERT_NEQ(NULL, s);
		for (i = 0; i < 8; i++) {
			ASSERT_EQ_FMT(0, rnd_stack_push(s, d + i), "%d");
		}
		ASSERT_EQ_FMT(0, rnd_stack_remove(s, 3, &a), "%d");
		ASSERT_EQ_FMT(0, data_cmp(d + 4, &a), "%d");
		ASSERT_EQ_FMT(7LU, (unsigned long)s->size, "%lu");
		ASSERT_EQ_FMT(0, rnd_stack_remove(s, 0, &a), "%d");
		ASSERT_EQ_FMT(0, data_cmp(d + 7, &a), "%d");
		ASSERT_EQ_FMT(6LU, (unsigned long)s->size, "%lu");
		ASSERT_EQ_FMT(0, rnd_stack_remove(s, 3, &a), "%d");
		ASSERT_EQ_FMT(0, data_cmp(d + 2, &a), "%d");
		ASSERT_EQ_FMT(5LU, (unsigned long)s->size, "%lu");
		ASSERT_EQ_FMT(0, rnd_stack_destroy(s, NULL), "%d");
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
		s = rnd_stack_create(sizeof(T), 1000);                             \
		ASSERT_NEQ(NULL, s);                                               \
		ASSERT_EQ_FMT(z, F1(s, 0), M);                                     \
		ASSERT_EQ_FMT(0, F2(s, a), "%d");                                  \
		ASSERT_EQ_FMT(z, F1(NULL, 0), M);                                  \
		ASSERT_EQ_FMT(z, F1(s, 1), M);                                     \
		ASSERT_EQ_FMT(1LU, (unsigned long)s->size, "%lu");                 \
		ASSERT_EQ_FMT(a, F1(s, 0), M);                                     \
		ASSERT_EQ_FMT(0LU, (unsigned long)s->size, "%lu");                 \
		ASSERT_EQ_FMT(0, rnd_stack_destroy(s, NULL), "%d");                \
                                                                                   \
		s = rnd_stack_create(sizeof(T), 100);                              \
		ASSERT_NEQ(NULL, s);                                               \
		for (i = 0; i < 100; i++) {                                        \
			d[i] = (V);                                                \
			ASSERT_EQ_FMT(0, F2(s, d[i]), "%d");                       \
		}                                                                  \
		for (i = 0; i < 100; i++) {                                        \
			T a;                                                       \
			size_t idx = IRANGE(0, s->size - 1), j;                    \
			int found = 0;                                             \
			ASSERT_NEQ(0, (a = F1(s, idx)));                           \
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
		ASSERT_EQ_FMT(0, rnd_stack_destroy(s, NULL), "%d");                \
                                                                                   \
		s = rnd_stack_create(sizeof(T), 10);                               \
		ASSERT_NEQ(NULL, s);                                               \
		for (i = 0; i < 8; i++) {                                          \
			ASSERT_EQ_FMT(0, F2(s, d[i]), "%d");                       \
		}                                                                  \
		ASSERT_EQ_FMT(d[4], F1(s, 3), M);                                  \
		ASSERT_EQ_FMT(7LU, (unsigned long)s->size, "%lu");                 \
		ASSERT_EQ_FMT(d[7], F1(s, 0), M);                                  \
		ASSERT_EQ_FMT(6LU, (unsigned long)s->size, "%lu");                 \
		ASSERT_EQ_FMT(d[2], F1(s, 3), M);                                  \
		ASSERT_EQ_FMT(5LU, (unsigned long)s->size, "%lu");                 \
		ASSERT_EQ_FMT(0, rnd_stack_destroy(s, NULL), "%d");                \
		s = rnd_stack_create(sizeof(T) + 1, 1000);                         \
		ASSERT_NEQ(NULL, s);                                               \
		s->size = 1;                                                       \
		ASSERT_EQ_FMT(z, F1(s, 0), M);                                     \
		ASSERT_EQ_FMT(0, rnd_stack_destroy(s, NULL), "%d");                \
	} while (0)
	test(char          , rnd_stack_removec , rnd_stack_pushc , IRANGE(1, CHAR_MAX) , "%hd");
	test(short         , rnd_stack_removes , rnd_stack_pushs , IRANGE(1, SHRT_MAX) , "%hd");
	test(int           , rnd_stack_removei , rnd_stack_pushi , FRANGE(1, INT_MAX)  , "%d");
	test(long          , rnd_stack_removel , rnd_stack_pushl , FRANGE(1, LONG_MAX) , "%ld");
	test(signed char   , rnd_stack_removesc, rnd_stack_pushsc, IRANGE(1, SCHAR_MAX), "%hd");
	test(unsigned char , rnd_stack_removeuc, rnd_stack_pushuc, IRANGE(1, UCHAR_MAX), "%hd");
	test(unsigned short, rnd_stack_removeus, rnd_stack_pushus, IRANGE(1, USHRT_MAX), "%hu");
	test(unsigned int  , rnd_stack_removeui, rnd_stack_pushui, FRANGE(1, UINT_MAX) , "%u");
	test(unsigned long , rnd_stack_removeul, rnd_stack_pushul, FRANGE(1, ULONG_MAX), "%lu");
	test(float         , rnd_stack_removef , rnd_stack_pushf , FRANGE(1, FLT_MAX)  , "%f");
	test(double        , rnd_stack_removed , rnd_stack_pushd , FRANGE(1, DBL_MAX)  , "%f");
	test(long double   , rnd_stack_removeld, rnd_stack_pushld, FRANGE(1, LDBL_MAX) , "%Lf");
#undef test
	PASS();
}

TEST t_qremove(void)
{
	struct rnd_stack *s;
	unsigned i;

	{ /* Generic form */
		struct data a;
		struct data d[100];
		s = rnd_stack_create(sizeof(int), 1000);
		ASSERT_NEQ(NULL, s);
		ASSERT_EQ_FMT(RND_EINDEX, rnd_stack_qremove(s, 0, &a), "%d");
		ASSERT_EQ_FMT(0, data_init(&a), "%d");
		ASSERT_EQ_FMT(0, rnd_stack_push(s, &a), "%d");
		ASSERT_EQ_FMT(RND_EINVAL, rnd_stack_qremove(NULL, 0, &a), "%d");
		ASSERT_EQ_FMT(RND_EINVAL, rnd_stack_qremove(NULL, 0, NULL), "%d");
		ASSERT_EQ_FMT(RND_EINDEX, rnd_stack_qremove(s, 1, &a), "%d");
		ASSERT_EQ_FMT(1LU, (unsigned long)s->size, "%lu");
		ASSERT_EQ_FMT(0, rnd_stack_qremove(s, 0, NULL), "%d");
		ASSERT_EQ_FMT(0LU, (unsigned long)s->size, "%lu");
		ASSERT_EQ_FMT(0, rnd_stack_destroy(s, NULL), "%d");
		ASSERT_EQ_FMT(0, data_dtor(&a), "%d");

		s = rnd_stack_create(sizeof(struct data), 100);
		ASSERT_NEQ(NULL, s);
		for (i = 0; i < 100; i++) {
			ASSERT_EQ_FMT(0, data_init(d + i), "%d");
			ASSERT_EQ_FMT(0, rnd_stack_push(s, d + i), "%d");
		}
		for (i = 0; i < 100; i++) {
			struct data a;
			size_t idx = IRANGE(0, s->size - 1), j;
			int found = 0;
			ASSERT_EQ_FMT(0, rnd_stack_qremove(s, idx, &a), "%d");
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
		ASSERT_EQ_FMT(0, rnd_stack_destroy(s, NULL), "%d");

		s = rnd_stack_create(sizeof(struct data), 10);
		ASSERT_NEQ(NULL, s);
		for (i = 0; i < 8; i++) {
			ASSERT_EQ_FMT(0, rnd_stack_push(s, d + i), "%d");
		}
		ASSERT_EQ_FMT(0, rnd_stack_qremove(s, 3, &a), "%d");
		ASSERT_EQ_FMT(0, data_cmp(d + 4, &a), "%d");
		ASSERT_EQ_FMT(0, rnd_stack_get(s, 3, &a), "%d");
		ASSERT_EQ_FMT(0, data_cmp(d + 3, &a), "%d");
		ASSERT_EQ_FMT(0, rnd_stack_qremove(s, 0, &a), "%d");
		ASSERT_EQ_FMT(0, data_cmp(d + 6, &a), "%d");
		ASSERT_EQ_FMT(0, rnd_stack_get(s, 0, &a), "%d");
		ASSERT_EQ_FMT(0, data_cmp(d + 5, &a), "%d");
		ASSERT_EQ_FMT(0, rnd_stack_qremove(s, 3, &a), "%d");
		ASSERT_EQ_FMT(0, data_cmp(d + 2, &a), "%d");
		ASSERT_EQ_FMT(0, rnd_stack_destroy(s, NULL), "%d");
		for (i = 0; i < 100; i++) {
			ASSERT_EQ_FMT(0, data_dtor(d + i), "%d");
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
		s = rnd_stack_create(sizeof(T), 1000);                             \
		ASSERT_NEQ(NULL, s);                                               \
		ASSERT_EQ_FMT(z, F1(s, 0), M);                                     \
		ASSERT_EQ_FMT(0, F2(s, a), "%d");                                  \
		ASSERT_EQ_FMT(z, F1(NULL, 0), M);                                  \
		ASSERT_EQ_FMT(z, F1(s, 1), M);                                     \
		ASSERT_EQ_FMT(1LU, (unsigned long)s->size, "%lu");                 \
		ASSERT_EQ_FMT(a, F1(s, 0), M);                                     \
		ASSERT_EQ_FMT(0LU, (unsigned long)s->size, "%lu");                 \
		ASSERT_EQ_FMT(0, rnd_stack_destroy(s, NULL), "%d");                \
                                                                                   \
		s = rnd_stack_create(sizeof(T), 100);                              \
		ASSERT_NEQ(NULL, s);                                               \
		for (i = 0; i < 100; i++) {                                        \
			d[i] = (V);                                                \
			ASSERT_EQ_FMT(0, F2(s, d[i]), "%d");                       \
		}                                                                  \
		for (i = 0; i < 100; i++) {                                        \
			T a;                                                       \
			size_t idx = IRANGE(0, s->size - 1), j;                    \
			int found = 0;                                             \
			ASSERT_NEQ(0, (a = F1(s, idx)));                           \
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
		ASSERT_EQ_FMT(0, rnd_stack_destroy(s, NULL), "%d");                \
                                                                                   \
		s = rnd_stack_create(sizeof(T), 10);                               \
		ASSERT_NEQ(NULL, s);                                               \
		for (i = 0; i < 8; i++) {                                          \
			ASSERT_EQ_FMT(0, F2(s, d[i]), "%d");                       \
		}                                                                  \
		ASSERT_EQ_FMT(d[4], F1(s, 3), M);                                  \
		ASSERT_EQ_FMT(d[3], F3(s, 3), M);                                  \
		ASSERT_EQ_FMT(d[6], F1(s, 0), M);                                  \
		ASSERT_EQ_FMT(d[5], F3(s, 0), M);                                  \
		ASSERT_EQ_FMT(d[2], F1(s, 3), M);                                  \
		ASSERT_EQ_FMT(d[1], F3(s, 3), M);                                  \
		ASSERT_EQ_FMT(0, rnd_stack_destroy(s, NULL), "%d");                \
		s = rnd_stack_create(sizeof(T) + 1, 1000);                         \
		ASSERT_NEQ(NULL, s);                                               \
		s->size = 1;                                                       \
		ASSERT_EQ_FMT(z, F1(s, 0), M);                                     \
		ASSERT_EQ_FMT(0, rnd_stack_destroy(s, NULL), "%d");                \
	} while (0)
	test(char          , rnd_stack_qremovec , rnd_stack_pushc , rnd_stack_getc , IRANGE(1, CHAR_MAX) , "%hd");
	test(short         , rnd_stack_qremoves , rnd_stack_pushs , rnd_stack_gets , IRANGE(1, SHRT_MAX) , "%hd");
	test(int           , rnd_stack_qremovei , rnd_stack_pushi , rnd_stack_geti , FRANGE(1, INT_MAX)  , "%d");
	test(long          , rnd_stack_qremovel , rnd_stack_pushl , rnd_stack_getl , FRANGE(1, LONG_MAX) , "%ld");
	test(signed char   , rnd_stack_qremovesc, rnd_stack_pushsc, rnd_stack_getsc, IRANGE(1, SCHAR_MAX), "%hd");
	test(unsigned char , rnd_stack_qremoveuc, rnd_stack_pushuc, rnd_stack_getuc, IRANGE(1, UCHAR_MAX), "%hd");
	test(unsigned short, rnd_stack_qremoveus, rnd_stack_pushus, rnd_stack_getus, IRANGE(1, USHRT_MAX), "%hu");
	test(unsigned int  , rnd_stack_qremoveui, rnd_stack_pushui, rnd_stack_getui, FRANGE(1, UINT_MAX) , "%u");
	test(unsigned long , rnd_stack_qremoveul, rnd_stack_pushul, rnd_stack_getul, FRANGE(1, ULONG_MAX), "%lu");
	test(float         , rnd_stack_qremovef , rnd_stack_pushf , rnd_stack_getf , FRANGE(1, FLT_MAX)  , "%f");
	test(double        , rnd_stack_qremoved , rnd_stack_pushd , rnd_stack_getd , FRANGE(1, DBL_MAX)  , "%f");
	test(long double   , rnd_stack_qremoveld, rnd_stack_pushld, rnd_stack_getld, FRANGE(1, LDBL_MAX) , "%Lf");
#undef test
	PASS();
}

TEST t_get(void)
{
	struct rnd_stack *s;

	{ /* Generic form */
		unsigned i;
		int a = 0;
		struct data d[1000];
		s = rnd_stack_create(sizeof(int), 1000);
		ASSERT_NEQ(NULL, s);
		ASSERT_EQ_FMT(RND_EINDEX, rnd_stack_get(s, 0, &a), "%d");
		ASSERT_EQ_FMT(0, rnd_stack_pushi(s, 10), "%d");
		ASSERT_EQ_FMT(RND_EINVAL, rnd_stack_get(NULL, 0, &a), "%d");
		ASSERT_EQ_FMT(RND_EINVAL, rnd_stack_get(s, 0, NULL), "%d");
		ASSERT_EQ_FMT(RND_EINVAL, rnd_stack_get(NULL, 0, NULL), "%d");
		ASSERT_EQ_FMT(RND_EINDEX, rnd_stack_get(s, 1, &a), "%d");
		ASSERT_EQ_FMT(0, rnd_stack_get(s, 0, &a), "%d");
		ASSERT_EQ_FMT(10, a, "%d");
		ASSERT_EQ_FMT(0, rnd_stack_destroy(s, NULL), "%d");
		s = rnd_stack_create(sizeof(struct data), 1000);
		ASSERT_NEQ(NULL, s);
		for (i = 0; i < 1000; i++) {
			ASSERT_EQ_FMT(0, data_init(d + i), "%d");
			ASSERT_EQ_FMT(0, rnd_stack_push(s, d + i), "%d");
		}
		for (i = 0; i < 1000; i++) {
			struct data a;
			ASSERT_EQ_FMT(0, rnd_stack_get(s, i, &a), "%d");
			ASSERT_EQ_FMT(0, data_cmp(&a, d + 999 - i), "%d");
		}
		ASSERT_EQ_FMT(0, rnd_stack_destroy(s, data_dtor), "%d");
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
		s = rnd_stack_create(sizeof(T), 1000);              \
		ASSERT_NEQ(NULL, s);                                \
		ASSERT_EQ_FMT(z, F1(s, 0), M);                      \
		ASSERT_EQ_FMT(0, F2(s, a), "%d");                   \
		ASSERT_EQ_FMT(z, F1(NULL, 0), M);                   \
		ASSERT_EQ_FMT(z, F1(s, 1), M);                      \
		ASSERT_EQ_FMT(a, F1(s, 0), M);                      \
		ASSERT_EQ_FMT(0, rnd_stack_destroy(s, NULL), "%d"); \
		s = rnd_stack_create(sizeof(T), 1000);              \
		ASSERT_NEQ(NULL, s);                                \
		for (i = 0; i < 1000; i++) {                        \
			d[i] = (V);                                 \
			ASSERT_EQ_FMT(0, F2(s, d[i]), "%d");        \
		}                                                   \
		for (i = 0; i < 1000; i++) {                        \
			ASSERT_EQ_FMT(d[999 - i], F1(s, i), M);     \
		}                                                   \
		ASSERT_EQ_FMT(0, rnd_stack_destroy(s, NULL), "%d"); \
	} while (0)
	test(char          , rnd_stack_getc , rnd_stack_pushc , IRANGE(1, CHAR_MAX) , "%hd");
	test(short         , rnd_stack_gets , rnd_stack_pushs , IRANGE(1, SHRT_MAX) , "%hd");
	test(int           , rnd_stack_geti , rnd_stack_pushi , FRANGE(1, INT_MAX)  , "%d");
	test(long          , rnd_stack_getl , rnd_stack_pushl , FRANGE(1, LONG_MAX) , "%ld");
	test(signed char   , rnd_stack_getsc, rnd_stack_pushsc, IRANGE(1, SCHAR_MAX), "%hd");
	test(unsigned char , rnd_stack_getuc, rnd_stack_pushuc, IRANGE(1, UCHAR_MAX), "%hd");
	test(unsigned short, rnd_stack_getus, rnd_stack_pushus, IRANGE(1, USHRT_MAX), "%hu");
	test(unsigned int  , rnd_stack_getui, rnd_stack_pushui, FRANGE(1, UINT_MAX) , "%u");
	test(unsigned long , rnd_stack_getul, rnd_stack_pushul, FRANGE(1, ULONG_MAX), "%lu");
	test(float         , rnd_stack_getf , rnd_stack_pushf , FRANGE(1, FLT_MAX)  , "%f");
	test(double        , rnd_stack_getd , rnd_stack_pushd , FRANGE(1, DBL_MAX)  , "%f");
	test(long double   , rnd_stack_getld, rnd_stack_pushld, FRANGE(1, LDBL_MAX) , "%Lf");
#undef test
	PASS();
}

TEST t_set(void)
{
	struct rnd_stack *s;

	{ /* Generic form */
		unsigned i;
		int a = 1;
		s = rnd_stack_create(sizeof(int), 1000);
		ASSERT_NEQ(NULL, s);
		ASSERT_EQ_FMT(RND_EINDEX, rnd_stack_set(s, 0, &a), "%d");
		ASSERT_EQ_FMT(0, rnd_stack_pushi(s, 10), "%d");
		ASSERT_EQ_FMT(RND_EINVAL, rnd_stack_set(NULL, 0, &a), "%d");
		ASSERT_EQ_FMT(RND_EINVAL, rnd_stack_set(s, 0, NULL), "%d");
		ASSERT_EQ_FMT(RND_EINVAL, rnd_stack_set(NULL, 0, NULL), "%d");
		ASSERT_EQ_FMT(RND_EINDEX, rnd_stack_set(s, 1, &a), "%d");
		ASSERT_EQ_FMT(0, rnd_stack_set(s, 0, &a), "%d");
		ASSERT_EQ_FMT(a, rnd_stack_geti(s, 0), "%d");
		ASSERT_EQ_FMT(0, rnd_stack_destroy(s, NULL), "%d");
		s = rnd_stack_create(sizeof(struct data), 1000);
		ASSERT_NEQ(NULL, s);
		for (i = 0; i < 1000; i++) {
			struct data d;
			ASSERT_EQ_FMT(0, data_init(&d), "%d");
			ASSERT_EQ_FMT(0, rnd_stack_push(s, &d), "%d");
		}
		for (i = 0; i < 1000; i++) {
			struct data a, b;
			ASSERT_EQ_FMT(0, data_init(&b), "%d");
			ASSERT_EQ_FMT(0, rnd_stack_get(s, i, &a), "%d");
			ASSERT_EQ_FMT(0, data_dtor(&a), "%d");
			ASSERT_EQ_FMT(0, rnd_stack_set(s, i, &b), "%d");
			ASSERT_EQ_FMT(0, rnd_stack_get(s, i, &a), "%d");
			ASSERT_EQ_FMT(0, data_cmp(&a, &b), "%d");
		}
		ASSERT_EQ_FMT(0, rnd_stack_destroy(s, data_dtor), "%d");
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
		s = rnd_stack_create(sizeof(T), 1000);              \
		ASSERT_NEQ(NULL, s);                                \
		ASSERT_EQ_FMT(RND_EINDEX, F1(s, 0, a), "%d");       \
		ASSERT_EQ_FMT(0, F2(s, (V)), "%d");                 \
		ASSERT_EQ_FMT(RND_EINVAL, F1(NULL, 0, a), "%d");    \
		ASSERT_EQ_FMT(RND_EINDEX, F1(s, 1, a), "%d");       \
		ASSERT_EQ_FMT(0, F1(s, 0, a), "%d");                \
		ASSERT_EQ_FMT(a, F3(s, 0), M);                      \
		ASSERT_EQ_FMT(0, rnd_stack_clear(s, NULL), "%d");   \
		for (i = 0; i < 1000; i++) {                        \
			ASSERT_EQ_FMT(0, F2(s, (V)), "%d");         \
		}                                                   \
		for (i = 0; i < 1000; i++) {                        \
			T b = (V);                                  \
			ASSERT_EQ_FMT(0, F1(s, i, b), "%d");        \
			ASSERT_EQ_FMT(b, F3(s, i), M);              \
		}                                                   \
		ASSERT_EQ_FMT(0, rnd_stack_destroy(s, NULL), "%d"); \
		s = rnd_stack_create(sizeof(T) + 1, 1000);          \
		ASSERT_NEQ(NULL, s);                                \
		s->size = 1;                                        \
		ASSERT_EQ_FMT(RND_EILLEGAL, F1(s, 0, (V)), "%d");   \
		ASSERT_EQ_FMT(0, rnd_stack_destroy(s, NULL), "%d"); \
	} while (0)
	test(char          , rnd_stack_setc , rnd_stack_pushc , rnd_stack_getc , IRANGE(1, CHAR_MAX) , "%hd");
	test(short         , rnd_stack_sets , rnd_stack_pushs , rnd_stack_gets , IRANGE(1, SHRT_MAX) , "%hd");
	test(int           , rnd_stack_seti , rnd_stack_pushi , rnd_stack_geti , FRANGE(1, INT_MAX)  , "%d");
	test(long          , rnd_stack_setl , rnd_stack_pushl , rnd_stack_getl , FRANGE(1, LONG_MAX) , "%ld");
	test(signed char   , rnd_stack_setsc, rnd_stack_pushsc, rnd_stack_getsc, IRANGE(1, SCHAR_MAX), "%hd");
	test(unsigned char , rnd_stack_setuc, rnd_stack_pushuc, rnd_stack_getuc, IRANGE(1, UCHAR_MAX), "%hd");
	test(unsigned short, rnd_stack_setus, rnd_stack_pushus, rnd_stack_getus, IRANGE(1, USHRT_MAX), "%hu");
	test(unsigned int  , rnd_stack_setui, rnd_stack_pushui, rnd_stack_getui, FRANGE(1, UINT_MAX) , "%u");
	test(unsigned long , rnd_stack_setul, rnd_stack_pushul, rnd_stack_getul, FRANGE(1, ULONG_MAX), "%lu");
	test(float         , rnd_stack_setf , rnd_stack_pushf , rnd_stack_getf , FRANGE(1, FLT_MAX)  , "%f");
	test(double        , rnd_stack_setd , rnd_stack_pushd , rnd_stack_getd , FRANGE(1, DBL_MAX)  , "%f");
	test(long double   , rnd_stack_setld, rnd_stack_pushld, rnd_stack_getld, FRANGE(1, LDBL_MAX) , "%Lf");
#undef test
	PASS();
}

TEST t_print(void)
{
	struct rnd_stack *s;

	{ /* Generic form */
		double a = 4.5, b = -3.14;
		s = rnd_stack_create(sizeof(double), 30);
		ASSERT_EQ_FMT(0, rnd_stack_push(s, &a), "%d");
		ASSERT_EQ_FMT(0, rnd_stack_push(s, &b), "%d");
		ASSERT_EQ_FMT(RND_EINVAL, rnd_stack_print(NULL), "%d");
		ASSERT_EQ_FMT(0, rnd_stack_print(s), "%d");
		ASSERT_EQ_FMT(0, rnd_stack_destroy(s, NULL), "%d");
	}

	/* Suffixed form
	 * T  - type
	 * F1 - push function
	 * F2 - print function
	 * A  - 1st value
	 * B  - 2nd value
	 */
#define test(T, F1, F2, A, B)                                       \
	do {                                                        \
		T a = A, b = B;                                     \
		s = rnd_stack_create(sizeof(T), 30);                \
		ASSERT_EQ_FMT(0, F1(s, a), "%d");                   \
		ASSERT_EQ_FMT(0, F1(s, b), "%d");                   \
		ASSERT_EQ_FMT(RND_EINVAL, F2(NULL), "%d");          \
		ASSERT_EQ_FMT(0, F2(s), "%d");                      \
		ASSERT_EQ_FMT(0, rnd_stack_destroy(s, NULL), "%d"); \
		s = rnd_stack_create(sizeof(T) + 1, 30);            \
		ASSERT_EQ_FMT(RND_EILLEGAL, F2(s), "%d");           \
		ASSERT_EQ_FMT(0, rnd_stack_destroy(s, NULL), "%d"); \
	} while(0)

	test(char          , rnd_stack_pushc , rnd_stack_printc , 'A', 'B');
	test(short         , rnd_stack_pushs , rnd_stack_prints , SHRT_MIN, SHRT_MAX);
	test(int           , rnd_stack_pushi , rnd_stack_printi , INT_MIN, INT_MAX);
	test(long          , rnd_stack_pushl , rnd_stack_printl , LONG_MIN, LONG_MAX);
	test(signed char   , rnd_stack_pushsc, rnd_stack_printsc, SCHAR_MIN, SCHAR_MAX);
	test(unsigned char , rnd_stack_pushuc, rnd_stack_printuc, 0, UCHAR_MAX);
	test(unsigned short, rnd_stack_pushus, rnd_stack_printus, 0, USHRT_MAX);
	test(unsigned int  , rnd_stack_pushui, rnd_stack_printui, 0, UINT_MAX);
	test(unsigned long , rnd_stack_pushul, rnd_stack_printul, 0, ULONG_MAX);
	test(float         , rnd_stack_pushf , rnd_stack_printf , FLT_MIN, FLT_MAX);
	test(double        , rnd_stack_pushd , rnd_stack_printd , DBL_MIN, DBL_MAX);
	test(long double   , rnd_stack_pushld, rnd_stack_printld, LDBL_MIN, LDBL_MAX);

	PASS();
}

SUITE(stack) {
	RUN_TEST(t_create);
	RUN_TEST(t_destroy);
	RUN_TEST(t_push);
	RUN_TEST(t_peek);
	RUN_TEST(t_pop);
	RUN_TEST(t_clear);
	RUN_TEST(t_foreach);
	RUN_TEST(t_copy);
	RUN_TEST(t_insert);
	RUN_TEST(t_qinsert);
	RUN_TEST(t_remove);
	RUN_TEST(t_qremove);
	RUN_TEST(t_get);
	RUN_TEST(t_set);
	RUN_TEST(t_print);
}

int main(int argc, char **argv)
{
	GREATEST_MAIN_BEGIN();

	srand(time(NULL));
	RUN_SUITE(stack);

	GREATEST_MAIN_END();
}
