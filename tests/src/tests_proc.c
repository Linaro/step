/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/ztest.h>
#include <string.h>
#include <zephyr/sys/printk.h>
#include <step/step.h>
#include <step/measurement/measurement.h>
#include <step/node.h>
#include <step/proc_mgr.h>
#include <step/sample_pool.h>
#include "data.h"
#include "floatcheck.h"

ZTEST_SUITE(tests_proc_manager, NULL, NULL, NULL, NULL, NULL);

ZTEST(tests_proc_manager, test_proc_reg_limit)
{
	int rc;
	uint32_t handle;

	/* Clear the node registry. */
	rc = step_pm_clear();
	zassert_equal(rc, 0, NULL);

	/* Register max number of processor nodes. */
	for (int i = 0; i < CONFIG_STEP_PROC_MGR_NODE_LIMIT; i++) {
		rc = step_pm_register(step_test_data_procnode_chain, 0, &handle);
		zassert_equal(rc, 0, NULL);
		zassert_equal(handle, i, NULL);
	}

	/* Try to add one more. */
	rc = step_pm_register(step_test_data_procnode_chain, 0, &handle);
	zassert_equal(rc, -ENOMEM, NULL);
	zassert_equal(handle, -1, NULL);

	/* Clear the node registry. */
	rc = step_pm_clear();
	zassert_equal(rc, 0, NULL);
}

K_SEM_DEFINE(sync, 0, 1);

void on_proc_completed(struct step_measurement *mes, uint32_t handle, void *user)
{
	k_sem_give(&sync);
}

/**
 * @brief This represents a minimal end-to-end workflow allocating, assigning,
 *        queueing, processing and freeing a single step_measurement.
 */
ZTEST(tests_proc_manager, test_proc_manual)
{
	int rc;
	uint32_t handle;

	struct step_measurement *mes;

	/* Clear processor node stats. */
	memset(&step_test_data_cb_stats, 0,
	       sizeof(struct step_test_data_procnode_cb_stats));

	/* Allocate memory for measurement. */
	mes = step_sp_alloc(step_test_mes_dietemp.header.srclen.len);
	zassert_not_null(mes, NULL);

	/* Copy test data into heap-based measurement instance. */
	memcpy(&(mes->header), &(step_test_mes_dietemp.header),
	       sizeof(struct step_mes_header));
	memcpy(mes->payload, step_test_mes_dietemp.payload,
	       step_test_mes_dietemp.header.srclen.len);

	/* Clear the processor node manager. */
	rc = step_pm_clear();
	zassert_equal(rc, 0, NULL);

	/* Register a processor node. */
	rc = step_pm_register(step_test_data_procnode_chain, 0, &handle);
	zassert_equal(rc, 0, NULL);
	zassert_equal(handle, 0, NULL);

	/* subscribe to the registered node */
	rc = step_pm_subscribe_to_node(handle, on_proc_completed, NULL);
	zassert_equal(rc, 0, NULL);

	/* Assign measurement to FIFO. */
	step_pm_put(mes);
	zassert_equal(rc, 0, NULL);

	/* subscribe to the node to make sure the chain was evaluated */
	rc = k_sem_take(&sync, K_MSEC(3000));
	zassert_equal(rc, 0, NULL);

	/* Verify that the processor callbacks have been fired. */
	zassert_equal(step_test_data_cb_stats.init, 2, NULL);
	zassert_equal(step_test_data_cb_stats.evaluate, 0, NULL);
	zassert_equal(step_test_data_cb_stats.matched, 1, NULL);
	zassert_equal(step_test_data_cb_stats.start, 2, NULL);
	zassert_equal(step_test_data_cb_stats.run, 2, NULL);
	zassert_equal(step_test_data_cb_stats.stop, 2, NULL);
	zassert_equal(step_test_data_cb_stats.error, 0, NULL);

	/* Make sure heap memory was freed. */
	zassert_equal(step_sp_bytes_alloc(), 0, NULL);

	/* Clear the node registry. */
	rc = step_pm_clear();
	zassert_equal(rc, 0, NULL);
}

/**
 * @brief This represents a minimal end-to-end workflow assigning,
 *        queueing and processing a single non-allocated (i.e. non-heap-based)
 *        step_measurement.
 */
ZTEST(tests_proc_manager, test_proc_manual_non_alloc)
{
	int rc;
	uint32_t handle;

	/* Point to a statically defined measurement. */
	struct step_measurement *mes = &step_test_mes_dietemp;

	/* Clear processor node stats. */
	memset(&step_test_data_cb_stats, 0,
	       sizeof(struct step_test_data_procnode_cb_stats));

	/* Clear the processor node manager. */
	rc = step_pm_clear();
	zassert_equal(rc, 0, NULL);

	/* Register a processor node. */
	rc = step_pm_register(step_test_data_procnode_chain, 0, &handle);
	zassert_equal(rc, 0, NULL);
	zassert_equal(handle, 0, NULL);

	/* subscribe to the registered node */
	rc = step_pm_subscribe_to_node(handle, on_proc_completed, NULL);
	zassert_equal(rc, 0, NULL);

	/* push the sample to the processor manager */
	rc = step_pm_put(mes);
	zassert_equal(rc, 0, NULL);
	
	/* subscribe to the node to make sure the chain was evaluated */
	rc = k_sem_take(&sync, K_MSEC(3000));
	zassert_equal(rc, 0, NULL);

	/* Verify that the processor callbacks have been fired. */
	zassert_equal(step_test_data_cb_stats.init, 2, NULL);
	zassert_equal(step_test_data_cb_stats.evaluate, 0, NULL);
	zassert_equal(step_test_data_cb_stats.matched, 1, NULL);
	zassert_equal(step_test_data_cb_stats.start, 2, NULL);
	zassert_equal(step_test_data_cb_stats.run, 2, NULL);
	zassert_equal(step_test_data_cb_stats.stop, 2, NULL);
	zassert_equal(step_test_data_cb_stats.error, 0, NULL);

	/* Clear the node registry. */
	rc = step_pm_clear();
	zassert_equal(rc, 0, NULL);
}

/**
 * @brief Tests the if check node API are correct
 */
ZTEST(tests_proc_manager, test_proc_get_node)
{
	int rc;
	uint32_t handle;
	struct step_node *node;

	/* Clear the processor node manager. */
	rc = step_pm_clear();
	zassert_equal(rc, 0, NULL);

	/* Register a processor node. */
	rc = step_pm_register(step_test_data_procnode_chain, 0, &handle);
	zassert_equal(rc, 0, NULL);
	zassert_equal(handle, 0, NULL);

	/* Register a second processor node. */
	rc = step_pm_register(step_test_data_procnode_chain, 0, &handle);
	zassert_equal(rc, 0, NULL);
	zassert_equal(handle, 1, NULL);

	/* Retrieve node 0:0. */
	node = step_pm_node_get(0, 0);
	zassert_not_null(node, NULL);
	zassert_equal(node->name, step_test_data_procnode_chain->name, NULL);

	/* Rerieve node 0:2 (non-existant). */
	node = step_pm_node_get(0, 2);
	zassert_is_null(node, NULL);

	/* Rerieve node 2:0 (non-existant). */
	node = step_pm_node_get(2, 0);
	zassert_is_null(node, NULL);
}

/**
 * @brief Test the subscription of registered node
 */
static struct step_measurement *received_measurement = NULL;
static uint32_t received_handle;
static uint32_t received_user_data;
static uint32_t callback_counts;
static uint32_t callbacks_on_second_node_counts;

void on_node_completed(struct step_measurement *mes, uint32_t handle, void *user)
{
	received_measurement = mes;
	received_handle = handle;
	received_user_data = (uint32_t)user;

	if(handle == 0) {
		callback_counts++;
	} else if(handle == 1) {
		callbacks_on_second_node_counts++;
	}

	k_sem_give(&sync);
}

ZTEST(tests_proc_manager, test_proc_subscribe_node_chain)
{
	int rc;
	uint32_t handle;

	/* Clear the processor node manager. */
	rc = step_pm_clear();
	zassert_equal(rc, 0, NULL);

	/* Register a processor node. */
	rc = step_pm_register(step_test_data_procnode_chain, 0, &handle);
	zassert_equal(rc, 0, NULL);
	zassert_equal(handle, 0, NULL);

	/* subscribe to the registered node */
	rc = step_pm_subscribe_to_node(handle, on_node_completed, (void *)0x12345678);
	zassert_equal(rc, 0, NULL);

	/* Point to a statically defined measurement. */
	struct step_measurement *mes = &step_test_mes_dietemp;

	/* Copy test data into heap-based measurement instance. */
	memcpy(&(mes->header), &(step_test_mes_dietemp.header),
	       sizeof(struct step_mes_header));
	memcpy(mes->payload, step_test_mes_dietemp.payload,
	       step_test_mes_dietemp.header.srclen.len);

	callback_counts	= 0;
	/* Assign measurement to FIFO. */
	rc = step_pm_put(mes);
	zassert_equal(rc, 0, NULL);

	rc = k_sem_take(&sync, K_MSEC(3000));
	zassert_equal(rc, 0, NULL);

	/* compare data sent with received from subscription callback */
	int match = memcmp(received_measurement->payload, mes->payload, step_test_mes_dietemp.header.srclen.len);
	zassert_equal(match, 0, NULL);
	match = memcmp(received_measurement, mes, sizeof(struct step_measurement));
	zassert_equal(match, 0, NULL);
	zassert_equal(handle, received_handle, NULL);
	zassert_equal(received_user_data, 0x12345678, NULL);
	zassert_equal(callback_counts, 1, NULL);
}
