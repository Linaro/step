/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef SDP_DATASAMPLE_EXT_NUMBER_H__
#define SDP_DATASAMPLE_EXT_NUMBER_H__

#include <sdp/sdp.h>

/**
 * @file
 * @brief SDP_DS_TYPE_NUMBER extended type definitions
 * @ingroup DATASAMPLE
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/** Extended data types for SDP_DS_TYPE_NUMBER (8-bit). */
enum sdp_ds_ext_number {
	/** Undefined */
	SDP_DS_EXT_TYPE_NUM_UNDEFINED   = 0,

	/* Unsigned integers. */
	SDP_DS_EXT_TYPE_NUM_U8          = 0x1,  /**< uint8_t */
	SDP_DS_EXT_TYPE_NUM_U16         = 0x2,  /**< uint16_t */
	SDP_DS_EXT_TYPE_NUM_U32         = 0x3,  /**< uint32_t */
	SDP_DS_EXT_TYPE_NUM_U64         = 0x4,  /**< uint64_t */
	SDP_DS_EXT_TYPE_NUM_U128        = 0x5,  /**< uint128_t */

	/* Signed integers. */
	SDP_DS_EXT_TYPE_NUM_S8          = 0x10, /**< int8_t */
	SDP_DS_EXT_TYPE_NUM_S16         = 0x11, /**< int16_t */
	SDP_DS_EXT_TYPE_NUM_S32         = 0x12, /**< int32_t */
	SDP_DS_EXT_TYPE_NUM_S64         = 0x13, /**< int64_t */
	SDP_DS_EXT_TYPE_NUM_S128        = 0x14, /**< int128_t */

	/* IEEE 754 floats. */
	SDP_DS_EXT_TYPE_NUM_IEEE754_F16 = 0x20, /**< 16-bit IEEE754 float */
	SDP_DS_EXT_TYPE_NUM_IEEE754_F32 = 0x21, /**< 32-bit IEEE754 float */
	SDP_DS_EXT_TYPE_NUM_IEEE754_F64 = 0x22, /**< 64-bit IEEE754 float */
};

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* SDP_DATASAMPLE_EXT_NUMBER_H__ */
