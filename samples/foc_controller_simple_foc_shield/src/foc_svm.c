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

/**
 * @brief Obtain sector based on a, b, c vector values.
 *
 * @param[in] a a component value.
 * @param[in] b b component value.
 * @param[in] c c component value.

 * @return Sector (1...6).
 */
static uint8_t get_sector(float a, float b, float c)
{
	uint8_t sector = 0U;

	if (c < 0.0f) {
		if (a < 0.0f) {
			sector = 2U;
		} else {
			if (b < 0.0f) {
				sector = 6U;
			} else {
				sector = 1U;
			}
		}
	} else {
		if (a < 0.0f) {
			if (b <= 0.0f) {
				sector = 4U;
			} else {
				sector = 3U;
			}
		} else {
			sector = 5U;
		}
	}

	return sector;
}

void foc_svm_set(float v_alpha, float v_beta, float *dca, float *dcb, float *dcc)
{
	float a, b, c, mod;
	float x, y, z;

	/* normalize and limit alpha-beta vector */
	(void)arm_sqrt_f32(v_alpha * v_alpha + v_beta * v_beta, &mod);
	if (mod > sqrt3_by_two) {
		v_alpha = v_alpha / mod * sqrt3_by_two;
		v_beta = v_beta / mod * sqrt3_by_two;
	}

	/* do a modified inverse clarke transform to get an auxiliary frame 
	 * to compute the sector we are
	 */
	a = v_alpha;
	b = 0.5f * (-v_alpha - SQRT_3 * v_beta);
	c = 0.5f * (-v_alpha + SQRT_3 * v_beta); 

	switch (get_sector(a, b, c)) {
	case 1U:
		x = a;
		y = b;
		z = 1.0f - (x + y);

		*dca = x + y + z * 0.5f;
		*dcb = y + z * 0.5f;
		*dcc = z * 0.5f;

		break;
	case 2U:
		x = -c;
		y = -a;
		z = 1.0f - (x + y);

		*dca = x + z * 0.5f;
		*dcb = x + y + z * 0.5f;
		*dcc = z * 0.5f;

		break;
	case 3U:
		x = b;
		y = c;
		z = 1.0f - (x + y);

		*dca = z * 0.5f;
		*dcb = x + y + z * 0.5f;
		*dcc = y + z * 0.5f;

		break;
	case 4U:
		x = -a;
		y = -b;
		z = 1.0f - (x + y);

		*dca = z * 0.5f;
		*dcb = x + z * 0.5f;
		*dcc = x + y + z * 0.5f;

		break;
	case 5U:
		x = c;
		y = a;
		z = 1.0f - (x + y);

		*dca = y + z * 0.5f;
		*dcb = z * 0.5f;
		*dcc = x + y + z * 0.5f;

		break;
	case 6U:
		x = -b;
		y = -c;
		z = 1.0f - (x + y);

		*dca = x + y + z * 0.5f;
		*dcb = z * 0.5f;
		*dcc = x + z * 0.5f;

		break;
	default:
		break;
	}
}
