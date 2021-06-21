/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <ztest.h>
#include <sys/printk.h>
#include <sdp/sdp.h>
#include <sdp/datasample.h>
#include "floatcheck.h"
#include "data.h"

void test_ds_check_header(void)
{
	/* Base data type. */
	zassert_equal(sdp_test_data_sample_dietemp.header.filter.data_type,
		      SDP_DS_TYPE_TEMPERATURE, NULL);

	/* Extended data type. */
	zassert_equal(sdp_test_data_sample_dietemp.header.filter.ext_type,
		      SDP_DS_EXT_TYPE_TEMP_DIE, NULL);

	/* Data format. */
	zassert_equal(sdp_test_data_sample_dietemp.header.filter.flags.data_format,
		      SDP_DS_FORMAT_NONE, NULL);

	/* Encoding. */
	zassert_equal(sdp_test_data_sample_dietemp.header.filter.flags.encoding,
		      SDP_DS_ENCODING_NONE, NULL);

	/* Timestamp format. */
	zassert_equal(sdp_test_data_sample_dietemp.header.filter.flags.timestamp,
		      SDP_DS_TIMESTAMP_EPOCH_32, NULL);

	/* Paylod length in bytes. */
	zassert_equal(sdp_test_data_sample_dietemp.header.srclen.len, 8,
		      NULL);

	/* Partial, non-final payload packet? */
	zassert_equal(sdp_test_data_sample_dietemp.header.srclen.is_partial, 0,
		      NULL);

	/* Source ID from the source manager registry. */
	zassert_equal(sdp_test_data_sample_dietemp.header.srclen.sourceid, 10,
		      NULL);
}

void test_ds_check_payload(void)
{
	struct payload {
		float temp_c;
		uint32_t timestamp;
	} payload;

	/* Check payload length. */
	zassert_equal(sizeof(payload),
		      sdp_test_data_sample_dietemp.header.srclen.len, NULL);

	/* Make a copy of the payload. */
	memcpy(&payload,
	       sdp_test_data_sample_dietemp.payload,
	       sdp_test_data_sample_dietemp.header.srclen.len);

	/* Verify temperature. */
	zassert_true(val_is_equal(payload.temp_c, 32.0F, 0.0001), NULL);

	/* Verify timestamp. */
	zassert_equal(payload.timestamp, 1624305803, NULL);
}