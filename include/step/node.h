/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef STEP_NODE_H__
#define STEP_NODE_H__

#include <step/step.h>
#include <step/filter.h>
#include <step/measurement/measurement.h>

/**
 * @defgroup NODE Nodes
 * @ingroup step_api
 * @brief API header file for STeP processor nodes.
 * 
 * Nodes are the main building block in STeP, and encapsulate the logic to
 * 'process' incoming @ref step_measurement packets. Thy can be used
 * individually, or connected together in 'processor chains', where the
 * measurment fed into the first node in the chain is handed off to subsequent
 * nodes for further processing.
 * 
 * The first record in a node chain contains a 'filter' or 'filter chain' that
 * determines if a @ref step_measurement should be processed with this node.
 * Assigning NULL to the filter field means that the node will accept all
 * incoming measurements, otherwise the measurement's filter field will be
 * evaluated against the node's filter chain via the filter evaluation engine.
 * @{
 */

/**
 * @file
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @typedef step_node_init_t
 * @brief Init callback prototype for node implementations.
 *
 * @param cfg       Pointer to the config struct/value for this node, if any.
 * @param handle    The handle of the source node this callback.
 * @param inst      step_node instance in a node chain (zero-based).
 *
 * @return 0 on success, negative error code on failure
 */
typedef int (*step_node_init_t)(void *cfg, uint32_t handle, uint32_t inst);

/**
 * @typedef step_node_callback_t
 * @brief Generic callback prototype for node implementations.
 *
 * @param mes       Pointer to the step_measurement being used.
 * @param handle    The handle of the source node this callback.
 * @param inst      step_node instance in a node chain (zero-based).
 *
 * @return 0 on success, negative error code on failure
 */
typedef int (*step_node_callback_t)(struct step_measurement *mes,
				    uint32_t handle, uint32_t inst);

/**
 * @typedef step_node_evaluate_t
 * @brief Callback prototype for node filter evaluation.
 *
 * @param mes       Pointer to the step_measurement being used.
 * @param handle    The handle of the source node this callback.
 * @param inst      step_node instance in a node chain (zero-based).
 *
 * @return 1 on a match, 0 on match failure.
 */
typedef bool (*step_node_evaluate_t)(struct step_measurement *mes,
				     uint32_t handle, uint32_t inst);

/**
 * @typedef step_node_error_t
 * @brief Callback prototype when a node fails to successfully run.
 *
 * @param mes       Pointer to the step_measurement being used.
 * @param handle    The handle of the source node this callback.
 * @param inst      step_node instance in a node chain (zero-based).
 * @param error     Negative error code produced during node execution.
 */
typedef void (*step_node_error_t)(struct step_measurement *mes,
				  uint32_t handle, uint32_t inst, int error);

/**
 * @brief Optional callback handlers for nodes.
 */
struct step_node_callbacks {
	/**
	 * @brief Callback to fire when the node is first registered. This
	 *        callback allows the node implementation to perform any
	 *        initiliasation steps required by subsequent callbacks, or
	 *        to process incoming measurement data.
	 *
	 * @note  Set this to NULL to skip the init callback on node registration.
	 */
	step_node_init_t init_handler;

	/**
	 * @brief Callback to fire when the node is being evaluated based on
	 *        its filter chain. This callback allows the node implementation
	 *        to override filter evaluation in the filter engine, and implement
	 *        the evaluation in the specified callback.
	 *
	 * @note  Set this to NULL to allow the default filter engine to evaluate
	 *        whether there is a match or not.
	 */
	step_node_evaluate_t evaluate_handler;

	/**
	 * @brief Callback to fire when the filter engine has indicated that a
	 *        match occurred. This function allows the node to implement
	 *        further filtering on secondary info or parameters, and
	 *        returning true or false from this function will override the
	 *        previous 'match' value.
	 *
	 * @note  Set this to NULL to accept previous filter evaluation.
	 */
	step_node_evaluate_t matched_handler;

	/**
	 * @brief Callback to fire when the node is triggered. This fires
	 *        before 'run' has been called, but after the filter engine
	 *        (or override callbacks) has determined that this node should be
	 *        executed.
	 *
	 * @note  This isn't the same as 'run', since the node hasn't started
	 *        executing yet. This callback can be used to prepare the data
	 *        for processing, or implement any statistical tracking required.
	 */
	step_node_callback_t start_handler;

	/**
	 * @brief Callback to implement the node's execution logic.
	 */
	step_node_callback_t exec_handler;

	/**
	 * @brief Callback to fire when the node has successfully
	 *        finished execution. This fires after 'run' has terminated.
	 */
	step_node_callback_t stop_handler;

	/**
	 * @brief Callback to fire when the 'run' command fails.
	 */
	step_node_error_t error_handler;
};

/**
 * @brief Node implementation.
 *
 * The callback handlers in this struct are used to implement a node or node
 * chain, and to define the node's filter properties to know when it should
 * be executed.
 */
struct step_node {
	/**
	 * @brief An optional display name for this processor node. Must be
	 *        NULL-terrminated.
	 */
	char *name;

	/**
	 * @brief The filter chain use to determine matches for this node.
	 */
	struct step_filter_chain filters;

	/**
	 * @brief Callback handlers for this node.
	 */
	struct step_node_callbacks callbacks;

	/**
	 * @brief Config settings for the node. The exact struct or
	 *        value(s) defined here are node-specific and implementation
	 *        defined.
	 */
	void *config;

	/**
	 * @brief Next node in the node chain. Set to NULL if none.
	 */
	struct step_node *next;
};

/**
 * @brief Prints details of the supplied processor node using printk.
 *
 * @param node The node to display.
 */
void step_node_print(struct step_node *node);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* STEP_NODE_H_ */
