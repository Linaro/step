/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef STEP_PROC_MGR_H__
#define STEP_PROC_MGR_H__

#include <step/step.h>
#include <step/node.h>

/**
 * @defgroup PROCMGR Processor Node Management
 * @ingroup step_api
 * @brief API header file for STEP processor node manager
 * 
 * This module manages the processor node registry, the evaluation of
 * measurements against records in the node registry, and if enabled the
 * polling thread used to retrieve and process any queued measurements.
 * 
 * It ensures that measurements are evaluated against the registry in the
 * correct order, based on the node or node chain's 'priority' field, and that
 * the @ref step_measurement is freed from the sample pool's heap memory once
 * processing is complete (if requested).
 * 
 * Nodes can be inserted, enabled or disabled at run time or at build time.
 * The maximum number of nodes stored in the registry is set via KConfig with
 * the CONFIG_STEP_PROC_MGR_NODE_LIMIT variable.
 * 
 * The sample rate for thee polling thread that checks the sample pool FIFO for
 * queued messages can be configured via CONFIG_STEP_PROC_MGR_POLL_RATE,
 * setting a value in Hertz. Setting this to 0 disables the polling thread,
 * and measurements will have to be manually processing via a call to
 * @ref step_pm_process
 * 
 * @{
 */

/**
 * @file
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Calllback fired when a node chain is completed.
 *
 * @param mes       The measurement processed by this node.
 * @param handle    The handle the node has been registered under.
 * @param user_data The pointer to the user data passed when callback is called.
 */
typedef void (*node_chain_completed_callback)(struct step_measurement *mes, uint32_t node_handle, void* user_data);

/**
 * @brief Registers a new processor node.
 *
 * This function registers a processor node or node chain, such that it will be
 * evaluated when processing incoming measurements.
 *
 * Since processor nodes can be destructive, and operate on the input
 * measurement directly, they can be assigned a priority value (@ref pri),
 * where a higher number indicates higher priority. Nodes will be inserted into
 * the linked list from highest to lowest priority, and nodes with the
 * same priority level will be inserted sequentionally in the order they are
 * registered.
 *
 * Non-destructive nodes should be placed first (at a higher priority), before
 * processing the measurement in destructive nodes later in the processing
 * pipeline.
 *
 * @param node      The processor node to register with the manager.
 * @param pri       The priority level for this node (larger = higher priority).
 * @param handle    The handle the node has been registered under. Set to
 *                  -1 (0xFFFFFFFF) if node registration limit has been reached.
 *
 * @return int  0 on success, negative error code on failure.
 */
int step_pm_register(struct step_node *node, uint16_t pri, uint32_t *handle);

/**
 * @brief Gets a pointer to the node or node chain associated with the
 *        specified handle.
 *
 * @param handle    The handle the node has been registered under.
 * @param inst      The instancee number in a node chain, otherwise 0.
 *
 * @return struct* spd_node Pointer to the associated node or node chain,
 *                          NULL on error.
 */
struct step_node* step_pm_node_get(uint32_t handle, uint32_t inst);

/**
 * @brief Initialises the timer thread used to periodically poll for queued
 *        measurements.
 *
 * @return int 0 on success, negative error code on failure.
 */
int step_pm_resume(void);

/**
 * @brief Stops the timer thread used to periodically poll for queued
 *        measurements.
 *
 * @return int 0 on success, negative error code on failure.
 */
int step_pm_suspend(void);

/**
 * @brief Clears the registry, and resets the manager to it's default state.
 *
 * @return int  0 on success, negative error code on failure.
 */
int step_pm_clear(void);

/**
 * @brief Processes the supplied @ref step_measurement using the current
 *        processor node registry, consuming the measurement and optionally
 *        releasing it from shared memory when completed.
 *
 * This function evaluates the measurement's filter value against the filter
 * chain of the first node of each active record in the node registry. If a
 * node matches the measurement's filter value, the appropriate callbacks will
 * be fired in the processor node chain, from top to bottom. Only the first
 * node in a processor node chain is evaluted for a filter match.
 *
 * When this function completes, the supplied @ref step_message can optionally
 * be freed from shared memory in the sample pool via the @ref free argument.
 *
 * @param mes       Pointer to the @ref step_measurement to parse.
 * @param matches   The number of matches that occured during processing.
 * @param free      If set to true (1), the measurement will be freed
 *                  from shared memory when processing is complete.
 *
 * @return int 0 on success, negative error code on failure.
 */
int step_pm_process(struct step_measurement *mes, int *matches, bool free);

/**
 * @brief Polls the sample pool for any incoming step_measurement(s) to
 *        process, and processes them on a first in, first processed basis.
 *
 * @param mcnt      Pointer to the number of samples read from the sample pool.
 * @param free      If set to true (1), the measurement will be freed
 *                  from shared memory when processing is complete.
 *
 * @return int 0 on success, negative error code on failure.
 */
int step_pm_poll(int *mcnt, bool free);

/**
 * @brief Disables a registered processor node.
 *
 * @param handle    The handle the node has been registered under.
 *
 * @return int  0 on success, negative error code on failure.
 */
int step_pm_disable_node(uint32_t handle);

/**
 * @brief Enables a registered processor node.
 *
 * @param handle    The handle the node has been registered under.
 *
 * @return int  0 on success, negative error code on failure.
 */
int step_pm_enable_node(uint32_t handle);

/**
 * @brief Registers a callback to a particular node chain.
 *
 * @param handle    The handle the node has been registered under.
 * @param cb        The callback tobe invoked when node chain completes.
 * @param user_data The pointer to a user defined data.
 *
 * @return int  0 on success, negative error code on failure.
 */
int step_pm_subscribe_to_node(uint32_t handle, node_chain_completed_callback cb, void *user_data);

/**
 * @brief Displays a list of registered processor nodes in the order which
 *        they are evaluated (highest to lowest priority).
 *
 * @return int
 */
int step_pm_list(void);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* STEP_PROC_MGR_H_ */
