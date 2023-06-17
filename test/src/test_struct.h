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
 *
 * -----------------------------------------------------------------------------
 * This header includes boilerplate struct and associated functions for purposes
 * of testing data structure operations and handling of compound (non-primitive)
 * types, things like external destructors, copy functions and such.
 */
#ifndef TEST_STRUCT_H
#define TEST_STRUCT_H

#include <stdlib.h>

#define LEN(X) (sizeof(X) / sizeof(*X))
#define IRANGE(X,Y) ((X) + (rand() % ((Y) - (X) + 1)))
#define FRANGE(X,Y) ((X) + ((double)rand() / RAND_MAX * ((long double)(Y) - (X))))

struct data {
	char    *name;
	char    *surname;
	unsigned age;
	unsigned id;
};

int data_init(struct data *d);
int data_cpy(void *dest, const void *src);
int data_cpy_bad(void *dest, const void *src);
int data_dtor(void *d);
int data_dtor_bad(void *d);
int data_cmp(const void *a, const void *b);

/* These two functions are used for testing "map" functions, e.g. sp_stack_map.
 * The goal is to first mutate a bunch of data structs in a loop, and then
 * verify if all the operations have been executed correctly. It's not relevant
 * what mutation actually does, so long it's something and it's verifiable.
 */
int data_mutate(void *d, size_t idx);
int data_mutate_bad(void *d, size_t idx);
int data_verify(void *d, size_t idx);

/* For testing generic "print" functions. */
int data_print(const void *d);
int data_print_bad(const void *d);

#endif /* TEST_STRUCT_H */
