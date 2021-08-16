/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef STEP_MEASUREMENT_BASE_H__
#define STEP_MEASUREMENT_BASE_H__

#include <step/step.h>

/**
 * @brief Base measurement type definitions
 * @ingroup MEASUREMENT
 * @{
 */

/**
 * @file
 */

#ifdef __cplusplus
extern "C" {
#endif

/** Base measurement types (8-bit) */
enum step_mes_type {
	/* 0 = Reserved */
	STEP_MES_TYPE_UNDEFINED         = 0x00,         /**< Undefined */

	/* 0x10..0x7F (16-127): Standardised base measurement types. */
	STEP_MES_TYPE_AREA              = 0x10,         /**< STEP_MES_UNIT_SI_METERS_2 */
	STEP_MES_TYPE_ACCELERATION      = 0x11,         /**< STEP_MES_UNIT_SI_METER_PER_SECOND_2 (linear, gravity) */
	STEP_MES_TYPE_AMPLITUDE         = 0x12,         /**< STEP_MES_UNIT_SI_INTERVAL */
	STEP_MES_TYPE_CAPACITANCE       = 0x13,         /**< STEP_MES_UNIT_SI_FARAD */
	STEP_MES_TYPE_COLOR             = 0x14,         /**< See extended type */
	STEP_MES_TYPE_COORDINATES       = 0x15,         /**< XY vector? */
	STEP_MES_TYPE_CURRENT           = 0x16,         /**< STEP_MES_UNIT_SI_AMPERE */
	STEP_MES_TYPE_DIMENSION         = 0x17,         /**< STEP_MES_UNIT_SI_METER (length, width, radius, distance, etc.) */
	STEP_MES_TYPE_FREQUENCY         = 0x18,         /**< STEP_MES_UNIT_SI_HERTZ */
	STEP_MES_TYPE_HUMIDITY          = 0x19,         /**< STEP_MES_UNIT_SI_RELATIVE_HUMIDITY */
	STEP_MES_TYPE_INDUCTANCE        = 0x1A,         /**< STEP_MES_UNIT_SI_HENRY */
	STEP_MES_TYPE_LIGHT             = 0x1B,         /**< STEP_MES_UNIT_SI_LUX */
	STEP_MES_TYPE_MAGNETIC_FIELD    = 0x1C,         /**< STEP_MES_UNIT_SI_TESLA */
	STEP_MES_TYPE_MASS              = 0x1D,         /**< STEP_MES_UNIT_SI_KILOGRAM */
	STEP_MES_TYPE_MOMENTUM          = 0x1E,         /**< Angular, Linear, Inertia */
	STEP_MES_TYPE_ORIENTATION       = 0x1F,         /**< XYZ vector */
	STEP_MES_TYPE_PHASE             = 0x20,         /**< STEP_MES_UNIT_SI_RADIAN */
	STEP_MES_TYPE_PRESSURE          = 0x21,         /**< STEP_MES_UNIT_SI_PASCAL */
	STEP_MES_TYPE_RESISTANCE        = 0x22,         /**< STEP_MES_UNIT_SI_OHM */
	STEP_MES_TYPE_SOUND             = 0x23,         /**< STEP_MES_UNIT_SI_HERTZ */
	STEP_MES_TYPE_TEMPERATURE       = 0x24,         /**< STEP_MES_UNIT_SI_DEGREE_CELSIUS */
	STEP_MES_TYPE_TIME              = 0x25,         /**< STEP_MES_UNIT_SI_SECOND */
	STEP_MES_TYPE_VELOCITY          = 0x26,         /**< STEP_MES_UNIT_SI_METERS_3_SECOND */
	STEP_MES_TYPE_VOLTAGE           = 0x27,         /**< STEP_MES_UNIT_SI_VOLT */
	STEP_MES_TYPE_VOLUME            = 0x28,         /**< STEP_MES_UNIT_SI_METERS_3 */
	STEP_MES_TYPE_ACIDITY           = 0x29,         /**< STEP_MES_UNIT_SI_PH */
	STEP_MES_TYPE_CONDUCTIVITY      = 0x2A,         /**< STEP_MES_UNIT_SI_SIEMENS_PER_METER */
	STEP_MES_TYPE_FORCE             = 0x2B,         /**< STEP_MES_UNIT_SI_NEWTON */
	STEP_MES_TYPE_ENERGY            = 0x2C,         /**< STEP_MES_UNIT_SI_JOULE */

	/* 0xF0..0xFE (240-254): User-defined types. */
	STEP_MES_TYPE_USER_1            = 0xF0,         /**< User defined 1 */
	STEP_MES_TYPE_USER_2            = 0xF1,         /**< User defined 2 */
	STEP_MES_TYPE_USER_3            = 0xF2,         /**< User defined 3 */
	STEP_MES_TYPE_USER_4            = 0xF3,         /**< User defined 4 */
	STEP_MES_TYPE_USER_5            = 0xF4,         /**< User defined 5 */
	STEP_MES_TYPE_USER_6            = 0xF5,         /**< User defined 6 */
	STEP_MES_TYPE_USER_7            = 0xF6,         /**< User defined 7 */
	STEP_MES_TYPE_USER_8            = 0xF7,         /**< User defined 8 */
	STEP_MES_TYPE_USER_9            = 0xF8,         /**< User defined 9 */
	STEP_MES_TYPE_USER_10           = 0xF9,         /**< User defined 10 */
	STEP_MES_TYPE_USER_11           = 0xFA,         /**< User defined 11 */
	STEP_MES_TYPE_USER_12           = 0xFB,         /**< User defined 12 */
	STEP_MES_TYPE_USER_13           = 0xFC,         /**< User defined 13 */
	STEP_MES_TYPE_USER_14           = 0xFD,         /**< User defined 14 */
	STEP_MES_TYPE_USER_15           = 0xFE,         /**< User defined 15 */

	/* 0xFF (255) reserved for future use. */
	STEP_MES_TYPE_LAST              = 0xFF    /**< Reserved */
};

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* STEP_MEASUREMENT_BASE_H__ */
