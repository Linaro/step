/*
 * Copyright (c) 2022 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdbool.h>
#include "foc_driver.h"

/**
 * @brief Starts scope data streaming.
 */
void foc_scope_enable(void);

/**
 * @brief Stops scope data streaming.
 */
void foc_scope_disable(void);

/**
 * @brief Push new FoC sample to scope driver.
 * 
 * @param p pointer to a sample containing FoC data.
 * 
 * @note no action is taken if p is invalid or if the buffer is full.
 * 
 */
void foc_scope_data_push(struct foc_controller_payload *p);