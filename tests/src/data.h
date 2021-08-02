/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_INCLUDE_STEP_TEST_DATA_H_
#define ZEPHYR_INCLUDE_STEP_TEST_DATA_H_

#include <step/measurement/measurement.h>

struct step_test_data_procnode_cb_stats {
	uint32_t evaluate;
	uint32_t matched;
	uint32_t start;
	uint32_t run;
	uint32_t stop;
	uint32_t error;
};

extern struct step_node *step_test_data_procnode_chain;
extern struct step_node step_test_data_procnode;
extern struct step_measurement step_test_mes_dietemp;
extern struct step_test_data_procnode_cb_stats step_test_data_cb_stats;

#endif /* ZEPHYR_INCLUDE_STEP_TEST_DATA_H_ */
