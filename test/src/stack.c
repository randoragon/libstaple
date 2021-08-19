#include <stdlib.h>
#include <time.h>
#include "../../src/rnd.h"
#include "test_struct.h"
#include <criterion/criterion.h>
#include <limits.h>
#include <float.h>

/* Make testing for size overflow feasible */
#ifdef SIZE_MAX
#undef SIZE_MAX
#endif
#define SIZE_MAX 65535LU

void setup(void)
{
	if (!rnd_is_debug())
		cr_log_error("librnd is not running in debug mode");
}

TestSuite(stack, .init=setup);

Test(stack, create)
{
	struct rnd_stack *s;
	s = rnd_stack_create(sizeof(int), 0);
	cr_assert_null(s);
	s = rnd_stack_create(0, 16);
	cr_assert_null(s);
	s = rnd_stack_create(0, 0);
	cr_assert_null(s);
	s = rnd_stack_create(SIZE_MAX, SIZE_MAX);
	cr_assert_null(s);
	s = rnd_stack_create(sizeof(int), 16);
	cr_assert_not_null(s);
	cr_assert_eq(0, rnd_stack_destroy(s, NULL));
}

Test(stack, destroy)
{
	struct rnd_stack *s;
	unsigned i;
	s = rnd_stack_create(sizeof(long double), 1000);
	cr_assert_not_null(s);
	cr_assert_eq(RND_EINVAL, rnd_stack_destroy(NULL, NULL));
	cr_assert_eq(0, rnd_stack_destroy(s, NULL));
	s = rnd_stack_create(sizeof(struct data), 1000);
	for (i = 0; i < 1000; i++) {
		struct data d;
		cr_assert_eq(0, data_init(&d));
		cr_assert_eq(0, rnd_stack_push(s, &d));
	}
	cr_assert_eq(RND_EHANDLER, rnd_stack_destroy(s, data_dtor_bad));
	cr_assert_eq(0, rnd_stack_destroy(s, data_dtor));
}

Test(stack, push)
{
	struct rnd_stack *s;

	{ /* Generic form */
		unsigned i;
		struct data d;
		s = rnd_stack_create(sizeof(struct data), 1000);
		cr_assert_not_null(s);
		data_init(&d);
		cr_assert_eq(RND_EINVAL, rnd_stack_push(s, NULL));
		cr_assert_eq(RND_EINVAL, rnd_stack_push(NULL, &d));
		cr_assert_eq(RND_EINVAL, rnd_stack_push(NULL, NULL));
		cr_assert_eq(0LU, (unsigned long)s->size, "%lu");
		cr_assert_eq(0, rnd_stack_push(s, &d));
		cr_assert_eq(1LU, (unsigned long)s->size, "%lu");
		cr_assert_eq(0, rnd_stack_push(s, &d));
		cr_assert_eq(2LU, (unsigned long)s->size, "%lu");
		cr_assert_eq(0, rnd_stack_clear(s, NULL));
		for (i = 0; i < SIZE_MAX / sizeof(struct data); i++) {
			struct data a, b;
			cr_assert_eq(0, data_init(&a));
			cr_assert_eq(0, rnd_stack_push(s, &a));
			cr_assert_eq(0, rnd_stack_get(s, 0, &b));
			cr_assert_eq(0, data_cmp(&a, &b));
		}
		cr_assert_eq(RND_ERANGE, rnd_stack_push(s, &d));
		cr_assert_eq(0, data_dtor(&d));
		cr_assert_eq(0, rnd_stack_destroy(s, data_dtor));
	}

	/* Suffixed form
	 * T  - type
	 * F1 - push function
	 * F2 - get function
	 * V  - random value snippet
	 * M  - printf format string
	 */
#define test(T, F1, F2, V, M) do {                                \
		unsigned i;                                       \
		s = rnd_stack_create(sizeof(T), 1000);            \
		cr_assert_not_null(s);                            \
		cr_assert_eq(RND_EINVAL, F1(NULL, (V)));          \
		cr_assert_eq(0LU, (unsigned long)s->size, "%lu"); \
		cr_assert_eq(0, F1(s, (V)));                      \
		cr_assert_eq(1LU, (unsigned long)s->size, "%lu"); \
		cr_assert_eq(0, F1(s, (V)));                      \
		cr_assert_eq(2LU, (unsigned long)s->size, "%lu"); \
		cr_assert_eq(0, rnd_stack_clear(s, NULL));        \
		for (i = 0; i < SIZE_MAX / sizeof(T); i++) {      \
			T a = (V);                                \
			cr_assert_eq(0, F1(s, a));                \
			cr_assert_eq(a, F2(s, 0), M);             \
		}                                                 \
		cr_assert_eq(RND_ERANGE, F1(s, (V)));             \
		cr_assert_eq(0, rnd_stack_destroy(s, NULL));      \
		s = rnd_stack_create(sizeof(T) + 1, 1);           \
		cr_assert_not_null(s);                            \
		cr_assert_eq(RND_EILLEGAL, F1(s, (V)));           \
		cr_assert_eq(0, rnd_stack_destroy(s, NULL));      \
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
}

Test(stack, peek)
{
	struct rnd_stack *s;

	{ /* Generic form */
		struct data a, b;
		s = rnd_stack_create(sizeof(struct data), 2);
		cr_assert_not_null(s);
		cr_assert_eq(RND_EILLEGAL, rnd_stack_peek(s, &b));
		cr_assert_eq(RND_EINVAL, rnd_stack_peek(s, NULL));
		cr_assert_eq(RND_EINVAL, rnd_stack_peek(NULL, &b));
		cr_assert_eq(RND_EINVAL, rnd_stack_peek(NULL, NULL));
		data_init(&a);
		cr_assert_eq(0, rnd_stack_push(s, &a));
		cr_assert_eq(0, rnd_stack_peek(s, &b));
		cr_assert_eq(0, data_cmp(&a, &b));
		cr_assert_eq(0, rnd_stack_destroy(s, data_dtor));
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
		s = rnd_stack_create(sizeof(T), 2);          \
		cr_assert_not_null(s);                       \
		cr_assert_eq(z, F1(s), M);                   \
		cr_assert_eq(z, F1(NULL), M);                \
		cr_assert_eq(0, F2(s, a));                   \
		cr_assert_eq(a, F1(s), M);                   \
		cr_assert_eq(0, rnd_stack_destroy(s, NULL)); \
		s = rnd_stack_create(sizeof(T) + 1, 1);      \
		cr_assert_not_null(s);                       \
		s->size = 1;                                 \
		cr_assert_eq(z, F1(s), M);                   \
		cr_assert_eq(0, rnd_stack_destroy(s, NULL)); \
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
}

Test(stack, pop)
{
	struct rnd_stack *s;

	{ /* Generic form */
		struct data a, b;
		s = rnd_stack_create(sizeof(struct data), 2);
		cr_assert_not_null(s);
		cr_assert_eq(RND_EILLEGAL, rnd_stack_pop(s, NULL));
		cr_assert_eq(RND_EINVAL, rnd_stack_pop(NULL, NULL));
		data_init(&a);
		cr_assert_eq(0, rnd_stack_push(s, &a));
		cr_assert_eq(1LU, (unsigned long)s->size, "%lu");
		cr_assert_eq(0, rnd_stack_pop(s, &b));
		cr_assert_eq(0LU, (unsigned long)s->size, "%lu");
		cr_assert_eq(0, data_cmp(&a, &b));
		cr_assert_eq(0, rnd_stack_destroy(s, NULL));
		cr_assert_eq(0, data_dtor(&b));
	}

	/* Suffixed form
	 * T  - type
	 * F1 - pop function
	 * F2 - push function
	 * V  - random value snippet
	 * M  - printf format string
	 */
#define test(T, F1, F2, V, M) do {                                \
		T a = (V), z = 0;                                 \
		s = rnd_stack_create(sizeof(T), 2);               \
		cr_assert_not_null(s);                            \
		cr_assert_eq(z, F1(s), M);                        \
		cr_assert_eq(z, F1(NULL), M);                     \
		cr_assert_eq(0, F2(s, a));                        \
		cr_assert_eq(1LU, (unsigned long)s->size, "%lu"); \
		cr_assert_eq(a, F1(s), M);                        \
		cr_assert_eq(0LU, (unsigned long)s->size, "%lu"); \
		cr_assert_eq(0, rnd_stack_destroy(s, NULL));      \
		s = rnd_stack_create(sizeof(T) + 1, 1);           \
		cr_assert_not_null(s);                            \
		s->size = 1;                                      \
		cr_assert_eq(z, F1(s), M);                        \
		cr_assert_eq(0, rnd_stack_destroy(s, NULL));      \
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
}

Test(stack, clear)
{
	struct rnd_stack *s;
	unsigned i;
	s = rnd_stack_create(sizeof(long double), 1000);
	cr_assert_not_null(s);
	cr_assert_eq(RND_EINVAL, rnd_stack_clear(NULL, NULL));
	cr_assert_eq(0, rnd_stack_clear(s, NULL));
	cr_assert_eq(0LU, (unsigned long)s->size, "%lu");
	cr_assert_eq(0, rnd_stack_destroy(s, NULL));
	s = rnd_stack_create(sizeof(struct data), 1000);
	for (i = 0; i < 1000; i++) {
		struct data d;
		cr_assert_eq(0, data_init(&d));
		cr_assert_eq(0, rnd_stack_push(s, &d));
	}
	cr_assert_eq(RND_EHANDLER, rnd_stack_clear(s, data_dtor_bad));
	cr_assert_eq(0, rnd_stack_clear(s, data_dtor));
	cr_assert_eq(0LU, (unsigned long)s->size, "%lu");
	cr_assert_eq(0, rnd_stack_destroy(s, NULL));
}

Test(stack, foreach)
{
	struct rnd_stack *s;
	unsigned i;
	s = rnd_stack_create(sizeof(long double), 1000);
	cr_assert_not_null(s);
	cr_assert_eq(RND_EINVAL, rnd_stack_foreach(NULL, data_mutate));
	cr_assert_eq(RND_EINVAL, rnd_stack_foreach(s, NULL));
	cr_assert_eq(RND_EINVAL, rnd_stack_foreach(NULL, NULL));
	cr_assert_eq(0, rnd_stack_destroy(s, NULL));
	s = rnd_stack_create(sizeof(struct data), 1000);
	for (i = 0; i < 1000; i++) {
		struct data d;
		cr_assert_eq(0, data_init(&d));
		cr_assert_eq(0, rnd_stack_push(s, &d));
	}
	cr_assert_eq(RND_EHANDLER, rnd_stack_foreach(s, data_mutate_bad));
	cr_assert_eq(0, rnd_stack_foreach(s, data_mutate));
	cr_assert_eq(0, rnd_stack_foreach(s, data_verify));
	cr_assert_eq(0, rnd_stack_destroy(s, data_dtor));
}

Test(stack, copy)
{
	struct rnd_stack *s, *p;
	unsigned i;
	s = rnd_stack_create(sizeof(int), 1000);
	p = rnd_stack_create(sizeof(int), 333);
	cr_assert_not_null(s);
	cr_assert_eq(RND_EINVAL, rnd_stack_copy(NULL, s, NULL));
	cr_assert_eq(RND_EINVAL, rnd_stack_copy(p, NULL, NULL));
	cr_assert_eq(RND_EINVAL, rnd_stack_copy(NULL, NULL, NULL));
	cr_assert_eq(0, rnd_stack_copy(s, p, NULL));
	cr_assert_eq((unsigned long)p->size, (unsigned long)s->size, "%lu");
	cr_assert_eq((unsigned long)p->elem_size, (unsigned long)s->elem_size, "%lu");
	cr_assert_eq(0, rnd_stack_copy(p, s, NULL));
	cr_assert_eq((unsigned long)p->size, (unsigned long)s->size, "%lu");
	cr_assert_eq((unsigned long)p->elem_size, (unsigned long)s->elem_size, "%lu");
	for (i = 0; i < 1000; i++) {
		cr_assert_eq(0, rnd_stack_pushi(s, FRANGE(INT_MIN, INT_MAX)));
	}
	cr_assert_eq(0, rnd_stack_copy(p, s, NULL));
	cr_assert_eq((unsigned long)p->size, (unsigned long)s->size, "%lu");
	cr_assert_eq((unsigned long)p->elem_size, (unsigned long)s->elem_size, "%lu");
	for (i = 0; i < 1000; i++) {
		int a, b;
		a = rnd_stack_geti(s, i);
		b = rnd_stack_geti(p, i);
		cr_assert_eq(a, b);
	}
	cr_assert_eq(0, rnd_stack_destroy(s, NULL));
	cr_assert_eq(0, rnd_stack_clear(p, NULL));
	s = rnd_stack_create(sizeof(struct data), 1000);
	for (i = 0; i < 1000; i++) {
		struct data d;
		cr_assert_eq(0, data_init(&d));
		cr_assert_eq(0, rnd_stack_push(s, &d));
	}
	cr_assert_eq(RND_EHANDLER, rnd_stack_copy(p, s, data_cpy_bad));
	cr_assert_eq(0, rnd_stack_copy(p, s, data_cpy));
	cr_assert_eq((unsigned long)p->size, (unsigned long)s->size, "%lu");
	cr_assert_eq((unsigned long)p->elem_size, (unsigned long)s->elem_size, "%lu");
	for (i = 0; i < 1000; i++) {
		struct data a, b;
		cr_assert_eq(0, rnd_stack_get(s, i, &a));
		cr_assert_eq(0, rnd_stack_get(p, i, &b));
		cr_assert_eq(0, data_cmp(&a, &b));
	}
	cr_assert_eq(0, rnd_stack_destroy(s, data_dtor));
	cr_assert_eq(0, rnd_stack_destroy(p, data_dtor));
}

Test(stack, insert)
{
	struct rnd_stack *s;
	unsigned i;

	{ /* Generic form */
		int a = 10;
		struct data d[1000];
		s = rnd_stack_create(sizeof(int), 1000);
		cr_assert_not_null(s);
		cr_assert_eq(RND_EINVAL, rnd_stack_insert(s, 0, NULL));
		cr_assert_eq(RND_EINVAL, rnd_stack_insert(NULL, 0, &a));
		cr_assert_eq(RND_EINVAL, rnd_stack_insert(NULL, 0, NULL));
		cr_assert_eq(RND_EINDEX, rnd_stack_insert(s, 1, &a));
		cr_assert_eq(0, rnd_stack_insert(s, 0, &a));
		cr_assert_eq(1LU, (unsigned long)s->size, "%lu");
		cr_assert_eq(10, rnd_stack_geti(s, 0));
		cr_assert_eq(0, rnd_stack_destroy(s, NULL));

		s = rnd_stack_create(sizeof(struct data), 1000);
		cr_assert_not_null(s);
		for (i = 0; i < 1000; i++) {
			struct data a;
			size_t idx = IRANGE(0, i);
			cr_assert_eq(0, data_init(d + i));
			cr_assert_eq(0, rnd_stack_insert(s, idx, d + i));
			cr_assert_eq((unsigned long)i + 1, (unsigned long)s->size, "%lu");
			cr_assert_eq(0, rnd_stack_get(s, idx, &a));
			cr_assert_eq(0, data_cmp(&a, d + i));
		}
		cr_assert_eq(0, rnd_stack_destroy(s, data_dtor));

		s = rnd_stack_create(sizeof(struct data), 10);
		cr_assert_not_null(s);
		for (i = 0; i < 5; i++) {
			cr_assert_eq(0, data_init(d + i));
			cr_assert_eq(0, rnd_stack_insert(s, i, d + i));
		}
		for (i = 0; i < 5; i++) {
			struct data a;
			cr_assert_eq(0, rnd_stack_get(s, i, &a));
			cr_assert_eq(0, data_cmp(&a, d + i));
		}
		{
			struct data a;
			cr_assert_eq(0, data_init(d + 5));
			cr_assert_eq(0, rnd_stack_insert(s, 1, d + 5));
			cr_assert_eq(0, rnd_stack_peek(s, &a));
			cr_assert_eq(0, data_cmp(&a, d));
			cr_assert_eq(0, data_init(d + 6));
			cr_assert_eq(0, rnd_stack_insert(s, 3, d + 6));
			cr_assert_eq(0, rnd_stack_peek(s, &a));
			cr_assert_eq(0, data_cmp(&a, d));
			cr_assert_eq(7LU, (unsigned long)s->size, "%lu");
			cr_assert_eq(0, rnd_stack_destroy(s, data_dtor));
		}
	}

	/* Suffixed form
	 * T  - type
	 * F1 - insert function
	 * F2 - get function
	 * V  - random value snippet
	 * M  - printf format string
	 */
#define test(T, F1, F2, V, M) do {                                                         \
		T a = (V);                                                                 \
		T d[1000];                                                                 \
		s = rnd_stack_create(sizeof(T), 1000);                                     \
		cr_assert_not_null(s);                                                     \
		cr_assert_eq(RND_EINVAL, F1(NULL, 0, a));                                  \
		cr_assert_eq(RND_EINDEX, F1(s, 1, a));                                     \
		cr_assert_eq(0, F1(s, 0, a));                                              \
		cr_assert_eq(1LU, (unsigned long)s->size, "%lu");                          \
		cr_assert_eq(a, F2(s, 0), M);                                              \
		cr_assert_eq(0, rnd_stack_destroy(s, NULL));                               \
                                                                                           \
		s = rnd_stack_create(sizeof(T), 1000);                                     \
		cr_assert_not_null(s);                                                     \
		for (i = 0; i < 1000; i++) {                                               \
			size_t idx = IRANGE(0, i);                                         \
			d[i] = (V);                                                        \
			cr_assert_eq(0, F1(s, idx, d[i]));                                 \
			cr_assert_eq((unsigned long)i + 1, (unsigned long)s->size, "%lu"); \
			cr_assert_eq(d[i], F2(s, idx), M);                                 \
		}                                                                          \
		cr_assert_eq(0, rnd_stack_destroy(s, NULL));                               \
                                                                                           \
		s = rnd_stack_create(sizeof(T), 10);                                       \
		cr_assert_not_null(s);                                                     \
		for (i = 0; i < 5; i++) {                                                  \
			d[i] = (V);                                                        \
			cr_assert_eq(0, F1(s, i, d[i]));                                   \
		}                                                                          \
		for (i = 0; i < 5; i++) {                                                  \
			cr_assert_eq(d[i], F2(s, i), M);                                   \
		}                                                                          \
		d[5] = (V);                                                                \
		cr_assert_eq(0, F1(s, 1, d[5]));                                           \
		cr_assert_eq(d[0], F2(s, 0), M);                                           \
		d[6] = (V);                                                                \
		cr_assert_eq(0, F1(s, 3, d[6]));                                           \
		cr_assert_eq(d[0], F2(s, 0), M);                                           \
		cr_assert_eq(0, rnd_stack_destroy(s, NULL));                               \
		s = rnd_stack_create(sizeof(T) + 1, 1000);                                 \
		cr_assert_not_null(s);                                                     \
		cr_assert_eq(RND_EILLEGAL, F1(s, 0, (V)));                                 \
		cr_assert_eq(0, rnd_stack_destroy(s, NULL));                               \
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
}

Test(stack, qinsert)
{
	struct rnd_stack *s;
	unsigned i;

	{ /* Generic form */
		int a = 10;
		struct data d[1000];
		s = rnd_stack_create(sizeof(int), 1000);
		cr_assert_not_null(s);
		cr_assert_eq(RND_EINVAL, rnd_stack_qinsert(s, 0, NULL));
		cr_assert_eq(RND_EINVAL, rnd_stack_qinsert(NULL, 0, &a));
		cr_assert_eq(RND_EINVAL, rnd_stack_qinsert(NULL, 0, NULL));
		cr_assert_eq(RND_EINDEX, rnd_stack_qinsert(s, 1, &a));
		cr_assert_eq(0, rnd_stack_qinsert(s, 0, &a));
		cr_assert_eq(1LU, (unsigned long)s->size, "%lu");
		cr_assert_eq(10, rnd_stack_geti(s, 0));
		cr_assert_eq(0, rnd_stack_destroy(s, NULL));

		s = rnd_stack_create(sizeof(struct data), 1000);
		cr_assert_not_null(s);
		for (i = 0; i < 1000; i++) {
			struct data a;
			size_t idx = IRANGE(0, i);
			cr_assert_eq(0, data_init(d + i));
			cr_assert_eq(0, rnd_stack_qinsert(s, idx, d + i));
			cr_assert_eq((unsigned long)i + 1, (unsigned long)s->size, "%lu");
			cr_assert_eq(0, rnd_stack_get(s, idx, &a));
			cr_assert_eq(0, data_cmp(&a, d + i));
		}
		cr_assert_eq(0, rnd_stack_destroy(s, data_dtor));

		s = rnd_stack_create(sizeof(struct data), 10);
		cr_assert_not_null(s);
		for (i = 0; i < 5; i++) {
			cr_assert_eq(0, data_init(d + i));
			cr_assert_eq(0, rnd_stack_qinsert(s, i, d + i));
		}
		for (i = 0; i < 5; i++) {
			struct data a;
			cr_assert_eq(0, rnd_stack_get(s, i, &a));
			cr_assert_eq(0, data_cmp(&a, d + (8 - i) % 5));
		}
		{
			struct data a;
			cr_assert_eq(0, data_init(d + 5));
			cr_assert_eq(0, rnd_stack_qinsert(s, 1, d + 5));
			cr_assert_eq(0, rnd_stack_peek(s, &a));
			cr_assert_eq(0, data_cmp(&a, d + 3));
			cr_assert_eq(0, data_init(d + 6));
			cr_assert_eq(0, rnd_stack_qinsert(s, 3, d + 6));
			cr_assert_eq(0, rnd_stack_peek(s, &a));
			cr_assert_eq(0, data_cmp(&a, d + 2));
			cr_assert_eq(7LU, (unsigned long)s->size, "%lu");
			cr_assert_eq(0, rnd_stack_destroy(s, data_dtor));
		}
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
		s = rnd_stack_create(sizeof(T), 1000);                                     \
		cr_assert_not_null(s);                                                     \
		cr_assert_eq(RND_EINVAL, F1(NULL, 0, a));                                  \
		cr_assert_eq(RND_EINDEX, F1(s, 1, a));                                     \
		cr_assert_eq(0, F1(s, 0, a));                                              \
		cr_assert_eq(1LU, (unsigned long)s->size, "%lu");                          \
		cr_assert_eq(a, F2(s, 0), M);                                              \
		cr_assert_eq(0, rnd_stack_destroy(s, NULL));                               \
                                                                                           \
		s = rnd_stack_create(sizeof(T), 1000);                                     \
		cr_assert_not_null(s);                                                     \
		for (i = 0; i < 1000; i++) {                                               \
			size_t idx = IRANGE(0, i);                                         \
			d[i] = (V);                                                        \
			cr_assert_eq(0, F1(s, idx, d[i]));                                 \
			cr_assert_eq((unsigned long)i + 1, (unsigned long)s->size, "%lu"); \
			cr_assert_eq(d[i], F2(s, idx), M);                                 \
		}                                                                          \
		cr_assert_eq(0, rnd_stack_destroy(s, NULL));                               \
                                                                                           \
		s = rnd_stack_create(sizeof(T), 10);                                       \
		cr_assert_not_null(s);                                                     \
		for (i = 0; i < 5; i++) {                                                  \
			d[i] = (V);                                                        \
			cr_assert_eq(0, F1(s, i, d[i]));                                   \
		}                                                                          \
		for (i = 0; i < 5; i++) {                                                  \
			cr_assert_eq(d[(8 - i) % 5], F2(s, i), M);                         \
		}                                                                          \
		d[5] = (V);                                                                \
		cr_assert_eq(0, F1(s, 1, d[5]));                                           \
		cr_assert_eq(d[3], F2(s, 0), M);                                           \
		d[6] = (V);                                                                \
		cr_assert_eq(0, F1(s, 3, d[6]));                                           \
		cr_assert_eq(d[2], F2(s, 0), M);                                           \
		cr_assert_eq(0, rnd_stack_destroy(s, NULL));                               \
		s = rnd_stack_create(sizeof(T) + 1, 1000);                                 \
		cr_assert_not_null(s);                                                     \
		cr_assert_eq(RND_EILLEGAL, F1(s, 0, (V)));                                 \
		cr_assert_eq(0, rnd_stack_destroy(s, NULL));                               \
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
}

Test(stack, remove)
{
	struct rnd_stack *s;
	unsigned i;

	{ /* Generic form */
		struct data a;
		struct data d[100];
		s = rnd_stack_create(sizeof(int), 1000);
		cr_assert_not_null(s);
		cr_assert_eq(RND_EINDEX, rnd_stack_remove(s, 0, &a));
		cr_assert_eq(0, data_init(&a));
		cr_assert_eq(0, rnd_stack_push(s, &a));
		cr_assert_eq(RND_EINVAL, rnd_stack_remove(NULL, 0, &a));
		cr_assert_eq(RND_EINVAL, rnd_stack_remove(NULL, 0, NULL));
		cr_assert_eq(RND_EINDEX, rnd_stack_remove(s, 1, &a));
		cr_assert_eq(1LU, (unsigned long)s->size, "%lu");
		cr_assert_eq(0, rnd_stack_remove(s, 0, NULL));
		cr_assert_eq(0LU, (unsigned long)s->size, "%lu");
		cr_assert_eq(0, rnd_stack_destroy(s, NULL));
		cr_assert_eq(0, data_dtor(&a));

		s = rnd_stack_create(sizeof(struct data), 100);
		cr_assert_not_null(s);
		for (i = 0; i < 100; i++) {
			cr_assert_eq(0, data_init(d + i));
			cr_assert_eq(0, rnd_stack_push(s, d + i));
		}
		for (i = 0; i < 100; i++) {
			struct data a;
			size_t idx = IRANGE(0, s->size - 1), j;
			int found = 0;
			cr_assert_eq(0, rnd_stack_remove(s, idx, &a));
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
		cr_assert_eq(0, rnd_stack_destroy(s, NULL));

		s = rnd_stack_create(sizeof(struct data), 10);
		cr_assert_not_null(s);
		for (i = 0; i < 8; i++) {
			cr_assert_eq(0, rnd_stack_push(s, d + i));
		}
		cr_assert_eq(0, rnd_stack_remove(s, 3, &a));
		cr_assert_eq(0, data_cmp(d + 4, &a));
		cr_assert_eq(7LU, (unsigned long)s->size, "%lu");
		cr_assert_eq(0, rnd_stack_remove(s, 0, &a));
		cr_assert_eq(0, data_cmp(d + 7, &a));
		cr_assert_eq(6LU, (unsigned long)s->size, "%lu");
		cr_assert_eq(0, rnd_stack_remove(s, 3, &a));
		cr_assert_eq(0, data_cmp(d + 2, &a));
		cr_assert_eq(5LU, (unsigned long)s->size, "%lu");
		cr_assert_eq(0, rnd_stack_destroy(s, NULL));
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
		s = rnd_stack_create(sizeof(T), 1000);                             \
		cr_assert_not_null(s);                                             \
		cr_assert_eq(z, F1(s, 0), M);                                      \
		cr_assert_eq(0, F2(s, a));                                         \
		cr_assert_eq(z, F1(NULL, 0), M);                                   \
		cr_assert_eq(z, F1(s, 1), M);                                      \
		cr_assert_eq(1LU, (unsigned long)s->size, "%lu");                  \
		cr_assert_eq(a, F1(s, 0), M);                                      \
		cr_assert_eq(0LU, (unsigned long)s->size, "%lu");                  \
		cr_assert_eq(0, rnd_stack_destroy(s, NULL));                       \
                                                                                   \
		s = rnd_stack_create(sizeof(T), 100);                              \
		cr_assert_not_null(s);                                             \
		for (i = 0; i < 100; i++) {                                        \
			d[i] = (V);                                                \
			cr_assert_eq(0, F2(s, d[i]));                              \
		}                                                                  \
		for (i = 0; i < 100; i++) {                                        \
			T a;                                                       \
			size_t idx = IRANGE(0, s->size - 1), j;                    \
			int found = 0;                                             \
			cr_assert_neq(0, (a = F1(s, idx)));                        \
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
		cr_assert_eq(0, rnd_stack_destroy(s, NULL));                       \
                                                                                   \
		s = rnd_stack_create(sizeof(T), 10);                               \
		cr_assert_not_null(s);                                             \
		for (i = 0; i < 8; i++) {                                          \
			cr_assert_eq(0, F2(s, d[i]));                              \
		}                                                                  \
		cr_assert_eq(d[4], F1(s, 3), M);                                   \
		cr_assert_eq(7LU, (unsigned long)s->size, "%lu");                  \
		cr_assert_eq(d[7], F1(s, 0), M);                                   \
		cr_assert_eq(6LU, (unsigned long)s->size, "%lu");                  \
		cr_assert_eq(d[2], F1(s, 3), M);                                   \
		cr_assert_eq(5LU, (unsigned long)s->size, "%lu");                  \
		cr_assert_eq(0, rnd_stack_destroy(s, NULL));                       \
		s = rnd_stack_create(sizeof(T) + 1, 1000);                         \
		cr_assert_not_null(s);                                             \
		s->size = 1;                                                       \
		cr_assert_eq(z, F1(s, 0), M);                                      \
		cr_assert_eq(0, rnd_stack_destroy(s, NULL));                       \
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
}

Test(stack, qremove)
{
	struct rnd_stack *s;
	unsigned i;

	{ /* Generic form */
		struct data a;
		struct data d[100];
		s = rnd_stack_create(sizeof(int), 1000);
		cr_assert_not_null(s);
		cr_assert_eq(RND_EINDEX, rnd_stack_qremove(s, 0, &a));
		cr_assert_eq(0, data_init(&a));
		cr_assert_eq(0, rnd_stack_push(s, &a));
		cr_assert_eq(RND_EINVAL, rnd_stack_qremove(NULL, 0, &a));
		cr_assert_eq(RND_EINVAL, rnd_stack_qremove(NULL, 0, NULL));
		cr_assert_eq(RND_EINDEX, rnd_stack_qremove(s, 1, &a));
		cr_assert_eq(1LU, (unsigned long)s->size, "%lu");
		cr_assert_eq(0, rnd_stack_qremove(s, 0, NULL));
		cr_assert_eq(0LU, (unsigned long)s->size, "%lu");
		cr_assert_eq(0, rnd_stack_destroy(s, NULL));
		cr_assert_eq(0, data_dtor(&a));

		s = rnd_stack_create(sizeof(struct data), 100);
		cr_assert_not_null(s);
		for (i = 0; i < 100; i++) {
			cr_assert_eq(0, data_init(d + i));
			cr_assert_eq(0, rnd_stack_push(s, d + i));
		}
		for (i = 0; i < 100; i++) {
			struct data a;
			size_t idx = IRANGE(0, s->size - 1), j;
			int found = 0;
			cr_assert_eq(0, rnd_stack_qremove(s, idx, &a));
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
		cr_assert_eq(0, rnd_stack_destroy(s, NULL));

		s = rnd_stack_create(sizeof(struct data), 10);
		cr_assert_not_null(s);
		for (i = 0; i < 8; i++) {
			cr_assert_eq(0, rnd_stack_push(s, d + i));
		}
		cr_assert_eq(0, rnd_stack_qremove(s, 3, &a));
		cr_assert_eq(0, data_cmp(d + 4, &a));
		cr_assert_eq(0, rnd_stack_get(s, 3, &a));
		cr_assert_eq(0, data_cmp(d + 3, &a));
		cr_assert_eq(0, rnd_stack_qremove(s, 0, &a));
		cr_assert_eq(0, data_cmp(d + 6, &a));
		cr_assert_eq(0, rnd_stack_get(s, 0, &a));
		cr_assert_eq(0, data_cmp(d + 5, &a));
		cr_assert_eq(0, rnd_stack_qremove(s, 3, &a));
		cr_assert_eq(0, data_cmp(d + 2, &a));
		cr_assert_eq(0, rnd_stack_destroy(s, NULL));
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
		s = rnd_stack_create(sizeof(T), 1000);                             \
		cr_assert_not_null(s);                                             \
		cr_assert_eq(z, F1(s, 0), M);                                      \
		cr_assert_eq(0, F2(s, a));                                         \
		cr_assert_eq(z, F1(NULL, 0), M);                                   \
		cr_assert_eq(z, F1(s, 1), M);                                      \
		cr_assert_eq(1LU, (unsigned long)s->size, "%lu");                  \
		cr_assert_eq(a, F1(s, 0), M);                                      \
		cr_assert_eq(0LU, (unsigned long)s->size, "%lu");                  \
		cr_assert_eq(0, rnd_stack_destroy(s, NULL));                       \
                                                                                   \
		s = rnd_stack_create(sizeof(T), 100);                              \
		cr_assert_not_null(s);                                             \
		for (i = 0; i < 100; i++) {                                        \
			d[i] = (V);                                                \
			cr_assert_eq(0, F2(s, d[i]));                              \
		}                                                                  \
		for (i = 0; i < 100; i++) {                                        \
			T a;                                                       \
			size_t idx = IRANGE(0, s->size - 1), j;                    \
			int found = 0;                                             \
			cr_assert_neq(0, (a = F1(s, idx)));                        \
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
		cr_assert_eq(0, rnd_stack_destroy(s, NULL));                       \
                                                                                   \
		s = rnd_stack_create(sizeof(T), 10);                               \
		cr_assert_not_null(s);                                             \
		for (i = 0; i < 8; i++) {                                          \
			cr_assert_eq(0, F2(s, d[i]));                              \
		}                                                                  \
		cr_assert_eq(d[4], F1(s, 3), M);                                   \
		cr_assert_eq(d[3], F3(s, 3), M);                                   \
		cr_assert_eq(d[6], F1(s, 0), M);                                   \
		cr_assert_eq(d[5], F3(s, 0), M);                                   \
		cr_assert_eq(d[2], F1(s, 3), M);                                   \
		cr_assert_eq(d[1], F3(s, 3), M);                                   \
		cr_assert_eq(0, rnd_stack_destroy(s, NULL));                       \
		s = rnd_stack_create(sizeof(T) + 1, 1000);                         \
		cr_assert_not_null(s);                                             \
		s->size = 1;                                                       \
		cr_assert_eq(z, F1(s, 0), M);                                      \
		cr_assert_eq(0, rnd_stack_destroy(s, NULL));                       \
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
}

Test(stack, get)
{
	struct rnd_stack *s;

	{ /* Generic form */
		unsigned i;
		int a = 0;
		struct data d[1000];
		s = rnd_stack_create(sizeof(int), 1000);
		cr_assert_not_null(s);
		cr_assert_eq(RND_EINDEX, rnd_stack_get(s, 0, &a));
		cr_assert_eq(0, rnd_stack_pushi(s, 10));
		cr_assert_eq(RND_EINVAL, rnd_stack_get(NULL, 0, &a));
		cr_assert_eq(RND_EINVAL, rnd_stack_get(s, 0, NULL));
		cr_assert_eq(RND_EINVAL, rnd_stack_get(NULL, 0, NULL));
		cr_assert_eq(RND_EINDEX, rnd_stack_get(s, 1, &a));
		cr_assert_eq(0, rnd_stack_get(s, 0, &a));
		cr_assert_eq(10, a);
		cr_assert_eq(0, rnd_stack_destroy(s, NULL));
		s = rnd_stack_create(sizeof(struct data), 1000);
		cr_assert_not_null(s);
		for (i = 0; i < 1000; i++) {
			cr_assert_eq(0, data_init(d + i));
			cr_assert_eq(0, rnd_stack_push(s, d + i));
		}
		for (i = 0; i < 1000; i++) {
			struct data a;
			cr_assert_eq(0, rnd_stack_get(s, i, &a));
			cr_assert_eq(0, data_cmp(&a, d + 999 - i));
		}
		cr_assert_eq(0, rnd_stack_destroy(s, data_dtor));
	}

	/* Suffixed form
	 * T  - type
	 * F1 - get function
	 * F2 - push function
	 * V  - random value snippet
	 * M  - printf format string
	 */
#define test(T, F1, F2, V, M) do {                             \
		unsigned i;                                    \
		T a = (V), z = 0;                              \
		T d[1000];                                     \
		s = rnd_stack_create(sizeof(T), 1000);         \
		cr_assert_not_null(s);                         \
		cr_assert_eq(z, F1(s, 0), M);                  \
		cr_assert_eq(0, F2(s, a));                     \
		cr_assert_eq(z, F1(NULL, 0), M);               \
		cr_assert_eq(z, F1(s, 1), M);                  \
		cr_assert_eq(a, F1(s, 0), M);                  \
		cr_assert_eq(0, rnd_stack_destroy(s, NULL));   \
		s = rnd_stack_create(sizeof(T), 1000);         \
		cr_assert_not_null(s);                         \
		for (i = 0; i < 1000; i++) {                   \
			d[i] = (V);                            \
			cr_assert_eq(0, F2(s, d[i]));          \
		}                                              \
		for (i = 0; i < 1000; i++) {                   \
			cr_assert_eq(d[999 - i], F1(s, i), M); \
		}                                              \
		cr_assert_eq(0, rnd_stack_destroy(s, NULL));   \
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
}

Test(stack, set)
{
	struct rnd_stack *s;

	{ /* Generic form */
		unsigned i;
		int a = 1;
		s = rnd_stack_create(sizeof(int), 1000);
		cr_assert_not_null(s);
		cr_assert_eq(RND_EINDEX, rnd_stack_set(s, 0, &a));
		cr_assert_eq(0, rnd_stack_pushi(s, 10));
		cr_assert_eq(RND_EINVAL, rnd_stack_set(NULL, 0, &a));
		cr_assert_eq(RND_EINVAL, rnd_stack_set(s, 0, NULL));
		cr_assert_eq(RND_EINVAL, rnd_stack_set(NULL, 0, NULL));
		cr_assert_eq(RND_EINDEX, rnd_stack_set(s, 1, &a));
		cr_assert_eq(0, rnd_stack_set(s, 0, &a));
		cr_assert_eq(a, rnd_stack_geti(s, 0));
		cr_assert_eq(0, rnd_stack_destroy(s, NULL));
		s = rnd_stack_create(sizeof(struct data), 1000);
		cr_assert_not_null(s);
		for (i = 0; i < 1000; i++) {
			struct data d;
			cr_assert_eq(0, data_init(&d));
			cr_assert_eq(0, rnd_stack_push(s, &d));
		}
		for (i = 0; i < 1000; i++) {
			struct data a, b;
			cr_assert_eq(0, data_init(&b));
			cr_assert_eq(0, rnd_stack_get(s, i, &a));
			cr_assert_eq(0, data_dtor(&a));
			cr_assert_eq(0, rnd_stack_set(s, i, &b));
			cr_assert_eq(0, rnd_stack_get(s, i, &a));
			cr_assert_eq(0, data_cmp(&a, &b));
		}
		cr_assert_eq(0, rnd_stack_destroy(s, data_dtor));
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
		s = rnd_stack_create(sizeof(T), 1000);       \
		cr_assert_not_null(s);                       \
		cr_assert_eq(RND_EINDEX, F1(s, 0, a));       \
		cr_assert_eq(0, F2(s, (V)));                 \
		cr_assert_eq(RND_EINVAL, F1(NULL, 0, a));    \
		cr_assert_eq(RND_EINDEX, F1(s, 1, a));       \
		cr_assert_eq(0, F1(s, 0, a));                \
		cr_assert_eq(a, F3(s, 0), M);                \
		cr_assert_eq(0, rnd_stack_clear(s, NULL));   \
		for (i = 0; i < 1000; i++) {                 \
			cr_assert_eq(0, F2(s, (V)));         \
		}                                            \
		for (i = 0; i < 1000; i++) {                 \
			T b = (V);                           \
			cr_assert_eq(0, F1(s, i, b));        \
			cr_assert_eq(b, F3(s, i), M);        \
		}                                            \
		cr_assert_eq(0, rnd_stack_destroy(s, NULL)); \
		s = rnd_stack_create(sizeof(T) + 1, 1000);   \
		cr_assert_not_null(s);                       \
		s->size = 1;                                 \
		cr_assert_eq(RND_EILLEGAL, F1(s, 0, (V)));   \
		cr_assert_eq(0, rnd_stack_destroy(s, NULL)); \
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
}

Test(stack, print)
{
	struct rnd_stack *s;

	{ /* Generic form */
		double a = 4.5, b = -3.14;
		s = rnd_stack_create(sizeof(double), 30);
		cr_assert_eq(0, rnd_stack_push(s, &a));
		cr_assert_eq(0, rnd_stack_push(s, &b));
		cr_assert_eq(RND_EINVAL, rnd_stack_print(NULL));
		cr_assert_eq(0, rnd_stack_print(s));
		cr_assert_eq(0, rnd_stack_destroy(s, NULL));
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
		s = rnd_stack_create(sizeof(T), 30);         \
		cr_assert_eq(0, F1(s, a));                   \
		cr_assert_eq(0, F1(s, b));                   \
		cr_assert_eq(RND_EINVAL, F2(NULL));          \
		cr_assert_eq(0, F2(s));                      \
		cr_assert_eq(0, rnd_stack_destroy(s, NULL)); \
		s = rnd_stack_create(sizeof(T) + 1, 30);     \
		cr_assert_eq(RND_EILLEGAL, F2(s));           \
		cr_assert_eq(0, rnd_stack_destroy(s, NULL)); \
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
}
