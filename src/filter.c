/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <step/filter.h>

void step_filt_print(struct step_filter_chain *fc)
{
	if (fc->count == 0) {
		return;
	}
	
	printk("Filters: %d\n", fc->count);
	for (uint8_t i = 0; i < fc->count; i++) {
		printk("  #%d: ", i);

		/* Operand */
		switch (fc->chain[i].op) {
		case STEP_FILTER_OP_IS:
			printk("IS ");
			break;
		case STEP_FILTER_OP_NOT:
			printk("NOT ");
			break;
		case STEP_FILTER_OP_AND:
			printk("AND ");
			break;
		case STEP_FILTER_OP_AND_NOT:
			printk("AND NOT ");
			break;
		case STEP_FILTER_OP_OR:
			printk("OR ");
			break;
		case STEP_FILTER_OP_OR_NOT:
			printk("OR NOT ");
			break;
		case STEP_FILTER_OP_XOR:
			printk("XOR ");
			break;
		}

		printk("exact match: 0x%08X", fc->chain[i].match);
		if (fc->chain[i].ignore_mask) {
			printk(" (mask 0x%08X)\n",
			       fc->chain[i].ignore_mask);
		} else {
			printk("\n");
		}
	}
}

/**
 * @brief Evaluates a single filter record.
 *
 * @param f			The filter to evaluate.
 * @param mes		The step_measurement to evaluate against.
 * @param prev		Sum of the previous match results.
 * @param match		1 if a match occurred, otherwise.
 */
static void step_filt_evaluate_filter(struct step_filter *f,
				     struct step_measurement *mes, int prev, int *match)
{
	int curr_eval = 0;
	uint32_t mes_cmp = mes->header.filter_bits;
	uint32_t f_cmp = f->match;

	*match = 0;

	/* Mask out any ignored bits. */
	if (f->ignore_mask) {
		mes_cmp &= ~(f->ignore_mask);
		f_cmp &= ~(f->ignore_mask);
	}

	/* Equality check. */
	curr_eval = (mes_cmp == f_cmp ? 1 : 0);

	/* Operand evaluation against prev result(s). */
	switch (f->op) {
	case STEP_FILTER_OP_IS:
		*match = curr_eval ? 1 : 0;
		break;
	case STEP_FILTER_OP_NOT:
		*match = curr_eval ? 0 : 1;
		break;
	case STEP_FILTER_OP_AND:
		*match = (curr_eval & prev) ? 1 : 0;
		break;
	case STEP_FILTER_OP_AND_NOT:
		*match = (prev && !curr_eval) ? 1 : 0;
		break;
	case STEP_FILTER_OP_OR:
		*match = (prev || curr_eval) ? 1 : 0;
		break;
	case STEP_FILTER_OP_OR_NOT:
		*match = (prev || !curr_eval) ? 1 : 0;
		break;
	case STEP_FILTER_OP_XOR:
		*match = (prev != curr_eval) ? 1 : 0;
		break;
	}
}

int step_filt_evaluate(struct step_filter_chain *fc, struct step_measurement *mes,
		      int *match)
{
	int rc = 0;
	int curr_eval, prev_eval;

	*match = curr_eval = prev_eval = 0;

	/* Make sure we have a measurement. */
	if (mes == NULL) {
		rc = -EINVAL;
		goto err;
	}
  
	/* Count = 0 or fc = NULL means accept all measurements. */
	if ((fc == NULL) || (fc->count == 0)) {
		rc = 0;
		*match = 1;
		goto bail;
	}

	/* Make sure we start the chain with IS or NOT operands. */
	if ((fc->chain[0].op != STEP_FILTER_OP_IS) &&
	    (fc->chain[0].op != STEP_FILTER_OP_NOT)) {
		rc = -EINVAL;
		goto err;
	}

	/* Iterate through filter chain, tracking previous + current results. */
	for (uint32_t i = 0; i < fc->count; i++) {
		curr_eval = 0;

		/* Evalute current filter. */
		step_filt_evaluate_filter(&(fc->chain[i]), mes,
					 prev_eval, &curr_eval);

		/* Store results for comparison against next filter. */
		prev_eval = curr_eval;
	}

	/* If final instance of curr_eval == 1, we have a match. */
	*match = curr_eval;

err:
bail:
	return rc;
}
