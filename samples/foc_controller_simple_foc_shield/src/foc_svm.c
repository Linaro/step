/*
 * Copyright (c) 2021 Teslabs Engineering S.L.
 * Copyright (c) 2022 Linaro 
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/sys/util.h>
#include <arm_math.h>
#include "foc_svm.h"

/** Value sqrt(3). */
#define SQRT_3 1.7320508075688773f
static const float sqrt3_by_two = SQRT_3 / 2.0f;

void foc_svm_set(float v_alpha, float v_beta, float *dca, float *dcb, float *dcc)
{
	*dca = v_alpha + 0.5f;
	*dcb = 0.5f * -v_alpha + sqrt3_by_two * v_beta + 0.5f;
	*dcc = 0.5f * -v_alpha - sqrt3_by_two * v_beta + 0.5f; 
}
