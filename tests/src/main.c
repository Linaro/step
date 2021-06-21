/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <ztest.h>

/* tests_datasample.c */
extern void test_ds_check_header(void);
extern void test_ds_check_payload(void);

/* tests_filter.c */
extern void test_filter_evaluate_fc_null(void);
extern void test_filter_evaluate_ds_null(void);
extern void test_filter_evaluate_fc_len(void);
extern void test_filter_evaluate_match_good(void);
extern void test_filter_evaluate_match_bad(void);

/* tests_proc.c */
extern void test_proc_sample(void);

void test_main(void)
{
	ztest_test_suite(sdp_tests,
		ztest_unit_test(test_ds_check_header),
		ztest_unit_test(test_ds_check_payload),
		ztest_unit_test(test_filter_evaluate_fc_null),
		ztest_unit_test(test_filter_evaluate_ds_null),
		ztest_unit_test(test_filter_evaluate_fc_len),
		ztest_unit_test(test_filter_evaluate_match_good),
		ztest_unit_test(test_filter_evaluate_match_bad),
		ztest_unit_test(test_proc_sample)
	);

	/* Test suites. */
	ztest_run_test_suite(sdp_tests);
}
