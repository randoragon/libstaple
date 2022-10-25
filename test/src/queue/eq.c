#define setup(T, X, Y) \
	struct sp_queue *s1, *s2; \
	ck_assert_ptr_nonnull(s1 = sp_queue_create(sizeof(T), X)); \
	ck_assert_ptr_nonnull(s2 = sp_queue_create(sizeof(T), Y));

#define teardown(D) \
	ck_assert_int_eq(0, sp_queue_destroy(s1, D)); \
	ck_assert_int_eq(0, sp_queue_destroy(s2, D));

START_TEST(eq_empty)
{
	setup(int, 10, 15);
	ck_assert_int_eq(1, sp_queue_eq(s1, s2, NULL));
	ck_assert_int_eq(1, sp_queue_eq(s1, s2, data_cmp));
	teardown(NULL);
}
END_TEST

START_TEST(eq_basic)
{
	setup(int, 10, 15);
	ck_assert_int_eq(0, sp_queue_pushi(s1, 1));
	ck_assert_int_eq(0, sp_queue_eq(s1, s2, NULL));
	ck_assert_int_eq(0, sp_queue_pushi(s2, 1));
	ck_assert_int_eq(1, sp_queue_eq(s1, s2, NULL));

	ck_assert_int_eq(0, sp_queue_pushi(s2, 2));
	ck_assert_int_eq(0, sp_queue_eq(s1, s2, NULL));
	ck_assert_int_eq(0, sp_queue_pushi(s1, 2));
	ck_assert_int_eq(1, sp_queue_eq(s1, s2, NULL));

	ck_assert_int_eq(0, sp_queue_pushi(s1, 3));
	ck_assert_int_eq(0, sp_queue_eq(s1, s2, NULL));
	ck_assert_int_eq(0, sp_queue_pushi(s2, 4));
	ck_assert_int_eq(0, sp_queue_eq(s1, s2, NULL));

	ck_assert_int_eq(0, sp_queue_clear(s1, NULL));
	ck_assert_int_eq(0, sp_queue_clear(s2, NULL));
	ck_assert_int_eq(1, sp_queue_eq(s1, s2, NULL));
	teardown(NULL);
}
END_TEST

START_TEST(eq_object)
{
	struct data a, b, c;
	setup(struct data, 10, 15);
	ck_assert_int_eq(0, data_init(&a));
	ck_assert_int_eq(0, data_cpy(&b, &a));
	ck_assert_int_eq(0, data_init(&c));

	ck_assert_int_eq(0, sp_queue_push(s1, &a));
	ck_assert_int_eq(0, sp_queue_eq(s1, s2, data_cmp));
	ck_assert_int_eq(0, sp_queue_push(s2, &b));
	ck_assert_int_eq(0, sp_queue_eq(s1, s2, NULL));
	ck_assert_int_eq(1, sp_queue_eq(s1, s2, data_cmp));

	ck_assert_int_eq(0, sp_queue_push(s2, &c));
	ck_assert_int_eq(0, sp_queue_eq(s1, s2, data_cmp));
	ck_assert_int_eq(0, sp_queue_push(s1, &c));
	ck_assert_int_eq(1, sp_queue_eq(s1, s2, data_cmp));

	ck_assert_int_eq(0, sp_queue_push(s1, &a));
	ck_assert_int_eq(0, sp_queue_eq(s1, s2, data_cmp));
	ck_assert_int_eq(0, sp_queue_push(s2, &c));
	ck_assert_int_eq(0, sp_queue_eq(s1, s2, data_cmp));

	ck_assert_int_eq(0, sp_queue_clear(s1, NULL));
	ck_assert_int_eq(0, sp_queue_clear(s2, NULL));
	ck_assert_int_eq(1, sp_queue_eq(s1, s2, data_cmp));

	ck_assert_int_eq(0, data_dtor(&a));
	ck_assert_int_eq(0, data_dtor(&b));
	ck_assert_int_eq(0, data_dtor(&c));
	teardown(NULL);
}
END_TEST

START_TEST(eq_bad_args)
{
	setup(int, 10, 15);
	ck_assert_int_eq(0, sp_queue_eq(NULL, NULL, NULL));
	ck_assert_int_eq(0, sp_queue_eq(s1, NULL, NULL));
	ck_assert_int_eq(0, sp_queue_eq(NULL, s2, NULL));
	ck_assert_int_eq(0, sp_queue_eq(NULL, NULL, data_cmp));
	ck_assert_int_eq(0, sp_queue_eq(s1, NULL, data_cmp));
	ck_assert_int_eq(0, sp_queue_eq(NULL, s2, data_cmp));
	teardown(NULL);
}
END_TEST

START_TEST(eq_bad_elem_size)
{
	struct sp_queue *s1, *s2;
	ck_assert_ptr_nonnull(s1 = sp_queue_create(1, 1));
	ck_assert_ptr_nonnull(s2 = sp_queue_create(2, 1));
	ck_assert_int_eq(0, sp_queue_eq(s1, s2, NULL));
	ck_assert_int_eq(0, sp_queue_eq(s1, s2, data_cmp));
	ck_assert_int_eq(0, sp_queue_destroy(s1, NULL));
	ck_assert_int_eq(0, sp_queue_destroy(s2, NULL));
}
END_TEST

void init_eq(Suite *suite, TCase *tc)
{
	suite_add_tcase(suite, tc);
	tcase_add_test(tc, eq_empty);
	tcase_add_test(tc, eq_basic);
	tcase_add_test(tc, eq_object);
	tcase_add_test(tc, eq_bad_args);
	tcase_add_test(tc, eq_bad_elem_size);
}

#undef setup
#undef teardown
