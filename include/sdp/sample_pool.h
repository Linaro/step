/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef SDP_SAMPLE_POOL_H__
#define SDP_SAMPLE_POOL_H__

#include <sdp/sdp.h>
#include <sdp/measurement/measurement.h>

/**
 * @defgroup SAMPLEPOOL Sample Pool Management
 * @ingroup sdp_api
 * @brief API header file for SDP sample pool
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Adds the specified sdp_measurement to the pool's FIFO.
 *
 * @param mes The sdp_measurement to add.
 */
void sdp_sp_put(struct sdp_measurement *mes);

/**
 * @brief Gets an sdp_measurement from the pool's FIFO, or NULL if nothing is
 *        available.
 *
 * @return A pointer to the measurement, or NULL if no measurement could be
 * retrieved.
 */
struct sdp_measurement *sdp_sp_get(void);

/**
 * @brief Frees the heap memory associated with 'ds'.
 *
 * @param mes Pointer to the sdp_measurement whose memory should be freed.
 */
void sdp_sp_free(struct sdp_measurement *mes);

/**
 * @brief Reads the entire sample pool FIFO, flushing any sdp_measurement(s)
 *        found from heap memory. Use with care!
 */
void sdp_sp_flush(void);

/**
 * @brief Allocates memory for a sdp_measurement from the sample pool's heap.
 *
 * @param sz Payload size in bytes. If the payload contents will be modified,
 *           make sure to request the maximum required payload size, including
 *           the optional timestamp value.
 *
 * @return A pointer to the measurement, or NULL if sufficient memory could not
 *         be allocated from the heap.
 */
struct sdp_measurement *sdp_sp_alloc(uint16_t sz);

/**
 * @brief Returns the number of bytes currently allocated from the sample
 *        pool's heap memory
 * 
 * @note  Records must be aligned on an 8-byte boundary with Zephyr's heap
 *        implementation, so this value may be larger than expected when
 *        unaligned records are allocated from the heap memory pool.
 * 
 * @note  This value does not take into account the memory taken up by the
 *        @ref k_heap struct, which also comes from the heap memory allocation.
 *        Actual memory available for records is limited to
 *        'CONFIG_SDP_POOL_SIZE - sizeof(struct k_heap)'.
 * 
 * @return int32_t The number of bytes currently allocated.
 */
int32_t sdp_sp_bytes_alloc(void);

/**
 * @brief Prints the contents of the statistics struct. Useful for debug
 *        purposes to detect memory leaks, etc.
 */
void sdp_sp_print_stats(void);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* SDP_SAMPLE_POOL_H_ */
