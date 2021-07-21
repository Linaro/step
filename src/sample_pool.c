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
LOG_MODULE_REGISTER(main);

K_FIFO_DEFINE(sdp_fifo);
K_HEAP_DEFINE(sdp_elem_pool, CONFIG_SDP_SAMPLE_POOL_SIZE);

struct k_mutex sdp_sp_alloc_mtx;

/* ToDo: Add variable to track the number of bytes currently allocated. */

int sdp_sp_init(void)
{
	return k_mutex_init(&sdp_sp_alloc_mtx);
}

void sdp_sp_put(struct sdp_measurement *mes)
{
	/* Alloc avoids the need to allocate a leading word in the mes struct. */
	k_fifo_put(&sdp_fifo, mes);
}

struct sdp_measurement *sdp_sp_get(void)
{
	return k_fifo_get(&sdp_fifo, K_NO_WAIT);
}

void sdp_sp_free(struct sdp_measurement *mes)
{
	k_heap_free(&sdp_elem_pool, mes);
}

void sdp_sp_flush(void)
{
	struct sdp_measurement *mes;

	do {
		mes = sdp_sp_get();
		if (mes) {
			sdp_sp_free(mes);
		}
	} while (mes != NULL);
}

/* TODO: How to handle fragmentation over time due to var. length sample? */
struct sdp_measurement *sdp_sp_alloc(uint16_t sz)
{
	struct sdp_measurement *mes;

	k_mutex_lock(&sdp_sp_alloc_mtx, K_FOREVER);
	mes = k_heap_alloc(&sdp_elem_pool,
			   sizeof(struct sdp_measurement) + sz,
			   K_NO_WAIT);
	k_mutex_unlock(&sdp_sp_alloc_mtx);

	/* Make sure memory available. */
	if (mes == NULL) {
		LOG_ERR("memory allocation failed!");
		return NULL;
	}

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
