#define setup(T, X, Y) \
	struct sp_stack *s1, *s2; \
	ck_assert_ptr_nonnull(s1 = sp_stack_create(sizeof(T), X)); \
	ck_assert_ptr_nonnull(s2 = sp_stack_create(sizeof(T), Y));

#define teardown(D) \
	ck_assert_int_eq(0, sp_stack_destroy(s1, D)); \
	ck_assert_int_eq(0, sp_stack_destroy(s2, D));


START_TEST(copy_empty)
{
	setup(int, 10, 10);
	ck_assert_int_eq(0, sp_stack_copy(s2, s1, NULL));
	ck_assert_uint_eq(0, s1->size);
	ck_assert_uint_eq(0, s2->size);
	teardown(NULL);
}
END_TEST

START_TEST(copy_basic)
{
	setup(int, 10, 10);
	ck_assert_int_eq(0, sp_stack_pushi(s1, 1));
	ck_assert_int_eq(0, sp_stack_pushi(s1, 2));
	ck_assert_int_eq(0, sp_stack_pushi(s1, 3));
	ck_assert_int_eq(0, sp_stack_copy(s2, s1, NULL));
	ck_assert_uint_eq(3, s1->size);
	ck_assert_int_eq(1, sp_stack_eq(s1, s2, NULL));
	teardown(NULL);
}
END_TEST

START_TEST(copy_with_enlargement)
{
	int i;
	setup(int, 1000, 50);
	for (i = 0; i < 420; i++)
		ck_assert_int_eq(0, sp_stack_pushi(s1, i));
	ck_assert_int_eq(0, sp_stack_copy(s2, s1, NULL));
	ck_assert_uint_eq(420, s1->size);
	ck_assert_uint_ge(420, s2->capacity);
	ck_assert_int_eq(1, sp_stack_eq(s1, s2, NULL));
	teardown(NULL);
}
END_TEST

START_TEST(copy_object_empty)
{
	setup(struct data, 10, 10);
	ck_assert_int_eq(0, sp_stack_copy(s2, s1, data_cpy));
	ck_assert_uint_eq(0, s1->size);
	ck_assert_uint_eq(0, s2->size);
	teardown(data_dtor);
}
END_TEST

START_TEST(copy_object_basic)
{
	int i;
	setup(struct data, 10, 10);
	for (i = 0; i < 3; i++) {
		struct data d;
		ck_assert_int_eq(0, data_init(&d));
		ck_assert_int_eq(0, sp_stack_push(s1, &d));
	}
	ck_assert_int_eq(0, sp_stack_copy(s2, s1, data_cpy));
	ck_assert_uint_eq(3, s1->size);
	ck_assert_int_eq(1, sp_stack_eq(s1, s2, data_cmp));
	teardown(data_dtor);
}
END_TEST

START_TEST(copy_object_with_enlargement)
{
	int i;
	setup(struct data, 1000, 50);
	for (i = 0; i < 420; i++) {
		struct data d;
		ck_assert_int_eq(0, data_init(&d));
		ck_assert_int_eq(0, sp_stack_push(s1, &d));
	}
	ck_assert_int_eq(0, sp_stack_copy(s2, s1, data_cpy));
	ck_assert_uint_eq(420, s1->size);
	ck_assert_uint_ge(420, s2->capacity);
	ck_assert_int_eq(1, sp_stack_eq(s1, s2, data_cmp));
	teardown(data_dtor);
}
END_TEST

START_TEST(copy_bad_args)
{
	setup(int, 10, 10);
	ck_assert_int_eq(SP_EINVAL, sp_stack_copy(NULL, NULL, NULL));
	ck_assert_int_eq(SP_EINVAL, sp_stack_copy(s2, NULL, NULL));
	ck_assert_int_eq(SP_EINVAL, sp_stack_copy(NULL, s1, NULL));
	ck_assert_int_eq(SP_EINVAL, sp_stack_copy(NULL, NULL, data_cpy));
	ck_assert_int_eq(SP_EINVAL, sp_stack_copy(s2, NULL, data_cpy));
	ck_assert_int_eq(SP_EINVAL, sp_stack_copy(NULL, s1, data_cpy));
	teardown(NULL);
}
END_TEST

START_TEST(copy_callback)
{
	setup(int, 10, 10);
	ck_assert_int_eq(0, sp_stack_pushi(s1, 21));
	ck_assert_int_eq(SP_ECALLBK, sp_stack_copy(s2, s1, data_cpy_bad));
	teardown(NULL);
}
END_TEST

void init_copy(Suite *suite, TCase *tc)
{
	suite_add_tcase(suite, tc);
	tcase_add_test(tc, copy_empty);
	tcase_add_test(tc, copy_basic);
	tcase_add_test(tc, copy_with_enlargement);
	tcase_add_test(tc, copy_object_empty);
	tcase_add_test(tc, copy_object_basic);
	tcase_add_test(tc, copy_object_with_enlargement);
	tcase_add_test(tc, copy_bad_args);
	tcase_add_test(tc, copy_callback);
}

#undef setup
#undef teardown
