/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef SDP_DATASAMPLE_BASE_H__
#define SDP_DATASAMPLE_BASE_H__

#include <sdp/sdp.h>

/**
 * @file
 * @brief Base data type definitions
 * @ingroup DATASAMPLE
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/** Base data sample types (8-bit) */
enum sdp_ds_type {
	/* 0 = Reserved */
	SDP_DS_TYPE_UNDEFINED           = 0,    /**< Undefined */

	/* 2 = System event to alert to processors and sinks. */
	SDP_DS_TYPE_EVENT               = 0x2,  /**< System event */

	/* 0x5..0xF (5-15): Standard data types (uncategorised data). */
	SDP_DS_TYPE_NUMBER              = 0x5,  /**< Generic numeric value */
	SDP_DS_TYPE_STRING              = 0x6,  /**< Null-terminated string */

	/* 0x10..0x7F (16-127): Standardised base data types. */
	/* Area? */
	SDP_DS_TYPE_AUDIO               = 0x10,
	SDP_DS_TYPE_ACCELEROMETER       = 0x11, /**< m/s^2 */
	SDP_DS_TYPE_AMPLITUDE           = 0x12, /**< 0..1.0 float */
	SDP_DS_TYPE_CAPACITANCE         = 0x13, /**< uF */
	SDP_DS_TYPE_COLOR               = 0x14, /**< See extended type */
	SDP_DS_TYPE_COORDINATES         = 0x15,
	SDP_DS_TYPE_CURRENT             = 0x16, /**< mA */
	SDP_DS_TYPE_DIMENSION           = 0x17, /**< cm (length, width, etc.) */
	SDP_DS_TYPE_DISTANCE            = 0x18, /**< space between two points */
	SDP_DS_TYPE_FREQUENCY           = 0x19, /**< kHz */
	SDP_DS_TYPE_GRAVITY             = 0x1A, /**< m/s^2 */
	SDP_DS_TYPE_GYROSCOPE           = 0x1B, /**< rad/s */
	SDP_DS_TYPE_HUMIDITY            = 0x1C, /**< relative humidity in percent */
	SDP_DS_TYPE_INDUCTANCE          = 0x1D, /**< nH? */
	SDP_DS_TYPE_LIGHT               = 0x1E, /**< Lux */
	SDP_DS_TYPE_LINEAR_ACCELERATION = 0x1F, /**< m/s^2, WITHOUT gravity */
	SDP_DS_TYPE_MAGNETIC_FIELD      = 0x20, /**< micro-Tesla */
	SDP_DS_TYPE_MASS                = 0x21, /**< Grams */
	SDP_DS_TYPE_ORIENTATION         = 0x22,
	SDP_DS_TYPE_PHASE               = 0x23, /**< radians? */
	SDP_DS_TYPE_PRESSURE            = 0x24, /**< hectopascal (hPa) */
	SDP_DS_TYPE_QUATERNION          = 0x25,
	SDP_DS_TYPE_RESISTANCE          = 0x26, /**< Ohms */
	/* Rotation speed in rpm? */
	SDP_DS_TYPE_ROTATION_VECTOR     = 0x27,
	SDP_DS_TYPE_SPECTRAL_POWER      = 0x28, /**< Array of nm + ampl. values */
	SDP_DS_TYPE_TEMPERATURE         = 0x29, /**< Celcius */
	SDP_DS_TYPE_TIME                = 0x2A, /**< Default = Epoch, Duration? */
	SDP_DS_TYPE_VOLTAGE             = 0x2B, /**< mV? */

	/* 0xF0..0xFE (240-254): User-defined types. */
	SDP_DS_TYPE_USER_1              = 0xF0, /**< User defined 1 */
	SDP_DS_TYPE_USER_2              = 0xF1, /**< User defined 2 */
	SDP_DS_TYPE_USER_3              = 0xF2, /**< User defined 3 */
	SDP_DS_TYPE_USER_4              = 0xF3, /**< User defined 4 */
	SDP_DS_TYPE_USER_5              = 0xF4, /**< User defined 5 */
	SDP_DS_TYPE_USER_6              = 0xF5, /**< User defined 6 */
	SDP_DS_TYPE_USER_7              = 0xF6, /**< User defined 7 */
	SDP_DS_TYPE_USER_8              = 0xF7, /**< User defined 8 */
	SDP_DS_TYPE_USER_9              = 0xF8, /**< User defined 9 */
	SDP_DS_TYPE_USER_10             = 0xF9, /**< User defined 10 */
	SDP_DS_TYPE_USER_11             = 0xFA, /**< User defined 11 */
	SDP_DS_TYPE_USER_12             = 0xFB, /**< User defined 12 */
	SDP_DS_TYPE_USER_13             = 0xFC, /**< User defined 13 */
	SDP_DS_TYPE_USER_14             = 0xFD, /**< User defined 14 */
	SDP_DS_TYPE_USER_15             = 0xFE, /**< User defined 15 */

	/* 0xFF (255) reserved for future use. */
	SDP_DS_TYPE_LAST                = 0xFF  /**< Reserved */
};

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* SDP_DATASAMPLE_BASE_H__ */
