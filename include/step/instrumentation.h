/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef STEP_INSTRUMENTATION_H__
#define STEP_INSTRUMENTATION_H__

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <step/step.h>

/**
 * @file
 */

#if CONFIG_STEP_INSTRUMENTATION
/**
 * @brief Reads the high-precision timer start time.
 */
#define STEP_INSTR_START(t) do {	      \
		t = k_cycle_get_32(); \
} while (0);

/**
 * @brief Reads the high-precision timer stop time and converts to ns. This
 *        function modifies the value of t to store the execution time in ns.
 */
#define STEP_INSTR_STOP(t) do {				      \
		uint32_t end = k_cycle_get_32();	      \
		t = end < t ? 0xFFFFFFFF - t + end : end - t; \
		t = k_cyc_to_ns_floor32(t);		      \
} while (0);

#else
#define STEP_INSTR_START(t) do { } while(0);
#define STEP_INSTR_STOP(t) do { } while(0);
#endif

#endif /* STEP_INSTRUMENTATION_H_ */
