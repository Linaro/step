/*
 * Copyright (c) 2022 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <zephyr.h>
#include <stdio.h>
#include <time.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>

#include <logging/log.h>
#include <step/proc_mgr.h>

#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <geometry_msgs/msg/quaternion.h>

#include <rclc/rclc.h>
#include <rclc/executor.h>

#include <rmw_microros/rmw_microros.h>
#include <microros_transports.h>

#include "imu_data_producer.h"
#include "zsl_fusion_config.h"

/* micro ros client building blocks :*/
static rcl_publisher_t publisher;
static rcl_allocator_t allocator;
static rclc_support_t support;
static rcl_init_options_t init_options;
static rcl_node_t node;
static rclc_executor_t executor;

/* message to send data via quaternion */
static geometry_msgs__msg__Quaternion ros_quaternion;

int sensor_init(void *cfg, uint32_t handle, uint32_t inst)
{

	/* transport over serial, or usb-cdc when available */
	rmw_uros_set_custom_transport(
		MICRO_ROS_FRAMING_REQUIRED,
		(void *)DEVICE_DT_GET(DT_ALIAS(uros_serial_port)),
		zephyr_transport_open,
		zephyr_transport_close,
		zephyr_transport_write,
		zephyr_transport_read
	);

	allocator = rcl_get_default_allocator();
	init_options = rcl_get_zero_initialized_init_options();
	rcl_init_options_init(&init_options, allocator);
	rcl_init_options_get_rmw_init_options(&init_options);
	rclc_support_init_with_options(&support, 0, NULL, &init_options, &allocator);
	rclc_node_init_default(&node, "step_imu_sensor_node", "", &support);

	rclc_publisher_init_default(
		&publisher,
		&node,
		ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg, Quaternion),
		"sensor/step/imu/quaternion");
	
	rclc_executor_init(&executor, &support.context, 1, &allocator);

	return 0;
}

int sensor_do_process(struct step_measurement *mes, uint32_t handle, uint32_t inst)
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
	
	ros_quaternion.x = q.i;
	ros_quaternion.y = q.j;
	ros_quaternion.z = q.k;
	ros_quaternion.w = q.r;

	rcl_publish(&publisher, &ros_quaternion, NULL);

	/* also spin the executor */
	rclc_executor_spin_some(&executor, 100);

	return 0;
}

/* Processor node chain. */
struct step_node sensor_node_chain_data[] = {
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
			.init_handler = sensor_init,
			.exec_handler = sensor_do_process,
		},

		/* Config settings */
		.config = NULL,

		/* Point to next processor node in chain. */
		.next = NULL,
	},
};

/* Pointer to node chain. */
struct step_node *sensor_node_chain = sensor_node_chain_data;
