/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <ztest.h>
#include <sys/printk.h>
#include <step/step.h>
#include <step/measurement/measurement.h>
#include "floatcheck.h"
#include "data.h"

void test_mes_check_header(void)
{
	/* Base data type. */
	zassert_equal(step_test_mes_dietemp.header.filter.base_type,
		      STEP_MES_TYPE_TEMPERATURE, NULL);

	/* Extended data type. */
	zassert_equal(step_test_mes_dietemp.header.filter.ext_type,
		      STEP_MES_EXT_TYPE_TEMP_DIE, NULL);

	/* Data format. */
	zassert_equal(step_test_mes_dietemp.header.filter.flags.data_format,
		      STEP_MES_FORMAT_NONE, NULL);

	/* Encoding. */
	zassert_equal(step_test_mes_dietemp.header.filter.flags.encoding,
		      STEP_MES_ENCODING_NONE, NULL);

	/* Compression. */
	zassert_equal(step_test_mes_dietemp.header.filter.flags.compression,
		      STEP_MES_COMPRESSION_NONE, NULL);

	/* Timestamp format. */
	zassert_equal(step_test_mes_dietemp.header.filter.flags.timestamp,
		      STEP_MES_TIMESTAMP_EPOCH_32, NULL);

	/* SI unit type. */
	zassert_equal(step_test_mes_dietemp.header.unit.si_unit,
		      STEP_MES_UNIT_SI_DEGREE_CELSIUS, NULL);

	/* Ctype. */
	zassert_equal(step_test_mes_dietemp.header.unit.ctype,
		      STEP_MES_UNIT_CTYPE_IEEE754_FLOAT32, NULL);

	/* Scale factor. */
	zassert_equal(step_test_mes_dietemp.header.unit.scale_factor,
		      STEP_MES_SI_SCALE_NONE, NULL);

	/* Payload length in bytes. */
	zassert_equal(step_test_mes_dietemp.header.srclen.len, 20,
		      NULL);

	/* Partial payload packet? */
	zassert_equal(step_test_mes_dietemp.header.srclen.fragment, 0,
		      NULL);

	/* Sample count (2^2 = 4 samples). */
	zassert_equal(step_test_mes_dietemp.header.srclen.samples, 2,
		      NULL);

	/* Source ID from the source manager registry. */
	zassert_equal(step_test_mes_dietemp.header.srclen.sourceid, 10,
		      NULL);
}

void test_mes_check_payload(void)
{
	struct payload {
		uint32_t timestamp;
		float temp_c[4];
	} *payload;

	/* Check payload length. */
	zassert_equal(sizeof(struct payload),
		      step_test_mes_dietemp.header.srclen.len, NULL);

	/* Get a reference to the payload. */
	payload = step_test_mes_dietemp.payload;

	/* Verify timestamp. */
	zassert_equal(payload->timestamp, 1624305803, NULL);

	/* Verify temperature. */
	zassert_true(val_is_equal(payload->temp_c[0], 32.0F, 0.0001), NULL);
	zassert_true(val_is_equal(payload->temp_c[1], 36.0F, 0.0001), NULL);
	zassert_true(val_is_equal(payload->temp_c[2], 21.0F, 0.0001), NULL);
	zassert_true(val_is_equal(payload->temp_c[3], -3.6F, 0.0001), NULL);
}

void test_mes_check_sample_count(void)
{
	/* ToDo: Verify arbitrary sample count, etc. */
}

void test_mes_check_payload_sz(void)
{
	int32_t sz;

	sz = step_mes_sz_payload(&(step_test_mes_dietemp.header));
	zassert_equal(sz, 20, NULL);
}
