/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <logging/log.h>
#include <sdp/sample_pool.h>
#include <sdp/datasample.h>

#define LOG_LEVEL LOG_LEVEL_DBG
LOG_MODULE_REGISTER(main);

K_FIFO_DEFINE(sp_fifo);
K_HEAP_DEFINE(sp_elem_pool, CONFIG_SDP_SAMPLE_POOL_SIZE);

void sdp_sp_put(struct sdp_datasample *ds)
{
	k_fifo_put(&sp_fifo, ds);
}

struct sdp_datasample *sdp_sp_get(void)
{
	return k_fifo_get(&sp_fifo, K_NO_WAIT);
}

void sdp_sp_free(struct sdp_datasample *ds)
{
	k_heap_free(&sp_elem_pool, ds);
}

void sdp_sp_flush(void)
{
	struct sdp_datasample *ds;

	do {
		ds = sdp_sp_get();
		if (ds) {
			sdp_sp_free(ds);
		}
	} while (ds != NULL);
}

/* TODO: How to handle fragmentation over time due to var. length sample? */
struct sdp_datasample *sdp_sp_alloc(size_t sz)
{
	struct sdp_datasample *ds;

	/* TODO: Validate sz range. */

	ds = k_heap_alloc(&sp_elem_pool,
			  sizeof(struct sdp_datasample) + sz,
			  K_NO_WAIT);
	if (ds == NULL) {
		LOG_ERR("datasample allocation failed!");
		sdp_sp_flush();

		ds = k_heap_alloc(&sp_elem_pool,
				  sizeof(struct sdp_datasample),
				  K_NO_WAIT);
		if (ds == NULL) {
			LOG_ERR("datasample memory corrupted.");
			__ASSERT_NO_MSG(0);
			return NULL;
		}
		return NULL;
	}

	/* Put ds in a sensible default state. */
	memset(ds, 0, sizeof(struct sdp_datasample) + sz);
	ds->header.srclen.len = sz;

	return ds;
}
