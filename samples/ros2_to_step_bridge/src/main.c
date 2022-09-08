/*
 * Copyright (c) 2022 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>>
#include <stdlib.h>
#include <string.h>
#include <step/step.h>
#include <step/proc_mgr.h>
#include <step/sample_pool.h>
#include <zephyr/shell/shell.h>
#include "sensor_node.h"
#include "imu_data_producer.h"
#include "zsl_fusion_config.h"

void on_imu_data_production(struct imu_data_payload *imu)
{
	struct step_measurement *imu_measurement = step_sp_alloc(sizeof(*imu));

	if(imu_measurement == NULL) {
		printk("Warning, no memory available from sample pool! \n");
		return;
	}

	imu->streamable = true;
	imu_measurement->payload = imu;

	/* fill measurements information before publishing: */
	imu_measurement->header.filter_bits = imu_measurement_header.filter_bits;
	imu_measurement->header.unit_bits = imu_measurement_header.unit_bits;
	imu_measurement->header.srclen_bits = imu_measurement_header.srclen_bits;

	/* publish sensor measurement to be processed by the waiters */
	step_sp_put(imu_measurement);
}

void main(void)
{
	uint32_t handle;
	struct step_measurement *mes;

	int rc = step_pm_register(sensor_node_chain, 0, &handle);
	if (rc) {
		printk("Node registration failed!\n");
		return;
	}

	/* then start the imu data producer */
	rc = imu_producer_start(on_imu_data_production);
	if (rc) {
		printk("IMU producer start failed!\n");
		return;
	}
}
