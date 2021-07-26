/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <string.h>
#include <sdp/cache.h>

#include <sys/printk.h>

#if CONFIG_SDP_FILTER_CACHE
static struct sdp_cache_rec sdp_cache_recs[CONFIG_SDP_FILTER_CACHE_DEPTH];

/* Sample pool statistics */
struct sdp_cache_stats {
	/**
	 * @brief The number of times 'sdp_cache_clear' has been called.
	 */
	uint32_t clear_calls;

	/**
	 * @brief The number of times 'sdp_cache_check' has been called.
	 */
	uint32_t check_calls;

	/**
	 * @brief The number of times 'sdp_cache_add' has been called.
	 */
	uint32_t add_calls;

	/**
	 * @brief The number of matches.
	 */
	uint32_t matches;

	/**
	 * @brief The number of records removed due to an overflow.
	 */
	uint32_t removals;
};

/* Stats tracking instance for SDP cache. */
static struct sdp_cache_stats sdp_cache_stats_inst = { 0 };

void sdp_cache_print(void)
{
	for (uint32_t i = 0; i < CONFIG_SDP_FILTER_CACHE_DEPTH; i++) {
		if (sdp_cache_recs[i].last_used != 0) {
			printk("%04d: 0x%08X 0x%02d %d (last_used: %" PRId64 ")\n", i,
			       sdp_cache_recs[i].input,
			       sdp_cache_recs[i].handle,
			       sdp_cache_recs[i].result,
			       sdp_cache_recs[i].last_used);
		} else {
			printk("%04d: empty\n", i);
		}
	}
}

void sdp_cache_print_stats(void)
{
	printk("clear calls: %d\n", sdp_cache_stats_inst.clear_calls);
	printk("check calls: %d\n", sdp_cache_stats_inst.check_calls);
	printk("add calls:   %d\n", sdp_cache_stats_inst.add_calls);
	printk("matches:     %d\n", sdp_cache_stats_inst.matches);
	printk("removals:    %d\n", sdp_cache_stats_inst.removals);
}

void sdp_cache_clear(void)
{
	sdp_cache_stats_inst.clear_calls++;

	memset(sdp_cache_recs, 0,
	       sizeof(struct sdp_cache_rec) * CONFIG_SDP_FILTER_CACHE_DEPTH);
}

int sdp_cache_check(uint32_t filter, uint32_t handle, int *result)
{
	int match = 0;

	sdp_cache_stats_inst.check_calls++;

	*result = 0;

	for (uint32_t i = 0; i < CONFIG_SDP_FILTER_CACHE_DEPTH; i++) {
		if ((sdp_cache_recs[i].last_used != 0) &&
		    (sdp_cache_recs[i].input == filter) &&
		    (sdp_cache_recs[i].handle == handle)) {
			match = 1;
			/* Get cached results. */
			*result = sdp_cache_recs[i].result;
			/* Update the last_used timestamp. */
			sdp_cache_recs[i].last_used = k_uptime_get();
			sdp_cache_stats_inst.matches++;
			goto exit;
		}
	}

exit:
	return match;
}

int sdp_cache_add(uint32_t filter, uint32_t handle, int result)
{
	int rc = 0;
	int free = -1;
	int64_t oldest = 0x7FFFFFFFFFFFFFFF;

	sdp_cache_stats_inst.add_calls++;

	/* Scan for a free slot in the cache. */
	for (uint32_t i = 0; i < CONFIG_SDP_FILTER_CACHE_DEPTH; i++) {
		if (sdp_cache_recs[i].last_used == 0) {
			free = i;
			goto insert;
		}
	}

	/* No free slot found, find the least recently used record. */
	for (uint32_t i = 0; i < CONFIG_SDP_FILTER_CACHE_DEPTH; i++) {
		if (sdp_cache_recs[i].last_used < oldest) {
			oldest = sdp_cache_recs[i].last_used;
			free = i;
		}
	}

	/* Free least recently used record. */
	memset(&(sdp_cache_recs[free]), 0, sizeof(struct sdp_cache_rec));

insert:
	sdp_cache_recs[free].input = filter;
	sdp_cache_recs[free].handle = handle;
	sdp_cache_recs[free].result = result;
	sdp_cache_recs[free].last_used = k_uptime_get();

	return rc;
}
#endif
