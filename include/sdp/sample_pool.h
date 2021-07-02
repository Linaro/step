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
 * @brief Initialises the sample pool.
 *
 * @return int Returns 0 on success, negative error code on failure.
 */
int sdp_sp_init(void);

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

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* SDP_SAMPLE_POOL_H_ */
