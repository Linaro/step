/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef STEP_MEASUREMENT_EXT_LIGHT_H__
#define STEP_MEASUREMENT_EXT_LIGHT_H__

#include <step/step.h>

/**
 * @brief STEP_MES_TYPE_LIGHT extended type definitions
 * @ingroup MEASUREMENT
 * @{
 */

/**
 * @file
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Extended measurement types for STEP_MES_TYPE_LIGHT (8-bit).
 *
 * <b>Radiometric Units (STEP_MES_EXT_TYPE_LIGHT_RADIO_*)</b>
 *
 * Electromagentic radiation is characterised by radiometric units, which
 * describe the physical properties of light (the number of photons, photon
 * energy, or radiant flux). These units have no relation to human vision.
 * Infrared radiation, for example, is not visible to the human eye (>780 nm)
 * but it clearly exists as a radiometric phenomenon and can be accurately
 * measured, described and analysed for scientifically significant purposes.
 *
 * <b>Photometric Units (STEP_MES_EXT_TYPE_LIGHT_PHOTO_*)</b>
 *
 * To characterise light relative to the human eye, we need to use
 * photometric units such as luminous intensity, which represents the
 * light intensity of a source as perceived by the human eye, measured in
 * candela (cd).
 *
 * Since photometric measurements are limited to what the human eye can
 * perceive, these measurements are restricted to the visible spectrum
 * of 380 nm to 780 nm wavelengths.
 *
 * One of the most common photometric units is luminous flux, which is
 * measured in lumens (lm). It's even more common derivative is illuminance,
 * which is the luminous flux incident per a specific area. Illuminance is
 * measured in lux (which is equal to lm/m^2).
 */
enum step_mes_ext_light {
	STEP_MES_EXT_TYPE_LIGHT_UNDEFINED                = 0,    /**< lux */

	/* Radiometric units */
	STEP_MES_EXT_TYPE_LIGHT_RADIO_RADIANT_FLUX       = 0x10, /**< W */
	STEP_MES_EXT_TYPE_LIGHT_RADIO_RADIANT_INTEN      = 0x11, /**< W/sr */
	STEP_MES_EXT_TYPE_LIGHT_RADIO_IRRADIANCE         = 0x12, /**< W/m^2 */
	STEP_MES_EXT_TYPE_LIGHT_RADIO_RADIANCE           = 0x13, /**< W/(sr m^2) */

	/* Photometric units */
	STEP_MES_EXT_TYPE_LIGHT_PHOTO_LUM_FLUX           = 0x20, /**< lm */
	STEP_MES_EXT_TYPE_LIGHT_PHOTO_LUM_INTEN          = 0x21, /**< lm/sr or cd */
	STEP_MES_EXT_TYPE_LIGHT_PHOTO_ILLUMINANCE        = 0x22, /**< lm/m^2 or lux */
	STEP_MES_EXT_TYPE_LIGHT_PHOTO_LUMINANCE          = 0x23, /**< cd/m^2 */
};

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* STEP_MEASUREMENT_EXT_LIGHT_H__ */
