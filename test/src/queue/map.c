#define setup(T, X) \
	struct sp_queue *s; \
	ck_assert_ptr_nonnull(s = sp_queue_create(sizeof(T), X));

#define teardown(D) \
	ck_assert_int_eq(0, sp_queue_destroy(s, D));

START_TEST(map_ok)
{
	int i;
	setup(struct data, 10);
	for (i = 0; i < 3; i++) {
		struct data d;
		ck_assert_int_eq(0, data_init(&d));
		ck_assert_int_eq(0, sp_queue_push(s, &d));
	}
	ck_assert_int_eq(0, sp_queue_map(s, data_mutate));
	ck_assert_int_eq(0, sp_queue_map(s, data_verify));
	teardown(data_dtor);
}
END_TEST

START_TEST(map_bad_args)
{
	setup(int, 10);
	ck_assert_int_eq(SP_EINVAL, sp_queue_map(s, NULL));
	ck_assert_int_eq(SP_EINVAL, sp_queue_map(NULL, data_mutate));
	ck_assert_int_eq(SP_EINVAL, sp_queue_map(NULL, NULL));
	teardown(NULL);
}
END_TEST

START_TEST(map_bad_callback)
{
	int i;
	setup(struct data, 10);
	for (i = 0; i < 3; i++) {
		struct data d;
		ck_assert_int_eq(0, data_init(&d));
		ck_assert_int_eq(0, sp_queue_push(s, &d));
	}
	ck_assert_int_eq(SP_ECALLBK, sp_queue_map(s, data_mutate_bad));
	teardown(data_dtor);
}
END_TEST

void init_map(Suite *suite, TCase *tc)
{
	suite_add_tcase(suite, tc);
	tcase_add_test(tc, map_ok);
	tcase_add_test(tc, map_bad_args);
	tcase_add_test(tc, map_bad_callback);
}

#undef setup
#undef teardown
