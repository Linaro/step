/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef SDP_DATASAMPLE_EXT_TEMP_H__
#define SDP_DATASAMPLE_EXT_TEMP_H__

#include <sdp/sdp.h>

/**
 * @file
 * @brief SDP_DS_TYPE_TEMPERATURE extended type definitions
 * @ingroup DATASAMPLE
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/** Extended data types for SDP_DS_TYPE_TEMPERATURE (8-bit). */
enum sdp_ds_ext_temperature {
	SDP_DS_EXT_TYPE_TEMP_UNDEFINED  = 0,    /**< Undefined temperature */
	SDP_DS_EXT_TYPE_TEMP_AMBIENT    = 1,    /**< Ambient temperature */
	SDP_DS_EXT_TYPE_TEMP_DIE        = 2,    /**< Die temperature */
	SDP_DS_EXT_TYPE_TEMP_OBJECT     = 3,    /**< Object temperature */
};

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* SDP_DATASAMPLE_EXT_TEMP_H__ */
