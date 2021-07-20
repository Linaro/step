/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <sys/printk.h>
#include <sdp/proc_mgr.h>
#include <sdp/filter.h>
#include <sdp/sample_pool.h>

static uint32_t g_sdp_pm_handle_counter = 0;

/* TODO: Setup timer for the poll function. */

int sdp_pm_process(struct sdp_measurement *mes)
{
	int rc = 0;

	/* ToDo: Cycle through registered nodes, checking which ones match. */
	/* ToDo: Implement filtere cache mechanism. */

	/* Free measurement from shared memory. */
	sdp_sp_free(mes);

	return rc;
}

int sdp_pm_poll(void)
{
	int rc = 0;

	struct sdp_measurement *mes;

	/* Check if we have samples in the FIFO. */
	mes = sdp_sp_get();
	while (mes != NULL) {
		rc = sdp_pm_process(mes);
		if (rc) {
			goto err;
		}
		mes = sdp_sp_get();
	}

err:
	return rc;
}

int sdp_pm_register(struct sdp_node *node, uint8_t *handle)
{
	*handle = ++g_sdp_pm_handle_counter;

	printk("Registering processor node %d:\n", *handle);
	sdp_node_print(node);

	return 0;
}

int sdp_pm_remove(uint8_t handle)
{
	printk("Removing processor node %d:\n", handle);

	return 0;
}

int sdp_pm_list(void)
{
	return 0;
}