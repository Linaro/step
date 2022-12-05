/*
 * Copyright (c) 2022 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/ipm.h>
#include <zephyr/device.h>
#include <zephyr/init.h>
#include <zephyr/shell/shell.h>

#include <step/step.h>
#include <step/proc_mgr.h>
#include <step/sample_pool.h>

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "kinematic_node.h"

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

static struct mobile_robot_node_payload payload;

void main(void)
{	
	int rc = 0;
	uint32_t handle;

	/* register the direct kinematic node*/
	rc = step_pm_register(node_chain, 0, &handle);
	if (rc) {
		printk("Node registration failed!\n");
		return;
	}
}

static int step_robot_cmd_kinematics(const struct shell *shell, size_t argc, char **argv)
{
	if (argc != 4) {
		return -EINVAL;
	}

	payload.kinematics.length_x = strtof(argv[1], NULL);
	payload.kinematics.length_y = strtof(argv[2], NULL);
	payload.kinematics.wheel_radius = strtof(argv[3], NULL) * 2.0f * 3.1416f;

	printk("Kinematics set Height: %f [mm], Width: %f [mm], %f Wheel Radius Scaled [mm] \n",
			payload.kinematics.length_x,
			payload.kinematics.length_y,
			payload.kinematics.wheel_radius);

	return 0;
}

static int step_robot_cmd_set_speeds(const struct shell *shell, size_t argc, char **argv)
{
	if (argc != 5) {
		return -EINVAL;
	}

	payload.command.joint_desired_speed_rpm[0] = strtof(argv[1], NULL);
	payload.command.joint_desired_speed_rpm[1] = strtof(argv[2], NULL);
	payload.command.joint_desired_speed_rpm[2] = strtof(argv[3], NULL);
	payload.command.joint_desired_speed_rpm[3] = strtof(argv[4], NULL);

	printk("Speed joint commmand vector: [ %f, %f, %f, %f] \n",
			payload.command.joint_desired_speed_rpm[0],
			payload.command.joint_desired_speed_rpm[1],
			payload.command.joint_desired_speed_rpm[2],
			payload.command.joint_desired_speed_rpm[3]);

	return 0;
}

static int step_robot_cmd_set_calculate(const struct shell *shell, size_t argc, char **argv)
{
	struct step_measurement *mes =  step_sp_alloc(sizeof(struct mobile_robot_node_payload));
	if(mes == NULL) {
		return -ENOMEM;
	}

	/* fill measurements information before publishing: */
	mes->header.filter_bits = joints_rpm_header.filter_bits;
	mes->header.unit_bits = joints_rpm_header.unit_bits;
	mes->header.srclen_bits = joints_rpm_header.srclen_bits;

	/* fill the measurement data with commands */
	memcpy(mes->payload, &payload, sizeof(struct mobile_robot_node_payload));

	step_pm_put(mes);

#if defined(CONFIG_SOC_MPS2_AN521)
	wakeup_cpu1();
	k_msleep(500);
#endif

	return 0;
}

/* Subcommand array for "step_foc" (level 1). */
SHELL_STATIC_SUBCMD_SET_CREATE(
	sub_step_robot,
	SHELL_CMD(kinematics, NULL, "Set robot base kinematics: Width[mm], Height[mm], and Wheel Radius[mm]", step_robot_cmd_kinematics),
	SHELL_CMD(set_speeds, NULL, "Set the four joints speeds[rpm] to obtain Kineamtics calculation", step_robot_cmd_set_speeds),
	SHELL_CMD(calculate, NULL, "Perform calculation with given parameters", step_robot_cmd_set_calculate),
	SHELL_SUBCMD_SET_END
	);

/* Root command "step_foc" (level 0). */
SHELL_CMD_REGISTER(step_robot, &sub_step_robot, "Secure telemetry pipeline Mobile Robot Kinematics commands", NULL);
