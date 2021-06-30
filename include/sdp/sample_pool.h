/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef SDP_SAMPLE_POOL_H__
#define SDP_SAMPLE_POOL_H__

#include <sdp/sdp.h>
#include <sdp/datasample.h>

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
 * @brief Adds the specified sdp_datasample to the pool's FIFO.
 *
 * @param sample The sdp_datasample to add.
 */
void sdp_sp_put(struct sdp_datasample *sample);

/**
 * @brief Gets an sdp_datasample from the pool's FIFO, or NULL if nothing is
 *        available.
 *
 * @return A pointer to the datasample, or NULL if no sample could be retrieved.
 */
struct sdp_datasample *sdp_sp_get(void);

/**
 * @brief Frees the heap memory associated with 'ds'.
 * 
 * @param ds Pointer to the sdp_datasample whose memory should be freed.
 */
void sdp_sp_free(struct sdp_datasample *ds);

/**
 * @brief Reads the entire sample pool FIFO, flushing any sdp_datasamples found
 *        from heap memory. Use with care!
 */
void sdp_sp_flush(void);

/**
 * @brief Allocates memory for a sdp_datasample from the sample pool's heap.
 * 
 * @param sz Payload size in bytes. If the payload contents will be modified,
 *           make sure to request the maximum required payload size, including
 *           the optional timestamp value.
 * 
 * @return A pointer to the datasample, or NULL if sufficient memory could not
 *         be allocated from the heap.
 */
struct sdp_datasample *sdp_sp_alloc(uint16_t sz);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* SDP_SAMPLE_POOL_H_ */
