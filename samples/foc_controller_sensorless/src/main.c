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
#include "foc_driver.h"
#include "foc_nodes.h"
#include "observer_node.h"
#include "foc_scope.h"

/* Default values given by HT2205 motor */
#define SAMPLE_MOTOR_POLE_PAIRS 		7.0f 
#define SAMPLE_ADC_TO_AMPS_RATIO 		0.00159608f /* (ADC_Vref / (Gain  * ADC_MAX * Shunt Resistance)) */
#define SAMPLE_MOTOR_PHASE_INDUCTANCE	0.00145f /* Henry */	
#define SAMPLE_MOTOR_PHASE_RESISTANCE   7.1f	 /* Ohms */
#define SAMPLE_MOTOR_BACK_EMF_CONSTANT  0.19f	 /* V/rad/s */
#define SAMPLE_MOTOR_FLUX_LINKAGE		(SAMPLE_MOTOR_BACK_EMF_CONSTANT / (2.0f * SAMPLE_MOTOR_POLE_PAIRS))
#define SAMPLE_MOTOR_OBSERVER_GAIN	    1.0f  
 
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
	uint32_t handle, obs_handle;

	motor_vq = 0.0f;
	motor_vd = 0.0f;

	/* configure the observer */
	motor_observer_node_conf.motor_phase_inductance = SAMPLE_MOTOR_PHASE_INDUCTANCE;
	motor_observer_node_conf.motor_phase_resistance = SAMPLE_MOTOR_PHASE_RESISTANCE;
	motor_observer_node_conf.motor_flux_linkage = SAMPLE_MOTOR_FLUX_LINKAGE;
	motor_observer_node_conf.observer_gain = SAMPLE_MOTOR_OBSERVER_GAIN;
	motor_observer_node_conf.motor_poles = SAMPLE_MOTOR_POLE_PAIRS * 2.0f;

	/* initialize the foc driver first */
	rc = foc_driver_initialize(SAMPLE_ADC_TO_AMPS_RATIO,
							motor_observer_node_conf.motor_poles / 2.0f,
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

	/* And the rotor position observer node: */
	rc = step_pm_register(observer_node_chain, 0, &obs_handle);
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

static int step_foc_cmd_motor_phy(const struct shell *shell, size_t argc, char **argv)
{
	if (argc != 5) {
		return -EINVAL;
	}

	float poles_temp = strtof(argv[3], NULL);

	if((int)poles_temp == 0) {
		return -EINVAL;
	}

	motor_observer_node_conf.motor_phase_inductance = strtof(argv[1], NULL);
	motor_observer_node_conf.motor_phase_resistance = strtof(argv[2], NULL);
	motor_observer_node_conf.motor_poles = poles_temp;
	motor_observer_node_conf.motor_flux_linkage = strtof(argv[4], NULL) / motor_observer_node_conf.motor_poles;
	return 0;
}

static int step_foc_cmd_observer_gain(const struct shell *shell, size_t argc, char **argv)
{
	if (argc != 2) {
		return -EINVAL;
	}

	float observer_temp = strtof(argv[1], NULL);
	if(observer_temp < 0.0f) {
		return -EINVAL;
	}

	motor_observer_node_conf.observer_gain = observer_temp;

	return 0;
}

static int step_foc_cmd_align(const struct shell *shell, size_t argc, char **argv)
{
	return foc_driver_set_encoder_offset();
}

static int step_foc_cmd_scope_on(const struct shell *shell, size_t argc, char **argv)
{
	foc_scope_enable();
	return 0;
}

static int step_foc_cmd_scope_off(const struct shell *shell, size_t argc, char **argv)
{
	foc_scope_disable();
	return 0;
}

/* Subcommand array for "step_foc" (level 1). */
SHELL_STATIC_SUBCMD_SET_CREATE(
	sub_step_foc,
	SHELL_CMD(set, NULL, "Set voltage d-q vector to control the motor between -1 and +1", step_foc_cmd_set),
	SHELL_CMD(align, NULL, "Align rotor and calibrate its position sensor", step_foc_cmd_align),
	SHELL_CMD(motor_phy, NULL, "Set motor physical parameters inductance, resistance, Back EMF constant and number of Poles", step_foc_cmd_motor_phy),
	SHELL_CMD(observer_gain, NULL, "Sets the observer gain, it must be > 0", step_foc_cmd_observer_gain),
	SHELL_CMD(scope, NULL, "Enables FoC scope data streaming", step_foc_cmd_scope_on),	
	SHELL_CMD(noscope, NULL, "Disables FoC scope data streaming", step_foc_cmd_scope_off),	
	SHELL_SUBCMD_SET_END
	);

/* Root command "step_foc" (level 0). */
SHELL_CMD_REGISTER(step_foc, &sub_step_foc, "Secure telemetry pipeline FoC sample commands", NULL);
