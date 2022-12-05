/*
 * Copyright (c) 2022 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KINEMATIC_NODE_H__
#define KINEMATIC_NODE_H__

#include <stdlib.h>
#include <stdbool.h>
#include <step/step.h>
#include <step/node.h>
#include <step/measurement/measurement.h>

#ifdef __cplusplus
extern "C" {
#endif

struct mobile_robot_kinematics {
    float length_x;
    float length_y;
    float wheel_radius;
    int total_wheels;
};

struct mobile_robot_joint_speeds {
    float joint_speed_rpm[4];
};

struct mobile_robot_body_velocity {
    float linear_velocity_x;
    float linear_velocity_y;
    float heading;
};

struct mobile_robot_command_data {
    float joint_desired_speed_rpm[4];
    float desired_body_speed_x;
    float desired_body_speed_y;
    float desired_body_heading;
};

struct mobile_robot_node_payload {
    struct mobile_robot_kinematics kinematics;
    struct mobile_robot_joint_speeds joints;
    struct mobile_robot_body_velocity body;
    struct mobile_robot_command_data command;
};

extern struct step_node *node_chain;

#ifdef __cplusplus
}
#endif

#endif /* KINEMATIC_NODE_H__ */
