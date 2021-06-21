/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <sys/printk.h>
#include <sdp/sample_pool.h>
#include <sdp/datasample.h>

/**
 * @brief Helper function to display the contents of the sdp_datasample.
 *
 * @param sample sdp_datasample to print.
 */
static void sdp_sp_print_sample(struct sdp_datasample *sample)
{
	printk("Filter:           0x%08X\n", sample->header.filter_bits);
	printk("  data_type:      0x%02X (%u)\n", sample->header.filter.data_type, sample->header.filter.data_type);
	printk("  ext_type:       0x%02X (%u)\n", sample->header.filter.ext_type, sample->header.filter.ext_type);
	printk("  Flags:          0x%04X\n", sample->header.filter.flags_bits);
	printk("    data_format:  %u\n", sample->header.filter.flags.data_format);
	printk("    encoding:     %u\n", sample->header.filter.flags.encoding);
	printk("    encrypr_alg:  %u\n", sample->header.filter.flags.encrypt_alg);
	printk("    timestamp:    %u\n", sample->header.filter.flags.timestamp);
	printk("    _rsvd1:       %u\n", sample->header.filter.flags._rsvd);
	printk("\n");
	printk("SrcLen:           0x%08X\n", sample->header.srclen_bits);
	printk("  len:            0x%04X (%u)\n", sample->header.srclen.len, sample->header.srclen.len);
	printk("  is_partial:     %u\n", sample->header.srclen.is_partial);
	printk("  _rsvd:          %u\n", sample->header.srclen._rsvd);
	printk("  sourceid:       %u\n", sample->header.srclen.sourceid);
	printk("\n");
	if (sample->header.srclen.len) {
		printk("Payload: ");
		for (uint32_t i = 0; i < sample->header.srclen.len; i++) {
			printk("%02X ", sample->payload[i]);
		}
		printk("\n");
	}
}

int sdp_sp_add(struct sdp_datasample *sample)
{
	sdp_sp_print_sample(sample);

	return 0;
}