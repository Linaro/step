/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef STEP_MEASUREMENT_H__
#define STEP_MEASUREMENT_H__

#include <step/step.h>
#include <step/measurement/base.h>
#include <step/measurement/ext_color.h>
#include <step/measurement/ext_light.h>
#include <step/measurement/ext_temperature.h>
#include <step/measurement/unit.h>

/**
 * @defgroup MEASUREMENT Measurements
 * @ingroup step_api
 * @brief API Header file for measurements
 *
 * <PRE>
 * Measurement
 * ===========
 *
 * Measurements consist of a measurement type (Base Type + Extended Type),
 * represented in a specific SI unit (SI Unit Type), and implemented in a
 * specific C type in memory (C Type).
 *
 * There is an option to adjust the measurement's scale in +/- 10^n steps (Scale
 * Factor) from the default SI unit and scale indicated by the SI Unit Type.
 * For example, if 'Ampere' is indicated as the SI unit, the measurement could
 * indicate that the value is in uA by setting the scale factor to -6.
 *
 * The Filter fields, which indicate the measurement type and certain config
 * flag for the payload, is used to allow measurement consumers to 'subscribe'
 * to samples of interest based on the value(s) present in this word.
 *
 * Measurements have the following representation in memory:
 *
 *    3                   2                   1
 *  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |              Flags            |  Ext. M Type  |  Base M Type  | <- Filter
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |     C Type    | Scale Factor  |         SI Unit Type          | <- Unit
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |   Source ID   | S Cnt | V | F |        Payload Length         | <- SrcLen
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      Timestamp (optional)                     |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                                               |
 * |                            Payload                            |
 * |                                                               |
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
 *     |    |     +---------------------- Compression (LZ4, etc.)
 *     |    +---------------------------- Timestamp
 *     +--------------------------------- Reserved (version flag?)
 *
 * Filter
 * ------
 * Indicates measurement type for payload parsing, and to enable exact-match or
 * selective filtering of measurements by processor nodes.
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
 *               Raw, unformatted binary data using the specified C type.
 *
 *           1 = CBOR (Concise Binary Object Representation, rfc8949)
 *
 *               The payload is encoded as a CBOR record(s) (rfc8949), which
 *               optionally allows the use of COSE (rfc8152) to sign and/or
 *               encrypt the CBOR record(s). For non-trivial data, this is the
 *               recommended data format to use for complex use cases.
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
 *           1 = LZ4
 *           2..7 = Reserved
 *
 *       o Timestamp      [10:12]
 *
 *           Indicates that a timestamp of the specified format is appended at
 *           the start of the payload. The length of the timestamp is INCLUDED
 *           in the packet's 'payload length' field.
 *
 *           0 = None
 *           1 = Unix Epoch 32-bit
 *           2 = Unix Epoch 64-bit
 *           3 = Uptime in milliseconds, 32-bit
 *           4 = Uptime in milliseconds, 64-bit
 *           5 = Uptime in microseconds, 64-bit
 *           6..7 = Reserved
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
 *     The base, derived or compound SI unit used to represent this measurement.
 *
 *   o Scale Factor [16:23] (Optional)
 *
 *     An optional 10^n scale factor from the default unit scale represented
 *     by the SI Unit Type. If the default SI Unit Type is 'Ampere', for
 *     example, a Scale Factor of -3 would indicate that this particular
 *     measurement represents mA.
 *
 *   o C Type [24:31] (Mandatory)
 *
 *     The underlying C type used to represent the measurement in memory.
 *
 * SrcLen
 * ------
 * Payload length for the measurement, and the source ID to correlate the
 * measurement with a source in the source registry.
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
 *   o Vector Size    [18:19]
 *
 *       Indicates that this is a vector (rather than a scalar) value, composed
 *       of vect_sz + 1 components. Since this is only two bits, it is limited
 *       to representing small vectors or quaternion-type units containing
 *       up to 4 distinct components. Leave at 0 to indicate a scalar value.
 *
 *   o Sample Count   [20:23]
 *
 *       If more than one sample is present in the payload, this field can
 *       be used to represent the number of samples present. Sample count is
 *       represented in 2^n format, except for 0xF, where:
 *
 *       0 = 1 sample (default)     8 = 256 samples
 *       1 = 2 samples              9 = 512 samples
 *       2 = 4 samples              10 = 1024 samples
 *       3 = 8 sammples             11 = 2048 samples
 *       4 = 16 samples             12 = 4096 samples
 *       5 = 32 samples             13 = 8192 samples
 *       6 = 64 samples             14 = 16384 samples
 *       7 = 128 samples            15 = Arbitrary (see below)
 *
 *       If the Sample Count field is set to 0xF (15), the payload contains an
 *       arbitrary number of samples, where the sample count is placed as a
 *       32-bit word at the very beginning of the payload (after the optional
 *       timestamp if present!), as an unsigned integer in little-endian
 *       format.
 *
 *       If more than one sample is present, and the timestamp is enabled,
 *       the timestamp value corresponds to the time when the first sample
 *       was taken, and the interval between samples will need to be
 *       communicated out-of-band.
 *
 *       This field only has meaning when 'Data Format' is set to 0, meaning
 *       unformatted data is present in the payload. When a specific data
 *       format is used (CBOR, etc.), multiple samples should be indicated
 *       using an appropriate mechanism in that data format, and this field
 *       should be left at 0 to indicate that only one formatted payload is
 *       present.
 *
 *   o Source ID      [24:31]
 *
 *       Source registry ID to correlate measurements with. This allows for
 *       additional information about the source driver to be retrieved via an
 *       out-of-band channel. This may include min/max value range, sample
 *       rate, gain settings, precision, model and driver number, etc.
 * </PRE>
 *
 * @{
 */

/**
 * @file
 * @brief API header file for STEP measurements.
 */

#ifdef __cplusplus
extern "C" {
#endif

/* Mask values for use with the filter word. */
#define STEP_MES_MASK_FULL_TYPE_POS   (0)
#define STEP_MES_MASK_FULL_TYPE       (0xFFFF << STEP_MES_MASK_FULL_TYPE_POS)
#define STEP_MES_MASK_BASE_TYPE_POS   (0)
#define STEP_MES_MASK_BASE_TYPE       (0xFF << STEP_MES_MASK_BASE_TYPE_POS)
#define STEP_MES_MASK_EXT_TYPE_POS    (8)
#define STEP_MES_MASK_EXT_TYPE        (0xFF << STEP_MES_MASK_EXT_TYPE_POS)
#define STEP_MES_MASK_FLAGS_POS       (16)
#define STEP_MES_MASK_FLAGS           (0xFFFF << STEP_MES_MASK_FLAGS_POS)
#define STEP_MES_MASK_FORMAT_POS      (16)
#define STEP_MES_MASK_FORMAT          (0x7 << STEP_MES_MASK_FORMAT_POS)
#define STEP_MES_MASK_ENCODING_POS    (19)
#define STEP_MES_MASK_ENCODING        (0xF << STEP_MES_MASK_ENCODING_POS)
#define STEP_MES_MASK_COMPRESSION_POS (23)
#define STEP_MES_MASK_COMPRESSION     (0x7 << STEP_MES_MASK_COMPRESSION_POS)
#define STEP_MES_MASK_TIMESTAMP_POS   (26)
#define STEP_MES_MASK_TIMESTAMP       (0x7 << STEP_MES_MASK_TIMESTAMP_POS)

/**
 * @brief Measurement header. All fields in little endian.
 */
struct step_mes_header {
	/** Filter (header upper word). */
	union {
		struct {
			/** Base measurement type. */
			uint8_t base_type;
			/** Extended measurement type (meaning depends on base type). */
			uint8_t ext_type;
			/** Flags */
			union {
				struct {
					/** Data format used (1 = CBOR). */
					uint16_t data_format : 3;
					/** Payload encoding used (1 = BASE64). */
					uint16_t encoding : 4;
					/** Compression algorithm used (1 = LZ4). */
					uint16_t compression : 3;
					/** Timestamp format (1 = epoch32, 2 = epoch64). */
					uint16_t timestamp : 3;
					/** Reserved for future use. Must be set to 0. */
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
			 * Must be a member of step_mes_unit_si.
			 */
			uint16_t si_unit;

			/**
			 * @brief The amount to scale the measurement value up or down
			 * (10^n), starting from the unit and scale indicated by si_unit.
			 * Typically, but not necessarily a member of step_mes_unit_scale.
			 */
			int8_t scale_factor;

			/**
			 * @brief The data type that this SI unit is represented by in C.
			 * Must be a member of step_mes_unit_ctype.
			 *
			 * This field can be used to determine how many bytes are required
			 * to represent this measurement value, and how to interpret the
			 * value in memory.
			 */
			uint8_t ctype;
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
				/** Indicates the number of vector units. 0 for scalar. */
				uint8_t vec_sz : 2;
				/** Sample count (2^n), 0 = 1 sample */
				uint8_t samples : 4;
			};
			/** Data source registery ID associated with this sample. */
			uint8_t sourceid;
		} srclen;
		/** 32-bit representation of all Src/Len bits. */
		uint32_t srclen_bits;
	};
};

/**
 * @brief Measurement packet wrapper.
 */
struct step_measurement {
	/**
	 * Zephyr's FIFO implementation requires that FIFO entries reserve a single
	 * word at the start of the record for kernel use (to add the .next
	 * pointer). This is wasteful, but is added here until a better alternative
	 * can be found, or 'k_fifo_alloc_put' can be made to work properly.
	 */
	int *reserved;

	/** Packet header containing filter data, SI unit and payload length. */
	struct step_mes_header header;

	/** Payload contents. */
	void *payload;
};

/** Payload data structure used. */
enum step_mes_format {
	/** No data structure (raw C type data). */
	STEP_MES_FORMAT_NONE    = 0,
	/** CBOR record(s). */
	STEP_MES_FORMAT_CBOR    = 1,
};

/** Payload encoding used. */
enum step_mes_encoding {
	/** No encoding used (binary data). */
	STEP_MES_ENCODING_NONE          = 0,
	/** BASE64 Encoding (rfc4648). */
	STEP_MES_ENCODING_BASE64        = 1,
	/** BASE45 Encoding (draft-faltstrom-base45-06). */
	STEP_MES_ENCODING_BASE45        = 2,
};

/** Payload compression algorithm used. */
enum step_mes_compression {
	/** No payload compression used. */
	STEP_MES_COMPRESSION_NONE       = 0,
	/** LZ4 compression. */
	STEP_MES_COMPRESSION_LZ4        = 1,
};

/** Optional timestamp format used. */
enum step_mes_timestamp {
	/** No timestamp included. */
	STEP_MES_TIMESTAMP_NONE         = 0,
	/** 32-bit Unix epoch timestamp present. */
	STEP_MES_TIMESTAMP_EPOCH_32     = 1,
	/** 64-bit Unix epoch timestamp present. */
	STEP_MES_TIMESTAMP_EPOCH_64     = 2,
	/** 32-bit millisecond device uptime counter. */
	STEP_MES_TIMESTAMP_UPTIME_MS_32 = 3,
	/** 64-bit millisecond device uptime counter. */
	STEP_MES_TIMESTAMP_UPTIME_MS_64 = 4,
	/** 64-bit microsecond device uptime counter. */
	STEP_MES_TIMESTAMP_UPTIME_US_64 = 5,
};

/** Packet fragments. */
enum step_mes_fragment {
	/** No a fragment (complete payload). */
	STEP_MES_FRAGMENT_NONE          = 0,
	/** Non-final fragment in a larger payload. */
	STEP_MES_FRAGMENT_PARTIAL       = 1,
	/** Final fragment in the larger payload. */
	STEP_MES_FRAGMENT_FINAL         = 2,
};

/** Vector size. */
enum step_mes_vector_sz {
	/** Scalar value. */
	STEP_MES_VECTOR_SZ_NONE         = 0,
	/** Pair, such as an X,Y co-ordinate. */
	STEP_MES_VECTOR_SZ_2            = 1,
	/** Vector 3, such as an XYZ vector. */
	STEP_MES_VECTOR_SZ_3            = 2,
	/** Vector 4, such as a Quaternion. */
	STEP_MES_VECTOR_SZ_4            = 3,
};

/**
 * @brief Calculates the minimum number of bytes required for the measurement
 *        payload, taking into account the timestamp, ctype, sample count and
 *        encoding scheme indicated in the supplied header.
 *
 * @param hdr       Pointer to the @ref step_mes_header to parse.
 *
 * @return int32_t  The minimum number of bytes required to represent @ref mes
 *                  in memory, or -1 if a minimum payload can not be determined.
 *
 * @note This function does not currently take into account data format
 *       (CBOR, etc.) or compression (LZ4, etc.) when determining the minimum
 *       payload size. Be sure to adjust the returned minimum size to provide
 *       sufficient overhead in the payload for these features if required.
 */
int32_t step_mes_sz_payload(struct step_mes_header *hdr);

/**
 * @brief Checks the populated @ref step_measurement for common errors, such
 *        as the payload length being too small for the minimum payload.
 * 
 * @param msg       The populated measurement to validate. 
 * 
 * @return int32_t  0 if the measurement is valid, otherwise a negative
 *                  error code.
 */
int32_t step_mes_validate(struct step_measurement *mes);

/**
 * @brief Helper function to display the contents of the step_measurement.
 *
 * @param sample step_measurement to print.
 */
void step_mes_print(struct step_measurement *sample);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* STEP_MEASUREMENT_H_ */
