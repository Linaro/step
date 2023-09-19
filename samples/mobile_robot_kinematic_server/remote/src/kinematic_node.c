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
static const struct step_mes_header joints_rpm_header = {
	/* Filter word. */
	.filter = {
		.base_type = STEP_MES_TYPE_VELOCITY,
		.flags = {
			.data_format = STEP_MES_FORMAT_NONE,
			.encoding = STEP_MES_ENCODING_NONE,
			.compression = STEP_MES_COMPRESSION_NONE,
			.timestamp = STEP_MES_TIMESTAMP_UPTIME_MS_32,
		},
	},
	/* SI Unit word. */
	.unit = {
		.si_unit = STEP_MES_UNIT_SI_RADIAN,
		.ctype = STEP_MES_UNIT_CTYPE_IEEE754_FLOAT32,
		.scale_factor = STEP_MES_SI_SCALE_NONE,
	},
	/* Source/Len word. */
	.srclen = {
		.len = sizeof(struct mobile_robot_node_payload),
		.fragment = 0,
		.sourceid = 0,
	}
};

int endpoint_cb(struct rpmsg_endpoint *ept, void *data,
		size_t len, uint32_t src, void *priv)
{
	struct mobile_robot_node_payload *rx_payload = (struct mobile_robot_node_payload *)data;
	struct step_measurement *mes = step_sp_alloc(sizeof(struct mobile_robot_node_payload));
	if(mes == NULL) {
		/* Just ignore no memory effects */
		return RPMSG_SUCCESS;
	}

	/* fill measurements information before publishing: */
	mes->header.filter_bits = joints_rpm_header.filter_bits;
	mes->header.unit_bits = joints_rpm_header.unit_bits;
	mes->header.srclen_bits = joints_rpm_header.srclen_bits;

	/* fill the measurement data with commands */
	memcpy(mes->payload, rx_payload, sizeof(struct mobile_robot_node_payload));
	step_pm_put(mes);

	return RPMSG_SUCCESS;
}

int do_process(struct step_measurement *mes, uint32_t handle, uint32_t inst)
{
	struct mobile_robot_node_payload *tx_payload = (struct mobile_robot_node_payload *)mes->payload;

	float linear_vel_x_mins;
	float linear_vel_y_mins;
	float angular_vel_z_mins;
	float tangential_vel;

	float x_rpm;
	float y_rpm;
	float tan_rpm;

	/* ignore linear velocity in y component if no Mecanum Wheel*/
	tx_payload->body.linear_velocity_y = 0;

	/* scale the velocities to rad/min*/
	linear_vel_x_mins = tx_payload->body.linear_velocity_x * 60;
	linear_vel_y_mins = tx_payload->body.linear_velocity_y * 60;
	angular_vel_z_mins = tx_payload->body.heading * 60;

	tangential_vel = angular_vel_z_mins *((tx_payload->kinematics.length_x / 2) +
							(tx_payload->kinematics.length_y / 2));

	x_rpm   = linear_vel_x_mins / tx_payload->kinematics.wheel_radius;;
	y_rpm   = linear_vel_y_mins / tx_payload->kinematics.wheel_radius;;
	tan_rpm = tangential_vel / tx_payload->kinematics.wheel_radius;;

	tx_payload->joints.joint_speed_rpm[0] = x_rpm - y_rpm - tan_rpm;
	tx_payload->joints.joint_speed_rpm[1] = x_rpm + y_rpm + tan_rpm;
	tx_payload->joints.joint_speed_rpm[2] = x_rpm + y_rpm - tan_rpm;
	tx_payload->joints.joint_speed_rpm[3] = x_rpm - y_rpm + tan_rpm;

	/*pushes robot data back to the transmitting core to compare the results */
	rpmsg_service_send(ep_id, tx_payload, sizeof(struct mobile_robot_node_payload));

	return 0;
}

static int do_init(void *cfg, uint32_t handle, uint32_t inst)
{
	return 0;
}

static void do_error_handler (struct step_measurement *mes,uint32_t handle, uint32_t inst, int error)
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
