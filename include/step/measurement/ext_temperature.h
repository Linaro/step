/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef STEP_MEASUREMENT_EXT_TEMP_H__
#define STEP_MEASUREMENT_EXT_TEMP_H__

#include <step/step.h>

/**
 * @file
 * @brief STEP_MES_TYPE_TEMPERATURE extended type definitions
 * @ingroup MEASUREMENT
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/** Extended measurement types for STEP_MES_TYPE_TEMPERATURE (8-bit). */
enum step_mes_ext_temperature {
	STEP_MES_EXT_TYPE_TEMP_UNDEFINED  = 0,    /**< Undefined temperature */
	STEP_MES_EXT_TYPE_TEMP_AMBIENT    = 1,    /**< Ambient temperature */
	STEP_MES_EXT_TYPE_TEMP_DIE        = 2,    /**< Die temperature */
	STEP_MES_EXT_TYPE_TEMP_OBJECT     = 3,    /**< Object temperature */
};

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* STEP_MEASUREMENT_EXT_TEMP_H__ */
