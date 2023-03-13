#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "../../src/sp_queue.h"
#include "test_struct.h"
#include <check.h>

/* Make testing for size overflow feasible */
#ifdef SIZE_MAX
#undef SIZE_MAX
#endif
#define SIZE_MAX 65535LU

/* Shortcut to reduce boilerplate */
#define init_and_add(N) \
	init_##N(suite, tc_##N); \
	suite_add_tcase(suite, tc_##N);

#include "queue/create.c"
#include "queue/destroy.c"
#include "queue/eq.c"
#include "queue/push.c"
#include "queue/peek.c"
#include "queue/pop.c"
#include "queue/clear.c"
#include "queue/get.c"
#include "queue/set.c"
#include "queue/insert.c"
#include "queue/remove.c"
#include "queue/qinsert.c"
#include "queue/qremove.c"
#include "queue/copy.c"
#include "queue/foreach.c"
#include "queue/print.c"

int main(void)
{
	SRunner *runner;
	Suite *suite;
	int nf, seed;
	TCase *tc_create  = tcase_create("create"),
	      *tc_destroy = tcase_create("destroy"),
	      *tc_eq      = tcase_create("eq"),
	      *tc_push    = tcase_create("push"),
	      *tc_peek    = tcase_create("peek"),
	      *tc_pop     = tcase_create("pop"),
	      *tc_clear   = tcase_create("clear"),
	      *tc_get     = tcase_create("get"),
	      *tc_set     = tcase_create("set"),
	      *tc_insert  = tcase_create("insert"),
	      *tc_remove  = tcase_create("remove"),
	      *tc_qinsert = tcase_create("qinsert"),
	      *tc_qremove = tcase_create("qremove"),
	      *tc_copy    = tcase_create("copy"),
	      *tc_foreach = tcase_create("foreach"),
	      *tc_print   = tcase_create("print");

	if (!sp_is_debug() || sp_is_abort()) {
		printf("staple was not compiled correctly for testing -- debug mode should be enabled and abort mode disabled\n");
		return EXIT_FAILURE;
	}

	seed = time(NULL);
	srand(seed);
	printf("seed: %d\n", seed);

	suite = suite_create("queue");
	init_and_add(create);
	init_and_add(destroy);
	init_and_add(eq);
	init_and_add(push);
	init_and_add(peek);
	init_and_add(pop);
	init_and_add(clear);
	init_and_add(get);
	init_and_add(set);
	init_and_add(insert);
	init_and_add(remove);
	init_and_add(qinsert);
	init_and_add(qremove);
	init_and_add(copy);
	init_and_add(foreach);
	init_and_add(print);

	runner = srunner_create(suite);
	srunner_run_all(runner, CK_ENV);
	nf = srunner_ntests_failed(runner);
	srunner_free(runner);

	return nf == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
