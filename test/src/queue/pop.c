#define setup(T, X) \
	struct sp_queue *s; \
	ck_assert_ptr_nonnull(s = sp_queue_create(sizeof(T), X));

#define teardown(D) \
	ck_assert_int_eq(0, sp_queue_destroy(s, D));

START_TEST(pop_basic)
{
	setup(int, 10);
	ck_assert_int_eq(0, sp_queue_pushi(s, 1));
	ck_assert_int_eq(0, sp_queue_pushi(s, 2));
	ck_assert_int_eq(0, sp_queue_pushi(s, 3));
	ck_assert_int_eq(1, sp_queue_popi(s));
	ck_assert_uint_eq(2, s->size);
	ck_assert_int_eq(2, sp_queue_popi(s));
	ck_assert_uint_eq(1, s->size);
	ck_assert_int_eq(0, sp_queue_pushi(s, 4));
	ck_assert_int_eq(3, sp_queue_popi(s));
	ck_assert_uint_eq(1, s->size);
	ck_assert_int_eq(4, sp_queue_popi(s));
	ck_assert_uint_eq(0, s->size);
	teardown(NULL);
}
END_TEST

START_TEST(pop_object)
{
	struct data a, b, c, d, t;
	setup(struct data, 10);
	data_init(&a);
	data_init(&b);
	data_init(&c);
	data_init(&d);
	ck_assert_int_eq(0, sp_queue_push(s, &a));
	ck_assert_int_eq(0, sp_queue_push(s, &b));
	ck_assert_int_eq(0, sp_queue_push(s, &c));
	ck_assert_int_eq(0, sp_queue_pop(s, &t));
	ck_assert_int_eq(0, data_cmp(&t, &a));
	ck_assert_uint_eq(2, s->size);
	data_dtor(&t);
	ck_assert_int_eq(0, sp_queue_pop(s, &t));
	ck_assert_int_eq(0, data_cmp(&t, &b));
	ck_assert_uint_eq(1, s->size);
	data_dtor(&t);
	ck_assert_int_eq(0, sp_queue_push(s, &d));
	ck_assert_int_eq(0, sp_queue_pop(s, &t));
	ck_assert_int_eq(0, data_cmp(&t, &c));
	ck_assert_uint_eq(1, s->size);
	data_dtor(&t);
	ck_assert_int_eq(0, sp_queue_pop(s, &t));
	ck_assert_int_eq(0, data_cmp(&t, &d));
	ck_assert_uint_eq(0, s->size);
	data_dtor(&t);
	/* Deliberately push unallocated memory, because
	 * it would be lost with the subsequent pop */
	ck_assert_int_eq(0, sp_queue_push(s, &a));
	ck_assert_int_eq(0, sp_queue_pop(s, NULL));
	ck_assert_uint_eq(0, s->size);
	teardown(data_dtor);
}
END_TEST

START_TEST(pop_string)
{
	char *str;
	setup(char*, 10);
	ck_assert_int_eq(0, sp_queue_pushstr(s, "first"));
	ck_assert_int_eq(0, sp_queue_pushstr(s, "second"));
	ck_assert_int_eq(0, sp_queue_pushstr(s, "third"));
	ck_assert_ptr_nonnull(str = sp_queue_popstr(s));
	ck_assert_str_eq("first", str);
	free(str);
	ck_assert_ptr_nonnull(str = sp_queue_popstr(s));
	ck_assert_str_eq("second", str);
	free(str);
	ck_assert_int_eq(0, sp_queue_pushstr(s, "fourth"));
	ck_assert_ptr_nonnull(str = sp_queue_popstr(s));
	ck_assert_str_eq("third", str);
	free(str);
	ck_assert_ptr_nonnull(str = sp_queue_popstr(s));
	ck_assert_str_eq("fourth", str);
	free(str);
	teardown(sp_free);
}
END_TEST

START_TEST(pop_empty)
{
	struct sp_queue *s;
	struct data a;
	ck_assert_ptr_nonnull(s = sp_queue_create(sizeof(int), 20));
	ck_assert_int_eq(0, sp_queue_popi(s));
	ck_assert_int_eq(0, sp_queue_destroy(s, NULL));

	ck_assert_ptr_nonnull(s = sp_queue_create(sizeof(struct data), 20));
	ck_assert_int_eq(SP_EILLEGAL, sp_queue_pop(s, &a));
	ck_assert_int_eq(0, sp_queue_destroy(s, data_dtor));

	ck_assert_ptr_nonnull(s = sp_queue_create(sizeof(char*), 20));
	ck_assert_ptr_null(sp_queue_popstr(s));
	ck_assert_int_eq(0, sp_queue_destroy(s, sp_free));
}
END_TEST

START_TEST(pop_bad_args)
{
	struct data a, b;
	data_init(&a);
	b = a;
	ck_assert_int_eq(0, sp_queue_popi(NULL));
	ck_assert_int_eq(SP_EINVAL, sp_queue_pop(NULL, NULL));
	ck_assert_int_eq(SP_EINVAL, sp_queue_pop(NULL, &a));
	ck_assert_int_eq(0, memcmp(&a, &b, sizeof(struct data)));
	ck_assert_ptr_null(sp_queue_popstr(NULL));
	data_dtor(&a);
}
END_TEST

START_TEST(pop_bad_elem_size)
{
	struct sp_queue *s;
	ck_assert_ptr_nonnull(s = sp_queue_create(1, 20));
	ck_assert_int_eq(0, sp_queue_popi(s));
	ck_assert_int_eq(0, sp_queue_destroy(s, NULL));

	ck_assert_ptr_nonnull(s = sp_queue_create(1, 20));
	ck_assert_ptr_null(sp_queue_popstr(s));
	ck_assert_int_eq(0, sp_queue_destroy(s, sp_free));
}
END_TEST

void init_pop(Suite *suite, TCase *tc)
{
	suite_add_tcase(suite, tc);
	tcase_add_test(tc, pop_basic);
	tcase_add_test(tc, pop_object);
	tcase_add_test(tc, pop_string);
	tcase_add_test(tc, pop_empty);
	tcase_add_test(tc, pop_bad_args);
	tcase_add_test(tc, pop_bad_elem_size);
}

#undef setup
#undef teardown
