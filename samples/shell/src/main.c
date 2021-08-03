/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <shell/shell.h>
#include <logging/log.h>
#include <step/sample_pool.h>
#include <step/proc_mgr.h>
#include <step/cache.h>
#include <step/instrumentation.h>
#include "pnodes.h"

#define LOG_LEVEL LOG_LEVEL_DBG
LOG_MODULE_REGISTER(step_shell);

#if CONFIG_STEP_INSTRUMENTATION
/**
 * @brief Instrumentation counter.
 */
static uint32_t _instr;
#endif

/* Die temperature with 32-bit timestamp payload. */
static struct {
	uint32_t timestamp;
	float temp_c;
} dietemp_payload = {
	.timestamp = 1624305803, /* Monday, June 21, 2021 8:03:23 PM */
	.temp_c = 32.0F,
};

/* Test die temp measurement, with timestamp. */
static struct step_measurement dietemp_mes = {
	/* Measurement metadata. */
	.header = {
		/* Filter word. */
		.filter = {
			.base_type = STEP_MES_TYPE_TEMPERATURE,
			.ext_type = STEP_MES_EXT_TYPE_TEMP_DIE,
			.flags = {
				.data_format = STEP_MES_FORMAT_NONE,
				.encoding = STEP_MES_ENCODING_NONE,
				.compression = STEP_MES_COMPRESSION_NONE,
				.timestamp = STEP_MES_TIMESTAMP_EPOCH_32,
			},
		},
		/* SI Unit word. */
		.unit = {
			.si_unit = STEP_MES_UNIT_SI_DEGREE_CELSIUS,
			.ctype = STEP_MES_UNIT_CTYPE_IEEE754_FLOAT32,
			.scale_factor = STEP_MES_SI_SCALE_NONE,
		},
		/* Source/Len word. */
		.srclen = {
			.len = sizeof(dietemp_payload),
			.fragment = 0,
			.sourceid = 10,
		},
	},

	/* Die temperature in C plus 32-bit epoch timestamp. */
	.payload = &dietemp_payload,
};

static int
step_shell_cmd_test_list(const struct shell *shell, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	step_pm_list();

	return 0;
}

static int
step_shell_cmd_test_add(const struct shell *shell, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	uint32_t handle = 0;

	/* Append a new instance of the node chain. */
	STEP_INSTR_START(_instr);
	step_pm_register(test_node_chain, 4, &handle);
	STEP_INSTR_STOP(_instr);

	/* step_node_print(test_node_chain); */

#if CONFIG_STEP_INSTRUMENTATION
	LOG_DBG("Took %d ns", _instr);
#endif

	return 0;
}

static int
step_shell_cmd_test_clr(const struct shell *shell, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	/* Reset the processor node registry. */
	STEP_INSTR_START(_instr);
	step_pm_clear();
	STEP_INSTR_STOP(_instr);

#if CONFIG_STEP_INSTRUMENTATION
	LOG_DBG("Took %d ns", _instr);
#endif

	return 0;
}

static int
step_shell_cmd_test_pub(const struct shell *shell, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	struct step_measurement *mes_alloc;

	/* WARNING: The instrumention values below are incomplete.
	 *
	 * In the case where the polling thread is used to process the measurement
	 * (CONFIG_STEP_PROC_MGR_POLL_RATE > 0), what is being calculated is the
	 * time it takes to allocate memory from the sample pool and assign a new
	 * measurement to the FIFO. It doesn't include the PROCESSING time for that
	 * sample once it is removed from the FIFO by the polling thread.
	 *
	 * The second code block (CONFIG_STEP_PROC_MGR_POLL_RATE = 0) includes
	 * the execution time when processing the sample, since it happens in the
	 * same thread.
	 */

	STEP_INSTR_START(_instr);

	/* Allocate memory for measurement. */
	mes_alloc = step_sp_alloc(dietemp_mes.header.srclen.len);

	/* Copy test data into heap-based measurement instance. */
	memcpy(&(mes_alloc->header), &(dietemp_mes.header),
	       sizeof(struct step_mes_header));
	memcpy(mes_alloc->payload, dietemp_mes.payload,
	       dietemp_mes.header.srclen.len);

#if (CONFIG_STEP_PROC_MGR_POLL_RATE > 0)
	/* Assign measurement to FIFO so polling thread finds it. */
	step_sp_put(mes_alloc);
#else
	/* Manually process the measurement when no polling thread is present. */
	step_pm_process(&dietemp_mes, NULL, false);
#endif

	STEP_INSTR_STOP(_instr);

	LOG_DBG("Published 1 measurement");
	/* step_mes_print(&dietemp_mes); */

#if CONFIG_STEP_INSTRUMENTATION
#if (CONFIG_STEP_PROC_MGR_POLL_RATE > 0)
	LOG_DBG("Took %d ns, " \
		"excluding polling thread processing time (run 'step list').", _instr);
#else
	LOG_DBG("Took %d ns", _instr);
#endif
#endif

	return 0;
}

static int
step_shell_cmd_test_stats(const struct shell *shell, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	shell_print(shell, "evaluate: %d", cb_stats.evaluate);
	shell_print(shell, "matched:  %d", cb_stats.matched);
	shell_print(shell, "start:    %d", cb_stats.start);
	shell_print(shell, "run:      %d", cb_stats.run);
	shell_print(shell, "stop:     %d", cb_stats.stop);
	shell_print(shell, "error:    %d", cb_stats.error);

	return 0;
}

static int
step_shell_cmd_test_pool(const struct shell *shell, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	step_sp_print_stats();

	return 0;
}

#if CONFIG_STEP_FILTER_CACHE
static int
step_shell_cmd_test_cache(const struct shell *shell, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	shell_print(shell, "Cache slots:\n");
	step_cache_print();
	shell_print(shell, "\nStats:\n");
	step_cache_print_stats();

	return 0;
}
#endif

void main(void)
{
	printk("Type 'step help' for command options.\n\n");
	printk("1.) Populate the processor registry: step add\n");
	printk("2.) Publish measurement(s):          step msg\n");
	printk("3.) Check results:                   step stats");

	while (1) {
		k_sleep(K_FOREVER);
	}
}

/* Subcommand array for "step" (level 1). */
SHELL_STATIC_SUBCMD_SET_CREATE(
	sub_step,
	/* 'list' command handler. */
	SHELL_CMD(list, NULL, "Display proc. registry", step_shell_cmd_test_list),
	/* 'add' command handler. */
	SHELL_CMD(add, NULL, "Populate proc. registry", step_shell_cmd_test_add),
	/* 'clr' command handler. */
	SHELL_CMD(clr, NULL, "Clear proc. registry", step_shell_cmd_test_clr),
	/* 'pub' command handler. */
	SHELL_CMD(pub, NULL, "Publish a measurement", step_shell_cmd_test_pub),
	/* 'stats' command handler. */
	SHELL_CMD(stats, NULL, "Display node cb stats", step_shell_cmd_test_stats),
	/* 'pool' command handler. */
	SHELL_CMD(pool, NULL, "Display meas. pool stats", step_shell_cmd_test_pool),
#if CONFIG_STEP_FILTER_CACHE
	/* 'cache' command handler. */
	SHELL_CMD(cache, NULL, "Display cache stats", step_shell_cmd_test_cache),
#endif

	/* Array terminator. */
	SHELL_SUBCMD_SET_END
	);

/* Root command "step" (level 0). */
SHELL_CMD_REGISTER(step, &sub_step, "Secure telemetry pipeline commands", NULL);
