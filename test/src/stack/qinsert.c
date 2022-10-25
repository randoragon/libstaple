#define setup(T, X) \
	struct sp_stack *s; \
	ck_assert_ptr_nonnull(s = sp_stack_create(sizeof(T), X));

#define teardown(D) \
	ck_assert_int_eq(0, sp_stack_destroy(s, D));

START_TEST(qinsert_basic)
{
	setup(int, 10);
	ck_assert_int_eq(0, sp_stack_qinserti(s, 0, 2));
	ck_assert_uint_eq(1, s->size);
	ck_assert_int_eq(2, sp_stack_peeki(s));

	ck_assert_int_eq(0, sp_stack_qinserti(s, 0, 4));
	ck_assert_uint_eq(2, s->size);
	ck_assert_int_eq(4, sp_stack_peeki(s));
	ck_assert_int_eq(2, sp_stack_geti(s, 1));

	ck_assert_int_eq(0, sp_stack_qinserti(s, 1, 3));
	ck_assert_uint_eq(3, s->size);
	ck_assert_int_eq(4, sp_stack_peeki(s));
	ck_assert_int_eq(3, sp_stack_geti(s, 1));
	ck_assert_int_eq(2, sp_stack_geti(s, 2));

	ck_assert_int_eq(0, sp_stack_qinserti(s, 3, 1));
	ck_assert_uint_eq(4, s->size);
	ck_assert_int_eq(2, sp_stack_peeki(s));
	ck_assert_int_eq(4, sp_stack_geti(s, 1));
	ck_assert_int_eq(3, sp_stack_geti(s, 2));
	ck_assert_int_eq(1, sp_stack_geti(s, 3));
	teardown(NULL);
}
END_TEST

START_TEST(qinsert_object)
{
	struct data a, b, c, d;
	setup(struct data, 10);
	data_init(&a);
	data_init(&b);
	data_init(&c);
	data_init(&d);
	ck_assert_int_eq(0, sp_stack_qinsert(s, 0, &b));
	ck_assert_uint_eq(1, s->size);
	ck_assert_int_eq(0, data_cmp(&b, sp_stack_peek(s)));

	ck_assert_int_eq(0, sp_stack_qinsert(s, 0, &d));
	ck_assert_uint_eq(2, s->size);
	ck_assert_int_eq(0, data_cmp(&d, sp_stack_peek(s)));
	ck_assert_int_eq(0, data_cmp(&b, sp_stack_get(s, 1)));

	ck_assert_int_eq(0, sp_stack_qinsert(s, 1, &c));
	ck_assert_uint_eq(3, s->size);
	ck_assert_int_eq(0, data_cmp(&d, sp_stack_peek(s)));
	ck_assert_int_eq(0, data_cmp(&c, sp_stack_get(s, 1)));
	ck_assert_int_eq(0, data_cmp(&b, sp_stack_get(s, 2)));

	ck_assert_int_eq(0, sp_stack_qinsert(s, 3, &a));
	ck_assert_uint_eq(4, s->size);
	ck_assert_int_eq(0, data_cmp(&b, sp_stack_peek(s)));
	ck_assert_int_eq(0, data_cmp(&d, sp_stack_get(s, 1)));
	ck_assert_int_eq(0, data_cmp(&c, sp_stack_get(s, 2)));
	ck_assert_int_eq(0, data_cmp(&a, sp_stack_get(s, 3)));
	teardown(data_dtor);
}
END_TEST

START_TEST(qinsert_string)
{
	setup(char*, 10);
	ck_assert_int_eq(0, sp_stack_qinsertstr(s, 0, "second"));
	ck_assert_uint_eq(1, s->size);
	ck_assert_str_eq("second", sp_stack_peekstr(s));

	ck_assert_int_eq(0, sp_stack_qinsertstr(s, 0, "fourth"));
	ck_assert_uint_eq(2, s->size);
	ck_assert_str_eq("fourth", sp_stack_peekstr(s));
	ck_assert_str_eq("second", sp_stack_getstr(s, 1));

	ck_assert_int_eq(0, sp_stack_qinsertstr(s, 1, "third"));
	ck_assert_uint_eq(3, s->size);
	ck_assert_str_eq("fourth", sp_stack_peekstr(s));
	ck_assert_str_eq("third", sp_stack_getstr(s, 1));
	ck_assert_str_eq("second", sp_stack_getstr(s, 2));

	ck_assert_int_eq(0, sp_stack_qinsertstr(s, 3, "first"));
	ck_assert_uint_eq(4, s->size);
	ck_assert_str_eq("second", sp_stack_peekstr(s));
	ck_assert_str_eq("fourth", sp_stack_getstr(s, 1));
	ck_assert_str_eq("third", sp_stack_getstr(s, 2));
	ck_assert_str_eq("first", sp_stack_getstr(s, 3));
	teardown(sp_free);
}
END_TEST

START_TEST(qinsert_substring)
{
	setup(char*, 10);
	ck_assert_int_eq(0, sp_stack_qinsertstrn(s, 0, "", 0));
	ck_assert_uint_eq(1, s->size);
	ck_assert_str_eq("", sp_stack_peekstr(s));

	ck_assert_int_eq(0, sp_stack_qinsertstrn(s, 0, "fourth", 3));
	ck_assert_uint_eq(2, s->size);
	ck_assert_str_eq("fou", sp_stack_peekstr(s));
	ck_assert_str_eq("", sp_stack_getstr(s, 1));

	ck_assert_int_eq(0, sp_stack_qinsertstrn(s, 1, "third", 5));
	ck_assert_uint_eq(3, s->size);
	ck_assert_str_eq("fou", sp_stack_peekstr(s));
	ck_assert_str_eq("third", sp_stack_getstr(s, 1));
	ck_assert_str_eq("", sp_stack_getstr(s, 2));

	ck_assert_int_eq(0, sp_stack_qinsertstrn(s, 3, "first", 4));
	ck_assert_uint_eq(4, s->size);
	ck_assert_str_eq("", sp_stack_peekstr(s));
	ck_assert_str_eq("fou", sp_stack_getstr(s, 1));
	ck_assert_str_eq("third", sp_stack_getstr(s, 2));
	ck_assert_str_eq("firs", sp_stack_getstr(s, 3));
	teardown(sp_free);
}
END_TEST

START_TEST(qinsert_bad_args)
{
	struct data a;
	struct sp_stack *s;
	ck_assert_int_eq(SP_EINVAL, sp_stack_qinserti(NULL, 0, 1));

	data_init(&a);
	ck_assert_ptr_nonnull(s = sp_stack_create(sizeof(struct data), 10));
	ck_assert_int_eq(SP_EINVAL, sp_stack_qinsert(NULL, 0, NULL));
	ck_assert_int_eq(SP_EINVAL, sp_stack_qinsert(NULL, 0, &a));
	ck_assert_int_eq(SP_EINVAL, sp_stack_qinsert(s, 0, NULL));
	ck_assert_int_eq(0, sp_stack_destroy(s, data_dtor));
	data_dtor(&a);

	ck_assert_ptr_nonnull(s = sp_stack_create(sizeof(char*), 10));
	ck_assert_int_eq(SP_EINVAL, sp_stack_qinsertstr(NULL, 0, NULL));
	ck_assert_int_eq(SP_EINVAL, sp_stack_qinsertstr(NULL, 0, "abc"));
	ck_assert_int_eq(SP_EINVAL, sp_stack_qinsertstr(s, 0, NULL));
	ck_assert_int_eq(SP_EINVAL, sp_stack_qinsertstrn(NULL, 0, NULL, 0));
	ck_assert_int_eq(SP_EINVAL, sp_stack_qinsertstrn(NULL, 0, "abc", 0));
	ck_assert_int_eq(SP_EINVAL, sp_stack_qinsertstrn(s, 0, NULL, 0));
	ck_assert_int_eq(0, sp_stack_destroy(s, sp_free));
}
END_TEST

START_TEST(qinsert_bad_index)
{
	struct data a, b, c;
	struct sp_stack *s;
	ck_assert_ptr_nonnull(s = sp_stack_create(sizeof(int), 10));
	ck_assert_int_eq(SP_EINDEX, sp_stack_qinserti(s, 1, 1));
	ck_assert_int_eq(0, sp_stack_pushi(s, 1));
	ck_assert_int_eq(SP_EINDEX, sp_stack_qinserti(s, 2, 2));
	ck_assert_int_eq(0, sp_stack_pushi(s, 2));
	ck_assert_int_eq(SP_EINDEX, sp_stack_qinserti(s, 3, 3));
	ck_assert_int_eq(0, sp_stack_destroy(s, NULL));

	data_init(&a);
	data_init(&b);
	data_init(&c);
	ck_assert_ptr_nonnull(s = sp_stack_create(sizeof(struct data), 10));
	ck_assert_int_eq(SP_EINDEX, sp_stack_qinsert(s, 1, &a));
	ck_assert_int_eq(0, sp_stack_push(s, &a));
	ck_assert_int_eq(SP_EINDEX, sp_stack_qinsert(s, 2, &b));
	ck_assert_int_eq(0, sp_stack_push(s, &b));
	ck_assert_int_eq(SP_EINDEX, sp_stack_qinsert(s, 3, &c));
	ck_assert_int_eq(0, sp_stack_destroy(s, data_dtor));
	data_dtor(&c);

	ck_assert_ptr_nonnull(s = sp_stack_create(sizeof(char*), 10));
	ck_assert_int_eq(SP_EINDEX, sp_stack_qinsertstr(s, 1, "first"));
	ck_assert_int_eq(SP_EINDEX, sp_stack_qinsertstrn(s, 1, "first", 3));
	ck_assert_int_eq(0, sp_stack_pushstr(s, "first"));
	ck_assert_int_eq(SP_EINDEX, sp_stack_qinsertstr(s, 2, "second"));
	ck_assert_int_eq(SP_EINDEX, sp_stack_qinsertstrn(s, 2, "second", 3));
	ck_assert_int_eq(0, sp_stack_pushstr(s, "second"));
	ck_assert_int_eq(SP_EINDEX, sp_stack_qinsertstr(s, 3, "third"));
	ck_assert_int_eq(SP_EINDEX, sp_stack_qinsertstrn(s, 3, "third", 3));
	ck_assert_int_eq(0, sp_stack_pushstr(s, "third"));
	ck_assert_int_eq(0, sp_stack_destroy(s, sp_free));
}
END_TEST

START_TEST(qinsert_buffer_max_limit)
{
	struct data a;
	struct sp_stack *s;
	size_t i;
	ck_assert_ptr_nonnull(s = sp_stack_create(sizeof(int), 10));
	for (i = 0; i < SIZE_MAX / s->elem_size; i++)
		ck_assert_int_eq(0, sp_stack_qinserti(s, 0, i));
	ck_assert_int_eq(SP_ERANGE, sp_stack_qinserti(s, 0, 0));
	ck_assert_int_eq(0, sp_stack_destroy(s, NULL));

	data_init(&a);
	ck_assert_ptr_nonnull(s = sp_stack_create(sizeof(struct data), 10));
	for (i = 0; i < SIZE_MAX / s->elem_size; i++)
		ck_assert_int_eq(0, sp_stack_qinsert(s, 0, &a));
	ck_assert_int_eq(SP_ERANGE, sp_stack_qinsert(s, 0, &a));
	ck_assert_int_eq(0, sp_stack_destroy(s, NULL));
	data_dtor(&a);

	ck_assert_ptr_nonnull(s = sp_stack_create(sizeof(char*), 10));
	for (i = 0; i < SIZE_MAX / s->elem_size; i++)
		ck_assert_int_eq(0, sp_stack_qinsertstr(s, 0, "abc"));
	ck_assert_int_eq(SP_ERANGE, sp_stack_qinsertstr(s, 0, "abc"));
	ck_assert_int_eq(SP_ERANGE, sp_stack_qinsertstrn(s, 0, "abc", 2));
	ck_assert_int_eq(0, sp_stack_destroy(s, sp_free));
}
END_TEST

START_TEST(qinsert_string_too_long)
{
	size_t i;
	char *str;
	setup(char*, 10);
	/* SIZE_MAX + 1 is safe, because we redefined SIZE_MAX to be small */
	ck_assert_ptr_nonnull(str = malloc(sizeof(char) * (SIZE_MAX + 1)));
	for (i = 0; i < SIZE_MAX + 1; i++)
		str[i] = ' ';
	str[SIZE_MAX] = '\0';
	ck_assert_int_eq(SP_ERANGE, sp_stack_pushstr(s, str));
	ck_assert_int_eq(SP_ERANGE, sp_stack_pushstrn(s, str, SIZE_MAX));
	free(str);
	teardown(sp_free);
}
END_TEST

START_TEST(qinsert_bad_elem_size)
{
	struct sp_stack *s;
	ck_assert_ptr_nonnull(s = sp_stack_create(sizeof(int) + 1, 10));
	ck_assert_int_eq(SP_EILLEGAL, sp_stack_pushi(s, 1));
	ck_assert_int_eq(0, sp_stack_destroy(s, NULL));

	ck_assert_ptr_nonnull(s = sp_stack_create(sizeof(char*) + 1, 10));
	ck_assert_int_eq(SP_EILLEGAL, sp_stack_pushstr(s, "abc"));
	ck_assert_int_eq(SP_EILLEGAL, sp_stack_pushstrn(s, "abc", 3));
	ck_assert_int_eq(0, sp_stack_destroy(s, NULL));
}
END_TEST


void init_qinsert(Suite *suite, TCase *tc)
{
	suite_add_tcase(suite, tc);
	tcase_add_test(tc, qinsert_basic);
	tcase_add_test(tc, qinsert_object);
	tcase_add_test(tc, qinsert_string);
	tcase_add_test(tc, qinsert_substring);
	tcase_add_test(tc, qinsert_bad_args);
	tcase_add_test(tc, qinsert_bad_index);
	tcase_add_test(tc, qinsert_buffer_max_limit);
	tcase_add_test(tc, qinsert_bad_elem_size);
	tcase_add_test(tc, qinsert_string_too_long);
}

#undef setup
#undef teardown
