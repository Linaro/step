/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <logging/log.h>
#include <sys/printk.h>
#include <sdp/sample_pool.h>
#include <sdp/measurement/measurement.h>

#define LOG_LEVEL LOG_LEVEL_DBG
LOG_MODULE_REGISTER(sample_pool);

K_FIFO_DEFINE(sdp_fifo);
K_HEAP_DEFINE(sdp_elem_pool, CONFIG_SDP_POOL_SIZE);
K_MUTEX_DEFINE(sdp_sp_alloc_mtx);

/* SDP statistics */
struct sdp_sp_stats {
	int32_t bytes_alloc;
	uint32_t bytes_alloc_total;
	uint32_t bytes_freed_total;
	uint32_t fifo_put_calls;
	uint32_t fifo_get_calls;
	uint32_t pool_free_calls;
	uint32_t pool_flush_calls;
	uint32_t pool_alloc_calls;
};

/* Track the number of bytes currently allocated, etc. */
static struct sdp_sp_stats sdp_sp_stats_inst = { 0 };

void sdp_sp_put(struct sdp_measurement *mes)
{
	sdp_sp_stats_inst.fifo_put_calls++;
	k_fifo_put(&sdp_fifo, mes);
}

struct sdp_measurement *sdp_sp_get(void)
{
	sdp_sp_stats_inst.fifo_get_calls++;
	return k_fifo_get(&sdp_fifo, K_NO_WAIT);
}

void sdp_sp_free(struct sdp_measurement *mes)
{
	int len;

	sdp_sp_stats_inst.pool_free_calls++;

	/* Track memory consumption.
	 * Note that Zephyr's heap stores records in blocks of 8 bytes memory, so
	 * there is some additional overhead when a record isn't an exact multiple
	 * of 8 bytes long. */
	len = mes->header.srclen.len +
	      sizeof(struct sdp_measurement) + ((mes->header.srclen.len +
						 sizeof(struct sdp_measurement)) % 8);
	sdp_sp_stats_inst.bytes_alloc -= len;
	sdp_sp_stats_inst.bytes_freed_total += len;


	/* Free memory in heap. */
	k_heap_free(&sdp_elem_pool, mes);
}

void sdp_sp_flush(void)
{
	struct sdp_measurement *mes;

	sdp_sp_stats_inst.pool_flush_calls++;

	/* ToDo: This is problematic since it only frees samples that have
	 * been pushed to the FIFO. Measurements allocated from heap but never
	 * pushed to the FIFO will not be freed. */
	do {
		mes = sdp_sp_get();
		if (mes) {
			sdp_sp_free(mes);
		}
	} while (mes != NULL);

	/* Warn if any memory is still consumed after flusing the meas. heap. */
	if (sdp_sp_stats_inst.bytes_alloc) {
		LOG_DBG("%d bytes left after flushing!", sdp_sp_stats_inst.bytes_alloc);
	}
}

struct sdp_measurement *sdp_sp_alloc(uint16_t sz)
{
	int len;
	struct sdp_measurement *mes;

	sdp_sp_stats_inst.pool_alloc_calls++;

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
	len = sz + sizeof(struct sdp_measurement) +
	      ((sz + sizeof(struct sdp_measurement)) % 8);
	sdp_sp_stats_inst.bytes_alloc += len;
	sdp_sp_stats_inst.bytes_alloc_total += len;

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
	return sdp_sp_stats_inst.bytes_alloc;
}

void sdp_sp_print_stats(void)
{
	printk("bytes_alloc (cur): %d\n", sdp_sp_stats_inst.bytes_alloc);
	printk("bytes_alloc_total: %d\n", sdp_sp_stats_inst.bytes_alloc_total);
	printk("bytes_freed_total: %d\n", sdp_sp_stats_inst.bytes_freed_total);
	printk("fifo_put_calls:    %d\n", sdp_sp_stats_inst.fifo_put_calls);
	printk("fifo_get_calls:    %d\n", sdp_sp_stats_inst.fifo_get_calls);
	printk("pool_free_calls:   %d\n", sdp_sp_stats_inst.pool_free_calls);
	printk("pool_flush_calls:  %d\n", sdp_sp_stats_inst.pool_flush_calls);
	printk("pool_alloc_calls:  %d\n", sdp_sp_stats_inst.pool_alloc_calls);
}