/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef STEP_SHELL_PNODES_H__
#define STEP_SHELL_PNODES_H__

#include <stdlib.h>
#include <stdbool.h>
#include <step/step.h>
#include <step/node.h>
#include <step/measurement/measurement.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Struct to track the number of times specific callbacks are fired. */
struct step_node_cb_stats {
	uint32_t evaluate;
	uint32_t matched;
	uint32_t start;
	uint32_t run;
	uint32_t stop;
	uint32_t error;
};

/* Defined in pnodes.c */
extern struct step_node_cb_stats cb_stats;
extern struct step_node *test_node_chain;

#ifdef __cplusplus
}
#endif

#endif /* STEP_SHELL_PNODES_H_ */
