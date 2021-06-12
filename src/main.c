/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <sys/printk.h>
#include "datasample.h"

void print_header(struct sdp_ds_header *header) {
    printk("Filter:           0x%08X\n", header->filter_bits);
    printk("  Flags:          0x%04X\n", header->filter.flags_bits);
    printk("    _rsvd1:       %u\n", header->filter.flags._rsvd);
    printk("    encrypr_alg:  %u\n", header->filter.flags.encrypt_alg);
    printk("    encoding_fmt: %u\n", header->filter.flags.encoding_fmt);
    printk("    data_struct:  %u\n", header->filter.flags.data_struct);
    printk("  ext_type:       0x%02X (%u)\n", header->filter.ext_type, header->filter.ext_type);
    printk("  data_type:      0x%02X (%u)\n", header->filter.data_type, header->filter.data_type);
    printk("\n");
    printk("SrcLen:           0x%08X\n", header->srclen_bits);
    printk("  sourceid:       %u\n", header->srclen.sourceid);
    printk("  _rsvd:          %u\n", header->srclen._rsvd);
    printk("  is_partial:     %u\n", header->srclen.is_partial);
    printk("  timestamp:      %u\n", header->srclen.timestamp);
    printk("  len:            0x%04X (%u)\n", header->srclen.len, header->srclen.len);
    printk("\n");
}

int main()
{
    struct sdp_ds_header header = { 0 };

    header.filter.flags.encrypt_alg = SDP_DS_ENCRYPT_ALG_NONE;
    header.filter.flags.encoding_fmt = SDP_DS_ENCODING_FMT_CBOR;
    header.filter.flags.data_struct = SDP_DS_STRUCTURE_TLV;
    header.filter.ext_type = SDP_DS_EXT_TYPE_NUM_IEEE754_F32;
    header.filter.data_type = SDP_DS_TYPE_NUMERIC;

	header.srclen.len = 0x1FF;
    header.srclen.is_partial = 1;
    header.srclen.timestamp = 2;
	header.srclen.sourceid = 0xA;

    print_header(&header);

    return 0;
}