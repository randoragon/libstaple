#include <stdlib.h>
#include <time.h>
#include "../../src/rnd.h"
#include "test_struct.h"
#include <greatest.h>
#include <limits.h>
#include <float.h>

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
	ASSERT_EQ_FMT(0, rnd_queue_destroy(q, NULL), "%d");
	q = rnd_queue_create(sizeof(struct data), 1000);
	for (i = 0; i < 1000; i++) {
		struct data d;
		ASSERT_EQ_FMT(0, data_init(&d), "%d");
		ASSERT_EQ_FMT(0, rnd_queue_push(q, &d), "%d");
	}
	ASSERT_EQ_FMT(0, rnd_queue_destroy(q, data_dtor), "%d");
	PASS();
}

TEST t_push(void)
{
	struct rnd_queue *q;
	PASS();
}

TEST t_peek(void)
{
	struct rnd_queue *q;
	PASS();
}

TEST t_pop(void)
{
	struct rnd_queue *q;
	PASS();
}

TEST t_clear(void)
{
	struct rnd_queue *q;
	PASS();
}

TEST t_map(void)
{
	struct rnd_queue *q;
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
