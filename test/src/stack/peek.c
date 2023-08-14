#define setup(E, C) \
	struct sp_stack *s; \
	ck_assert_ptr_nonnull(s = sp_stack_create(E, C));

#define teardown(D) \
	ck_assert_int_eq(0, sp_stack_destroy(s, D));

START_TEST(peek_basic)
{
	setup(sizeof(int), 10);
	ck_assert_int_eq(0, sp_stack_pushi(s, 1));
	ck_assert_int_eq(1, sp_stack_peeki(s));
	ck_assert_int_eq(1, sp_stack_peeki(s));
	ck_assert_int_eq(0, sp_stack_pushi(s, 2));
	ck_assert_int_eq(2, sp_stack_peeki(s));
	ck_assert_int_eq(2, sp_stack_peeki(s));
	ck_assert_int_eq(0, sp_stack_pushi(s, 3));
	ck_assert_int_eq(3, sp_stack_peeki(s));
	ck_assert_int_eq(3, sp_stack_peeki(s));
	teardown(NULL);
}
END_TEST

START_TEST(peek_bool)
{
	setup(SP_SIZEOF_BOOL, 10);
	ck_assert_int_eq(0, sp_stack_pushb(s, 1));
	ck_assert(sp_stack_peekb(s));
	ck_assert(sp_stack_peekb(s));
	ck_assert_int_eq(0, sp_stack_pushb(s, 0));
	ck_assert(!sp_stack_peekb(s));
	ck_assert(!sp_stack_peekb(s));
	ck_assert_int_eq(0, sp_stack_pushb(s, 1));
	ck_assert(sp_stack_peekb(s));
	ck_assert(sp_stack_peekb(s));
	teardown(NULL);
}
END_TEST

START_TEST(peek_object)
{
	struct data a, b, c;
	setup(sizeof(struct data), 10);
	data_init(&a);
	data_init(&b);
	data_init(&c);
	ck_assert_int_eq(0, sp_stack_push(s, &a));
	ck_assert_int_eq(0, data_cmp(&a, sp_stack_peek(s)));
	ck_assert_int_eq(0, data_cmp(&a, sp_stack_peek(s)));
	ck_assert_int_eq(0, sp_stack_push(s, &b));
	ck_assert_int_eq(0, data_cmp(&b, sp_stack_peek(s)));
	ck_assert_int_eq(0, data_cmp(&b, sp_stack_peek(s)));
	ck_assert_int_eq(0, sp_stack_push(s, &c));
	ck_assert_int_eq(0, data_cmp(&c, sp_stack_peek(s)));
	ck_assert_int_eq(0, data_cmp(&c, sp_stack_peek(s)));
	teardown(data_dtor);
}
END_TEST

START_TEST(peek_string)
{
	setup(sizeof(char*), 10);
	ck_assert_int_eq(0, sp_stack_pushstr(s, "first"));
	ck_assert_str_eq("first", sp_stack_peekstr(s));
	ck_assert_str_eq("first", sp_stack_peekstr(s));
	ck_assert_int_eq(0, sp_stack_pushstr(s, "second"));
	ck_assert_str_eq("second", sp_stack_peekstr(s));
	ck_assert_str_eq("second", sp_stack_peekstr(s));
	ck_assert_int_eq(0, sp_stack_pushstr(s, "third"));
	ck_assert_str_eq("third", sp_stack_peekstr(s));
	ck_assert_str_eq("third", sp_stack_peekstr(s));
	teardown(sp_free);
}
END_TEST

START_TEST(peek_empty)
{
	struct sp_stack *s;
	ck_assert_ptr_nonnull(s = sp_stack_create(sizeof(int), 10));
	ck_assert_int_eq(0, sp_stack_peeki(s));
	ck_assert_int_eq(0, sp_stack_destroy(s, NULL));

	ck_assert_ptr_nonnull(s = sp_stack_create(sizeof(struct data), 10));
	ck_assert_ptr_null(sp_stack_peek(s));
	ck_assert_int_eq(0, sp_stack_destroy(s, data_dtor));

	ck_assert_ptr_nonnull(s = sp_stack_create(sizeof(char*), 10));
	ck_assert_ptr_null(sp_stack_peekstr(s));
	ck_assert_int_eq(0, sp_stack_destroy(s, sp_free));
}
END_TEST

START_TEST(peek_bad_args)
{
	struct data a;
	struct sp_stack *s;
	ck_assert_ptr_nonnull(s = sp_stack_create(sizeof(int), 10));
	ck_assert_int_eq(0, sp_stack_pushi(s, 1));
	ck_assert_int_eq(0, sp_stack_peeki(NULL));
	ck_assert_int_eq(0, sp_stack_destroy(s, NULL));

	data_init(&a);
	ck_assert_ptr_nonnull(s = sp_stack_create(sizeof(struct data), 10));
	ck_assert_int_eq(0, sp_stack_push(s, &a));
	ck_assert_ptr_null(sp_stack_peek(NULL));
	ck_assert_int_eq(0, sp_stack_destroy(s, data_dtor));

	ck_assert_ptr_nonnull(s = sp_stack_create(sizeof(char*), 10));
	ck_assert_int_eq(0, sp_stack_pushstr(s, "abc"));
	ck_assert_ptr_null(sp_stack_peekstr(NULL));
	ck_assert_int_eq(0, sp_stack_destroy(s, sp_free));
}
END_TEST

START_TEST(peek_bad_elem_size)
{
	struct sp_stack *s;
	ck_assert_ptr_nonnull(s = sp_stack_create(1, 10));
	ck_assert_int_eq(0, sp_stack_peeki(s));
	ck_assert_int_eq(0, sp_stack_destroy(s, NULL));

	ck_assert_ptr_nonnull(s = sp_stack_create(1, 10));
	ck_assert_ptr_null(sp_stack_peekstr(s));
	ck_assert_int_eq(0, sp_stack_destroy(s, sp_free));
}
END_TEST

void init_peek(Suite *suite, TCase *tc)
{
	suite_add_tcase(suite, tc);
	tcase_add_test(tc, peek_basic);
	tcase_add_test(tc, peek_bool);
	tcase_add_test(tc, peek_object);
	tcase_add_test(tc, peek_string);
	tcase_add_test(tc, peek_empty);
	tcase_add_test(tc, peek_bad_args);
	tcase_add_test(tc, peek_bad_elem_size);
}

#undef setup
#undef teardown
