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
	struct sdp_datasample *ref = &sdp_test_data_sample_dietemp;
	uint16_t payload_len = sdp_test_data_sample_dietemp.header.srclen.len;

	/* Allocate a datasample with an too large payload buffer. */
	ds = sdp_sp_alloc(16384);
	zassert_is_null(ds, NULL);

	/* Allocate a datasample with no payload. */
	ds = sdp_sp_alloc(0);
	zassert_not_null(ds, NULL);
	zassert_true(ds->header.srclen.len == 0, NULL);
	zassert_is_null(ds->payload, NULL);
	sdp_sp_free(ds);

	/* Allocate a datasample with an appropriate payload buffer. */
	ds = sdp_sp_alloc(payload_len);
	zassert_not_null(ds, NULL);

	/* Check payload len. */
	zassert_true(ds->header.srclen.len == payload_len, NULL);

	/* Make sure payload is empty. */
	for (size_t i = 0; i < payload_len; i++) {
		zassert_true(((uint8_t *)ds->payload)[i] == 0, NULL);
	}

	/* Manually fill sample with reference values to test struct definition. */
	ds->header.filter.data_type = ref->header.filter.data_type;
	ds->header.filter.ext_type = ref->header.filter.ext_type;
	ds->header.filter.flags.compression = ref->header.filter.flags.compression;
	ds->header.filter.flags.data_format = ref->header.filter.flags.data_format;
	ds->header.filter.flags.encoding = ref->header.filter.flags.encoding;
	ds->header.filter.flags.timestamp = ref->header.filter.flags.timestamp;
	ds->header.srclen.fragment = ref->header.srclen.fragment;
	ds->header.srclen.len = ref->header.srclen.len;
	ds->header.srclen.sourceid = ref->header.srclen.sourceid;

	/* Copy the test payload. */
	memcpy(ds->payload, ref->payload, payload_len);

	/* Compare header. */
	zassert_mem_equal(&ds->header, &ref->header,
			  sizeof(struct sdp_ds_header), NULL);

	/* Compare payload. */
	zassert_mem_equal(ds->payload, ref->payload, payload_len, NULL);

	/* Free sample memory from pool heap. */
	sdp_sp_free(ds);
}
