/*
 * Copyright (c) 2022 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef PLATFORM_H__
#define PLATFORM_H__

#if defined(__ZEPHYR__)
#include <stdbool.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/slist.h>

struct step_platform_queue {
    struct k_work work;
    bool free_after_use;
};

#else
#error "Platform not supported yet!"
#endif

#endif
