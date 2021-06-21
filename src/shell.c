/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h> /* TODO: Remove when shell print with float fixed. */
#include <stdlib.h>
#include <ctype.h>
#include <shell/shell.h>

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

/* Subcommand array for "sdp" (level 1). */
SHELL_STATIC_SUBCMD_SET_CREATE(
	sub_sdp,
	/* 'version' command handler. */
	SHELL_CMD(version, NULL, "library version", sdp_shell_cmd_version),

	/* Array terminator. */
	SHELL_SUBCMD_SET_END
	);

/* Root command "sdp" (level 0). */
SHELL_CMD_REGISTER(sdp, &sub_sdp, "Secure data pipeline commands", NULL);

#endif
