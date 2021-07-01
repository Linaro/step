/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef SDP_DATASAMPLE_UNIT_H__
#define SDP_DATASAMPLE_UNIT_H__

#include <sdp/sdp.h>

/**
 * @file
 * @brief SI unit type, ctype and scale definitions
 * @ingroup DATASAMPLE
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief C type used to represent a unit in memory (8-bits).
 *
 * @note All types are little-endian.
 *
 * Memory map:
 *   - 0 = Undefined
 *   - 0x10..0x4F = Standard C types
 *   - 0x50..0x7F = Reserved
 *   - 0x80..0x8F = Range types (unit interval, percent, etc.)
 *   - 0xA0..0xEF = Reserved
 *   - 0xF0..0xFE = User-defined types
 */
enum sdp_ds_unit_ctype {
	SDP_DS_UNIT_CTYPE_UNDEFINED             = 0x00,

	/* 0x10..0x4F: Standard C types. */
	SDP_DS_UNIT_CTYPE_IEEE754_FLOAT32       = 0x10,
	SDP_DS_UNIT_CTYPE_IEEE754_FLOAT64       = 0x11,
	SDP_DS_UNIT_CTYPE_IEEE754_FLOAT128      = 0x12,
	SDP_DS_UNIT_CTYPE_S8                    = 0x13,
	SDP_DS_UNIT_CTYPE_S16                   = 0x14,
	SDP_DS_UNIT_CTYPE_S32                   = 0x15,
	SDP_DS_UNIT_CTYPE_S64                   = 0x16,
	SDP_DS_UNIT_CTYPE_S128                  = 0x17,
	SDP_DS_UNIT_CTYPE_U8                    = 0x18,
	SDP_DS_UNIT_CTYPE_U16                   = 0x19,
	SDP_DS_UNIT_CTYPE_U32                   = 0x1A,
	SDP_DS_UNIT_CTYPE_U64                   = 0x1B,
	SDP_DS_UNIT_CTYPE_U128                  = 0x1C,
	SDP_DS_UNIT_CTYPE_BOOL                  = 0x1D, /**< SDP_DS_UNIT_CTYPE_U8 */
	SDP_DS_UNIT_CTYPE_COMPLEX_32            = 0x30,
	SDP_DS_UNIT_CTYPE_COMPLEX_64            = 0x31,

	/* 0x80..0x8F: Range types (unit interval, percent, etc.) */
	/** @brief 0..1.0 inclusive, SDP_DS_UNIT_CTYPE_FLOAT32. */
	SDP_DS_UNIT_CTYPE_RANG_UNIT_INTERVAL_32 = 0x80,
	/** @brief 0..1.0 inclusive, SDP_DS_UNIT_CTYPE_FLOAT64. */
	SDP_DS_UNIT_CTYPE_RANG_UNIT_INTERVAL_64 = 0x81,
	/** @brief 0.0..100.0 inclusive, SDP_DS_UNIT_CTYPE_FLOAT32. */
	SDP_DS_UNIT_CTYPE_RANG_PERCENT_32       = 0x82,
	/** @brief 0.0..100.0 inclusive, SDP_DS_UNIT_CTYPE_FLOAT64. */
	SDP_DS_UNIT_CTYPE_RANG_PERCENT_64       = 0x83,

	/* 0xF0..0xFE: User-defined types. */
	SDP_DS_UNIT_CTYPE_USER_1                = 0xF0,
	SDP_DS_UNIT_CTYPE_USER_2                = 0xF1,
	SDP_DS_UNIT_CTYPE_USER_3                = 0xF2,
	SDP_DS_UNIT_CTYPE_USER_4                = 0xF3,
	SDP_DS_UNIT_CTYPE_USER_5                = 0xF4,
	SDP_DS_UNIT_CTYPE_USER_6                = 0xF5,
	SDP_DS_UNIT_CTYPE_USER_7                = 0xF6,
	SDP_DS_UNIT_CTYPE_USER_8                = 0xF7,
	SDP_DS_UNIT_CTYPE_USER_9                = 0xF8,
	SDP_DS_UNIT_CTYPE_USER_10               = 0xF9,
	SDP_DS_UNIT_CTYPE_USER_11               = 0xFA,
	SDP_DS_UNIT_CTYPE_USER_12               = 0xFB,
	SDP_DS_UNIT_CTYPE_USER_13               = 0xFC,
	SDP_DS_UNIT_CTYPE_USER_14               = 0xFD,
	SDP_DS_UNIT_CTYPE_USER_15               = 0xFE,

	SDP_DS_UNIT_CTYPE_MAX                   = 0xFF
};

/**
 * @brief Standard SI units (16-bits).
 *
 * @note Base and derived SI units can be represented in 8-bits, while
 * combined units require 16-bits to represent.
 *
 * Memory map:
 *   - 0              = Undefined unit
 *   - 0x0001..0x000F = Reserved
 *   - 0x0010..0x001F = SI base units
 *   - 0x0020..0x003F = SI derived units
 *   - 0x0040..0x007F = Reserved
 *   - 0x0080..0x008F = Unitless values (percent, interval, etc.)
 *   - 0x0090..0x00FF = Reserved
 *   - 0x0100..0x7FFF = SI Combined units
 *   - 0x8000..0xFEFF = Reserved
 *   - 0xFF00..0xFFFE = User defined units
 */
enum zsl_ds_unit_si {
	SDP_DS_UNIT_SI_UNDEFINED                = 0,

	/* 0x0010..0x001F (16-31): SI Base Units. */
	/** @brief A, electric current */
	SDP_DS_UNIT_SI_AMPERE                   = 0x10,
	/** @brief cd, Luminous intensity */
	SDP_DS_UNIT_SI_CANDELA                  = 0x11,
	/** @brief K, thermodynamic temp. */
	SDP_DS_UNIT_SI_KELVIN                   = 0x12,
	/** @brief kg, mass */
	SDP_DS_UNIT_SI_KILOGRAM                 = 0x13,
	/** @brief m, length */
	SDP_DS_UNIT_SI_METER                    = 0x14,
	/** @brief mol, Amount of substance */
	SDP_DS_UNIT_SI_MOLE                     = 0x15,
	/** @brief s, time */
	SDP_DS_UNIT_SI_SECOND                   = 0x16,

	/* 0x0020..0x003F (32-63): SI Derived Units. */
	/** @brief Bq, radionucl. activity, 1/s */
	SDP_DS_UNIT_SI_BECQUEREL                = 0x20,
	/** @brief C, electric charge, A*s */
	SDP_DS_UNIT_SI_COULOMB                  = 0x21,
	/** @brief Degrees C, Celsius temp, K */
	SDP_DS_UNIT_SI_DEGREE_CELSIUS           = 0x22,
	/** @brief F, elec. capaticance, C/V or s^4*A^2/m^2*kg */
	SDP_DS_UNIT_SI_FARAD                    = 0x23,
	/** @brief Gy, Absorbed dose, J/kg or m^2/s^2 */
	SDP_DS_UNIT_SI_GRAY                     = 0x24,
	/** @brief H, electric inductance, Wb/A or kg*m^2/s^2*A^2 */
	SDP_DS_UNIT_SI_HENRY                    = 0x25,
	/** @brief Hz, frequency, 1/s */
	SDP_DS_UNIT_SI_HERTZ                    = 0x26,
	/** @brief J, energy, work, N*m or kg*m^2/s^2 */
	SDP_DS_UNIT_SI_JOULE                    = 0x27,
	/** @brief kat, Catamytic activ., mol/s */
	SDP_DS_UNIT_SI_KATAL                    = 0x28,
	/** @brief lm, Luminous flux, cd*sr */
	SDP_DS_UNIT_SI_LUMEN                    = 0x29,
	/** @brief lx, illuminance, lm/m^2 */
	SDP_DS_UNIT_SI_LUX                      = 0x2A,
	/** @brief N, force, m*kg/s^2 */
	SDP_DS_UNIT_SI_NEWTON                   = 0x2B,
	/** @brief electric resistence, V/A or kg*m^2/s^3*A^2*/
	SDP_DS_UNIT_SI_OHM                      = 0x2C,
	/** @brief Pa, pressure, stress, N/m^2 or kg/m*s^2 */
	SDP_DS_UNIT_SI_PASCAL                   = 0x2D,
	/** @brief rad, plane angle, m/m */
	SDP_DS_UNIT_SI_RADIAN                   = 0x2E,
	/** @brief S, electric conductance, A/V */
	SDP_DS_UNIT_SI_SIEMENS                  = 0x2F,
	/** @brief Sv, Dose equivalent, J/kg or m^2/s^2 */
	SDP_DS_UNIT_SI_SIEVERT                  = 0x30,
	/** @brief sr, solid angle, m^2/m^2 */
	SDP_DS_UNIT_SI_STERADIAN                = 0x31,
	/** @brief T, magn. flux dens., Wb/m^2 or kg/A*s^2*/
	SDP_DS_UNIT_SI_TESLA                    = 0x32,
	/** @brief V, elec. poten. diff, W/A or kg*m^2/A*s^3 */
	SDP_DS_UNIT_SI_VOLT                     = 0x33,
	/** @brief W, power, radiant flux, J/s or kg*m^2/s^3 */
	SDP_DS_UNIT_SI_WATT                     = 0x34,
	/** @brief Wb, magnetic flux, V*s or kg*m^2/s^2*A */
	SDP_DS_UNIT_SI_WEBER                    = 0x35,

	/* 0x0080..0x008F (128-143): 'Unitless' Units. */
	/** @brief %, 0.0 .. 100.0 inclusive. */
	SDP_DS_UNIT_SI_PERCENT                  = 0x80,
	/** @brief 0.0 .. 1.0 inclusive. */
	SDP_DS_UNIT_SI_INTERVAL                 = 0x81,

	/* 0x0100..0x7FFF: Combined Units. */
	/** @brief m^2 */
	SDP_DS_UNIT_SI_METERS_2                 = 0x1000,
	/** @brief m/s^2 */
	SDP_DS_UNIT_SI_METER_PER_SECOND_2       = 0x1100,

	/* 0xFF00..0xFFFE = User defined units. */
	SDP_DS_UNIT_SI_USER_DEFINED_1           = 0xFF00,
	SDP_DS_UNIT_SI_USER_DEFINED_255         = 0xFFFE,

	SDP_DS_UNIT_SI_MAX                      = 0xFFFF
};

/**
 * @brief Standard SI scales/powers.
 */
enum sdp_ds_si_scale {
	SDP_DS_SI_SCALE_YOTTA   = 24,           /**< Y: Septillion */
	SDP_DS_SI_SCALE_ZETTA   = 21,           /**< Z: Sextillion */
	SDP_DS_SI_SCALE_EXA     = 18,           /**< E: Quintillion */
	SDP_DS_SI_SCALE_PETA    = 15,           /**< P: Quadrillion */
	SDP_DS_SI_SCALE_TERA    = 12,           /**< T: Trillion */
	SDP_DS_SI_SCALE_GIGA    = 9,            /**< G: Billion */
	SDP_DS_SI_SCALE_MEGA    = 6,            /**< M: Million */
	SDP_DS_SI_SCALE_KILO    = 3,            /**< k: Thousand */
	SDP_DS_SI_SCALE_HECTO   = 2,            /**< h: Hundred */
	SDP_DS_SI_SCALE_DECA    = 1,            /**< da: Ten*/
	SDP_DS_SI_SCALE_NONE    = 0,            /**< One */
	SDP_DS_SI_SCALE_DECI    = -1,           /**< d: Tenth */
	SDP_DS_SI_SCALE_CENTI   = -2,           /**< c: Hundredth */
	SDP_DS_SI_SCALE_MILLI   = -3,           /**< m: Thousandth */
	SDP_DS_SI_SCALE_MICRO   = -6,           /**< u: Millionth */
	SDP_DS_SI_SCALE_NANO    = -9,           /**< n: Billionth */
	SDP_DS_SI_SCALE_PICO    = -12,          /**< p: Trillionth */
	SDP_DS_SI_SCALE_FEMTO   = -15,          /**< f: Quadrillionth */
	SDP_DS_SI_SCALE_ATTO    = -18,          /**< a: Quintillionth */
	SDP_DS_SI_SCALE_ZEPTO   = -21,          /**< z: Sextillionth */
	SDP_DS_SI_SCALE_YOCTO   = -24,          /**< y: Septillionth */
};

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* SDP_DATASAMPLE_UNIT_H__ */
