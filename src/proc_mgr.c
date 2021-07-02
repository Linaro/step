/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <sys/printk.h>
#include <sdp/proc_mgr.h>
#include <sdp/filter.h>

static uint32_t g_sdp_pm_handle_counter = 0;

int sdp_pm_process(struct sdp_measurement *mes)
{
	int rc = 0;

	/* ToDo: Check what processor nodes can handle this measurement. */

	return rc;
}

int sdp_pm_poll(void)
{
	int rc = 0;

	/* ToDo: Check sample pool for measurements to process. */

	return rc;
}

int sdp_pm_register(struct sdp_node *node, uint8_t *handle)
{
	*handle = ++g_sdp_pm_handle_counter;

	printk("Registering processor node %d:\n", *handle);
	sdp_node_print(node);

	return 0;
}

int sdp_pm_list(void)
{
	return 0;
}