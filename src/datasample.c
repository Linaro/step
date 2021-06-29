/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <sdp/filter.h>
#include <sdp/datasample.h>

#include <sys/printk.h>

void sdp_ds_print(struct sdp_datasample *sample)
{
	printk("Filter:           0x%08X\n", sample->header.filter_bits);
	printk("  data_type:      0x%02X (%u)\n", sample->header.filter.data_type, sample->header.filter.data_type);
	printk("  ext_type:       0x%02X (%u)\n", sample->header.filter.ext_type, sample->header.filter.ext_type);
	printk("  Flags:          0x%04X\n", sample->header.filter.flags_bits);
	printk("    data_format:  %u\n", sample->header.filter.flags.data_format);
	printk("    encoding:     %u\n", sample->header.filter.flags.encoding);
	printk("    compression:  %u\n", sample->header.filter.flags.compression);
	printk("    timestamp:    %u\n", sample->header.filter.flags.timestamp);
	printk("    _rsvd:        %u\n", sample->header.filter.flags._rsvd);
	printk("\n");
	printk("SrcLen:           0x%08X\n", sample->header.srclen_bits);
	printk("  len:            0x%04X (%u)\n", sample->header.srclen.len, sample->header.srclen.len);
	printk("  fragment:       %u\n", sample->header.srclen.fragment);
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
