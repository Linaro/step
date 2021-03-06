/*
 * Copyright (c) 2022 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef FUSION_NODES_H__
#define FUSION_NODES_H__

#include <stdlib.h>
#include <stdbool.h>
#include <step/step.h>
#include <step/node.h>
#include <step/measurement/measurement.h>

#include <zsl/zsl.h>
#include <zsl/orientation/fusion/fusion.h>
#include <zsl/orientation/orientation.h>

#ifdef __cplusplus
extern "C" {
#endif

extern struct zsl_euler fusion_node_result;
extern struct step_node *fusion_node_chain;

#ifdef __cplusplus
}
#endif

#endif /* STEP_SHELL_NODES_H_ */
