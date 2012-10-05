// =====================================================================
//  dc/include/dc.h
// =====================================================================

//  Author:         (C) 2010-2011 Camil Demetrescu
//  License:        See the end of this file for license information
//  Created:        December 28, 2010
//  Module:         dc

//  Last changed:   $Date: 2011/04/02 13:56:57 $
//  Changed by:     $Author: demetres $
//  Revision:       $Revision: 1.30 $


#ifndef __dc__
#define __dc__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include "rm.h"


/** @file dc.h
    DC library main header. The header defines data types and functions
    for handling constraints, groups, and reactive memory blocks.
*/


// macros

// ---------------------------------------------------------------------
//  dc_inactive
// ---------------------------------------------------------------------
/** @def dc_inactive(lvalue)
    Macro for accessing a reactive lvalue as if it was non-reactive.
    Modifying the content of dc_inactive(lvalue) does not trigger any
    constraint re-execution and no dependency is logged if the content 
    of dc_inactive(lvalue) is read in constraint execution mode.
    @param lvalue expression that denotes an lvalue
    @note if lvalue is non-reactive, the expression dc_inactive(lvalue)
          if equivalent to the expression lvalue.
    @warning the expression lvalue is evaluated more than once by the
             macro, so it should not contain operations with 
             side-effects.
*/
#define dc_inactive(lvalue)                                          \
    (rm_is_reactive(&(lvalue)) ?                                     \
        *(typeof(lvalue)*)rm_get_inactive_ptr(&(lvalue)): (lvalue))


// ---------------------------------------------------------------------
//  dc_is_reactive
// ---------------------------------------------------------------------
/** @def dc_is_reactive(addr)
    Macro for checking if a given address points to a reactive memory 
    cell.
    @param addr memory address
    @return nonzero if addr points to a reactive memory cell, zero
            otherwise
*/
#define dc_is_reactive rm_is_reactive


// typedefs

// ---------------------------------------------------------------------
//  dc_cons
// ---------------------------------------------------------------------
/** Type of a constraint object */
typedef struct dc_cons  dc_cons;          


// ---------------------------------------------------------------------
//  dc_group
// ---------------------------------------------------------------------
/** Type of a group object */
typedef struct dc_group dc_group;


// ---------------------------------------------------------------------
//  dc_cons_f
// ---------------------------------------------------------------------
/** Type of a function that defines the code for a constraint or
    a final handler. 
    @param param user-defined parameter associated with the constraint
           by dc_new_cons()
*/
typedef void (*dc_cons_f)(void *param);


// ---------------------------------------------------------------------
//  dc_scheduler_type
// ---------------------------------------------------------------------
/** Type that defines the operations of a user-defined constraint 
    scheduling data structure. Users can define scheduling data 
    structures that specify custom constraint re-evaluation orders for 
    a given group of constraints: this is achieved by defining 
    appropriate operations for inserting (schedule()) and extracting 
    (pick()) constraint from the data structure. The schedule() 
    operation is called by the DC constraint solver when a constraint 
    is discovered to be out of date since one of the reactive memory 
    locations it depends on has changed. The pick() operation is 
    repeatedly called by the constraint solver to bring up to date all 
    constraints that have been scheduled for re-execution. A custom 
    scheduler can only be specified for a group of constraints; 
    constraints that do not belong to any group are always scheduled 
    by DC's built-in data structure. Operations create(), schedule(),
    pick(), and delete() should not tamper with DC's internal state by
    calling, e.g., dc_begin_at(), dc_end_at(), dc_new_cons(), etc.
    @see dc_set_scheduler_type(). Specific scheduler may offer further
    operations such as, e.g., setting a comparator for custom priority-
    based scheduling.
*/
typedef struct {

    // -----------------------------------------------------------------
    //  create
    // -----------------------------------------------------------------
    /** Scheduling data structure constructor. The operation
        creates a new instance of the scheduling data structure. It is 
        called by DC when the scheduler is assigned to a group. Users 
        should never call create() directly.
        @return pointer to a new instance of the scheduling data
                structure, or NULL if no instance could be created
    */
    void* (*create)();

    // -----------------------------------------------------------------
    //  schedule
    // -----------------------------------------------------------------
    /** Operation that schedules a constraint for execution. The 
        operation inserts a constraint into the scheduling data 
        structure. It is called by the DC constraint solver when a 
        constraint is discovered to be out of date since one of the 
        reactive memory locations it depends on has changed. Users 
        should never call schedule() directly.
        @param scheduler pointer to the scheduling data structure
        @param cons constraint to be inserted into the data structure
        @return 0 if the operation was successful, -1 if the operation
                failed
    */
    int (*schedule)(void* scheduler, dc_cons* cons);

    // -----------------------------------------------------------------
    //  pick
    // -----------------------------------------------------------------
    /** Operation that picks a constraint to be executed. The operation 
        extracts a constraint to be executed from the scheduling data 
        structure. It is repeatedly called by the constraint solver to 
        bring up to date all constraints that have been scheduled for 
        re-execution. Users should never call pick() directly.
        @param scheduler pointer to the scheduling data structure
        @return constraint to be executed, or NULL if the scheduling 
                data stucture is empty
    */
    dc_cons* (*pick)(void* scheduler);

    // -----------------------------------------------------------------
    //  destroy
    // -----------------------------------------------------------------
    /** Scheduling data structure destructor. The operation
        deletes an instance of a scheduling data structure previously
        created with create(). It is called by DC when a group is 
        deleted or a scheduler is replaced for a group. Users should 
        never call destroy() directly.
        @param scheduler pointer to the scheduling data structure to be
               deleted
        @see create()
    */
    void (*destroy)(void* scheduler);

} dc_scheduler_type;


// functions

// ---------------------------------------------------------------------
//  dc_init
// ---------------------------------------------------------------------
/** Initialize DC. Explicit initialization is optional, as DC is
    initialized automatically by dc_new_group(), dc_new_cons(),
    dc_malloc(), dc_calloc().
    @return 0 if the operation was successful, -1 otherwise
*/
int dc_init();


// ---------------------------------------------------------------------
//  dc_new_group
// ---------------------------------------------------------------------
/** Create new group of constraints.
    @return pointer to the newly created group, or NULL if the group 
            could not be created
*/
dc_group* dc_new_group();


// ---------------------------------------------------------------------
//  dc_del_group
// ---------------------------------------------------------------------
/** Delete group of constraints. If the group is not empty, then the 
    operation fails.
    @param group group of constraints
    @return 0 if the operation was successful, and -1 otherwise
*/
int dc_del_group(dc_group* group);


// ---------------------------------------------------------------------
//  dc_set_self_trigger
// ---------------------------------------------------------------------
/** Specify if constraints in a group can trigger themselves. 
    Self-triggering can be safely disabled for a group g if the reactive
    portion of a program's state after the execution of any constraint c 
    in g is a fixpoint for c, i.e., executing c more than once in a row 
    does not change any reactive memory locations. As this is the most
    common case, self-triggering is disabled by default for performance 
    reasons.
    @param group group of constraints
    @param self_trigger non-zero if self-triggering should be enabled, 
           zero otherwise
*/
void dc_set_self_trigger(dc_group* group, int self_trigger);


// ---------------------------------------------------------------------
//  dc_set_scheduler_type
// ---------------------------------------------------------------------
/** Set user-defined constraint scheduler type for a group. For some 
    applications, simple schedulers (e.g., lists, stacks, or FIFO 
    queues) may deliver better performances than the default priority-
    based scheduler.
    @param group group of constraints
    @param scheduler_type structure specifying a user-defined scheduler 
           type, or NULL to restore the default scheduler type
    @return 0 if the operation succeeded, or -1 if a new instance of the
            scheduler could not be created
*/
int dc_set_scheduler_type(dc_group* group, 
                          dc_scheduler_type* scheduler_type);


// ---------------------------------------------------------------------
//  dc_get_scheduler
// ---------------------------------------------------------------------
/** Get pointer to the scheduling data structure associated with a 
    group. The data structure contains all constraints of the group 
    that are scheduled for execution.
    @param group group of constraints
    @return pointer to the constraint scheduling data structure of the 
            group
*/
void* dc_get_scheduler(dc_group* group);


// ---------------------------------------------------------------------
//  dc_new_cons
// ---------------------------------------------------------------------
/** Create new constraint. If the call is made in normal execution mode 
    outside of any atomic block, the constraint function is executed 
    immediately with parameter param. If the call is made in constraint 
    execution mode or within an atomic block, the execution of the 
    constraint function is deferred until the end of the currently 
    executed constraint, or the end of the current atomic block. 
    During the execution of a constraint, the runtime system is in 
    constraint execution mode. Each constraint belongs to one group
    (possibly the default group). When a constraint of a non-default 
    group is executed, all scheduled constraints of the same group will 
    be executed before the constraints of any other group. If the
    execution of a constraint c creates other constraints, those will be 
    excuted immediately after the end of c (and before any other 
    constraint already scheduled) in the same order as they have been 
    created by c.
    @param fun pointer to a function defining the constraint
    @param param user-defined parameter to be passed to the constraint
    @param group group to which the constraint is added. If group is
           NULL, then the constraint is added to the default group.
    @return pointer to the newly created constraint, or NULL if 
            the constraint could not be created
    @see dc_del_cons()
*/
dc_cons* dc_new_cons(dc_cons_f fun, void *param, dc_group* group);


// ---------------------------------------------------------------------
//  dc_del_cons
// ---------------------------------------------------------------------
/** Delete constraint. 
    @param cons address of constraint to be deleted
    @see dc_new_cons()
*/
void dc_del_cons(dc_cons* cons);


// ---------------------------------------------------------------------
//  dc_schedule_final
// ---------------------------------------------------------------------
/** Schedule final handler execution for a constraint. Passing a NULL
    handler cancels any previous scheduling request for the constraint.
    A final handler scheduled for a constraint with dc_schedule_final()
    is executed at the end of the current group execution, if the 
    constraint belongs to the currently executed group, or at the end 
    of current constraint solving session, if the constraint does not 
    belong to any group. The handler is passed the same parameter as 
    the constraint it refers to. The operation fails if invoked in 
    normal execution mode.
    @param cons constraint
    @param final pointer to a finalization function for the constraint, 
           or NULL if a previous schedule operation has to be cancelled.
           If final is NULL and there is no pending handler scheduled,
           no action is performed.
    @return 0 on success, -1 if call is made in normal execution mode
            or the constraint does not belong to the currently
            executed group.
    @warning once a final handler has been specified for a constraint
             during a constraint solving session, there is no way to 
             replace it during that session, so any non-null final 
             parameter passed to successive calls is ignored
*/
int dc_schedule_final(dc_cons* cons, dc_cons_f final);


// ---------------------------------------------------------------------
//  dc_get_curr_cons
// ---------------------------------------------------------------------
/** Get currently executed constraint.
    @return pointer to currently executed constraint, or NULL if
            call is made in normal execution mode
*/
dc_cons* dc_get_curr_cons();


// ---------------------------------------------------------------------
//  dc_get_cons_fun
// ---------------------------------------------------------------------
/** Get constraint function.
    @param cons constraint
    @return pointer to the function associated with the constraint
*/
dc_cons_f dc_get_cons_fun(dc_cons* cons);


// ---------------------------------------------------------------------
//  dc_get_cons_param
// ---------------------------------------------------------------------
/** Get constraint parameter.
    @param cons constraint
    @return parameter associated with the constraint
*/
void* dc_get_cons_param(dc_cons* cons);


// ---------------------------------------------------------------------
//  dc_get_cons_group
// ---------------------------------------------------------------------
/** Get constraint group.
    @param cons constraint
    @return group to which the constraint belongs
*/
dc_group* dc_get_cons_group(dc_cons* cons);


// ---------------------------------------------------------------------
//  dc_begin_at
// ---------------------------------------------------------------------
/** Begin an atomic block. Atomic blocks can be nested: each call to
    dc_begin_at() increases by one the counter of open atomic blocks.
    If the call is made in constraint execution mode, no action occurs.
    @return 0 if the operation was successful, -1 if the call is made 
              in constraint execution mode, or there are already 
              UINT_MAX nested atomic blocks open.
*/
int dc_begin_at();


// ---------------------------------------------------------------------
//  dc_end_at
// ---------------------------------------------------------------------
/** End an atomic block. The operation decrements the atomic block 
    counter. If it reaches zero, then the solver re-executes all 
    constraints depending upon the reactive memory locations modified 
    during the atomic block being closed. The operation also executes 
    all new constraints created during the atomic block.
    If the call is made in constraint execution mode, no action occurs.
    @return 0 if the operation was successful, -1 if the call is made 
              in constraint execution mode, DC is not initialized, or
              no atomic block is currently open
*/
int dc_end_at();


// ---------------------------------------------------------------------
//  dc_begin_no_log
// ---------------------------------------------------------------------
/** Begin a no-log block. In a no-log block, no depencencies
    of constraints from reactive cells are logged. No-log blocks 
    can be nested: each call to dc_begin_no_log() increases by one the 
    counter of open no-log blocks. At the beginning of each constraint 
    execution, the counter is zero by default.
    If the call is made in normal execution mode, no action occurs.
    @return 0 if the operation was successful, -1 if the call is made 
              in normal execution mode, or there are already 
              UINT_MAX nested no-log blocks open.
*/
int dc_begin_no_log();


// ---------------------------------------------------------------------
//  dc_end_no_log
// ---------------------------------------------------------------------
/** End a no-log block. The operation decrements the no-log block 
    counter. If it reaches zero, then dependency logging is resumed
    for the currently executed constraint. 
    If the call is made in normal execution mode, no action occurs.
    @return 0 if the operation was successful, -1 if the call is made 
              in normal execution mode, DC is not initialized, or
              no no-log block is currently open
*/
int dc_end_no_log();


// ---------------------------------------------------------------------
//  dc_malloc
// ---------------------------------------------------------------------
/** Allocate reactive memory block. Allocates a block of size bytes of 
    reactive memory, returning a pointer to the beginning of the block. 
    The content of the newly allocated block of memory is not 
    initialized, remaining with indeterminate values. The semantics is 
    identical to the C malloc function, except that dc_malloc() 
    allocates a reactive memory block.
    @param size size of the memory block, in bytes
    @return On success, a pointer to the reactive memory block allocated 
            by the function. The type of this pointer is always void*, 
            which can be cast to the desired type of data pointer in 
            order to be dereferenceable. If the function failed to 
            allocate the requested block of memory, a null pointer is 
            returned.
    @see dc_free(), dc_realloc()
*/
void* dc_malloc(size_t size);


// ---------------------------------------------------------------------
//  dc_calloc
// ---------------------------------------------------------------------
/** Allocate space for array in reactive memory. Allocates a block of 
    reactive memory for an array of num elements, each of them size 
    bytes long, and initializes all its bits to zero. The effective 
    result is the allocation of an zero-initialized memory block of 
    (num*size) bytes. Initialization does not trigger the re-execution
    of constraints. The semantics is identical to the C calloc function, 
    except that dc_calloc() allocates a reactive memory block.
    @param num number of array elements to be allocated
    @param size size of array elements, in bytes
    @return a pointer to the reactive memory block allocated by the 
            function. The type of this pointer is always void*, which 
            can be cast to the desired type of data pointer in order to 
            be dereferenceable. If the function failed to allocate the 
            requested block of memory, a NULL pointer is returned.
    @see dc_free(), dc_realloc()
*/
void* dc_calloc(size_t num, size_t size);


// ---------------------------------------------------------------------
//  dc_realloc
// ---------------------------------------------------------------------
/** Reallocate reactive memory block. The size of the memory block 
    pointed to by the ptr parameter is changed to the size bytes, 
    expanding or reducing the amount of memory available in the block. 
    The function may move the memory block to a new location, in which 
    case the new location is returned. The content of the memory block 
    is preserved up to the lesser of the new and old sizes, even if the 
    block is moved. If the new size is larger, the value of the newly 
    allocated portion is indeterminate. In case that ptr is NULL, the 
    function behaves exactly as dc_malloc(), assigning a new block of  
    size bytes and returning a pointer to the beginning of it. In case  
    that the size is 0, the reactive memory previously allocated in ptr 
    is deallocated as if a call to free was made, and a NULL pointer is 
    returned. The semantics is identical to the C realloc function, 
    except that dc_realloc() reallocates a reactive memory block.
    @param ptr address of block to be reallocated
    @param size new size of the block
    @return non-zero if ptr points to a reactive memory region, zero 
            otherwise
    @see dc_free(), dc_malloc(), dc_calloc()
*/
void* dc_realloc(void* ptr, size_t size);


// ---------------------------------------------------------------------
//  dc_free
// ---------------------------------------------------------------------
/** Deallocate space in reactive memory. A block of reactive memory 
    previously allocated using a call to dc_malloc(), dc_calloc(), or 
    dc_realloc() is deallocated, making it available again for further 
    allocations. Notice that this function leaves the value of ptr 
    unchanged, hence it still points to the same (now invalid) location, 
    and not to the null pointer. The semantics is identical to the C 
    free function, except that dc_free() frees a reactive memory block.
    For performance reasons, constraint dependencies from a reactive 
    memory block are not cleared when the block is freed, so constraints 
    keep on depending on it until they are re-executed or the reactive 
    cells are written.
    @param ptr pointer to a memory block previously allocated with 
           dc_malloc(), dc_calloc(), or dc_realloc() to be deallocated. 
           If a null pointer is passed as argument, or the pointer 
           refers to a non-reactive memory region, no action occurs.
    @see dc_malloc(), dc_calloc()
*/
void dc_free(void* ptr);


#ifdef __cplusplus
}
#endif
#endif


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
