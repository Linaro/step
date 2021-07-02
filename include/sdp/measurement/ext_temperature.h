/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef SDP_MEASUREMENT_EXT_TEMP_H__
#define SDP_MEASUREMENT_EXT_TEMP_H__

#include <sdp/sdp.h>

/**
 * @file
 * @brief SDP_MES_TYPE_TEMPERATURE extended type definitions
 * @ingroup MEASUREMENT
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/** Extended measurement types for SDP_MES_TYPE_TEMPERATURE (8-bit). */
enum sdp_mes_ext_temperature {
	SDP_MES_EXT_TYPE_TEMP_UNDEFINED  = 0,    /**< Undefined temperature */
	SDP_MES_EXT_TYPE_TEMP_AMBIENT    = 1,    /**< Ambient temperature */
	SDP_MES_EXT_TYPE_TEMP_DIE        = 2,    /**< Die temperature */
	SDP_MES_EXT_TYPE_TEMP_OBJECT     = 3,    /**< Object temperature */
};

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* SDP_MEASUREMENT_EXT_TEMP_H__ */
