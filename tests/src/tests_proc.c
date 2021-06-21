/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <ztest.h>
#include <sys/printk.h>
#include <sdp/sdp.h>
#include <sdp/datasample.h>
#include <sdp/node.h>
#include "floatcheck.h"

void test_proc_sample(void)
{
	int rc;

	rc = 1 - 1;
	zassert_true(rc == 0, NULL);
}

