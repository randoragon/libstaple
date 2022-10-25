#define setup(T, X) \
	struct sp_queue *s; \
	ck_assert_ptr_nonnull(s = sp_queue_create(sizeof(T), X));

#define teardown(D) \
	ck_assert_int_eq(0, sp_queue_destroy(s, D));

START_TEST(peek_basic)
{
	setup(int, 10);
	ck_assert_int_eq(0, sp_queue_pushi(s, 1));
	ck_assert_int_eq(1, sp_queue_peeki(s));
	ck_assert_int_eq(1, sp_queue_peeki(s));
	ck_assert_int_eq(0, sp_queue_pushi(s, 2));
	ck_assert_int_eq(1, sp_queue_peeki(s));
	ck_assert_int_eq(1, sp_queue_peeki(s));
	ck_assert_int_eq(0, sp_queue_pushi(s, 3));
	ck_assert_int_eq(1, sp_queue_peeki(s));
	ck_assert_int_eq(1, sp_queue_peeki(s));
	teardown(NULL);
}
END_TEST

START_TEST(peek_object)
{
	struct data a, b, c;
	setup(struct data, 10);
	data_init(&a);
	data_init(&b);
	data_init(&c);
	ck_assert_int_eq(0, sp_queue_push(s, &a));
	ck_assert_int_eq(0, data_cmp(&a, sp_queue_peek(s)));
	ck_assert_int_eq(0, data_cmp(&a, sp_queue_peek(s)));
	ck_assert_int_eq(0, sp_queue_push(s, &b));
	ck_assert_int_eq(0, data_cmp(&a, sp_queue_peek(s)));
	ck_assert_int_eq(0, data_cmp(&a, sp_queue_peek(s)));
	ck_assert_int_eq(0, sp_queue_push(s, &c));
	ck_assert_int_eq(0, data_cmp(&a, sp_queue_peek(s)));
	ck_assert_int_eq(0, data_cmp(&a, sp_queue_peek(s)));
	teardown(data_dtor);
}
END_TEST

START_TEST(peek_string)
{
	setup(char*, 10);
	ck_assert_int_eq(0, sp_queue_pushstr(s, "first"));
	ck_assert_str_eq("first", sp_queue_peekstr(s));
	ck_assert_str_eq("first", sp_queue_peekstr(s));
	ck_assert_int_eq(0, sp_queue_pushstr(s, "second"));
	ck_assert_str_eq("first", sp_queue_peekstr(s));
	ck_assert_str_eq("first", sp_queue_peekstr(s));
	ck_assert_int_eq(0, sp_queue_pushstr(s, "third"));
	ck_assert_str_eq("first", sp_queue_peekstr(s));
	ck_assert_str_eq("first", sp_queue_peekstr(s));
	teardown(sp_free);
}
END_TEST

START_TEST(peek_empty)
{
	struct sp_queue *s;
	ck_assert_ptr_nonnull(s = sp_queue_create(sizeof(int), 10));
	ck_assert_int_eq(0, sp_queue_peeki(s));
	ck_assert_int_eq(0, sp_queue_destroy(s, NULL));

	ck_assert_ptr_nonnull(s = sp_queue_create(sizeof(struct data), 10));
	ck_assert_ptr_null(sp_queue_peek(s));
	ck_assert_int_eq(0, sp_queue_destroy(s, data_dtor));

	ck_assert_ptr_nonnull(s = sp_queue_create(sizeof(char*), 10));
	ck_assert_ptr_null(sp_queue_peekstr(s));
	ck_assert_int_eq(0, sp_queue_destroy(s, sp_free));
}
END_TEST

START_TEST(peek_bad_args)
{
	struct data a;
	struct sp_queue *s;
	ck_assert_ptr_nonnull(s = sp_queue_create(sizeof(int), 10));
	ck_assert_int_eq(0, sp_queue_pushi(s, 1));
	ck_assert_int_eq(0, sp_queue_peeki(NULL));
	ck_assert_int_eq(0, sp_queue_destroy(s, NULL));

	data_init(&a);
	ck_assert_ptr_nonnull(s = sp_queue_create(sizeof(struct data), 10));
	ck_assert_int_eq(0, sp_queue_push(s, &a));
	ck_assert_ptr_null(sp_queue_peek(NULL));
	ck_assert_int_eq(0, sp_queue_destroy(s, data_dtor));

	ck_assert_ptr_nonnull(s = sp_queue_create(sizeof(char*), 10));
	ck_assert_int_eq(0, sp_queue_pushstr(s, "abc"));
	ck_assert_ptr_null(sp_queue_peekstr(NULL));
	ck_assert_int_eq(0, sp_queue_destroy(s, sp_free));
}
END_TEST

START_TEST(peek_bad_elem_size)
{
	struct sp_queue *s;
	ck_assert_ptr_nonnull(s = sp_queue_create(1, 10));
	ck_assert_int_eq(0, sp_queue_peeki(s));
	ck_assert_int_eq(0, sp_queue_destroy(s, NULL));

	ck_assert_ptr_nonnull(s = sp_queue_create(1, 10));
	ck_assert_ptr_null(sp_queue_peekstr(s));
	ck_assert_int_eq(0, sp_queue_destroy(s, sp_free));
}
END_TEST

void init_peek(Suite *suite, TCase *tc)
{
	suite_add_tcase(suite, tc);
	tcase_add_test(tc, peek_basic);
	tcase_add_test(tc, peek_object);
	tcase_add_test(tc, peek_string);
	tcase_add_test(tc, peek_empty);
	tcase_add_test(tc, peek_bad_args);
	tcase_add_test(tc, peek_bad_elem_size);
}

#undef setup
#undef teardown
