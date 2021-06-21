/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef SDP_DATASAMPLE_EXT_COLOR_H__
#define SDP_DATASAMPLE_EXT_COLOR_H__

#include <sdp/sdp.h>

/**
 * @defgroup DATASAMPLE Data sample definitions.
 * @ingroup sdp_api
 * @{
 */

/**
 * @file
 * @brief SDP data sample SDP_DS_TYPE_COLOR extended type definitions
 */

#ifdef __cplusplus
extern "C" {
#endif

/** Extended data type values for SDP_DS_TYPE_COLOR (8-bit). */
enum sdp_ds_ext_color {
	SDP_DS_EXT_TYPE_COLOR_UNDEFINED         = 0,    /**< Undefined RGBA */

	/* Standard RGB(+A) triplets */
	SDP_DS_EXT_TYPE_COLOR_RGBA8             = 1,    /**< 8-bit RGBA */
	SDP_DS_EXT_TYPE_COLOR_RGBA16            = 2,    /**< 16-bit RGBA */
	SDP_DS_EXT_TYPE_COLOR_RGBAF             = 3,    /**< 0..1.0 float32 RGBA */
	
	/* CIE values. */
	SDP_DS_EXT_TYPE_COLOR_CIE1931_XYZ       = 10,   /**< CIE1931 XYZ tristimulus */
	SDP_DS_EXT_TYPE_COLOR_CIE1931_XYY       = 11,   /**< CIE1931 xyY chromaticity */
	SDP_DS_EXT_TYPE_COLOR_CIE1960_UCS       = 12,   /**< CIE1960 UCS chromaticity */
	SDP_DS_EXT_TYPE_COLOR_CIE1976_UCS       = 13,   /**< CIE1976 UCS chromaticity */
	SDP_DS_EXT_TYPE_COLOR_CIE1960_CCT       = 20,   /**< CIE1960 CCT (Duv = 0) */
	SDP_DS_EXT_TYPE_COLOR_CIE1960_CCT_DUV   = 21,   /**< CIE1960 CCT and Duv */
};

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* SDP_DATASAMPLE_EXT_COLOR_H__ */
