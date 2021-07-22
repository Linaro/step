/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h> /* TODO: Remove when shell print with float fixed. */
#include <stdlib.h>
#include <ctype.h>
#include <shell/shell.h>
#include <sdp/node.h>
#include <sdp/measurement/measurement.h>
#include <sdp/proc_mgr.h>

#if CONFIG_SDP_SHELL

struct sdp_test_data_procnode_cb_stats {
	uint32_t evaluate;
	uint32_t matched;
	uint32_t start;
	uint32_t run;
	uint32_t stop;
	uint32_t error;
};

/* Track callback entry statistics. */
struct sdp_test_data_procnode_cb_stats sdp_test_data_cb_stats = { 0 };

bool node_evaluate(struct sdp_measurement *mes, void *cfg)
{
	/* Overrides the filter engine when evaluating this node. */
	sdp_test_data_cb_stats.evaluate++;

	return true;
}

bool node_matched(struct sdp_measurement *mes, void *cfg)
{
	/* Fires when the filter engine has indicated a match for this node. */
	sdp_test_data_cb_stats.matched++;

	return true;
}

int node_start(struct sdp_measurement *mes, void *cfg)
{
	/* Fires before the node runs. */
	sdp_test_data_cb_stats.start++;

	return 0;
}

int node_run(struct sdp_measurement *mes, void *cfg)
{
	/* Node logic implementation. */
	sdp_test_data_cb_stats.run++;

	return 0;
}

int node_stop(struct sdp_measurement *mes, void *cfg)
{
	/* Fires when the node has been successfully run. */
	sdp_test_data_cb_stats.stop++;

	return 0;
}

void node_error(struct sdp_measurement *mes, void *cfg, int error)
{
	/* Fires when an error occurs running this node. */
	sdp_test_data_cb_stats.error++;
}

/* Processor node chain. */
struct sdp_node sdp_test_data_procnode_chain_data[] = {
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
		.next = &sdp_test_data_procnode_chain_data[1]
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
struct sdp_node *sdp_test_data_procnode_chain =
	sdp_test_data_procnode_chain_data;

static int
sdp_shell_invalid_arg(const struct shell *shell, char *arg_name)
{
	shell_print(shell, "Error: invalid argument \"%s\"\n", arg_name);

	return -EINVAL;
}

static int
sdp_shell_cmd_version(const struct shell *shell, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	shell_print(shell, "%s", "0.0.0");

	return 0;
}

static int
sdp_shell_cmd_test_msg(const struct shell *shell, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	/* Die temperature with 32-bit timestamp payload. */
	struct {
		float temp_c;
		uint32_t timestamp;
	} sdp_test_data_dietemp_payload = {
		.temp_c = 32.0F,
		.timestamp = 1624305803, /* Monday, June 21, 2021 8:03:23 PM */
	};

	/* Test die temp measurement, with timestamp. */
	struct sdp_measurement sdp_test_mes_dietemp = {
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
				.len = sizeof(sdp_test_data_dietemp_payload),
				.fragment = 0,
				.sourceid = 10,
			},
		},

		/* Die temperature in C plus 32-bit epoch timestamp. */
		.payload = &sdp_test_data_dietemp_payload,
	};

	int matches;

	sdp_pm_process(&sdp_test_mes_dietemp, &matches, false);
	printk("%d match(es) during processing.\n", matches);
	// sdp_mes_print(&sdp_test_mes_dietemp);

	return 0;
}

static int
sdp_shell_cmd_test_proc(const struct shell *shell, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	uint8_t handle = 0;

	sdp_pm_clear();

	sdp_pm_register(sdp_test_data_procnode_chain, 0, &handle);

	// sdp_node_print(sdp_test_data_procnode_chain);
	sdp_pm_list();

	return 0;
}

static int
sdp_shell_cmd_test_stats(const struct shell *shell, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	printk("evaluate: %d\n", sdp_test_data_cb_stats.evaluate);
	printk("matched:  %d\n", sdp_test_data_cb_stats.matched);
	printk("start:    %d\n", sdp_test_data_cb_stats.start);
	printk("run:      %d\n", sdp_test_data_cb_stats.run);
	printk("stop:     %d\n", sdp_test_data_cb_stats.stop);
	printk("error:    %d\n", sdp_test_data_cb_stats.error);

	return 0;
}

/* Subcommand array for "sdp" (level 1). */
SHELL_STATIC_SUBCMD_SET_CREATE(
	sub_sdp,
	/* 'version' command handler. */
	SHELL_CMD(version, NULL, "Library version", sdp_shell_cmd_version),
	/* 'msg' command handler. */
	SHELL_CMD(msg, NULL, "Process test message", sdp_shell_cmd_test_msg),
	/* 'proc' command handler. */
	SHELL_CMD(proc, NULL, "Populate processor registry", sdp_shell_cmd_test_proc),
	/* 'stats' command handler. */
	SHELL_CMD(stats, NULL, "Prints processing stats", sdp_shell_cmd_test_stats),

	/* Array terminator. */
	SHELL_SUBCMD_SET_END
	);

/* Root command "sdp" (level 0). */
SHELL_CMD_REGISTER(sdp, &sub_sdp, "Secure data pipeline commands", NULL);

#endif
