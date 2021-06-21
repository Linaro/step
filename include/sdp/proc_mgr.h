/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef SDP_PROC_MGR_H__
#define SDP_PROC_MGR_H__

#include <sdp/sdp.h>
#include <sdp/node.h>

/**
 * @defgroup PROCMGR Processor node manager
 * @ingroup sdp_api
 * @{
 */

/**
 * @file
 * @brief API header file for SDP processor node manager
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Processes the supplied sdp_datasample.
 *
 * @param sample Pointer to the sdp_datasample to parse/
 *
 * @return int 0 on success, negative error code on failure.
 */
int sdp_pm_process_ds(struct sdp_datasample *sample);

/**
 * @brief Polls the sample pool for any incoming sdp_datasamples to
 *        process, and processes them on a first in, first processed basis.
 *
 * @return int 0 on success, negative error code on failure.
 */
int sdp_pm_poll(void);

/**
 * @brief Registers a new processor node.
 *
 * @param node      The processor node to register with the manager.
 * @param handle    The handle the node has been registered under.
 *
 * @return int  0 on success, negative error code on failure.
 */
int sdp_pm_register(struct sdp_node *node, uint8_t *handle);

/**
 * @brief Displays a list of registered processor nodes.
 * 
 * @return int 
 */
int sdp_pm_list(void);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* SDP_PROC_MGR_H_ */
