/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <logging/log.h>
#include <sys/printk.h>
#include <step/sample_pool.h>
#include <step/measurement/measurement.h>

#define LOG_LEVEL LOG_LEVEL_DBG
LOG_MODULE_REGISTER(sample_pool);

K_FIFO_DEFINE(step_fifo);
K_HEAP_DEFINE(step_elem_pool, CONFIG_STEP_POOL_SIZE);
K_MUTEX_DEFINE(step_sp_alloc_mtx);

/* Sample pool statistics */
struct step_sp_stats {
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
static struct step_sp_stats step_sp_stats_inst = { 0 };

void step_sp_put(struct step_measurement *mes)
{
	step_sp_stats_inst.fifo_put_calls++;
	k_fifo_put(&step_fifo, mes);
}

struct step_measurement *step_sp_get(void)
{
	step_sp_stats_inst.fifo_get_calls++;
	return k_fifo_get(&step_fifo, K_NO_WAIT);
}

void step_sp_free(struct step_measurement *mes)
{
	int len;

	step_sp_stats_inst.pool_free_calls++;

	/* Track memory consumption.
	 * Note that Zephyr's heap stores records in blocks of 8 bytes memory, so
	 * there is some additional overhead when a record isn't an exact multiple
	 * of 8 bytes long. */
	len = mes->header.srclen.len +
	      sizeof(struct step_measurement) + ((mes->header.srclen.len +
						 sizeof(struct step_measurement)) % 8);
	step_sp_stats_inst.bytes_alloc -= len;
	step_sp_stats_inst.bytes_freed_total += len;


	/* Free memory in heap. */
	k_heap_free(&step_elem_pool, mes);
}

void step_sp_flush(void)
{
	struct step_measurement *mes;

	step_sp_stats_inst.pool_flush_calls++;

	/* ToDo: This is problematic since it only frees samples that have
	 * been pushed to the FIFO. Measurements allocated from heap but never
	 * pushed to the FIFO will not be freed. */
	do {
		mes = step_sp_get();
		if (mes) {
			step_sp_free(mes);
		}
	} while (mes != NULL);

	/* Warn if any memory is still consumed after flusing the meas. heap. */
	if (step_sp_stats_inst.bytes_alloc) {
		LOG_DBG("%d bytes left after flushing!", step_sp_stats_inst.bytes_alloc);
	}
}

struct step_measurement *step_sp_alloc(uint16_t sz)
{
	int len;
	struct step_measurement *mes;

	step_sp_stats_inst.pool_alloc_calls++;

	k_mutex_lock(&step_sp_alloc_mtx, K_FOREVER);
	mes = k_heap_alloc(&step_elem_pool,
			   sizeof(struct step_measurement) + sz,
			   K_NO_WAIT);
	k_mutex_unlock(&step_sp_alloc_mtx);

	/* Make sure memory is available. */
	if (mes == NULL) {
		LOG_ERR("memory allocation failed!");
		return NULL;
	}

	/* Track memory consumption.
	 * Note that Zephyr's heap stores records in blocks of 8 bytes memory, so
	 * there is some additional overhead when a record isn't an exact multiple
	 * of 8 bytes long. */
	len = sz + sizeof(struct step_measurement) +
	      ((sz + sizeof(struct step_measurement)) % 8);
	step_sp_stats_inst.bytes_alloc += len;
	step_sp_stats_inst.bytes_alloc_total += len;

	/* Put the allocated struct in default state, and setup payload pointer. */
	memset(mes, 0, sizeof(struct step_mes_header));
	mes->header.srclen.len = sz;
	mes->payload = NULL;
	if (sz) {
		/* Payload starts just after the sample struct. */
		mes->payload = mes + sizeof(struct step_measurement);
		memset(mes->payload, 0, sz);
	}

	return mes;
}

int32_t step_sp_bytes_alloc(void)
{
	return step_sp_stats_inst.bytes_alloc;
}

void step_sp_print_stats(void)
{
	printk("bytes_alloc (cur): %d\n", step_sp_stats_inst.bytes_alloc);
	printk("bytes_alloc_total: %d\n", step_sp_stats_inst.bytes_alloc_total);
	printk("bytes_freed_total: %d\n", step_sp_stats_inst.bytes_freed_total);
	printk("fifo_put_calls:    %d\n", step_sp_stats_inst.fifo_put_calls);
	printk("fifo_get_calls:    %d\n", step_sp_stats_inst.fifo_get_calls);
	printk("pool_free_calls:   %d\n", step_sp_stats_inst.pool_free_calls);
	printk("pool_flush_calls:  %d\n", step_sp_stats_inst.pool_flush_calls);
	printk("pool_alloc_calls:  %d\n", step_sp_stats_inst.pool_alloc_calls);
}