/*
 * Copyright (c) 2022 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <stdlib.h>
#include <step/step.h>
#include <step/proc_mgr.h>
#include <shell/shell.h>
#include "fusion_nodes.h"
#include "imu_data_producer.h"

void on_imu_data_production(struct imu_data_payload *imu)
{

}

void main(void)
{	
	int rc = 0;
	uint32_t handle;

	/* start the imu data producer first*/
	rc = imu_producer_start(on_imu_data_production);
	if (rc) {
		printk("Node registration failed!\n");
		return;
	}

	/* then register the fusion processing pipeline */
	rc = step_pm_register(fusion_node_chain, 0, &handle);
	if (rc) {
		printk("Node registration failed!\n");
		return;
	}
}
