#define setup(T, X) \
	struct sp_queue *s; \
	ck_assert_ptr_nonnull(s = sp_queue_create(sizeof(T), X));

#define teardown(D) \
	ck_assert_int_eq(0, sp_queue_destroy(s, D));

START_TEST(set_basic)
{
	setup(int, 10);
	ck_assert_int_eq(0, sp_queue_pushi(s, 1));
	ck_assert_int_eq(0, sp_queue_seti(s, 0, 2));
	ck_assert_int_eq(2, sp_queue_geti(s, 0));

	ck_assert_int_eq(0, sp_queue_pushi(s, 2));
	ck_assert_int_eq(0, sp_queue_seti(s, 0, 3));
	ck_assert_int_eq(3, sp_queue_geti(s, 0));
	ck_assert_int_eq(2, sp_queue_geti(s, 1));

	ck_assert_int_eq(0, sp_queue_seti(s, 1, 1));
	ck_assert_int_eq(3, sp_queue_geti(s, 0));
	ck_assert_int_eq(1, sp_queue_geti(s, 1));

	ck_assert_int_eq(0, sp_queue_pushi(s, 3));
	ck_assert_int_eq(0, sp_queue_seti(s, 0, 1));
	ck_assert_int_eq(1, sp_queue_geti(s, 0));
	ck_assert_int_eq(1, sp_queue_geti(s, 1));
	ck_assert_int_eq(3, sp_queue_geti(s, 2));

	ck_assert_int_eq(0, sp_queue_seti(s, 1, 2));
	ck_assert_int_eq(1, sp_queue_geti(s, 0));
	ck_assert_int_eq(2, sp_queue_geti(s, 1));
	ck_assert_int_eq(3, sp_queue_geti(s, 2));
	teardown(NULL);
}
END_TEST

START_TEST(set_object)
{
	struct data a, b, c;
	setup(struct data, 10);
	data_init(&a);
	data_init(&b);
	data_init(&c);
	ck_assert_int_eq(0, sp_queue_push(s, &a));
	ck_assert_int_eq(0, sp_queue_set(s, 0, &b));
	ck_assert_int_eq(0, data_cmp(&b, sp_queue_get(s, 0)));

	ck_assert_int_eq(0, sp_queue_push(s, &b));
	ck_assert_int_eq(0, sp_queue_set(s, 0, &c));
	ck_assert_int_eq(0, data_cmp(&c, sp_queue_get(s, 0)));
	ck_assert_int_eq(0, data_cmp(&b, sp_queue_get(s, 1)));

	ck_assert_int_eq(0, sp_queue_set(s, 1, &a));
	ck_assert_int_eq(0, data_cmp(&c, sp_queue_get(s, 0)));
	ck_assert_int_eq(0, data_cmp(&a, sp_queue_get(s, 1)));

	ck_assert_int_eq(0, sp_queue_push(s, &c));
	ck_assert_int_eq(0, sp_queue_set(s, 0, &a));
	ck_assert_int_eq(0, data_cmp(&a, sp_queue_get(s, 0)));
	ck_assert_int_eq(0, data_cmp(&a, sp_queue_get(s, 1)));
	ck_assert_int_eq(0, data_cmp(&c, sp_queue_get(s, 2)));

	ck_assert_int_eq(0, sp_queue_set(s, 1, &b));
	ck_assert_int_eq(0, data_cmp(&a, sp_queue_get(s, 0)));
	ck_assert_int_eq(0, data_cmp(&b, sp_queue_get(s, 1)));
	ck_assert_int_eq(0, data_cmp(&c, sp_queue_get(s, 2)));
	teardown(data_dtor);
}
END_TEST

START_TEST(set_string)
{
	setup(char*, 10);
	ck_assert_int_eq(0, sp_queue_pushstr(s, "first"));
	ck_assert_int_eq(0, sp_queue_setstr(s, 0, "second"));
	ck_assert_str_eq("second", sp_queue_getstr(s, 0));

	ck_assert_int_eq(0, sp_queue_pushstr(s, "second"));
	ck_assert_int_eq(0, sp_queue_setstr(s, 0, "third"));
	ck_assert_str_eq("third", sp_queue_getstr(s, 0));
	ck_assert_str_eq("second", sp_queue_getstr(s, 1));

	ck_assert_int_eq(0, sp_queue_setstr(s, 1, "first"));
	ck_assert_str_eq("third", sp_queue_getstr(s, 0));
	ck_assert_str_eq("first", sp_queue_getstr(s, 1));

	ck_assert_int_eq(0, sp_queue_pushstr(s, "third"));
	ck_assert_int_eq(0, sp_queue_setstr(s, 0, "first"));
	ck_assert_str_eq("first", sp_queue_getstr(s, 0));
	ck_assert_str_eq("first", sp_queue_getstr(s, 1));
	ck_assert_str_eq("third", sp_queue_getstr(s, 2));

	ck_assert_int_eq(0, sp_queue_setstr(s, 1, "second"));
	ck_assert_str_eq("first", sp_queue_getstr(s, 0));
	ck_assert_str_eq("second", sp_queue_getstr(s, 1));
	ck_assert_str_eq("third", sp_queue_getstr(s, 2));
	teardown(sp_free);
}
END_TEST

START_TEST(set_substring)
{
	setup(char*, 10);
	ck_assert_int_eq(0, sp_queue_pushstr(s, "first"));
	ck_assert_int_eq(0, sp_queue_setstrn(s, 0, "second", 3));
	ck_assert_str_eq("sec", sp_queue_getstr(s, 0));

	ck_assert_int_eq(0, sp_queue_pushstr(s, "second"));
	ck_assert_int_eq(0, sp_queue_setstrn(s, 0, "third", 0));
	ck_assert_str_eq("", sp_queue_getstr(s, 0));
	ck_assert_str_eq("second", sp_queue_getstr(s, 1));

	ck_assert_int_eq(0, sp_queue_setstrn(s, 1, "first", 4));
	ck_assert_str_eq("", sp_queue_getstr(s, 0));
	ck_assert_str_eq("firs", sp_queue_getstr(s, 1));

	ck_assert_int_eq(0, sp_queue_pushstr(s, "third"));
	ck_assert_int_eq(0, sp_queue_setstrn(s, 0, "first", 1));
	ck_assert_str_eq("f", sp_queue_getstr(s, 0));
	ck_assert_str_eq("firs", sp_queue_getstr(s, 1));
	ck_assert_str_eq("third", sp_queue_getstr(s, 2));

	ck_assert_int_eq(0, sp_queue_setstrn(s, 1, "second", 6));
	ck_assert_str_eq("f", sp_queue_getstr(s, 0));
	ck_assert_str_eq("second", sp_queue_getstr(s, 1));
	ck_assert_str_eq("third", sp_queue_getstr(s, 2));
	teardown(sp_free);
}
END_TEST

START_TEST(set_bad_args)
{
	struct data a;
	struct sp_queue *s;
	ck_assert_int_eq(SP_EINVAL, sp_queue_seti(NULL, 0, 1));

	data_init(&a);
	ck_assert_ptr_nonnull(s = sp_queue_create(sizeof(struct data), 10));
	ck_assert_int_eq(SP_EINVAL, sp_queue_set(NULL, 0, NULL));
	ck_assert_int_eq(SP_EINVAL, sp_queue_set(NULL, 0, &a));
	ck_assert_int_eq(SP_EINVAL, sp_queue_set(s, 0, NULL));
	ck_assert_int_eq(0, sp_queue_destroy(s, data_dtor));
	data_dtor(&a);

	ck_assert_ptr_nonnull(s = sp_queue_create(sizeof(char*), 10));
	ck_assert_int_eq(SP_EINVAL, sp_queue_setstr(NULL, 0, NULL));
	ck_assert_int_eq(SP_EINVAL, sp_queue_setstr(NULL, 0, "abc"));
	ck_assert_int_eq(SP_EINVAL, sp_queue_setstr(s, 0, NULL));
	ck_assert_int_eq(SP_EINVAL, sp_queue_setstrn(NULL, 0, NULL, 0));
	ck_assert_int_eq(SP_EINVAL, sp_queue_setstrn(NULL, 0, "abc", 0));
	ck_assert_int_eq(SP_EINVAL, sp_queue_setstrn(s, 0, NULL, 0));
	ck_assert_int_eq(0, sp_queue_destroy(s, sp_free));
}
END_TEST

START_TEST(set_bad_index)
{
	struct data a, b, c;
	struct sp_queue *s;
	ck_assert_ptr_nonnull(s = sp_queue_create(sizeof(int), 10));
	ck_assert_int_eq(SP_EINDEX, sp_queue_seti(s, 1, 1));
	ck_assert_int_eq(0, sp_queue_pushi(s, 1));
	ck_assert_int_eq(SP_EINDEX, sp_queue_seti(s, 2, 2));
	ck_assert_int_eq(0, sp_queue_pushi(s, 2));
	ck_assert_int_eq(SP_EINDEX, sp_queue_seti(s, 3, 3));
	ck_assert_int_eq(0, sp_queue_destroy(s, NULL));

	data_init(&a);
	data_init(&b);
	data_init(&c);
	ck_assert_ptr_nonnull(s = sp_queue_create(sizeof(struct data), 10));
	ck_assert_int_eq(SP_EINDEX, sp_queue_set(s, 1, &a));
	ck_assert_int_eq(0, sp_queue_push(s, &a));
	ck_assert_int_eq(SP_EINDEX, sp_queue_set(s, 2, &b));
	ck_assert_int_eq(0, sp_queue_push(s, &b));
	ck_assert_int_eq(SP_EINDEX, sp_queue_set(s, 3, &c));
	ck_assert_int_eq(0, sp_queue_destroy(s, data_dtor));
	data_dtor(&c);

	ck_assert_ptr_nonnull(s = sp_queue_create(sizeof(char*), 10));
	ck_assert_int_eq(SP_EINDEX, sp_queue_setstr(s, 1, "first"));
	ck_assert_int_eq(SP_EINDEX, sp_queue_setstrn(s, 1, "first", 3));
	ck_assert_int_eq(0, sp_queue_pushstr(s, "first"));
	ck_assert_int_eq(SP_EINDEX, sp_queue_setstr(s, 2, "second"));
	ck_assert_int_eq(SP_EINDEX, sp_queue_setstrn(s, 2, "second", 3));
	ck_assert_int_eq(0, sp_queue_pushstr(s, "second"));
	ck_assert_int_eq(SP_EINDEX, sp_queue_setstr(s, 3, "third"));
	ck_assert_int_eq(SP_EINDEX, sp_queue_setstrn(s, 3, "third", 3));
	ck_assert_int_eq(0, sp_queue_destroy(s, sp_free));
}
END_TEST

START_TEST(set_string_too_long)
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

START_TEST(set_bad_elem_size)
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


void init_set(Suite *suite, TCase *tc)
{
	suite_add_tcase(suite, tc);
	tcase_add_test(tc, set_basic);
	tcase_add_test(tc, set_object);
	tcase_add_test(tc, set_string);
	tcase_add_test(tc, set_substring);
	tcase_add_test(tc, set_bad_args);
	tcase_add_test(tc, set_bad_elem_size);
	tcase_add_test(tc, set_bad_index);
	tcase_add_test(tc, set_string_too_long);
}

#undef setup
#undef teardown
