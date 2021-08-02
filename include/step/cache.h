/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef STEP_CACHE_H__
#define STEP_CACHE_H__

#include <step/step.h>

/**
 * @defgroup CACHE LRU Cache
 * @ingroup step_api
 * @brief API header file for a least recently used (LRU) cache
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Cache record
 */
struct step_cache_rec {
	/**
	 * @brief The input value to be stored in cache.
	 */
	uint32_t input;

	/**
	 * @brief The handle that the uncached input value was validated against.
	 */
	uint32_t handle;

	/**
	 * @brief The results of evaluating the uncached input value against the
	 *        current record's handle.
	 */
	uint32_t result;

	/**
	 * @brief The time that this record was last used. Required to remove
	 *        the least recently used record from cache on overflow.
	 */
	int64_t last_used;
};

/**
 * @brief Prints the current cache contents using printk.
 */
void step_cache_print(void);

/**
 * @brief Prints the current cache statistics using printk.
 */
void step_cache_print_stats(void);

/**
 * @brief Clears all existing records from cache memory.
 */
void step_cache_clear(void);

/**
 * @brief Evaluates the supplied filter and node handle against the cache.
 *
 * This function evaluates the supplied @ref filter and node @ref handle
 * against the cache to determine if there is an existing record that matches
 * the input values. If a record is available in cache memory, the cached
 * results will be assigned to @ref result, and 1 will be returned. If no
 * result is found, @ref result will be set to 0, and 0 will be returned.
 *
 * Calling this function will update the timestamp associated with any matching
 * record in cache, to ensure that the most frequently accessed values remain
 * in cache.
 *
 * @param filter    The input filter value to evalute for a match.
 * @param handle    The node handle to evaluate for a match.
 * @param result 	Pointer to the evaluation result. Set to cached result
 *                  on a match, otherwise 0.
 *
 * @return int 		1 if a match was found in cache, otherwise 0.
 */
int step_cache_check(uint32_t filter, uint32_t handle, int *result);

/**
 * @brief Inserts a new record in cache memory. If cache memory is full, the
 *        least recently used record will be removed from cache memory to make
 *        room for the new record.
 *
 * @param filter    The input filter value to add to cache.
 * @param handle    The node handle to add to cache.
 * @param result 	The evaluation result to add to cache for this filter and
 *                  handle combination.
 *
 * @return int 		Zero on normal execution, otherwise a negative error code.
 */
int step_cache_add(uint32_t filter, uint32_t handle, int result);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* STEP_CACHE_H_ */
