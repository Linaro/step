/*
 * Copyright (c) 2022 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "zsl_fusion_config.h"

static zsl_real_t _mahn_intfb[3] = { 0.0, 0.0, 0.0 };
struct zsl_fus_mahn_cfg mahn_cfg = {
	.kp = 200,
	.ki = 0.00200,
	.intfb = {
		.sz = 3,
		.data = _mahn_intfb,
	},
};

const struct zsl_fus_drv fusion_driver = {
	.init_handler = zsl_fus_mahn_init,
	.feed_handler = zsl_fus_mahn_feed,
	.error_handler = zsl_fus_mahn_error,
	.config = &mahn_cfg,
};
