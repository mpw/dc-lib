// =====================================================================
//  dc/src/dc_objects.c
// =====================================================================

//  Author:         (C) 2010-2011 Camil Demetrescu
//  License:        See the end of this file for license information
//  Created:        December 30, 2010
//  Module:         dc

//  Last changed:   $Date: 2011/04/02 14:05:36 $
//  Changed by:     $Author: demetres $
//  Revision:       $Revision: 1.43 $


#include "dc_globals.h"
#include "dc_profile.h"


// ---------------------------------------------------------------------
//  dc_new_group
// ---------------------------------------------------------------------
dc_group* dc_new_group() {

    dc_group* group;

    // allocate new group object
    pool_alloc(dc_group_pool_g, dc_group_pool_free_list_g, 
               group, dc_group);

    // check if allocation failed
    if (group == NULL) {
        #if DC_DEBUG == 1
        _dc_panic("dc_new_group failed: "
                  "can't allocate dc_group object");
        #else
        return NULL;
        #endif
    }

    #if DC_TRACE > 0
    _dc_fmessage(NULL, ">< dc_new_group %p", group);
    #endif

    // init fields
    group->time_stamp   = DC_MIN_TIME_STAMP;
    group->sched_class  = NULL;
    group->sched_obj    = NULL;
    _dc_init_g_comp(group);

    // set default scheduler
    dc_set_scheduler_type(group, NULL);

    return group;
}


// ---------------------------------------------------------------------
//  dc_del_group
// ---------------------------------------------------------------------
int dc_del_group(dc_group* group) {

    // if group is not empty, then the operation fails
    if (_dc_get_cons_count(group) > 0) {
        #if DC_DEBUG == 1
        _dc_panic("dc_del_group failed: group %p is not empty", group);
        #endif        
        return -1; 
    }

    // dispose of scheduler
    if (group->sched_class != NULL)
        group->sched_class->destroy(group->sched_obj);

    #if DC_TRACE > 0
    _dc_fmessage(NULL, "** free group %p", group);
    #endif

    // free group object
    pool_free(group, dc_group_pool_free_list_g);

    return 0;
}


// ---------------------------------------------------------------------
//  dc_set_self_trigger
// ---------------------------------------------------------------------
void dc_set_self_trigger(dc_group* group, int self_trigger) {
    _dc_set_self_trigger(group, self_trigger);
}


// ---------------------------------------------------------------------
//  dc_set_scheduler_type
// ---------------------------------------------------------------------
int dc_set_scheduler_type(dc_group* group, 
                          dc_scheduler_type* scheduler) {

    // restore default scheduler
    if (scheduler == NULL)
        scheduler = DC_DEFAULT_GROUP_SCHEDULER_STRUCT;

    // attempt to create new scheduler instance
    void* instance = scheduler->create();
    if (instance == NULL) {
        #if DC_DEBUG == 1
        _dc_panic("dc_set_scheduler failed: "
                  "cannot create scheduler instance");
        #endif
        return -1;
    }

    // dispose of old scheduler
    if (group->sched_class != NULL)
        group->sched_class->destroy(group->sched_obj);

    // set new scheduler
    group->sched_class = scheduler;
    group->sched_obj   = instance;

    return 0;
}


// ---------------------------------------------------------------------
//  dc_get_scheduler
// ---------------------------------------------------------------------
void* dc_get_scheduler(dc_group* group) {
    return group->sched_obj;
}


// ---------------------------------------------------------------------
//  dc_new_cons
// ---------------------------------------------------------------------
dc_cons* dc_new_cons(dc_cons_f fun, void *param, dc_group* group) {

    dc_cons* cons;

    // sanity check: 
    //     two least significant bits of group address must be zero
    _dc_assert(((dc_ptr)group & 0x3) == 0);

    // allocate new constraint object
    pool_alloc(dc_cons_pool_g, dc_cons_pool_free_list_g, 
               cons, dc_cons);

    // check if allocation failed
    if (cons == NULL) {
        #if DC_DEBUG == 1
        _dc_panic("dc_new_cons failed: "
                  "can't allocate dc_cons object");
        #else
        return NULL;
        #endif
    }

    #ifdef DC_PROFILE
    dc_curr_profile.num_new_cons++;
    if (dc_curr_profile.cons_peak_num < 
        dc_curr_profile.num_new_cons - dc_curr_profile.num_del_cons) 
        dc_curr_profile.cons_peak_num = 
            dc_curr_profile.num_new_cons - dc_curr_profile.num_del_cons;
    cons->prof.num_exec = 0;
    cons->prof.session = 0;
    #endif

    #if DC_TRACE > 0
    _dc_fmessage(NULL, ">> enter dc_new_cons %p", cons);
    _dc_add_to_message_indent(_DC_MSG_TAB_SIZE);
    #endif

    // init object fields
    cons->fun = fun;
    cons->param_next = param;
    _dc_init_c_comp(cons, group);
    cons->time_stamp = DC_SCHED_TIME_STAMP;

    // if constraint belongs to a group, increment group counter
    if (group != NULL) _dc_incr_cons_count(group);

    // if dc_new_cons is called in atomic execution mode, we add cons
    // to the temporary list of newly created constraints; the list 
    // contains all constraints created during the execution of the 
    // current constraint, in creation order; nodes in this list are 
    // added to the list of constraints to be executed 
    // (dc_init_cons_list_head_g) when the execution of the constraint 
    // terminates, or when all open atomic blocks are closed
    if (dc_atomic_blocks_count_g) {

        dc_init_cons* node;

        // allocate list node
        pool_alloc(dc_init_cons_pool_g, dc_init_cons_pool_free_list_g, 
               node, dc_init_cons);

        // check if allocation failed
        if (node == NULL) {

            // dispose of constraint object
            pool_free(cons, dc_cons_pool_free_list_g);

            #if DC_DEBUG == 1
            _dc_panic("dc_new_cons failed: "
                      "can't allocate dc_init_cons object");
            #else
            return NULL;
            #endif
        }

        // setup node
        node->cons = cons;
        node->next = NULL;

        // add node to list
        if (dc_init_cons_temp_list_head_g == NULL) {
            dc_init_cons_temp_list_head_g = node;
            dc_init_cons_temp_list_tail_g = node;
        }
        else {
            dc_init_cons_temp_list_tail_g->next = node;
            dc_init_cons_temp_list_tail_g       = node;
        }

        #if DC_TRACE > 0
        _dc_fmessage(NULL, "++ new constraint %p added to list "
            "of new constraints", cons);
        #endif
    }

    // constraint is executed right away
    else _exec_new_cons(cons);

    #if DC_TRACE > 0
    _dc_add_to_message_indent(-_DC_MSG_TAB_SIZE);
    _dc_fmessage(NULL, "<< leave dc_new_cons %p", cons);
    #endif

    return cons;
}


// ---------------------------------------------------------------------
//  dc_del_cons
// ---------------------------------------------------------------------
void dc_del_cons(dc_cons* cons) {

    // if constraint belongs to a group, decrement group counter
    dc_group* group = _dc_get_group(cons);
    if (group != NULL) _dc_decr_cons_count(group);

    // if constraint is currently unscheduled, deallocate it right away
    if (_dc_is_unscheduled(cons)) {

        #if DC_TRACE > 0
        _dc_fmessage(NULL, "** free cons %p", cons);
        #endif

        pool_free(cons, dc_cons_pool_free_list_g);
    } 

    #if DC_TRACE > 0
    else _dc_fmessage(NULL, "## mark cons %p for deletion", cons);
    #endif

    // mark constraint as deleted. This has two consequences:
    // 1) if it is currently scheduled, it will be freed when it is 
    //    extracted from the scheduling data structure 
    //    (deferred deletion)
    // 2) all dependencies involving the deleted constraint become 
    //    stale. Notice that the memory region of a freed constraint 
    //    object remains valid until a new constraint reuses it.
    cons->time_stamp = DC_DEL_TIME_STAMP;

    #ifdef DC_PROFILE
    dc_curr_profile.num_del_cons++;
    #endif
}


// ---------------------------------------------------------------------
//  dc_schedule_final
// ---------------------------------------------------------------------
int dc_schedule_final(dc_cons* cons, dc_cons_f final) {

    dc_final** sched_final;
    dc_ui32*   sched_final_size;
    dc_ui32*   sched_final_count;

    // get final handler flags
    unsigned fflags = _dc_get_fflags(cons);

    // in normal execution mode, the operation fails
    if (dc_curr_cons_g == NULL) {

        #if DC_DEBUG == 1
        _dc_panic("dc_schedule_final failed: "
                  "function called in normal execution mode");
        #endif

        return -1;
    }

    // final parameter is null: sheduling is cancelled
    if (final == NULL) {

        // cancel any previous scheduling request
        if (fflags == DC_CONS_FINAL_SCHED)
            _dc_set_fflags(cons, DC_CONS_FINAL_CANC);

        return 0;
    }

    // if final handler is already scheduled, no action is performed
    // remark: once a final handler has been specified for a constraint,
    //         there is no way to replace it during the current
    //         constraint solving session
    if (fflags != DC_CONS_FINAL_UNSCHED) return 0;

    // no group currently executing
    if (dc_curr_group_g == NULL) {

        // prepare for array filling
        sched_final       = &dc_sched_final_g;
        sched_final_size  = &dc_sched_final_size_g;
        sched_final_count = &dc_sched_final_count_g;
    }

    // group currently executing
    else {

        // check if constraint belongs to current group
        if (_dc_get_group(cons) != dc_curr_group_g) {

            #if DC_DEBUG == 1
            _dc_panic("dc_schedule_final failed: "
                      "constraint not in current group");
            #endif

            return -1;
        }

        // prepare for array filling
        sched_final       = &dc_sched_group_final_g;
        sched_final_size  = &dc_sched_group_final_size_g;
        sched_final_count = &dc_sched_group_final_count_g;
    }

    // check if there is no room left in the array of scheduled 
    // final handlers
    if (*sched_final_count == *sched_final_size) {

        // resize by 50% array of scheduled final handlers
        // (dc_sched_final_size_g must be at least 2)
        *sched_final_size += (*sched_final_size >> 1);

        // reallocate array of scheduled final handlers
        *sched_final = 
            (dc_final*)realloc(
                *sched_final, 
                *sched_final_size * sizeof(dc_final));

        // check if reallocation failed
        if (*sched_final == NULL)
            _dc_panic("out of memory: "
                      "too many final handlers scheduled by cons %p", 
                      dc_curr_cons_g);
    }

    // push record to array
    (*sched_final)[*sched_final_count].final = final;
    (*sched_final)[*sched_final_count].cons = cons;
    (*sched_final_count)++;

    // set constraint final handler flags
    _dc_set_fflags(cons, DC_CONS_FINAL_SCHED);

    return 0;
}


// ---------------------------------------------------------------------
//  dc_get_curr_cons
// ---------------------------------------------------------------------
dc_cons* dc_get_curr_cons() {
    return dc_curr_cons_g;
}


// ---------------------------------------------------------------------
//  dc_get_cons_fun
// ---------------------------------------------------------------------
dc_cons_f dc_get_cons_fun(dc_cons* cons) {
    return cons->fun;
}


// ---------------------------------------------------------------------
//  dc_get_cons_param
// ---------------------------------------------------------------------
void* dc_get_cons_param(dc_cons* cons) {
    return cons->param_next;
}


// ---------------------------------------------------------------------
//  dc_get_cons_group
// ---------------------------------------------------------------------
dc_group* dc_get_cons_group(dc_cons* cons) {
    return _dc_get_group(cons);
}


// Copyright (C) 2010-2011 Camil Demetrescu

// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
