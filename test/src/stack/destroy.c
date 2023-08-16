START_TEST(destroy_ok)
{
	struct sp_stack *s;
	ck_assert_ptr_nonnull(s = sp_stack_create(sizeof(int), 30));
	ck_assert_int_eq(0, sp_stack_pushi(s, 1));
	ck_assert_int_eq(0, sp_stack_pushi(s, 2));
	ck_assert_int_eq(0, sp_stack_pushi(s, 3));
	ck_assert_int_eq(0, sp_stack_destroy(s, NULL));
}
END_TEST

START_TEST(destroy_object)
{
	struct sp_stack *s;
	struct data a, b, c;
	data_init(&a);
	data_init(&b);
	data_init(&c);
	ck_assert_ptr_nonnull(s = sp_stack_create(sizeof(struct data), 5));
	ck_assert_int_eq(0, sp_stack_push(s, &a));
	ck_assert_int_eq(0, sp_stack_push(s, &b));
	ck_assert_int_eq(0, sp_stack_push(s, &c));
	ck_assert_int_eq(0, sp_stack_destroy(s, data_dtor));
}
END_TEST

START_TEST(destroy_bad_args)
{
	ck_assert_int_eq(SP_EINVAL, sp_stack_destroy(NULL, NULL));
	ck_assert_int_eq(SP_EINVAL, sp_stack_destroy(NULL, data_dtor));
}
END_TEST

START_TEST(destroy_bad_dtor)
{
	struct sp_stack *s;
	struct data a;
	data_init(&a);
	ck_assert_ptr_nonnull(s = sp_stack_create(sizeof(struct data), 10));
	ck_assert_int_eq(0, sp_stack_push(s, &a));
	ck_assert_int_eq(SP_ECALLBK, sp_stack_destroy(s, data_dtor_bad));
	ck_assert_int_eq(0, sp_stack_destroy(s, NULL));
	data_dtor(&a);
}
END_TEST

START_TEST(destroy_bool_dtor)
{
	struct sp_stack *s;
	ck_assert_ptr_nonnull(s = sp_stack_create(SP_SIZEOF_BOOL, 10));
	ck_assert_int_eq(0, sp_stack_pushb(s, 1));
	ck_assert_int_eq(SP_EILLEGAL, sp_stack_destroy(s, sp_free));
	ck_assert_int_eq(SP_EILLEGAL, sp_stack_destroy(s, data_dtor));
	ck_assert_int_eq(0, sp_stack_destroy(s, NULL));
}
END_TEST

void init_destroy(Suite *suite, TCase *tc)
{
	suite_add_tcase(suite, tc);
	tcase_add_test(tc, destroy_ok);
	tcase_add_test(tc, destroy_object);
	tcase_add_test(tc, destroy_bad_args);
	tcase_add_test(tc, destroy_bad_dtor);
	tcase_add_test(tc, destroy_bool_dtor);
}
