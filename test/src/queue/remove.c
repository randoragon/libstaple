#define setup(T, X) \
	struct sp_queue *s; \
	ck_assert_ptr_nonnull(s = sp_queue_create(sizeof(T), X));

#define teardown(D) \
	ck_assert_int_eq(0, sp_queue_destroy(s, D));

START_TEST(remove_basic)
{
	setup(int, 15);
	ck_assert_int_eq(0, sp_queue_pushi(s, 1));
	ck_assert_int_eq(1, sp_queue_removei(s, 0));
	ck_assert_uint_eq(0, s->size);

	ck_assert_int_eq(0, sp_queue_pushi(s, 2));
	ck_assert_int_eq(0, sp_queue_pushi(s, 3));
	ck_assert_int_eq(0, sp_queue_pushi(s, 4));
	ck_assert_int_eq(3, sp_queue_removei(s, 1));
	ck_assert_uint_eq(2, s->size);
	ck_assert_int_eq(2, sp_queue_peeki(s));
	ck_assert_int_eq(4, sp_queue_geti(s, 1));

	ck_assert_int_eq(0, sp_queue_pushi(s, 5));
	ck_assert_int_eq(5, sp_queue_removei(s, 2));
	ck_assert_uint_eq(2, s->size);
	ck_assert_int_eq(2, sp_queue_peeki(s));
	ck_assert_int_eq(4, sp_queue_geti(s, 1));
	ck_assert_int_eq(4, sp_queue_removei(s, 1));
	ck_assert_uint_eq(1, s->size);
	ck_assert_int_eq(2, sp_queue_peeki(s));
	ck_assert_int_eq(2, sp_queue_removei(s, 0));
	ck_assert_uint_eq(0, s->size);
	teardown(NULL);
}
END_TEST

START_TEST(remove_object)
{
	struct data a, b, c, d, e, t;
	setup(struct data, 15);
	data_init(&a);
	data_init(&b);
	data_init(&c);
	data_init(&d);
	data_init(&e);
	ck_assert_int_eq(0, sp_queue_push(s, &a));
	ck_assert_int_eq(0, sp_queue_remove(s, 0, &t));
	ck_assert_int_eq(0, data_cmp(&a, &t));
	ck_assert_uint_eq(0, s->size);

	ck_assert_int_eq(0, sp_queue_push(s, &b));
	ck_assert_int_eq(0, sp_queue_push(s, &c));
	ck_assert_int_eq(0, sp_queue_push(s, &d));
	ck_assert_int_eq(0, sp_queue_remove(s, 1, &t));
	ck_assert_int_eq(0, data_cmp(&c, &t));
	ck_assert_uint_eq(2, s->size);
	ck_assert_int_eq(0, data_cmp(&b, sp_queue_peek(s)));
	ck_assert_int_eq(0, data_cmp(&d, sp_queue_get(s, 1)));

	ck_assert_int_eq(0, sp_queue_push(s, &e));
	ck_assert_int_eq(0, sp_queue_remove(s, 2, &t));
	ck_assert_int_eq(0, data_cmp(&e, &t));
	ck_assert_uint_eq(2, s->size);
	ck_assert_int_eq(0, data_cmp(&b, sp_queue_peek(s)));
	ck_assert_int_eq(0, data_cmp(&d, sp_queue_get(s, 1)));
	ck_assert_int_eq(0, sp_queue_remove(s, 1, &t));
	ck_assert_int_eq(0, data_cmp(&d, &t));
	ck_assert_uint_eq(1, s->size);
	ck_assert_int_eq(0, data_cmp(&b, sp_queue_peek(s)));
	ck_assert_int_eq(0, sp_queue_remove(s, 0, &t));
	ck_assert_int_eq(0, data_cmp(&b, &t));
	ck_assert_uint_eq(0, s->size);
	data_dtor(&a);
	data_dtor(&b);
	data_dtor(&c);
	data_dtor(&d);
	data_dtor(&e);
	teardown(NULL);
}
END_TEST

START_TEST(remove_string)
{
	char *str;
	setup(char*, 15);
	ck_assert_int_eq(0, sp_queue_pushstr(s, "first"));
	ck_assert_str_eq("first", str = sp_queue_removestr(s, 0));
	free(str);
	ck_assert_uint_eq(0, s->size);

	ck_assert_int_eq(0, sp_queue_pushstr(s, "second"));
	ck_assert_int_eq(0, sp_queue_pushstr(s, "third"));
	ck_assert_int_eq(0, sp_queue_pushstr(s, "fourth"));
	ck_assert_str_eq("third", str = sp_queue_removestr(s, 1));
	free(str);
	ck_assert_uint_eq(2, s->size);
	ck_assert_str_eq("second", sp_queue_peekstr(s));
	ck_assert_str_eq("fourth", sp_queue_getstr(s, 1));

	ck_assert_int_eq(0, sp_queue_pushstr(s, "fifth"));
	ck_assert_str_eq("fifth", str = sp_queue_removestr(s, 2));
	free(str);
	ck_assert_uint_eq(2, s->size);
	ck_assert_str_eq("second", sp_queue_peekstr(s));
	ck_assert_str_eq("fourth", sp_queue_getstr(s, 1));
	ck_assert_str_eq("fourth", str = sp_queue_removestr(s, 1));
	free(str);
	ck_assert_uint_eq(1, s->size);
	ck_assert_str_eq("second", sp_queue_peekstr(s));
	ck_assert_str_eq("second", str = sp_queue_removestr(s, 0));
	free(str);
	ck_assert_uint_eq(0, s->size);
	teardown(NULL);
}
END_TEST

START_TEST(remove_bad_args)
{
	struct data a, b;
	data_init(&a);
	b = a;
	ck_assert_int_eq(0, sp_queue_removei(NULL, 0));
	ck_assert_int_eq(SP_EINVAL, sp_queue_remove(NULL, 0, NULL));
	ck_assert_int_eq(SP_EINVAL, sp_queue_remove(NULL, 0, &a));
	ck_assert_int_eq(0, memcmp(&a, &b, sizeof(struct data)));
	ck_assert_ptr_null(sp_queue_removestr(NULL, 0));
	data_dtor(&a);
}
END_TEST

START_TEST(remove_bad_elem_size)
{
	struct sp_queue *s;
	ck_assert_ptr_nonnull(s = sp_queue_create(sizeof(int) + 1, 20));
	ck_assert_int_eq(0, sp_queue_removei(s, 0));
	ck_assert_int_eq(0, sp_queue_destroy(s, NULL));

	ck_assert_ptr_nonnull(s = sp_queue_create(sizeof(char*) + 1, 20));
	ck_assert_ptr_null(sp_queue_removestr(s, 0));
	ck_assert_int_eq(0, sp_queue_destroy(s, sp_free));
}
END_TEST

START_TEST(remove_bad_index)
{
	struct sp_queue *s;
	struct data a;
	ck_assert_ptr_nonnull(s = sp_queue_create(sizeof(int), 10));
	ck_assert_int_eq(0, sp_queue_removei(s, 0));
	ck_assert_int_eq(0, sp_queue_pushi(s, 1));
	ck_assert_int_eq(0, sp_queue_removei(s, 1));
	ck_assert_int_eq(0, sp_queue_pushi(s, 2));
	ck_assert_int_eq(0, sp_queue_removei(s, 2));
	ck_assert_int_eq(0, sp_queue_destroy(s, NULL));

	data_init(&a);
	ck_assert_ptr_nonnull(s = sp_queue_create(sizeof(struct data), 10));
	ck_assert_int_eq(SP_EINDEX, sp_queue_remove(s, 0, &a));
	ck_assert_int_eq(0, sp_queue_push(s, &a));
	ck_assert_int_eq(SP_EINDEX, sp_queue_remove(s, 1, &a));
	ck_assert_int_eq(0, sp_queue_push(s, &a));
	ck_assert_int_eq(SP_EINDEX, sp_queue_remove(s, 2, &a));
	ck_assert_int_eq(0, sp_queue_destroy(s, NULL));
	data_dtor(&a);

	ck_assert_ptr_nonnull(s = sp_queue_create(sizeof(char*), 10));
	ck_assert_ptr_null(sp_queue_removestr(s, 0));
	ck_assert_int_eq(0, sp_queue_pushstr(s, "first"));
	ck_assert_ptr_null(sp_queue_removestr(s, 1));
	ck_assert_int_eq(0, sp_queue_pushstr(s, "second"));
	ck_assert_ptr_null(sp_queue_removestr(s, 2));
	ck_assert_int_eq(0, sp_queue_destroy(s, sp_free));
}
END_TEST

void init_remove(Suite *suite, TCase *tc)
{
	suite_add_tcase(suite, tc);
	tcase_add_test(tc, remove_basic);
	tcase_add_test(tc, remove_object);
	tcase_add_test(tc, remove_string);
	tcase_add_test(tc, remove_bad_args);
	tcase_add_test(tc, remove_bad_elem_size);
	tcase_add_test(tc, remove_bad_index);
}

#undef setup
#undef teardown
