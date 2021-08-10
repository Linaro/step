/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef STEP_MEASUREMENT_UNIT_H__
#define STEP_MEASUREMENT_UNIT_H__

#include <step/step.h>

/**
 * @file
 * @brief SI unit type, ctype and scale definitions
 * @ingroup MEASUREMENT
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief C type used to represent a measurement in memory (8-bits).
 *
 * @note All types are little-endian.
 *
 * Memory map:
 *   - 0 = Undefined
 *   - 0x10..0x4F = Standard C types
 *   - 0x50..0x7F = Reserved
 *   - 0x80..0x8F = Range types (unit interval, percent, etc.)
 *   - 0x90..0xEF = Reserved
 *   - 0xF0..0xFE = User-defined types
 */
enum step_mes_unit_ctype {
	STEP_MES_UNIT_CTYPE_UNDEFINED                   = 0x00,

	/* 0x10..0x4F: Standard C types. */
	STEP_MES_UNIT_CTYPE_IEEE754_FLOAT32             = 0x10,
	STEP_MES_UNIT_CTYPE_IEEE754_FLOAT64             = 0x11,
	STEP_MES_UNIT_CTYPE_IEEE754_FLOAT128            = 0x12,
	STEP_MES_UNIT_CTYPE_S8                          = 0x13,
	STEP_MES_UNIT_CTYPE_S16                         = 0x14,
	STEP_MES_UNIT_CTYPE_S32                         = 0x15,
	STEP_MES_UNIT_CTYPE_S64                         = 0x16,
	STEP_MES_UNIT_CTYPE_S128                        = 0x17,
	STEP_MES_UNIT_CTYPE_U8                          = 0x18,
	STEP_MES_UNIT_CTYPE_U16                         = 0x19,
	STEP_MES_UNIT_CTYPE_U32                         = 0x1A,
	STEP_MES_UNIT_CTYPE_U64                         = 0x1B,
	STEP_MES_UNIT_CTYPE_U128                        = 0x1C,
	STEP_MES_UNIT_CTYPE_BOOL                        = 0x1D,
	STEP_MES_UNIT_CTYPE_COMPLEX_32                  = 0x30,
	STEP_MES_UNIT_CTYPE_COMPLEX_64                  = 0x31,

	/* 0x80..0x8F: Range types (unit interval, percent, etc.) */
	/** @brief 0..1.0 inclusive, STEP_MES_UNIT_CTYPE_FLOAT32. */
	STEP_MES_UNIT_CTYPE_RANG_UNIT_INTERVAL_32       = 0x80,
	/** @brief 0..1.0 inclusive, STEP_MES_UNIT_CTYPE_FLOAT64. */
	STEP_MES_UNIT_CTYPE_RANG_UNIT_INTERVAL_64       = 0x81,
	/** @brief 0.0..100.0 inclusive, STEP_MES_UNIT_CTYPE_FLOAT32. */
	STEP_MES_UNIT_CTYPE_RANG_PERCENT_32             = 0x82,
	/** @brief 0.0..100.0 inclusive, STEP_MES_UNIT_CTYPE_FLOAT64. */
	STEP_MES_UNIT_CTYPE_RANG_PERCENT_64             = 0x83,

	/* 0xF0..0xFE: User-defined types. */
	STEP_MES_UNIT_CTYPE_USER_1                      = 0xF0,
	STEP_MES_UNIT_CTYPE_USER_2                      = 0xF1,
	STEP_MES_UNIT_CTYPE_USER_3                      = 0xF2,
	STEP_MES_UNIT_CTYPE_USER_4                      = 0xF3,
	STEP_MES_UNIT_CTYPE_USER_5                      = 0xF4,
	STEP_MES_UNIT_CTYPE_USER_6                      = 0xF5,
	STEP_MES_UNIT_CTYPE_USER_7                      = 0xF6,
	STEP_MES_UNIT_CTYPE_USER_8                      = 0xF7,
	STEP_MES_UNIT_CTYPE_USER_9                      = 0xF8,
	STEP_MES_UNIT_CTYPE_USER_10                     = 0xF9,
	STEP_MES_UNIT_CTYPE_USER_11                     = 0xFA,
	STEP_MES_UNIT_CTYPE_USER_12                     = 0xFB,
	STEP_MES_UNIT_CTYPE_USER_13                     = 0xFC,
	STEP_MES_UNIT_CTYPE_USER_14                     = 0xFD,
	STEP_MES_UNIT_CTYPE_USER_15                     = 0xFE,

	STEP_MES_UNIT_CTYPE_MAX                         = 0xFF
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
 *   - 0x0100..0x0FFF = SI combined units (generic)
 *   - 0x1000..0w7FFF = Base type specific units, in groups of 0xFF.
 *   - 0x8000..0xFEFF = Reserved
 *   - 0xFF00..0xFFFE = User defined units
 */
enum step_mes_unit_si {
	STEP_MES_UNIT_SI_UNDEFINED              = 0,

	/* 0x0010..0x001F (16-31): SI Base Units. */
	/** @brief A, electric current */
	STEP_MES_UNIT_SI_AMPERE                 = 0x10,
	/** @brief cd, Luminous intensity */
	STEP_MES_UNIT_SI_CANDELA                = 0x11,
	/** @brief K, thermodynamic temp. */
	STEP_MES_UNIT_SI_KELVIN                 = 0x12,
	/** @brief kg, mass */
	STEP_MES_UNIT_SI_KILOGRAM               = 0x13,
	/** @brief m, length */
	STEP_MES_UNIT_SI_METER                  = 0x14,
	/** @brief mol, Amount of substance */
	STEP_MES_UNIT_SI_MOLE                   = 0x15,
	/** @brief s, time */
	STEP_MES_UNIT_SI_SECOND                 = 0x16,

	/* 0x0020..0x003F (32-63): SI Derived Units. */
	/** @brief Bq, radionucl. activity, 1/s */
	STEP_MES_UNIT_SI_BECQUEREL              = 0x20,
	/** @brief C, electric charge, A*s */
	STEP_MES_UNIT_SI_COULOMB                = 0x21,
	/** @brief Degrees C, Celsius temp, K */
	STEP_MES_UNIT_SI_DEGREE_CELSIUS         = 0x22,
	/** @brief F, elec. capaticance, C/V or s^4*A^2/m^2*kg */
	STEP_MES_UNIT_SI_FARAD                  = 0x23,
	/** @brief Gy, Absorbed dose, J/kg or m^2/s^2 */
	STEP_MES_UNIT_SI_GRAY                   = 0x24,
	/** @brief H, electric inductance, Wb/A or kg*m^2/s^2*A^2 */
	STEP_MES_UNIT_SI_HENRY                  = 0x25,
	/** @brief Hz, frequency, 1/s */
	STEP_MES_UNIT_SI_HERTZ                  = 0x26,
	/** @brief J, energy, work, N*m or kg*m^2/s^2 */
	STEP_MES_UNIT_SI_JOULE                  = 0x27,
	/** @brief kat, Catamytic activ., mol/s */
	STEP_MES_UNIT_SI_KATAL                  = 0x28,
	/** @brief lm, Luminous flux, cd*sr */
	STEP_MES_UNIT_SI_LUMEN                  = 0x29,
	/** @brief lx, illuminance, lm/m^2 */
	STEP_MES_UNIT_SI_LUX                    = 0x2A,
	/** @brief N, force, m*kg/s^2 */
	STEP_MES_UNIT_SI_NEWTON                 = 0x2B,
	/** @brief electric resistence, V/A or kg*m^2/s^3*A^2*/
	STEP_MES_UNIT_SI_OHM                    = 0x2C,
	/** @brief Pa, pressure, stress, N/m^2 or kg/m*s^2 */
	STEP_MES_UNIT_SI_PASCAL                 = 0x2D,
	/** @brief rad, plane angle, m/m */
	STEP_MES_UNIT_SI_RADIAN                 = 0x2E,
	/** @brief S, electric conductance, A/V */
	STEP_MES_UNIT_SI_SIEMENS                = 0x2F,
	/** @brief Sv, Dose equivalent, J/kg or m^2/s^2 */
	STEP_MES_UNIT_SI_SIEVERT                = 0x30,
	/** @brief sr, solid angle, m^2/m^2 */
	STEP_MES_UNIT_SI_STERADIAN              = 0x31,
	/** @brief T, magn. flux dens., Wb/m^2 or kg/A*s^2*/
	STEP_MES_UNIT_SI_TESLA                  = 0x32,
	/** @brief V, elec. poten. diff, W/A or kg*m^2/A*s^3 */
	STEP_MES_UNIT_SI_VOLT                   = 0x33,
	/** @brief W, power, radiant flux, J/s or kg*m^2/s^3 */
	STEP_MES_UNIT_SI_WATT                   = 0x34,
	/** @brief Wb, magnetic flux, V*s or kg*m^2/s^2*A */
	STEP_MES_UNIT_SI_WEBER                  = 0x35,

	/* 0x0080..0x008F (128-143): 'Unitless' Units. */
	/** @brief %, 0.0 .. 100.0 inclusive. */
	STEP_MES_UNIT_SI_PERCENT                = 0x80,
	/** @brief 0.0 .. 1.0 inclusive. */
	STEP_MES_UNIT_SI_INTERVAL               = 0x81,

	/* 0x1000..0x10FF: STEP_MES_TYPE_AREA Combined Units. */
	/** @brief m^2 */
	STEP_MES_UNIT_SI_METERS_2               = 0x1000,

	/* 0x1100..0x11FF: STEP_MES_TYPE_ACCELERATION Combined Units. */
	/** @brief m/s^2 */
	STEP_MES_UNIT_SI_METER_PER_SECOND_2     = 0x1100,

	/* 0x1200..0x12FF: STEP_MES_TYPE_AMPLITUDE Combined Units. */

	/* 0x1300..0x13FF: STEP_MES_TYPE_CAPACITANCE Combined Units. */

	/* 0x1400..0x14FF: STEP_MES_TYPE_COLOR Combined Units. */

	/* 0x1500..0x15FF: STEP_MES_TYPE_COORDINATES Combined Units. */

	/* 0x1600..0x16FF: STEP_MES_TYPE_CURRENT Combined Units. */

	/* 0x1700..0x17FF: STEP_MES_TYPE_DIMENSION Combined Units. */

	/* 0x1800..0x18FF: STEP_MES_TYPE_FREQUENCYY Combined Units. */

	/* 0x1900..0x19FF: STEP_MES_TYPE_HUMIDITY Combined Units. */
	/** @brief Percent */
	STEP_MES_UNIT_SI_RELATIVE_HUMIDITY = 0x1900,

	/* 0x1A00..0x1AFF: STEP_MES_TYPE_INDUCTANCE Combined Units. */

	/* 0x1B00..0x1BFF: STEP_MES_TYPE_LIGHT Combined Units. */
	/** @brief cd/m^2 */
	STEP_MES_UNIT_SI_CANDELA_PER_METER_2                    = 0x1B00,
	/** @brief J/m^2 */
	STEP_MES_UNIT_SI_JOULE_PER_METER_2                      = 0x1B01,
	/** @brief J/m^2/Hz */
	STEP_MES_UNIT_SI_JOULE_PER_METER_2_PER_HZ               = 0x1B02,
	/** @brief J/m^2/nm */
	STEP_MES_UNIT_SI_JOULE_PER_METER_2_PER_NM               = 0x1B03,
	/** @brief Energy density, J/m^3 */
	STEP_MES_UNIT_SI_JOULE_PER_METER_3                      = 0x1B04,
	/** @brief lm/m^2 */
	STEP_MES_UNIT_SI_LUMEN_PER_METER_2                      = 0x1B05,
	/** @brief lm/W */
	STEP_MES_UNIT_SI_LUMEN_PER_WATT                         = 0x1B06,
	/** @brief lm s, AKA talbot */
	STEP_MES_UNIT_SI_LUMEN_SECOND                           = 0x1B07,
	/** @brief lm s/m^3 */
	STEP_MES_UNIT_SI_LUMEN_SECOND_PER_METER_3               = 0x1B08,
	/** @brief lx s */
	STEP_MES_UNIT_SI_LUX_SECOND                             = 0x1B09,
	/** @brief W/Hz */
	STEP_MES_UNIT_SI_WATTS_PER_HERTZ                        = 0x1B0A,
	/** @brief W/m^2 */
	STEP_MES_UNIT_SI_WATTS_PER_METER_2                      = 0x1B0B,
	/** @brief W/m^2/Hz */
	STEP_MES_UNIT_SI_WATTS_PER_METER_2_PER_HZ               = 0x1B0C,
	/** @brief W/m^2/nm */
	STEP_MES_UNIT_SI_WATTS_PER_METER_2_PER_NM               = 0x1B0D,
	/** @brief W/nm */
	STEP_MES_UNIT_SI_WATTS_PER_NM                           = 0x1B0E,
	/** @brief W/sr */
	STEP_MES_UNIT_SI_WATTS_PER_STERADIAN                    = 0x1B0F,
	/** @brief W/sr/Hz */
	STEP_MES_UNIT_SI_WATTS_PER_STERADIAN_PER_HERTZ          = 0x1B10,
	/** @brief W/sr/m^2 */
	STEP_MES_UNIT_SI_WATTS_PER_STERADIAN_PER_METER_2        = 0x1B11,
	/** @brief W/sr/m^2/Hz */
	STEP_MES_UNIT_SI_WATTS_PER_STERADIAN_PER_METER_2_PER_HZ = 0x1B12,
	/** @brief W/sr/m^2/nm */
	STEP_MES_UNIT_SI_WATTS_PER_STERADIAN_PER_METER_2_PER_NM = 0x1B13,
	/** @brief W/sr/nm */
	STEP_MES_UNIT_SI_WATTS_PER_STERADIAN_PER_NM             = 0x1B14,

	/* 0x1C00..0x1CFF: STEP_MES_TYPE_MAGNETIC_FIELD Combined Units. */
	/** @brief uT */
	STEP_MES_UNIT_SI_MICROTESLA                             = 0x1C00,

	/* 0x1D00..0x1DFF: STEP_MES_TYPE_MASS Combined Units. */
	/** @brief g */
	STEP_MES_UNIT_SI_GRAMS                                  = 0x1D00,

	/* 0x1E00..0x1EFF: STEP_MES_TYPE_MOMENTUM Combined Units. */

	/* 0x1F00..0x1FFF: STEP_MES_TYPE_ORIENTATION Combined Units. */

	/* 0x2000..0x20FF: STEP_MES_TYPE_PHASE Combined Units. */

	/* 0x2100..0x21FF: STEP_MES_TYPE_PRESSURE Combined Units. */
	/** @brief hPA */
	STEP_MES_UNIT_SI_HECTOPASCAL = 0x2100,

	/* 0x2200..0x22FF: STEP_MES_TYPE_RESISTANCE Combined Units. */

	/* 0x2300..0x23FF: STEP_MES_TYPE_SOUND Combined Units. */

	/* 0x2400..0x24FF: STEP_MES_TYPE_TEMPERATURE Combined Units. */

	/* 0x2500..0x25FF: STEP_MES_TYPE_TIME Combined Units. */

	/* 0x2600..0x26FF: STEP_MES_TYPE_VELOCITY Combined Units. */
	/** @brief m^3/s, flow rate. */
	STEP_MES_UNIT_SI_METERS_3_SECOND        = 0x2601,

	/* 0x2700..0x27FF: STEP_MES_TYPE_VOLTAGE Combined Units. */
	/** @brief mV */
	STEP_MES_UNIT_SI_MILLIVOLTS             = 0x2700,

	/* 0x2800..0x28FF: STEP_MES_TYPE_VOLUME Combined Units. */
	/** @brief m^3 */
	STEP_MES_UNIT_SI_METERS_3               = 0x2800,

	/* 0x2900..0x29FF: STEP_MES_TYPE_ACIDITY Combined Units. */
	/** @brief pH level (not actually a unit, shhh!) */
	STEP_MES_UNIT_SI_PH                     = 0x2900,

	/* 0x2A00..0x2AFF: STEP_MES_TYPE_CONDUCTIVITY Combined Units. */
	/** @brief S/m */
	STEP_MES_UNIT_SI_SIEMENS_PER_METER      = 0x2A00,

	/* 0x2B00..0x2BFF: STEP_MES_TYPE_FORCE Combined Units. */

	/* 0x2C00..0x2CFF: STEP_MES_TYPE_ENERGY Combined Units. */

	/* 0xFF00..0xFFFE = User defined units. */
	STEP_MES_UNIT_SI_USER_DEFINED_1         = 0xFF00,
	STEP_MES_UNIT_SI_USER_DEFINED_255       = 0xFFFE,

	STEP_MES_UNIT_SI_MAX                    = 0xFFFF
};

/**
 * @brief Standard SI scales/powers.
 */
enum step_mes_si_scale {
	STEP_MES_SI_SCALE_YOTTA = 24,   /**< Y: Septillion */
	STEP_MES_SI_SCALE_ZETTA = 21,   /**< Z: Sextillion */
	STEP_MES_SI_SCALE_EXA   = 18,   /**< E: Quintillion */
	STEP_MES_SI_SCALE_PETA  = 15,   /**< P: Quadrillion */
	STEP_MES_SI_SCALE_TERA  = 12,   /**< T: Trillion */
	STEP_MES_SI_SCALE_GIGA  = 9,    /**< G: Billion */
	STEP_MES_SI_SCALE_MEGA  = 6,    /**< M: Million */
	STEP_MES_SI_SCALE_KILO  = 3,    /**< k: Thousand */
	STEP_MES_SI_SCALE_HECTO = 2,    /**< h: Hundred */
	STEP_MES_SI_SCALE_DECA  = 1,    /**< da: Ten*/
	STEP_MES_SI_SCALE_NONE  = 0,    /**< One */
	STEP_MES_SI_SCALE_DECI  = -1,   /**< d: Tenth */
	STEP_MES_SI_SCALE_CENTI = -2,   /**< c: Hundredth */
	STEP_MES_SI_SCALE_MILLI = -3,   /**< m: Thousandth */
	STEP_MES_SI_SCALE_MICRO = -6,   /**< u: Millionth */
	STEP_MES_SI_SCALE_NANO  = -9,   /**< n: Billionth */
	STEP_MES_SI_SCALE_PICO  = -12,  /**< p: Trillionth */
	STEP_MES_SI_SCALE_FEMTO = -15,  /**< f: Quadrillionth */
	STEP_MES_SI_SCALE_ATTO  = -18,  /**< a: Quintillionth */
	STEP_MES_SI_SCALE_ZEPTO = -21,  /**< z: Sextillionth */
	STEP_MES_SI_SCALE_YOCTO = -24,  /**< y: Septillionth */
};

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* STEP_MEASUREMENT_UNIT_H__ */
