#define setup(T, X) \
	struct sp_queue *s; \
	ck_assert_ptr_nonnull(s = sp_queue_create(sizeof(T), X));

#define teardown(D) \
	ck_assert_int_eq(0, sp_queue_destroy(s, D));

START_TEST(push_basic)
{
	setup(int, 30);
	ck_assert_int_eq(0, sp_queue_pushi(s, 1));
	ck_assert_uint_eq(1, s->size);
	ck_assert_int_eq(1, sp_queue_peeki(s));

	ck_assert_int_eq(0, sp_queue_pushi(s, 2));
	ck_assert_uint_eq(2, s->size);
	ck_assert_int_eq(1, sp_queue_peeki(s));
	ck_assert_int_eq(2, sp_queue_geti(s, 1));

	ck_assert_int_eq(0, sp_queue_pushi(s, 3));
	ck_assert_uint_eq(3, s->size);
	ck_assert_int_eq(1, sp_queue_peeki(s));
	ck_assert_int_eq(2, sp_queue_geti(s, 1));
	ck_assert_int_eq(3, sp_queue_geti(s, 2));
	teardown(NULL);
}
END_TEST

START_TEST(push_object)
{
	struct data a, b, c;
	setup(struct data, 30);
	data_init(&a);
	data_init(&b);
	data_init(&c);
	ck_assert_int_eq(0, sp_queue_push(s, &a));
	ck_assert_uint_eq(1, s->size);
	ck_assert_int_eq(0, data_cmp(&a, sp_queue_peek(s)));

	ck_assert_int_eq(0, sp_queue_push(s, &b));
	ck_assert_uint_eq(2, s->size);
	ck_assert_int_eq(0, data_cmp(&a, sp_queue_peek(s)));
	ck_assert_int_eq(0, data_cmp(&b, sp_queue_get(s, 1)));

	ck_assert_int_eq(0, sp_queue_push(s, &c));
	ck_assert_uint_eq(3, s->size);
	ck_assert_int_eq(0, data_cmp(&a, sp_queue_peek(s)));
	ck_assert_int_eq(0, data_cmp(&b, sp_queue_get(s, 1)));
	ck_assert_int_eq(0, data_cmp(&c, sp_queue_get(s, 2)));
	teardown(data_dtor);
}
END_TEST

START_TEST(push_string)
{
	setup(char*, 20);
	ck_assert_int_eq(0, sp_queue_pushstr(s, "first"));
	ck_assert_uint_eq(1, s->size);
	ck_assert_str_eq("first", sp_queue_peekstr(s));

	ck_assert_int_eq(0, sp_queue_pushstr(s, "second"));
	ck_assert_uint_eq(2, s->size);
	ck_assert_str_eq("first", sp_queue_peekstr(s));
	ck_assert_str_eq("second", sp_queue_getstr(s, 1));

	ck_assert_int_eq(0, sp_queue_pushstr(s, "third"));
	ck_assert_uint_eq(3, s->size);
	ck_assert_str_eq("first", sp_queue_peekstr(s));
	ck_assert_str_eq("second", sp_queue_getstr(s, 1));
	ck_assert_str_eq("third", sp_queue_getstr(s, 2));
	teardown(sp_free);
}
END_TEST

START_TEST(push_substring)
{
	setup(char*, 20);
	ck_assert_int_eq(0, sp_queue_pushstrn(s, "first", 0));
	ck_assert_uint_eq(1, s->size);
	ck_assert_str_eq("", sp_queue_peekstr(s));

	ck_assert_int_eq(0, sp_queue_pushstrn(s, "second", 3));
	ck_assert_uint_eq(2, s->size);
	ck_assert_str_eq("", sp_queue_peekstr(s));
	ck_assert_str_eq("sec", sp_queue_getstr(s, 1));

	ck_assert_int_eq(0, sp_queue_pushstrn(s, "third", 5));
	ck_assert_uint_eq(3, s->size);
	ck_assert_str_eq("", sp_queue_peekstr(s));
	ck_assert_str_eq("sec", sp_queue_getstr(s, 1));
	ck_assert_str_eq("third", sp_queue_getstr(s, 2));
	teardown(sp_free);
}
END_TEST

START_TEST(push_bad_args)
{
	struct data a;
	struct sp_queue *s;
	ck_assert_ptr_nonnull(s = sp_queue_create(sizeof(int), 10));
	ck_assert_int_eq(SP_EINVAL, sp_queue_pushi(NULL, 1));
	ck_assert_int_eq(0, sp_queue_destroy(s, NULL));

	data_init(&a);
	ck_assert_ptr_nonnull(s = sp_queue_create(sizeof(struct data), 10));
	ck_assert_int_eq(SP_EINVAL, sp_queue_push(NULL, NULL));
	ck_assert_int_eq(SP_EINVAL, sp_queue_push(s, NULL));
	ck_assert_int_eq(SP_EINVAL, sp_queue_push(NULL, &a));
	ck_assert_int_eq(0, sp_queue_destroy(s, NULL));
	data_dtor(&a);

	ck_assert_ptr_nonnull(s = sp_queue_create(sizeof(char*), 10));
	ck_assert_int_eq(SP_EINVAL, sp_queue_pushstr(NULL, NULL));
	ck_assert_int_eq(SP_EINVAL, sp_queue_pushstr(s, NULL));
	ck_assert_int_eq(SP_EINVAL, sp_queue_pushstr(NULL, "abc"));
	ck_assert_int_eq(SP_EINVAL, sp_queue_pushstrn(NULL, NULL, 0));
	ck_assert_int_eq(SP_EINVAL, sp_queue_pushstrn(s, NULL, 0));
	ck_assert_int_eq(SP_EINVAL, sp_queue_pushstrn(NULL, "abc", 0));
	ck_assert_int_eq(0, sp_queue_destroy(s, NULL));
}
END_TEST

START_TEST(push_bad_elem_size)
{
	struct sp_queue *s;
	ck_assert_ptr_nonnull(s = sp_queue_create(sizeof(int) + 1, 10));
	ck_assert_int_eq(SP_EILLEGAL, sp_queue_pushi(s, 1));
	ck_assert_int_eq(0, sp_queue_destroy(s, NULL));

	ck_assert_ptr_nonnull(s = sp_queue_create(sizeof(char*) + 1, 10));
	ck_assert_int_eq(SP_EILLEGAL, sp_queue_pushstr(s, "abc"));
	ck_assert_int_eq(SP_EILLEGAL, sp_queue_pushstrn(s, "abc", 3));
	ck_assert_int_eq(0, sp_queue_destroy(s, NULL));
}
END_TEST

START_TEST(push_buffer_max_limit)
{
	struct data a;
	struct sp_queue *s;
	size_t i;
	ck_assert_ptr_nonnull(s = sp_queue_create(sizeof(int), 10));
	for (i = 0; i < SIZE_MAX / s->elem_size; i++)
		ck_assert_int_eq(0, sp_queue_pushi(s, i));
	ck_assert_int_eq(SP_ERANGE, sp_queue_pushi(s, 0));
	ck_assert_int_eq(0, sp_queue_destroy(s, NULL));

	data_init(&a);
	ck_assert_ptr_nonnull(s = sp_queue_create(sizeof(struct data), 10));
	for (i = 0; i < SIZE_MAX / s->elem_size; i++)
		ck_assert_int_eq(0, sp_queue_push(s, &a));
	ck_assert_int_eq(SP_ERANGE, sp_queue_push(s, &a));
	ck_assert_int_eq(0, sp_queue_destroy(s, NULL));
	data_dtor(&a);

	ck_assert_ptr_nonnull(s = sp_queue_create(sizeof(char*), 10));
	for (i = 0; i < SIZE_MAX / s->elem_size; i++)
		ck_assert_int_eq(0, sp_queue_pushstr(s, "abc"));
	ck_assert_int_eq(SP_ERANGE, sp_queue_pushstr(s, "abc"));
	ck_assert_int_eq(SP_ERANGE, sp_queue_pushstrn(s, "abc", 2));
	ck_assert_int_eq(0, sp_queue_destroy(s, sp_free));
}
END_TEST

START_TEST(push_string_too_long)
{
	size_t i;
	char *str;
	setup(char*, 10);
	/* SIZE_MAX + 1 is safe, because we redefined SIZE_MAX to be small */
	ck_assert_ptr_nonnull(str = malloc(sizeof(char) * (SIZE_MAX + 1)));
	for (i = 0; i < SIZE_MAX + 1; i++)
		str[i] = ' ';
	str[SIZE_MAX] = '\0';
	ck_assert_int_eq(SP_ERANGE, sp_queue_pushstr(s, str));
	ck_assert_int_eq(SP_ERANGE, sp_queue_pushstrn(s, str, SIZE_MAX));
	free(str);
	teardown(sp_free);
}
END_TEST

void init_push(Suite *suite, TCase *tc)
{
	suite_add_tcase(suite, tc);
	tcase_add_test(tc, push_basic);
	tcase_add_test(tc, push_object);
	tcase_add_test(tc, push_string);
	tcase_add_test(tc, push_substring);
	tcase_add_test(tc, push_bad_args);
	tcase_add_test(tc, push_bad_elem_size);
	tcase_add_test(tc, push_buffer_max_limit);
	tcase_add_test(tc, push_string_too_long);
}

#undef setup
#undef teardown
