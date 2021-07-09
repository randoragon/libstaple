#include <stdlib.h>
#include <time.h>
#include "../../src/rnd.h"
#include <greatest.h>
#include <limits.h>
#include <float.h>

#define IRANGE(X,Y) ((X) + (rand() % ((Y) - (X) + 1)))
#define FRANGE(X,Y) ((X) + ((double)rand() / RAND_MAX * ((Y) - (X))))

GREATEST_MAIN_DEFS();

TEST create_destroy(void)
{
	struct rnd_stack *s;
	s = rnd_stack_create(sizeof(int), 10);
	ASSERT_NEQ(NULL, s);
	rnd_stack_destroy(s, NULL);
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
		s = rnd_stack_create(sizeof(int), 3);
		rnd_stack_pushp(s, &a);
		ASSERT_EQ_FMT(a, *(int*)rnd_stack_peekp(s), "%d");
		rnd_stack_pushp(s, &b);
		ASSERT_EQ_FMT(b, *(int*)rnd_stack_peekp(s), "%d");
		rnd_stack_pushp(s, &c);
		ASSERT_EQ_FMT(c, *(int*)rnd_stack_peekp(s), "%d");
		ASSERT_EQ_FMT(3lu, s->capacity, "%lu");
		rnd_stack_destroy(s, NULL);
	}

	/* With this macro we can easily run an equivalent to the above block of
	 * code for every primitive type */
#define test(T, F1, F2, F, A, B, C)                     \
	do {                                            \
		T a = A;                                \
		T b = B;                                \
		T c = C;                                \
		s = rnd_stack_create(sizeof(T), 3);     \
		F1(s, a);                               \
		ASSERT_EQ_FMT(a, F2(s), F);             \
		F1(s, b);                               \
		ASSERT_EQ_FMT(b, F2(s), F);             \
		F1(s, c);                               \
		ASSERT_EQ_FMT(c, F2(s), F);             \
		ASSERT_EQ_FMT(3lu, s->capacity, "%lu"); \
		rnd_stack_destroy(s, NULL);             \
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
	rnd_stack_pushi(s, 2);
	ASSERT_EQ_FMT(4lu, s->capacity, "%lu");
	rnd_stack_pushi(s, 1);
	ASSERT_EQ_FMT(4lu, s->capacity, "%lu");
	rnd_stack_pushi(s, 3);
	ASSERT_EQ_FMT(4lu, s->capacity, "%lu");
	rnd_stack_pushi(s, 7);
	ASSERT_EQ_FMT(4lu, s->capacity, "%lu");
	rnd_stack_pushi(s, 0);
	ASSERT_EQ_FMT(8lu, s->capacity, "%lu");
	rnd_stack_destroy(s, NULL);
	PASS();
}

SUITE(stack) {
	RUN_TEST(create_destroy);
	RUN_TEST(push_peek_pop);
	RUN_TEST(push_realloc);
}

int main(int argc, char **argv)
{
	GREATEST_MAIN_BEGIN();

	srand(time(NULL));
	RUN_SUITE(stack);

	GREATEST_MAIN_END();
}
