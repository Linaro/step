/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>
#include <zephyr/logging/log.h>
#include <step/sample_pool.h>
#include <step/proc_mgr.h>
#include <step/cache.h>
#include <step/instrumentation.h>
#include "driver.h"
#include "nodes.h"

static void on_node_completed(struct step_measurement *mes, uint32_t handle, void *user)
{
	struct drv_payload *p = mes->payload;
	printk("Receving subscription callback from node %d \n", handle);
	printk("Die temp: %10.6f [C] at %d [ms]\n", p->temp_c, p->timestamp);
	
	struct step_measurement *next_mes;
	step_shell_drv_get_mes(&next_mes);
	step_pm_put(next_mes);	
}

void main(void)
{
	uint32_t handle;
	struct step_measurement *mes;

	int rc = step_pm_register(test_node_chain, 0, &handle);
	if (rc) {
		printk("Node registration failed!\n");
		return;
	}

	/* subscribe to the registered node */
	rc = step_pm_subscribe_to_node(handle, on_node_completed, NULL);
	if (rc) {
		printk("Node subscription failed!\n");
		return;
	}

	/* generate the first sample, the callback will generate the others */
	rc = step_shell_drv_get_mes(&mes);
	if (rc) {
		printk("Node subscription failed!\n");
		return;
	}

	step_pm_put(mes);
	if (rc) {
		printk("Node subscription failed!\n");
		return;
	}
}
