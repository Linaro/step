/*
 * Copyright (c) 2018, NXP
 * Copyright (c) 2018, Nordic Semiconductor ASA
 * Copyright (c) 2018-2022, Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/ipm.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>

#include <step/step.h>
#include <step/proc_mgr.h>
#include <step/sample_pool.h>

#include "kinematic_node.h"

void main(void)
{
    uint32_t handle;

	/* Just register the inverse kinematic node */
	int rc = step_pm_register(node_chain, 0, &handle);
	if (rc) {
		printk("Node registration failed!\n");
		return;
	}
}
