/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <logging/log.h>
#include <sdp/sample_pool.h>
#include <sdp/measurement/measurement.h>

#define LOG_LEVEL LOG_LEVEL_DBG
LOG_MODULE_REGISTER(sample_pool);

K_FIFO_DEFINE(sdp_fifo);
K_HEAP_DEFINE(sdp_elem_pool, CONFIG_SDP_SAMPLE_POOL_SIZE);
K_MUTEX_DEFINE(sdp_sp_alloc_mtx);

/* Variable to track the number of bytes currently allocated. */
static int32_t sdp_sp_bytes_allocated = 0;

void sdp_sp_put(struct sdp_measurement *mes)
{
	k_fifo_put(&sdp_fifo, mes);
}

struct sdp_measurement *sdp_sp_get(void)
{
	return k_fifo_get(&sdp_fifo, K_NO_WAIT);
}

void sdp_sp_free(struct sdp_measurement *mes)
{
	k_heap_free(&sdp_elem_pool, mes);

	/* Track memory consumption.
	 * Note that Zephyr's heap stores records in blocks of 8 bytes memory, so
	 * there is some additional overhead when a record isn't an exact multiple
	 * of 8 bytes long. */
	sdp_sp_bytes_allocated -= mes->header.srclen.len +
				  sizeof(struct sdp_measurement) + ((mes->header.srclen.len +
								     sizeof(struct sdp_measurement)) % 8);
}

void sdp_sp_flush(void)
{
	struct sdp_measurement *mes;

	do {
		/* ToDo: This is problematic since it only frees samples that have
		 * been pushed to the FIFO. Measurements allocated from heap but never
		 * pushed to the FIFO will not be freed. */
		mes = sdp_sp_get();
		if (mes) {
			sdp_sp_free(mes);
		}
	} while (mes != NULL);
}

struct sdp_measurement *sdp_sp_alloc(uint16_t sz)
{
	struct sdp_measurement *mes;

	k_mutex_lock(&sdp_sp_alloc_mtx, K_FOREVER);
	mes = k_heap_alloc(&sdp_elem_pool,
			   sizeof(struct sdp_measurement) + sz,
			   K_NO_WAIT);
	k_mutex_unlock(&sdp_sp_alloc_mtx);

	/* Make sure memory is available. */
	if (mes == NULL) {
		LOG_ERR("memory allocation failed!");
		return NULL;
	}

	/* Track memory consumption.
	 * Note that Zephyr's heap stores records in blocks of 8 bytes memory, so
	 * there is some additional overhead when a record isn't an exact multiple
	 * of 8 bytes long. */
	sdp_sp_bytes_allocated += sz + sizeof(struct sdp_measurement) +
				  ((sz + sizeof(struct sdp_measurement)) % 8);

	/* Put the allocated struct in default state, and setup payload pointer. */
	memset(mes, 0, sizeof(struct sdp_mes_header));
	mes->header.srclen.len = sz;
	mes->payload = NULL;
	if (sz) {
		/* Payload starts just after the sample struct. */
		mes->payload = mes + sizeof(struct sdp_measurement);
		memset(mes->payload, 0, sz);
	}

	return mes;
}

int32_t sdp_sp_bytes_alloc(void)
{
	return sdp_sp_bytes_allocated;
}