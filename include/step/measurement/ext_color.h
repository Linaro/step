/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef STEP_MEASUREMENT_EXT_COLOR_H__
#define STEP_MEASUREMENT_EXT_COLOR_H__

#include <step/step.h>

/**
 * @file
 * @brief STEP_MES_TYPE_COLOR extended type definitions
 * @ingroup MEASUREMENT
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/** Extended measuremnt types for STEP_MES_TYPE_COLOR (8-bit). */
enum step_mes_ext_color {
	STEP_MES_EXT_TYPE_COLOR_UNDEFINED        = 0,    /**< Undefined RGBA */

	/* Standard RGB(+A) triplets */
	STEP_MES_EXT_TYPE_COLOR_RGBA8            = 0x10, /**< 8-bit RGBA */
	STEP_MES_EXT_TYPE_COLOR_RGBA16           = 0x11, /**< 16-bit RGBA */
	STEP_MES_EXT_TYPE_COLOR_RGBAF            = 0x12, /**< 0..1.0 float32 RGBA */

	/* CIE values. */
	STEP_MES_EXT_TYPE_COLOR_CIE1931_XYZ      = 0x20, /**< CIE1931 XYZ tristimulus */
	STEP_MES_EXT_TYPE_COLOR_CIE1931_XYY      = 0x21, /**< CIE1931 xyY chromaticity */
	STEP_MES_EXT_TYPE_COLOR_CIE1960_UCS      = 0x22, /**< CIE1960 UCS chromaticity */
	STEP_MES_EXT_TYPE_COLOR_CIE1976_UCS      = 0x23, /**< CIE1976 UCS chromaticity */
	STEP_MES_EXT_TYPE_COLOR_CIE1960_CCT      = 0x24, /**< CIE1960 CCT (Duv = 0) */
	STEP_MES_EXT_TYPE_COLOR_CIE1960_CCT_DUV  = 0x25, /**< CIE1960 CCT and Duv */
};

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* STEP_MEASUREMENT_EXT_COLOR_H__ */
