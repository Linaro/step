/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <sys/printk.h>
#include <step/filter.h>
#include <step/measurement/measurement.h>

/**
 * @brief Simplistic ceil function for positive numbers.
 *
 * @param n The input floating point value.
 *
 * @return uint32_t The ceil of @ref n.
 */
static int32_t step_mes_sz_ceil(float n)
{
	int32_t i = (int32_t)n;

	if (n == (float)i) {
		return i;
	}

	return i + 1;
}

/**
 * @brief Simplistic base 2 pow function for integers.
 *
 * @param e The input exponent.
 *
 * @return uint32_t The ceil of @ref n.
 */
static uint32_t step_mes_sz_pow2(uint32_t e)
{
	uint32_t x = 1;

	for (uint32_t i = 0; i < e; i++) {
		x *= 2;
	}

	return x;
}

/**
 * @brief Calculates the number of bytes required to represent the specified
 *        timestamp in memory.
 *
 * @param ts        The @ref step_mes_timestamp to get the size of.
 *
 * @return uint32_t The number of bytes required to represent @ref ts in
 *                  memory.
 */
static uint32_t step_mes_sz_timestamp(enum step_mes_timestamp ts)
{
	uint32_t len = 0;

	switch (ts) {
	case STEP_MES_TIMESTAMP_NONE:
		len = 0;
		break;
	case STEP_MES_TIMESTAMP_EPOCH_32:
	case STEP_MES_TIMESTAMP_UPTIME_MS_32:
		len = 4;
		break;
	case STEP_MES_TIMESTAMP_EPOCH_64:
	case STEP_MES_TIMESTAMP_UPTIME_MS_64:
	case STEP_MES_TIMESTAMP_UPTIME_US_64:
		len = 8;
		break;
	}

	return len;
}

/**
 * @brief Calculates the number of bytes required to represent the specified
 *        ctype in memory if a standard, fixed-size type is provided,
 *        or 0 if the size can't be determined.
 *
 * @param ctype     The @ref step_mes_unit_ctype to get the size of.
 *
 * @return int32_t  The number of bytes required to represent @ref ctype in
 *                  memory, or 0 if the exact size can't be determined.
 */
static uint32_t step_mes_sz_ctype(enum step_mes_unit_ctype ctype)
{
	uint32_t len;

	switch (ctype) {
	case STEP_MES_UNIT_CTYPE_S8:
	case STEP_MES_UNIT_CTYPE_U8:
	case STEP_MES_UNIT_CTYPE_BOOL:
		len = 1;
		break;
	case STEP_MES_UNIT_CTYPE_S16:
	case STEP_MES_UNIT_CTYPE_U16:
		len = 2;
		break;
	case STEP_MES_UNIT_CTYPE_IEEE754_FLOAT32:
	case STEP_MES_UNIT_CTYPE_S32:
	case STEP_MES_UNIT_CTYPE_U32:
	case STEP_MES_UNIT_CTYPE_RANG_UNIT_INTERVAL_32:
	case STEP_MES_UNIT_CTYPE_RANG_PERCENT_32:
		len = 4;
		break;
	case STEP_MES_UNIT_CTYPE_IEEE754_FLOAT64:
	case STEP_MES_UNIT_CTYPE_S64:
	case STEP_MES_UNIT_CTYPE_U64:
	case STEP_MES_UNIT_CTYPE_COMPLEX_32:
	case STEP_MES_UNIT_CTYPE_RANG_UNIT_INTERVAL_64:
	case STEP_MES_UNIT_CTYPE_RANG_PERCENT_64:
		len = 8;
		break;
	case STEP_MES_UNIT_CTYPE_IEEE754_FLOAT128:
	case STEP_MES_UNIT_CTYPE_S128:
	case STEP_MES_UNIT_CTYPE_U128:
	case STEP_MES_UNIT_CTYPE_COMPLEX_64:
		len = 16;
		break;
	default:
		len = 0;
		break;
	}

	return len;
}

int32_t step_mes_sz_payload(struct step_mes_header *hdr)
{
	int32_t len = 0;
    uint32_t cnt = 0;
	uint32_t tmp;

	/* Timestamp. */
	if (hdr->filter.flags.timestamp) {
		len += step_mes_sz_timestamp(hdr->filter.flags.timestamp);
	}

	/* Sample count. */
	if (hdr->srclen.samples < 15) {
		cnt = step_mes_sz_pow2(hdr->srclen.samples);
	} else {
		/* Can't determine length for an arbitrary sample count. */
		len = -1;
		goto err;
	}

	/* CType. */
	tmp = step_mes_sz_ctype(hdr->unit.ctype);
	if (tmp) {
		len += tmp * cnt;
	} else {
		/* Can't determine min payload length with an ambiguous ctype. */
		len = -1;
		goto err;
	}

	/* Encoding. */
	switch (hdr->filter.flags.encoding) {
	case STEP_MES_ENCODING_BASE64:
		/* BASE64 encodes 3 bytes in 4 characters. */
		len = step_mes_sz_ceil((float)len / 3.0F) * 4;
		break;
	case STEP_MES_ENCODING_BASE45:
		/* Worst-case scenario for BASE45 = 1.67, except with 1 char. */
		if (len == 1) {
			len = 2;
		} else {
			len = step_mes_sz_ceil((float)len * 1.67);
		}
		break;
	case STEP_MES_ENCODING_NONE:
	default:
		/* No encoding, leave len as-is. */
		break;
	}

err:
	return len;
}

int32_t step_mes_validate(struct step_measurement *mes)
{
	int rc = 0;
	int32_t sz;

	if (mes == NULL) {
		rc = -EINVAL;
		goto err;
	}

	sz = step_mes_sz_payload(&(mes->header));
	if ((sz >= 0) && (sz > mes->header.srclen.len)) {
		/* Payload buffer isn't large enough. */
		rc = -ENOSPC;
	}

	/* ToDo: Make sure timestamp, arbitrary size, etc. are valid if used. */

err:
	return rc;
}

void step_mes_print(struct step_measurement *mes)
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
