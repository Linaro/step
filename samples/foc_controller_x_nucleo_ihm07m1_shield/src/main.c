/*
 * Copyright (c) 2022 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <stdlib.h>
#include <step/step.h>
#include <step/proc_mgr.h>
#include <zephyr/shell/shell.h>
#include "foc_driver.h"
#include "foc_nodes.h"

#define SAMPLE_MOTOR_POLE_PAIRS 	7

static float motor_vq;
static float motor_vd;

void on_foc_measurement(struct step_measurement *mes)
{
	/* just update the desired voltage, data publish is handled by
	 * by the driver automatically
	 */
	struct foc_controller_payload *p = mes->payload;
	p->cmd.voltage_q = motor_vq;
	p->cmd.voltage_d = motor_vd;
}

void main(void)
{	
	int rc = 0;
	uint32_t handle;

	motor_vq = 0.0f;
	motor_vd = 0.0f;

	/* initialize the foc driver first */
	rc = foc_driver_initialize(SAMPLE_MOTOR_POLE_PAIRS,
							on_foc_measurement);
	if (rc) {
		printk("foc driver init failed!\n");
		return;
	}

	/* Then register the FoC processing pipeline */
	rc = step_pm_register(foc_node_chain, 0, &handle);
	if (rc) {
		printk("Node registration failed!\n");
		return;
	}
}

static int step_foc_cmd_set(const struct shell *shell, size_t argc, char **argv)
{
	if (argc != 3) {
		return -EINVAL;
	}

	motor_vq = strtof(argv[1], NULL);
	motor_vd = strtof(argv[2], NULL);
	return 0;
}

static int step_foc_cmd_align(const struct shell *shell, size_t argc, char **argv)
{
	return foc_driver_set_encoder_offset();
}

/* Subcommand array for "step_foc" (level 1). */
SHELL_STATIC_SUBCMD_SET_CREATE(
	sub_step_foc,
	SHELL_CMD(set, NULL, "Set voltage d-q vector to control the motor between -1 and +1", step_foc_cmd_set),
	SHELL_CMD(align, NULL, "Align rotor and calibrate its position sensor", step_foc_cmd_align),	
	SHELL_SUBCMD_SET_END
	);

/* Root command "step_foc" (level 0). */
SHELL_CMD_REGISTER(step_foc, &sub_step_foc, "Secure telemetry pipeline FoC sample commands", NULL);
