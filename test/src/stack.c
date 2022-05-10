/*  Staple - A general-purpose data structure library in pure C89.
 *  Copyright (C) 2021  Randoragon
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation;
 *  version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include <stdlib.h>
#include <time.h>
#include "../../src/sp_stack.h"
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

void setup(void)
{
	if (!sp_is_debug())
		cr_log_error("libstaple is not running in debug mode");
}

TestSuite(stack, .init=setup);

Test(stack, create)
{
	struct sp_stack *s;
	s = sp_stack_create(sizeof(int), 0);
	cr_assert_null(s);
	s = sp_stack_create(0, 16);
	cr_assert_null(s);
	s = sp_stack_create(0, 0);
	cr_assert_null(s);
	s = sp_stack_create(SIZE_MAX, SIZE_MAX);
	cr_assert_null(s);
	s = sp_stack_create(sizeof(int), 16);
	cr_assert_not_null(s);
	cr_assert_eq(0, sp_stack_destroy(s, NULL));
}

Test(stack, destroy)
{
	struct sp_stack *s;
	unsigned i;
	s = sp_stack_create(sizeof(long double), 1000);
	cr_assert_not_null(s);
	cr_assert_eq(SP_EINVAL, sp_stack_destroy(NULL, NULL));
	cr_assert_eq(0, sp_stack_destroy(s, NULL));
	s = sp_stack_create(sizeof(struct data), 1000);
	for (i = 0; i < 1000; i++) {
		struct data d;
		cr_assert_eq(0, data_init(&d));
		cr_assert_eq(0, sp_stack_push(s, &d));
	}
	cr_assert_eq(SP_EHANDLER, sp_stack_destroy(s, data_dtor_bad));
	cr_assert_eq(0, sp_stack_destroy(s, data_dtor));
}

Test(stack, push)
{
	struct sp_stack *s;

	{ /* Generic form */
		unsigned i;
		struct data d;
		s = sp_stack_create(sizeof(struct data), 1000);
		cr_assert_not_null(s);
		data_init(&d);
		cr_assert_eq(SP_EINVAL, sp_stack_push(s, NULL));
		cr_assert_eq(SP_EINVAL, sp_stack_push(NULL, &d));
		cr_assert_eq(SP_EINVAL, sp_stack_push(NULL, NULL));
		cr_assert_eq(0LU, (unsigned long)s->size);
		cr_assert_eq(0, sp_stack_push(s, &d));
		cr_assert_eq(1LU, (unsigned long)s->size);
		cr_assert_eq(0, sp_stack_push(s, &d));
		cr_assert_eq(2LU, (unsigned long)s->size);
		cr_assert_eq(0, sp_stack_clear(s, NULL));
		for (i = 0; i < SIZE_MAX / sizeof(struct data); i++) {
			struct data a, b;
			cr_assert_eq(0, data_init(&a));
			cr_assert_eq(0, sp_stack_push(s, &a));
			b = *(struct data*)sp_stack_get(s, 0);
			cr_assert_eq(0, data_cmp(&a, &b));
		}
		cr_assert_eq(SP_ERANGE, sp_stack_push(s, &d));
		cr_assert_eq(0, data_dtor(&d));
		cr_assert_eq(0, sp_stack_destroy(s, data_dtor));
	}

	/* Suffixed form
	 * T  - type
	 * F1 - push function
	 * F2 - get function
	 * V  - random value snippet
	 * M  - printf format string
	 */
#define test(T, F1, F2, V, M) do {                           \
		unsigned i;                                  \
		s = sp_stack_create(sizeof(T), 1000);        \
		cr_assert_not_null(s);                       \
		cr_assert_eq(SP_EINVAL, F1(NULL, (V)));      \
		cr_assert_eq(0LU, (unsigned long)s->size);   \
		cr_assert_eq(0, F1(s, (V)));                 \
		cr_assert_eq(1LU, (unsigned long)s->size);   \
		cr_assert_eq(0, F1(s, (V)));                 \
		cr_assert_eq(2LU, (unsigned long)s->size);   \
		cr_assert_eq(0, sp_stack_clear(s, NULL));    \
		for (i = 0; i < SIZE_MAX / sizeof(T); i++) { \
			T a = (V);                           \
			cr_assert_eq(0, F1(s, a));           \
			cr_assert_eq(a, F2(s, 0));           \
		}                                            \
		cr_assert_eq(SP_ERANGE, F1(s, (V)));         \
		cr_assert_eq(0, sp_stack_destroy(s, NULL));  \
		s = sp_stack_create(sizeof(T) + 1, 1);       \
		cr_assert_not_null(s);                       \
		cr_assert_eq(SP_EILLEGAL, F1(s, (V)));       \
		cr_assert_eq(0, sp_stack_destroy(s, NULL));  \
	} while (0)
	test(char          , sp_stack_pushc , sp_stack_getc , IRANGE(CHAR_MIN , CHAR_MAX) , "%hd");
	test(short         , sp_stack_pushs , sp_stack_gets , IRANGE(SHRT_MIN , SHRT_MAX) , "%hd");
	test(int           , sp_stack_pushi , sp_stack_geti , FRANGE(INT_MIN  , INT_MAX)  , "%d");
	test(long          , sp_stack_pushl , sp_stack_getl , FRANGE(LONG_MIN , LONG_MAX) , "%ld");
	test(signed char   , sp_stack_pushsc, sp_stack_getsc, IRANGE(SCHAR_MIN, SCHAR_MAX), "%hd");
	test(unsigned char , sp_stack_pushuc, sp_stack_getuc, IRANGE(0        , UCHAR_MAX), "%hd");
	test(unsigned short, sp_stack_pushus, sp_stack_getus, IRANGE(0        , USHRT_MAX), "%hu");
	test(unsigned int  , sp_stack_pushui, sp_stack_getui, FRANGE(0        , UINT_MAX) , "%u");
	test(unsigned long , sp_stack_pushul, sp_stack_getul, FRANGE(0        , ULONG_MAX), "%lu");
	test(float         , sp_stack_pushf , sp_stack_getf , FRANGE(FLT_MIN  , FLT_MAX)  , "%f");
	test(double        , sp_stack_pushd , sp_stack_getd , FRANGE(DBL_MIN  , DBL_MAX)  , "%f");
	test(long double   , sp_stack_pushld, sp_stack_getld, FRANGE(LDBL_MIN , LDBL_MAX) , "%Lf");
#undef test

	{ /* Suffixed form - strings */
		unsigned i;
		s = sp_stack_create(sizeof(char*), 1000);
		cr_assert_not_null(s);
		cr_assert_eq(SP_EINVAL, sp_stack_pushstr(s, NULL));
		cr_assert_eq(SP_EINVAL, sp_stack_pushstr(NULL, "test"));
		cr_assert_eq(0LU, (unsigned long)s->size);
		cr_assert_eq(0, sp_stack_pushstr(s, "test"));
		cr_assert_eq(1LU, (unsigned long)s->size);
		cr_assert_eq(0, sp_stack_pushstr(s, "another test"));
		cr_assert_eq(2LU, (unsigned long)s->size);
		cr_assert_eq(0, sp_stack_clear(s, sp_free));
		for (i = 0; i < SIZE_MAX / sizeof(char*); i++) {
			cr_assert_eq(0, sp_stack_pushstr(s, "yet another test"));
			cr_assert_eq(0, strcmp("yet another test", sp_stack_getstr(s, 0)));
		}
		cr_assert_eq(SP_ERANGE, sp_stack_pushstr(s, "size_t overflow"));
		cr_assert_eq(0, sp_stack_clear(s, sp_free));
		char too_long[SIZE_MAX];
		for (i = 0; i < SIZE_MAX; i++)
			too_long[i] = 'a';
		too_long[SIZE_MAX] = '\0';
		cr_assert_eq(SP_ERANGE, sp_stack_pushstr(s, too_long));
		cr_assert_eq(0, sp_stack_destroy(s, sp_free));
		s = sp_stack_create(sizeof(char*) + 1, 1);
		cr_assert_not_null(s);
		cr_assert_eq(SP_EILLEGAL, sp_stack_pushstr(s, "incompatible elem_size"));
		cr_assert_eq(0, sp_stack_destroy(s, sp_free));

		/* substrings (strn) */
		s = sp_stack_create(sizeof(char*), 1000);
		cr_assert_not_null(s);
		cr_assert_eq(SP_EINVAL, sp_stack_pushstrn(s, NULL, 0));
		cr_assert_eq(SP_EINVAL, sp_stack_pushstrn(NULL, "test", 2));
		cr_assert_eq(0LU, (unsigned long)s->size);
		cr_assert_eq(0, sp_stack_pushstrn(s, "test", 2));
		cr_assert_eq(1LU, (unsigned long)s->size);
		cr_assert_eq(0, sp_stack_pushstrn(s, "another test", 7));
		cr_assert_eq(2LU, (unsigned long)s->size);
		cr_assert_eq(0, sp_stack_clear(s, sp_free));
		for (i = 0; i < SIZE_MAX / sizeof(char*); i++) {
			cr_assert_eq(0, sp_stack_pushstrn(s, "yet another test", 3));
			cr_assert_eq(0, strcmp("yet", sp_stack_getstr(s, 0)));
		}
		cr_assert_eq(SP_ERANGE, sp_stack_pushstrn(s, "size_t overflow", 6));
		cr_assert_eq(0, sp_stack_clear(s, sp_free));
		cr_assert_eq(SP_ERANGE, sp_stack_pushstrn(s, "too long", SIZE_MAX));
		cr_assert_eq(0, sp_stack_destroy(s, sp_free));
		s = sp_stack_create(sizeof(char*) + 1, 1);
		cr_assert_not_null(s);
		cr_assert_eq(SP_EILLEGAL, sp_stack_pushstrn(s, "incompatible elem_size", 12));
		cr_assert_eq(0, sp_stack_destroy(s, sp_free));
	}
}

Test(stack, peek)
{
	struct sp_stack *s;

	{ /* Generic form */
		struct data a, b;
		s = sp_stack_create(sizeof(struct data), 2);
		cr_assert_not_null(s);
		cr_assert_eq(0, sp_stack_peek(s));
		cr_assert_eq(0, sp_stack_peek(NULL));
		data_init(&a);
		cr_assert_eq(0, sp_stack_push(s, &a));
		b = *(struct data*)sp_stack_peek(s);
		cr_assert_eq(0, data_cmp(&a, &b));
		cr_assert_eq(0, sp_stack_destroy(s, data_dtor));
	}

	/* Suffixed form
	 * T  - type
	 * F1 - peek function
	 * F2 - push function
	 * V  - random value snippet
	 * M  - printf format string
	 */
#define test(T, F1, F2, V, M) do {                          \
		T a = (V), z = 0;                           \
		s = sp_stack_create(sizeof(T), 2);          \
		cr_assert_not_null(s);                      \
		cr_assert_eq(z, F1(s));                     \
		cr_assert_eq(z, F1(NULL));                  \
		cr_assert_eq(0, F2(s, a));                  \
		cr_assert_eq(a, F1(s));                     \
		cr_assert_eq(0, sp_stack_destroy(s, NULL)); \
		s = sp_stack_create(sizeof(T) + 1, 1);      \
		cr_assert_not_null(s);                      \
		s->size = 1;                                \
		cr_assert_eq(z, F1(s));                     \
		cr_assert_eq(0, sp_stack_destroy(s, NULL)); \
	} while (0)
	test(char          , sp_stack_peekc , sp_stack_pushc , IRANGE(1, CHAR_MAX) , "%hd");
	test(short         , sp_stack_peeks , sp_stack_pushs , IRANGE(1, SHRT_MAX) , "%hd");
	test(int           , sp_stack_peeki , sp_stack_pushi , FRANGE(1, INT_MAX)  , "%d");
	test(long          , sp_stack_peekl , sp_stack_pushl , FRANGE(1, LONG_MAX) , "%ld");
	test(signed char   , sp_stack_peeksc, sp_stack_pushsc, IRANGE(1, SCHAR_MAX), "%hd");
	test(unsigned char , sp_stack_peekuc, sp_stack_pushuc, IRANGE(1, UCHAR_MAX), "%hd");
	test(unsigned short, sp_stack_peekus, sp_stack_pushus, IRANGE(1, USHRT_MAX), "%hu");
	test(unsigned int  , sp_stack_peekui, sp_stack_pushui, FRANGE(1, UINT_MAX) , "%u");
	test(unsigned long , sp_stack_peekul, sp_stack_pushul, FRANGE(1, ULONG_MAX), "%lu");
	test(float         , sp_stack_peekf , sp_stack_pushf , FRANGE(1, FLT_MAX)  , "%f");
	test(double        , sp_stack_peekd , sp_stack_pushd , FRANGE(1, DBL_MAX)  , "%f");
	test(long double   , sp_stack_peekld, sp_stack_pushld, FRANGE(1, LDBL_MAX) , "%Lf");
#undef test

	{ /* Suffixed form - strings */
		const char *a = "testing";
		s = sp_stack_create(sizeof(char*), 2);
		cr_assert_not_null(s);
		cr_assert_eq(0, sp_stack_peekstr(s));
		cr_assert_eq(0, sp_stack_peekstr(NULL));
		cr_assert_eq(0, sp_stack_pushstr(s, a));
		cr_assert_eq(0, strcmp(a, sp_stack_peekstr(s)));
		cr_assert_eq(0, sp_stack_destroy(s, sp_free));
		s = sp_stack_create(sizeof(char*) + 1, 1);
		cr_assert_not_null(s);
		s->size = 1;
		cr_assert_eq(0, sp_stack_peekstr(s));
		cr_assert_eq(0, sp_stack_destroy(s, NULL));
	}
}

Test(stack, pop)
{
	struct sp_stack *s;

	{ /* Generic form */
		struct data a, b;
		s = sp_stack_create(sizeof(struct data), 2);
		cr_assert_not_null(s);
		cr_assert_eq(SP_EILLEGAL, sp_stack_pop(s, NULL));
		cr_assert_eq(SP_EINVAL, sp_stack_pop(NULL, NULL));
		data_init(&a);
		cr_assert_eq(0, sp_stack_push(s, &a));
		cr_assert_eq(1LU, (unsigned long)s->size);
		cr_assert_eq(0, sp_stack_pop(s, &b));
		cr_assert_eq(0LU, (unsigned long)s->size);
		cr_assert_eq(0, data_cmp(&a, &b));
		cr_assert_eq(0, sp_stack_destroy(s, NULL));
		cr_assert_eq(0, data_dtor(&b));
	}

	/* Suffixed form
	 * T  - type
	 * F1 - pop function
	 * F2 - push function
	 * V  - random value snippet
	 * M  - printf format string
	 */
#define test(T, F1, F2, V, M) do {                          \
		T a = (V), z = 0;                           \
		s = sp_stack_create(sizeof(T), 2);          \
		cr_assert_not_null(s);                      \
		cr_assert_eq(z, F1(s));                     \
		cr_assert_eq(z, F1(NULL));                  \
		cr_assert_eq(0, F2(s, a));                  \
		cr_assert_eq(1LU, (unsigned long)s->size);  \
		cr_assert_eq(a, F1(s));                     \
		cr_assert_eq(0LU, (unsigned long)s->size);  \
		cr_assert_eq(0, sp_stack_destroy(s, NULL)); \
		s = sp_stack_create(sizeof(T) + 1, 1);      \
		cr_assert_not_null(s);                      \
		s->size = 1;                                \
		cr_assert_eq(z, F1(s));                     \
		cr_assert_eq(0, sp_stack_destroy(s, NULL)); \
	} while (0)
	test(char          , sp_stack_popc , sp_stack_pushc , IRANGE(1, CHAR_MAX) , "%hd");
	test(short         , sp_stack_pops , sp_stack_pushs , IRANGE(1, SHRT_MAX) , "%hd");
	test(int           , sp_stack_popi , sp_stack_pushi , FRANGE(1, INT_MAX)  , "%d");
	test(long          , sp_stack_popl , sp_stack_pushl , FRANGE(1, LONG_MAX) , "%ld");
	test(signed char   , sp_stack_popsc, sp_stack_pushsc, IRANGE(1, SCHAR_MAX), "%hd");
	test(unsigned char , sp_stack_popuc, sp_stack_pushuc, IRANGE(1, UCHAR_MAX), "%hd");
	test(unsigned short, sp_stack_popus, sp_stack_pushus, IRANGE(1, USHRT_MAX), "%hu");
	test(unsigned int  , sp_stack_popui, sp_stack_pushui, FRANGE(1, UINT_MAX) , "%u");
	test(unsigned long , sp_stack_popul, sp_stack_pushul, FRANGE(1, ULONG_MAX), "%lu");
	test(float         , sp_stack_popf , sp_stack_pushf , FRANGE(1, FLT_MAX)  , "%f");
	test(double        , sp_stack_popd , sp_stack_pushd , FRANGE(1, DBL_MAX)  , "%f");
	test(long double   , sp_stack_popld, sp_stack_pushld, FRANGE(1, LDBL_MAX) , "%Lf");
#undef test

	{ /* Suffixed form - strings */
		const char *a = "test string";
		s = sp_stack_create(sizeof(char*), 2);
		cr_assert_not_null(s);
		cr_assert_eq(NULL, sp_stack_popstr(s));
		cr_assert_eq(NULL, sp_stack_popstr(NULL));
		cr_assert_eq(0, sp_stack_pushstr(s, a));
		cr_assert_eq(1LU, (unsigned long)s->size);
		cr_assert_eq(0, strcmp(a, sp_stack_popstr(s)));
		cr_assert_eq(0LU, (unsigned long)s->size);
		cr_assert_eq(0, sp_stack_destroy(s, sp_free));
		s = sp_stack_create(sizeof(char*) + 1, 1);
		cr_assert_not_null(s);
		s->size = 1;
		cr_assert_eq(NULL, sp_stack_popstr(s));
		cr_assert_eq(0, sp_stack_destroy(s, NULL));
	}
}

Test(stack, clear)
{
	struct sp_stack *s;
	unsigned i;
	s = sp_stack_create(sizeof(long double), 1000);
	cr_assert_not_null(s);
	cr_assert_eq(SP_EINVAL, sp_stack_clear(NULL, NULL));
	cr_assert_eq(0, sp_stack_clear(s, NULL));
	cr_assert_eq(0LU, (unsigned long)s->size);
	cr_assert_eq(0, sp_stack_destroy(s, NULL));
	s = sp_stack_create(sizeof(struct data), 1000);
	for (i = 0; i < 1000; i++) {
		struct data d;
		cr_assert_eq(0, data_init(&d));
		cr_assert_eq(0, sp_stack_push(s, &d));
	}
	cr_assert_eq(SP_EHANDLER, sp_stack_clear(s, data_dtor_bad));
	cr_assert_eq(0, sp_stack_clear(s, data_dtor));
	cr_assert_eq(0LU, (unsigned long)s->size);
	cr_assert_eq(0, sp_stack_destroy(s, NULL));
}

Test(stack, foreach)
{
	struct sp_stack *s;
	unsigned i;
	s = sp_stack_create(sizeof(long double), 1000);
	cr_assert_not_null(s);
	cr_assert_eq(SP_EINVAL, sp_stack_foreach(NULL, data_mutate));
	cr_assert_eq(SP_EINVAL, sp_stack_foreach(s, NULL));
	cr_assert_eq(SP_EINVAL, sp_stack_foreach(NULL, NULL));
	cr_assert_eq(0, sp_stack_destroy(s, NULL));
	s = sp_stack_create(sizeof(struct data), 1000);
	for (i = 0; i < 1000; i++) {
		struct data d;
		cr_assert_eq(0, data_init(&d));
		cr_assert_eq(0, sp_stack_push(s, &d));
	}
	cr_assert_eq(SP_EHANDLER, sp_stack_foreach(s, data_mutate_bad));
	cr_assert_eq(0, sp_stack_foreach(s, data_mutate));
	cr_assert_eq(0, sp_stack_foreach(s, data_verify));
	cr_assert_eq(0, sp_stack_destroy(s, data_dtor));
}

Test(stack, copy)
{
	struct sp_stack *s, *p;
	unsigned i;
	s = sp_stack_create(sizeof(int), 1000);
	p = sp_stack_create(sizeof(int), 333);
	cr_assert_not_null(s);
	cr_assert_eq(SP_EINVAL, sp_stack_copy(NULL, s, NULL));
	cr_assert_eq(SP_EINVAL, sp_stack_copy(p, NULL, NULL));
	cr_assert_eq(SP_EINVAL, sp_stack_copy(NULL, NULL, NULL));
	cr_assert_eq(0, sp_stack_copy(s, p, NULL));
	cr_assert_eq((unsigned long)p->size, (unsigned long)s->size);
	cr_assert_eq((unsigned long)p->elem_size, (unsigned long)s->elem_size);
	cr_assert_eq(0, sp_stack_copy(p, s, NULL));
	cr_assert_eq((unsigned long)p->size, (unsigned long)s->size);
	cr_assert_eq((unsigned long)p->elem_size, (unsigned long)s->elem_size);
	for (i = 0; i < 1000; i++) {
		cr_assert_eq(0, sp_stack_pushi(s, FRANGE(INT_MIN, INT_MAX)));
	}
	cr_assert_eq(0, sp_stack_copy(p, s, NULL));
	cr_assert_eq((unsigned long)p->size, (unsigned long)s->size);
	cr_assert_eq((unsigned long)p->elem_size, (unsigned long)s->elem_size);
	for (i = 0; i < 1000; i++) {
		int a, b;
		a = sp_stack_geti(s, i);
		b = sp_stack_geti(p, i);
		cr_assert_eq(a, b);
	}
	cr_assert_eq(0, sp_stack_destroy(s, NULL));
	cr_assert_eq(0, sp_stack_clear(p, NULL));
	s = sp_stack_create(sizeof(struct data), 1000);
	for (i = 0; i < 1000; i++) {
		struct data d;
		cr_assert_eq(0, data_init(&d));
		cr_assert_eq(0, sp_stack_push(s, &d));
	}
	cr_assert_eq(SP_EHANDLER, sp_stack_copy(p, s, data_cpy_bad));
	cr_assert_eq(0, sp_stack_copy(p, s, data_cpy));
	cr_assert_eq((unsigned long)p->size, (unsigned long)s->size);
	cr_assert_eq((unsigned long)p->elem_size, (unsigned long)s->elem_size);
	for (i = 0; i < 1000; i++) {
		struct data a, b;
		a = *(struct data*)sp_stack_get(s, i);
		b = *(struct data*)sp_stack_get(p, i);
		cr_assert_eq(0, data_cmp(&a, &b));
	}
	cr_assert_eq(0, sp_stack_destroy(s, data_dtor));
	cr_assert_eq(0, sp_stack_destroy(p, data_dtor));
}

Test(stack, insert)
{
	struct sp_stack *s;
	unsigned i;

	{ /* Generic form */
		int a = 10;
		struct data d[1000];
		s = sp_stack_create(sizeof(int), 1000);
		cr_assert_not_null(s);
		cr_assert_eq(SP_EINVAL, sp_stack_insert(s, 0, NULL));
		cr_assert_eq(SP_EINVAL, sp_stack_insert(NULL, 0, &a));
		cr_assert_eq(SP_EINVAL, sp_stack_insert(NULL, 0, NULL));
		cr_assert_eq(SP_EINDEX, sp_stack_insert(s, 1, &a));
		cr_assert_eq(0, sp_stack_insert(s, 0, &a));
		cr_assert_eq(1LU, (unsigned long)s->size);
		cr_assert_eq(10, sp_stack_geti(s, 0));
		cr_assert_eq(0, sp_stack_destroy(s, NULL));

		s = sp_stack_create(sizeof(struct data), 1000);
		cr_assert_not_null(s);
		for (i = 0; i < 1000; i++) {
			struct data a;
			size_t idx = IRANGE(0, i);
			cr_assert_eq(0, data_init(d + i));
			cr_assert_eq(0, sp_stack_insert(s, idx, d + i));
			cr_assert_eq((unsigned long)i + 1, (unsigned long)s->size);
			a = *(struct data*)sp_stack_get(s, idx);
			cr_assert_eq(0, data_cmp(&a, d + i));
		}
		cr_assert_eq(0, sp_stack_destroy(s, data_dtor));

		s = sp_stack_create(sizeof(struct data), 10);
		cr_assert_not_null(s);
		for (i = 0; i < 5; i++) {
			cr_assert_eq(0, data_init(d + i));
			cr_assert_eq(0, sp_stack_insert(s, i, d + i));
		}
		for (i = 0; i < 5; i++) {
			struct data a;
			a = *(struct data*)sp_stack_get(s, i);
			cr_assert_eq(0, data_cmp(&a, d + i));
		}
		{
			struct data a;
			cr_assert_eq(0, data_init(d + 5));
			cr_assert_eq(0, sp_stack_insert(s, 1, d + 5));
			a = *(struct data*)sp_stack_peek(s);
			cr_assert_eq(0, data_cmp(&a, d));
			cr_assert_eq(0, data_init(d + 6));
			cr_assert_eq(0, sp_stack_insert(s, 3, d + 6));
			a = *(struct data*)sp_stack_peek(s);
			cr_assert_eq(0, data_cmp(&a, d));
			cr_assert_eq(7LU, (unsigned long)s->size);
			cr_assert_eq(0, sp_stack_destroy(s, data_dtor));
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
		s = sp_stack_create(sizeof(T), 1000);                                       \
		cr_assert_not_null(s);                                                      \
		cr_assert_eq(SP_EINVAL, F1(NULL, 0, a));                                    \
		cr_assert_eq(SP_EINDEX, F1(s, 1, a));                                       \
		cr_assert_eq(0, F1(s, 0, a));                                               \
		cr_assert_eq(1LU, (unsigned long)s->size);                                  \
		cr_assert_eq(a, F2(s, 0));                                                  \
		cr_assert_eq(0, sp_stack_destroy(s, NULL));                                 \
                                                                                            \
		s = sp_stack_create(sizeof(T), 1000);                                       \
		cr_assert_not_null(s);                                                      \
		for (i = 0; i < 1000; i++) {                                                \
			size_t idx = IRANGE(0, i);                                          \
			d[i] = (V);                                                         \
			cr_assert_eq(0, F1(s, idx, d[i]));                                  \
			cr_assert_eq((unsigned long)i + 1, (unsigned long)s->size);         \
			cr_assert_eq(d[i], F2(s, idx));                                     \
		}                                                                           \
		cr_assert_eq(0, sp_stack_destroy(s, NULL));                                 \
                                                                                            \
		s = sp_stack_create(sizeof(T), 10);                                         \
		cr_assert_not_null(s);                                                      \
		for (i = 0; i < 5; i++) {                                                   \
			d[i] = (V);                                                         \
			cr_assert_eq(0, F1(s, i, d[i]));                                    \
		}                                                                           \
		for (i = 0; i < 5; i++) {                                                   \
			cr_assert_eq(d[i], F2(s, i));                                       \
		}                                                                           \
		d[5] = (V);                                                                 \
		cr_assert_eq(0, F1(s, 1, d[5]));                                            \
		cr_assert_eq(d[0], F2(s, 0));                                               \
		d[6] = (V);                                                                 \
		cr_assert_eq(0, F1(s, 3, d[6]));                                            \
		cr_assert_eq(d[0], F2(s, 0));                                               \
		cr_assert_eq(0, sp_stack_destroy(s, NULL));                                 \
		s = sp_stack_create(sizeof(T) + 1, 1000);                                   \
		cr_assert_not_null(s);                                                      \
		cr_assert_eq(SP_EILLEGAL, F1(s, 0, (V)));                                   \
		cr_assert_eq(0, sp_stack_destroy(s, NULL));                                 \
	} while (0)
	test(char          , sp_stack_insertc , sp_stack_getc , IRANGE(1, CHAR_MAX) , "%hd");
	test(short         , sp_stack_inserts , sp_stack_gets , IRANGE(1, SHRT_MAX) , "%hd");
	test(int           , sp_stack_inserti , sp_stack_geti , FRANGE(1, INT_MAX)  , "%d");
	test(long          , sp_stack_insertl , sp_stack_getl , FRANGE(1, LONG_MAX) , "%ld");
	test(signed char   , sp_stack_insertsc, sp_stack_getsc, IRANGE(1, SCHAR_MAX), "%hd");
	test(unsigned char , sp_stack_insertuc, sp_stack_getuc, IRANGE(1, UCHAR_MAX), "%hd");
	test(unsigned short, sp_stack_insertus, sp_stack_getus, IRANGE(1, USHRT_MAX), "%hu");
	test(unsigned int  , sp_stack_insertui, sp_stack_getui, FRANGE(1, UINT_MAX) , "%u");
	test(unsigned long , sp_stack_insertul, sp_stack_getul, FRANGE(1, ULONG_MAX), "%lu");
	test(float         , sp_stack_insertf , sp_stack_getf , FRANGE(1, FLT_MAX)  , "%f");
	test(double        , sp_stack_insertd , sp_stack_getd , FRANGE(1, DBL_MAX)  , "%f");
	test(long double   , sp_stack_insertld, sp_stack_getld, FRANGE(1, LDBL_MAX) , "%Lf");
#undef test

	{ /* Suffixed form - strings */
		char a[25] = " test string";
		char d[1000][25];
		s = sp_stack_create(sizeof(char*), 1000);
		cr_assert_not_null(s);
		cr_assert_eq(SP_EINVAL, sp_stack_insertstr(NULL, 0, a));
		cr_assert_eq(SP_EINVAL, sp_stack_insertstr(s, 0, NULL));
		cr_assert_eq(SP_EINDEX, sp_stack_insertstr(s, 1, a));
		cr_assert_eq(0, sp_stack_insertstr(s, 0, a));
		cr_assert_eq(1LU, (unsigned long)s->size);
		cr_assert_eq(0, sp_stack_destroy(s, sp_free));

		s = sp_stack_create(sizeof(char*), 1000);
		cr_assert_not_null(s);
		for (i = 0; i < 1000; i++) {
			size_t idx = IRANGE(0, i);
			a[0] = IRANGE(' ', '~');
			strcpy(d[i], a);
			cr_assert_eq(0, sp_stack_insertstr(s, idx, d[i]));
			cr_assert_eq((unsigned long)i + 1, (unsigned long)s->size);
			cr_assert_eq(0, strcmp(d[i], sp_stack_getstr(s, idx)));
		}
		cr_assert_eq(0, sp_stack_destroy(s, sp_free));

		s = sp_stack_create(sizeof(char*), 1000);
		for (i = 0; i < SIZE_MAX / sizeof(char*); i++) {
			cr_assert_eq(0, sp_stack_insertstr(s, 0, a));
			cr_assert_eq(0, strcmp(a, sp_stack_getstr(s, 0)));
		}
		cr_assert_eq(SP_ERANGE, sp_stack_insertstr(s, 0, a));
		cr_assert_eq(0, sp_stack_destroy(s, sp_free));

		s = sp_stack_create(sizeof(char*), 1);
		char too_long[SIZE_MAX];
		for (i = 0; i < SIZE_MAX; i++)
			too_long[i] = 'a';
		too_long[SIZE_MAX] = '\0';
		cr_assert_eq(SP_ERANGE, sp_stack_insertstr(s, 0, too_long));
		cr_assert_eq(0, sp_stack_destroy(s, sp_free));

		s = sp_stack_create(sizeof(char*), 10);
		cr_assert_not_null(s);
		for (i = 0; i < 5; i++)
			cr_assert_eq(0, sp_stack_insertstr(s, i, d[i]));
		for (i = 0; i < 5; i++)
			cr_assert_eq(0, strcmp(d[i], sp_stack_getstr(s, i)));
		cr_assert_eq(0, sp_stack_insertstr(s, 1, d[5]));
		cr_assert_eq(0, strcmp(d[0], sp_stack_getstr(s, 0)));
		cr_assert_eq(0, sp_stack_insertstr(s, 3, d[6]));
		cr_assert_eq(0, strcmp(d[0], sp_stack_getstr(s, 0)));
		cr_assert_eq(0, sp_stack_destroy(s, sp_free));
		s = sp_stack_create(sizeof(char*) + 1, 1000);
		cr_assert_not_null(s);
		cr_assert_eq(SP_EILLEGAL, sp_stack_insertstr(s, 0, "test string"));
		cr_assert_eq(0, sp_stack_destroy(s, NULL));

		/* substrings (strn) */
		s = sp_stack_create(sizeof(char*), 1000);
		cr_assert_not_null(s);
		cr_assert_eq(SP_EINVAL, sp_stack_insertstrn(NULL, 0, "test string", 5));
		cr_assert_eq(SP_EINVAL, sp_stack_insertstrn(s, 0, NULL, 5));
		cr_assert_eq(SP_EINDEX, sp_stack_insertstrn(s, 1, "another test", 3));
		cr_assert_eq(0, sp_stack_insertstrn(s, 0, "yet another test", 11));
		cr_assert_eq(1LU, (unsigned long)s->size);
		cr_assert_eq(0, sp_stack_insertstrn(s, 0, "one more", 3));
		cr_assert_eq(2LU, (unsigned long)s->size);
		cr_assert_eq(0, sp_stack_insertstrn(s, 0, "last test", 0));
		cr_assert_eq(3LU, (unsigned long)s->size);
		cr_assert_eq(0, strcmp("", sp_stack_getstr(s, 0)));
		cr_assert_eq(0, strcmp("one", sp_stack_getstr(s, 1)));
		cr_assert_eq(0, strcmp("yet another", sp_stack_getstr(s, 2)));
		cr_assert_eq(0, sp_stack_destroy(s, sp_free));

		s = sp_stack_create(sizeof(char*), 1000);
		cr_assert_not_null(s);
		for (i = 0; i < 1000; i++) {
			size_t idx = IRANGE(0, i);
			a[0] = IRANGE(' ', '~');
			strcpy(d[i], a);
			cr_assert_eq(0, sp_stack_insertstrn(s, idx, d[i], 24));
			cr_assert_eq((unsigned long)i + 1, (unsigned long)s->size);
			cr_assert_eq(0, strcmp(d[i], sp_stack_getstr(s, idx)));
		}
		cr_assert_eq(0, sp_stack_destroy(s, sp_free));

		s = sp_stack_create(sizeof(char*), 1000);
		for (i = 0; i < SIZE_MAX / sizeof(char*); i++) {
			cr_assert_eq(0, sp_stack_insertstrn(s, 0, "test", 3));
			cr_assert_eq(0, strcmp("tes", sp_stack_getstr(s, 0)));
		}
		cr_assert_eq(SP_ERANGE, sp_stack_insertstrn(s, 0, "test", 3));
		cr_assert_eq(0, sp_stack_destroy(s, sp_free));

		s = sp_stack_create(sizeof(char*), 10);
		cr_assert_not_null(s);
		for (i = 0; i < 5; i++)
			cr_assert_eq(0, sp_stack_insertstrn(s, i, d[i], 24));
		for (i = 0; i < 5; i++)
			cr_assert_eq(0, strcmp(d[i], sp_stack_getstr(s, i)));
		cr_assert_eq(0, sp_stack_insertstrn(s, 1, d[5], 24));
		cr_assert_eq(0, strcmp(d[0], sp_stack_getstr(s, 0)));
		cr_assert_eq(0, sp_stack_insertstrn(s, 3, d[6], 24));
		cr_assert_eq(0, strcmp(d[0], sp_stack_getstr(s, 0)));
		cr_assert_eq(0, sp_stack_destroy(s, sp_free));
		s = sp_stack_create(sizeof(char*) + 1, 1000);
		cr_assert_not_null(s);
		cr_assert_eq(SP_EILLEGAL, sp_stack_insertstrn(s, 0, "test string", 4));
		cr_assert_eq(0, sp_stack_destroy(s, NULL));
	}
}

Test(stack, qinsert)
{
	struct sp_stack *s;
	unsigned i;

	{ /* Generic form */
		int a = 10;
		struct data d[1000];
		s = sp_stack_create(sizeof(int), 1000);
		cr_assert_not_null(s);
		cr_assert_eq(SP_EINVAL, sp_stack_qinsert(s, 0, NULL));
		cr_assert_eq(SP_EINVAL, sp_stack_qinsert(NULL, 0, &a));
		cr_assert_eq(SP_EINVAL, sp_stack_qinsert(NULL, 0, NULL));
		cr_assert_eq(SP_EINDEX, sp_stack_qinsert(s, 1, &a));
		cr_assert_eq(0, sp_stack_qinsert(s, 0, &a));
		cr_assert_eq(1LU, (unsigned long)s->size);
		cr_assert_eq(10, sp_stack_geti(s, 0));
		cr_assert_eq(0, sp_stack_destroy(s, NULL));

		s = sp_stack_create(sizeof(struct data), 1000);
		cr_assert_not_null(s);
		for (i = 0; i < 1000; i++) {
			struct data a;
			size_t idx = IRANGE(0, i);
			cr_assert_eq(0, data_init(d + i));
			cr_assert_eq(0, sp_stack_qinsert(s, idx, d + i));
			cr_assert_eq((unsigned long)i + 1, (unsigned long)s->size);
			a = *(struct data*)sp_stack_get(s, idx);
			cr_assert_eq(0, data_cmp(&a, d + i));
		}
		cr_assert_eq(0, sp_stack_destroy(s, data_dtor));

		s = sp_stack_create(sizeof(struct data), 10);
		cr_assert_not_null(s);
		for (i = 0; i < 5; i++) {
			cr_assert_eq(0, data_init(d + i));
			cr_assert_eq(0, sp_stack_qinsert(s, i, d + i));
		}
		for (i = 0; i < 5; i++) {
			struct data a;
			a = *(struct data*)sp_stack_get(s, i);
			cr_assert_eq(0, data_cmp(&a, d + (8 - i) % 5));
		}
		{
			struct data a;
			cr_assert_eq(0, data_init(d + 5));
			cr_assert_eq(0, sp_stack_qinsert(s, 1, d + 5));
			a = *(struct data*)sp_stack_peek(s);
			cr_assert_eq(0, data_cmp(&a, d + 3));
			cr_assert_eq(0, data_init(d + 6));
			cr_assert_eq(0, sp_stack_qinsert(s, 3, d + 6));
			a = *(struct data*)sp_stack_peek(s);
			cr_assert_eq(0, data_cmp(&a, d + 2));
			cr_assert_eq(7LU, (unsigned long)s->size);
			cr_assert_eq(0, sp_stack_destroy(s, data_dtor));
		}
	}

	/* Suffixed form
	 * T  - type
	 * F1 - qinsert function
	 * F2 - get function
	 * V  - random value snippet
	 * M  - printf format string
	 */
#define test(T, F1, F2, V, M) do {                                                          \
		T a = (V);                                                                  \
		T d[1000];                                                                  \
		s = sp_stack_create(sizeof(T), 1000);                                       \
		cr_assert_not_null(s);                                                      \
		cr_assert_eq(SP_EINVAL, F1(NULL, 0, a));                                    \
		cr_assert_eq(SP_EINDEX, F1(s, 1, a));                                       \
		cr_assert_eq(0, F1(s, 0, a));                                               \
		cr_assert_eq(1LU, (unsigned long)s->size);                                  \
		cr_assert_eq(a, F2(s, 0));                                                  \
		cr_assert_eq(0, sp_stack_destroy(s, NULL));                                 \
                                                                                            \
		s = sp_stack_create(sizeof(T), 1000);                                       \
		cr_assert_not_null(s);                                                      \
		for (i = 0; i < 1000; i++) {                                                \
			size_t idx = IRANGE(0, i);                                          \
			d[i] = (V);                                                         \
			cr_assert_eq(0, F1(s, idx, d[i]));                                  \
			cr_assert_eq((unsigned long)i + 1, (unsigned long)s->size);         \
			cr_assert_eq(d[i], F2(s, idx));                                     \
		}                                                                           \
		cr_assert_eq(0, sp_stack_destroy(s, NULL));                                 \
                                                                                            \
		s = sp_stack_create(sizeof(T), 10);                                         \
		cr_assert_not_null(s);                                                      \
		for (i = 0; i < 5; i++) {                                                   \
			d[i] = (V);                                                         \
			cr_assert_eq(0, F1(s, i, d[i]));                                    \
		}                                                                           \
		for (i = 0; i < 5; i++) {                                                   \
			cr_assert_eq(d[(8 - i) % 5], F2(s, i));                             \
		}                                                                           \
		d[5] = (V);                                                                 \
		cr_assert_eq(0, F1(s, 1, d[5]));                                            \
		cr_assert_eq(d[3], F2(s, 0));                                               \
		d[6] = (V);                                                                 \
		cr_assert_eq(0, F1(s, 3, d[6]));                                            \
		cr_assert_eq(d[2], F2(s, 0));                                               \
		cr_assert_eq(0, sp_stack_destroy(s, NULL));                                 \
		s = sp_stack_create(sizeof(T) + 1, 1000);                                   \
		cr_assert_not_null(s);                                                      \
		cr_assert_eq(SP_EILLEGAL, F1(s, 0, (V)));                                   \
		cr_assert_eq(0, sp_stack_destroy(s, NULL));                                 \
	} while (0)
	test(char          , sp_stack_qinsertc , sp_stack_getc , IRANGE(1, CHAR_MAX) , "%hd");
	test(short         , sp_stack_qinserts , sp_stack_gets , IRANGE(1, SHRT_MAX) , "%hd");
	test(int           , sp_stack_qinserti , sp_stack_geti , FRANGE(1, INT_MAX)  , "%d");
	test(long          , sp_stack_qinsertl , sp_stack_getl , FRANGE(1, LONG_MAX) , "%ld");
	test(signed char   , sp_stack_qinsertsc, sp_stack_getsc, IRANGE(1, SCHAR_MAX), "%hd");
	test(unsigned char , sp_stack_qinsertuc, sp_stack_getuc, IRANGE(1, UCHAR_MAX), "%hd");
	test(unsigned short, sp_stack_qinsertus, sp_stack_getus, IRANGE(1, USHRT_MAX), "%hu");
	test(unsigned int  , sp_stack_qinsertui, sp_stack_getui, FRANGE(1, UINT_MAX) , "%u");
	test(unsigned long , sp_stack_qinsertul, sp_stack_getul, FRANGE(1, ULONG_MAX), "%lu");
	test(float         , sp_stack_qinsertf , sp_stack_getf , FRANGE(1, FLT_MAX)  , "%f");
	test(double        , sp_stack_qinsertd , sp_stack_getd , FRANGE(1, DBL_MAX)  , "%f");
	test(long double   , sp_stack_qinsertld, sp_stack_getld, FRANGE(1, LDBL_MAX) , "%Lf");
#undef test

	{ /* Suffixed form - strings */
		char a[25] = " test string";
		char d[1000][25];
		s = sp_stack_create(sizeof(char*), 1000);
		cr_assert_not_null(s);
		cr_assert_eq(SP_EINVAL, sp_stack_qinsertstr(NULL, 0, a));
		cr_assert_eq(SP_EINVAL, sp_stack_qinsertstr(s, 0, NULL));
		cr_assert_eq(SP_EINDEX, sp_stack_qinsertstr(s, 1, a));
		cr_assert_eq(0, sp_stack_qinsertstr(s, 0, a));
		cr_assert_eq(1LU, (unsigned long)s->size);
		cr_assert_eq(0, strcmp(a, sp_stack_getstr(s, 0)));
		cr_assert_eq(0, sp_stack_destroy(s, sp_free));

		s = sp_stack_create(sizeof(char*), 1000);
		cr_assert_not_null(s);
		for (i = 0; i < 1000; i++) {
			size_t idx = IRANGE(0, i);
			a[0] = IRANGE(' ', '~');
			strcpy(d[i], a);
			cr_assert_eq(0, sp_stack_qinsertstr(s, idx, d[i]));
			cr_assert_eq((unsigned long)i + 1, (unsigned long)s->size);
			cr_assert_eq(0, strcmp(d[i], sp_stack_getstr(s, idx)));
		}
		cr_assert_eq(0, sp_stack_destroy(s, sp_free));

		s = sp_stack_create(sizeof(char*), 1000);
		for (i = 0; i < SIZE_MAX / sizeof(char*); i++) {
			cr_assert_eq(0, sp_stack_qinsertstr(s, 0, a));
			cr_assert_eq(0, strcmp(a, sp_stack_getstr(s, 0)));
		}
		cr_assert_eq(SP_ERANGE, sp_stack_qinsertstr(s, 0, a));
		cr_assert_eq(0, sp_stack_destroy(s, sp_free));

		s = sp_stack_create(sizeof(char*), 1);
		char too_long[SIZE_MAX];
		for (i = 0; i < SIZE_MAX; i++)
			too_long[i] = 'a';
		too_long[SIZE_MAX] = '\0';
		cr_assert_eq(SP_ERANGE, sp_stack_qinsertstr(s, 0, too_long));
		cr_assert_eq(0, sp_stack_destroy(s, sp_free));

		s = sp_stack_create(sizeof(char*), 10);
		cr_assert_not_null(s);
		for (i = 0; i < 5; i++)
			cr_assert_eq(0, sp_stack_qinsertstr(s, i, d[i]));
		for (i = 0; i < 5; i++)
			cr_assert_eq(0, strcmp(d[(8 - i) % 5], sp_stack_getstr(s, i)));
		cr_assert_eq(0, sp_stack_qinsertstr(s, 1, d[5]));
		cr_assert_eq(0, strcmp(d[3], sp_stack_getstr(s, 0)));
		cr_assert_eq(0, sp_stack_qinsertstr(s, 3, d[6]));
		cr_assert_eq(0, strcmp(d[2], sp_stack_getstr(s, 0)));
		cr_assert_eq(0, sp_stack_destroy(s, sp_free));
		s = sp_stack_create(sizeof(char*) + 1, 1000);
		cr_assert_not_null(s);
		cr_assert_eq(SP_EILLEGAL, sp_stack_qinsertstr(s, 0, a));
		cr_assert_eq(0, sp_stack_destroy(s, NULL));

		/* substrings (strn) */
		s = sp_stack_create(sizeof(char*), 1000);
		cr_assert_not_null(s);
		cr_assert_eq(SP_EINVAL, sp_stack_qinsertstrn(NULL, 0, "test string", 5));
		cr_assert_eq(SP_EINVAL, sp_stack_qinsertstrn(s, 0, NULL, 5));
		cr_assert_eq(SP_EINDEX, sp_stack_qinsertstrn(s, 1, "another test", 3));
		cr_assert_eq(0, sp_stack_qinsertstrn(s, 0, "yet another test", 11));
		cr_assert_eq(1LU, (unsigned long)s->size);
		cr_assert_eq(0, sp_stack_qinsertstrn(s, 0, "one more", 3));
		cr_assert_eq(2LU, (unsigned long)s->size);
		cr_assert_eq(0, sp_stack_qinsertstrn(s, 0, "last test", 0));
		cr_assert_eq(3LU, (unsigned long)s->size);
		cr_assert_eq(0, strcmp("", sp_stack_getstr(s, 0)));
		cr_assert_eq(0, strcmp("one", sp_stack_getstr(s, 1)));
		cr_assert_eq(0, strcmp("yet another", sp_stack_getstr(s, 2)));
		cr_assert_eq(0, sp_stack_destroy(s, sp_free));

		s = sp_stack_create(sizeof(char*), 1000);
		cr_assert_not_null(s);
		for (i = 0; i < 1000; i++) {
			size_t idx = IRANGE(0, i);
			a[0] = IRANGE(' ', '~');
			strcpy(d[i], a);
			cr_assert_eq(0, sp_stack_qinsertstrn(s, idx, d[i], 24));
			cr_assert_eq((unsigned long)i + 1, (unsigned long)s->size);
			cr_assert_eq(0, strcmp(d[i], sp_stack_getstr(s, idx)));
		}
		cr_assert_eq(0, sp_stack_destroy(s, sp_free));

		s = sp_stack_create(sizeof(char*), 1000);
		for (i = 0; i < SIZE_MAX / sizeof(char*); i++) {
			cr_assert_eq(0, sp_stack_qinsertstrn(s, 0, "test", 3));
			cr_assert_eq(0, strcmp("tes", sp_stack_getstr(s, 0)));
		}
		cr_assert_eq(SP_ERANGE, sp_stack_qinsertstrn(s, 0, "test", 3));
		cr_assert_eq(0, sp_stack_destroy(s, sp_free));

		s = sp_stack_create(sizeof(char*), 10);
		cr_assert_not_null(s);
		for (i = 0; i < 5; i++)
			cr_assert_eq(0, sp_stack_qinsertstrn(s, i, d[i], 24));
		for (i = 0; i < 5; i++)
			cr_assert_eq(0, strcmp(d[(8 - i) % 5], sp_stack_getstr(s, i)));
		cr_assert_eq(0, sp_stack_qinsertstrn(s, 1, d[5], 24));
		cr_assert_eq(0, strcmp(d[3], sp_stack_getstr(s, 0)));
		cr_assert_eq(0, sp_stack_qinsertstrn(s, 3, d[6], 24));
		cr_assert_eq(0, strcmp(d[2], sp_stack_getstr(s, 0)));
		cr_assert_eq(0, sp_stack_destroy(s, sp_free));
		s = sp_stack_create(sizeof(char*) + 1, 1000);
		cr_assert_not_null(s);
		cr_assert_eq(SP_EILLEGAL, sp_stack_qinsertstrn(s, 0, "test string", 4));
		cr_assert_eq(0, sp_stack_destroy(s, NULL));
	}
}

Test(stack, remove)
{
	struct sp_stack *s;
	unsigned i;

	{ /* Generic form */
		struct data a;
		struct data d[100];
		s = sp_stack_create(sizeof(int), 1000);
		cr_assert_not_null(s);
		cr_assert_eq(SP_EINDEX, sp_stack_remove(s, 0, &a));
		cr_assert_eq(0, data_init(&a));
		cr_assert_eq(0, sp_stack_push(s, &a));
		cr_assert_eq(SP_EINVAL, sp_stack_remove(NULL, 0, &a));
		cr_assert_eq(SP_EINVAL, sp_stack_remove(NULL, 0, NULL));
		cr_assert_eq(SP_EINDEX, sp_stack_remove(s, 1, &a));
		cr_assert_eq(1LU, (unsigned long)s->size);
		cr_assert_eq(0, sp_stack_remove(s, 0, NULL));
		cr_assert_eq(0LU, (unsigned long)s->size);
		cr_assert_eq(0, sp_stack_destroy(s, NULL));
		cr_assert_eq(0, data_dtor(&a));

		s = sp_stack_create(sizeof(struct data), 100);
		cr_assert_not_null(s);
		for (i = 0; i < 100; i++) {
			cr_assert_eq(0, data_init(d + i));
			cr_assert_eq(0, sp_stack_push(s, d + i));
		}
		for (i = 0; i < 100; i++) {
			struct data a;
			size_t idx = IRANGE(0, s->size - 1), j;
			int found = 0;
			cr_assert_eq(0, sp_stack_remove(s, idx, &a));
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
		cr_assert_eq(0, sp_stack_destroy(s, NULL));

		s = sp_stack_create(sizeof(struct data), 10);
		cr_assert_not_null(s);
		for (i = 0; i < 8; i++) {
			cr_assert_eq(0, sp_stack_push(s, d + i));
		}
		cr_assert_eq(0, sp_stack_remove(s, 3, &a));
		cr_assert_eq(0, data_cmp(d + 4, &a));
		cr_assert_eq(7LU, (unsigned long)s->size);
		cr_assert_eq(0, sp_stack_remove(s, 0, &a));
		cr_assert_eq(0, data_cmp(d + 7, &a));
		cr_assert_eq(6LU, (unsigned long)s->size);
		cr_assert_eq(0, sp_stack_remove(s, 3, &a));
		cr_assert_eq(0, data_cmp(d + 2, &a));
		cr_assert_eq(5LU, (unsigned long)s->size);
		cr_assert_eq(0, sp_stack_destroy(s, NULL));
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
#define test(T, F1, F2, V, M) do {                                                  \
		T a = (V), z = 0;                                                   \
		T d[100];                                                           \
		s = sp_stack_create(sizeof(T), 1000);                               \
		cr_assert_not_null(s);                                              \
		cr_assert_eq(z, F1(s, 0));                                          \
		cr_assert_eq(0, F2(s, a));                                          \
		cr_assert_eq(z, F1(NULL, 0));                                       \
		cr_assert_eq(z, F1(s, 1));                                          \
		cr_assert_eq(1LU, (unsigned long)s->size);                          \
		cr_assert_eq(a, F1(s, 0));                                          \
		cr_assert_eq(0LU, (unsigned long)s->size);                          \
		cr_assert_eq(0, sp_stack_destroy(s, NULL));                         \
                                                                                    \
		s = sp_stack_create(sizeof(T), 100);                                \
		cr_assert_not_null(s);                                              \
		for (i = 0; i < 100; i++) {                                         \
			d[i] = (V);                                                 \
			cr_assert_eq(0, F2(s, d[i]));                               \
		}                                                                   \
		for (i = 0; i < 100; i++) {                                         \
			T a;                                                        \
			size_t idx = IRANGE(0, s->size - 1), j;                     \
			int found = 0;                                              \
			cr_assert_neq(0, (a = F1(s, idx)));                         \
			for (j = 0; j < 100; j++) {                                 \
				/* Lazy and slow way to check, but on average it's
				 * enough */                                        \
				if (a == d[j]) {                                    \
					found = 1;                                  \
					break;                                      \
				}                                                   \
			}                                                           \
			cr_assert_eq(1, found);                                     \
		}                                                                   \
		cr_assert_eq(0, sp_stack_destroy(s, NULL));                         \
                                                                                    \
		s = sp_stack_create(sizeof(T), 10);                                 \
		cr_assert_not_null(s);                                              \
		for (i = 0; i < 8; i++) {                                           \
			cr_assert_eq(0, F2(s, d[i]));                               \
		}                                                                   \
		cr_assert_eq(d[4], F1(s, 3));                                       \
		cr_assert_eq(7LU, (unsigned long)s->size);                          \
		cr_assert_eq(d[7], F1(s, 0));                                       \
		cr_assert_eq(6LU, (unsigned long)s->size);                          \
		cr_assert_eq(d[2], F1(s, 3));                                       \
		cr_assert_eq(5LU, (unsigned long)s->size);                          \
		cr_assert_eq(0, sp_stack_destroy(s, NULL));                         \
		s = sp_stack_create(sizeof(T) + 1, 1000);                           \
		cr_assert_not_null(s);                                              \
		s->size = 1;                                                        \
		cr_assert_eq(z, F1(s, 0));                                          \
		cr_assert_eq(0, sp_stack_destroy(s, NULL));                         \
	} while (0)
	test(char          , sp_stack_removec , sp_stack_pushc , IRANGE(1, CHAR_MAX) , "%hd");
	test(short         , sp_stack_removes , sp_stack_pushs , IRANGE(1, SHRT_MAX) , "%hd");
	test(int           , sp_stack_removei , sp_stack_pushi , FRANGE(1, INT_MAX)  , "%d");
	test(long          , sp_stack_removel , sp_stack_pushl , FRANGE(1, LONG_MAX) , "%ld");
	test(signed char   , sp_stack_removesc, sp_stack_pushsc, IRANGE(1, SCHAR_MAX), "%hd");
	test(unsigned char , sp_stack_removeuc, sp_stack_pushuc, IRANGE(1, UCHAR_MAX), "%hd");
	test(unsigned short, sp_stack_removeus, sp_stack_pushus, IRANGE(1, USHRT_MAX), "%hu");
	test(unsigned int  , sp_stack_removeui, sp_stack_pushui, FRANGE(1, UINT_MAX) , "%u");
	test(unsigned long , sp_stack_removeul, sp_stack_pushul, FRANGE(1, ULONG_MAX), "%lu");
	test(float         , sp_stack_removef , sp_stack_pushf , FRANGE(1, FLT_MAX)  , "%f");
	test(double        , sp_stack_removed , sp_stack_pushd , FRANGE(1, DBL_MAX)  , "%f");
	test(long double   , sp_stack_removeld, sp_stack_pushld, FRANGE(1, LDBL_MAX) , "%Lf");
#undef test

	{ /* Suffixed form - strings */
		char a[25] = " test string";
		char d[100][25];
		s = sp_stack_create(sizeof(char*), 1000);
		cr_assert_not_null(s);
		cr_assert_eq(NULL, sp_stack_removestr(s, 0));
		cr_assert_eq(0, sp_stack_pushstr(s, a));
		cr_assert_eq(NULL, sp_stack_removestr(NULL, 0));
		cr_assert_eq(NULL, sp_stack_removestr(s, 1));
		cr_assert_eq(1LU, (unsigned long)s->size);
		cr_assert_eq(0, strcmp(a, sp_stack_removestr(s, 0)));
		cr_assert_eq(0LU, (unsigned long)s->size);
		cr_assert_eq(0, sp_stack_destroy(s, sp_free));

		s = sp_stack_create(sizeof(char*), 100);
		cr_assert_not_null(s);
		for (i = 0; i < 100; i++) {
			a[0] = IRANGE(' ', '~');
			strcpy(d[i], a);
			cr_assert_eq(0, sp_stack_pushstr(s, d[i]));
		}
		for (i = 0; i < 100; i++) {
			char *a;
			size_t idx = IRANGE(0, s->size - 1), j;
			int found = 0;
			cr_assert_neq(NULL, (a = sp_stack_removestr(s, idx)));
			for (j = 0; j < 100; j++) {
				/* Lazy and slow way to check, but on average
				 * it's enough */
				if (strcmp(a, d[j]) == 0) {
					found = 1;
					break;
				}
			}
			cr_assert_eq(1, found);
			free(a);
		}
		cr_assert_eq(0, sp_stack_destroy(s, sp_free));

		s = sp_stack_create(sizeof(char*), 10);
		cr_assert_not_null(s);
		for (i = 0; i < 8; i++)
			cr_assert_eq(0, sp_stack_pushstr(s, d[i]));
		cr_assert_eq(0, strcmp(d[4], sp_stack_removestr(s, 3)));
		cr_assert_eq(7LU, (unsigned long)s->size);
		cr_assert_eq(0, strcmp(d[7], sp_stack_removestr(s, 0)));
		cr_assert_eq(6LU, (unsigned long)s->size);
		cr_assert_eq(0, strcmp(d[2], sp_stack_removestr(s, 3)));
		cr_assert_eq(5LU, (unsigned long)s->size);
		cr_assert_eq(0, sp_stack_destroy(s, sp_free));
		s = sp_stack_create(sizeof(char*) + 1, 1000);
		cr_assert_not_null(s);
		s->size = 1;
		cr_assert_eq(NULL, sp_stack_removestr(s, 0));
		cr_assert_eq(0, sp_stack_destroy(s, NULL));
	}
}

Test(stack, qremove)
{
	struct sp_stack *s;
	unsigned i;

	{ /* Generic form */
		struct data a;
		struct data d[100];
		s = sp_stack_create(sizeof(int), 1000);
		cr_assert_not_null(s);
		cr_assert_eq(SP_EINDEX, sp_stack_qremove(s, 0, &a));
		cr_assert_eq(0, data_init(&a));
		cr_assert_eq(0, sp_stack_push(s, &a));
		cr_assert_eq(SP_EINVAL, sp_stack_qremove(NULL, 0, &a));
		cr_assert_eq(SP_EINVAL, sp_stack_qremove(NULL, 0, NULL));
		cr_assert_eq(SP_EINDEX, sp_stack_qremove(s, 1, &a));
		cr_assert_eq(1LU, (unsigned long)s->size);
		cr_assert_eq(0, sp_stack_qremove(s, 0, NULL));
		cr_assert_eq(0LU, (unsigned long)s->size);
		cr_assert_eq(0, sp_stack_destroy(s, NULL));
		cr_assert_eq(0, data_dtor(&a));

		s = sp_stack_create(sizeof(struct data), 100);
		cr_assert_not_null(s);
		for (i = 0; i < 100; i++) {
			cr_assert_eq(0, data_init(d + i));
			cr_assert_eq(0, sp_stack_push(s, d + i));
		}
		for (i = 0; i < 100; i++) {
			struct data a;
			size_t idx = IRANGE(0, s->size - 1), j;
			int found = 0;
			cr_assert_eq(0, sp_stack_qremove(s, idx, &a));
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
		cr_assert_eq(0, sp_stack_destroy(s, NULL));

		s = sp_stack_create(sizeof(struct data), 10);
		cr_assert_not_null(s);
		for (i = 0; i < 8; i++) {
			cr_assert_eq(0, sp_stack_push(s, d + i));
		}
		cr_assert_eq(0, sp_stack_qremove(s, 3, &a));
		cr_assert_eq(0, data_cmp(d + 4, &a));
		a = *(struct data*)sp_stack_get(s, 3);
		cr_assert_eq(0, data_cmp(d + 3, &a));
		cr_assert_eq(0, sp_stack_qremove(s, 0, &a));
		cr_assert_eq(0, data_cmp(d + 6, &a));
		a = *(struct data*)sp_stack_get(s, 0);
		cr_assert_eq(0, data_cmp(d + 5, &a));
		cr_assert_eq(0, sp_stack_qremove(s, 3, &a));
		cr_assert_eq(0, data_cmp(d + 2, &a));
		cr_assert_eq(0, sp_stack_destroy(s, NULL));
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
#define test(T, F1, F2, F3, V, M) do {                                              \
		T a = (V), z = 0;                                                   \
		T d[100];                                                           \
		s = sp_stack_create(sizeof(T), 1000);                               \
		cr_assert_not_null(s);                                              \
		cr_assert_eq(z, F1(s, 0));                                          \
		cr_assert_eq(0, F2(s, a));                                          \
		cr_assert_eq(z, F1(NULL, 0));                                       \
		cr_assert_eq(z, F1(s, 1));                                          \
		cr_assert_eq(1LU, (unsigned long)s->size);                          \
		cr_assert_eq(a, F1(s, 0));                                          \
		cr_assert_eq(0LU, (unsigned long)s->size);                          \
		cr_assert_eq(0, sp_stack_destroy(s, NULL));                         \
                                                                                    \
		s = sp_stack_create(sizeof(T), 100);                                \
		cr_assert_not_null(s);                                              \
		for (i = 0; i < 100; i++) {                                         \
			d[i] = (V);                                                 \
			cr_assert_eq(0, F2(s, d[i]));                               \
		}                                                                   \
		for (i = 0; i < 100; i++) {                                         \
			T a;                                                        \
			size_t idx = IRANGE(0, s->size - 1), j;                     \
			int found = 0;                                              \
			cr_assert_neq(0, (a = F1(s, idx)));                         \
			for (j = 0; j < 100; j++) {                                 \
				/* Lazy and slow way to check, but on average it's
				 * enough */                                        \
				if (a == d[j]) {                                    \
					found = 1;                                  \
					break;                                      \
				}                                                   \
			}                                                           \
			cr_assert_eq(1, found);                                     \
		}                                                                   \
		cr_assert_eq(0, sp_stack_destroy(s, NULL));                         \
                                                                                    \
		s = sp_stack_create(sizeof(T), 10);                                 \
		cr_assert_not_null(s);                                              \
		for (i = 0; i < 8; i++) {                                           \
			cr_assert_eq(0, F2(s, d[i]));                               \
		}                                                                   \
		cr_assert_eq(d[4], F1(s, 3));                                       \
		cr_assert_eq(d[3], F3(s, 3));                                       \
		cr_assert_eq(d[6], F1(s, 0));                                       \
		cr_assert_eq(d[5], F3(s, 0));                                       \
		cr_assert_eq(d[2], F1(s, 3));                                       \
		cr_assert_eq(d[1], F3(s, 3));                                       \
		cr_assert_eq(0, sp_stack_destroy(s, NULL));                         \
		s = sp_stack_create(sizeof(T) + 1, 1000);                           \
		cr_assert_not_null(s);                                              \
		s->size = 1;                                                        \
		cr_assert_eq(z, F1(s, 0));                                          \
		cr_assert_eq(0, sp_stack_destroy(s, NULL));                         \
	} while (0)
	test(char          , sp_stack_qremovec , sp_stack_pushc , sp_stack_getc , IRANGE(1, CHAR_MAX) , "%hd");
	test(short         , sp_stack_qremoves , sp_stack_pushs , sp_stack_gets , IRANGE(1, SHRT_MAX) , "%hd");
	test(int           , sp_stack_qremovei , sp_stack_pushi , sp_stack_geti , FRANGE(1, INT_MAX)  , "%d");
	test(long          , sp_stack_qremovel , sp_stack_pushl , sp_stack_getl , FRANGE(1, LONG_MAX) , "%ld");
	test(signed char   , sp_stack_qremovesc, sp_stack_pushsc, sp_stack_getsc, IRANGE(1, SCHAR_MAX), "%hd");
	test(unsigned char , sp_stack_qremoveuc, sp_stack_pushuc, sp_stack_getuc, IRANGE(1, UCHAR_MAX), "%hd");
	test(unsigned short, sp_stack_qremoveus, sp_stack_pushus, sp_stack_getus, IRANGE(1, USHRT_MAX), "%hu");
	test(unsigned int  , sp_stack_qremoveui, sp_stack_pushui, sp_stack_getui, FRANGE(1, UINT_MAX) , "%u");
	test(unsigned long , sp_stack_qremoveul, sp_stack_pushul, sp_stack_getul, FRANGE(1, ULONG_MAX), "%lu");
	test(float         , sp_stack_qremovef , sp_stack_pushf , sp_stack_getf , FRANGE(1, FLT_MAX)  , "%f");
	test(double        , sp_stack_qremoved , sp_stack_pushd , sp_stack_getd , FRANGE(1, DBL_MAX)  , "%f");
	test(long double   , sp_stack_qremoveld, sp_stack_pushld, sp_stack_getld, FRANGE(1, LDBL_MAX) , "%Lf");
#undef test

	{ /* Suffixed form - strings */
		char a[25] = "test string";
		char d[100][25];
		s = sp_stack_create(sizeof(char*), 1000);
		cr_assert_not_null(s);
		cr_assert_eq(NULL, sp_stack_qremovestr(s, 0));
		cr_assert_eq(0, sp_stack_pushstr(s, a));
		cr_assert_eq(NULL, sp_stack_qremovestr(NULL, 0));
		cr_assert_eq(NULL, sp_stack_qremovestr(s, 1));
		cr_assert_eq(1LU, (unsigned long)s->size);
		cr_assert_eq(0, strcmp(a, sp_stack_qremovestr(s, 0)));
		cr_assert_eq(0LU, (unsigned long)s->size);
		cr_assert_eq(0, sp_stack_destroy(s, NULL));

		s = sp_stack_create(sizeof(char*), 100);
		cr_assert_not_null(s);
		for (i = 0; i < 100; i++) {
			a[0] = IRANGE(' ', '~');
			strcpy(d[i], a);
			cr_assert_eq(0, sp_stack_pushstr(s, d[i]));
		}
		for (i = 0; i < 100; i++) {
			char *a;
			size_t idx = IRANGE(0, s->size - 1), j;
			int found = 0;
			cr_assert_neq(NULL, (a = sp_stack_qremovestr(s, idx)));
			for (j = 0; j < 100; j++) {
				/* Lazy and slow way to check, but on average
				 * it's enough */
				if (strcmp(a, d[j]) == 0) {
					found = 1;
					break;
				}
			}
			cr_assert_eq(1, found);
			free(a);
		}
		cr_assert_eq(0, sp_stack_destroy(s, sp_free));

		s = sp_stack_create(sizeof(char*), 10);
		cr_assert_not_null(s);
		for (i = 0; i < 8; i++) {
			cr_assert_eq(0, sp_stack_pushstr(s, d[i]));
		}
		cr_assert_eq(0, strcmp(d[4], sp_stack_qremovestr(s, 3)));
		cr_assert_eq(0, strcmp(d[3], sp_stack_getstr(s, 3)));
		cr_assert_eq(0, strcmp(d[6], sp_stack_qremovestr(s, 0)));
		cr_assert_eq(0, strcmp(d[5], sp_stack_getstr(s, 0)));
		cr_assert_eq(0, strcmp(d[2], sp_stack_qremovestr(s, 3)));
		cr_assert_eq(0, strcmp(d[1], sp_stack_getstr(s, 3)));
		cr_assert_eq(0, sp_stack_destroy(s, sp_free));
		s = sp_stack_create(sizeof(char*) + 1, 1000);
		cr_assert_not_null(s);
		s->size = 1;
		cr_assert_eq(NULL, sp_stack_qremovestr(s, 0));
		cr_assert_eq(0, sp_stack_destroy(s, NULL));
	}
}

Test(stack, get)
{
	struct sp_stack *s;

	{ /* Generic form */
		unsigned i;
		int a = 0;
		struct data d[1000];
		s = sp_stack_create(sizeof(int), 1000);
		cr_assert_not_null(s);
		cr_assert_null(sp_stack_get(s, 0));
		cr_assert_eq(0, sp_stack_pushi(s, 10));
		cr_assert_null(sp_stack_get(NULL, 0));
		cr_assert_null(sp_stack_get(s, 1));
		a = *(int*)sp_stack_get(s, 0);
		cr_assert_eq(10, a);
		cr_assert_eq(0, sp_stack_destroy(s, NULL));
		s = sp_stack_create(sizeof(struct data), 1000);
		cr_assert_not_null(s);
		for (i = 0; i < 1000; i++) {
			cr_assert_eq(0, data_init(d + i));
			cr_assert_eq(0, sp_stack_push(s, d + i));
		}
		for (i = 0; i < 1000; i++) {
			struct data a;
			a = *(struct data*)sp_stack_get(s, i);
			cr_assert_eq(0, data_cmp(&a, d + 999 - i));
		}
		cr_assert_eq(0, sp_stack_destroy(s, data_dtor));
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
		s = sp_stack_create(sizeof(T), 1000);          \
		cr_assert_not_null(s);                         \
		cr_assert_eq(z, F1(s, 0));                     \
		cr_assert_eq(0, F2(s, a));                     \
		cr_assert_eq(z, F1(NULL, 0));                  \
		cr_assert_eq(z, F1(s, 1));                     \
		cr_assert_eq(a, F1(s, 0));                     \
		cr_assert_eq(0, sp_stack_destroy(s, NULL));    \
		s = sp_stack_create(sizeof(T), 1000);          \
		cr_assert_not_null(s);                         \
		for (i = 0; i < 1000; i++) {                   \
			d[i] = (V);                            \
			cr_assert_eq(0, F2(s, d[i]));          \
		}                                              \
		for (i = 0; i < 1000; i++) {                   \
			cr_assert_eq(d[999 - i], F1(s, i));    \
		}                                              \
		cr_assert_eq(0, sp_stack_destroy(s, NULL));    \
	} while (0)
	test(char          , sp_stack_getc , sp_stack_pushc , IRANGE(1, CHAR_MAX) , "%hd");
	test(short         , sp_stack_gets , sp_stack_pushs , IRANGE(1, SHRT_MAX) , "%hd");
	test(int           , sp_stack_geti , sp_stack_pushi , FRANGE(1, INT_MAX)  , "%d");
	test(long          , sp_stack_getl , sp_stack_pushl , FRANGE(1, LONG_MAX) , "%ld");
	test(signed char   , sp_stack_getsc, sp_stack_pushsc, IRANGE(1, SCHAR_MAX), "%hd");
	test(unsigned char , sp_stack_getuc, sp_stack_pushuc, IRANGE(1, UCHAR_MAX), "%hd");
	test(unsigned short, sp_stack_getus, sp_stack_pushus, IRANGE(1, USHRT_MAX), "%hu");
	test(unsigned int  , sp_stack_getui, sp_stack_pushui, FRANGE(1, UINT_MAX) , "%u");
	test(unsigned long , sp_stack_getul, sp_stack_pushul, FRANGE(1, ULONG_MAX), "%lu");
	test(float         , sp_stack_getf , sp_stack_pushf , FRANGE(1, FLT_MAX)  , "%f");
	test(double        , sp_stack_getd , sp_stack_pushd , FRANGE(1, DBL_MAX)  , "%f");
	test(long double   , sp_stack_getld, sp_stack_pushld, FRANGE(1, LDBL_MAX) , "%Lf");
#undef test

	{ /* Suffixed form - strings */
		unsigned i;
		char a[25] = " test string";
		char d[1000][25];
		s = sp_stack_create(sizeof(char*), 1000);
		cr_assert_not_null(s);
		cr_assert_eq(NULL, sp_stack_getstr(s, 0));
		cr_assert_eq(0, sp_stack_pushstr(s, a));
		cr_assert_eq(NULL, sp_stack_getstr(NULL, 0));
		cr_assert_eq(NULL, sp_stack_getstr(s, 1));
		cr_assert_eq(0, strcmp(a, sp_stack_getstr(s, 0)));
		cr_assert_eq(0, sp_stack_destroy(s, NULL));
		s = sp_stack_create(sizeof(char*), 1000);
		cr_assert_not_null(s);
		for (i = 0; i < 1000; i++) {
			a[0] = IRANGE(' ', '~');
			strcpy(d[i], a);
			cr_assert_eq(0, sp_stack_pushstr(s, d[i]));
		}
		for (i = 0; i < 1000; i++) {
			cr_assert_eq(0, strcmp(d[999 - i], sp_stack_getstr(s, i)));
		}
		cr_assert_eq(0, sp_stack_destroy(s, sp_free));
	}
}

Test(stack, set)
{
	struct sp_stack *s;

	{ /* Generic form */
		unsigned i;
		int a = 1;
		s = sp_stack_create(sizeof(int), 1000);
		cr_assert_not_null(s);
		cr_assert_eq(SP_EINDEX, sp_stack_set(s, 0, &a));
		cr_assert_eq(0, sp_stack_pushi(s, 10));
		cr_assert_eq(SP_EINVAL, sp_stack_set(NULL, 0, &a));
		cr_assert_eq(SP_EINVAL, sp_stack_set(s, 0, NULL));
		cr_assert_eq(SP_EINVAL, sp_stack_set(NULL, 0, NULL));
		cr_assert_eq(SP_EINDEX, sp_stack_set(s, 1, &a));
		cr_assert_eq(0, sp_stack_set(s, 0, &a));
		cr_assert_eq(a, sp_stack_geti(s, 0));
		cr_assert_eq(0, sp_stack_destroy(s, NULL));
		s = sp_stack_create(sizeof(struct data), 1000);
		cr_assert_not_null(s);
		for (i = 0; i < 1000; i++) {
			struct data d;
			cr_assert_eq(0, data_init(&d));
			cr_assert_eq(0, sp_stack_push(s, &d));
		}
		for (i = 0; i < 1000; i++) {
			struct data a, b;
			cr_assert_eq(0, data_init(&b));
			a = *(struct data*)sp_stack_get(s, i);
			cr_assert_eq(0, data_dtor(&a));
			cr_assert_eq(0, sp_stack_set(s, i, &b));
			a = *(struct data*)sp_stack_get(s, i);
			cr_assert_eq(0, data_cmp(&a, &b));
		}
		cr_assert_eq(0, sp_stack_destroy(s, data_dtor));
	}

	/* Suffixed form
	 * T  - type
	 * F1 - set function
	 * F2 - push function
	 * F3 - get function
	 * V  - random value snippet
	 * M  - printf format string
	 */
#define test(T, F1, F2, F3, V, M) do {                      \
		unsigned i;                                 \
		T a = (V);                                  \
		s = sp_stack_create(sizeof(T), 1000);       \
		cr_assert_not_null(s);                      \
		cr_assert_eq(SP_EINDEX, F1(s, 0, a));       \
		cr_assert_eq(0, F2(s, (V)));                \
		cr_assert_eq(SP_EINVAL, F1(NULL, 0, a));    \
		cr_assert_eq(SP_EINDEX, F1(s, 1, a));       \
		cr_assert_eq(0, F1(s, 0, a));               \
		cr_assert_eq(a, F3(s, 0));                  \
		cr_assert_eq(0, sp_stack_clear(s, NULL));   \
		for (i = 0; i < 1000; i++) {                \
			cr_assert_eq(0, F2(s, (V)));        \
		}                                           \
		for (i = 0; i < 1000; i++) {                \
			T b = (V);                          \
			cr_assert_eq(0, F1(s, i, b));       \
			cr_assert_eq(b, F3(s, i));          \
		}                                           \
		cr_assert_eq(0, sp_stack_destroy(s, NULL)); \
		s = sp_stack_create(sizeof(T) + 1, 1000);   \
		cr_assert_not_null(s);                      \
		s->size = 1;                                \
		cr_assert_eq(SP_EILLEGAL, F1(s, 0, (V)));   \
		cr_assert_eq(0, sp_stack_destroy(s, NULL)); \
	} while (0)
	test(char          , sp_stack_setc , sp_stack_pushc , sp_stack_getc , IRANGE(1, CHAR_MAX) , "%hd");
	test(short         , sp_stack_sets , sp_stack_pushs , sp_stack_gets , IRANGE(1, SHRT_MAX) , "%hd");
	test(int           , sp_stack_seti , sp_stack_pushi , sp_stack_geti , FRANGE(1, INT_MAX)  , "%d");
	test(long          , sp_stack_setl , sp_stack_pushl , sp_stack_getl , FRANGE(1, LONG_MAX) , "%ld");
	test(signed char   , sp_stack_setsc, sp_stack_pushsc, sp_stack_getsc, IRANGE(1, SCHAR_MAX), "%hd");
	test(unsigned char , sp_stack_setuc, sp_stack_pushuc, sp_stack_getuc, IRANGE(1, UCHAR_MAX), "%hd");
	test(unsigned short, sp_stack_setus, sp_stack_pushus, sp_stack_getus, IRANGE(1, USHRT_MAX), "%hu");
	test(unsigned int  , sp_stack_setui, sp_stack_pushui, sp_stack_getui, FRANGE(1, UINT_MAX) , "%u");
	test(unsigned long , sp_stack_setul, sp_stack_pushul, sp_stack_getul, FRANGE(1, ULONG_MAX), "%lu");
	test(float         , sp_stack_setf , sp_stack_pushf , sp_stack_getf , FRANGE(1, FLT_MAX)  , "%f");
	test(double        , sp_stack_setd , sp_stack_pushd , sp_stack_getd , FRANGE(1, DBL_MAX)  , "%f");
	test(long double   , sp_stack_setld, sp_stack_pushld, sp_stack_getld, FRANGE(1, LDBL_MAX) , "%Lf");
#undef test

	{ /* Suffixed form - strings */
		unsigned i;
		s = sp_stack_create(sizeof(char*), 1000);
		cr_assert_not_null(s);
		cr_assert_eq(SP_EINDEX, sp_stack_setstr(s, 0, "abc"));
		cr_assert_eq(0, sp_stack_pushstr(s, "abc"));
		cr_assert_eq(SP_EINVAL, sp_stack_setstr(NULL, 0, "def"));
		cr_assert_eq(SP_EINDEX, sp_stack_setstr(s, 1, "def"));
		cr_assert_eq(0, sp_stack_setstr(s, 0, "xyz"));
		cr_assert_eq(0, strcmp("xyz", sp_stack_getstr(s, 0)));
		cr_assert_eq(0, sp_stack_clear(s, sp_free));
		for (i = 0; i < 1000; i++)
			cr_assert_eq(0, sp_stack_pushstr(s, "test string"));
		for (i = 0; i < 1000; i++) {
			const char *b = "another string";
			cr_assert_eq(0, sp_stack_setstr(s, i, b));
			cr_assert_eq(0, strcmp(b, sp_stack_getstr(s, i)));
		}
		cr_assert_eq(0, sp_stack_destroy(s, sp_free));
		s = sp_stack_create(sizeof(char*) + 1, 1000);
		cr_assert_not_null(s);
		s->size = 1;
		cr_assert_eq(SP_EILLEGAL, sp_stack_setstr(s, 0, "test"));
		cr_assert_eq(0, sp_stack_destroy(s, NULL));

		/* substrings (strn) */
		s = sp_stack_create(sizeof(char*), 1000);
		cr_assert_not_null(s);
		cr_assert_eq(SP_EINDEX, sp_stack_setstrn(s, 0, "abcdef", 3));
		cr_assert_eq(0, sp_stack_pushstr(s, "abc"));
		cr_assert_eq(SP_EINVAL, sp_stack_setstrn(NULL, 0, "defghi", 3));
		cr_assert_eq(SP_EINDEX, sp_stack_setstrn(s, 1, "defghi", 3));
		cr_assert_eq(0, sp_stack_setstrn(s, 0, "xyzabc", 3));
		cr_assert_eq(0, strcmp("xyz", sp_stack_getstr(s, 0)));
		cr_assert_eq(0, sp_stack_setstrn(s, 0, "testing", 0));
		cr_assert_eq(0, strcmp("", sp_stack_getstr(s, 0)));
		cr_assert_eq(0, sp_stack_clear(s, sp_free));
		for (i = 0; i < 1000; i++)
			cr_assert_eq(0, sp_stack_pushstr(s, "test string"));
		for (i = 0; i < 1000; i++) {
			cr_assert_eq(0, sp_stack_setstrn(s, i, "another string", 7));
			cr_assert_eq(0, strcmp("another", sp_stack_getstr(s, i)));
		}
		cr_assert_eq(0, sp_stack_destroy(s, sp_free));
		s = sp_stack_create(sizeof(char*) + 1, 1000);
		cr_assert_not_null(s);
		s->size = 1;
		cr_assert_eq(SP_EILLEGAL, sp_stack_setstrn(s, 0, "test", 0));
		cr_assert_eq(0, sp_stack_destroy(s, NULL));
	}
}

Test(stack, print)
{
	struct sp_stack *s;

	{ /* Generic form */
		struct data a, b;
		cr_assert_eq(0, data_init(&a));
		cr_assert_eq(0, data_init(&b));
		s = sp_stack_create(sizeof(struct data), 30);
		cr_assert_eq(0, sp_stack_push(s, &a));
		cr_assert_eq(0, sp_stack_push(s, &b));
		cr_assert_eq(SP_EINVAL, sp_stack_print(NULL, NULL));
		cr_assert_eq(SP_EHANDLER, sp_stack_print(s, data_print_bad));
		cr_assert_eq(0, sp_stack_print(s, NULL));
		cr_assert_eq(0, sp_stack_print(s, data_print));
		cr_assert_eq(0, sp_stack_destroy(s, data_dtor));
	}

	/* Suffixed form
	 * T  - type
	 * F1 - push function
	 * F2 - print function
	 * A  - 1st value
	 * B  - 2nd value
	 */
#define test(T, F1, F2, A, B)                               \
	do {                                                \
		T a = A, b = B;                             \
		s = sp_stack_create(sizeof(T), 30);         \
		cr_assert_eq(0, F1(s, a));                  \
		cr_assert_eq(0, F1(s, b));                  \
		cr_assert_eq(SP_EINVAL, F2(NULL));          \
		cr_assert_eq(0, F2(s));                     \
		cr_assert_eq(0, sp_stack_destroy(s, NULL)); \
		s = sp_stack_create(sizeof(T) + 1, 30);     \
		cr_assert_eq(SP_EILLEGAL, F2(s));           \
		cr_assert_eq(0, sp_stack_destroy(s, NULL)); \
	} while(0)

	test(char          , sp_stack_pushc , sp_stack_printc , 'A', 'B');
	test(short         , sp_stack_pushs , sp_stack_prints , SHRT_MIN, SHRT_MAX);
	test(int           , sp_stack_pushi , sp_stack_printi , INT_MIN, INT_MAX);
	test(long          , sp_stack_pushl , sp_stack_printl , LONG_MIN, LONG_MAX);
	test(signed char   , sp_stack_pushsc, sp_stack_printsc, SCHAR_MIN, SCHAR_MAX);
	test(unsigned char , sp_stack_pushuc, sp_stack_printuc, 0, UCHAR_MAX);
	test(unsigned short, sp_stack_pushus, sp_stack_printus, 0, USHRT_MAX);
	test(unsigned int  , sp_stack_pushui, sp_stack_printui, 0, UINT_MAX);
	test(unsigned long , sp_stack_pushul, sp_stack_printul, 0, ULONG_MAX);
	test(float         , sp_stack_pushf , sp_stack_printf , FLT_MIN, FLT_MAX);
	test(double        , sp_stack_pushd , sp_stack_printd , DBL_MIN, DBL_MAX);
	test(long double   , sp_stack_pushld, sp_stack_printld, LDBL_MIN, LDBL_MAX);

	{ /* Suffixed form - strings */
		const char *a = "test string #1", *b = "test string #2";
		s = sp_stack_create(sizeof(char*), 30);
		cr_assert_eq(0, sp_stack_pushstr(s, a));
		cr_assert_eq(0, sp_stack_pushstr(s, b));
		cr_assert_eq(SP_EINVAL, sp_stack_printstr(NULL));
		cr_assert_eq(0, sp_stack_printstr(s));
		cr_assert_eq(0, sp_stack_destroy(s, NULL));
		s = sp_stack_create(sizeof(char*) + 1, 30);
		cr_assert_eq(SP_EILLEGAL, sp_stack_printstr(s));
		cr_assert_eq(0, sp_stack_destroy(s, NULL));
	}
}
