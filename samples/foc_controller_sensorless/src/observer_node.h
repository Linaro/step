/*
 * Copyright (c) 2022 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef OBSERVER_NODE_H__
#define OBSERVER_NODE_H__

#include <stdlib.h>
#include <stdbool.h>
#include <step/step.h>
#include <step/node.h>
#include <step/measurement/measurement.h>

#ifdef __cplusplus
extern "C" {
#endif

struct motor_observer_node_config {
    float motor_phase_resistance;
    float motor_phase_inductance;
    float motor_flux_linkage;
    float motor_poles;
    float observer_gain;
};

extern struct step_node *observer_node_chain;
extern struct motor_observer_node_config motor_observer_node_conf;

#ifdef __cplusplus
}
#endif

#endif /* STEP_SHELL_NODES_H_ */
