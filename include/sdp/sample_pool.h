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
 * @brief Adds the specified sdp_datasample to the pool.
 *
 * @param sample The sdp_datasample to add.
 *
 * @return int 0 on success, otherwise a negative error code.
 */
int sdp_sp_add(struct sdp_datasample *sample);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* SDP_SAMPLE_POOL_H_ */
