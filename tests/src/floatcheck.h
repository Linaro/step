/*
 * Copyright (c) 2019-2020 Kevin Townsend (KTOWN)
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Helper functions to test floating point values.
 *
 * This file contains helpers to test floating point values.
 */

#ifndef ZEPHYR_INCLUDE_STEP_FLOATCHECK_H_
#define ZEPHYR_INCLUDE_STEP_FLOATCHECK_H_

#include <zephyr.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Checks if a float is equal to the specified value, with a +/- margin of
 * 'epsilon'.
 *
 * @param a         The value to check
 * @param b         The value to compare against
 * @param b         The +/- margin for equality
 *
 * @return  True if value 'a' is equal to 'b', otherwise false.
 */
bool val_is_equal(float a, float b, float epsilon);

/**
 * Checks if a float is equal to or greater than the specified value.
 *
 * @param a         The value to check
 * @param b         The value to compare against
 *
 * @return  True if value 'a' is greater than or equal to 'b', otherwise false.
 */
bool val_is_at_least(float a, float b);

/**
 * Checks if a float is less than the specified value.
 *
 * @param a         The value to check
 * @param b         The value to compare against
 *
 * @return  True if value 'a' is less than 'b', otherwise false.
 */
bool val_is_less_than(float a, float b);

/**
 * Checks if a float is greater than the specified value.
 *
 * @param a         The value to check
 * @param b         The value to compare against
 *
 * @return  True if value 'a' is greater than 'b', otherwise false.
 */
bool val_is_greater_than(float a, float b);

/**
 * Checks if a float is within the range of 'upper' and 'lower'.
 *
 * @param a         The value to check
 * @param upper     The upper value to compare against (inclusive)
 * @param lower     The lower value to compare against (inclusive)
 *
 * @return  True if value 'a' is <= 'upper' and >='lower', otherwise false.
 */
bool val_is_within(float a, float upper, float lower);

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_STEP_FLOATCHECK_H_ */
