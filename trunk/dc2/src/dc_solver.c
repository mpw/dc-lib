// =====================================================================
//  dc/src/dc_solver.c
// =====================================================================

//  Author:         (C) 2010-2011 Camil Demetrescu
//  License:        See the end of this file for license information
//  Created:        December 30, 2010
//  Module:         dc

//  Last changed:   $Date: 2011/03/11 09:43:34 $
//  Changed by:     $Author: demetres $
//  Revision:       $Revision: 1.68 $


#include "dc_globals.h"
#include "dc_profile.h"

#include <stdio.h>
#include <limits.h>


// local config
#define __dc_inline__  inline


// private functions
static __dc_inline__ void _schedule(dc_cell*);
static __dc_inline__ void _exec_cons(dc_cons*);
static __dc_inline__ void _dispatch();
static __dc_inline__ void _exec_final(dc_final*, int); 
static __dc_inline__ void _finalize_atomic();


// debugging macros
#define TRACE_READ_HANDLER 0


// ---------------------------------------------------------------------
//  _dc_read_handler
// ---------------------------------------------------------------------
void _dc_read_handler(dc_ptr* shadow_rec) {

    #if DC_TRACE > 0
    _dc_fmessage(NULL, "_dc_read_handler (addr = %p, shadow_rec=%p)", 
        rm_get_addr_fast(shadow_rec, dc_ui32, dc_ptr),
        shadow_rec);
    #endif

    #ifdef DC_PROFILE
    dc_curr_profile.num_first_read++;
    #endif

    // check if there is no room left in the array of read cells;
    if (dc_rd_cells_count_g == dc_rd_cells_size_g) {

        // resize by 50% array of read cells
        // (dc_rd_cells_size_g must be at least 2)
        dc_rd_cells_size_g += (dc_rd_cells_size_g >> 1);

        // reallocate array of read cells
        dc_rd_cells_g = 
            (dc_ptr**)realloc(
                dc_rd_cells_g, dc_rd_cells_size_g * sizeof(dc_ptr*));

        // check if reallocation failed
        if (dc_rd_cells_g == NULL)
            _dc_panic("out of memory: "
                      "too many cells read by constraint %p", 
                      dc_curr_cons_g);
    }

    // add address of shadow record to list of read cells
    dc_rd_cells_g[dc_rd_cells_count_g++] = shadow_rec;

    // if cell is first accessed, allocate cell info record
    dc_cell* cell = (dc_cell*)*shadow_rec;
    if (cell == NULL) {

        // allocate info record for reactive cell
        pool_alloc(dc_cell_pool_g, dc_cell_pool_free_list_g, 
                   cell, dc_cell);

        // cells must be aligned to 32-bit boundaries
        _dc_assert(((dc_ptr)cell & 0x3) == 0);

        // update shadow record
        *shadow_rec = (dc_ptr)cell;

        // initialize info record
        cell->first     = NULL;
        cell->countdown = DC_CELL_COUNTDOWN;
        cell->val       = 0;
    }

    // notice that the two least significant bits of cell are always
    // zero here, so cell contains the pointer to the cell info record

    dc_dep* dep = cell->first;

    #if TRACE_READ_HANDLER
    printf("-------- dep = cell->first == %p\n", dep);
    if (dep != NULL) 
        printf("-------- dep->time_stamp == %u, "
               "dep->cons->time_stamp == %u\n", 
               dep->time_stamp, dep->cons->time_stamp);
    #endif

    // if dependency list is empty or its first node is not stale, 
    // then allocate a new dependency node.
    if (dep == NULL || _dc_is_up_to_date(dep)) {

        // allocate depencency node
        pool_alloc(dc_dep_pool_g, dc_dep_pool_free_list_g, 
                   cell->first, dc_dep);

        // add it to the front of the list
        cell->first->next = dep;
        dep = cell->first;

        #if TRACE_READ_HANDLER
        printf("-------- new dep = cell->first == %p "
               "(dep->next == %p)\n", dep, dep->next);
        #endif

        // if cell countdown is greater than zero, decrement it
        if (cell->countdown > 0) {
            cell->countdown--;

            #if TRACE_READ_HANDLER
            printf("-------- countdown decreased to %u\n", 
                cell->countdown);
            #endif
        }

        // countdown has reached zero: remove all stale nodes from the 
        // list, letting the new countdown be the number of non-stale 
        // nodes remaining in the list. Using countdowns, the amortized
        // cost of cleaning up dependency lists is constant, while
        // keeping the fraction of stale nodes in the depencendy list 
        // no more than 50% of the total list length.
        else {

            dc_dep* p = dep;

            #if TRACE_READ_HANDLER
            printf("-------- scanning successors of %p\n", p);
            #endif

            #if DC_PROFILE
            dc_curr_profile.stale_cleanup_num++;
            #endif

            while (p->next != NULL) {

                // if next node is stale, then remove it
                if (p->next->time_stamp < 
                    p->next->cons->time_stamp) {
        
                    #if TRACE_READ_HANDLER
                    printf("-------- stale node %p\n", p->next);
                    #endif

                    // get reference to stale node to be deallocated
                    dc_dep* stale = p->next;
        
                    // unlink stale node
                    p->next = p->next->next;
        
                    // deallocate stale node
                    pool_free(stale, dc_dep_pool_free_list_g);
                }
    
                // if next node is not stale, skip it and increment
                // countdown
                else {
                    #if TRACE_READ_HANDLER
                    printf("-------- non-stale stale node %p\n", 
                        p->next);
                    #endif

                    p = p->next;
                    cell->countdown++;
                }
            }
        }
    }

    #if TRACE_READ_HANDLER
    printf("-------- setting up dep node %p\n", dep);
    #endif

    // setup dependency
    dep->time_stamp = dc_time_g;
    dep->cons = dc_curr_cons_g;

    // mark reactive cell as read
    *shadow_rec |= DC_READ_CELL;

    #if TRACE_READ_HANDLER
    printf("-------- end _dc_read_handler()\n");
    #endif
}


// ---------------------------------------------------------------------
//  _dc_write_handler
// ---------------------------------------------------------------------
void _dc_write_handler(dc_ptr* shadow_rec) {

    #if DC_TRACE > 0
    _dc_fmessage(NULL, "_dc_write_handler (addr = %p, shadow_rec=%p)", 
        rm_get_addr_fast(shadow_rec, dc_ui32, dc_ptr),
        shadow_rec);
    #endif

    #ifdef DC_PROFILE
    dc_curr_profile.num_first_write++;
    #endif

    // get pointer to cell info record;
    // since the cell may have been previously read in constraint 
    // execution mode, the least significant bit of the shadow record 
    // may be set to 1, so we retrieve the pointer by masking the 
    // shadow record value
    dc_cell* cell = (dc_cell*)(*shadow_rec & ~0x3);

    // if cell is first accessed, allocate cell info record
    if (cell == NULL) {

        // allocate info record for reactive cell
        pool_alloc(dc_cell_pool_g, dc_cell_pool_free_list_g, 
                   cell, dc_cell);

        // cells must be aligned to 32-bit boundaries
        _dc_assert(((dc_ptr)cell & 0x3) == 0);

        // update shadow record
        *shadow_rec = (dc_ptr)cell;

        // initialize info record
        cell->first     = NULL;
        cell->countdown = DC_CELL_COUNTDOWN;
        cell->val       = 0;
    }

    // in atomic execution mode (normal or constraint), just log the 
    // write operation
    if (dc_atomic_blocks_count_g) {

        // check if there is no room left in the array of written cells;
        if (dc_wr_cells_count_g == dc_wr_cells_size_g) {

            // resize by 50% arrays of written cells and their values
            // (dc_wr_cells_size_g must be at least 2)
            dc_wr_cells_size_g += (dc_wr_cells_size_g >> 1);

            // reallocate array of written cells
            dc_wr_cells_g = (dc_ptr**)realloc(
                dc_wr_cells_g, 
                    dc_wr_cells_size_g * sizeof(dc_ptr*));

            // reallocate array of written cell values
            dc_wr_cells_val_g = (dc_ui32*)realloc(
                dc_wr_cells_val_g, 
                    dc_wr_cells_size_g * sizeof(dc_ui32));

            // check if reallocation failed
            if (dc_wr_cells_g == NULL || dc_wr_cells_val_g == NULL)
                _dc_panic("out of memory: "
                          "too many cells written by constraint %p", 
                          dc_curr_cons_g);
        }

        // add address of shadow record to list of written cells
        dc_wr_cells_g[dc_wr_cells_count_g] = shadow_rec;
    
        // log cell value before the write operation
        dc_wr_cells_val_g[dc_wr_cells_count_g] = cell->val;
    
        // increment counter of written cells
        dc_wr_cells_count_g++;
    
        // mark reactive cell as written
        *shadow_rec |= DC_WRITTEN_CELL;

        return;
    }

    // if we get here, we are in normal, non-atomic execution mode

    // get value just written to the reactive cell
    dc_ui32 val = 
        *(dc_ui32*)rm_get_inactive_ptr(
            rm_get_addr_fast(shadow_rec, 
                             sizeof(dc_ui32), sizeof(dc_ptr)));

    // check if the new value is different from the previous one
    if (cell->val != val) {

        // update cell with new value
        cell->val = val;

        // schedule all constraints that depend upon the modified cell
        _schedule(cell);                    // inlined

        // enter level #1 atomic mode;
        // the execution of constraints is atomic
        dc_atomic_blocks_count_g = 1;

        #ifdef DC_PROFILE
        dc_curr_profile_auxdata.tentative_session = 1;
        #endif

        // dispatch scheduled constraints
        _dispatch();                        // inlined

        #ifdef DC_PROFILE
        dc_curr_profile_auxdata.tentative_session = 0;
        #endif

        // leave level #1 atomic mode
        dc_atomic_blocks_count_g = 0;
    }
}


// ---------------------------------------------------------------------
//  dc_begin_at
// ---------------------------------------------------------------------
int dc_begin_at() {

    // operation not allowed in constraint execution mode, or if too 
    // many nested atomic blocks are open
    if (dc_curr_cons_g != NULL || dc_atomic_blocks_count_g == UINT_MAX) 
        return -1;

    // increment counter of open atomic blocks
    ++dc_atomic_blocks_count_g;

    #if DC_TRACE > 0
    _dc_fmessage(NULL, ">> dc_begin_at() -- level #%u", 
        dc_atomic_blocks_count_g);
    _dc_add_to_message_indent(_DC_MSG_TAB_SIZE);
    #endif

    return 0;
}


// ---------------------------------------------------------------------
//  dc_end_at
// ---------------------------------------------------------------------
int dc_end_at() {

    // operation not allowed in constraint execution mode and if no 
    // blocks are open
    if (dc_curr_cons_g != NULL || dc_atomic_blocks_count_g == 0) 
        return -1;

    #if DC_TRACE > 0
    _dc_add_to_message_indent(-_DC_MSG_TAB_SIZE);
    _dc_fmessage(NULL, "<< dc_end_at() -- level #%u", 
        dc_atomic_blocks_count_g);
    #endif

    // decrement counter of open atomic blocks
    --dc_atomic_blocks_count_g;

    // if counter reaches zero, resume the solver
    if (dc_atomic_blocks_count_g == 0) {

        // schedule constraints and cleanup data structures
        _finalize_atomic();                 // inlined

        // enter level #1 atomic mode;
        // the execution of constraints is atomic
        dc_atomic_blocks_count_g = 1;

        #ifdef DC_PROFILE
        dc_curr_profile_auxdata.tentative_session = 1;
        #endif

        // dispatch scheduled constraints
        _dispatch();                        // inlined

        #ifdef DC_PROFILE
        dc_curr_profile_auxdata.tentative_session = 0;
        #endif

        // leave level #1 atomic mode
        dc_atomic_blocks_count_g = 0;
    }

    return 0;
}


// ---------------------------------------------------------------------
//  dc_begin_no_log
// ---------------------------------------------------------------------
int dc_begin_no_log() {

    // operation not allowed in normal execution mode, or if too 
    // many nested no-log blocks are open
    if (dc_curr_cons_g == NULL || dc_no_log_blocks_count_g == UINT_MAX) 
        return -1;

    // increment counter of open no-log blocks
    ++dc_no_log_blocks_count_g;

    #if DC_TRACE > 0
    _dc_fmessage(NULL, ">> dc_begin_no_log() -- level #%u", 
        dc_no_log_blocks_count_g);
    _dc_add_to_message_indent(_DC_MSG_TAB_SIZE);
    #endif

    return 0;
}


// ---------------------------------------------------------------------
//  dc_end_no_log
// ---------------------------------------------------------------------
int dc_end_no_log() {

    // operation not allowed in normal execution mode and if no 
    // blocks are open
    if (dc_curr_cons_g == NULL || dc_no_log_blocks_count_g == 0) 
        return -1;

    #if DC_TRACE > 0
    _dc_add_to_message_indent(-_DC_MSG_TAB_SIZE);
    _dc_fmessage(NULL, "<< dc_end_no_log() -- level #%u", 
        dc_no_log_blocks_count_g);
    #endif

    // decrement counter of open no-log blocks
    --dc_no_log_blocks_count_g;

    return 0;
}


// ---------------------------------------------------------------------
//  _schedule
// ---------------------------------------------------------------------
static __dc_inline__ void _schedule(dc_cell* cell) {

    // get pointer to first node
    dc_dep* dep = cell->first;

    // if list is empty, no action is performed
    if (dep != NULL) {

        // here, dep points to the first node in the list, which
        // contains at least one node

        // scan dependency list
        for (;;) {

            // if node is not stale, schedule constraint execution
            if (_dc_is_up_to_date(dep)) {

                // get constraint's group
                dc_group* group = _dc_get_group(dep->cons);

                // if constraint does not belong to any group
                if (group == NULL) {

                    // if constraint is not scheduled and either
                    // self-triggering is enabled, or the current
                    // constraint is not triggering itself
                    if (_dc_is_unscheduled(dep->cons) &&
                        (dc_self_trigger_g || 
                         dep->cons != dc_curr_cons_g)) {

                        #if DC_TRACE > 0
                        _dc_fmessage(NULL, 
                            "++ add cons %p to top-level scheduler", 
                            dep->cons);
                        #endif

                        // add constraint to the top-level scheduler
                        DC_SCHED_SCHEDULE(dc_top_level_scheduler_g, 
                                          dep->cons);

                        // mark it as scheduled
                        dep->cons->time_stamp = DC_SCHED_TIME_STAMP;
                    }
                }

                // else constraint belongs to a group
                else {

                    // if constraint is not scheduled and either
                    // self-triggering is enabled, or the current
                    // constraint is not triggering itself
                    if (_dc_is_unscheduled(dep->cons) &&
                        (dc_self_trigger_g || 
                         dep->cons != dc_curr_cons_g)) {

                        #if DC_TRACE > 0
                        _dc_fmessage(NULL, 
                            "++ add cons %p to group %p scheduler", 
                            dep->cons, group);
                        #endif

                        // add constraint to group's scheduler
                        group->sched_class->schedule(
                            group->sched_obj, dep->cons);

                        // mark it as scheduled
                        dep->cons->time_stamp = DC_SCHED_TIME_STAMP;
                    }

                    // if group is not yet scheduled
                    if (_dc_is_unscheduled(group)) {

                        #if DC_TRACE > 0
                        _dc_fmessage(NULL, 
                            "++ add group %p to top-level scheduler", 
                            group);
                        #endif

                        // add group to the top-level scheduler
                        DC_SCHED_SCHEDULE(dc_top_level_scheduler_g, 
                            (dc_cons*)(void*)group);

                        // mark it as scheduled
                        group->time_stamp = DC_SCHED_TIME_STAMP;
                    }
                }
            }

            // if dep is the last node, break cycle
            if (dep->next == NULL) break;

            // get next dependency
            dep = dep->next;
        }

        // here, dep points to last node in the list

        // since all constraints are going to be re-executed, all
        // dependencied in the list are becoming stale; therefore
        // we free the entire list but the first node, which is likely
        // to be reused
        dep->next = dc_dep_pool_free_list_g;
        dc_dep_pool_free_list_g = cell->first->next;
        cell->first->next = NULL;
    }
}


// ---------------------------------------------------------------------
//  _exec_new_cons
// ---------------------------------------------------------------------
void _exec_new_cons(dc_cons* cons) {

    // enter level #1 atomic mode;
    // the execution of constraints is atomic
    dc_atomic_blocks_count_g = 1;

    // set current group
    dc_curr_group_g = _dc_get_group(cons);

    // set global self-triggering flag for the current group 
    // execution
    if (dc_curr_group_g != NULL) {

        // set self-trigger flag
        dc_self_trigger_g = _dc_get_self_trigger(dc_curr_group_g);

        // mark group as scheduled
        dc_curr_group_g->time_stamp = DC_SCHED_TIME_STAMP;
    }

    #if DC_TRACE > 0
    if (dc_curr_group_g != NULL) {
        _dc_fmessage(NULL, ">> enter group %p", dc_curr_group_g);
        _dc_add_to_message_indent(_DC_MSG_TAB_SIZE);
    }
    #endif

    #ifdef DC_PROFILE
    dc_curr_profile_auxdata.tentative_session = 1;
    #endif

    // execute constraint
    _exec_cons(cons);

    // dispatch any constraints that may have been created or triggered
    // by the constraint
    _dispatch();

    #ifdef DC_PROFILE
    dc_curr_profile_auxdata.tentative_session = 0;
    #endif

    // leave level #1 atomic mode
    dc_atomic_blocks_count_g = 0;
}


// ---------------------------------------------------------------------
//  _exec_cons
// ---------------------------------------------------------------------
static __dc_inline__ void _exec_cons(dc_cons* cons) {

    // if constraint has been marked as deleted, free it 
    if (cons->time_stamp == DC_DEL_TIME_STAMP) {

        #if DC_TRACE > 0
        _dc_fmessage(NULL, "** free cons %p", cons);
        #endif

        pool_free(cons, dc_cons_pool_free_list_g);
    }

    // else execute it
    else {

        // increment global time-stamp (time-stamps start from 1)
        dc_time_g++;

        // set constraint time-stamp
        cons->time_stamp = dc_time_g;

        // check for time-stamp overflow
        if (dc_time_g > DC_MAX_TIME_STAMP) {

            // ***CD110111: renumber time-stamps (to be done)

            _dc_panic("fatal error: global time-stamp overflow");
        }

        // enter constraint execution mode:
        // let cons be the current constraint
        dc_curr_cons_g = cons;

        // start dependency logging
        dc_no_log_blocks_count_g = 0;

        #if DC_TRACE > 0
        _dc_fmessage(NULL, ">> enter constraint %p "
                           "(param == %p, time-stamp == %u)", 
                           cons, cons->param_next, dc_time_g);
        _dc_add_to_message_indent(_DC_MSG_TAB_SIZE);
        #endif

        #ifdef DC_PROFILE
        cons->prof.num_exec++;
        dc_curr_profile.num_exec_cons++;
        if (dc_curr_profile_auxdata.tentative_session == 1) {
            dc_curr_profile.num_solver_sessions++;
            dc_curr_profile_auxdata.tentative_session = 0;
        }
        if (cons->prof.session < dc_curr_profile.num_solver_sessions) {
            cons->prof.session = dc_curr_profile.num_solver_sessions;
            dc_curr_profile.num_distinct_exec_cons++;
        }
        #endif

        // execute constraint
        cons->fun(cons->param_next);

        // stop dependency logging
        dc_no_log_blocks_count_g = 1;

        #if DC_TRACE > 0
        _dc_add_to_message_indent(-_DC_MSG_TAB_SIZE);
        _dc_fmessage(NULL, "<< leave constraint %p", cons);
        #endif

        // post-process atomic constraint execution
        _finalize_atomic();

        // leave constraint execution mode
        dc_curr_cons_g = NULL;
    }
}


// ---------------------------------------------------------------------
//  _dispatch
// ---------------------------------------------------------------------
static __dc_inline__ void _dispatch() {

    dc_cons* cons;

    // main dispatch loop
    loop:

    // dispatch first execution of new constraints
    if (dc_init_cons_list_head_g != NULL) {

        // pick list head node
        dc_init_cons* head = dc_init_cons_list_head_g;

        // get constraint to be executed
        cons = head->cons;

        #if DC_TRACE > 0
        _dc_fmessage(NULL, "-- pick cons %p from new cons list", cons);
        #endif

        // remove head node from list
        dc_init_cons_list_head_g = dc_init_cons_list_head_g->next;
        pool_free(head, dc_init_cons_pool_free_list_g);

        // execute constraint
        _exec_cons(cons);

        goto loop;
    }

    // dispatch execution of further constraints scheduled in 
    // the current group
    if (dc_curr_group_g != NULL) {

        // pick constraint in the current group, if any
        cons = dc_curr_group_g->sched_class->pick(
                    dc_curr_group_g->sched_obj);

        #if DC_TRACE > 0
        _dc_fmessage(NULL, 
            "-- pick cons %p from group %p scheduler", 
            cons, dc_curr_group_g);
        #endif

        // if constraint was scheduled in the group, execute it
        if (cons != NULL) {
            _exec_cons(cons);
            goto loop;
        }

        // set group time-stamp as the time of the latest execution
        // of a constraint in the group
        dc_curr_group_g->time_stamp = dc_time_g;

        #if DC_TRACE > 0
        _dc_add_to_message_indent(-_DC_MSG_TAB_SIZE);
        _dc_fmessage(NULL, "<< leave group %p", dc_curr_group_g);
        #endif

        // no group is currently being executed
        dc_curr_group_g = NULL;

        // reset self-triggering flag
        dc_self_trigger_g = 0;

        // check if there are final handlers scheduled in the
        // current group
        if (dc_sched_group_final_count_g) {

            // execute final handlers that have been scheduled
            _exec_final(dc_sched_group_final_g, 
                        dc_sched_group_final_count_g);

            // reset list of scheduled final handlers
            dc_sched_group_final_count_g = 0;

            // post-process atomic execution of final handlers
            _finalize_atomic();

            // dispatch constraints created or triggered by final 
            // handlers
            goto loop;
        }
    }

    // pick constraint or group (if any) from the top-level scheduler
    dc_cons_group* cons_group = 
        (dc_cons_group*)(void*)DC_SCHED_PICK(dc_top_level_scheduler_g);

    #if DC_TRACE > 0
    _dc_fmessage(NULL, 
        "-- pick %s%p from top-level scheduler",
        cons_group == NULL ? "" : 
        _dc_is_group(cons_group) ? "group " : "cons ",
        cons_group);
    #endif

    // if constraint or group was scheduled, process it
    if (cons_group != NULL) {

        // check if scheduled object is a group
        if (_dc_is_group(cons_group)) {

            // let it be the current group
            dc_curr_group_g = (dc_group*)(void*)cons_group;

            // set global self-triggering flag for the current group 
            // execution
            dc_self_trigger_g = _dc_get_self_trigger(dc_curr_group_g);

            #if DC_TRACE > 0
            _dc_fmessage(NULL, ">> enter group %p", dc_curr_group_g);
            _dc_add_to_message_indent(_DC_MSG_TAB_SIZE);
            #endif

            // pick constraint in the current group 
            // (if we got here, there ought to be one)
            cons_group = (dc_cons_group*)(void*)
                dc_curr_group_g->sched_class->pick(
                    dc_curr_group_g->sched_obj);

            #if DC_TRACE > 0
            _dc_fmessage(NULL, 
                "-- pick cons %p from group %p scheduler", 
                cons_group, dc_curr_group_g);
            #endif

            _dc_assert(cons_group != NULL);
        }

        // execute constraint
        _exec_cons((dc_cons*)(void*)cons_group);

        goto loop;        
    }

    // check if there are scheduled final handlers
    if (dc_sched_final_count_g) {

        // execute final handlers that have been scheduled
        _exec_final(dc_sched_final_g, dc_sched_final_count_g);

        // reset list of scheduled final handlers
        dc_sched_final_count_g = 0;
    
        // post-process atomic execution of final handlers
        _finalize_atomic();

        // dispatch constraints created or triggered by final handlers
        goto loop;
    }
}


// ---------------------------------------------------------------------
//  _exec_final
// ---------------------------------------------------------------------
static __dc_inline__ void _exec_final(dc_final* sched_final, 
                                      int count) {

    size_t i = 0;

    // iterate over list of final handlers
    // handlers are executed in atomic mode
    do {

        // get constraint
        dc_cons* cons = sched_final[i].cons;

        // if request was not cancelled, execute final handler
        if (_dc_get_fflags(cons) != DC_CONS_FINAL_CANC) {                

            #if DC_TRACE > 0
            _dc_fmessage(NULL, ">> enter final handler for cons %p "
                               "(param == %p, final handler == %p)", 
                               cons, cons->param_next, 
                               sched_final[i].final);
            _dc_add_to_message_indent(_DC_MSG_TAB_SIZE);
            #endif

            // execute final handler
            sched_final[i].final(cons->param_next);

            #if DC_TRACE > 0
            _dc_add_to_message_indent(-_DC_MSG_TAB_SIZE);
            _dc_fmessage(NULL, "<< leave final handler for cons %p", 
                         cons);
            #endif
        }

        // reset final handler flags
        _dc_set_fflags(cons, DC_CONS_FINAL_UNSCHED);

    } while (++i < count);
}


// ---------------------------------------------------------------------
//  _finalize_atomic
// ---------------------------------------------------------------------
static __dc_inline__ void _finalize_atomic() {

    size_t i;

    // if new constraints have been created during the atomic block,
    // add them to the list of constraints to be executed
    if (dc_init_cons_temp_list_head_g != NULL) {

        // let next field of last node of temporary list point to
        // the first node in the init cons list (or NULL)
        dc_init_cons_temp_list_tail_g->next = 
            dc_init_cons_list_head_g;

        // let head of init cons list be the first node of the
        // init temp list
        dc_init_cons_list_head_g = dc_init_cons_temp_list_head_g;

        // clear init temp list head (don't care about tail...)
        dc_init_cons_temp_list_head_g = NULL;
    }

    // scan list of cells read during the atomic block
    // and clear their shadow record flags
    for (i = 0; i < dc_rd_cells_count_g; ++i) 
        *dc_rd_cells_g[i] &= ~0x3;

    // reset list of read cells
    dc_rd_cells_count_g = 0;

    // scan list of cells written during the atomic block
    for (i = 0; i < dc_wr_cells_count_g; ++i) {

        // get current value of reactive cell
        dc_ui32 val = 
            *(dc_ui32*)rm_get_inactive_ptr(
                rm_get_addr_fast(dc_wr_cells_g[i], 
                                 sizeof(dc_ui32), sizeof(dc_ptr)));

        // clear shadow record flags
        *(dc_wr_cells_g[i]) &= ~0x3;

        // get pointer to reactive cell info record
        dc_cell* cell = (dc_cell*)*dc_wr_cells_g[i];

        // check if the current value of the cell is different from its
        // original value at the beginning of the atomic block
        if (cell->val != val) {

            // update cell with new value
            cell->val = val;

            // schedule all constr. that depend upon the modified cell
            _schedule(cell);            // inlined
        }
    }

    // reset list of written cells
    dc_wr_cells_count_g = 0;
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
