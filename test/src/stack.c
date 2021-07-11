#include <stdlib.h>
#include <time.h>
#include "../../src/rnd.h"
#include "../../src/errcodes.h"
#include <greatest.h>
#include <limits.h>
#include <float.h>

#define IRANGE(X,Y) ((X) + (rand() % ((Y) - (X) + 1)))
#define FRANGE(X,Y) ((X) + ((double)rand() / RAND_MAX * ((Y) - (X))))
#define SIZE_MAX 65535LU

GREATEST_MAIN_DEFS();

TEST create_destroy(void)
{
	struct rnd_stack *s;
	s = rnd_stack_create(sizeof(int), 10);
	ASSERT_NEQ(NULL, s);
	ASSERT_EQ_FMT(0, rnd_stack_destroy(s, NULL), "%d");
	PASS();
}

TEST push_peek_pop(void)
{
	struct rnd_stack *s;

	/* (void*) */
	{
		int a = IRANGE(SHRT_MIN, SHRT_MAX),
		    b = IRANGE(SHRT_MIN, SHRT_MAX),
		    c = IRANGE(SHRT_MIN, SHRT_MAX);
		int out;
		s = rnd_stack_create(sizeof(int), 3);
		ASSERT_EQ_FMT(0, rnd_stack_push(s, &a), "%d");
		ASSERT_EQ_FMT(0, rnd_stack_peek(s, &out), "%d");
		ASSERT_EQ_FMT(a, out, "%d");
		ASSERT_EQ_FMT(0, rnd_stack_push(s, &b), "%d");;
		ASSERT_EQ_FMT(0, rnd_stack_peek(s, &out), "%d");
		ASSERT_EQ_FMT(b, out, "%d");
		ASSERT_EQ_FMT(0, rnd_stack_push(s, &c), "%d");;
		ASSERT_EQ_FMT(0, rnd_stack_peek(s, &out), "%d");
		ASSERT_EQ_FMT(c, out, "%d");
		ASSERT_EQ_FMT(3lu, s->capacity, "%lu");
		ASSERT_EQ_FMT(0, rnd_stack_destroy(s, NULL), "%d");;
	}

	/* With this macro we can easily run an equivalent to the above block of
	 * code for every primitive type */
#define test(T, F1, F2, F, A, B, C)                                 \
	do {                                                        \
		T a = A;                                            \
		T b = B;                                            \
		T c = C;                                            \
		s = rnd_stack_create(sizeof(T), 3);                 \
		ASSERT_EQ_FMT(0, F1(s, a), "%d");                   \
		ASSERT_EQ_FMT(a, F2(s), F);                         \
		ASSERT_EQ_FMT(0, F1(s, b), "%d");                   \
		ASSERT_EQ_FMT(b, F2(s), F);                         \
		ASSERT_EQ_FMT(0, F1(s, c), "%d");                   \
		ASSERT_EQ_FMT(c, F2(s), F);                         \
		ASSERT_EQ_FMT(3lu, s->capacity, "%lu");             \
		ASSERT_EQ_FMT(0, rnd_stack_destroy(s, NULL), "%d"); \
	} while (0)

	test(char, rnd_stack_pushc, rnd_stack_peekc, "%d",
			IRANGE(CHAR_MIN, CHAR_MAX),
			IRANGE(CHAR_MIN, CHAR_MAX),
			IRANGE(CHAR_MIN, CHAR_MAX));
	test(signed char, rnd_stack_pushsc, rnd_stack_peeksc, "%d",
			IRANGE(SCHAR_MIN, SCHAR_MAX),
			IRANGE(SCHAR_MIN, SCHAR_MAX),
			IRANGE(SCHAR_MIN, SCHAR_MAX));
	test(unsigned char, rnd_stack_pushuc, rnd_stack_peekuc, "%d",
			IRANGE(0, UCHAR_MAX),
			IRANGE(0, UCHAR_MAX),
			IRANGE(0, UCHAR_MAX));
	test(short, rnd_stack_pushs, rnd_stack_peeks, "%d",
			IRANGE(SHRT_MIN, SHRT_MAX),
			IRANGE(SHRT_MIN, SHRT_MAX),
			IRANGE(SHRT_MIN, SHRT_MAX));
	test(unsigned short, rnd_stack_pushus, rnd_stack_peekus, "%u",
			IRANGE(0, USHRT_MAX),
			IRANGE(0, USHRT_MAX),
			IRANGE(0, USHRT_MAX));
	test(int, rnd_stack_pushi, rnd_stack_peeki, "%d",
			IRANGE(SHRT_MIN, SHRT_MAX),
			IRANGE(SHRT_MIN, SHRT_MAX),
			IRANGE(SHRT_MIN, SHRT_MAX));
	test(unsigned int, rnd_stack_pushui, rnd_stack_peekui, "%u",
			IRANGE(0, USHRT_MAX),
			IRANGE(0, USHRT_MAX),
			IRANGE(0, USHRT_MAX));
	test(long, rnd_stack_pushl, rnd_stack_peekl, "%ld",
			IRANGE(SHRT_MIN + 1, SHRT_MAX),
			IRANGE(SHRT_MIN + 1, SHRT_MAX),
			IRANGE(SHRT_MIN + 1, SHRT_MAX));
	test(unsigned long, rnd_stack_pushul, rnd_stack_peekul, "%lu",
			IRANGE(0, SHRT_MAX),
			IRANGE(0, SHRT_MAX),
			IRANGE(0, SHRT_MAX));
	test(float, rnd_stack_pushf, rnd_stack_peekf, "%f",
			FRANGE(FLT_MIN, FLT_MAX),
			FRANGE(FLT_MIN, FLT_MAX),
			FRANGE(FLT_MIN, FLT_MAX));
	test(double, rnd_stack_pushd, rnd_stack_peekd, "%f",
			FRANGE(DBL_MIN, DBL_MAX),
			FRANGE(DBL_MIN, DBL_MAX),
			FRANGE(DBL_MIN, DBL_MAX));
	test(long double, rnd_stack_pushld, rnd_stack_peekld, "%Lf",
			FRANGE(LDBL_MIN, LDBL_MAX),
			FRANGE(LDBL_MIN, LDBL_MAX),
			FRANGE(LDBL_MIN, LDBL_MAX));
#undef test
	PASS();
}

TEST push_realloc(void)
{
	struct rnd_stack *s;
	s = rnd_stack_create(sizeof(int), 4);
	ASSERT_EQ_FMT(4lu, s->capacity, "%lu");
	ASSERT_EQ_FMT(0, rnd_stack_pushi(s, 2), "%d");
	ASSERT_EQ_FMT(4lu, s->capacity, "%lu");
	ASSERT_EQ_FMT(0, rnd_stack_pushi(s, 1), "%d");
	ASSERT_EQ_FMT(4lu, s->capacity, "%lu");
	ASSERT_EQ_FMT(0, rnd_stack_pushi(s, 3), "%d");
	ASSERT_EQ_FMT(4lu, s->capacity, "%lu");
	ASSERT_EQ_FMT(0, rnd_stack_pushi(s, 7), "%d");
	ASSERT_EQ_FMT(4lu, s->capacity, "%lu");
	ASSERT_EQ_FMT(0, rnd_stack_pushi(s, 0), "%d");
	ASSERT_EQ_FMT(8lu, s->capacity, "%lu");
	ASSERT_EQ_FMT(0, rnd_stack_destroy(s, NULL), "%d");
	PASS();
}

TEST insert_remove(void)
{
	struct rnd_stack *s;

	{
		int a = 5, b = -14, c = -3547, out;
		s = rnd_stack_create(sizeof(int), 1);
		ASSERT_EQ_FMT(RND_EINDEX, rnd_stack_insert(s, 1, &a), "%d");
		ASSERT_EQ_FMT(0, rnd_stack_insert(s, 0, &a), "%d");
		ASSERT_EQ_FMT(0, rnd_stack_peek(s, &out), "%d");
		ASSERT_EQ_FMT(a, out, "%d");
		ASSERT_EQ_FMT(1lu, s->size, "%lu");
		ASSERT_EQ_FMT(0, rnd_stack_remove(s, 0, &out), "%d");
		ASSERT_EQ_FMT(a, out, "%d");
		ASSERT_EQ_FMT(0lu, s->size, "%lu");
		ASSERT_EQ_FMT(0, rnd_stack_insert(s, 0, &a), "%d");
		ASSERT_EQ_FMT(0, rnd_stack_peek(s, &out), "%d");
		ASSERT_EQ_FMT(a, out, "%d");
		ASSERT_EQ_FMT(0, rnd_stack_insert(s, 1, &b), "%d");
		ASSERT_EQ_FMT(0, rnd_stack_peek(s, &out), "%d");
		ASSERT_EQ_FMT(a, out, "%d");
		ASSERT_EQ_FMT(0, rnd_stack_insert(s, 2, &c), "%d");
		ASSERT_EQ_FMT(0, rnd_stack_peek(s, &out), "%d");
		ASSERT_EQ_FMT(a, out, "%d");
		ASSERT_EQ_FMT(0, rnd_stack_remove(s, 0, &out), "%d");
		ASSERT_EQ_FMT(a, out, "%d");
		ASSERT_EQ_FMT(0, rnd_stack_peek(s, &out), "%d");
		ASSERT_EQ_FMT(b, out, "%d");
		ASSERT_EQ_FMT(0, rnd_stack_remove(s, 0, &out), "%d");
		ASSERT_EQ_FMT(b, out, "%d");
		ASSERT_EQ_FMT(0, rnd_stack_peek(s, &out), "%d");
		ASSERT_EQ_FMT(c, out, "%d");
		ASSERT_EQ_FMT(1lu, s->size, "%lu");
		ASSERT_EQ_FMT(4lu, s->capacity, "%lu");
		ASSERT_EQ_FMT(0, rnd_stack_destroy(s, NULL), "%d");
	}

	/* A > B > C */
	/* F1 - insert, F2 - peek, F3 - remove */
#define test(T, F1, F2, F3, F, A, B, C)                             \
	do {                                                        \
		s = rnd_stack_create(sizeof(T), 1);                 \
		ASSERT_EQ_FMT(RND_EINDEX, F1(s, 1, A), "%d");       \
		ASSERT_EQ_FMT(0, F1(s, 0, A), "%d");                \
		ASSERT_EQ_FMT(A, F2(s), F);                         \
		ASSERT_EQ_FMT(1lu, s->size, "%lu");                 \
		ASSERT_EQ_FMT(A, F3(s, 0), F);                      \
		ASSERT_EQ_FMT(0lu, s->size, "%lu");                 \
		ASSERT_EQ_FMT(0, F1(s, 0, A), "%d");                \
		ASSERT_EQ_FMT(A, F2(s), F);                         \
		ASSERT_EQ_FMT(0, F1(s, 1, B), "%d");                \
		ASSERT_EQ_FMT(A, F2(s), F);                         \
		ASSERT_EQ_FMT(0, F1(s, 2, C), "%d");                \
		ASSERT_EQ_FMT(A, F2(s), F);                         \
		ASSERT_EQ_FMT(B, F3(s, 1), F);                      \
		ASSERT_EQ_FMT(A, F2(s), F);                         \
		ASSERT_EQ_FMT(A, F3(s, 0), F);                      \
		ASSERT_EQ_FMT(C, F2(s), F);                         \
		ASSERT_EQ_FMT(1lu, s->size, "%lu");                 \
		ASSERT_EQ_FMT(4lu, s->capacity, "%lu");             \
		ASSERT_EQ_FMT(0, rnd_stack_destroy(s, NULL), "%d"); \
	} while (0)

	test(char          , rnd_stack_insertc , rnd_stack_peekc , rnd_stack_removec , "%d", 9, 4, 1);
	test(short         , rnd_stack_inserts , rnd_stack_peeks , rnd_stack_removes , "%d", 13, 12, -555);
	test(int           , rnd_stack_inserti , rnd_stack_peeki , rnd_stack_removei , "%d", INT_MAX, 0, INT_MIN);
	test(long          , rnd_stack_insertl , rnd_stack_peekl , rnd_stack_removel , "%ld", 555l, 55l, 5l);
	test(signed char   , rnd_stack_insertsc, rnd_stack_peeksc, rnd_stack_removesc, "%d", 2, 1, 0);
	test(unsigned char , rnd_stack_insertuc, rnd_stack_peekuc, rnd_stack_removeuc, "%d", 10, 5, 3);
	test(unsigned short, rnd_stack_insertus, rnd_stack_peekus, rnd_stack_removeus, "%d", 787, 32, 1);
	test(unsigned int  , rnd_stack_insertui, rnd_stack_peekui, rnd_stack_removeui, "%u", 10000, 200, 3);
	test(unsigned long , rnd_stack_insertul, rnd_stack_peekul, rnd_stack_removeul, "%lu", 3ul, 2ul, 1ul);
	test(float         , rnd_stack_insertf , rnd_stack_peekf , rnd_stack_removef , "%f", 99., 98., 97.);
	test(double        , rnd_stack_insertd , rnd_stack_peekd , rnd_stack_removed , "%f", 43., 21., -7421.);
	test(long double   , rnd_stack_insertld, rnd_stack_peekld, rnd_stack_removeld, "%Lf", 3.5L, 2.5L, -9.3219L);

	/* A > B > C */
	/* F1 - insert, F2 - peek, F3 - remove */
#undef test
#define test(T, F1, F2, F3, F, A, B, C)                             \
	do {                                                        \
		s = rnd_stack_create(sizeof(T), 1);                 \
		ASSERT_EQ_FMT(RND_EINDEX, F1(s, 1, A), "%d");       \
		ASSERT_EQ_FMT(0, F1(s, 0, A), "%d");                \
		ASSERT_EQ_FMT(A, F2(s), F);                         \
		ASSERT_EQ_FMT(1lu, s->size, "%lu");                 \
		ASSERT_EQ_FMT(A, F3(s, 0), F);                      \
		ASSERT_EQ_FMT(0lu, s->size, "%lu");                 \
		ASSERT_EQ_FMT(0, F1(s, 0, A), "%d");                \
		ASSERT_EQ_FMT(A, F2(s), F);                         \
		ASSERT_EQ_FMT(0, F1(s, 1, B), "%d");                \
		ASSERT_EQ_FMT(A, F2(s), F);                         \
		ASSERT_EQ_FMT(0, F1(s, 2, C), "%d");                \
		ASSERT_EQ_FMT(B, F2(s), F);                         \
		ASSERT_EQ_FMT(A, F3(s, 1), F);                      \
		ASSERT_EQ_FMT(B, F2(s), F);                         \
		ASSERT_EQ_FMT(B, F3(s, 0), F);                      \
		ASSERT_EQ_FMT(C, F2(s), F);                         \
		ASSERT_EQ_FMT(1lu, s->size, "%lu");                 \
		ASSERT_EQ_FMT(4lu, s->capacity, "%lu");             \
		ASSERT_EQ_FMT(0, rnd_stack_destroy(s, NULL), "%d"); \
	} while (0)

	test(char          , rnd_stack_quickinsertc , rnd_stack_peekc , rnd_stack_quickremovec , "%d", 9, 4, 1);
	test(short         , rnd_stack_quickinserts , rnd_stack_peeks , rnd_stack_quickremoves , "%d", 13, 12, -555);
	test(int           , rnd_stack_quickinserti , rnd_stack_peeki , rnd_stack_quickremovei , "%d", INT_MAX, 0, INT_MIN);
	test(long          , rnd_stack_quickinsertl , rnd_stack_peekl , rnd_stack_quickremovel , "%ld", 555l, 55l, 5l);
	test(signed char   , rnd_stack_quickinsertsc, rnd_stack_peeksc, rnd_stack_quickremovesc, "%d", 2, 1, 0);
	test(unsigned char , rnd_stack_quickinsertuc, rnd_stack_peekuc, rnd_stack_quickremoveuc, "%d", 10, 5, 3);
	test(unsigned short, rnd_stack_quickinsertus, rnd_stack_peekus, rnd_stack_quickremoveus, "%d", 787, 32, 1);
	test(unsigned int  , rnd_stack_quickinsertui, rnd_stack_peekui, rnd_stack_quickremoveui, "%u", 10000, 200, 3);
	test(unsigned long , rnd_stack_quickinsertul, rnd_stack_peekul, rnd_stack_quickremoveul, "%lu", 3ul, 2ul, 1ul);
	test(float         , rnd_stack_quickinsertf , rnd_stack_peekf , rnd_stack_quickremovef , "%f", 99., 98., 97.);
	test(double        , rnd_stack_quickinsertd , rnd_stack_peekd , rnd_stack_quickremoved , "%f", 43., 21., -7421.);
	test(long double   , rnd_stack_quickinsertld, rnd_stack_peekld, rnd_stack_quickremoveld, "%Lf", 3.5L, 2.5L, -9.3219L);

#undef test
	PASS();
}

TEST get_set(void)
{
	struct rnd_stack *s;

	{
		double a = 4.5, b = -3.14, out;
		s = rnd_stack_create(sizeof(double), 30);
		ASSERT_EQ_FMT(0, rnd_stack_push(s, &a), "%d");
		ASSERT_EQ_FMT(0, rnd_stack_get(s, 0, &out), "%d");
		ASSERT_EQ_FMT(a, out, "%f");
		ASSERT_EQ_FMT(0, rnd_stack_set(s, 0, &b), "%d");
		ASSERT_EQ_FMT(0, rnd_stack_get(s, 0, &out), "%d");
		ASSERT_EQ_FMT(b, out, "%f");
		ASSERT_EQ_FMT(0, rnd_stack_pop(s, &out), "%d");
		ASSERT_EQ_FMT(b, out, "%f");
		ASSERT_EQ_FMT(RND_EILLEGAL, rnd_stack_get(s, 0, &out), "%d");
		ASSERT_EQ_FMT(0, rnd_stack_destroy(s, NULL), "%d");
	}

	/* F1 - push, F2 - pop, F3 - get, F4 - set */
#define test(T, F1, F2, F3, F4, F, A, B)                            \
	do {                                                        \
		T a = A, b = B;                                     \
		s = rnd_stack_create(sizeof(T), 30);                \
		ASSERT_EQ_FMT(0, F1(s, a), "%d");                   \
		ASSERT_EQ_FMT(a, F3(s, 0), F);                      \
		ASSERT_EQ_FMT(0, F4(s, 0, b), "%d");                \
		ASSERT_EQ_FMT(b, F3(s, 0), F);                      \
		ASSERT_EQ_FMT(b, F2(s), F);                         \
		ASSERT_EQ_FMT((T)0, F3(s, 0), F);                   \
		ASSERT_EQ_FMT(0, rnd_stack_destroy(s, NULL), "%d"); \
	} while(0)

	test(char          , rnd_stack_pushc , rnd_stack_popc , rnd_stack_getc , rnd_stack_setc , "%d", 9, 4);
	test(short         , rnd_stack_pushs , rnd_stack_pops , rnd_stack_gets , rnd_stack_sets , "%d", 13, 12);
	test(int           , rnd_stack_pushi , rnd_stack_popi , rnd_stack_geti , rnd_stack_seti , "%d", INT_MAX, 0);
	test(long          , rnd_stack_pushl , rnd_stack_popl , rnd_stack_getl , rnd_stack_setl , "%ld", 555l, 55l);
	test(signed char   , rnd_stack_pushsc, rnd_stack_popsc, rnd_stack_getsc, rnd_stack_setsc, "%d", 2, 1);
	test(unsigned char , rnd_stack_pushuc, rnd_stack_popuc, rnd_stack_getuc, rnd_stack_setuc, "%d", 10, 5);
	test(unsigned short, rnd_stack_pushus, rnd_stack_popus, rnd_stack_getus, rnd_stack_setus, "%d", 787, 32);
	test(unsigned int  , rnd_stack_pushui, rnd_stack_popui, rnd_stack_getui, rnd_stack_setui, "%u", 10000, 200);
	test(unsigned long , rnd_stack_pushul, rnd_stack_popul, rnd_stack_getul, rnd_stack_setul, "%lu", 3ul, 2ul);
	test(float         , rnd_stack_pushf , rnd_stack_popf , rnd_stack_getf , rnd_stack_setf , "%f", 99., 98.);
	test(double        , rnd_stack_pushd , rnd_stack_popd , rnd_stack_getd , rnd_stack_setd , "%f", 43., 21.);
	test(long double   , rnd_stack_pushld, rnd_stack_popld, rnd_stack_getld, rnd_stack_setld, "%Lf", 3.5L, 2.5L);

#undef test
	PASS();
}

TEST copy_clear(void)
{
	struct rnd_stack *s1, *s2;
	unsigned int i;

	s1 = rnd_stack_create(sizeof(int), 10);
	ASSERT_NEQ(NULL, (s2 = malloc(sizeof(*s2))));
	ASSERT_EQ_FMT(0, rnd_stack_clear(s1, NULL), "%d");
	for (i = 0; i < 5000; i++)
		ASSERT_EQ_FMT(0, rnd_stack_pushi(s1, IRANGE(0, USHRT_MAX)), "%d");
	ASSERT_EQ_FMT(0, rnd_stack_copy(s2, s1, NULL), "%d");
	for (i = 0; i < 5000; i++) {
		ASSERT_EQ(rnd_stack_geti(s1, i), rnd_stack_geti(s2, i));
	}
	ASSERT_EQ_FMT(0, rnd_stack_seti(s1, 0, -1), "%d");
	ASSERT_NEQ(rnd_stack_geti(s1, 0), rnd_stack_geti(s2, 0));
	ASSERT_EQ_FMT(0, rnd_stack_clear(s2, NULL), "%d");
	ASSERT_EQ_FMT(0, rnd_stack_destroy(s1, NULL), "%d");
	ASSERT_EQ_FMT(0, rnd_stack_destroy(s2, NULL), "%d");
	PASS();
}

TEST size_overflow(void)
{
	struct rnd_stack *s;
	size_t i;
	s = rnd_stack_create(sizeof(char), 4096);
	for (i = SIZE_MAX; i-- != 0;)
		ASSERT_EQ_FMT(0, rnd_stack_pushuc(s, i % 256), "%d");
	ASSERT_EQ_FMT(RND_ERANGE, rnd_stack_pushuc(s, 0), "%d");
	ASSERT_EQ_FMT(RND_ERANGE, rnd_stack_pushuc(s, 0), "%d");
	ASSERT_EQ_FMT(RND_ERANGE, rnd_stack_pushuc(s, 0), "%d");
	ASSERT_EQ_FMT(0, rnd_stack_destroy(s, NULL), "%d");
	PASS();
}

SUITE(stack) {
	RUN_TEST(create_destroy);
	RUN_TEST(push_peek_pop);
	RUN_TEST(push_realloc);
	RUN_TEST(insert_remove);
	RUN_TEST(get_set);
	RUN_TEST(copy_clear);
	RUN_TEST(size_overflow);
}

int main(int argc, char **argv)
{
	GREATEST_MAIN_BEGIN();

	srand(time(NULL));
	RUN_SUITE(stack);

	GREATEST_MAIN_END();
}
