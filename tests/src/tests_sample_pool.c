/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <ztest.h>
#include <sys/printk.h>
#include <sdp/sdp.h>
#include <sdp/datasample.h>
#include <sdp/sample_pool.h>
#include "floatcheck.h"
#include "data.h"

void test_sp_alloc(void)
{
	struct sdp_datasample *ds;

	/* Allocate a datasample with a 16-byte buffer. */
	ds = sdp_sp_alloc(16);

	zassert_not_null(ds, NULL);

	/* Check payload len. */
	zassert_true(ds->header.srclen.len == 16, NULL);

	/* Free sample memory. */
	sdp_sp_free(ds);
}
