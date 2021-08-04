/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <ztest.h>
#include <step/step.h>
#include <step/measurement/measurement.h>
#include <step/sample_pool.h>
#include "floatcheck.h"
#include "data.h"

void test_sp_alloc(void)
{
	struct step_measurement *mes;
	struct step_measurement *ref = &step_test_mes_dietemp;
	uint16_t payload_len = step_test_mes_dietemp.header.srclen.len;

	/* Flush the fifo. */
	step_sp_flush();

	/* Allocate a datasample with an oversized payload buffer. */
	mes = step_sp_alloc(16384);
	zassert_is_null(mes, NULL);

	/* Allocate a datasample with no payload. */
	mes = step_sp_alloc(0);
	zassert_not_null(mes, NULL);
	zassert_true(mes->header.srclen.len == 0, NULL);
	zassert_is_null(mes->payload, NULL);
	zassert_true(step_sp_bytes_alloc() ==
		     sizeof(struct step_measurement) +
		     (8 - (sizeof(struct step_measurement) % 8)), NULL);
	step_sp_free(mes);
	zassert_true(step_sp_bytes_alloc() == 0, NULL);

	/* Allocate a datasample with an appropriate payload buffer. */
	mes = step_sp_alloc(payload_len);
	zassert_not_null(mes, NULL);
	zassert_true(step_sp_bytes_alloc() ==
		     sizeof(struct step_measurement) + payload_len +
		     (8 - ((sizeof(struct step_measurement) + payload_len) % 8)),
		     NULL);

	/* Check payload len. */
	zassert_true(mes->header.srclen.len == payload_len, NULL);

	/* Make sure payload is empty. */
	for (size_t i = 0; i < payload_len; i++) {
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
	step_sp_free(mes);
	zassert_true(step_sp_bytes_alloc() == 0, NULL);
}

void test_sp_alloc_limit(void)
{
	int rec_size = sizeof(struct step_measurement) +
		       (8 - (sizeof(struct step_measurement) % 8));
	int max_samples = (CONFIG_STEP_POOL_SIZE - sizeof(struct k_heap)) /
			  rec_size;
	struct step_measurement *mes;

	/* Flush the heap. */
	step_sp_flush();
	zassert_true(step_sp_bytes_alloc() == 0, NULL);

	/* Fill the buffer to the limit. */
	for (int i = 0; i < max_samples - 1; i++) {
		mes = step_sp_alloc(0);
		zassert_not_null(mes, NULL);
		step_sp_put(mes);
	}
	zassert_true(step_sp_bytes_alloc() == rec_size * (max_samples - 1), NULL);

	/* Try to allocate one more sample. */
	mes = step_sp_alloc(0);
	zassert_is_null(mes, NULL);
	zassert_true(step_sp_bytes_alloc() == rec_size * (max_samples - 1), NULL);

	/* Flush heap memory. */
	step_sp_flush();
	zassert_true(step_sp_bytes_alloc() == 0, NULL);
}

void test_sp_fifo(void)
{
	struct step_measurement *src;
	struct step_measurement *dst;
	float payload = 32.0F;

	/* Flush the fifo. */
	step_sp_flush();

	/* Check empty fifo. */
	src = step_sp_get();
	zassert_is_null(src, NULL);

	/* Allocate a datasample. */
	src = step_sp_alloc(sizeof payload);
	zassert_not_null(src, NULL);
	zassert_true(step_sp_bytes_alloc() ==
		     sizeof(struct step_measurement) + sizeof payload +
		     (8 - ((sizeof(struct step_measurement) + sizeof payload) % 8)),
		     NULL);
	memcpy(src->payload, &payload, sizeof payload);
	src->header.filter.base_type = STEP_MES_TYPE_LIGHT;
	src->header.filter.ext_type = STEP_MES_EXT_TYPE_LIGHT_PHOTO_ILLUMINANCE;
	src->header.unit.si_unit = STEP_MES_UNIT_SI_LUX;
	src->header.unit.ctype = STEP_MES_UNIT_CTYPE_IEEE754_FLOAT32;
	src->header.unit.scale_factor = STEP_MES_SI_SCALE_NONE;

	/* Add to FIFO. */
	step_sp_put(src);

	/* Read back from FIFO. */
	dst = step_sp_get();
	zassert_not_null(dst, NULL);

	/* Compare src and dst. */
	zassert_mem_equal(src, dst,
			  sizeof(struct step_measurement) + sizeof payload, NULL);

	/* Free memory. */
	step_sp_free(src);
	zassert_true(step_sp_bytes_alloc() == 0, NULL);

	/* FIFO should be empty. */
	src = step_sp_get();
	zassert_is_null(src, NULL);
}
