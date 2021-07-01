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
#include <sdp/datasample/unit.h>

/**
 * @defgroup DATASAMPLE Data Samples
 * @ingroup sdp_api
 * @brief API Header file for Data Samples
 *
 * <PRE>
 * Data Sample
 * ===========
 * 
 * Data samples consist of a measurement type (Base Type + Extended Type),
 * represented in a specific SI unit (SI Unit Type), and implemented in a
 * specific C type in memory (C Type).
 * 
 * There is an option to adjust the sample's scale in +/- 10^n steps (Scale
 * Factor) from the default SI unit and scale indicated by the SI Unit Type.
 * For example, if 'Ampere' is indicated as the SI unit, the sample could
 * indicate that the value is in uA by setting the scale factor to -6.
 *
 * The Filter fields, which indicate the measurement type and certain config
 * flag for the payload, is used to allow data sample consumers to 'subscribe'
 * to samples of interest based on the value(s) present in this word.
 * 
 * Data samples have the following representation in memory:
 *
 *    3                   2                   1
 *  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |              Flags            |  Ext. M Type  |  Base M Type  | <- Filter
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |  Scale Factor |    C Type     |         SI Unit Type          | <- Unit
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |   Source ID   |  Reserved | F |        Payload Length         | <- SrcLen
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
 * | Res | TSt | CMP | Encod |  DF | <- Flags
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *     |    |     |      |      |
 *     |    |     |      |      +-------- Data Format (CBOR, etc.)
 *     |    |     |      +--------------- Encoding (BASE64, BASE45, etc.)
 *     |    |     +---------------------- Compression (ZLIB DEFLATE, LZ4, etc.)
 *     |    +---------------------------- Timestampp
 *     +--------------------------------- Reserved
 *
 * Filter
 * ------
 * Indicates measurement type for payload parsing, and to enable exact-match or
 * selective filtering of data samples by processor nodes or data sinks.
 *
 *   o Base Measurement Type [0:7] (Mandatory)
 *
 *     The base measurement type of the sample
 *
 *   o Extended Measurement Type [8:15] (Optional)
 *
 *     The extended measurement type of the sample. Exact meaning is specific
 *     to the base measurement type indicated.
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
 *       o Compression [7:9]
 * 
 *           Compression algorithm used on the payload:
 * 
 *           0 = None
 *           1 = DEFLATE (RFC1951)
 *           2 = LZ4
 *           3..7 = Reserved
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
 * Unit
 * ----
 * Indicates the SI unit, scale factor and underlying C type used to implement
 * the specified base + extended measurement value.
 *
 *   o SI Unit Type [0:15] (Mandatory)
 *
 *     The base, derived or compound SI unit used to represent this sample.
 *
 *   o C Type [16:23] (Mandatory)
 *
 *     The underlying C type used to represent the sample in memory.
 *
 *   o Scale Factor [24:31] (Optional)
 *
 *     An optional 10^n scale factor from the default unit scale represented
 *     by the SI Unit Type. If the default SI Unit Type is 'Ampere', for
 *     example, a Scale Factor of -3 would indicate that this particular
 *     data sample represents mA.
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
 *   o Fragment       [16:17]
 *
 *       Indicates that this is a packet fragment, and the contents should be
 *       appended to the previous packets from this source before being parsed.
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
					/** Data format used (1 = CBOR). */
					uint16_t data_format : 3;
					/** Payload encoding used (1 = BASE64). */
					uint16_t encoding : 4;
					/** Compression algorithm used (1 = DEFLATE, 2 = LZ4). */
					uint16_t compression : 3;
					/** Timestamp format (1 = epoch32, 2 = epoch64). */
					uint16_t timestamp : 3;
					/** Reserved for future use. */
					uint16_t _rsvd : 3;
				} flags;
				/** Flag bits (cbor, timestamp, etc.). */
				uint16_t flags_bits;
			};
		} filter;
		/** 32-bit representation of all Filter bits. */
		uint32_t filter_bits;
	};

	/** Unit (header middle word). */
	union {
		struct {
			/**
			 * @brief The SI unit and default scale used for this measurement.
			 * Must be a member of zsl_dt_unit_si.
			 */
			uint16_t si_unit;

			/**
			 * @brief The data type that this SI unit is represented by in C.
			 * Must be a member of zsl_dt_unit_ctype.
			 *
			 * This field can be used to determine how many bytes are required
			 * to represent this measurement value, and how to nterpret the
			 * value in memory.
			 */
			uint8_t ctype;

			/**
			 * @brief The amount to scale the value up or down (10^n),
			 * starting from the unit and scale indicated by si_unit.
			 * Typically, but not necessarily a member of zsl_dt_unit_scale.
			 */
			int8_t scale_factor;
		} unit;
		/** 32-bit representation of si_unit, ctype and scale_factor. */
		uint32_t unit_bits;
	};

	/** Src/Len (header lower word). */
	union {
		struct {
			/** Payload length, excluding the header, including timestamp. */
			uint16_t len;
			struct {
				/** Indicates this is a fragment of a larger packet. */
				uint8_t fragment : 2;
				/** Reserved for future used */
				uint8_t _rsvd : 6;
			};
			/** Data source registery ID associated with this sample. */
			uint8_t sourceid;
		} srclen;
		/** 32-bit representation of all Src/Len bits. */
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
	void *payload;
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

/** Payload compression algorithm used. */
enum sdp_ds_compression {
	/** No payload compression used. */
	SDP_DS_COMPRESSION_NONE    = 0,
	/** DEFLATE compression (rfc1951). */
	SDP_DS_COMPRESSION_DEFLATE  = 1,
	/** LZ4 compression. */
	SDP_DS_COMPRESSION_LZ4  = 2,
};

/** Packet fragments. */
enum sdp_ds_fragment {
	/** No a fragment (complete payload). */
	SDP_DS_FRAGMENT_NONE    = 0,
	/** Non-final fragment in a larger payload. */
	SDP_DS_FRAGMENT_PARTIAL  = 1,
	/** Final fragment in the larger payload. */
	SDP_DS_FRAGMENT_FINAL  = 2,
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

/**
 * @brief Helper function to display the contents of the sdp_datasample.
 *
 * @param sample sdp_datasample to print.
 */
void sdp_ds_print(struct sdp_datasample *sample);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* SDP_DATASAMPLE_H_ */
