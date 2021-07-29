/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <shell/shell.h>
#include <sdp/node.h>
#include <sdp/measurement/measurement.h>
#include <sdp/sample_pool.h>
#include <sdp/proc_mgr.h>
#include <sdp/cache.h>
#include <sdp/instrumentation.h>

#if CONFIG_SDP_INSTRUMENTATION
/**
 * @brief Instrumentation counter.
 */
static uint32_t _instr;
#endif

/* Struct to track the number of times specific callbacks are fired. */
struct sdp_node_cb_stats {
	uint32_t evaluate;
	uint32_t matched;
	uint32_t start;
	uint32_t run;
	uint32_t stop;
	uint32_t error;
};

struct sdp_node_cb_stats cb_stats = { 0 };

bool node_evaluate(struct sdp_measurement *mes, void *cfg)
{
	/* Overrides the filter engine when evaluating this node. */
	cb_stats.evaluate++;

	return true;
}

bool node_matched(struct sdp_measurement *mes, void *cfg)
{
	/* Fires when the filter engine has indicated a match for this node. */
	cb_stats.matched++;

	return true;
}

int node_start(struct sdp_measurement *mes, void *cfg)
{
	/* Fires before the node runs. */
	cb_stats.start++;

	return 0;
}

int node_run(struct sdp_measurement *mes, void *cfg)
{
	/* Node logic implementation. */
	cb_stats.run++;

	return 0;
}

int node_stop(struct sdp_measurement *mes, void *cfg)
{
	/* Fires when the node has been successfully run. */
	cb_stats.stop++;

	return 0;
}

void node_error(struct sdp_measurement *mes, void *cfg, int error)
{
	/* Fires when an error occurs running this node. */
	cb_stats.error++;
}

/* Processor node chain. */
static struct sdp_node test_node_chain_data[] = {
	/* Processor node 0. */
	{
		.name = "Temperature",
		.filters = {
			.count = 3,
			.chain = (struct sdp_filter[]){
				{
					/* Temperature (base type). */
					.match = SDP_MES_TYPE_TEMPERATURE,
					.ignore_mask = ~SDP_MES_MASK_FULL_TYPE,
				},
				{
					/* Die temperature. */
					.op = SDP_FILTER_OP_OR,
					.match = SDP_MES_TYPE_TEMPERATURE +
						 (SDP_MES_EXT_TYPE_TEMP_DIE <<
						  SDP_MES_MASK_EXT_TYPE_POS),
					.ignore_mask = ~SDP_MES_MASK_FULL_TYPE,
				},
				{
					/* Make sure timestamp (bits 26-28) = EPOCH32 */
					.op = SDP_FILTER_OP_AND,
					.match = (SDP_MES_TIMESTAMP_EPOCH_32 <<
						  SDP_MES_MASK_TIMESTAMP_POS),
					.ignore_mask = ~SDP_MES_MASK_TIMESTAMP,
				},
			},
		},

		/* Callbacks */
		.callbacks = {
			.evaluate_handler = NULL,
			.matched_handler = node_matched,
			.start_handler = node_start,
			.stop_handler = node_stop,
			.run_handler = node_run,
			.error_handler = node_error,
		},

		/* Config settings */
		.config = NULL,

		/* Point to next processor node in chain. */
		.next = &test_node_chain_data[1]
	},
	/* Processor node 1. */
	{
		.name = "Secondary temp processor",
		/* Callbacks */
		.callbacks = {
			.matched_handler = node_matched,
			.start_handler = node_start,
			.stop_handler = node_stop,
			.run_handler = node_run,
			.error_handler = node_error,
		},

		/* Config settings */
		.config = NULL,

		/* End of the chain. */
		.next = NULL
	}
};

/* Pointer to node chain. */
static struct sdp_node *test_node_chain = test_node_chain_data;

/* Die temperature with 32-bit timestamp payload. */
static struct {
	uint32_t timestamp;
	float temp_c;
} dietemp_payload = {
	.timestamp = 1624305803, /* Monday, June 21, 2021 8:03:23 PM */
	.temp_c = 32.0F,
};

/* Test die temp measurement, with timestamp. */
static struct sdp_measurement dietemp_mes = {
	/* Measurement metadata. */
	.header = {
		/* Filter word. */
		.filter = {
			.base_type = SDP_MES_TYPE_TEMPERATURE,
			.ext_type = SDP_MES_EXT_TYPE_TEMP_DIE,
			.flags = {
				.data_format = SDP_MES_FORMAT_NONE,
				.encoding = SDP_MES_ENCODING_NONE,
				.compression = SDP_MES_COMPRESSION_NONE,
				.timestamp = SDP_MES_TIMESTAMP_EPOCH_32,
			},
		},
		/* SI Unit word. */
		.unit = {
			.si_unit = SDP_MES_UNIT_SI_DEGREE_CELSIUS,
			.ctype = SDP_MES_UNIT_CTYPE_IEEE754_FLOAT32,
			.scale_factor = SDP_MES_SI_SCALE_NONE,
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
sdp_shell_cmd_test_list(const struct shell *shell, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	sdp_pm_list();

	return 0;
}

static int
sdp_shell_cmd_test_add(const struct shell *shell, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	uint8_t handle = 0;

	/* Append a new instance of the node chain. */
	SDP_INSTR_START(_instr);
	sdp_pm_register(test_node_chain, 0, &handle);
	SDP_INSTR_STOP(_instr);

	sdp_node_print(test_node_chain);

#if CONFIG_SDP_INSTRUMENTATION
	shell_print(shell, "Took %d ns", _instr);
#endif

	return 0;
}

static int
sdp_shell_cmd_test_clr(const struct shell *shell, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	/* Reset the processor node registry. */
	SDP_INSTR_START(_instr);
	sdp_pm_clear();
	SDP_INSTR_STOP(_instr);

#if CONFIG_SDP_INSTRUMENTATION
	shell_print(shell, "Took %d ns", _instr);
#endif

	return 0;
}

static int
sdp_shell_cmd_test_pub(const struct shell *shell, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	struct sdp_measurement *mes_alloc;

	/* WARNING: The instrumention values below are incomplete.
	 *
	 * In the case where the polling thread is used to process the measurement
	 * (CONFIG_SDP_PROC_MGR_POLL_RATE > 0), what is being calculated is the
	 * time it takes to allocate memory from the sample pool and assign a new
	 * measurement to the FIFO. It doesn't include the PROCESSING time for that
	 * sample once it is removed from the FIFO by the polling thread.
	 *
	 * The second code block (CONFIG_SDP_PROC_MGR_POLL_RATE = 0) includes
	 * the execution time when processing the sample, since it happens in the
	 * same thread.
	 */

	SDP_INSTR_START(_instr);

	/* Allocate memory for measurement. */
	mes_alloc = sdp_sp_alloc(dietemp_mes.header.srclen.len);

	/* Copy test data into heap-based measurement instance. */
	memcpy(&(mes_alloc->header), &(dietemp_mes.header),
	       sizeof(struct sdp_mes_header));
	memcpy(mes_alloc->payload, dietemp_mes.payload,
	       dietemp_mes.header.srclen.len);

#if (CONFIG_SDP_PROC_MGR_POLL_RATE > 0)
	/* Assign measurement to FIFO so polling thread finds it. */
	sdp_sp_put(mes_alloc);
#else
	/* Manually process the measurement when no polling thread is present. */
	sdp_pm_process(&dietemp_mes, NULL, false);
#endif

	SDP_INSTR_STOP(_instr);

	shell_print(shell, "Published 1 measurement:");
	sdp_mes_print(&dietemp_mes);

#if CONFIG_SDP_INSTRUMENTATION
#if (CONFIG_SDP_PROC_MGR_POLL_RATE > 0)
	shell_print(shell, "Took %d ns, " \
		"excluding polling thread processing time (run 'sdp list').", _instr);
#else
	shell_print(shell, "Took %d ns. ", _instr);
#endif
#endif

	return 0;
}

static int
sdp_shell_cmd_test_stats(const struct shell *shell, size_t argc, char **argv)
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
sdp_shell_cmd_test_pool(const struct shell *shell, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	sdp_sp_print_stats();

	return 0;
}

#if CONFIG_SDP_FILTER_CACHE
static int
sdp_shell_cmd_test_cache(const struct shell *shell, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	shell_print(shell, "Cache slots:\n");
	sdp_cache_print();
	shell_print(shell, "\nStats:\n");
	sdp_cache_print_stats();

	return 0;
}
#endif

void main(void)
{
	printk("Type 'sdp help' for command options.\n\n");
	printk("1.) Populate the processor registry: sdp add\n");
	printk("2.) Publish measurement(s):          sdp msg\n");
	printk("3.) Check results:                   sdp stats");

	while (1) {
		k_sleep(K_FOREVER);
	}
}

/* Subcommand array for "sdp" (level 1). */
SHELL_STATIC_SUBCMD_SET_CREATE(
	sub_sdp,
	/* 'list' command handler. */
	SHELL_CMD(list, NULL, "Display proc. registry", sdp_shell_cmd_test_list),
	/* 'add' command handler. */
	SHELL_CMD(add, NULL, "Populate proc. registry", sdp_shell_cmd_test_add),
	/* 'clr' command handler. */
	SHELL_CMD(clr, NULL, "Clear proc. registry", sdp_shell_cmd_test_clr),
	/* 'pub' command handler. */
	SHELL_CMD(pub, NULL, "Publish a measurement", sdp_shell_cmd_test_pub),
	/* 'stats' command handler. */
	SHELL_CMD(stats, NULL, "Display node cb stats", sdp_shell_cmd_test_stats),
	/* 'pool' command handler. */
	SHELL_CMD(pool, NULL, "Display meas. pool stats", sdp_shell_cmd_test_pool),
#if CONFIG_SDP_FILTER_CACHE
	/* 'cache' command handler. */
	SHELL_CMD(cache, NULL, "Display cache stats", sdp_shell_cmd_test_cache),
#endif

	/* Array terminator. */
	SHELL_SUBCMD_SET_END
	);

/* Root command "sdp" (level 0). */
SHELL_CMD_REGISTER(sdp, &sub_sdp, "Secure data pipeline commands", NULL);
