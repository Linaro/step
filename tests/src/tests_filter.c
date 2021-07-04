/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <ztest.h>
#include <sys/printk.h>
#include <sdp/sdp.h>
#include <sdp/node.h>
#include <sdp/filter.h>
#include "floatcheck.h"
#include "data.h"

void test_filter_evaluate_fc_null(void)
{
	int rc = 0;
	int match;

	/* Clear callback counter stats. */
	memset(&sdp_test_data_cb_stats, 0,
	       sizeof(struct sdp_test_data_procnode_cb_stats));

	/* Filter chain null = catch all, should pass. */
	rc = sdp_filt_evaluate(
		NULL,
		&sdp_test_mes_dietemp,
		&match);
	zassert_equal(rc, 0, NULL);

	/* Make sure we have a match. */
	zassert_equal(match, 1, NULL);
}

void test_filter_evaluate_mes_null(void)
{
	int rc = 0;
	int match;

	/* Clear callback counter stats. */
	memset(&sdp_test_data_cb_stats, 0,
	       sizeof(struct sdp_test_data_procnode_cb_stats));

	/* Message null = error condition. */
	rc = sdp_filt_evaluate(
		&(sdp_test_data_procnode.filters),
		NULL,
		&match);
	zassert_equal(rc, -EINVAL, NULL);

	/* Make sure we have a failed match. */
	zassert_equal(match, 0, NULL);
}

void test_filter_evaluate_fc_len(void)
{
	int rc = 0;
	int match;

	int old_count = sdp_test_data_procnode.filters.count;

	sdp_test_data_procnode.filters.count = 0;

	/* Clear callback counter stats. */
	memset(&sdp_test_data_cb_stats, 0,
	       sizeof(struct sdp_test_data_procnode_cb_stats));

	/* Filter chain len = 0 means catch all, should pass. */
	rc = sdp_filt_evaluate(
		&(sdp_test_data_procnode.filters),
		&sdp_test_mes_dietemp,
		&match);

	/* Reset count. */
	sdp_test_data_procnode.filters.count = old_count;

	zassert_equal(rc, 0, NULL);
	zassert_equal(match, 1, NULL);
}

void test_filter_evaluate_fc_bad_op0(void)
{
	int rc = 0;
	int match;

	int old_op = sdp_test_data_procnode.filters.chain[0].op;

	sdp_test_data_procnode.filters.chain[0].op = SDP_FILTER_OP_OR_NOT;

	/* Clear callback counter stats. */
	memset(&sdp_test_data_cb_stats, 0,
	       sizeof(struct sdp_test_data_procnode_cb_stats));

	/* Filter chain op 0 must be IS or NOT. */
	rc = sdp_filt_evaluate(
		&(sdp_test_data_procnode.filters),
		&sdp_test_mes_dietemp,
		&match);

	/* Reset op. */
	sdp_test_data_procnode.filters.chain[0].op = old_op;

	zassert_equal(rc, -EINVAL, NULL);
	zassert_equal(match, 0, NULL);
}

void test_filter_evaluate_match_good(void)
{
	int rc = 0;
	int match;

	/* Clear callback counter stats. */
	memset(&sdp_test_data_cb_stats, 0,
	       sizeof(struct sdp_test_data_procnode_cb_stats));

	/* Evaluate the die temp sample against the test processor node. */
	rc = sdp_filt_evaluate(
		&(sdp_test_data_procnode.filters),
		&sdp_test_mes_dietemp,
		&match);
	zassert_equal(rc, 0, NULL);

	/* Make sure we have a successful match. */
	zassert_equal(match, 1, NULL);

	// /* Matched callback counter should be 1. */
	// zassert_equal(sdp_test_data_cb_stats.matched, 1, NULL);
}

void test_filter_evaluate_match_bad(void)
{
	/* TODO */

	zassert_equal(1, 1, NULL);
}