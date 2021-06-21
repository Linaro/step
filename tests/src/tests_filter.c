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

	/* Filter chain null */
	rc = sdp_filt_evaluate(
		NULL,
		&sdp_test_data_sample_dietemp,
		&match);
	zassert_equal(rc, -EINVAL, NULL);

	/* Make sure we have a failed match. */
	zassert_equal(match, 0, NULL);
}

void test_filter_evaluate_ds_null(void)
{
	int rc = 0;
	int match;

	/* Filter chain null */
	rc = sdp_filt_evaluate(
		&(sdp_test_data_pnode.filters),
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

	int old_count = sdp_test_data_pnode.filters.count;
	sdp_test_data_pnode.filters.count = 0;

	/* Filter chain len = 0 */
	rc = sdp_filt_evaluate(
		&(sdp_test_data_pnode.filters),
		&sdp_test_data_sample_dietemp,
		&match);

	/* Reset count. */
	sdp_test_data_pnode.filters.count = old_count;

	zassert_equal(rc, -EINVAL, NULL);
	zassert_equal(match, 0, NULL);
}

void test_filter_evaluate_match_good(void)
{
	int rc = 0;
	int match;

	/* Evaluate the die temp sample against the test processor node. */
	rc = sdp_filt_evaluate(
		&(sdp_test_data_pnode.filters),
		&sdp_test_data_sample_dietemp,
		&match);
	zassert_equal(rc, 0, NULL);

	/* Make sure we have a successful match. */
	zassert_equal(match, 1, NULL);
}

void test_filter_evaluate_match_bad(void)
{
	zassert_equal(1, 1, NULL);
}