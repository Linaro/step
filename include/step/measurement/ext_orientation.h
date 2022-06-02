/*
 * Copyright (c) 2022 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef STEP_MEASUREMENT_EXT_ORIENT_H__
#define STEP_MEASUREMENT_EXT_ORIENT_H__

#include <step/step.h>

/**
 * @brief STEP_MES_TYPE_ORIENTATION extended type definitions
 * @ingroup MEASUREMENT
 * @{
 */

/**
 * @file
 */

#ifdef __cplusplus
extern "C" {
#endif

/** Extended measurement types for STEP_MES_TYPE_ORIENTATION (8-bit). */
enum step_mes_ext_orientation {
	STEP_MES_EXT_TYPE_ORIENTATION_UNDEFINED        = 0, /**< Radian */
	STEP_MES_EXT_TYPE_ORIENTATION_ANG_MEASURE      = 1, /**< Radian or Degree */
	STEP_MES_EXT_TYPE_ORIENTATION_UNIT_QUATERNION  = 2,	/**< AKA versor */
};

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* STEP_MEASUREMENT_EXT_ORIENT_H__ */
