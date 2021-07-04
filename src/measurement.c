/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <sdp/filter.h>
#include <sdp/measurement/measurement.h>

#include <sys/printk.h>

void sdp_mes_print(struct sdp_measurement *mes)
{
	printk("Filter:           0x%08X\n", mes->header.filter_bits);
	printk("  base_type:      0x%02X (%u)\n", mes->header.filter.base_type, mes->header.filter.base_type);
	printk("  ext_type:       0x%02X (%u)\n", mes->header.filter.ext_type, mes->header.filter.ext_type);
	printk("  Flags:          0x%04X\n", mes->header.filter.flags_bits);
	printk("    data_format:  %u\n", mes->header.filter.flags.data_format);
	printk("    encoding:     %u\n", mes->header.filter.flags.encoding);
	printk("    compression:  %u\n", mes->header.filter.flags.compression);
	printk("    timestamp:    %u\n", mes->header.filter.flags.timestamp);
	printk("    _rsvd:        %u\n", mes->header.filter.flags._rsvd);
	printk("\n");
	printk("Unit:             0x%08X\n", mes->header.unit_bits);
	printk("  si_unit:        0x%04X (%u)\n", mes->header.unit.si_unit, mes->header.unit.si_unit);
	printk("  scale_factor:   0x%02X (10^%d)\n", mes->header.unit.scale_factor, mes->header.unit.scale_factor);
	printk("  ctype:          0x%02X (%u)\n", mes->header.unit.ctype, mes->header.unit.ctype);
	printk("\n");
	printk("SrcLen:           0x%08X\n", mes->header.srclen_bits);
	printk("  len:            0x%04X (%u)\n", mes->header.srclen.len, mes->header.srclen.len);
	printk("  fragment:       %u\n", mes->header.srclen.fragment);
	printk("  _rsvd:          %u\n", mes->header.srclen._rsvd);
	if (mes->header.srclen.samples) {
		printk("  samples:        2^%u\n", mes->header.srclen.samples);
	} else {
		printk("  samples:        0 (1 sample)\n");
	}
	printk("  sourceid:       %u\n", mes->header.srclen.sourceid);
	printk("\n");
	if (mes->header.srclen.len) {
		printk("Payload: ");
		for (uint32_t i = 0; i < mes->header.srclen.len; i++) {
			printk("%02X ", ((uint8_t *)mes->payload)[i]);
		}
		printk("\n");
	}
}
