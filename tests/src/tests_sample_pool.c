/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <ztest.h>
#include <sys/printk.h>
#include <sdp/sdp.h>
#include <sdp/measurement/measurement.h>
#include <sdp/sample_pool.h>
#include "floatcheck.h"
#include "data.h"

void test_sp_alloc(void)
{
	struct sdp_measurement *mes;
	struct sdp_measurement *ref = &sdp_test_mes_dietemp;
	uint16_t payload_len = sdp_test_mes_dietemp.header.srclen.len;

	/* Setup the mutex. */
	sdp_sp_init();

	/* Flush the fifo. */
	sdp_sp_flush();

	/* Allocate a datasample with an too large payload buffer. */
	mes = sdp_sp_alloc(16384);
	zassert_is_null(mes, NULL);

	/* Allocate a datasample with no payload. */
	mes = sdp_sp_alloc(0);
	zassert_not_null(mes, NULL);
	zassert_true(mes->header.srclen.len == 0, NULL);
	zassert_is_null(mes->payload, NULL);
	sdp_sp_free(mes);

	/* Allocate a datasample with an appropriate payload buffer. */
	mes = sdp_sp_alloc(payload_len);
	zassert_not_null(mes, NULL);

	/* Check payload len. */
	zassert_true(mes->header.srclen.len == payload_len, NULL);

	/* Make sure payload is empty. */
	for (size_t i = 0; i < payload_len; i++) {
		printf("%02X ", ((uint8_t *)mes->payload)[i]);
		zassert_true(((uint8_t *)mes->payload)[i] == 0, NULL);
	}

	/* Manually fill sample with reference values to test struct definition. */
	mes->header.filter.base_type = ref->header.filter.base_type;
	mes->header.filter.ext_type = ref->header.filter.ext_type;
	mes->header.filter.flags.compression = ref->header.filter.flags.compression;
	mes->header.filter.flags.data_format = ref->header.filter.flags.data_format;
	mes->header.filter.flags.encoding = ref->header.filter.flags.encoding;
	mes->header.filter.flags.timestamp = ref->header.filter.flags.timestamp;
	mes->header.unit.si_unit = ref->header.unit.si_unit;
	mes->header.unit.ctype = ref->header.unit.ctype;
	mes->header.unit.scale_factor = ref->header.unit.scale_factor;
	mes->header.srclen.fragment = ref->header.srclen.fragment;
	mes->header.srclen.samples = ref->header.srclen.samples;
	mes->header.srclen.len = ref->header.srclen.len;
	mes->header.srclen.sourceid = ref->header.srclen.sourceid;

	/* Copy the test payload. */
	memcpy(mes->payload, ref->payload, payload_len);

	/* Compare payload. */
	zassert_mem_equal(mes->payload, ref->payload, payload_len, NULL);

	/* Free sample memory from pool heap. */
	sdp_sp_free(mes);
}

void test_sp_fifo(void)
{
	struct sdp_measurement *src;
	struct sdp_measurement *dst;
	float payload = 32.0F;

	/* Setup the mutex. */
	sdp_sp_init();

	/* Flush the fifo. */
	sdp_sp_flush();

	/* Check empty fifo. */
	src = sdp_sp_get();
	zassert_is_null(src, NULL);

	/* Allocate a datasample. */
	src = sdp_sp_alloc(sizeof payload);
	zassert_not_null(src, NULL);
	memcpy(src->payload, &payload, sizeof payload);
	src->header.filter.base_type = SDP_MES_TYPE_LIGHT;
	src->header.filter.ext_type = SDP_MES_EXT_TYPE_LIGHT_PHOTO_ILLUMINANCE;
	src->header.unit.si_unit = SDP_MES_UNIT_SI_LUX;
	src->header.unit.ctype = SDP_MES_UNIT_CTYPE_IEEE754_FLOAT32;
	src->header.unit.scale_factor = SDP_MES_SI_SCALE_NONE;

	/* Add to FIFO. */
	sdp_sp_put(src);

	/* Read back from FIFO. */
	dst = sdp_sp_get();
	zassert_not_null(dst, NULL);

	/* Compare src and dst. */
	zassert_mem_equal(src, dst,
			  sizeof(struct sdp_measurement) + sizeof payload, NULL);

	/* Free memory. */
	sdp_sp_free(src);

	/* FIFO should be empty. */
	src = sdp_sp_get();
	zassert_is_null(src, NULL);
}
