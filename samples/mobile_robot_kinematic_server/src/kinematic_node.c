/*
 * Copyright (c) 2022 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/ipm.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/init.h>
#include <zephyr/ipc/rpmsg_service.h>
#include <zephyr/logging/log.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <step/proc_mgr.h>
#include "kinematic_node.h"

static int ep_id;
static struct mobile_robot_node_payload rx_payload;

int endpoint_cb(struct rpmsg_endpoint *ept, void *data,
		size_t len, uint32_t src, void *priv)
{
	memcpy(&rx_payload, data, len);

	printk("========= Calculated Kinematics for the Robot base: ============ \n");
	printk("Base length: %f [mm] \n", rx_payload.kinematics.length_x);
	printk("Base height: %f [mm] \n", rx_payload.kinematics.length_y);
	printk("Wheel Radius: %f [mm] \n", rx_payload.kinematics.wheel_radius);
	printk("========= Body Velocity from Direct Kinematics:  ============ \n");
	printk("Desired RPMs: %f, %f, %f, %f [rad/min] \n", rx_payload.command.joint_desired_speed_rpm[0],
													rx_payload.command.joint_desired_speed_rpm[1],
													rx_payload.command.joint_desired_speed_rpm[2],
													rx_payload.command.joint_desired_speed_rpm[3]);
	printk("Estimated Body Velocity[x,y, heading]: %f, %f, %f\n", rx_payload.body.linear_velocity_x,
																rx_payload.body.linear_velocity_y,
																rx_payload.body.heading);
	printk("========= Joint Speeds from Inverse Kinematics:  ============ \n");
	printk("Estimated Body Velocity in Local Core [x,y, heading]: %f, %f, %f \n", rx_payload.body.linear_velocity_x,
																				rx_payload.body.linear_velocity_y,
																				rx_payload.body.heading);
	printk("Output joint RPMs from Remote Core: %f, %f, %f, %f [rad/min] \n", rx_payload.joints.joint_speed_rpm[0],
																			rx_payload.joints.joint_speed_rpm[1],
																			rx_payload.joints.joint_speed_rpm[2],
																			rx_payload.joints.joint_speed_rpm[3]);
	printk("=====================================================================\n");

	return RPMSG_SUCCESS;
}

int do_process(struct step_measurement *mes, uint32_t handle, uint32_t inst)
{
	struct mobile_robot_node_payload *tx_payload = (struct mobile_robot_node_payload *)mes->payload;

	/* calculates the direct kinematics from the desired speed */
    float average_rps_x;
    float average_rps_y;
    float average_rps_a;

	/* obtain each axis rotation and scales to revolutions per second*/
    average_rps_x = ((tx_payload->command.joint_desired_speed_rpm[0] + 
					tx_payload->command.joint_desired_speed_rpm[1] +
					tx_payload->command.joint_desired_speed_rpm[2] +
					tx_payload->command.joint_desired_speed_rpm[3]) / 4) / 60;

    average_rps_y = ((-tx_payload->command.joint_desired_speed_rpm[0] + 
					tx_payload->command.joint_desired_speed_rpm[1] +
					tx_payload->command.joint_desired_speed_rpm[2] -
					tx_payload->command.joint_desired_speed_rpm[3]) / 4) / 60;
  
    average_rps_a = ((-tx_payload->command.joint_desired_speed_rpm[0] + 
					tx_payload->command.joint_desired_speed_rpm[1] -
					tx_payload->command.joint_desired_speed_rpm[2] +
					tx_payload->command.joint_desired_speed_rpm[3]) / 4) / 60;

    tx_payload->body.linear_velocity_x = average_rps_x * tx_payload->kinematics.wheel_radius;
	/* Only considered in case of a mecanum wheel */
    tx_payload->body.linear_velocity_y = average_rps_y * tx_payload->kinematics.wheel_radius;
    tx_payload->body.heading = (average_rps_y * tx_payload->kinematics.wheel_radius) /
							((tx_payload->kinematics.length_x / 2) +
							(tx_payload->kinematics.length_y / 2));

	/*pushes robot data to the remote core to perform the inverse kinematics calculation */
	rpmsg_service_send(ep_id, tx_payload, sizeof(struct mobile_robot_node_payload));

	return 0;
}

static int do_init(void *cfg, uint32_t handle, uint32_t inst)
{
	while (!rpmsg_service_endpoint_is_bound(ep_id)) {
		k_sleep(K_MSEC(1));
	}

	return 0;
}

void do_error_handler (struct step_measurement *mes,uint32_t handle, uint32_t inst, int error)
{
	printk("Kinematic node error code: %d \n", error);
}

/* Processor node chain. */
struct step_node node_chain_data[] = {
	/* Root processor node. */
	{
		.name = "Root sensor sample processor node",
		.filters = {
			.count = 1,
			.chain = (struct step_filter[]){
				{
					/* Phase (base type). */
					.match = STEP_MES_TYPE_VELOCITY,
					.ignore_mask = ~STEP_MES_MASK_FULL_TYPE,
				}
			},
		},

		/* Callbacks */
		.callbacks = {
			.init_handler = do_init,
			.exec_handler = do_process,
			.error_handler = do_error_handler,
		},

		/* Config settings */
		.config = NULL,

		/* Point to next processor node in chain. */
		.next = NULL,
	},
};

/* Pointer to node chain. */
struct step_node *node_chain = node_chain_data;

/* Make sure we register endpoint before RPMsg Service is initialized. */
int register_endpoint(const struct device *arg)
{
	int status;

	status = rpmsg_service_register_endpoint("step_ep", endpoint_cb);

	if (status < 0) {
		printk("rpmsg_create_ept failed %d\n", status);
		return status;
	}

	printk("rpmsg registered endpoint number: %d\n", status);

	ep_id = status;

	return 0;
}

SYS_INIT(register_endpoint, POST_KERNEL, CONFIG_RPMSG_SERVICE_EP_REG_PRIORITY);
