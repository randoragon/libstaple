/* This header includes boilerplate struct and associated functions for purposes
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
int data_cmp(const struct data *a, const struct data *b);

/* These two functions are used for testing "foreach" functions, e.g.
 * sp_stack_foreach.  The goal is to first mutate a bunch of data structs in a
 * loop, and then verify if all the operations have been executed correctly.
 * It's not relevant what mutation actually does, so long it's something and
 * it's verifiable.
 */
int data_mutate(void *d, size_t idx);
int data_mutate_bad(void *d, size_t idx);
int data_verify(void *d, size_t idx);

#endif /* TEST_STRUCT_H */
