/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef SDP_DATASAMPLE_H__
#define SDP_DATASAMPLE_H__

#include <sdp/sdp.h>
#include <sdp/datasample/base.h>
#include <sdp/datasample/ext_color.h>
#include <sdp/datasample/ext_light.h>
#include <sdp/datasample/ext_number.h>
#include <sdp/datasample/ext_temperature.h>

/**
 * @defgroup DATASAMPLE Data Samples
 * @ingroup sdp_api
 * @brief API Header file for Data Samples
 *
 * <PRE>
 * Data Sample
 * ===========
 *
 *    3                   2                   1
 *  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |              Flags            |   Ext. Type   |   Base Type   | <- Filter
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |   Source ID   |   Reserved  |P|        Payload Length         | <- SrcLen
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                                               |
 * |                            Payload                            |
 * |                                                               |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      Timestamp (optional)                     |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 *            1
 *  5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * | Res | TSt | Res | Encod |  DF | <- Flags
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *     |    |     |      |      |
 *     |    |     |      |      +-------- Data Format (CBOR, etc.)
 *     |    |     |      +--------------- Encoding (BASE64, BASE45, etc.)
 *     |    |     +---------------------- Reserved
 *     |    +---------------------------- Timestampp
 *     +--------------------------------- Reserved
 *
 * Filter
 * ------
 * Indicates data type for payload parsing, and to enable exact-match or
 * selective filtering of data samples by processor nodes or data sinks.
 *
 *   o Base Data Type [0:7] (Mandatory)
 *
 *     The base data type of the sample
 *
 *   o Extended Data Type [8:15] (Optional)
 *
 *     The extended data type of the sample. Exact meaning is specific to
 *     the base data type indicated.
 *
 *   o Flags [16:31] (Optional)
 *
 *       o Data Format [0:2]
 *
 *           Data format used to organise the payload (if any).
 *
 *           0 = Raw binary data
 *
 *               A single data sample in binary format of the base and
 *               extended data type value supplied.
 *
 *           1 = CBOR (Concise Binary Object Representation, rfc8949)
 * 
 *               The sample is encoded as CBOR records (rfc8949), which
 *               optionally allows the use of COSE (rfc8152) to sign and/or
 *               encrypt the CBOR record(s). For non-trivial data, this is the
 *               recommended data format to use.
 *
 *           2..7 Reserved
 *
 *       o Payload Encoding [3:6]
 *
 *           Encoding format used for the payload:
 *
 *           0 = None
 *           1 = BASE64 encoding	Data has been BASE64 encoded
 *           2 = BASE45 encoding    Data has been BASE45 encoded
 *           3..15 = Reserved
 *
 *       o Reserved [7:9]
 * 
 *           Must be set to 0.
 *
 *       o Timestamp      [10:12]
 *
 *           Indicates that a timestamp of the specified format is appended at
 *           the end of the record. The length of the timestamp is INCLUDED in
 *           the packet's 'payload length' field. Timestamp size should only be
 *           considered when 'Partial' is 0, meaning that this is the final
 *           packet in the payload and contains the timestamp value.
 *
 *           0 = None
 *           1 = Unix Epoch 32-bit
 *           2 = Unix Epoch 64-bit
 *           3..7 = Reserved
 *
 *       o Reserved [13:15]
 *
 *           Must be set to 0.
 *
 * SrcLen
 * ------
 * Payload length for the data sample, and the source ID to correlate the
 * data sample with a source in the source registry.
 *
 *   o Payload Length [0:15]
 *
 *       Payload length in bytes, minus the header, including optional
 *       timestamp if present.
 *
 *   o Partial        [16]
 *
 *       A 1 indicates that this is a partial and non-final packet, and the
 *       contents should be appended to the previous packets from this source
 *       before being parsed.
 *
 *   o Reserved       [17:23]
 *
 *       Reserved
 *
 *   o Source ID      [24:31]
 *
 *       Source registry ID to correlate data with
 * </PRE>
 * 
 * @{
 */

/**
 * @file datasample.h
 * @brief API header file for SDP data samples.
 */

#ifdef __cplusplus
extern "C" {
#endif

/** Data sample header. All fields in little endian. */
struct sdp_ds_header {
	/** Filter (header upper word). */
	union {
		struct {
			/** Data type this record contains. */
			uint8_t data_type;
			/** Extended data type value (meaning depends on datatype). */
			uint8_t ext_type;
			/** Flags */
			union {
				struct {
					/** Data format used (0 = none, 1 = CBOR). */
					uint16_t data_format : 3;
					/** Payload encoding used (0 = none, 1 = BASE64). */
					uint16_t encoding : 4;
					/** Reserved for future use. */
					uint16_t _rsvd1 : 3;
					/** Timestamp format (0 for none, 1 = epoch32, 2 = epoch64). */
					uint16_t timestamp : 3;
					/** Reserved for future use. */
					uint16_t _rsvd2 : 3;
				} flags;
				/** Flag bits (cbor, timestamp, etc.). */
				uint16_t flags_bits;
			};
		} filter;
		/** Filter bits. */
		uint32_t filter_bits;
	};

	/** Src/Len (header lower word). */
	union {
		struct {
			/** Data length, excluding the header. */
			uint16_t len;
			struct {
				/** Indicates this is a partial, non-final packet. */
				uint8_t is_partial : 1;
				/** Reserved for future used */
				uint8_t _rsvd : 4;
			};
			/** Data source registery ID associated with this sample. */
			uint8_t sourceid;
		} srclen;
		/** Src/Len bits. */
		uint32_t srclen_bits;
	};
};

/**
 * @brief Data sample packet wrapper.
 */
struct sdp_datasample {
	/** Packet header containing filter data and payload length. */
	struct sdp_ds_header header;

	/** Payload contents. */
	uint8_t *payload;
};

/** Payload data structure used. */
enum sdp_ds_format {
	/** No data structure (single record). */
	SDP_DS_FORMAT_NONE      = 0,
	/** CBOR record(s). */
	SDP_DS_FORMAT_CBOR      = 1,
};

/** Payload encoding used. */
enum sdp_ds_encoding {
	/** No encoding used (binary data). */
	SDP_DS_ENCODING_NONE    = 0,
	/** BASE64 Encoding (rfc4648). */
	SDP_DS_ENCODING_BASE64  = 1,
	/** BASE45 Encoding (draft-faltstrom-base45-06). */
	SDP_DS_ENCODING_BASE45  = 2,
};

/** Optional timestamp format used. */
enum sdp_ds_timestamp {
	/** No timestamp included. */
	SDP_DS_TIMESTAMP_NONE           = 0,
	/** 32-bit Unix epoch timestamp present. */
	SDP_DS_TIMESTAMP_EPOCH_32       = 1,
	/** 64-bit Unix epoch timestamp present. */
	SDP_DS_TIMESTAMP_EPOCH_64       = 2,
	/** 32-bit millisecond device uptime counter. */
	SDP_DS_TIMESTAMP_UPTIME_MS_32   = 3,
	/** 64-bit millisecond device uptime counter. */
	SDP_DS_TIMESTAMP_UPTIME_MS_64   = 4,
	/** 64-bit microsecond device uptime counter. */
	SDP_DS_TIMESTAMP_UPTIME_US_64   = 5,
};

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* SDP_DATASAMPLE_H_ */
