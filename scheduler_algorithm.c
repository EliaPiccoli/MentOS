/// @file scheduler_algorithm.c
/// @brief Round Robin algorithm.
/// @date Mar 2019.

#include "prio.h"
#include "debug.h"
#include "assert.h"
#include "list_head.h"
#include "scheduler.h"
#include "stdio.h"

#define GET_WEIGHT(prio) prio_to_weight[USER_PRIO((prio))]
#define NICE_0_LOAD GET_WEIGHT(DEFAULT_PRIO)

task_struct *pick_next_task(runqueue_t *runqueue, time_t delta_exec)
{
	// Pointer to the next task to schedule.
	task_struct *next = NULL;

#if defined(SCHEDULER_RR)
	//==== Implementation of the Round-Robin Scheduling algorithm ============

        //nNode = next(curr) -> get the next process in the queue after the last one executed
	list_head* nNode = runqueue->curr->run_list.next;

	//nNode may be the head of the list, if so, i have to go to the next list_head element
	if(nNode == &runqueue->queue)
	        nNode = nNode->next;

	//get the list_entry of the next process
	next = list_entry(nNode, struct task_struct, run_list);

	//=======================================================================
#elif defined(SCHEDULER_PRIORITY)
	//==== Implementation of the Priority Scheduling algorithm ===============

        // Set the lowest priority to the max value
        time_t min = 140;

        //As suggested we could have done in this way (list will always have a next -> init (and bash))
        //next = list_entry(runqueue->queue.next, struct task_struct, run_list);
        //time_t min = next->se.prio;

        list_head *ptr;
        // Inter over the runqueue to find the task with the smallest priority value
        list_for_each (ptr, &runqueue->queue) {
                task_struct *entry = list_entry(ptr, struct task_struct, run_list);
                // Check if entry has a lower priority
                //must put <= otherwise it will be stuck at the first element (all processes have same priority)
                if (entry->se.prio <= min) {
                        next = entry;
                        min = next->se.prio;
                }
        }

	//=======================================================================
#elif defined(SCHEDULER_CFS)
	//==== Implementation of the Completely Fair Scheduling ==================

	//get the last executed process
	task_struct* current = runqueue->curr;
	// Get the weight of the current process
	// (use GET_WEIGHT macro!)
	int weight = GET_WEIGHT(current->se.prio);

	if (weight != NICE_0_LOAD) {
		// get the multiplicative factor for its delta_exec = 1024/process' weight
		double factor = NICE_0_LOAD/(double) weight;

		// weight the delta_exec with the multiplicative factor
		delta_exec *= factor;
	}

	// Update vruntime of the current process
	current->se.vruntime += delta_exec;
	//hp: the next process will be the same one
	next = current;
	// Inter over the runqueue to find the task with the smallest vruntime value
	list_head* ptr;
	list_for_each(ptr, &runqueue->queue) {
                task_struct* entry = list_entry(ptr, struct task_struct, run_list);
                if(entry->se.vruntime < next->se.vruntime)
                        //update the next process with the one with lowest vruntime
                        next = entry;
	}

	//========================================================================
#else
#error "You should enable a scheduling algorithm!"
#endif
	assert(next && "No valid task selected. Have you implemented a scheduling algorithm?");

	return next;
}
