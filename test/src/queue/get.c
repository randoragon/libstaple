#define setup(T, X) \
	struct sp_queue *s; \
	ck_assert_ptr_nonnull(s = sp_queue_create(sizeof(T), X));

#define teardown(D) \
	ck_assert_int_eq(0, sp_queue_destroy(s, D));

START_TEST(get_basic)
{
	setup(int, 10);
	ck_assert_int_eq(0, sp_queue_pushi(s, 1));
	ck_assert_int_eq(1, sp_queue_geti(s, 0));

	ck_assert_int_eq(0, sp_queue_pushi(s, 2));
	ck_assert_int_eq(1, sp_queue_geti(s, 0));
	ck_assert_int_eq(2, sp_queue_geti(s, 1));

	ck_assert_int_eq(0, sp_queue_pushi(s, 3));
	ck_assert_int_eq(1, sp_queue_geti(s, 0));
	ck_assert_int_eq(2, sp_queue_geti(s, 1));
	ck_assert_int_eq(3, sp_queue_geti(s, 2));
	teardown(NULL);
}
END_TEST

START_TEST(get_object)
{
	struct data a, b, c;
	setup(struct data, 10);
	data_init(&a);
	data_init(&b);
	data_init(&c);
	ck_assert_int_eq(0, sp_queue_push(s, &a));
	ck_assert_int_eq(0, data_cmp(&a, sp_queue_get(s, 0)));

	ck_assert_int_eq(0, sp_queue_push(s, &b));
	ck_assert_int_eq(0, data_cmp(&a, sp_queue_get(s, 0)));
	ck_assert_int_eq(0, data_cmp(&b, sp_queue_get(s, 1)));

	ck_assert_int_eq(0, sp_queue_push(s, &c));
	ck_assert_int_eq(0, data_cmp(&a, sp_queue_get(s, 0)));
	ck_assert_int_eq(0, data_cmp(&b, sp_queue_get(s, 1)));
	ck_assert_int_eq(0, data_cmp(&c, sp_queue_get(s, 2)));
	teardown(data_dtor);
}
END_TEST

START_TEST(get_string)
{
	setup(char*, 10);
	ck_assert_int_eq(0, sp_queue_pushstr(s, "first"));
	ck_assert_str_eq("first", sp_queue_getstr(s, 0));

	ck_assert_int_eq(0, sp_queue_pushstr(s, "second"));
	ck_assert_str_eq("first", sp_queue_getstr(s, 0));
	ck_assert_str_eq("second", sp_queue_getstr(s, 1));

	ck_assert_int_eq(0, sp_queue_pushstr(s, "third"));
	ck_assert_str_eq("first", sp_queue_getstr(s, 0));
	ck_assert_str_eq("second", sp_queue_getstr(s, 1));
	ck_assert_str_eq("third", sp_queue_getstr(s, 2));
	teardown(sp_free);
}
END_TEST

START_TEST(get_bad_args)
{
	ck_assert_int_eq(0, sp_queue_geti(NULL, 0));
	ck_assert_ptr_null(sp_queue_get(NULL, 0));
	ck_assert_ptr_null(sp_queue_getstr(NULL, 0));
}
END_TEST

START_TEST(get_bad_elem_size)
{
	struct sp_queue *s;
	ck_assert_ptr_nonnull(s = sp_queue_create(sizeof(int) + 1, 20));
	ck_assert_int_eq(0, sp_queue_geti(s, 0));
	ck_assert_int_eq(0, sp_queue_destroy(s, NULL));

	ck_assert_ptr_nonnull(s = sp_queue_create(sizeof(char*) + 1, 20));
	ck_assert_ptr_null(sp_queue_getstr(s, 0));
	ck_assert_int_eq(0, sp_queue_destroy(s, sp_free));
}
END_TEST

START_TEST(get_bad_index)
{
	struct sp_queue *s;
	struct data a;
	ck_assert_ptr_nonnull(s = sp_queue_create(sizeof(int), 10));
	ck_assert_int_eq(0, sp_queue_geti(s, 0));
	ck_assert_int_eq(0, sp_queue_pushi(s, 1));
	ck_assert_int_eq(0, sp_queue_geti(s, 1));
	ck_assert_int_eq(0, sp_queue_pushi(s, 2));
	ck_assert_int_eq(0, sp_queue_geti(s, 2));
	ck_assert_int_eq(0, sp_queue_destroy(s, NULL));

	data_init(&a);
	ck_assert_ptr_nonnull(s = sp_queue_create(sizeof(struct data), 10));
	ck_assert_ptr_null(sp_queue_get(s, 0));
	ck_assert_int_eq(0, sp_queue_push(s, &a));
	ck_assert_ptr_null(sp_queue_get(s, 1));
	ck_assert_int_eq(0, sp_queue_push(s, &a));
	ck_assert_ptr_null(sp_queue_get(s, 2));
	ck_assert_int_eq(0, sp_queue_destroy(s, NULL));
	data_dtor(&a);

	ck_assert_ptr_nonnull(s = sp_queue_create(sizeof(char*), 10));
	ck_assert_ptr_null(sp_queue_getstr(s, 0));
	ck_assert_int_eq(0, sp_queue_pushstr(s, "first"));
	ck_assert_ptr_null(sp_queue_getstr(s, 1));
	ck_assert_int_eq(0, sp_queue_pushstr(s, "second"));
	ck_assert_ptr_null(sp_queue_getstr(s, 2));
	ck_assert_int_eq(0, sp_queue_destroy(s, sp_free));
}
END_TEST

void init_get(Suite *suite, TCase *tc)
{
	suite_add_tcase(suite, tc);
	tcase_add_test(tc, get_basic);
	tcase_add_test(tc, get_object);
	tcase_add_test(tc, get_string);
	tcase_add_test(tc, get_bad_args);
	tcase_add_test(tc, get_bad_elem_size);
	tcase_add_test(tc, get_bad_index);
}

#undef setup
#undef teardown
