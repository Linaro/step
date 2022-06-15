/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef STEP_SHELL_DRIVER_H__
#define STEP_SHELL_DRIVER_H__

#include <step/step.h>
#include <step/measurement/measurement.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Payload struct for convenience sake.
 */
struct drv_payload {
	uint32_t timestamp;
	float temp_c;
};

/**
 * @brief Gets a populated measurement from the driver.
 * 
 * @param mes   Pointer to the @ref step_measurement placeholder pointer.
 * 
 * @return int  0 on success, otherwise a negative error code.
 * 
 * @note Double pointer required for @ref mes since the sample pool alloc
 *       function will change the measurement location in memory.
 */
int step_shell_drv_get_mes(struct step_measurement **mes);

#ifdef __cplusplus
}
#endif

#endif /* STEP_SHELL_DRIVER_H_ */
