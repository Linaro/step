/*
 * Copyright (c) 2022 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <stdlib.h>
#include <string.h>
#include <step/step.h>
#include <step/proc_mgr.h>
#include <step/sample_pool.h>
#include <zephyr/shell/shell.h>
#include "fusion_nodes.h"
#include "imu_data_producer.h"
#include "zsl_fusion_config.h"

static struct imu_data_payload raw;
static bool enable_stream = false;

static void stream_end(struct k_timer *t) 
{
	enable_stream = false;
}

static void stream_stop(struct k_timer *t) 
{
	
}

K_TIMER_DEFINE(stream_timer, stream_end, stream_stop);

void on_imu_data_production(struct imu_data_payload *imu)
{
	struct step_measurement *imu_measurement = step_sp_alloc(sizeof(*imu));

	if(imu_measurement == NULL) {
		printk("Warning, no memory available from sample pool! \n");
		return;
	}

	imu->streamable = enable_stream;
	imu_measurement->payload = imu;

	/* fill measurements information before publishing: */
	imu_measurement->header.filter_bits = imu_measurement_header.filter_bits;
	imu_measurement->header.unit_bits = imu_measurement_header.unit_bits;
	imu_measurement->header.srclen_bits = imu_measurement_header.srclen_bits;

	/* make a copy of the IMU incoming data for shelll presentation: */
	memcpy(&raw, imu, sizeof(raw));

	/* publish sensor measurement to be processed by the waiters */
	step_pm_put(imu_measurement);	
}

static int step_fusion_cmd_imu(const struct shell *shell, size_t argc, char **argv)
{
	shell_print(shell,
			"Accelerometer[m/s^2] x = %10.6f y = %10.6f z = %10.6f\n\rGyroscope[rad/s] x = %10.6f y = %10.6f z = %10.6f\n\rMagnetometer[uT] x = %10.6f y = %10.6f z = %10.6f \n\r",
			raw.imu_accel_x,
			raw.imu_accel_y,
			raw.imu_accel_z,
			raw.imu_gyro_x,
			raw.imu_gyro_y,
			raw.imu_gyro_z,
			raw.imu_mag_x,
			raw.imu_mag_y,
			raw.imu_mag_z);

	return 0;
}

static int step_fusion_cmd_euler(const struct shell *shell, size_t argc, char **argv)
{
	shell_print(shell,
			"Euler angles x = %10.6f[d] y = %10.6f[d]",
			fusion_node_result.x * 180 / ZSL_PI,
			fusion_node_result.y * 180 / ZSL_PI);

	return 0;
}

static int step_fusion_cmd_set(const struct shell *shell, size_t argc, char **argv)
{
	if (argc != 3) {
		return -EINVAL;
	}

	mahn_cfg.kp = (zsl_real_t)strtof(argv[1], NULL);
	mahn_cfg.ki = (zsl_real_t)strtof(argv[2], NULL);

	return 0;
}

static int step_fusion_cmd_stream(const struct shell *shell, size_t argc, char **argv)
{
	int stream_time = 0; 

	if ((argc == 1) || (strcmp(argv[1], "help") == 0) || argc > 2) {
		shell_print(shell, "Starts streaming orientation data.\n");
		shell_print(shell, "  $ %s %s <timeout_s>\n", argv[-1], argv[0]);
		shell_print(shell, "  <timeout_s> Time to stream in seconds. 0 = non-stop.\n");
		shell_print(shell, "Example: %s %s 10", argv[-1], argv[0]);
		return 0;
	}

	// Get timeout
	stream_time = strtol(argv[1], NULL, 10) * 1000;
	if(stream_time < 0) {
		shell_print(shell, "Invalid timeout_s: %d", stream_time / 1000);
		return -EINVAL;
	}

	// Enable stream
	enable_stream = true;
	
	// Start the timer if time isn't infinite (0)
	if (stream_time) {
		k_timer_start(&stream_timer, K_MSEC(stream_time), K_MSEC(0));
	}

	return 0;
}

static int step_fusion_cmd_calibrate(const struct shell *shell, size_t argc, char **argv)
{
	imu_producer_calibrate();
	return 0;
}

void main(void)
{	
	int rc = 0;
	uint32_t handle;

	/* register the fusion processing pipeline first*/
	rc = step_pm_register(fusion_node_chain, 0, &handle);
	if (rc) {
		printk("Node registration failed!\n");
		return;
	}

	/* then start the imu data producer */
	rc = imu_producer_start(on_imu_data_production);
	if (rc) {
		printk("Node registration failed!\n");
		return;
	}
}

/* Subcommand array for "step_fusion" (level 1). */
SHELL_STATIC_SUBCMD_SET_CREATE(
	sub_step_fusion,
	SHELL_CMD(euler, NULL, "Prints sensor fusion result in euler angles in degress", step_fusion_cmd_euler),
	SHELL_CMD(imu, NULL, "Prints raw sensor data", step_fusion_cmd_imu),
	SHELL_CMD(calibrate, NULL, "Calibrates the IMU sensor", step_fusion_cmd_calibrate),
	SHELL_CMD(set_mahoney, NULL, "Sets Mahoney filter kp and ki", step_fusion_cmd_set),
	SHELL_CMD(stream, NULL, "Streams orientation data", step_fusion_cmd_stream),
	SHELL_SUBCMD_SET_END
	);

/* Root command "step_foc" (level 0). */
SHELL_CMD_REGISTER(step_fusion, &sub_step_fusion, "Secure telemetry pipeline fusion sample commands", NULL);
