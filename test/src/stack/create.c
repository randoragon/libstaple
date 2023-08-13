START_TEST(create_ok)
{
	struct sp_stack *s;
	ck_assert_ptr_nonnull(s = sp_stack_create(5, 10));
	ck_assert_uint_eq(5, s->elem_size);
	ck_assert_uint_eq(0, s->size);
	ck_assert_uint_eq(10, s->capacity);
	ck_assert_int_eq(0, sp_stack_destroy(s, NULL));

	ck_assert_ptr_nonnull(s = sp_stack_create(SIZE_MAX, 1));
	ck_assert_uint_eq(SIZE_MAX, s->elem_size);
	ck_assert_uint_eq(0, s->size);
	ck_assert_uint_eq(1, s->capacity);
	ck_assert_int_eq(0, sp_stack_destroy(s, NULL));

	ck_assert_ptr_nonnull(s = sp_stack_create(1, SIZE_MAX));
	ck_assert_uint_eq(1,  s->elem_size);
	ck_assert_uint_eq(0,  s->size);
	ck_assert_uint_eq(SIZE_MAX, s->capacity);
	ck_assert_int_eq(0, sp_stack_destroy(s, NULL));

	ck_assert_ptr_nonnull(s = sp_stack_create(2, SIZE_MAX / 2));
	ck_assert_uint_eq(2,  s->elem_size);
	ck_assert_uint_eq(0,  s->size);
	ck_assert_uint_eq(SIZE_MAX / 2, s->capacity);
	ck_assert_int_eq(0, sp_stack_destroy(s, NULL));

	ck_assert_ptr_nonnull(s = sp_stack_create(SIZE_MAX / 2, 2));
	ck_assert_uint_eq(SIZE_MAX / 2,  s->elem_size);
	ck_assert_uint_eq(0,  s->size);
	ck_assert_uint_eq(2, s->capacity);
	ck_assert_int_eq(0, sp_stack_destroy(s, NULL));
}
END_TEST

START_TEST(create_buffer_too_big)
{
	ck_assert_ptr_null(sp_stack_create(2, 1 + SIZE_MAX / 2));
	ck_assert_ptr_null(sp_stack_create(SIZE_MAX, SIZE_MAX));
}
END_TEST

START_TEST(create_bad_args)
{
	ck_assert_ptr_null(sp_stack_create(10, 0));
}
END_TEST

START_TEST(create_bool_roundup)
{
	struct sp_stack *s;
	ck_assert_ptr_nonnull(s = sp_stack_create(SP_SIZEOF_BOOL, 1));
	ck_assert_uint_eq(s->capacity, SP_BYTE_SIZE);
	ck_assert_int_eq(0, sp_stack_destroy(s, NULL));
	ck_assert_ptr_nonnull(s = sp_stack_create(SP_SIZEOF_BOOL, SP_BYTE_SIZE));
	ck_assert_uint_eq(s->capacity, SP_BYTE_SIZE);
	ck_assert_int_eq(0, sp_stack_destroy(s, NULL));
	ck_assert_ptr_nonnull(s = sp_stack_create(SP_SIZEOF_BOOL, SP_BYTE_SIZE + 1));
	ck_assert_uint_eq(s->capacity, 2 * SP_BYTE_SIZE);
	ck_assert_int_eq(0, sp_stack_destroy(s, NULL));
}
END_TEST

void init_create(Suite *suite, TCase *tc)
{
	suite_add_tcase(suite, tc);
	tcase_add_test(tc, create_ok);
	tcase_add_test(tc, create_buffer_too_big);
	tcase_add_test(tc, create_bad_args);
	tcase_add_test(tc, create_bool_roundup);
}
