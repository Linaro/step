/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include <zephyr.h>
#include <sys/printk.h>
#include <step/sample_pool.h>
#include <step/instrumentation.h>
#include "driver.h"

#define DRV_PAYLOAD_SZ      (sizeof(struct drv_payload))
#define DRV_PAYLOAD_SRC_ID  (2)

/**
 * @brief Pre-populated measurement header to copy into allocated measurements.
 */
static struct step_mes_header drv_header = {
	/* Filter word. */
	.filter = {
		.base_type = STEP_MES_TYPE_TEMPERATURE,
		.ext_type = STEP_MES_EXT_TYPE_TEMP_DIE,
		.flags = {
			.data_format = STEP_MES_FORMAT_NONE,
			.encoding = STEP_MES_ENCODING_NONE,
			.compression = STEP_MES_COMPRESSION_NONE,
			.timestamp = STEP_MES_TIMESTAMP_UPTIME_MS_32,
		},
	},
	/* SI Unit word. */
	.unit = {
		.si_unit = STEP_MES_UNIT_SI_DEGREE_CELSIUS,
		.ctype = STEP_MES_UNIT_CTYPE_IEEE754_FLOAT32,
		.scale_factor = STEP_MES_SI_SCALE_NONE,
	},
	/* Source/Len word. */
	.srclen = {
		.len = DRV_PAYLOAD_SZ,
		.fragment = 0,
		.sourceid = DRV_PAYLOAD_SRC_ID,
	}
};

int step_shell_drv_get_mes(struct step_measurement **mes)
{
	int rc = 0;

	/* Allocate measurement from the sample pool. */
	*mes = step_sp_alloc(DRV_PAYLOAD_SZ);
	if (*mes == NULL) {
		rc = -ENOMEM;
		goto err;
	}

	/* Copy the measurement header. */
	(*mes)->header.filter_bits = drv_header.filter_bits;
	(*mes)->header.unit_bits = drv_header.unit_bits;
	(*mes)->header.srclen_bits = drv_header.srclen_bits;

	/* Assign the payload. */
	struct drv_payload *payload = (*mes)->payload;
	payload->timestamp = k_uptime_get_32();
	payload->temp_c = 32.0F;

err:
	return rc;
}
