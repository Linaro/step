/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/ztest.h>
#include <zephyr/sys/printk.h>
#include <step/step.h>
#include <step/node.h>
#include <step/filter.h>
#include "floatcheck.h"
#include "data.h"

ZTEST_SUITE(tests_filter, NULL, NULL, NULL, NULL, NULL);

ZTEST(tests_filter, test_filter_evaluate_fc_null)
{
	int rc = 0;
	int match;

	/* Clear callback counter stats. */
	memset(&step_test_data_cb_stats, 0,
	       sizeof(struct step_test_data_procnode_cb_stats));

	/* Filter chain null = catch all, should pass. */
	rc = step_filt_evaluate(
		NULL,
		&step_test_mes_dietemp,
		&match);
	zassert_equal(rc, 0, NULL);

	/* Make sure we have a match. */
	zassert_equal(match, 1, NULL);
}

ZTEST(tests_filter, test_filter_evaluate_mes_null)
{
	int rc = 0;
	int match;

	/* Clear callback counter stats. */
	memset(&step_test_data_cb_stats, 0,
	       sizeof(struct step_test_data_procnode_cb_stats));

	/* Message null = error condition. */
	rc = step_filt_evaluate(
		&(step_test_data_procnode.filters),
		NULL,
		&match);
	zassert_equal(rc, -EINVAL, NULL);

	/* Make sure we have a failed match. */
	zassert_equal(match, 0, NULL);
}

ZTEST(tests_filter, test_filter_evaluate_fc_len)
{
	int rc = 0;
	int match;

	int old_count = step_test_data_procnode.filters.count;

	step_test_data_procnode.filters.count = 0;

	/* Clear callback counter stats. */
	memset(&step_test_data_cb_stats, 0,
	       sizeof(struct step_test_data_procnode_cb_stats));

	/* Filter chain len = 0 means catch all, should pass. */
	rc = step_filt_evaluate(
		&(step_test_data_procnode.filters),
		&step_test_mes_dietemp,
		&match);

	/* Reset count. */
	step_test_data_procnode.filters.count = old_count;

	zassert_equal(rc, 0, NULL);
	zassert_equal(match, 1, NULL);
}

ZTEST(tests_filter, test_filter_evaluate_fc_bad_op0)
{
	int rc = 0;
	int match;

	int old_op = step_test_data_procnode.filters.chain[0].op;

	step_test_data_procnode.filters.chain[0].op = STEP_FILTER_OP_OR_NOT;

	/* Clear callback counter stats. */
	memset(&step_test_data_cb_stats, 0,
	       sizeof(struct step_test_data_procnode_cb_stats));

	/* Filter chain op 0 must be IS or NOT. */
	rc = step_filt_evaluate(
		&(step_test_data_procnode.filters),
		&step_test_mes_dietemp,
		&match);

	/* Reset op. */
	step_test_data_procnode.filters.chain[0].op = old_op;

	zassert_equal(rc, -EINVAL, NULL);
	zassert_equal(match, 0, NULL);
}

ZTEST(tests_filter, test_filter_evaluate_match_good)
{
	int rc = 0;
	int match;

	/* Clear callback counter stats. */
	memset(&step_test_data_cb_stats, 0,
	       sizeof(struct step_test_data_procnode_cb_stats));

	/* Evaluate the die temp sample against the test processor node. */
	rc = step_filt_evaluate(
		&(step_test_data_procnode.filters),
		&step_test_mes_dietemp,
		&match);
	zassert_equal(rc, 0, NULL);

	/* Make sure we have a successful match. */
	zassert_equal(match, 1, NULL);

	// /* Matched callback counter should be 1. */
	// zassert_equal(step_test_data_cb_stats.matched, 1, NULL);
}

ZTEST(tests_filter, test_filter_evaluate_match_bad)
{
	/* TODO */

	zassert_equal(1, 1, NULL);
}