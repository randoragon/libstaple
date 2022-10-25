#define setup(T, X) \
	struct sp_stack *s; \
	ck_assert_ptr_nonnull(s = sp_stack_create(sizeof(T), X));

#define teardown(D) \
	ck_assert_int_eq(0, sp_stack_destroy(s, D));

START_TEST(foreach_ok)
{
	int i;
	setup(struct data, 10);
	for (i = 0; i < 3; i++) {
		struct data d;
		ck_assert_int_eq(0, data_init(&d));
		ck_assert_int_eq(0, sp_stack_push(s, &d));
	}
	ck_assert_int_eq(0, sp_stack_foreach(s, data_mutate));
	ck_assert_int_eq(0, sp_stack_foreach(s, data_verify));
	teardown(data_dtor);
}
END_TEST

START_TEST(foreach_bad_args)
{
	setup(int, 10);
	ck_assert_int_eq(SP_EINVAL, sp_stack_foreach(s, NULL));
	ck_assert_int_eq(SP_EINVAL, sp_stack_foreach(NULL, data_mutate));
	ck_assert_int_eq(SP_EINVAL, sp_stack_foreach(NULL, NULL));
	teardown(NULL);
}
END_TEST

START_TEST(foreach_bad_callback)
{
	int i;
	setup(struct data, 10);
	for (i = 0; i < 3; i++) {
		struct data d;
		ck_assert_int_eq(0, data_init(&d));
		ck_assert_int_eq(0, sp_stack_push(s, &d));
	}
	ck_assert_int_eq(SP_ECALLBK, sp_stack_foreach(s, data_mutate_bad));
	teardown(data_dtor);
}
END_TEST

void init_foreach(Suite *suite, TCase *tc)
{
	suite_add_tcase(suite, tc);
	tcase_add_test(tc, foreach_ok);
	tcase_add_test(tc, foreach_bad_args);
	tcase_add_test(tc, foreach_bad_callback);
}

#undef setup
#undef teardown
