/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <ztest.h>
#include <string.h>
#include <sys/printk.h>
#include <sdp/sdp.h>
#include <sdp/measurement/measurement.h>
#include <sdp/node.h>
#include <sdp/proc_mgr.h>
#include <sdp/sample_pool.h>
#include "data.h"
#include "floatcheck.h"

void test_proc_reg_limit(void)
{
	int rc;
	uint32_t handle;

	/* Make sure the polling thread is stopped. */
	rc = sdp_pm_suspend();
	zassert_equal(rc, 0, NULL);

	/* Clear the node registry. */
	rc = sdp_pm_clear();
	zassert_equal(rc, 0, NULL);

	/* Register max number of processor nodes. */
	for (int i = 0; i < CONFIG_SDP_PROC_MGR_NODE_LIMIT; i++) {
		rc = sdp_pm_register(sdp_test_data_procnode_chain, 0, &handle);
		zassert_equal(rc, 0, NULL);
		zassert_equal(handle, i, NULL);
	}

	/* Try to add one more. */
	rc = sdp_pm_register(sdp_test_data_procnode_chain, 0, &handle);
	zassert_equal(rc, -ENOMEM, NULL);
	zassert_equal(handle, -1, NULL);

	/* Clear the node registry. */
	rc = sdp_pm_clear();
	zassert_equal(rc, 0, NULL);
}

/**
 * @brief This represents a minimal end-to-end workflow allocating, assigning,
 *        queueing and processing a single sdp_measurement.
 */
void test_proc_manual(void)
{
	int rc;
	uint32_t handle;
	int msgcnt;

	struct sdp_measurement *mes;

	/* Make sure the polling thread is stopped. */
	rc = sdp_pm_suspend();
	zassert_equal(rc, 0, NULL);

	/* Clear processor node stats. */
	memset(&sdp_test_data_cb_stats, 0,
	       sizeof(struct sdp_test_data_procnode_cb_stats));

	/* Allocate memory for measurement. */
	mes = sdp_sp_alloc(sdp_test_mes_dietemp.header.srclen.len);
	zassert_not_null(mes, NULL);

	/* Copy test data into heap-based measurement instance. */
	memcpy(&(mes->header), &(sdp_test_mes_dietemp.header),
	       sizeof(struct sdp_mes_header));
	memcpy(mes->payload, sdp_test_mes_dietemp.payload,
	       sdp_test_mes_dietemp.header.srclen.len);

	/* Assign measurement to FIFO. */
	sdp_sp_put(mes);

	/* Clear the processor node manager. */
	rc = sdp_pm_clear();
	zassert_equal(rc, 0, NULL);

	/* Register a processor node. */
	rc = sdp_pm_register(sdp_test_data_procnode_chain, 0, &handle);
	zassert_equal(rc, 0, NULL);
	zassert_equal(handle, 0, NULL);

	/* Poll the sample pool, which should trigger message processing. */
	rc = sdp_pm_poll(&msgcnt, true);
	zassert_equal(rc, 0, NULL);
	zassert_equal(msgcnt, 1, NULL);

	/* Verify that the processor callbacks have been fired. */
	zassert_equal(sdp_test_data_cb_stats.evaluate, 0, NULL);
	zassert_equal(sdp_test_data_cb_stats.matched, 1, NULL);
	zassert_equal(sdp_test_data_cb_stats.start, 2, NULL);
	zassert_equal(sdp_test_data_cb_stats.run, 2, NULL);
	zassert_equal(sdp_test_data_cb_stats.stop, 2, NULL);
	zassert_equal(sdp_test_data_cb_stats.error, 0, NULL);

	/* Make sure the sample pool FIFO is empty. */
	rc = sdp_pm_poll(&msgcnt, true);
	zassert_equal(rc, 0, NULL);
	zassert_equal(msgcnt, 0, NULL);

	/* Also check the FIFO directly. */
	mes = sdp_sp_get();
	zassert_is_null(mes, NULL);

	/* Make sure heap memory was freed. */
	zassert_equal(sdp_sp_bytes_alloc(), 0, NULL);

	/* Clear the node registry. */
	rc = sdp_pm_clear();
	zassert_equal(rc, 0, NULL);
}

/**
 * @brief Tests the automatic polling of queued measurements.
 */
void test_proc_thread(void)
{
	int rc;
	uint32_t handle;
	int msgcnt;

	struct sdp_measurement *mes;

	/* Make sure the polling thread is stopped. */
	rc = sdp_pm_suspend();
	zassert_equal(rc, 0, NULL);

	/* Clear processor node stats. */
	memset(&sdp_test_data_cb_stats, 0,
	       sizeof(struct sdp_test_data_procnode_cb_stats));

	/* Allocate memory for measurement. */
	mes = sdp_sp_alloc(sdp_test_mes_dietemp.header.srclen.len);
	zassert_not_null(mes, NULL);

	/* Copy test data into heap-based measurement instance. */
	memcpy(&(mes->header), &(sdp_test_mes_dietemp.header),
	       sizeof(struct sdp_mes_header));
	memcpy(mes->payload, sdp_test_mes_dietemp.payload,
	       sdp_test_mes_dietemp.header.srclen.len);

	/* Assign measurement to FIFO. */
	sdp_sp_put(mes);

	/* Clear the processor node manager. */
	rc = sdp_pm_clear();
	zassert_equal(rc, 0, NULL);

	/* Register a processor node. */
	rc = sdp_pm_register(sdp_test_data_procnode_chain, 0, &handle);
	zassert_equal(rc, 0, NULL);
	zassert_equal(handle, 0, NULL);

	/* Start the polling thread if stopped. */
	rc = sdp_pm_resume();
	zassert_equal(rc, 0, NULL);

	/* Insert an appropriate delay. */
	k_sleep(K_MSEC(1000));

	/* Verify that the processor callbacks have been fired. */
	zassert_equal(sdp_test_data_cb_stats.evaluate, 0, NULL);
	zassert_equal(sdp_test_data_cb_stats.matched, 1, NULL);
	zassert_equal(sdp_test_data_cb_stats.start, 2, NULL);
	zassert_equal(sdp_test_data_cb_stats.run, 2, NULL);
	zassert_equal(sdp_test_data_cb_stats.stop, 2, NULL);
	zassert_equal(sdp_test_data_cb_stats.error, 0, NULL);

	/* Make sure the sample pool FIFO is empty. */
	rc = sdp_pm_poll(&msgcnt, true);
	zassert_equal(rc, 0, NULL);
	zassert_equal(msgcnt, 0, NULL);

	/* Also check the FIFO directly. */
	mes = sdp_sp_get();
	zassert_is_null(mes, NULL);

	/* Make sure heap memory was freed. */
	zassert_equal(sdp_sp_bytes_alloc(), 0, NULL);

	/* Clear the node registry (should also lock out the polling thread). */
	rc = sdp_pm_clear();
	zassert_equal(rc, 0, NULL);

	/* Stop the polling thread. */
	rc = sdp_pm_suspend();
	zassert_equal(rc, 0, NULL);
}
