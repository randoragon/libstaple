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
		data_dtor(&d);
		for (i = 0; i < 1000; i++) {
			struct data d;
			ASSERT_EQ_FMT(0, data_init(&d), "%d");
			ASSERT_EQ_FMT(0, rnd_queue_push(q, &d), "%d");
		}
		ASSERT_EQ_FMT(0, rnd_queue_destroy(q, data_dtor), "%d");
	}

	/* Suffixed form
	 * T - type
	 * F - function
	 * V - random value snippet
	 * M - printf format string
	 */
#define test(T, F, V, M) do {                                       \
		unsigned i;                                         \
		q = rnd_queue_create(sizeof(T), 1000);              \
		ASSERT_NEQ(NULL, q);                                \
		ASSERT_EQ_FMT(RND_EINVAL, F(NULL, (V)), "%d");      \
		for (i = 0; i < SIZE_MAX / sizeof(T); i++) {        \
			ASSERT_EQ_FMT(0, F(q, (V)), "%d");          \
		}                                                   \
		ASSERT_EQ_FMT(RND_ERANGE, F(q, (V)), "%d");         \
		ASSERT_EQ_FMT(0, rnd_queue_destroy(q, NULL), "%d"); \
		q = rnd_queue_create(sizeof(T) + 1, 1);             \
		ASSERT_NEQ(NULL, q);                                \
		ASSERT_EQ_FMT(RND_EILLEGAL, F(q, (V)), "%d");       \
		ASSERT_EQ_FMT(0, rnd_queue_destroy(q, NULL), "%d"); \
	} while (0)
	test(char          , rnd_queue_pushc , IRANGE(CHAR_MIN , CHAR_MAX) , "%hd");
	test(short         , rnd_queue_pushs , IRANGE(SHRT_MIN , SHRT_MAX) , "%hd");
	test(int           , rnd_queue_pushi , FRANGE(INT_MIN  , INT_MAX)  , "%d");
	test(long          , rnd_queue_pushl , FRANGE(LONG_MIN , LONG_MAX) , "%ld");
	test(signed char   , rnd_queue_pushsc, IRANGE(SCHAR_MIN, SCHAR_MAX), "%hd");
	test(unsigned char , rnd_queue_pushuc, IRANGE(0        , UCHAR_MAX), "%hd");
	test(unsigned short, rnd_queue_pushus, IRANGE(0        , USHRT_MAX), "%hu");
	test(unsigned int  , rnd_queue_pushui, FRANGE(0        , UINT_MAX) , "%u");
	test(unsigned long , rnd_queue_pushul, FRANGE(0        , ULONG_MAX), "%lu");
	test(float         , rnd_queue_pushf , FRANGE(FLT_MIN  , FLT_MAX)  , "%f");
	test(double        , rnd_queue_pushd , FRANGE(DBL_MIN  , DBL_MAX)  , "%f");
	test(long double   , rnd_queue_pushld, FRANGE(LDBL_MIN , LDBL_MAX) , "%Lf");
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
		ASSERT_EQ(a.age, b.age);
		ASSERT_EQ(a.id, b.id);
		ASSERT_EQ(0, strcmp(a.name, b.name));
		ASSERT_EQ(0, strcmp(a.surname, b.surname));
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
		ASSERT_EQ_FMT(0, rnd_queue_pop(q, &b), "%d");
		ASSERT_EQ(a.age, b.age);
		ASSERT_EQ(a.id, b.id);
		ASSERT_EQ(0, strcmp(a.name, b.name));
		ASSERT_EQ(0, strcmp(a.surname, b.surname));
		ASSERT_EQ_FMT(0, rnd_queue_destroy(q, NULL), "%d");
		data_dtor(&b);
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
		ASSERT_EQ_FMT(a, F1(q), M);                         \
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
	struct rnd_queue *q;
	PASS();
}

TEST t_insert(void)
{
	struct rnd_queue *q;
	PASS();
}

TEST t_quickinsert(void)
{
	struct rnd_queue *q;
	PASS();
}

TEST t_remove(void)
{
	struct rnd_queue *q;
	PASS();
}

TEST t_quickremove(void)
{
	struct rnd_queue *q;
	PASS();
}

TEST t_get(void)
{
	struct rnd_queue *q;
	PASS();
}

TEST t_set(void)
{
	struct rnd_queue *q;
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
