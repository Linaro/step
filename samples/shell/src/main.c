/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <sys/printk.h>

void main(void)
{
	printk("Type 'sdp help' for command options.\n");

	while (1) {
		k_sleep(K_FOREVER);
	}
}
