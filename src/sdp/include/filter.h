/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef SDP_FILTER_H__
#define SDP_FILTER_H__

#include "sdp.h"

/**
 * @defgroup FILTER Filter engine definitions.
 * @ingroup sdp_api
 * @{
 */

/**
 * @file
 * @brief API header file for the SDP filter engine.
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Combinatorial boolean logic operand used between the current and
 *        previous filter values in a filter chain. (Prev AND current = match,
 *        Prev OR current = match, etc.)
 */
enum sdp_filter_boolean_op {
	SDP_FILTER_COMB_OP_NONE = 0,    /**< Undefined. */
	SDP_FILTER_COMB_OP_AND  = 1,    /**< Current AND previous filter must
	                                     match. */
	SDP_FILTER_COMB_OP_OR   = 2,    /**< Either current and/or previous filter
	                                     must match. */
	SDP_FILTER_COMB_OP_XOR  = 3,    /**< Only ONE of the current or previous
	                                     filters can match. */
};

/**
 * @brief Operand to apply to the current filter value.
 */
enum sdp_filter_op {
	SDP_FILTER_OP_NONE      = 0,    /**< Undefined. */
	SDP_FILTER_OP_NOT       = 1     /**< Current filter must NOT match. */
};

/**
 * @brief An individual filter entry.
 */
struct sdp_filter {
	/**
	 * @brief The combinatorial operand to apply between the current and
     *        previous sdp_filters.
	 */
	enum sdp_filter_boolean_op comb_op;

	/**
	 * @brief The operand to apply to the current sdp_filter.
	 */
	enum sdp_filter_op op;

	/**
	 * @brief The data sample's filter value must exactly match this value.
	 *
	 * @note This value has a higher priority than the 'bits' fields, and an
	 *       exact_match will cause the filter engine to skip parsing the
	 *       'bits' fields.
	 */
	uint32_t exact_match;

	/**
	 * @brief When an exact match isn't required, the bits fields can be
	 *        used to pattern match based on mask, set and cleared values.
	 */
	struct {
		/**
		 * @brief Any bits in the mask field set to '1' will be ignored, and
         *        considered to have a positive match. Only '0' bits will be
         *        evaluated for a match.
         * 
         * @note This value has a higher priority that the 'set' and 'cleared'
         *       fields. Any masked bits set in those fields will be ignored
         *       when evaluating a potential match.
		 */
		uint32_t mask;

		/**
		 * @brief All bits set to '1' here will match if they are also '1' in
		 *        the data sample's filter field.
         * 
         * @note In the case of both 'set' and 'cleared' being enabled
         *       for the same bit, the bit will be considered as a positive
         *       match regardless of it's value. This is the same effect as
         *       setting the bit in the 'mask' field.
		 */
		uint32_t set;

		/**
		 * @brief All bits set to '1' here will match if they are also '0' in
		 *        the data sample's filter field.
         * 
         * @note In the case of both 'set' and 'cleared' being enabled
         *       for the same bit, the bit will be considered as a positive
         *       match regardless of it's value. This is the same effect as
         *       setting the bit in the 'mask' field.
		 */
		uint32_t cleared;
	} bits;
};

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* SDP_FILTER_H_ */
