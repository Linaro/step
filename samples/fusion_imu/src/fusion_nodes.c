/*
 * Copyright (c) 2022 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <stdio.h>
#include <logging/log.h>
#include <step/proc_mgr.h>
#include "imu_data_producer.h"
#include "zsl_fusion_config.h"

struct zsl_euler fusion_node_result = { 0 };

int fusion_do_process(struct step_measurement *mes, uint32_t handle, uint32_t inst)
{
	struct zsl_quat q = { .r = 1.0, .i = 0.0, .j = 0.0, .k = 0.0 };
	struct imu_data_payload *imu = mes->payload;

	ZSL_VECTOR_DEF(av, 3);
	ZSL_VECTOR_DEF(gv, 3);

	zsl_vec_init(&av);
	zsl_vec_init(&gv);

	av.data[0] = imu->imu_accel_x;
	av.data[1] = imu->imu_accel_y;
	av.data[2] = imu->imu_accel_z;

	gv.data[0] = imu->imu_gyro_x;
	gv.data[1] = imu->imu_gyro_y;
	gv.data[2] = imu->imu_gyro_z;

	fusion_driver.feed_handler(&av,
							NULL,
							&gv,
							NULL,
							&q, 
							fusion_driver.config);

	zsl_quat_to_euler(&q, &fusion_node_result);
	return 0;
}

int fusion_do_streaming(struct step_measurement *mes, uint32_t handle, uint32_t inst)
{
	/* just printk over the serial port */
	struct imu_data_payload *imu = mes->payload;

	/* also reduce the streaming rate for better visualization */
	if(imu->streamable && ((imu->timestamp % 10) == 0)) {
		printk("%d\t%f\t%f\n",
			imu->timestamp,
			fusion_node_result.x * 180 / ZSL_PI ,
			fusion_node_result.y * 180 / ZSL_PI);
	}
	
	return 0;
}

/* Processor node chain. */
struct step_node fusion_node_chain_data[] = {
	/* Root processor node. */
	{
		.name = "Root sensor sample processor node",
		.filters = {
			.count = 1,
			.chain = (struct step_filter[]){
				{
					/* Phase (base type). */
					.match = STEP_MES_TYPE_ORIENTATION,
					.ignore_mask = ~STEP_MES_MASK_FULL_TYPE,
				}
			},
		},

		/* Callbacks */
		.callbacks = {
			.exec_handler = fusion_do_process,
		},

		/* Config settings */
		.config = NULL,

		/* Point to next processor node in chain. */
		.next = &fusion_node_chain_data[1],
	},
	{
		.name = "IMU streaming node",
		/* Callbacks */
		.callbacks = {
			.exec_handler = fusion_do_streaming,
		},

		/* Config settings */
		.config = NULL,

		/* End of the chain. */
		.next = NULL
	},
};

/* Pointer to node chain. */
struct step_node *fusion_node_chain = fusion_node_chain_data;
