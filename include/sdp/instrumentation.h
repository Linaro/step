/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef SDP_INSTRUMENTATION_H__
#define SDP_INSTRUMENTATION_H__

#include <zephyr.h>
#include <sys/printk.h>
#include <sdp/sdp.h>

#if CONFIG_SDP_INSTRUMENTATION
/**
 * @brief Reads the high-precision timer start time.
 */
#define SDP_INSTR_START(t) do {	      \
		t = k_cycle_get_32(); \
} while (0);

/**
 * @brief Reads the high-precision timer stop time and converts to ns. This
 *        function modifies the value of t to store the execution time in ns.
 */
#define SDP_INSTR_STOP(t) do {				      \
		uint32_t end = k_cycle_get_32();	      \
		t = end < t ? 0xFFFFFFFF - t + end : end - t; \
		t = k_cyc_to_ns_floor32(t);		      \
} while (0);

#else
#define SDP_INSTR_START(t) do { } while(0);
#define SDP_INSTR_STOP(t) do { } while(0);
#endif

#endif /* SDP_INSTRUMENTATION_H_ */
