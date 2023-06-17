#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "../../src/sp_stack.h"
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

#include "stack/create.c"
#include "stack/destroy.c"
#include "stack/eq.c"
#include "stack/push.c"
#include "stack/peek.c"
#include "stack/pop.c"
#include "stack/clear.c"
#include "stack/get.c"
#include "stack/set.c"
#include "stack/insert.c"
#include "stack/remove.c"
#include "stack/qinsert.c"
#include "stack/qremove.c"
#include "stack/copy.c"
#include "stack/map.c"
#include "stack/print.c"

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
	      *tc_map     = tcase_create("map"),
	      *tc_print   = tcase_create("print");

	if (!sp_is_debug() || sp_is_abort()) {
		printf("staple was not compiled correctly for testing -- debug mode should be enabled and abort mode disabled\n");
		return EXIT_FAILURE;
	}

	seed = time(NULL);
	srand(seed);
	printf("seed: %d\n", seed);

	suite = suite_create("stack");
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
	init_and_add(map);
	init_and_add(print);

	runner = srunner_create(suite);
	srunner_run_all(runner, CK_ENV);
	nf = srunner_ntests_failed(runner);
	srunner_free(runner);

	return nf == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
