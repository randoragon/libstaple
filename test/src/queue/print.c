#define setup(T, X) \
	struct sp_queue *s; \
	ck_assert_ptr_nonnull(s = sp_queue_create(sizeof(T), X));

#define teardown(D) \
	ck_assert_int_eq(0, sp_queue_destroy(s, D));

START_TEST(print_basic)
{
	setup(int, 10);
	ck_assert_int_eq(0, sp_queue_printi(s));
	ck_assert_int_eq(0, sp_queue_pushi(s, 1));
	ck_assert_int_eq(0, sp_queue_pushi(s, 2));
	ck_assert_int_eq(0, sp_queue_pushi(s, 3));
	ck_assert_int_eq(0, sp_queue_printi(s));
	teardown(NULL);
}
END_TEST

START_TEST(print_object)
{
	struct data a, b, c;
	setup(struct data, 10);
	data_init(&a);
	data_init(&b);
	data_init(&c);
	ck_assert_int_eq(0, sp_queue_print(s, NULL));
	ck_assert_int_eq(0, sp_queue_print(s, data_print));
	ck_assert_int_eq(0, sp_queue_push(s, &a));
	ck_assert_int_eq(0, sp_queue_push(s, &b));
	ck_assert_int_eq(0, sp_queue_push(s, &c));
	ck_assert_int_eq(0, sp_queue_print(s, NULL));
	ck_assert_int_eq(0, sp_queue_print(s, data_print));
	teardown(data_dtor);
}
END_TEST

START_TEST(print_bad_args)
{
	ck_assert_int_eq(SP_EINVAL, sp_queue_printi(NULL));
	ck_assert_int_eq(SP_EINVAL, sp_queue_print(NULL, NULL));
	ck_assert_int_eq(SP_EINVAL, sp_queue_print(NULL, data_print));
	ck_assert_int_eq(SP_EINVAL, sp_queue_printstr(NULL));
}
END_TEST

START_TEST(print_bad_elem_size)
{
	struct sp_queue *s;
	ck_assert_ptr_nonnull(s = sp_queue_create(sizeof(int) + 1, 10));
	ck_assert_int_eq(SP_EILLEGAL, sp_queue_printi(s));
	ck_assert_int_eq(0, sp_queue_destroy(s, NULL));

	ck_assert_ptr_nonnull(s = sp_queue_create(sizeof(char*) + 1, 10));
	ck_assert_int_eq(SP_EILLEGAL, sp_queue_printstr(s));
	ck_assert_int_eq(0, sp_queue_destroy(s, NULL));
}
END_TEST

START_TEST(print_bad_callback)
{
	struct data a, b, c;
	setup(struct data, 10);
	data_init(&a);
	data_init(&b);
	data_init(&c);
	ck_assert_int_eq(0, sp_queue_print(s, data_print_bad));
	ck_assert_int_eq(0, sp_queue_push(s, &a));
	ck_assert_int_eq(0, sp_queue_push(s, &b));
	ck_assert_int_eq(0, sp_queue_push(s, &c));
	ck_assert_int_eq(SP_ECALLBK, sp_queue_print(s, data_print_bad));
	teardown(data_dtor);
}
END_TEST

void init_print(Suite *suite, TCase *tc)
{
	suite_add_tcase(suite, tc);
	tcase_add_test(tc, print_basic);
	tcase_add_test(tc, print_object);
	tcase_add_test(tc, print_bad_args);
	tcase_add_test(tc, print_bad_elem_size);
	tcase_add_test(tc, print_bad_callback);
}

#undef setup
#undef teardown
