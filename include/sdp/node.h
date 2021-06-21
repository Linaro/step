/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef SDP_NODE_H__
#define SDP_NODE_H__

#include <sdp/sdp.h>
#include <sdp/filter.h>
#include <sdp/datasample.h>

/**
 * @defgroup NODE Node definition
 * @ingroup sdp_api
 * @{
 */

/**
 * @file
 * @brief API header file for SDP processor and sink nodes.
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @typedef sdp_node_callback_t
 * @brief Generic callback prototype for node implementations.
 *
 * @param sample    Pointer to the sdp_datasample being used.
 * @param cfg       Pointer to the config struct/value being used.
 *
 * @return 0 on success, negative error code on failure
 */
typedef int (*sdp_node_callback_t)(struct sdp_datasample *sample, void *cfg);

/**
 * @typedef sdp_node_evaluate_t
 * @brief Callback prototype for node filter evaluation.
 *
 * @param sample    Pointer to the sdp_datasample being used.
 * @param cfg       Pointer to the config struct/value being used.
 *
 * @return 1 on a match, 0 on match failure.
 */
typedef bool (*sdp_node_evaluate_t)(struct sdp_datasample *sample, void *cfg);

/**
 * @typedef sdp_node_error_t
 * @brief Callback prototype when a node fails to successfully run.
 *
 * @param sample    Pointer to the sdp_datasample being used.
 * @param cfg       Pointer to the config struct/value being used.
 * @param error     Negative error code produced during node execution.
 */
typedef void (*sdp_node_error_t)(struct sdp_datasample *sample, void *cfg,
				 int error);

/**
 * @brief Optional callback handlers for nodes.
 */
struct sdp_node_callbacks {
	/**
	 * @brief Callback to fire when the node is being evaluated based on
	 *        its filter chain. This callback allows the node implementation
	 *        to override filter evaluation in the filter engine, and implement
	 *        the evaluation in the specified callback.
	 *
	 * @note  Set this to NULL to allow the default filter engine to evaluate
	 *        whether there is a match or not.
	 */
	sdp_node_evaluate_t evaluate_handler;

	/**
	 * @brief Callback to fire when the filter engine has indicated that a
	 *        match occured. This function allows the node to implement
	 *        further filtering on secondary info or parameters, and
	 *        returning true or false from this function will override the
	 *        previous 'match' value.
	 *
	 * @note  Set this to NULL to accept previous filter evaluation.
	 */
	sdp_node_evaluate_t matched_handler;

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
	sdp_node_callback_t start_handler;

	/**
	 * @brief Callback to implement the node's execution logic.
	 */
	sdp_node_callback_t run_handler;

	/**
	 * @brief Callback to fire when the node has successfully
	 *        finished execution. This fires after 'run' has terminated.
	 */
	sdp_node_callback_t stop_handler;

	/**
	 * @brief Callback to fire when the 'run' command fails.
	 */
	sdp_node_error_t error_handler;
};

/**
 * @brief Node implementation.
 *
 * The callback handlers in this struct are used to implement a node or node
 * chain, and to define the node's filter properties to know when it should
 * be executed.
 */
struct sdp_node {
	/**
	 * @brief The filter chain use to determine matches for this node.
	 */
	struct sdp_filter_chain filters;

	/**
	 * @brief Callback handlers for this node.
	 */
	struct sdp_node_callbacks callbacks;

	/**
	 * @brief Config settings for the node. The exact struct or
	 *        value(s) defined here are node-specific and defined
	 *        by the implementing module.
	 */
	void *config;
};

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* SDP_NODE_H_ */
