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
#include <sdp/sample_pool.h>
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
	shell_print(shell, "Error: invalid argument \"%s\"", arg_name);

	return -EINVAL;
}

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
	sdp_pm_register(sdp_test_data_procnode_chain, 0, &handle);
	sdp_node_print(sdp_test_data_procnode_chain);

	return 0;
}

static int
sdp_shell_cmd_test_clr(const struct shell *shell, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	/* Reset the processor node registry. */
	sdp_pm_clear();

	return 0;
}

static int
sdp_shell_cmd_test_msg(const struct shell *shell, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	/* Die temperature with 32-bit timestamp payload. */
	struct {
		uint32_t timestamp;
		float temp_c;
	} sdp_test_data_dietemp_payload = {
		.timestamp = 1624305803, /* Monday, June 21, 2021 8:03:23 PM */
		.temp_c = 32.0F,
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

#if (CONFIG_SDP_PROC_MGR_POLL_RATE > 0)
	/* Let the polling thread do all the work. */
	struct sdp_measurement *mes;

	/* Allocate memory for measurement. */
	mes = sdp_sp_alloc(sdp_test_mes_dietemp.header.srclen.len);

	/* Copy test data into heap-based measurement instance. */
	memcpy(&(mes->header), &(sdp_test_mes_dietemp.header),
	       sizeof(struct sdp_mes_header));
	memcpy(mes->payload, sdp_test_mes_dietemp.payload,
	       sdp_test_mes_dietemp.header.srclen.len);

	/* Assign measurement to FIFO. */
	sdp_sp_put(mes);
#else
	/* Manually process the measurement when no polling thread is present. */
	int matches;
	sdp_pm_process(&sdp_test_mes_dietemp, &matches, false);
	shell_print(shell,
		    "%d match(es) during processing.", matches);
#endif

	shell_print(shell, "Published 1 measurement:");
	sdp_mes_print(&sdp_test_mes_dietemp);

	return 0;
}

static int
sdp_shell_cmd_test_stats(const struct shell *shell, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	shell_print(shell, "evaluate: %d", sdp_test_data_cb_stats.evaluate);
	shell_print(shell, "matched:  %d", sdp_test_data_cb_stats.matched);
	shell_print(shell, "start:    %d", sdp_test_data_cb_stats.start);
	shell_print(shell, "run:      %d", sdp_test_data_cb_stats.run);
	shell_print(shell, "stop:     %d", sdp_test_data_cb_stats.stop);
	shell_print(shell, "error:    %d", sdp_test_data_cb_stats.error);

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
/* Subcommand array for "sdp" (level 1). */
SHELL_STATIC_SUBCMD_SET_CREATE(
	sub_sdp,
	/* 'list' command handler. */
	SHELL_CMD(list, NULL, "Display proc. registry", sdp_shell_cmd_test_list),
	/* 'add' command handler. */
	SHELL_CMD(add, NULL, "Populate proc. registry", sdp_shell_cmd_test_add),
	/* 'clr' command handler. */
	SHELL_CMD(clr, NULL, "Clear proc. registry", sdp_shell_cmd_test_clr),
	/* 'msg' command handler. */
	SHELL_CMD(msg, NULL, "Publish a measurement", sdp_shell_cmd_test_msg),
	/* 'stats' command handler. */
	SHELL_CMD(stats, NULL, "Display processing stats", sdp_shell_cmd_test_stats),
	/* 'pool' command handler. */
	SHELL_CMD(pool, NULL, "Display meas. pool stats", sdp_shell_cmd_test_pool),

	/* Array terminator. */
	SHELL_SUBCMD_SET_END
	);

/* Root command "sdp" (level 0). */
SHELL_CMD_REGISTER(sdp, &sub_sdp, "Secure data pipeline commands", NULL);

#endif
