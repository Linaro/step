/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <sys/printk.h>

void main(void)
{
	printk("Type 'sdp help' for command options.\n\n");
	printk("1.) Populate the processor registry via `sdp pop`\n");
	printk("2.) Publish measurements via 'sdp push'\n");
	printk("3.) Check results via 'sdp stats'");

	while (1) {
		k_sleep(K_FOREVER);
	}
}
