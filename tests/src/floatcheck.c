/*
 * Copyright (c) 2019-2020 Kevin Townsend (KTOWN)
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <ctype.h>
#include "floatcheck.h"

bool
val_is_equal(float a, float b, float epsilon)
{
	float c;

	c = a - b;

	if (c < epsilon && -c < epsilon) {
		return 1;
	} else {
		return 0;
	}
}

bool
val_is_at_least(float a, float b)
{
	return a >= b ? 1 : 0;
}

bool
val_is_less_than(float a, float b)
{
	return a < b ? 1 : 0;
}

bool
val_is_greater_than(float a, float b)
{
	return a > b ? 1 : 0;
}

bool
val_is_within(float a, float upper, float lower)
{
	if ((a <= upper) && (a >= lower)) {
		return 1;
	} else {
		return 0;
	}
}
