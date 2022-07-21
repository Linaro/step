/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <string.h>
#include <logging/log.h>
#include <sys/printk.h>
#include <sys/slist.h>
#include <step/proc_mgr.h>
#include <step/sample_pool.h>
#include <step/cache.h>
#include <step/instrumentation.h>

#define LOG_LEVEL LOG_LEVEL_DBG
LOG_MODULE_REGISTER(proc_mgr);

/**
 * @brief represents a callback chain to be invoked after a node is processed.
 */
struct step_node_sub_callback {
	/**
	 * @brief Singly-linked list node reference.
	 */
	sys_snode_t snode;
	/**
	 * @brief callback to be called.
	 */
	node_chain_completed_callback cb;
	/**
	 * @brief caller specific data.
	 */
	void* user_data;
};

/**
 * @brief Struct used to represent a @ref step_node in the registry.
 */
struct step_pm_node_record {
	/**
	 * @brief Singly-linked list node reference.
	 */
	sys_snode_t snode;

	/**
	 * @brief Singly-linked list of subscriber callbacks.
	 */
	sys_slist_t sub_callbacks;

	/**
	 * @brief Pointer to the @ref step_node to register.
	 */
	struct step_node *node;

	/**
	 * @brief Handle associated with this processor node in the proc. manager.
	 */
	uint32_t handle;

	/**
	 * @brief Priority level (larger = higher priority).
	 */
	uint16_t priority;

	/**
	 * @brief Config flags for this node in the linked list.
	 */
	struct {
		uint16_t enabled : 1;
	} flags;

#if CONFIG_STEP_INSTRUMENTATION
	/**
	 * @brief Runtime spent inside the node or node chain.
	 */
	uint32_t runtime_ns;

	/**
	 * @brief Number of times the node or node chain has run.
	 */
	uint32_t runs;
#endif
};

/* Processor node registry. This static array provides a fixed location in
 * memory for individual records in the processor node registry, along with
 * any relevant meta-data required when evaluating them. */
static struct step_pm_node_record step_pm_nodes[CONFIG_STEP_PROC_MGR_NODE_LIMIT];
static uint32_t step_pm_handle_counter = 0;

/* Node registry linked list. This ordered list contains a reference
 * to every registered processor node or node chain, in the order which
 * they should be evaluated. The 'process' function traverses this list. */
static sys_slist_t pm_node_slist = SYS_SLIST_STATIC_INIT(&pm_node_slist);

#if (CONFIG_STEP_PROC_MGR_POLL_RATE > 0) || defined(CONFIG_STEP_PROC_MGR_EVENT_DRIVEN) 
static void step_pm_poll_thread(bool free);
/* Create the polling thread. */
K_THREAD_DEFINE(step_pm_tid, CONFIG_STEP_PROC_MGR_STACK_SIZE,
		step_pm_poll_thread, 1, NULL, NULL,
		CONFIG_STEP_PROC_MGR_PRIORITY, 0, 0);
#endif

/* Registry should be locked during processing or when modifying it. */
K_MUTEX_DEFINE(step_pm_reg_access);
K_HEAP_DEFINE(step_callbacks_pool, CONFIG_STEP_PROC_MGR_CALLBACKS_NUM * sizeof(struct step_node_sub_callback));

int step_pm_register(struct step_node *node, uint16_t pri, uint32_t *handle)
{
	int rc = 0;
	struct step_pm_node_record *pnode;
	struct step_pm_node_record *tmp;
	struct step_pm_node_record *prev, *match;
	struct step_node *n = node;
	uint32_t idx = 0;

	/* Lock registry access during registration. */
	k_mutex_lock(&step_pm_reg_access, K_FOREVER);

	if (step_pm_handle_counter == CONFIG_STEP_PROC_MGR_NODE_LIMIT) {
		*handle = -1;   /* 0xFFFFFFFF */
		rc = -ENOMEM;
		LOG_ERR("Registration failed: node limit reached");
		goto err;
	}

	/* Assign and increment the handle counter. */
	*handle = step_pm_handle_counter++;

	LOG_DBG("Registering node/chain (handle %d, pri %d)", *handle, pri);

	/* Clear the processor node record placeholder. */
	memset(&step_pm_nodes[*handle], 0, sizeof(struct step_pm_node_record));
	step_pm_nodes[*handle].node = node;
	step_pm_nodes[*handle].priority = pri;
	step_pm_nodes[*handle].handle = *handle;
	step_pm_nodes[*handle].flags.enabled = 1;

	sys_slist_init(&step_pm_nodes[*handle].sub_callbacks);

	/* Find the correct insertion point based on priority. */
	match = prev = NULL;
	SYS_SLIST_FOR_EACH_CONTAINER_SAFE(&pm_node_slist, pnode, tmp, snode) {
		if ((match == NULL) && (pri > pnode->priority)) {
			match = pnode;
			break;
		}
		/* If no match, track current node for next iteration. */
		if (match == NULL) {
			prev = pnode;
		}
	}

	/* Insert new node record in appropriate position based on priority. */
	if ((match != NULL) && (prev == NULL)) {
		/* Prepend before sole record. */
		sys_slist_prepend(&pm_node_slist, &(step_pm_nodes[*handle].snode));
	} else if (match != NULL) {
		/* Insert before first lower priority record. */
		sys_slist_insert(&pm_node_slist,
				 &(prev->snode), &(step_pm_nodes[*handle].snode));
	} else {
		/* Insert at the end. */
		sys_slist_append(&pm_node_slist, &(step_pm_nodes[*handle].snode));
	}

	/* Node initialisation callbacks. */
	do {
		if (n->callbacks.init_handler != NULL) {
			/* Use the node's evaluate callback to determine match. */
			rc = n->callbacks.init_handler(n->config, *handle, idx);
			if (rc) {
				n->callbacks.error_handler(NULL, *handle, idx, rc);
			}
		}
		idx++;
		n = n->next;
	} while (n != NULL);

err:
	/* Release the registry lock. */
	k_mutex_unlock(&step_pm_reg_access);

	return rc;
}

struct step_node *step_pm_node_get(uint32_t handle, uint32_t inst)
{
	struct step_node *n;

	if (handle > step_pm_handle_counter) {
		LOG_ERR("Invalid handle: %d", handle);
		return NULL;
	}

	n = step_pm_nodes[handle].node;

	/* Return the first node instance if requested. */
	if (inst == 0) {
		return n;
	}

	/* Find the requested node instance. */
	/* ToDo: we could speed this up calculating the offset in memory. */
	for (uint32_t i = 0; i < inst; i++) {
		n = n->next;
		if (n == NULL) {
			LOG_ERR("Node instance out of bounds: %d:%d", handle, inst);
			return NULL;
		}
	}
	return n;
}

int step_pm_resume(void)
{
	int rc = 0;

#if (CONFIG_STEP_PROC_MGR_POLL_RATE > 0)
	k_thread_resume(step_pm_tid);
#endif

	return rc;
}

int step_pm_suspend(void)
{
	int rc = 0;

#if (CONFIG_STEP_PROC_MGR_POLL_RATE > 0)
	k_thread_suspend(step_pm_tid);
#endif

	return rc;
}

int step_pm_clear(void)
{
	int rc = 0;

	LOG_DBG("Resetting processor node registry");

	/* Lock registry access during clear. */
	k_mutex_lock(&step_pm_reg_access, K_FOREVER);

#if CONFIG_STEP_FILTER_CACHE
	/* Clear match cache. */
	step_cache_clear();
#endif

	/* Reset the handle counter. */
	step_pm_handle_counter = 0;

	/* Free the node record placeholders. */
	for (uint8_t i = 0; i < CONFIG_STEP_PROC_MGR_NODE_LIMIT; i++) {
		memset(&step_pm_nodes[i], 0, sizeof(struct step_pm_node_record));
	}

	/* Reset the linked list. */
	sys_slist_init(&pm_node_slist);

	/* Release the registry lock. */
	k_mutex_unlock(&step_pm_reg_access);

	return rc;
}

int step_pm_process(struct step_measurement *mes, int *matches, bool free)
{
	int rc = 0;
	int match = 0;
	int match_count = 0;
	int cached = 0;
	int node_idx = 0;
	struct step_pm_node_record *pnode;
	struct step_pm_node_record *tmp;
	struct step_node *n;

#if CONFIG_STEP_INSTRUMENTATION
	uint32_t instr = 0;
#endif

	/* Lock registry access during processing. */
	k_mutex_lock(&step_pm_reg_access, K_FOREVER);

	/* No nodes registered ... warn that sample will be lost. */
	if (sys_slist_is_empty(&pm_node_slist)) {
		LOG_WRN("Measurement lost: no processor node(s) registered");
		goto abort;
	}

	/* Cycle through registered nodes, checking for filter matches. */
	SYS_SLIST_FOR_EACH_CONTAINER_SAFE(&pm_node_slist, pnode, tmp, snode) {
		/* Skip disabled nodes. */
		if (pnode->flags.enabled) {
			n = pnode->node;

			/* Reset node index counter for node chains. */
			node_idx = 0;

#if CONFIG_STEP_INSTRUMENTATION
			/* Start total runtime INSTR timer. */
			STEP_INSTR_START(instr);
#endif

#if CONFIG_STEP_FILTER_CACHE
			/* Check filter cache for cached match results. */
			cached = step_cache_check(mes->header.filter_bits,
						  pnode->handle, &match);
#endif
			/* Evaluate filter match. */
			if (!cached) {
				if (n->callbacks.evaluate_handler != NULL) {
					/* Use the node's evaluate callback to determine match. */
					match = n->callbacks.evaluate_handler(mes,
									      pnode->handle, node_idx);
				} else {
					/* Standard evaluation against the node's filter chain. */
					rc = step_filt_evaluate(&(n->filters), mes, &match);
				}

				/* Call matched handler if requested, can negate match value. */
				if (match && (n->callbacks.matched_handler != NULL)) {
					match = n->callbacks.matched_handler(mes,
									     pnode->handle, node_idx);
				}

#if CONFIG_STEP_FILTER_CACHE
				/* Add match results to cache. */
				step_cache_add(mes->header.filter_bits, pnode->handle, match);
#endif
			}

			/* Release the registry lock. */
			k_mutex_unlock(&step_pm_reg_access);

			/* Execute processor node chain on match. */
			if (match) {
				/* Sequentially fire each node in the node chain. */
				do {
					int node_rc;
					/* Call start handler if requested. */
					if (n->callbacks.start_handler != NULL) {
						node_rc = n->callbacks.start_handler(mes,
										     pnode->handle, node_idx);
						/* Call error handler if necessary. */
						if (node_rc) {
							n->callbacks.error_handler(mes,
										   pnode->handle, node_idx, node_rc);
						}
					}

					/* Execute the main node processor. */
					if (n->callbacks.exec_handler != NULL) {
						node_rc = n->callbacks.exec_handler(mes,
										    pnode->handle, node_idx);
						/* Call error handler if necessary. */
						if (node_rc) {
							n->callbacks.error_handler(mes,
										   pnode->handle, node_idx, node_rc);
						}
					}

					/* Call stop handler if requested. */
					if (n->callbacks.stop_handler != NULL) {
						node_rc = n->callbacks.stop_handler(mes,
										    pnode->handle, node_idx);
						/* Call error handler if necessary. */
						if (node_rc) {
							n->callbacks.error_handler(mes,
										   pnode->handle, node_idx, node_rc);
						}
					}					
					/* Move to next node in the chain, if present. */
					node_idx++;
					n = n->next;
				} while (n != NULL);

				/* evaluate the subscriptors at end of node processing */
				struct step_node_sub_callback *subs;
				struct step_node_sub_callback *subs_tmp;
				SYS_SLIST_FOR_EACH_CONTAINER_SAFE(&pnode->sub_callbacks,subs,subs_tmp,snode) {
					if(subs->cb) {
						subs->cb(mes,pnode->handle,subs->user_data);
					}
				}

				/* Track the total match count. */
				match_count += 1;
			}

		/* Lock registry access during processing. */
		k_mutex_lock(&step_pm_reg_access, K_FOREVER);

#if CONFIG_STEP_INSTRUMENTATION
			/* Stop total runtime INSTR timer. */
			STEP_INSTR_STOP(instr);
			pnode->runtime_ns += instr;
			pnode->runs++;
#endif
		}
	}

	/* No matches ... warn that sample will be lost. */
	if (match_count == 0) {
		LOG_WRN("Measurement lost: no processor node(s) matched");
	}

abort:
	/* Return the number of filter matches in node registry if requested. */
	if (matches != NULL) {
		*matches = match_count;
	}

	/* Free measurement from shared memory if requested. */
	if (free) {
		step_sp_free(mes);
	}

	/* Release the registry lock. */
	k_mutex_unlock(&step_pm_reg_access);

	return rc;
}

int step_pm_poll(int *mcnt, bool free)
{
	int rc = 0;
	struct step_measurement *mes;

	if (mcnt != NULL) {
		*mcnt = 0;
	}

#ifdef CONFIG_STEP_PROC_MGR_EVENT_DRIVEN
		if(k_current_get() != step_pm_tid) {
			/* for compatibillity reasons if this function is called outside from
			 * poll thread it should not block:
			 */
			mes = step_sp_get();
		} else {
			/* block the thread until we get any sample on the FIFO */
			mes = step_sp_get_until_available();
		}
#else
	mes = step_sp_get();
#endif
	while (mes != NULL) {
		if (mcnt != NULL) {
			*mcnt += 1;
		}
		/* Evaluate the measurement against the processor node registry. */
		rc = step_pm_process(mes, NULL, free);
		if (rc) {
			goto err;
		}
		/* Check if we have more measurements available. */
		mes = step_sp_get();
	}

err:
	return rc;
}

#if (CONFIG_STEP_PROC_MGR_POLL_RATE > 0) || defined(STEP_PROC_MGR_EVENT_DRIVEN) 

/**
 * @brief Polling handler if automatic polling is requested.
 *
 * @param free Whether to free measurement from heap memory once processed.
 */
static void step_pm_poll_thread(bool free)
{
	int matches = 0;

	while (1) {
		step_pm_poll(&matches, free);
#if !defined(CONFIG_STEP_PROC_MGR_EVENT_DRIVEN) && (CONFIG_STEP_PROC_MGR_POLL_RATE > 0) 
		k_sleep(K_MSEC(1000 / CONFIG_STEP_PROC_MGR_POLL_RATE));
#endif

	}
}
#endif

int step_pm_disable_node(uint32_t handle)
{
	int rc = 0;

	LOG_DBG("Disabling processor node %d:", handle);

	if (handle > step_pm_handle_counter) {
		LOG_ERR("Invalid handle: %d", handle);
		rc = -EINVAL;
		goto err;
	}
	step_pm_nodes[handle].flags.enabled = 0;

err:
	return rc;
}

int step_pm_enable_node(uint32_t handle)
{
	int rc = 0;

	LOG_DBG("Enabling processor node %d:", handle);

	if (handle > step_pm_handle_counter) {
		LOG_ERR("Invalid handle: %d", handle);
		rc = -EINVAL;
		goto err;
	}
	step_pm_nodes[handle].flags.enabled = 1;

err:
	return rc;
}

int step_pm_subscribe_to_node(uint32_t handle, node_chain_completed_callback cb, void *user_data)
{
	int rc = 0;

	/* Lock registry access during registration. */
	k_mutex_lock(&step_pm_reg_access, K_FOREVER);

	LOG_DBG("Subscribing to processor node %d:", handle);

	if (handle > step_pm_handle_counter) {
		LOG_ERR("Invalid handle: %d", handle);
		rc = -EINVAL;
		goto err;
	}

	struct step_pm_node_record *r = &step_pm_nodes[handle];
	struct step_node_sub_callback *nc = k_heap_alloc(&step_callbacks_pool, 
												sizeof(struct step_node_sub_callback), 
												K_NO_WAIT);

	if(nc == NULL) {
		LOG_ERR("No more callbacks available: %d", handle);
		rc = -ENOMEM;
		goto err;
	}

	/* fill and attach this callback to the node chain callback list*/
	nc->cb = cb;
	nc->user_data = user_data;
	sys_slist_append(&r->sub_callbacks, &nc->snode);
	
err:
	k_mutex_unlock(&step_pm_reg_access);
	return rc;
}

int step_pm_list(void)
{
	struct step_pm_node_record *pnode;
	struct step_pm_node_record *tmp;
	uint32_t inst;

	printk("Processor node registry:\n");

	if (sys_slist_is_empty(&pm_node_slist)) {
		printk("  empty\n");
		goto abort;
	}

	/* Cycle through registered nodes. */
	SYS_SLIST_FOR_EACH_CONTAINER_SAFE(&pm_node_slist, pnode, tmp, snode) {
		printk("%d (priority %d):\n", pnode->handle, pnode->priority);
#if CONFIG_STEP_INSTRUMENTATION
		/* Note: This value is strictly limited to node evaluation inside the
		 * 'step_pm_process' function, and doesn't take into account the
		 * additional processing overhead in the larger pipeline. */
		printk("  Total processing time: %d ns (%d avg, %d runs)\n",
		       pnode->runtime_ns,
		       pnode->runtime_ns / pnode->runs, pnode->runs);
#endif
		/* Print individual nodes. */
		struct step_node *n = pnode->node;
		inst = 0;
		do {
			printk("  [%d] %s\n", inst, n->name);
			inst++;
			n = n->next;
		} while (n != NULL);
	}

abort:
	return 0;
}