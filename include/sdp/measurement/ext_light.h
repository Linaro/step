/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef SDP_MEASUREMENT_EXT_LIGHT_H__
#define SDP_MEASUREMENT_EXT_LIGHT_H__

#include <sdp/sdp.h>

/**
 * @file
 * @brief SDP_MES_TYPE_LIGHT extended type definitions
 * @ingroup MEASUREMENT
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Extended measurement types for SDP_MES_TYPE_LIGHT (8-bit).
 *
 * <b>Radiometric Units (SDP_MES_EXT_TYPE_LIGHT_RADIO_*)</b>
 *
 * Electromagentic radiation is characterised by radiometric units, which
 * describe the physical properties of light (the number of photons, photon
 * energy, or radiant flux). These units have no relation to human vision.
 * Infrared radiation, for example, is not visible to the human eye (>780 nm)
 * but it clearly exists as a radiometric phenomenon and can be accurately
 * measured, described and analysed for scientifically significant purposes.
 *
 * <b>Photometric Units (SDP_MES_EXT_TYPE_LIGHT_PHOTO_*)</b>
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
enum sdp_mes_ext_light {
	SDP_MES_EXT_TYPE_LIGHT_UNDEFINED                = 0,    /**< lux */

	/* Radiometric units */
	SDP_MES_EXT_TYPE_LIGHT_RADIO_RADIANT_FLUX       = 0x10, /**< W */
	SDP_MES_EXT_TYPE_LIGHT_RADIO_RADIANT_INTEN      = 0x11, /**< W/sr */
	SDP_MES_EXT_TYPE_LIGHT_RADIO_IRRADIANCE         = 0x12, /**< W/m^2 */
	SDP_MES_EXT_TYPE_LIGHT_RADIO_RADIANCE           = 0x13, /**< W/(sr m^2) */

	/* Photometric units */
	SDP_MES_EXT_TYPE_LIGHT_PHOTO_LUM_FLUX           = 0x20, /**< lm */
	SDP_MES_EXT_TYPE_LIGHT_PHOTO_LUM_INTEN          = 0x21, /**< lm/sr or cd */
	SDP_MES_EXT_TYPE_LIGHT_PHOTO_ILLUMINANCE        = 0x22, /**< lm/m^2 or lux */
	SDP_MES_EXT_TYPE_LIGHT_PHOTO_LUMINANCE          = 0x23, /**< cd/m^2 */
};

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* SDP_MEASUREMENT_EXT_LIGHT_H__ */
