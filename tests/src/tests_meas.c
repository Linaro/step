/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <ztest.h>
#include <sys/printk.h>
#include <sdp/sdp.h>
#include <sdp/measurement/measurement.h>
#include "floatcheck.h"
#include "data.h"

void test_mes_check_header(void)
{
	/* Base data type. */
	zassert_equal(sdp_test_mes_dietemp.header.filter.base_type,
		      SDP_MES_TYPE_TEMPERATURE, NULL);

	/* Extended data type. */
	zassert_equal(sdp_test_mes_dietemp.header.filter.ext_type,
		      SDP_MES_EXT_TYPE_TEMP_DIE, NULL);

	/* Data format. */
	zassert_equal(sdp_test_mes_dietemp.header.filter.flags.data_format,
		      SDP_MES_FORMAT_NONE, NULL);

	/* Encoding. */
	zassert_equal(sdp_test_mes_dietemp.header.filter.flags.encoding,
		      SDP_MES_ENCODING_NONE, NULL);

	/* Compression. */
	zassert_equal(sdp_test_mes_dietemp.header.filter.flags.compression,
		      SDP_MES_COMPRESSION_NONE, NULL);

	/* Timestamp format. */
	zassert_equal(sdp_test_mes_dietemp.header.filter.flags.timestamp,
		      SDP_MES_TIMESTAMP_EPOCH_32, NULL);

	/* SI unit type. */
	zassert_equal(sdp_test_mes_dietemp.header.unit.si_unit,
		      SDP_MES_UNIT_SI_DEGREE_CELSIUS, NULL);

	/* Ctype. */
	zassert_equal(sdp_test_mes_dietemp.header.unit.ctype,
		      SDP_MES_UNIT_CTYPE_IEEE754_FLOAT32, NULL);

	/* Scale factor. */
	zassert_equal(sdp_test_mes_dietemp.header.unit.scale_factor,
		      SDP_MES_SI_SCALE_NONE, NULL);

	/* Payload length in bytes. */
	zassert_equal(sdp_test_mes_dietemp.header.srclen.len, 8,
		      NULL);

	/* Partial payload packet? */
	zassert_equal(sdp_test_mes_dietemp.header.srclen.fragment, 0,
		      NULL);

	/* Sample count. */
	zassert_equal(sdp_test_mes_dietemp.header.srclen.samples, 0,
		      NULL);

	/* Source ID from the source manager registry. */
	zassert_equal(sdp_test_mes_dietemp.header.srclen.sourceid, 10,
		      NULL);
}

void test_mes_check_payload(void)
{
	struct payload {
		float temp_c;
		uint32_t timestamp;
	} payload;

	/* Check payload length. */
	zassert_equal(sizeof(payload),
		      sdp_test_mes_dietemp.header.srclen.len, NULL);

	/* Make a copy of the payload. */
	memcpy(&payload,
	       sdp_test_mes_dietemp.payload,
	       sdp_test_mes_dietemp.header.srclen.len);

	/* Verify temperature. */
	zassert_true(val_is_equal(payload.temp_c, 32.0F, 0.0001), NULL);

	/* Verify timestamp. */
	zassert_equal(payload.timestamp, 1624305803, NULL);
}