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
#include "../../src/sp_queue.h"
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

TestSuite(queue, .init=setup);

Test(queue, create)
{
	struct sp_queue *q;
	q = sp_queue_create(sizeof(int), 0);
	cr_assert_null(q);
	q = sp_queue_create(0, 16);
	cr_assert_null(q);
	q = sp_queue_create(0, 0);
	cr_assert_null(q);
	q = sp_queue_create(SIZE_MAX, SIZE_MAX);
	cr_assert_null(q);
	q = sp_queue_create(sizeof(int), 16);
	cr_assert_not_null(q);
	cr_assert_eq(0, sp_queue_destroy(q, NULL));
}

Test(queue, destroy)
{
	struct sp_queue *q;
	unsigned i;
	q = sp_queue_create(sizeof(long double), 1000);
	cr_assert_not_null(q);
	cr_assert_eq(SP_EINVAL, sp_queue_destroy(NULL, NULL));
	cr_assert_eq(0, sp_queue_destroy(q, NULL));
	q = sp_queue_create(sizeof(struct data), 1000);
	for (i = 0; i < 1000; i++) {
		struct data d;
		cr_assert_eq(0, data_init(&d));
		cr_assert_eq(0, sp_queue_push(q, &d));
	}
	cr_assert_eq(SP_EHANDLER, sp_queue_destroy(q, data_dtor_bad));
	cr_assert_eq(0, sp_queue_destroy(q, data_dtor));
}

Test(queue, push)
{
	struct sp_queue *q;

	{ /* Generic form */
		unsigned i;
		struct data d;
		q = sp_queue_create(sizeof(struct data), 1000);
		cr_assert_not_null(q);
		data_init(&d);
		cr_assert_eq(SP_EINVAL, sp_queue_push(q, NULL));
		cr_assert_eq(SP_EINVAL, sp_queue_push(NULL, &d));
		cr_assert_eq(SP_EINVAL, sp_queue_push(NULL, NULL));
		cr_assert_eq(q->head, q->data, "%p");
		cr_assert_eq(q->tail, q->data, "%p");
		cr_assert_eq(0, sp_queue_push(q, &d));
		cr_assert_eq(q->head, q->data, "%p");
		cr_assert_eq(q->tail, q->data, "%p");
		cr_assert_eq(0, sp_queue_push(q, &d));
		cr_assert_eq(q->head, q->data, "%p");
		cr_assert_eq(q->tail, (char*)q->data + q->elem_size, "%p");
		cr_assert_eq(0, sp_queue_clear(q, NULL));
		for (i = 0; i < SIZE_MAX / sizeof(struct data); i++) {
			struct data a, b;
			cr_assert_eq(0, data_init(&a));
			cr_assert_eq(0, sp_queue_push(q, &a));
			cr_assert_eq(0, sp_queue_get(q, i, &b));
			cr_assert_eq(0, data_cmp(&a, &b));
		}
		cr_assert_eq(SP_ERANGE, sp_queue_push(q, &d));
		cr_assert_eq(0, data_dtor(&d));
		cr_assert_eq(0, sp_queue_destroy(q, data_dtor));
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
		q = sp_queue_create(sizeof(T), 1000);                      \
		cr_assert_not_null(q);                                      \
		cr_assert_eq(SP_EINVAL, F1(NULL, (V)));                    \
		cr_assert_eq(q->head, q->data, "%p");                       \
		cr_assert_eq(q->tail, q->data, "%p");                       \
		cr_assert_eq(0, F1(q, (V)));                                \
		cr_assert_eq(q->head, q->data, "%p");                       \
		cr_assert_eq(q->tail, q->data, "%p");                       \
		cr_assert_eq(0, F1(q, (V)));                                \
		cr_assert_eq(q->head, q->data, "%p");                       \
		cr_assert_eq(q->tail, (char*)q->data + q->elem_size, "%p"); \
		cr_assert_eq(0, sp_queue_clear(q, NULL));                  \
		for (i = 0; i < SIZE_MAX / sizeof(T); i++) {                \
			T a = (V);                                          \
			cr_assert_eq(0, F1(q, a));                          \
			cr_assert_eq(a, F2(q, i), M);                       \
		}                                                           \
		cr_assert_eq(SP_ERANGE, F1(q, (V)));                       \
		cr_assert_eq(0, sp_queue_destroy(q, NULL));                \
		q = sp_queue_create(sizeof(T) + 1, 1);                     \
		cr_assert_not_null(q);                                      \
		cr_assert_eq(SP_EILLEGAL, F1(q, (V)));                     \
		cr_assert_eq(0, sp_queue_destroy(q, NULL));                \
	} while (0)
	test(char          , sp_queue_pushc , sp_queue_getc , IRANGE(CHAR_MIN , CHAR_MAX) , "%hd");
	test(short         , sp_queue_pushs , sp_queue_gets , IRANGE(SHRT_MIN , SHRT_MAX) , "%hd");
	test(int           , sp_queue_pushi , sp_queue_geti , FRANGE(INT_MIN  , INT_MAX)  , "%d");
	test(long          , sp_queue_pushl , sp_queue_getl , FRANGE(LONG_MIN , LONG_MAX) , "%ld");
	test(signed char   , sp_queue_pushsc, sp_queue_getsc, IRANGE(SCHAR_MIN, SCHAR_MAX), "%hd");
	test(unsigned char , sp_queue_pushuc, sp_queue_getuc, IRANGE(0        , UCHAR_MAX), "%hd");
	test(unsigned short, sp_queue_pushus, sp_queue_getus, IRANGE(0        , USHRT_MAX), "%hu");
	test(unsigned int  , sp_queue_pushui, sp_queue_getui, FRANGE(0        , UINT_MAX) , "%u");
	test(unsigned long , sp_queue_pushul, sp_queue_getul, FRANGE(0        , ULONG_MAX), "%lu");
	test(float         , sp_queue_pushf , sp_queue_getf , FRANGE(FLT_MIN  , FLT_MAX)  , "%f");
	test(double        , sp_queue_pushd , sp_queue_getd , FRANGE(DBL_MIN  , DBL_MAX)  , "%f");
	test(long double   , sp_queue_pushld, sp_queue_getld, FRANGE(LDBL_MIN , LDBL_MAX) , "%Lf");
#undef test
}

Test(queue, peek)
{
	struct sp_queue *q;

	{ /* Generic form */
		struct data a, b;
		q = sp_queue_create(sizeof(struct data), 2);
		cr_assert_not_null(q);
		cr_assert_eq(SP_EILLEGAL, sp_queue_peek(q, &b));
		cr_assert_eq(SP_EINVAL, sp_queue_peek(q, NULL));
		cr_assert_eq(SP_EINVAL, sp_queue_peek(NULL, &b));
		cr_assert_eq(SP_EINVAL, sp_queue_peek(NULL, NULL));
		data_init(&a);
		cr_assert_eq(0, sp_queue_push(q, &a));
		cr_assert_eq(0, sp_queue_peek(q, &b));
		cr_assert_eq(0, data_cmp(&a, &b));
		cr_assert_eq(0, sp_queue_destroy(q, data_dtor));
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
		q = sp_queue_create(sizeof(T), 2);          \
		cr_assert_not_null(q);                       \
		cr_assert_eq(z, F1(q), M);                   \
		cr_assert_eq(z, F1(NULL), M);                \
		cr_assert_eq(0, F2(q, a));                   \
		cr_assert_eq(a, F1(q), M);                   \
		cr_assert_eq(0, sp_queue_destroy(q, NULL)); \
		q = sp_queue_create(sizeof(T) + 1, 1);      \
		cr_assert_not_null(q);                       \
		q->size = 1;                                 \
		cr_assert_eq(z, F1(q), M);                   \
		cr_assert_eq(0, sp_queue_destroy(q, NULL)); \
	} while (0)
	test(char          , sp_queue_peekc , sp_queue_pushc , IRANGE(1, CHAR_MAX) , "%hd");
	test(short         , sp_queue_peeks , sp_queue_pushs , IRANGE(1, SHRT_MAX) , "%hd");
	test(int           , sp_queue_peeki , sp_queue_pushi , FRANGE(1, INT_MAX)  , "%d");
	test(long          , sp_queue_peekl , sp_queue_pushl , FRANGE(1, LONG_MAX) , "%ld");
	test(signed char   , sp_queue_peeksc, sp_queue_pushsc, IRANGE(1, SCHAR_MAX), "%hd");
	test(unsigned char , sp_queue_peekuc, sp_queue_pushuc, IRANGE(1, UCHAR_MAX), "%hd");
	test(unsigned short, sp_queue_peekus, sp_queue_pushus, IRANGE(1, USHRT_MAX), "%hu");
	test(unsigned int  , sp_queue_peekui, sp_queue_pushui, FRANGE(1, UINT_MAX) , "%u");
	test(unsigned long , sp_queue_peekul, sp_queue_pushul, FRANGE(1, ULONG_MAX), "%lu");
	test(float         , sp_queue_peekf , sp_queue_pushf , FRANGE(1, FLT_MAX)  , "%f");
	test(double        , sp_queue_peekd , sp_queue_pushd , FRANGE(1, DBL_MAX)  , "%f");
	test(long double   , sp_queue_peekld, sp_queue_pushld, FRANGE(1, LDBL_MAX) , "%Lf");
#undef test
}

Test(queue, pop)
{
	struct sp_queue *q;

	{ /* Generic form */
		struct data a, b;
		q = sp_queue_create(sizeof(struct data), 2);
		cr_assert_not_null(q);
		cr_assert_eq(SP_EILLEGAL, sp_queue_pop(q, NULL));
		cr_assert_eq(SP_EINVAL, sp_queue_pop(NULL, NULL));
		data_init(&a);
		cr_assert_eq(0, sp_queue_push(q, &a));
		cr_assert_eq(q->head, q->data, "%p");
		cr_assert_eq(q->tail, q->data, "%p");
		cr_assert_eq(0, sp_queue_pop(q, &b));
		cr_assert_eq(q->head, q->data, "%p");
		cr_assert_eq(q->tail, q->data, "%p");
		cr_assert_eq(0, data_cmp(&a, &b));
		cr_assert_eq(0, sp_queue_destroy(q, NULL));
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
		q = sp_queue_create(sizeof(T), 2);          \
		cr_assert_not_null(q);                       \
		cr_assert_eq(z, F1(q), M);                   \
		cr_assert_eq(z, F1(NULL), M);                \
		cr_assert_eq(0, F2(q, a));                   \
		cr_assert_eq(q->head, q->data, "%p");        \
		cr_assert_eq(q->tail, q->data, "%p");        \
		cr_assert_eq(a, F1(q), M);                   \
		cr_assert_eq(q->head, q->data, "%p");        \
		cr_assert_eq(q->tail, q->data, "%p");        \
		cr_assert_eq(0, sp_queue_destroy(q, NULL)); \
		q = sp_queue_create(sizeof(T) + 1, 1);      \
		cr_assert_not_null(q);                       \
		q->size = 1;                                 \
		cr_assert_eq(z, F1(q), M);                   \
		cr_assert_eq(0, sp_queue_destroy(q, NULL)); \
	} while (0)
	test(char          , sp_queue_popc , sp_queue_pushc , IRANGE(1, CHAR_MAX) , "%hd");
	test(short         , sp_queue_pops , sp_queue_pushs , IRANGE(1, SHRT_MAX) , "%hd");
	test(int           , sp_queue_popi , sp_queue_pushi , FRANGE(1, INT_MAX)  , "%d");
	test(long          , sp_queue_popl , sp_queue_pushl , FRANGE(1, LONG_MAX) , "%ld");
	test(signed char   , sp_queue_popsc, sp_queue_pushsc, IRANGE(1, SCHAR_MAX), "%hd");
	test(unsigned char , sp_queue_popuc, sp_queue_pushuc, IRANGE(1, UCHAR_MAX), "%hd");
	test(unsigned short, sp_queue_popus, sp_queue_pushus, IRANGE(1, USHRT_MAX), "%hu");
	test(unsigned int  , sp_queue_popui, sp_queue_pushui, FRANGE(1, UINT_MAX) , "%u");
	test(unsigned long , sp_queue_popul, sp_queue_pushul, FRANGE(1, ULONG_MAX), "%lu");
	test(float         , sp_queue_popf , sp_queue_pushf , FRANGE(1, FLT_MAX)  , "%f");
	test(double        , sp_queue_popd , sp_queue_pushd , FRANGE(1, DBL_MAX)  , "%f");
	test(long double   , sp_queue_popld, sp_queue_pushld, FRANGE(1, LDBL_MAX) , "%Lf");
#undef test
}

Test(queue, clear)
{
	struct sp_queue *q;
	unsigned i;
	q = sp_queue_create(sizeof(long double), 1000);
	cr_assert_not_null(q);
	cr_assert_eq(SP_EINVAL, sp_queue_clear(NULL, NULL));
	cr_assert_eq(0, sp_queue_clear(q, NULL));
	cr_assert_eq(0LU, (unsigned long)q->size, "%lu");
	cr_assert_eq(0, sp_queue_destroy(q, NULL));
	q = sp_queue_create(sizeof(struct data), 1000);
	for (i = 0; i < 1000; i++) {
		struct data d;
		cr_assert_eq(0, data_init(&d));
		cr_assert_eq(0, sp_queue_push(q, &d));
	}
	cr_assert_eq(SP_EHANDLER, sp_queue_clear(q, data_dtor_bad));
	cr_assert_eq(0, sp_queue_clear(q, data_dtor));
	cr_assert_eq(0LU, (unsigned long)q->size, "%lu");
	cr_assert_eq(0, sp_queue_destroy(q, NULL));
}

Test(queue, foreach)
{
	struct sp_queue *q;
	unsigned i;
	q = sp_queue_create(sizeof(long double), 1000);
	cr_assert_not_null(q);
	cr_assert_eq(SP_EINVAL, sp_queue_foreach(NULL, data_mutate));
	cr_assert_eq(SP_EINVAL, sp_queue_foreach(q, NULL));
	cr_assert_eq(SP_EINVAL, sp_queue_foreach(NULL, NULL));
	cr_assert_eq(0, sp_queue_destroy(q, NULL));
	q = sp_queue_create(sizeof(struct data), 1000);
	for (i = 0; i < 1000; i++) {
		struct data d;
		cr_assert_eq(0, data_init(&d));
		cr_assert_eq(0, sp_queue_push(q, &d));
	}
	cr_assert_eq(SP_EHANDLER, sp_queue_foreach(q, data_mutate_bad));
	cr_assert_eq(0, sp_queue_foreach(q, data_mutate));
	cr_assert_eq(0, sp_queue_foreach(q, data_verify));
	cr_assert_eq(0, sp_queue_destroy(q, data_dtor));
}

Test(queue, copy)
{
	struct sp_queue *q, *p;
	unsigned i;
	q = sp_queue_create(sizeof(int), 1000);
	p = sp_queue_create(sizeof(int), 333);
	cr_assert_not_null(q);
	cr_assert_eq(SP_EINVAL, sp_queue_copy(NULL, q, NULL));
	cr_assert_eq(SP_EINVAL, sp_queue_copy(p, NULL, NULL));
	cr_assert_eq(SP_EINVAL, sp_queue_copy(NULL, NULL, NULL));
	cr_assert_eq(0, sp_queue_copy(q, p, NULL));
	cr_assert_eq((unsigned long)p->size, (unsigned long)q->size, "%lu");
	cr_assert_eq((unsigned long)p->elem_size, (unsigned long)q->elem_size, "%lu");
	cr_assert_eq(0, sp_queue_copy(p, q, NULL));
	cr_assert_eq((unsigned long)p->size, (unsigned long)q->size, "%lu");
	cr_assert_eq((unsigned long)p->elem_size, (unsigned long)q->elem_size, "%lu");
	for (i = 0; i < 1000; i++) {
		cr_assert_eq(0, sp_queue_pushi(q, FRANGE(INT_MIN, INT_MAX)));
	}
	cr_assert_eq(0, sp_queue_copy(p, q, NULL));
	cr_assert_eq((unsigned long)p->size, (unsigned long)q->size, "%lu");
	cr_assert_eq((unsigned long)p->elem_size, (unsigned long)q->elem_size, "%lu");
	for (i = 0; i < 1000; i++) {
		int a, b;
		a = sp_queue_geti(q, i);
		b = sp_queue_geti(p, i);
		cr_assert_eq(a, b);
	}
	cr_assert_eq(0, sp_queue_destroy(q, NULL));
	cr_assert_eq(0, sp_queue_clear(p, NULL));
	q = sp_queue_create(sizeof(struct data), 1000);
	for (i = 0; i < 1000; i++) {
		struct data d;
		cr_assert_eq(0, data_init(&d));
		cr_assert_eq(0, sp_queue_push(q, &d));
	}
	cr_assert_eq(SP_EHANDLER, sp_queue_copy(p, q, data_cpy_bad));
	cr_assert_eq(0, sp_queue_copy(p, q, data_cpy));
	cr_assert_eq((unsigned long)p->size, (unsigned long)q->size, "%lu");
	cr_assert_eq((unsigned long)p->elem_size, (unsigned long)q->elem_size, "%lu");
	for (i = 0; i < 1000; i++) {
		struct data a, b;
		cr_assert_eq(0, sp_queue_get(q, i, &a));
		cr_assert_eq(0, sp_queue_get(p, i, &b));
		cr_assert_eq(0, data_cmp(&a, &b));
	}
	cr_assert_eq(0, sp_queue_destroy(q, data_dtor));
	cr_assert_eq(0, sp_queue_destroy(p, data_dtor));
}

Test(queue, insert)
{
	struct sp_queue *q;
	unsigned i;

	{ /* Generic form */
		int a = 10;
		struct data d[1000];
		q = sp_queue_create(sizeof(int), 1000);
		cr_assert_not_null(q);
		cr_assert_eq(SP_EINVAL, sp_queue_insert(q, 0, NULL));
		cr_assert_eq(SP_EINVAL, sp_queue_insert(NULL, 0, &a));
		cr_assert_eq(SP_EINVAL, sp_queue_insert(NULL, 0, NULL));
		cr_assert_eq(SP_EINDEX, sp_queue_insert(q, 1, &a));
		cr_assert_eq(0, sp_queue_insert(q, 0, &a));
		cr_assert_eq(1LU, (unsigned long)q->size, "%lu");
		cr_assert_eq(10, sp_queue_geti(q, 0));
		cr_assert_eq(0, sp_queue_destroy(q, NULL));

		q = sp_queue_create(sizeof(struct data), 1000);
		cr_assert_not_null(q);
		for (i = 0; i < 1000; i++) {
			struct data a;
			size_t idx = IRANGE(0, i);
			cr_assert_eq(0, data_init(d + i));
			cr_assert_eq(0, sp_queue_insert(q, idx, d + i));
			cr_assert_eq((unsigned long)i + 1, (unsigned long)q->size, "%lu");
			cr_assert_eq(0, sp_queue_get(q, idx, &a));
			cr_assert_eq(0, data_cmp(&a, d + i));
		}
		cr_assert_eq(0, sp_queue_destroy(q, data_dtor));

		q = sp_queue_create(sizeof(struct data), 10);
		cr_assert_not_null(q);
		for (i = 0; i < 5; i++) {
			cr_assert_eq(0, data_init(d + i));
			cr_assert_eq(0, sp_queue_insert(q, i, d + i));
		}
		for (i = 0; i < 5; i++) {
			struct data a;
			cr_assert_eq(0, sp_queue_get(q, i, &a));
			cr_assert_eq(0, data_cmp(&a, d + i));
		}
		cr_assert_eq(0, data_init(d + 5));
		cr_assert_eq(0, sp_queue_insert(q, 1, d + 5));
		cr_assert_eq(q->head, (char*)q->data + (q->capacity - 1) * q->elem_size, "%p");
		cr_assert_eq(0, data_init(d + 6));
		cr_assert_eq(0, sp_queue_insert(q, 3, d + 6));
		cr_assert_eq(q->tail, (char*)q->data + 5 * q->elem_size, "%p");
		cr_assert_eq(0, sp_queue_destroy(q, data_dtor));
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
		q = sp_queue_create(sizeof(T), 1000);                                          \
		cr_assert_not_null(q);                                                          \
		cr_assert_eq(SP_EINVAL, F1(NULL, 0, a));                                       \
		cr_assert_eq(SP_EINDEX, F1(q, 1, a));                                          \
		cr_assert_eq(0, F1(q, 0, a));                                                   \
		cr_assert_eq(1LU, (unsigned long)q->size, "%lu");                               \
		cr_assert_eq(a, F2(q, 0), M);                                                   \
		cr_assert_eq(0, sp_queue_destroy(q, NULL));                                    \
                                                                                                \
		q = sp_queue_create(sizeof(T), 1000);                                          \
		cr_assert_not_null(q);                                                          \
		for (i = 0; i < 1000; i++) {                                                    \
			size_t idx = IRANGE(0, i);                                              \
			d[i] = (V);                                                             \
			cr_assert_eq(0, F1(q, idx, d[i]));                                      \
			cr_assert_eq((unsigned long)i + 1, (unsigned long)q->size, "%lu");      \
			cr_assert_eq(d[i], F2(q, idx), M);                                      \
		}                                                                               \
		cr_assert_eq(0, sp_queue_destroy(q, NULL));                                    \
                                                                                                \
		q = sp_queue_create(sizeof(T), 10);                                            \
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
		cr_assert_eq(0, sp_queue_destroy(q, NULL));                                    \
		q = sp_queue_create(sizeof(T) + 1, 1000);                                      \
		cr_assert_not_null(q);                                                          \
		cr_assert_eq(SP_EILLEGAL, F1(q, 0, (V)));                                      \
		cr_assert_eq(0, sp_queue_destroy(q, NULL));                                    \
	} while (0)
	test(char          , sp_queue_insertc , sp_queue_getc , IRANGE(1, CHAR_MAX) , "%hd");
	test(short         , sp_queue_inserts , sp_queue_gets , IRANGE(1, SHRT_MAX) , "%hd");
	test(int           , sp_queue_inserti , sp_queue_geti , FRANGE(1, INT_MAX)  , "%d");
	test(long          , sp_queue_insertl , sp_queue_getl , FRANGE(1, LONG_MAX) , "%ld");
	test(signed char   , sp_queue_insertsc, sp_queue_getsc, IRANGE(1, SCHAR_MAX), "%hd");
	test(unsigned char , sp_queue_insertuc, sp_queue_getuc, IRANGE(1, UCHAR_MAX), "%hd");
	test(unsigned short, sp_queue_insertus, sp_queue_getus, IRANGE(1, USHRT_MAX), "%hu");
	test(unsigned int  , sp_queue_insertui, sp_queue_getui, FRANGE(1, UINT_MAX) , "%u");
	test(unsigned long , sp_queue_insertul, sp_queue_getul, FRANGE(1, ULONG_MAX), "%lu");
	test(float         , sp_queue_insertf , sp_queue_getf , FRANGE(1, FLT_MAX)  , "%f");
	test(double        , sp_queue_insertd , sp_queue_getd , FRANGE(1, DBL_MAX)  , "%f");
	test(long double   , sp_queue_insertld, sp_queue_getld, FRANGE(1, LDBL_MAX) , "%Lf");
#undef test
}

Test(queue, qinsert)
{
	struct sp_queue *q;
	unsigned i;

	{ /* Generic form */
		int a = 10;
		struct data d[1000];
		q = sp_queue_create(sizeof(int), 1000);
		cr_assert_not_null(q);
		cr_assert_eq(SP_EINVAL, sp_queue_qinsert(q, 0, NULL));
		cr_assert_eq(SP_EINVAL, sp_queue_qinsert(NULL, 0, &a));
		cr_assert_eq(SP_EINVAL, sp_queue_qinsert(NULL, 0, NULL));
		cr_assert_eq(SP_EINDEX, sp_queue_qinsert(q, 1, &a));
		cr_assert_eq(0, sp_queue_qinsert(q, 0, &a));
		cr_assert_eq(1LU, (unsigned long)q->size, "%lu");
		cr_assert_eq(10, sp_queue_geti(q, 0));
		cr_assert_eq(0, sp_queue_destroy(q, NULL));

		q = sp_queue_create(sizeof(struct data), 1000);
		cr_assert_not_null(q);
		for (i = 0; i < 1000; i++) {
			struct data a;
			size_t idx = IRANGE(0, i);
			cr_assert_eq(0, data_init(d + i));
			cr_assert_eq(0, sp_queue_qinsert(q, idx, d + i));
			cr_assert_eq((unsigned long)i + 1, (unsigned long)q->size, "%lu");
			cr_assert_eq(0, sp_queue_get(q, idx, &a));
			cr_assert_eq(0, data_cmp(&a, d + i));
		}
		cr_assert_eq(0, sp_queue_destroy(q, data_dtor));

		q = sp_queue_create(sizeof(struct data), 10);
		cr_assert_not_null(q);
		for (i = 0; i < 5; i++) {
			cr_assert_eq(0, data_init(d + i));
			cr_assert_eq(0, sp_queue_qinsert(q, i, d + i));
		}
		for (i = 0; i < 5; i++) {
			struct data a;
			cr_assert_eq(0, sp_queue_get(q, i, &a));
			cr_assert_eq(0, data_cmp(&a, d + i));
		}
		cr_assert_eq(0, data_init(d + 5));
		cr_assert_eq(0, sp_queue_qinsert(q, 1, d + 5));
		cr_assert_eq(q->tail, (char*)q->data + 5 * q->elem_size, "%p");
		cr_assert_eq(0, data_init(d + 6));
		cr_assert_eq(0, sp_queue_qinsert(q, 3, d + 6));
		cr_assert_eq(q->tail, (char*)q->data + 6 * q->elem_size, "%p");
		cr_assert_eq(0, sp_queue_destroy(q, data_dtor));
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
		q = sp_queue_create(sizeof(T), 1000);                                     \
		cr_assert_not_null(q);                                                     \
		cr_assert_eq(SP_EINVAL, F1(NULL, 0, a));                                  \
		cr_assert_eq(SP_EINDEX, F1(q, 1, a));                                     \
		cr_assert_eq(0, F1(q, 0, a));                                              \
		cr_assert_eq(1LU, (unsigned long)q->size, "%lu");                          \
		cr_assert_eq(a, F2(q, 0), M);                                              \
		cr_assert_eq(0, sp_queue_destroy(q, NULL));                               \
                                                                                           \
		q = sp_queue_create(sizeof(T), 1000);                                     \
		cr_assert_not_null(q);                                                     \
		for (i = 0; i < 1000; i++) {                                               \
			size_t idx = IRANGE(0, i);                                         \
			d[i] = (V);                                                        \
			cr_assert_eq(0, F1(q, idx, d[i]));                                 \
			cr_assert_eq((unsigned long)i + 1, (unsigned long)q->size, "%lu"); \
			cr_assert_eq(d[i], F2(q, idx), M);                                 \
		}                                                                          \
		cr_assert_eq(0, sp_queue_destroy(q, NULL));                               \
                                                                                           \
		q = sp_queue_create(sizeof(T), 10);                                       \
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
		cr_assert_eq(0, sp_queue_destroy(q, NULL));                               \
		q = sp_queue_create(sizeof(T) + 1, 1000);                                 \
		cr_assert_not_null(q);                                                     \
		cr_assert_eq(SP_EILLEGAL, F1(q, 0, (V)));                                 \
		cr_assert_eq(0, sp_queue_destroy(q, NULL));                               \
	} while (0)
	test(char          , sp_queue_qinsertc , sp_queue_getc , IRANGE(1, CHAR_MAX) , "%hd");
	test(short         , sp_queue_qinserts , sp_queue_gets , IRANGE(1, SHRT_MAX) , "%hd");
	test(int           , sp_queue_qinserti , sp_queue_geti , FRANGE(1, INT_MAX)  , "%d");
	test(long          , sp_queue_qinsertl , sp_queue_getl , FRANGE(1, LONG_MAX) , "%ld");
	test(signed char   , sp_queue_qinsertsc, sp_queue_getsc, IRANGE(1, SCHAR_MAX), "%hd");
	test(unsigned char , sp_queue_qinsertuc, sp_queue_getuc, IRANGE(1, UCHAR_MAX), "%hd");
	test(unsigned short, sp_queue_qinsertus, sp_queue_getus, IRANGE(1, USHRT_MAX), "%hu");
	test(unsigned int  , sp_queue_qinsertui, sp_queue_getui, FRANGE(1, UINT_MAX) , "%u");
	test(unsigned long , sp_queue_qinsertul, sp_queue_getul, FRANGE(1, ULONG_MAX), "%lu");
	test(float         , sp_queue_qinsertf , sp_queue_getf , FRANGE(1, FLT_MAX)  , "%f");
	test(double        , sp_queue_qinsertd , sp_queue_getd , FRANGE(1, DBL_MAX)  , "%f");
	test(long double   , sp_queue_qinsertld, sp_queue_getld, FRANGE(1, LDBL_MAX) , "%Lf");
#undef test
}

Test(queue, remove)
{
	struct sp_queue *q;
	unsigned i;

	{ /* Generic form */
		struct data a;
		struct data d[100];
		q = sp_queue_create(sizeof(int), 1000);
		cr_assert_not_null(q);
		cr_assert_eq(SP_EINDEX, sp_queue_remove(q, 0, &a));
		cr_assert_eq(0, data_init(&a));
		cr_assert_eq(0, sp_queue_push(q, &a));
		cr_assert_eq(SP_EINVAL, sp_queue_remove(NULL, 0, &a));
		cr_assert_eq(SP_EINVAL, sp_queue_remove(NULL, 0, NULL));
		cr_assert_eq(SP_EINDEX, sp_queue_remove(q, 1, &a));
		cr_assert_eq(q->data, q->head, "%p");
		cr_assert_eq(q->data, q->tail, "%p");
		cr_assert_eq(0, sp_queue_remove(q, 0, NULL));
		cr_assert_eq(0LU, (unsigned long)q->size, "%lu");
		cr_assert_eq(q->data, q->head, "%p");
		cr_assert_eq(q->data, q->tail, "%p");
		cr_assert_eq(0, sp_queue_destroy(q, NULL));
		cr_assert_eq(0, data_dtor(&a));

		q = sp_queue_create(sizeof(struct data), 100);
		cr_assert_not_null(q);
		for (i = 0; i < 100; i++) {
			cr_assert_eq(0, data_init(d + i));
			cr_assert_eq(0, sp_queue_push(q, d + i));
		}
		for (i = 0; i < 100; i++) {
			struct data a;
			size_t idx = IRANGE(0, q->size - 1), j;
			int found = 0;
			cr_assert_eq(0, sp_queue_remove(q, idx, &a));
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
		cr_assert_eq(0, sp_queue_destroy(q, NULL));

		q = sp_queue_create(sizeof(struct data), 10);
		cr_assert_not_null(q);
		for (i = 0; i < 8; i++) {
			cr_assert_eq(0, sp_queue_push(q, d + i));
		}
		cr_assert_eq(0, sp_queue_remove(q, 3, &a));
		cr_assert_eq(0, data_cmp(d + 3, &a));
		cr_assert_eq(q->head, (char*)q->data + q->elem_size, "%p");
		cr_assert_eq(0, sp_queue_remove(q, 0, &a));
		cr_assert_eq(0, data_cmp(d, &a));
		cr_assert_eq(q->head, (char*)q->data + 2 * q->elem_size, "%p");
		cr_assert_eq(0, sp_queue_remove(q, 3, &a));
		cr_assert_eq(0, data_cmp(d + 5, &a));
		cr_assert_eq(q->tail, (char*)q->data + 6 * q->elem_size, "%p");
		cr_assert_eq(0, sp_queue_destroy(q, NULL));
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
		q = sp_queue_create(sizeof(T), 1000);                             \
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
		cr_assert_eq(0, sp_queue_destroy(q, NULL));                       \
                                                                                   \
		q = sp_queue_create(sizeof(T), 100);                              \
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
		cr_assert_eq(0, sp_queue_destroy(q, NULL));                       \
                                                                                   \
		q = sp_queue_create(sizeof(T), 10);                               \
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
		cr_assert_eq(0, sp_queue_destroy(q, NULL));                       \
		q = sp_queue_create(sizeof(T) + 1, 1000);                         \
		cr_assert_not_null(q);                                             \
		q->size = 1;                                                       \
		cr_assert_eq(z, F1(q, 0), M);                                      \
		cr_assert_eq(0, sp_queue_destroy(q, NULL));                       \
	} while (0)
	test(char          , sp_queue_removec , sp_queue_pushc , IRANGE(1, CHAR_MAX) , "%hd");
	test(short         , sp_queue_removes , sp_queue_pushs , IRANGE(1, SHRT_MAX) , "%hd");
	test(int           , sp_queue_removei , sp_queue_pushi , FRANGE(1, INT_MAX)  , "%d");
	test(long          , sp_queue_removel , sp_queue_pushl , FRANGE(1, LONG_MAX) , "%ld");
	test(signed char   , sp_queue_removesc, sp_queue_pushsc, IRANGE(1, SCHAR_MAX), "%hd");
	test(unsigned char , sp_queue_removeuc, sp_queue_pushuc, IRANGE(1, UCHAR_MAX), "%hd");
	test(unsigned short, sp_queue_removeus, sp_queue_pushus, IRANGE(1, USHRT_MAX), "%hu");
	test(unsigned int  , sp_queue_removeui, sp_queue_pushui, FRANGE(1, UINT_MAX) , "%u");
	test(unsigned long , sp_queue_removeul, sp_queue_pushul, FRANGE(1, ULONG_MAX), "%lu");
	test(float         , sp_queue_removef , sp_queue_pushf , FRANGE(1, FLT_MAX)  , "%f");
	test(double        , sp_queue_removed , sp_queue_pushd , FRANGE(1, DBL_MAX)  , "%f");
	test(long double   , sp_queue_removeld, sp_queue_pushld, FRANGE(1, LDBL_MAX) , "%Lf");
#undef test
}

Test(queue, qremove)
{
	struct sp_queue *q;
	unsigned i;

	{ /* Generic form */
		struct data a;
		struct data d[100];
		q = sp_queue_create(sizeof(int), 1000);
		cr_assert_not_null(q);
		cr_assert_eq(SP_EINDEX, sp_queue_qremove(q, 0, &a));
		cr_assert_eq(0, data_init(&a));
		cr_assert_eq(0, sp_queue_push(q, &a));
		cr_assert_eq(SP_EINVAL, sp_queue_qremove(NULL, 0, &a));
		cr_assert_eq(SP_EINVAL, sp_queue_qremove(NULL, 0, NULL));
		cr_assert_eq(SP_EINDEX, sp_queue_qremove(q, 1, &a));
		cr_assert_eq(q->data, q->head, "%p");
		cr_assert_eq(q->data, q->tail, "%p");
		cr_assert_eq(0, sp_queue_qremove(q, 0, NULL));
		cr_assert_eq(0LU, (unsigned long)q->size, "%lu");
		cr_assert_eq(q->data, q->head, "%p");
		cr_assert_eq(q->data, q->tail, "%p");
		cr_assert_eq(0, sp_queue_destroy(q, NULL));
		cr_assert_eq(0, data_dtor(&a));

		q = sp_queue_create(sizeof(struct data), 100);
		cr_assert_not_null(q);
		for (i = 0; i < 100; i++) {
			cr_assert_eq(0, data_init(d + i));
			cr_assert_eq(0, sp_queue_push(q, d + i));
		}
		for (i = 0; i < 100; i++) {
			struct data a;
			size_t idx = IRANGE(0, q->size - 1), j;
			int found = 0;
			cr_assert_eq(0, sp_queue_qremove(q, idx, &a));
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
		cr_assert_eq(0, sp_queue_destroy(q, NULL));

		q = sp_queue_create(sizeof(struct data), 10);
		cr_assert_not_null(q);
		for (i = 0; i < 8; i++) {
			cr_assert_eq(0, sp_queue_push(q, d + i));
		}
		cr_assert_eq(0, sp_queue_qremove(q, 3, &a));
		cr_assert_eq(0, data_cmp(d + 3, &a));
		cr_assert_eq(0, sp_queue_get(q, 3, &a));
		cr_assert_eq(0, data_cmp(d + 7, &a));
		cr_assert_eq(0, sp_queue_qremove(q, 0, &a));
		cr_assert_eq(0, data_cmp(d, &a));
		cr_assert_eq(0, sp_queue_get(q, 0, &a));
		cr_assert_eq(0, data_cmp(d + 6, &a));
		cr_assert_eq(0, sp_queue_qremove(q, 3, &a));
		cr_assert_eq(0, data_cmp(d + 7, &a));
		cr_assert_eq(q->tail, (char*)q->data + 4 * q->elem_size, "%p");
		cr_assert_eq(0, sp_queue_destroy(q, NULL));
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
		q = sp_queue_create(sizeof(T), 1000);                             \
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
		cr_assert_eq(0, sp_queue_destroy(q, NULL));                       \
                                                                                   \
		q = sp_queue_create(sizeof(T), 100);                              \
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
		cr_assert_eq(0, sp_queue_destroy(q, NULL));                       \
                                                                                   \
		q = sp_queue_create(sizeof(T), 10);                               \
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
		cr_assert_eq(0, sp_queue_destroy(q, NULL));                       \
		q = sp_queue_create(sizeof(T) + 1, 1000);                         \
		cr_assert_not_null(q);                                             \
		q->size = 1;                                                       \
		cr_assert_eq(z, F1(q, 0), M);                                      \
		cr_assert_eq(0, sp_queue_destroy(q, NULL));                       \
	} while (0)
	test(char          , sp_queue_qremovec , sp_queue_pushc , sp_queue_getc , IRANGE(1, CHAR_MAX) , "%hd");
	test(short         , sp_queue_qremoves , sp_queue_pushs , sp_queue_gets , IRANGE(1, SHRT_MAX) , "%hd");
	test(int           , sp_queue_qremovei , sp_queue_pushi , sp_queue_geti , FRANGE(1, INT_MAX)  , "%d");
	test(long          , sp_queue_qremovel , sp_queue_pushl , sp_queue_getl , FRANGE(1, LONG_MAX) , "%ld");
	test(signed char   , sp_queue_qremovesc, sp_queue_pushsc, sp_queue_getsc, IRANGE(1, SCHAR_MAX), "%hd");
	test(unsigned char , sp_queue_qremoveuc, sp_queue_pushuc, sp_queue_getuc, IRANGE(1, UCHAR_MAX), "%hd");
	test(unsigned short, sp_queue_qremoveus, sp_queue_pushus, sp_queue_getus, IRANGE(1, USHRT_MAX), "%hu");
	test(unsigned int  , sp_queue_qremoveui, sp_queue_pushui, sp_queue_getui, FRANGE(1, UINT_MAX) , "%u");
	test(unsigned long , sp_queue_qremoveul, sp_queue_pushul, sp_queue_getul, FRANGE(1, ULONG_MAX), "%lu");
	test(float         , sp_queue_qremovef , sp_queue_pushf , sp_queue_getf , FRANGE(1, FLT_MAX)  , "%f");
	test(double        , sp_queue_qremoved , sp_queue_pushd , sp_queue_getd , FRANGE(1, DBL_MAX)  , "%f");
	test(long double   , sp_queue_qremoveld, sp_queue_pushld, sp_queue_getld, FRANGE(1, LDBL_MAX) , "%Lf");
#undef test
}

Test(queue, get)
{
	struct sp_queue *q;

	{ /* Generic form */
		unsigned i;
		int a = 0;
		struct data d[1000];
		q = sp_queue_create(sizeof(int), 1000);
		cr_assert_not_null(q);
		cr_assert_eq(SP_EINDEX, sp_queue_get(q, 0, &a));
		cr_assert_eq(0, sp_queue_pushi(q, 10));
		cr_assert_eq(SP_EINVAL, sp_queue_get(NULL, 0, &a));
		cr_assert_eq(SP_EINVAL, sp_queue_get(q, 0, NULL));
		cr_assert_eq(SP_EINVAL, sp_queue_get(NULL, 0, NULL));
		cr_assert_eq(SP_EINDEX, sp_queue_get(q, 1, &a));
		cr_assert_eq(0, sp_queue_get(q, 0, &a));
		cr_assert_eq(10, a);
		cr_assert_eq(0, sp_queue_destroy(q, NULL));
		q = sp_queue_create(sizeof(struct data), 1000);
		cr_assert_not_null(q);
		for (i = 0; i < 1000; i++) {
			cr_assert_eq(0, data_init(d + i));
			cr_assert_eq(0, sp_queue_push(q, d + i));
		}
		for (i = 0; i < 1000; i++) {
			struct data a;
			cr_assert_eq(0, sp_queue_get(q, i, &a));
			cr_assert_eq(0, data_cmp(&a, d + i));
		}
		cr_assert_eq(0, sp_queue_destroy(q, data_dtor));
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
		q = sp_queue_create(sizeof(T), 1000);       \
		cr_assert_not_null(q);                       \
		cr_assert_eq(z, F1(q, 0), M);                \
		cr_assert_eq(0, F2(q, a));                   \
		cr_assert_eq(z, F1(NULL, 0), M);             \
		cr_assert_eq(z, F1(q, 1), M);                \
		cr_assert_eq(a, F1(q, 0), M);                \
		cr_assert_eq(0, sp_queue_destroy(q, NULL)); \
		q = sp_queue_create(sizeof(T), 1000);       \
		cr_assert_not_null(q);                       \
		for (i = 0; i < 1000; i++) {                 \
			d[i] = (V);                          \
			cr_assert_eq(0, F2(q, d[i]));        \
		}                                            \
		for (i = 0; i < 1000; i++) {                 \
			cr_assert_eq(d[i], F1(q, i), M);     \
		}                                            \
		cr_assert_eq(0, sp_queue_destroy(q, NULL)); \
	} while (0)
	test(char          , sp_queue_getc , sp_queue_pushc , IRANGE(1, CHAR_MAX) , "%hd");
	test(short         , sp_queue_gets , sp_queue_pushs , IRANGE(1, SHRT_MAX) , "%hd");
	test(int           , sp_queue_geti , sp_queue_pushi , FRANGE(1, INT_MAX)  , "%d");
	test(long          , sp_queue_getl , sp_queue_pushl , FRANGE(1, LONG_MAX) , "%ld");
	test(signed char   , sp_queue_getsc, sp_queue_pushsc, IRANGE(1, SCHAR_MAX), "%hd");
	test(unsigned char , sp_queue_getuc, sp_queue_pushuc, IRANGE(1, UCHAR_MAX), "%hd");
	test(unsigned short, sp_queue_getus, sp_queue_pushus, IRANGE(1, USHRT_MAX), "%hu");
	test(unsigned int  , sp_queue_getui, sp_queue_pushui, FRANGE(1, UINT_MAX) , "%u");
	test(unsigned long , sp_queue_getul, sp_queue_pushul, FRANGE(1, ULONG_MAX), "%lu");
	test(float         , sp_queue_getf , sp_queue_pushf , FRANGE(1, FLT_MAX)  , "%f");
	test(double        , sp_queue_getd , sp_queue_pushd , FRANGE(1, DBL_MAX)  , "%f");
	test(long double   , sp_queue_getld, sp_queue_pushld, FRANGE(1, LDBL_MAX) , "%Lf");
#undef test
}

Test(queue, set)
{
	struct sp_queue *q;

	{ /* Generic form */
		unsigned i;
		int a = 1;
		q = sp_queue_create(sizeof(int), 1000);
		cr_assert_not_null(q);
		cr_assert_eq(SP_EINDEX, sp_queue_set(q, 0, &a));
		cr_assert_eq(0, sp_queue_pushi(q, 10));
		cr_assert_eq(SP_EINVAL, sp_queue_set(NULL, 0, &a));
		cr_assert_eq(SP_EINVAL, sp_queue_set(q, 0, NULL));
		cr_assert_eq(SP_EINVAL, sp_queue_set(NULL, 0, NULL));
		cr_assert_eq(SP_EINDEX, sp_queue_set(q, 1, &a));
		cr_assert_eq(0, sp_queue_set(q, 0, &a));
		cr_assert_eq(a, sp_queue_geti(q, 0));
		cr_assert_eq(0, sp_queue_destroy(q, NULL));
		q = sp_queue_create(sizeof(struct data), 1000);
		cr_assert_not_null(q);
		for (i = 0; i < 1000; i++) {
			struct data d;
			cr_assert_eq(0, data_init(&d));
			cr_assert_eq(0, sp_queue_push(q, &d));
		}
		for (i = 0; i < 1000; i++) {
			struct data a, b;
			cr_assert_eq(0, data_init(&b));
			cr_assert_eq(0, sp_queue_get(q, i, &a));
			cr_assert_eq(0, data_dtor(&a));
			cr_assert_eq(0, sp_queue_set(q, i, &b));
			cr_assert_eq(0, sp_queue_get(q, i, &a));
			cr_assert_eq(0, data_cmp(&a, &b));
		}
		cr_assert_eq(0, sp_queue_destroy(q, data_dtor));
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
		q = sp_queue_create(sizeof(T), 1000);       \
		cr_assert_not_null(q);                       \
		cr_assert_eq(SP_EINDEX, F1(q, 0, a));       \
		cr_assert_eq(0, F2(q, (V)));                 \
		cr_assert_eq(SP_EINVAL, F1(NULL, 0, a));    \
		cr_assert_eq(SP_EINDEX, F1(q, 1, a));       \
		cr_assert_eq(0, F1(q, 0, a));                \
		cr_assert_eq(a, F3(q, 0), M);                \
		cr_assert_eq(0, sp_queue_clear(q, NULL));   \
		for (i = 0; i < 1000; i++) {                 \
			cr_assert_eq(0, F2(q, (V)));         \
		}                                            \
		for (i = 0; i < 1000; i++) {                 \
			T b = (V);                           \
			cr_assert_eq(0, F1(q, i, b));        \
			cr_assert_eq(b, F3(q, i), M);        \
		}                                            \
		cr_assert_eq(0, sp_queue_destroy(q, NULL)); \
		q = sp_queue_create(sizeof(T) + 1, 1000);   \
		cr_assert_not_null(q);                       \
		q->size = 1;                                 \
		cr_assert_eq(SP_EILLEGAL, F1(q, 0, (V)));   \
		cr_assert_eq(0, sp_queue_destroy(q, NULL)); \
	} while (0)
	test(char          , sp_queue_setc , sp_queue_pushc , sp_queue_getc , IRANGE(1, CHAR_MAX) , "%hd");
	test(short         , sp_queue_sets , sp_queue_pushs , sp_queue_gets , IRANGE(1, SHRT_MAX) , "%hd");
	test(int           , sp_queue_seti , sp_queue_pushi , sp_queue_geti , FRANGE(1, INT_MAX)  , "%d");
	test(long          , sp_queue_setl , sp_queue_pushl , sp_queue_getl , FRANGE(1, LONG_MAX) , "%ld");
	test(signed char   , sp_queue_setsc, sp_queue_pushsc, sp_queue_getsc, IRANGE(1, SCHAR_MAX), "%hd");
	test(unsigned char , sp_queue_setuc, sp_queue_pushuc, sp_queue_getuc, IRANGE(1, UCHAR_MAX), "%hd");
	test(unsigned short, sp_queue_setus, sp_queue_pushus, sp_queue_getus, IRANGE(1, USHRT_MAX), "%hu");
	test(unsigned int  , sp_queue_setui, sp_queue_pushui, sp_queue_getui, FRANGE(1, UINT_MAX) , "%u");
	test(unsigned long , sp_queue_setul, sp_queue_pushul, sp_queue_getul, FRANGE(1, ULONG_MAX), "%lu");
	test(float         , sp_queue_setf , sp_queue_pushf , sp_queue_getf , FRANGE(1, FLT_MAX)  , "%f");
	test(double        , sp_queue_setd , sp_queue_pushd , sp_queue_getd , FRANGE(1, DBL_MAX)  , "%f");
	test(long double   , sp_queue_setld, sp_queue_pushld, sp_queue_getld, FRANGE(1, LDBL_MAX) , "%Lf");
#undef test
}

Test(queue, print)
{
	struct sp_queue *q;

	{ /* Generic form */
		double a = 4.5, b = -3.14;
		q = sp_queue_create(sizeof(double), 30);
		cr_assert_eq(0, sp_queue_push(q, &a));
		cr_assert_eq(0, sp_queue_push(q, &b));
		cr_assert_eq(SP_EINVAL, sp_queue_print(NULL));
		cr_assert_eq(0, sp_queue_print(q));
		cr_assert_eq(0, sp_queue_destroy(q, NULL));
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
		q = sp_queue_create(sizeof(T), 30);         \
		cr_assert_eq(0, F1(q, a));                   \
		cr_assert_eq(0, F1(q, b));                   \
		cr_assert_eq(SP_EINVAL, F2(NULL));          \
		cr_assert_eq(0, F2(q));                      \
		cr_assert_eq(0, sp_queue_destroy(q, NULL)); \
		q = sp_queue_create(sizeof(T) + 1, 30);     \
		cr_assert_eq(SP_EILLEGAL, F2(q));           \
		cr_assert_eq(0, sp_queue_destroy(q, NULL)); \
	} while(0)

	test(char          , sp_queue_pushc , sp_queue_printc , 'A', 'B');
	test(short         , sp_queue_pushs , sp_queue_prints , SHRT_MIN, SHRT_MAX);
	test(int           , sp_queue_pushi , sp_queue_printi , INT_MIN, INT_MAX);
	test(long          , sp_queue_pushl , sp_queue_printl , LONG_MIN, LONG_MAX);
	test(signed char   , sp_queue_pushsc, sp_queue_printsc, SCHAR_MIN, SCHAR_MAX);
	test(unsigned char , sp_queue_pushuc, sp_queue_printuc, 0, UCHAR_MAX);
	test(unsigned short, sp_queue_pushus, sp_queue_printus, 0, USHRT_MAX);
	test(unsigned int  , sp_queue_pushui, sp_queue_printui, 0, UINT_MAX);
	test(unsigned long , sp_queue_pushul, sp_queue_printul, 0, ULONG_MAX);
	test(float         , sp_queue_pushf , sp_queue_printf , FLT_MIN, FLT_MAX);
	test(double        , sp_queue_pushd , sp_queue_printd , DBL_MIN, DBL_MAX);
	test(long double   , sp_queue_pushld, sp_queue_printld, LDBL_MIN, LDBL_MAX);
}

Test(queue, ringbuf_resize)
{
	struct sp_queue *q;
	unsigned i;
	q = sp_queue_create(sizeof(int), 5);
	cr_assert_not_null(q);
	for (i = 0; i < 5; i++)
		cr_assert_eq(0, sp_queue_pushi(q, FRANGE(1, INT_MAX)));
	cr_assert_eq(q->data, q->head, "%p");
	cr_assert_eq((char*)q->data + (q->size - 1) * q->elem_size, q->tail, "%p");
	cr_assert_eq(5LU, (unsigned long)q->capacity, "%lu");
	cr_assert_eq(0, sp_queue_pushi(q, FRANGE(1, INT_MAX)));
	cr_assert_eq(q->data, q->head, "%p");
	cr_assert_eq((char*)q->data + (q->size - 1) * q->elem_size, q->tail, "%p");
	cr_assert_eq(10LU, (unsigned long)q->capacity, "%lu");
	for (i = 0; i < 3; i++)
		cr_assert_neq(0, sp_queue_popi(q));
	for (i = 0; i < 7; i++)
		cr_assert_eq(0, sp_queue_pushi(q, FRANGE(1, INT_MAX)));
	cr_assert_eq((char*)q->data + 3 * q->elem_size, q->head, "%p");
	cr_assert_eq((char*)q->data + 2 * q->elem_size, q->tail, "%p");
	cr_assert_eq(10LU, (unsigned long)q->capacity, "%lu");
	cr_assert_eq(0, sp_queue_pushi(q, FRANGE(1, INT_MAX)));
	cr_assert_eq((char*)q->data + 3 * q->elem_size, q->head, "%p");
	cr_assert_eq((char*)q->data + 13 * q->elem_size, q->tail, "%p");
	cr_assert_eq(20LU, (unsigned long)q->capacity, "%lu");
	cr_assert_eq(0, sp_queue_destroy(q, NULL));
}
