/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <string.h>
#include <sys/printk.h>
#include <sys/slist.h>
#include <sdp/proc_mgr.h>
#include <sdp/filter.h>
#include <sdp/sample_pool.h>

/* Initialize the slist */
static sys_slist_t pm_node_slist = SYS_SLIST_STATIC_INIT(&pm_node_slist);

/**
 * @brief Struct used to represent a @ref sdp_node in the registry.
 */
struct sdp_pm_node_record {
	/**
	 * @brief Singly-linked list node reference.
	 */
	sys_snode_t snode;

	/**
	 * @brief Pointer to the @ref sdp_node to register.
	 */
	struct sdp_node *node;

	/**
	 * @brief Priority level (larger = higher priority).
	 */
	uint8_t priority;

	/**
	 * @brief Handle associated with this processor node in the proc. manager.
	 */
	uint8_t handle;

	/**
	 * @brief Config flags for this node in the linked list.
	 */
	struct {
		uint16_t enabled : 1;
	} flags;
};

/* Processor node registry. */
static struct sdp_pm_node_record sdp_pm_nodes[CONFIG_SDP_PROC_NODE_LIMIT];
static uint32_t sdp_pm_handle_counter = 0;

int sdp_pm_process(struct sdp_measurement *mes, int *matches, bool free)
{
	int rc = 0;
	int match = 0;
	struct sdp_pm_node_record *pnode;
	struct sdp_pm_node_record *tmp;
	struct sdp_node *n;

	if (matches != NULL) {
		*matches = 0;
	}

	if (sys_slist_is_empty(&pm_node_slist)) {
		printk("No processor nodes registered.\n");
		goto abort;
	}

	/* Cycle through registered nodes, checking for filter matches. */
	SYS_SLIST_FOR_EACH_CONTAINER_SAFE(&pm_node_slist, pnode, tmp, snode) {
		/* Skip disabled nodes. */
		if (pnode->flags.enabled) {
			n = pnode->node;

			/* Evaluate filter match. */
			/* ToDo: Implement filter cache mechanism. */
			if (n->callbacks.evaluate_handler != NULL) {
				/* Use the node's evaluate callback to determine match. */
				match = n->callbacks.evaluate_handler(mes, n->config);
			} else {
				/* Standard evaluation against the node's filter chain. */
				rc = sdp_filt_evaluate(&(n->filters), mes, &match);
			}

			/* Call matched handler if requested, can negate match value. */
			if (match && (n->callbacks.matched_handler != NULL)) {
				match = n->callbacks.matched_handler(mes, n->config);
			}

			/* Execute processor node chain on match. */
			if (match) {
				/* Fire individual nodes in the node chain. */
				do {
					/* Call start handler if requested. */
					if (n->callbacks.start_handler != NULL) {
						n->callbacks.start_handler(mes, n->config);
					}

					/* Execute the current node. */
					if (n->callbacks.run_handler != NULL) {
						n->callbacks.run_handler(mes, n->config);
					}

					/* Call stop handler if requested. */
					if (match && (n->callbacks.stop_handler != NULL)) {
						n->callbacks.stop_handler(mes, n->config);
					}

					/* Move to next node in the chain, if present. */
					n = n->next;
				} while (n != NULL);
			}

			/* Track the match count if requested. */
			if (match && (matches != NULL)) {
				*matches += 1;
			}
		}
	}

abort:
	/* Free measurement from shared memory. */
	if (free) {
		sdp_sp_free(mes);
	}

	return rc;
}

/* TODO: Setup timer for poll function using CONFIG_SDP_PROC_MGR_POLL_RATE. */
int sdp_pm_poll(int *mcnt, bool free)
{
	int rc = 0;
	struct sdp_measurement *mes;

	if (mcnt != NULL) {
		*mcnt = 0;
	}

	/* Check FIFO for unprocessed measurements. */
	mes = sdp_sp_get();
	while (mes != NULL) {
		if (mcnt != NULL) {
			*mcnt += 1;
		}
		/* Evaluate the measurement against the processor node registry. */
		rc = sdp_pm_process(mes, NULL, free);
		if (rc) {
			goto err;
		}
		/* Check if we have more measurements available. */
		mes = sdp_sp_get();
	}

err:
	return rc;
}

int sdp_pm_register(struct sdp_node *node, uint8_t pri, uint8_t *handle)
{
	int rc = 0;

	*handle = sdp_pm_handle_counter++;

	if (*handle >= CONFIG_SDP_PROC_NODE_LIMIT) {
		handle = NULL;
		rc = -ENOMEM;
		goto err;
	}

	printk("Registering node/chain (handle = %02d)\n", *handle);
	// sdp_node_print(node);

	memset(&sdp_pm_nodes[*handle], 0, sizeof(struct sdp_pm_node_record));
	sdp_pm_nodes[*handle].node = node;
	sdp_pm_nodes[*handle].priority = pri;
	sdp_pm_nodes[*handle].handle = *handle;
	sdp_pm_nodes[*handle].flags.enabled = 1;

	/* ToDo: Make sure nodes are registered in order of priority. */
	sys_slist_append(&pm_node_slist, &(sdp_pm_nodes[*handle].snode));

err:
	return rc;
}

int sdp_pm_disable(uint8_t handle)
{
	printk("Disabling processor node %d:\n", handle);

	return 0;
}

int sdp_pm_enable(uint8_t handle)
{
	printk("Enabling processor node %d:\n", handle);

	return 0;
}

int sdp_pm_clear(void)
{
	int rc = 0;

	printk("Resetting processor node registry\n");

	/* ToDo: Clear match cache. */

	sdp_pm_handle_counter = 0;
	for (uint8_t i = 0; i < CONFIG_SDP_PROC_NODE_LIMIT - 1; i++) {
		memset(&sdp_pm_nodes[i], 0, sizeof(struct sdp_pm_node_record));
	}

	sys_slist_init(&pm_node_slist);

	return rc;
}

int sdp_pm_list(void)
{
	struct sdp_pm_node_record *pnode;
	struct sdp_pm_node_record *tmp;

	printk("Processor node registry:\n");

	if (sys_slist_is_empty(&pm_node_slist)) {
		printk("  empty\n");
		goto abort;
	}

	/* Cycle through registered nodes. */
	SYS_SLIST_FOR_EACH_CONTAINER_SAFE(&pm_node_slist, pnode, tmp, snode) {
		printk("  %02d:", pnode->handle);
		struct sdp_node *n = pnode->node;

		do {
			printk(" [%s]", n->name);
			n = n->next;
		} while (n != NULL);
	}

abort:
	return 0;
}