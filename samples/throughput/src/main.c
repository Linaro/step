/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <step/sample_pool.h>
#include <step/proc_mgr.h>
#include <step/instrumentation.h>

/* The number of measurements to publish. */
#define STEP_THROUGHPUT_MSGS (1000)

/* Accelerometer measurement payload. */
struct accel_payload {
	uint32_t timestamp;
	float accel_x;
	float accel_y;
	float accel_z;
};

/* Initialisation callback (fires when node added to registry). */
int node_init(void *cfg, uint32_t handle, uint32_t inst)
{
	return 0;
}

/* Node exec callback. */
int node_exec(struct step_measurement *mes, uint32_t handle, uint32_t inst)
{
	struct step_node *node;

	/* Get a reference to the source node from the proc. node registry. */
	node = step_pm_node_get(handle, inst);
	if (node == NULL) {
		return -EINVAL;
	}

	/* No processing done since we only want to measure pipeline throughput. */
	// struct accel_payload *tp = mes->payload;
	// printk("[%d:%d] %.2f %.2f %.2f m/s^2\n", handle, inst,
	//        tp->accel_x, tp->accel_y, tp->accel_z);

	return 0;
}

/* Simple processor node. */
struct step_node test_node_data = {
	.name = "Accelerometer node",
	.filters = {
		.count = 1,
		.chain = (struct step_filter[]){
			{
				/* Any accelerometer. */
				.match = STEP_MES_TYPE_ACCELERATION,
				.ignore_mask = ~STEP_MES_MASK_BASE_TYPE,
			},
		},
	},

	/* Callbacks */
	.callbacks = {
		.init_handler = node_init,
		.exec_handler = node_exec,
	},
};

/* Pointer to node chain. */
struct step_node *test_node = &test_node_data;

/**
 * @brief Pre-populated measurement header.
 */
static struct step_mes_header accel_header = {
	/* Filter word. */
	.filter = {
		.base_type = STEP_MES_TYPE_ACCELERATION,
		.flags = {
			.data_format = STEP_MES_FORMAT_NONE,
			.encoding = STEP_MES_ENCODING_NONE,
			.compression = STEP_MES_COMPRESSION_NONE,
			.timestamp = STEP_MES_TIMESTAMP_UPTIME_MS_32,
		},
	},
	/* SI Unit word. */
	.unit = {
		.si_unit = STEP_MES_UNIT_SI_METER_PER_SECOND_2,
		.ctype = STEP_MES_UNIT_CTYPE_IEEE754_FLOAT32,
		.scale_factor = STEP_MES_SI_SCALE_NONE,
	},
	/* Source/Len word. */
	.srclen = {
		/* Indicate that this is an XYZ vector. */
		.vec_sz = STEP_MES_VECTOR_SZ_3,
		.len = sizeof(struct accel_payload),
	}
};

void main(void)
{
	int rc = 0;
	uint32_t handle;
	uint32_t instr = 0;
	uint32_t instr_total = 0;
	struct step_measurement *mes;
	struct accel_payload *payload;

	/* Register a minimal processor node. */
	rc = step_pm_register(test_node, 0, &handle);
	if (rc) {
		printk("Node registration failed!\n");
		goto err;
	}

	/* Allocate and publish a series of measurements. */
	for (uint32_t i = 0; i < STEP_THROUGHPUT_MSGS; i++) {
		STEP_INSTR_START(instr);

		/* Allocate measurement from the sample pool. */
		mes = step_sp_alloc(accel_header.srclen.len);
		if (mes == NULL) {
			printk("Out of memory!\n");
			goto err;
		}

		/* Copy the measurement header. */
		mes->header.filter_bits = accel_header.filter_bits;
		mes->header.unit_bits = accel_header.unit_bits;
		mes->header.srclen_bits = accel_header.srclen_bits;

		/* Assign a pointer to the payload for easy reference. */
		payload = mes->payload;
		payload->timestamp = k_uptime_get_32();
		payload->accel_x = 0.0F;
		payload->accel_y = 0.0F;
		payload->accel_z = 0.0F;

#if (CONFIG_STEP_PROC_MGR_POLL_RATE > 0)
		/* Assign measurement to FIFO so polling thread finds it. */
		step_sp_put(mes);
#else
		/* Manually process the measurement (more accurate timing!). */
		step_pm_process(mes, NULL, true);
#endif

		STEP_INSTR_STOP(instr);
		instr_total += instr;

#if (CONFIG_STEP_PROC_MGR_POLL_RATE > 0)
		/* Give the polling thread some time to run and free up memory. */
		k_sleep(K_MSEC(10));
#endif
	}

	/* Display timing results. */
	printk("\n");
	printk("Processed %d measurements:\n\n", STEP_THROUGHPUT_MSGS);
	printk("total time: %d us\n", instr_total / 1000);
	printk("per sample: %d us\n", instr_total / STEP_THROUGHPUT_MSGS / 1000);
	printk("mes/s:      %d\n", 1000000 /
	       (instr_total / STEP_THROUGHPUT_MSGS / 1000));
	printk("\n");

	/* Display sample pool stats. */
	k_sleep(K_MSEC(100));
	printk("Sample pool statistics:\n\n");
	step_sp_print_stats();
	printk("\n");

err:
	while (1) {
		k_sleep(K_FOREVER);
	}
}
