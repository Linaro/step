/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <step/filter.h>
#include <step/node.h>

void step_node_print(struct step_node *node)
{
	int count = 0;

	while (node != NULL) {
		printk("Node #%d\n", count);
		printk("-------\n");
		/* Name. */
		if (node->name != NULL) {
			printk("Name: %s\n", node->name);
		}

		/* Filter chain. */
		step_filt_print(&node->filters);

		/* Callback handlers */
		printk("Handlers:\n");
		printk("  init: %s\n",
		       node->callbacks.init_handler == NULL ? "no" : "yes");
		printk("  evaluate: %s\n",
		       node->callbacks.evaluate_handler == NULL ? "no" : "yes");
		printk("  matched: %s\n",
		       node->callbacks.matched_handler == NULL ? "no" : "yes");
		printk("  start: %s\n",
		       node->callbacks.start_handler == NULL ? "no" : "yes");
		printk("  run: %s\n",
		       node->callbacks.exec_handler == NULL ? "no" : "yes");
		printk("  stop: %s\n",
		       node->callbacks.stop_handler == NULL ? "no" : "yes");
		printk("  error: %s\n",
		       node->callbacks.error_handler == NULL ? "no" : "yes");

		printk("End of chain: %s\n\n", node->next == NULL ? "yes" : "no");

		count++;
		node = node->next;
	}
}
