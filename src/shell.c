/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h> /* TODO: Remove when shell print with float fixed. */
#include <stdlib.h>
#include <ctype.h>
#include <shell/shell.h>
#include <sdp/datasample.h>

#if CONFIG_SDP_SHELL

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
sdp_shell_cmd_test(const struct shell *shell, size_t argc, char **argv)
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

	/* Test die temp data sample, with timestamp. */
	struct sdp_datasample sdp_test_data_sample_dietemp = {
		/* Data sample metadata. */
		.header = {
			/* Filter word. */
			.filter = {
				.data_type = SDP_DS_TYPE_TEMPERATURE,
				.ext_type = SDP_DS_EXT_TYPE_TEMP_DIE,
				.flags = {
					.data_format = SDP_DS_FORMAT_NONE,
					.encoding = SDP_DS_ENCODING_NONE,
					.compression = SDP_DS_COMPRESSION_NONE,
					.timestamp = SDP_DS_TIMESTAMP_EPOCH_32,
				},
			},
			/* SI Unit word. */
			.unit = {
				.si_unit = SDP_DS_UNIT_SI_DEGREE_CELSIUS,
				.ctype = SDP_DS_UNIT_CTYPE_IEEE754_FLOAT32,
				.scale_factor = SDP_DS_SI_SCALE_NONE,
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

	sdp_ds_print(&sdp_test_data_sample_dietemp);

	return 0;
}

/* Subcommand array for "sdp" (level 1). */
SHELL_STATIC_SUBCMD_SET_CREATE(
	sub_sdp,
	/* 'version' command handler. */
	SHELL_CMD(version, NULL, "library version", sdp_shell_cmd_version),
	/* 'test' command handler. */
	SHELL_CMD(test, NULL, "test command", sdp_shell_cmd_test),

	/* Array terminator. */
	SHELL_SUBCMD_SET_END
	);

/* Root command "sdp" (level 0). */
SHELL_CMD_REGISTER(sdp, &sub_sdp, "Secure data pipeline commands", NULL);

#endif
