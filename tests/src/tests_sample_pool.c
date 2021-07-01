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

	/* Setup the mutex. */
	sdp_sp_init();

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
	ds->header.unit.si_unit = ref->header.unit.si_unit;
	ds->header.unit.ctype = ref->header.unit.ctype;
	ds->header.unit.scale_factor = ref->header.unit.scale_factor;
	ds->header.srclen.fragment = ref->header.srclen.fragment;
	ds->header.srclen.samples = ref->header.srclen.samples;
	ds->header.srclen.len = ref->header.srclen.len;
	ds->header.srclen.sourceid = ref->header.srclen.sourceid;

	/* Copy the test payload. */
	memcpy(ds->payload, ref->payload, payload_len);

	/* Compare payload. */
	zassert_mem_equal(ds->payload, ref->payload, payload_len, NULL);

	/* Free sample memory from pool heap. */
	sdp_sp_free(ds);
}

void test_sp_fifo(void)
{
	struct sdp_datasample *src;
	struct sdp_datasample *dst;
	float payload = 32.0F;

	/* Check empty fifo. */
	src = sdp_sp_get();
	zassert_is_null(src, NULL);

	/* Allocate a datasample. */
	src = sdp_sp_alloc(sizeof payload);
	zassert_not_null(src, NULL);
	memcpy(src->payload, &payload, sizeof payload);
	src->header.filter.data_type = SDP_DS_TYPE_LIGHT;
	src->header.filter.ext_type = SDP_DS_EXT_TYPE_LIGHT_PHOTO_ILLUMINANCE;
	src->header.unit.si_unit = SDP_DS_UNIT_SI_LUX;
	src->header.unit.ctype = SDP_DS_UNIT_CTYPE_IEEE754_FLOAT32;
	src->header.unit.scale_factor = SDP_DS_SI_SCALE_NONE;

	/* Add to FIFO. */
	sdp_sp_put(src);

	/* Read back from FIFO. */
	dst = sdp_sp_get();

	/* Compare src and dst. */
	zassert_mem_equal(src, dst,
			  sizeof(struct sdp_datasample) + sizeof payload, NULL);

	/* Free memory. */
	sdp_sp_free(src);

	/* FIFO should be empty. */
	src = sdp_sp_get();
	zassert_is_null(src, NULL);
}
