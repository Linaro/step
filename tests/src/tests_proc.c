/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <ztest.h>
#include <string.h>
#include <sys/printk.h>
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

	/* Make sure the polling thread is stopped. */
	rc = step_pm_suspend();
	zassert_equal(rc, 0, NULL);

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

/**
 * @brief This represents a minimal end-to-end workflow allocating, assigning,
 *        queueing, processing and freeing a single step_measurement.
 */
ZTEST(tests_proc_manager, test_proc_manual)
{
	int rc;
	uint32_t handle;
	int msgcnt;

	struct step_measurement *mes;

	/* Make sure the polling thread is stopped. */
	rc = step_pm_suspend();
	zassert_equal(rc, 0, NULL);

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

	/* Assign measurement to FIFO. */
	step_sp_put(mes);

	/* Clear the processor node manager. */
	rc = step_pm_clear();
	zassert_equal(rc, 0, NULL);

	/* Register a processor node. */
	rc = step_pm_register(step_test_data_procnode_chain, 0, &handle);
	zassert_equal(rc, 0, NULL);
	zassert_equal(handle, 0, NULL);

	/* Poll the sample pool, which should trigger message processing. */
	msgcnt = step_sp_fifo_count();
	rc = step_pm_poll(&msgcnt, true);
	zassert_equal(rc, 0, NULL);
	zassert_equal(msgcnt, 1, NULL);

	/* Verify that the processor callbacks have been fired. */
	zassert_equal(step_test_data_cb_stats.init, 2, NULL);
	zassert_equal(step_test_data_cb_stats.evaluate, 0, NULL);
	zassert_equal(step_test_data_cb_stats.matched, 1, NULL);
	zassert_equal(step_test_data_cb_stats.start, 2, NULL);
	zassert_equal(step_test_data_cb_stats.run, 2, NULL);
	zassert_equal(step_test_data_cb_stats.stop, 2, NULL);
	zassert_equal(step_test_data_cb_stats.error, 0, NULL);

	/* Make sure the sample pool FIFO is empty. */
	msgcnt = step_sp_fifo_count();
	rc = step_pm_poll(&msgcnt, true);
	zassert_equal(rc, 0, NULL);
	zassert_true(step_sp_fifo_count() == 0, NULL);
	zassert_equal(msgcnt, 0, NULL);

	/* Also check the FIFO directly. */
	mes = step_sp_get();
	zassert_is_null(mes, NULL);

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
	int msgcnt;

	/* Point to a statically defined measurement. */
	struct step_measurement *mes = &step_test_mes_dietemp;

	/* Make sure the polling thread is stopped. */
	rc = step_pm_suspend();
	zassert_equal(rc, 0, NULL);

	/* Clear processor node stats. */
	memset(&step_test_data_cb_stats, 0,
	       sizeof(struct step_test_data_procnode_cb_stats));

	/* Assign measurement to FIFO. */
	step_sp_put(mes);

	/* Clear the processor node manager. */
	rc = step_pm_clear();
	zassert_equal(rc, 0, NULL);

	/* Register a processor node. */
	rc = step_pm_register(step_test_data_procnode_chain, 0, &handle);
	zassert_equal(rc, 0, NULL);
	zassert_equal(handle, 0, NULL);

	/* Poll the sample pool, which should trigger message processing,
	 * indicate that memory should NOT be freed when finished since the
	 * step_measurement is not taken from the sample pool heap.
	 */
	msgcnt = step_sp_fifo_count();
	rc = step_pm_poll(&msgcnt, false);
	zassert_equal(rc, 0, NULL);
	zassert_equal(msgcnt, 1, NULL);

	/* Verify that the processor callbacks have been fired. */
	zassert_equal(step_test_data_cb_stats.init, 2, NULL);
	zassert_equal(step_test_data_cb_stats.evaluate, 0, NULL);
	zassert_equal(step_test_data_cb_stats.matched, 1, NULL);
	zassert_equal(step_test_data_cb_stats.start, 2, NULL);
	zassert_equal(step_test_data_cb_stats.run, 2, NULL);
	zassert_equal(step_test_data_cb_stats.stop, 2, NULL);
	zassert_equal(step_test_data_cb_stats.error, 0, NULL);

	/* Make sure the sample pool FIFO is empty. */
	msgcnt = step_sp_fifo_count();
	rc = step_pm_poll(&msgcnt, false);
	zassert_true(step_sp_fifo_count() == 0, NULL);
	zassert_equal(rc, 0, NULL);
	zassert_equal(msgcnt, 0, NULL);

	/* Also check the FIFO directly. */
	mes = step_sp_get();
	zassert_is_null(mes, NULL);

	/* Clear the node registry. */
	rc = step_pm_clear();
	zassert_equal(rc, 0, NULL);
}

/**
 * @brief Tests the automatic polling of queued measurements.
 */
ZTEST(tests_proc_manager, test_proc_thread)
{
	int rc;
	uint32_t handle;
	int msgcnt;

	struct step_measurement *mes;

	/* Make sure the polling thread is stopped. */
	rc = step_pm_suspend();
	zassert_equal(rc, 0, NULL);

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

	/* Assign measurement to FIFO. */
	step_sp_put(mes);

	/* Clear the processor node manager. */
	rc = step_pm_clear();
	zassert_equal(rc, 0, NULL);

	/* Register a processor node. */
	rc = step_pm_register(step_test_data_procnode_chain, 0, &handle);
	zassert_equal(rc, 0, NULL);
	zassert_equal(handle, 0, NULL);

	/* Start the polling thread if stopped. */
	rc = step_pm_resume();
	zassert_equal(rc, 0, NULL);

	/* Insert an appropriate delay. */
	k_sleep(K_MSEC(1000));

	/* Verify that the processor callbacks have been fired. */
	zassert_equal(step_test_data_cb_stats.init, 2, NULL);
	zassert_equal(step_test_data_cb_stats.evaluate, 0, NULL);
	zassert_equal(step_test_data_cb_stats.matched, 1, NULL);
	zassert_equal(step_test_data_cb_stats.start, 2, NULL);
	zassert_equal(step_test_data_cb_stats.run, 2, NULL);
	zassert_equal(step_test_data_cb_stats.stop, 2, NULL);
	zassert_equal(step_test_data_cb_stats.error, 0, NULL);

	/* Make sure the sample pool FIFO is empty. */
	rc = step_pm_poll(&msgcnt, true);
	zassert_equal(rc, 0, NULL);
	msgcnt = step_sp_fifo_count();
	zassert_equal(msgcnt, 0, NULL);

	/* Also check the FIFO directly. */
	mes = step_sp_get();
	zassert_is_null(mes, NULL);

	/* Make sure heap memory was freed. */
	zassert_equal(step_sp_bytes_alloc(), 0, NULL);

	/* Clear the node registry (should also lock out the polling thread). */
	rc = step_pm_clear();
	zassert_equal(rc, 0, NULL);

	/* Stop the polling thread. */
	rc = step_pm_suspend();
	zassert_equal(rc, 0, NULL);
}

/**
 * @brief Tests the automatic polling of queued measurements.
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
}

ZTEST(tests_proc_manager, test_proc_subscribe_node_chain)
{
	int rc;
	int msgcnt = 0;
	uint32_t handle;

	rc = step_pm_suspend();

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

	/* Allocate memory for measurement. */
	struct step_measurement *mes = step_sp_alloc(step_test_mes_dietemp.header.srclen.len);
	zassert_not_null(mes, NULL);

	/* Copy test data into heap-based measurement instance. */
	memcpy(&(mes->header), &(step_test_mes_dietemp.header),
	       sizeof(struct step_mes_header));
	memcpy(mes->payload, step_test_mes_dietemp.payload,
	       step_test_mes_dietemp.header.srclen.len);

	callback_counts	= 0;
	/* Assign measurement to FIFO. */
	step_sp_put(mes);
	msgcnt = step_sp_fifo_count();
	rc = step_pm_poll(&msgcnt, false);
	zassert_equal(rc, 0, NULL);
	zassert_equal(msgcnt, 1, NULL);

	/* compare data sent with received from subscription callback */
	int match = memcmp(received_measurement->payload, mes->payload, step_test_mes_dietemp.header.srclen.len);
	zassert_equal(match, 0, NULL);
	match = memcmp(received_measurement, mes, sizeof(struct step_measurement));
	zassert_equal(match, 0, NULL);
	zassert_equal(handle, received_handle, NULL);
	zassert_equal(received_user_data, 0x12345678, NULL);
	zassert_equal(callback_counts, 1, NULL);
	step_sp_free(mes);

}

ZTEST(tests_proc_manager, test_proc_subscribe_node_chain_multiple_callbacks)
{
	int rc;
	int msgcnt = 0;
	uint32_t handle;

	rc = step_pm_suspend();

	/* Clear the processor node manager. */
	rc = step_pm_clear();
	zassert_equal(rc, 0, NULL);

	/* Register a processor node. */
	rc = step_pm_register(step_test_data_procnode_chain, 0, &handle);
	zassert_equal(rc, 0, NULL);
	zassert_equal(handle, 0, NULL);

	/* fill lots of callbacks in single node */
	for (int i = 0 ; i < 4; i++) {
		rc = step_pm_subscribe_to_node(handle, on_node_completed, (void *)0x12345678);
		zassert_equal(rc, 0, NULL);
	}

	/* Allocate memory for measurement. */
	struct step_measurement *mes = step_sp_alloc(step_test_mes_dietemp.header.srclen.len);
	zassert_not_null(mes, NULL);

	/* Copy test data into heap-based measurement instance. */
	memcpy(&(mes->header), &(step_test_mes_dietemp.header),
	       sizeof(struct step_mes_header));
	memcpy(mes->payload, step_test_mes_dietemp.payload,
	       step_test_mes_dietemp.header.srclen.len);

	callback_counts = 0;

	/* Assign measurement to FIFO. */
	step_sp_put(mes);
	msgcnt = step_sp_fifo_count();
	rc = step_pm_poll(&msgcnt, false);
	zassert_equal(rc, 0, NULL);
	zassert_equal(msgcnt, 1, NULL);

	/* compare data sent with received from subscription callback */
	int match = memcmp(received_measurement->payload, mes->payload, step_test_mes_dietemp.header.srclen.len);
	zassert_equal(match, 0, NULL);
	match = memcmp(received_measurement, mes, sizeof(struct step_measurement));
	zassert_equal(match, 0, NULL);
	zassert_equal(handle, received_handle, NULL);
	zassert_equal(received_user_data, 0x12345678, NULL);
	zassert_equal(callback_counts, 4, "callback_counts: %d", callback_counts);
	step_sp_free(mes);
}

ZTEST(tests_proc_manager, test_proc_subscribe_multiple_node_chain_multiple_callbacks)
{
	int rc;
	int msgcnt = 0;
	uint32_t handle1, handle2;

	rc = step_pm_suspend();
	
	/* Clear the processor node manager. */
	rc = step_pm_clear();
	zassert_equal(rc, 0, NULL);

	/* Register a processor node. */
	rc = step_pm_register(step_test_data_procnode_chain, 0, &handle1);
	zassert_equal(rc, 0, NULL);
	zassert_equal(handle1, 0, NULL);

	/* Register a second processor node. */
	rc = step_pm_register(step_test_data_procnode_chain, 0, &handle2);
	zassert_equal(rc, 0, NULL);
	zassert_equal(handle2, 1, NULL);

	/* fill lots of callbacks in first node */
	for (int i = 0 ; i < 4; i++) {
		rc = step_pm_subscribe_to_node(handle1, on_node_completed, (void *)0x12345678);
		zassert_equal(rc, 0, NULL);
	}

	/* fill lots of callbacks in second node */
	for (int i = 0 ; i < 4; i++) {
		rc = step_pm_subscribe_to_node(handle2, on_node_completed, (void *)0x12345678);
		zassert_equal(rc, 0, NULL);
	}

	/* Allocate memory for measurement. */
	struct step_measurement *mes = step_sp_alloc(step_test_mes_dietemp.header.srclen.len);
	zassert_not_null(mes, NULL);

	/* Copy test data into heap-based measurement instance. */
	memcpy(&(mes->header), &(step_test_mes_dietemp.header),
	       sizeof(struct step_mes_header));
	memcpy(mes->payload, step_test_mes_dietemp.payload,
	       step_test_mes_dietemp.header.srclen.len);

	callback_counts = 0;
	callbacks_on_second_node_counts = 0;

	/* Assign measurement to FIFO. */
	step_sp_put(mes);
	msgcnt = step_sp_fifo_count();

	rc = step_pm_poll(&msgcnt, false);
	zassert_equal(rc, 0, NULL);
	zassert_equal(msgcnt, 1, NULL);

	/* compare data sent with received from subscription callback */
	int match = memcmp(received_measurement->payload, mes->payload, step_test_mes_dietemp.header.srclen.len);
	zassert_equal(match, 0, NULL);
	match = memcmp(received_measurement, mes, sizeof(struct step_measurement));
	zassert_equal(match, 0, NULL);
	zassert_equal(received_user_data, 0x12345678, NULL);
	zassert_equal(callback_counts, 4, "callback_counts: %d", callback_counts);
	zassert_equal(callbacks_on_second_node_counts, 4, "callback_counts: %d", callbacks_on_second_node_counts);

	step_sp_free(mes);
}
