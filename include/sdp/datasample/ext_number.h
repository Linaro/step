/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef SDP_DATASAMPLE_EXT_NUMBER_H__
#define SDP_DATASAMPLE_EXT_NUMBER_H__

#include <sdp/sdp.h>

/**
 * @defgroup DATASAMPLE Data sample definitions.
 * @ingroup sdp_api
 * @{
 */

/**
 * @file
 * @brief SDP data sample SDP_DS_TYPE_NUMBER extended type definitions
 */

#ifdef __cplusplus
extern "C" {
#endif

/** Extended data types for SDP_DS_TYPE_NUMBER (8-bit). */
enum sdp_ds_ext_number {
	SDP_DS_EXT_TYPE_NUM_UNDEFINED   = 0,

	/* Unsigned integers. */
	SDP_DS_EXT_TYPE_NUM_U8          = 0x1,
	SDP_DS_EXT_TYPE_NUM_U16         = 0x2,
	SDP_DS_EXT_TYPE_NUM_U32         = 0x3,
	SDP_DS_EXT_TYPE_NUM_U64         = 0x4,
	SDP_DS_EXT_TYPE_NUM_U128        = 0x5,

	/* Signed integers. */
	SDP_DS_EXT_TYPE_NUM_S8          = 0x10,
	SDP_DS_EXT_TYPE_NUM_S16         = 0x11,
	SDP_DS_EXT_TYPE_NUM_S32         = 0x12,
	SDP_DS_EXT_TYPE_NUM_S64         = 0x13,
	SDP_DS_EXT_TYPE_NUM_S128        = 0x14,

	/* IEEE 754 floats. */
	SDP_DS_EXT_TYPE_NUM_IEEE754_F16 = 0x20,
	SDP_DS_EXT_TYPE_NUM_IEEE754_F32 = 0x21,
	SDP_DS_EXT_TYPE_NUM_IEEE754_F64 = 0x22,
};

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* SDP_DATASAMPLE_EXT_NUMBER_H__ */
