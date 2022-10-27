/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/ztest.h>
#include <step/step.h>
#include <step/measurement/measurement.h>
#include <step/sample_pool.h>
#include <step/proc_mgr.h>
#include "floatcheck.h"
#include "data.h"

ZTEST_SUITE(tests_sample_pool, NULL, NULL, NULL, NULL, NULL);

ZTEST(tests_sample_pool, test_sp_alloc)
{
	struct step_measurement *mes;
	struct step_measurement *ref = &step_test_mes_dietemp;
	uint16_t payload_len = step_test_mes_dietemp.header.srclen.len;

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

ZTEST(tests_sample_pool, test_sp_alloc_limit)
{
	int rec_size = sizeof(struct step_measurement) +
		       (8 - (sizeof(struct step_measurement) % 8));
	int max_samples = (CONFIG_STEP_POOL_SIZE - sizeof(struct k_heap)) /
			  rec_size;
	struct step_measurement *mes[max_samples];

	zassert_true(step_sp_bytes_alloc() == 0, NULL);

	/* Fill the buffer to the limit. */
	for (int i = 0; i < max_samples - 1; i++) {
		mes[i] = step_sp_alloc(0);
		zassert_not_null(mes[i], NULL);
	}

	zassert_true(step_sp_bytes_alloc() == rec_size * (max_samples - 1), NULL);

	/* Try to allocate one more sample. */
	struct step_measurement *fault_sample = step_sp_alloc(0);
	zassert_is_null(fault_sample, NULL);

	/* Free the allocated samples */
	for (int i = 0; i < max_samples - 1; i++) {
		step_sp_free(mes[i]);
	}
	zassert_true(step_sp_bytes_alloc() == 0, NULL);
}
