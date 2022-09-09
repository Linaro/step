/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef STEP_SAMPLE_POOL_H__
#define STEP_SAMPLE_POOL_H__

#include <step/step.h>
#include <step/measurement/measurement.h>

/**
 * @defgroup SAMPLEPOOL Sample Pool Management
 * @ingroup step_api
 * @brief API header file for STEP sample pool
 * 
 * This module provides a means to allocate and free @ref step_measurement
 * instances from a shared memory heap, and to queue measurements for
 * processing in a simple FIFO buffer.
 * 
 * Allocating measurements from heap isn't mandatory, but it does provide a
 * number of benefits with asynchronous processing of measurements, such as
 * efficient use of limited memory, and automatic release of the measurement
 * from memory once processing is complete, meaning the processing state
 * doesn't need to be tracked by the data source.
 * 
 * The heap size is set via KConfig using the CONFIG_STEP_POOL_SIZE property.
 * @{
 */

/**
 * @file
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Adds the specified step_measurement to the pool's FIFO.
 *
 * @param mes The step_measurement to add.
 */
void step_sp_put(struct step_measurement *mes);

/**
 * @brief Gets an step_measurement from the pool's FIFO, or NULL if nothing is
 *        available.
 *
 * @return A pointer to the measurement, or NULL if no measurement could be
 * retrieved.
 */
struct step_measurement *step_sp_get(void);

/**
 * @brief Frees the heap memory associated with 'ds'.
 *
 * @param mes Pointer to the step_measurement whose memory should be freed.
 */
void step_sp_free(struct step_measurement *mes);

/**
 * @brief Reads the entire sample pool FIFO, flushing any step_measurement(s)
 *        found from heap memory. Use with care!
 */
void step_sp_flush(void);

/**
 * @brief Allocates memory for a step_measurement from the sample pool's heap.
 *
 * @param sz Payload size in bytes. If the payload contents will be modified,
 *           make sure to request the maximum required payload size, including
 *           the optional timestamp value.
 *
 * @return A pointer to the measurement, or NULL if sufficient memory could not
 *         be allocated from the heap.
 */
struct step_measurement *step_sp_alloc(uint16_t sz);

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
 *        'CONFIG_STEP_POOL_SIZE - sizeof(struct k_heap)'.
 * 
 * @return int32_t The number of bytes currently allocated.
 */
int32_t step_sp_bytes_alloc(void);

/**
 * @brief Checks how many messages are pending in the sample FIFO.
 *
 * @return positive number indicating the amout of pending messages, 0 otherwise.
 */
uint32_t step_sp_fifo_count(void);

/**
 * @brief Checks if internal FIFO of sample pool has any sample pending.
 *
 * @return true if sample FIFO is empty, otherwise false.
 */
bool step_sp_is_fifo_empty(void);

/**
 * @brief Prints the contents of the statistics struct. Useful for debug
 *        purposes to detect memory leaks, etc.
 */
void step_sp_print_stats(void);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* STEP_SAMPLE_POOL_H_ */
