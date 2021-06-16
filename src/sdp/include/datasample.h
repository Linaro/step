/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef SDP_DATASAMPLE_H__
#define SDP_DATASAMPLE_H__

#include "sdp.h"

/**
 * @defgroup DATASAMPLE Data sample definitions.
 * @ingroup sdp_api
 * @{
 */

/**
 * @file
 * @brief API header file for SDP data samples.
 */

#ifdef __cplusplus
extern "C" {
#endif

/*
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
 * | Res | TSt | Alg | Encod |  DF | <- Flags
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *     |    |     |      |      |
 *     |    |     |      |      +-------- Data Format (TLV, CBOR, etc.)
 *     |    |     |      +--------------- Encoding (BASE64, etc.)
 *     |    |     +---------------------- Encryption Algorithm
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
 *           1 = TLV (Type, Length, Value record)
 *
 *               The sample consists of one or more TLV (type, length, value)
 *               records, with the full array of records being 'Record Length'
 *               bytes in total. TLV samples should be read until the end of
 *               the packet is reached.
 *
 *               This allow for additional meta-data (timestamps, etc.) to
 *               be included with a sample, or for multiple samples to be
 *               included in a single data sample record for efficieny sake.
 *
 *           2 = CBOR (Concise Binary Object Representation, rfc8949)
 *           3 = JSON (JavaScript Object Notation, rfc8259)
 *           4..7 Reserved
 *
 *       o Payload Encoding [3:6]
 *
 *           Encoding format used for the payload:
 *
 *           0 = None
 *           1 = BASE64 encoding	Data has been BASE64 encoded
 *           2..15 = Reserved
 *
 *       o Encryption Algorithm [7:9]
 *
 *           Encryption algorithm used (Data Structure = TLV Array):
 *
 *           0..7 = Reserved (TBD)
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
 * 			 Must be set to 0.
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
 *
 */

/** Data sample header. All fields in little endian. */
struct sdp_ds_header {
	/* Filter (header upper word). */
	union {
		struct {
			/* Data type this record contains. */
			uint8_t data_type;
			/* Extended data type value (meaning depends on datatype). */
			uint8_t ext_type;
			/* Flags */
			union {
				struct {
					/* Data format used (0 = none, 1 = TLV, 2 = CBOR, 3 = JSON). */
					uint16_t data_format : 3;
					/* Payload encoding used (0 = none, 1 = BASE64). */
					uint16_t encoding : 4;
					/* Encryption algorithm used (0 for none). */
					uint16_t encrypt_alg : 3;
				    /* Timestamp format (0 for none, 1 = epoch32, 2 = epoch64). */
					uint16_t timestamp : 3;
					/* Reserved for future used. */
					uint16_t _rsvd : 3;
				} flags;
				/* Flag bits (cbor, TLV array, etc.). */
				uint16_t flags_bits;
			};
		} filter;
		/* Filter bits. */
		uint32_t filter_bits;
	};

	/* Src/Len (header lower word). */
	union {
		struct {
			/* Data length, excluding the header. */
			uint16_t len;
			struct {
				/* Indicates this is a partial, non-final packet. */
				uint8_t is_partial : 1;
				/* Reserved for future used */
				uint8_t _rsvd : 4;
			};
			/* Data source registery ID associated with this sample. */
			uint8_t sourceid;
		} srclen;
		/* Src/Len bits. */
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

/** Base data sample types (8-bit) */
enum sdp_ds_type {
	/* 0 = Metadata only? */
	SDP_DS_TYPE_NONE        = 0,

	/* 1 = System event to alert to processors and sinks. */
	SDP_DS_TYPE_EVENT       = 1,

	/* 0x2..0xF (2-15): Unclassified standard data types. */
	SDP_DS_TYPE_NUMERIC     = 2,            /**< Unclassified numeric values. */
	SDP_DS_TYPE_STRING,                     /**< Null-terminated string. */

	/* 0x10..0x7F (16-127): Standardised base data types. */
	SDP_DS_TYPE_AUDIO       = 16,
	SDP_DS_TYPE_ACCELEROMETER,              /**< m/s^2 */
	SDP_DS_TYPE_AMPLITUDE,                  /**< 0..1.0 float */
	SDP_DS_TYPE_COLOR,                      /**< See extended type */
	SDP_DS_TYPE_COORDINATES,
	SDP_DS_TYPE_CURRENT,                    /**< mA */
	SDP_DS_TYPE_DIMENSION,                  /**< cm (length, width, etc.) */
	SDP_DS_TYPE_DISTANCE,                   /**< space between two points (proximity, etc.), cm */
	SDP_DS_TYPE_GRAVITY,                    /**< m/s^2 */
	SDP_DS_TYPE_GYROSCOPE,                  /**< rad/s */
	SDP_DS_TYPE_HUMIDITY,                   /**< relative humidity in percent */
	SDP_DS_TYPE_INDUCTANCE,                 /**< nH? */
	SDP_DS_TYPE_LIGHT,                      /**< Lux */
	SDP_DS_TYPE_LINEAR_ACCELERATION,        /**< m/s^2, WITHOUT gravity */
	SDP_DS_TYPE_MAGNETIC_FIELD,             /**< micro-Tesla */
	SDP_DS_TYPE_MASS,                       /**< Grams */
	SDP_DS_TYPE_ORIENTATION,                /* Rename Euler? */
	SDP_DS_TYPE_PRESSURE,                   /**< hectopascal (hPa) */
	SDP_DS_TYPE_QUATERNION,
	SDP_DS_TYPE_RESISTANCE,                 /**< Ohms */
	SDP_DS_TYPE_ROTATION_VECTOR,
	SDP_DS_TYPE_SPECTRAL_POWER,             /**< Array of values with nm + value */
	SDP_DS_TYPE_TEMPERATURE,                /**< Celcius */
	SDP_DS_TYPE_TIME,                       /**< Default = Epoch, Duration? */
	SDP_DS_TYPE_VOLTAGE,                    /**< mV? */

	/* 0xF0..0xFE (240-254): User-defined types. */
	SDP_DS_TYPE_USER_1      = 240,
	SDP_DS_TYPE_USER_2      = 241,
	SDP_DS_TYPE_USER_3      = 242,
	SDP_DS_TYPE_USER_4      = 243,
	SDP_DS_TYPE_USER_5      = 244,
	SDP_DS_TYPE_USER_6      = 245,
	SDP_DS_TYPE_USER_7      = 246,
	SDP_DS_TYPE_USER_8      = 247,
	SDP_DS_TYPE_USER_9      = 248,
	SDP_DS_TYPE_USER_10     = 249,
	SDP_DS_TYPE_USER_11     = 250,
	SDP_DS_TYPE_USER_12     = 251,
	SDP_DS_TYPE_USER_13     = 252,
	SDP_DS_TYPE_USER_14     = 253,
	SDP_DS_TYPE_USER_15     = 254,

	/* 0xFF (255) reserved for future use. */
	SDP_DS_TYPE_LAST        = 0xFF
};

/** Extended data types for SDP_DS_TYPE_NUMERIC. */
enum sdp_ds_ext_number {
	SDP_DS_EXT_TYPE_NUM_UNDEFINED = 0,
	/* Unsigned integers. */
	SDP_DS_EXT_TYPE_NUM_U8 = 0x01,
	SDP_DS_EXT_TYPE_NUM_U16,
	SDP_DS_EXT_TYPE_NUM_U32,
	SDP_DS_EXT_TYPE_NUM_U64,
	SDP_DS_EXT_TYPE_NUM_U128,
	/* Signed integers. */
	SDP_DS_EXT_TYPE_NUM_S8 = 0x10,
	SDP_DS_EXT_TYPE_NUM_S16,
	SDP_DS_EXT_TYPE_NUM_S32,
	SDP_DS_EXT_TYPE_NUM_S64,
	SDP_DS_EXT_TYPE_NUM_S128,
	/* IEEE 754 floats. */
	SDP_DS_EXT_TYPE_NUM_IEEE754_F16 = 0x20,
	SDP_DS_EXT_TYPE_NUM_IEEE754_F32,
	SDP_DS_EXT_TYPE_NUM_IEEE754_F64
	/* ToDo: Fixed point values. */
};

/** Extended data types for SDP_DS_TYPE_TEMPERATURE. */
enum sdp_ds_ext_temperature {
	SDP_DS_EXT_TYPE_TEMP_UNDEFINED  = 0,    /**< Undefined temperature */
	SDP_DS_EXT_TYPE_TEMP_AMBIENT    = 1,    /**< Ambient temperature */
	SDP_DS_EXT_TYPE_TEMP_DIE        = 2,    /**< Die temperature */
	SDP_DS_EXT_TYPE_TEMP_OBJECT     = 3     /**< Object temperature */
};

/** Extended data type values for SDP_DS_TYPE_COLOR. */
enum sdp_ds_ext_color {
	SDP_DS_EXT_TYPE_COLOR_UNDEFINED         = 0,    /**< Undefined RGBA */
	/* Standard RGB(+A) triplets */
	SDP_DS_EXT_TYPE_COLOR_RGBA8             = 1,    /**< 8-bit RGBA */
	SDP_DS_EXT_TYPE_COLOR_RGBA16            = 2,    /**< 16-bit RGBA */
	SDP_DS_EXT_TYPE_COLOR_RGBAF             = 3,    /**< 0..1.0 float32 RGBA */
	/* CIE values. */
	SDP_DS_EXT_TYPE_COLOR_CIE1931_XYZ       = 10,   /**< CIE1931 XYZ tristimulus */
	SDP_DS_EXT_TYPE_COLOR_CIE1931_XYY       = 11,   /**< CIE1931 xyY chromaticity */
	SDP_DS_EXT_TYPE_COLOR_CIE1960_UCS       = 12,   /**< CIE1960 UCS chromaticity */
	SDP_DS_EXT_TYPE_COLOR_CIE1976_UCS       = 13,   /**< CIE1976 UCS chromaticity */
	SDP_DS_EXT_TYPE_COLOR_CIE1960_CCT       = 20,   /**< CIE1960 CCT (Duv = 0) */
	SDP_DS_EXT_TYPE_COLOR_CIE1960_CCT_DUV   = 21    /**< CIE1960 CCT and Duv */
};

/** Payload data structure used. */
enum sdp_ds_format {
	/** No data structure (single record). */
	SDP_DS_FORMAT_NONE   = 0,
	/** Type, Length, Value record(s). */
	SDP_DS_FORMAT_TLV    = 1,
	/** CBOR record(s). */
	SDP_DS_FORMAT_CBOR   = 2,
	/** JSON record(s). */
	SDP_DS_FORMAT_JSON   = 3
};

/** Payload encoding used. */
enum sdp_ds_encoding {
	/** No encoding used (binary data). */
	SDP_DS_ENCODING_NONE        = 0,
	/** BASE64 Encoding. */
	SDP_DS_ENCODING_BASE64      = 1
};

/** Payload encryption algorithm. */
enum sdp_ds_encrypt_alg {
	/** No encryption used. */
	SDP_DS_ENCRYPT_ALG_NONE = 0
};

enum sdp_ds_timestamp
{
	/** No timestamp defined. */
	SDP_DS_TIMESTAMP_NONE = 0,
	/** 32-bit Unix epoch timestamp present. */
	SDP_DS_TIMESTAMP_EPOCH_32 = 1,
	/** 64-bit Unix epoch timestamp present. */
	SDP_DS_TIMESTAMP_EPOCH_64 = 2
};

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* SDP_DATASAMPLE_H_ */
