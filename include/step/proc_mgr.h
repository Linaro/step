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
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

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
