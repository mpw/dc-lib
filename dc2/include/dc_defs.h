// =====================================================================
//  dc/include/dc_defs.h
// =====================================================================

//  Author:         (C) 2010-2011 Camil Demetrescu
//  License:        See the end of this file for license information
//  Created:        December 29, 2010
//  Module:         dc

//  Last changed:   $Date: 2011/04/02 14:14:42 $
//  Changed by:     $Author: demetres $
//  Revision:       $Revision: 1.51 $


#ifndef __dc_defs__
#define __dc_defs__

#include <stdarg.h>
#include <stdio.h>

#include "dc.h"
#include "dc_profile.h"
#include "profile.h"
#include "rm.h"

#ifdef __cplusplus
extern "C" {
#endif


// config parameters

// supported scheduling data structure types
#define STACK_SCHEDULER 1
#define HEAP_SCHEDULER  2

// default group scheduler setup
#define DC_DEFAULT_GROUP_SCHEDULER  HEAP_SCHEDULER

// top-level scheduler for groups and non-grouped constraints setup
#define TOP_LEVEL_SCHEDULER         HEAP_SCHEDULER

// scheduling data structure definitions
#if   DC_DEFAULT_GROUP_SCHEDULER == STACK_SCHEDULER
#include "dc_sched_stack.h"
#define DC_DEFAULT_GROUP_SCHEDULER_STRUCT dc_sched_stack_g

#elif DC_DEFAULT_GROUP_SCHEDULER == HEAP_SCHEDULER
#include "dc_sched_heap.h"
#define DC_DEFAULT_GROUP_SCHEDULER_STRUCT dc_sched_heap_g
#endif

#if   TOP_LEVEL_SCHEDULER == STACK_SCHEDULER
#include "dc_sched_stack.h"
#define DC_SCHED_CREATE   dc_sched_stack_create
#define DC_SCHED_SCHEDULE dc_sched_stack_schedule
#define DC_SCHED_PICK     dc_sched_stack_pick
#define DC_SCHED_DESTROY  dc_sched_stack_destroy

#elif TOP_LEVEL_SCHEDULER == HEAP_SCHEDULER
#include "dc_sched_heap.h"
#define DC_SCHED_CREATE   dc_sched_heap_create
#define DC_SCHED_SCHEDULE dc_sched_heap_schedule
#define DC_SCHED_PICK     dc_sched_heap_pick
#define DC_SCHED_DESTROY  dc_sched_heap_destroy
#endif

// number of blocks per page
#define DC_MEM_POOL_PAGE_SIZE               1000

// size in bytes of a reactive cell (do not change)
#define DC_REACTIVE_CELL_SIZE               4

// initial cell countdown to cleanup
#define DC_CELL_COUNTDOWN                   2

// minimum valid time stamp
#define DC_MIN_TIME_STAMP                   0x00000000

// maximum valid time stamp
#define DC_MAX_TIME_STAMP                   0xFFFFFFFD

// maximum number of constraints per group
#define DC_MAX_CONS_PER_GROUP               ((1<<29)-1)

// time stamp marker for scheduled constraints/groups
#define DC_SCHED_TIME_STAMP                 0xFFFFFFFF

// time stamp marker for deleted constraints
#define DC_DEL_TIME_STAMP                   0xFFFFFFFE

// initial size of arrays of read/written cells
// (must be at least 2)
#define DC_READ_WRITTEN_CELL_ARRAY_SIZE     1024

// initial size of arrays of final handlers
// (must be at least 2)
#define DC_SCHED_FINAL_ARRAY_SIZE           512

// verbose output line prefix
#define _DC_MSG_PREFIX                      "[dc] "

// verbose output tab size
#define _DC_MSG_TAB_SIZE                    4

// default output stream for dc messages
#define _DC_DEFAULT_MSG_STREAM              stdout

// profiling overhead calibration trials
#define _DC_PROF_OVH_CALIBRATION            10000


// intrinsic types
typedef signed   char      dc_si8;
typedef unsigned char      dc_ui8;
typedef signed   short     dc_si16;
typedef unsigned short     dc_ui16;
typedef signed   int       dc_si32;
typedef unsigned int       dc_ui32;
typedef signed   long long dc_si64;
typedef unsigned long long dc_ui64;

// 64-bit not really supported yet...
#if DC_64_BIT == 1
typedef dc_ui64 dc_ptr;
#else
typedef dc_ui32 dc_ptr;
#endif


// enums

enum dc_cons_group_flags {
    DC_CONS_FINAL_UNSCHED  = 0x00000000,
    DC_CONS_FINAL_SCHED    = 0x00000001,
    DC_CONS_GROUP_IS_GROUP = 0x00000002,
    DC_CONS_FINAL_CANC     = 0x00000003,
    DC_GROUP_SELF_TRIGGER  = 0x00000004,
    DC_GROUP_CONS_COUNT    = 0xFFFFFFF8
};

enum dc_cell_flags {
    DC_UNTOUCHED_CELL      = 0x00000000,
    DC_READ_CELL           = 0x00000001,
    DC_WRITTEN_CELL        = 0x00000003
};


// macros

// access c_comp composite field of constraint object
#define _dc_init_c_comp(c,g)         ((c)->c_comp =  (dc_ptr)(g))
#define _dc_get_group(c) ((dc_group*)((c)->c_comp & ~(dc_ptr)0x3))
#define _dc_get_fflags(c)            ((c)->c_comp &  (dc_ptr)0x3)
#define _dc_set_fflags(c,f)                                     \
          ((c)->c_comp = ((c)->c_comp & ~(dc_ptr)0x3) | (f))

// access g_comp composite field of group object
#define _dc_init_g_comp(g)      ((g)->g_comp = DC_CONS_GROUP_IS_GROUP)
#define _dc_get_cons_count(g)   ((g)->g_comp >> 3)
#define _dc_get_self_trigger(g) ((g)->g_comp & DC_GROUP_SELF_TRIGGER)
#define _dc_set_self_trigger(g,f)                               \
    ((g)->g_comp = (f) == 0 ?                                   \
        (g)->g_comp & ~DC_GROUP_SELF_TRIGGER :                  \
        (g)->g_comp |  DC_GROUP_SELF_TRIGGER )

#if DC_DEBUG == 1
#define _dc_incr_cons_count(g)                                  \
    do {                                                        \
        if (_dc_get_cons_count(g) < DC_MAX_CONS_PER_GROUP)      \
            (g)->g_comp += 0x8;                                 \
        else _dc_panic("too many constraints in group %p",      \
                       (g));                                    \
    } while(0)
#else
#define _dc_incr_cons_count(g) ((g)->g_comp += 0x8)
#endif

#if DC_DEBUG == 1
#define _dc_decr_cons_count(g)                                  \
    do {                                                        \
        if (_dc_get_cons_count(g) > 0)                          \
            (g)->g_comp -= 0x8;                                 \
        else _dc_panic("internal error: negative cons_count "   \
                       "for group %p", (g));                    \
    } while(0)
#else
#define _dc_decr_cons_count(g) ((g)->g_comp -= 0x8)
#endif

// check if a dc_cons_group object is a group
#define _dc_is_group(cg)                                        \
    (((cg)->cg_comp & 0x3) == DC_CONS_GROUP_IS_GROUP)

// check if a dependency node is stale or up to date
#define _dc_is_up_to_date(d) ((d)->time_stamp >= (d)->cons->time_stamp)
#define _dc_is_stale(d)      ((d)->time_stamp <  (d)->cons->time_stamp)

// check if a constraint or group is unscheduled
#define _dc_is_unscheduled(cg) ((cg)->time_stamp < DC_DEL_TIME_STAMP)

// debugging macro
#if DC_DEBUG == 1
#define _dc_assert(cond)                                               \
    do {                                                               \
        if (!(cond))                                                   \
            _dc_panic("internal error: assertion (" #cond ") failed, " \
                      "file: %s, line: %d", __FILE__, __LINE__);       \
    } while(0)
#else
#define _dc_assert(cond)
#endif


// cell-constraint dependency object
// size: 12 bytes (32-bit platform), 20 bytes (64-bit platform)
typedef struct dc_dep dc_dep;
struct dc_dep {
    dc_dep*  next;       // next dependency record (must be first field)
    dc_ui32  time_stamp; // dependency log time
    dc_cons* cons;       // constraint depending upon reactive cell
};


// cell info object
// warning: must be aligned to 32-bit boundaries
// size: 12 bytes (32-bit platform), 20 bytes (64-bit platform)
typedef struct dc_cell dc_cell;
struct dc_cell {
    dc_dep*  first;      // first dependency record
    dc_ui32  countdown;  // countdown to next dependency cleanup, i.e.,
                         // number of remaining nodes that can be added 
                         // to the dependency list before discarding 
                         // stale nodes
    dc_ui32  val;        // value of cell
};


#ifdef DC_PROFILE
// profiling info associated with constraint objects
typedef struct {
    unsigned long long num_exec; // number of executions of the cons
    unsigned long long session;  // session of the latest execution
                                 // of the constraint
} dc_cons_prof;
#endif


// constraint object
// size: 16 bytes (32-bit platform), 28 bytes (64-bit platform)
// note: we use the bit-stealing technique to pack the group pointer and
//       final flags within the composite field
// note: we cannot use the bit-stealing technique in the first field,
//       as we cannot assume that the user-defined parameter is always a 
//       pointer aligned to 32-bit boundaries
struct dc_cons {
    void*     param_next;   // constraint's user-defined parameter if
                            // the object is in use, pointer to the
                            // next free constraint object otherwise
                            // (*must be the 1st field*)
    dc_ui32   time_stamp;   // time of the latest execution of the
                            // constraint
                            // (*must be the 2nd field*)
    dc_ptr    c_comp;       // composite field
                            // - bits 63/31:2 = group pointer / 4
                            // - bits 1:0 = flags:
                            //      10: object is group
                            //      00: object is constraint and no
                            //          final handler is scheduled
                            //      01: object is constraint, a final
                            //          handler is scheduled and not
                            //          cancelled
                            //      11: object is constraint, a final
                            //          handler is scheduled and
                            //          cancelled
    dc_cons_f fun;          // constraint function pointer

    #ifdef DC_PROFILE
    dc_cons_prof prof;
    #endif
};


// group object
// size: 16 bytes (32-bit platform), 24 bytes (64-bit platform)
struct dc_group {
    void*         sched_obj;    // group scheduler instance
                                // (*must be 1st field*)
    dc_ui32       time_stamp;   // time of the latest execution of a
                                // constraint in the group
                                // (*must be 2nd field*)
    dc_ui32       g_comp;       // composite field:
                                // - bits 31:3 = number of constraints 
                                //               in the group
                                //               (29 bits, max 2^{29}-1)
                                // - bit 2 = tells whether self-trig-
                                //           gering is enabled (1) or 
                                //           disabled (0) for 
                                //           constraints in the group
                                // - bits 1:0 = always set to 10
    dc_scheduler_type* 
                  sched_class;  // group scheduler class
};


// common object prefix for constraint and group objects
// warning: since the composite field has pointer-size in dc_cons and
//          is a 32-bit word in dc_group, we assume a 32-bit platform or 
//          a little-endian platform so that the two least significant
//          bits, which are used to tell whether an object is a 
//          constraint or a group, are always in the lowest-address byte 
//          of the field
typedef struct dc_cons_group dc_cons_group;
struct dc_cons_group {
    void*   ptr;            // object-dependent pointer field
    dc_ui32 time_stamp;     // time of the latest execution of
                            // constraint or group
    dc_ui32 cg_comp;        // composite field
                            // - bits 31:2 = object-dependent
                            // - bits  1:0 = 
                            //     10:         object is group
                            //     00, 01, 11: object is constraint
};


// final handler object
// size: 8 bytes (32-bit platform), 16 bytes (64-bit platform)
typedef struct dc_final dc_final;
struct dc_final {
    dc_cons_f final;           // final function    
    dc_cons*  cons;            // constraint
};


// node of list of new constraints scheduled for first execution
// size: 8 bytes (32-bit platform), 16 bytes (64-bit platform)
typedef struct dc_init_cons dc_init_cons;
struct dc_init_cons {
    dc_cons*      cons;  // newly created constraint
    dc_init_cons* next;  // next node in list
};


// profiling auxiliary data
typedef struct {

    // flag needed for counting the number of sessions
    int tentative_session;

} dc_profile_auxdata;


// private functions
int    _dc_init_patch_table(
            void* patch_table[rm_patch_num][rm_size_num], 
            size_t size_table[rm_patch_num][rm_size_num]);

void   _dc_init_profiler();
void   _dc_read_handler(dc_ptr* shadow_rec)  __attribute__((cdecl));
void   _dc_write_handler(dc_ptr* shadow_rec) __attribute__((cdecl));
void   _exec_new_cons(dc_cons* cons);

void   _dc_add_to_message_indent(int delta);
void   _dc_set_message_indent(size_t indent);
size_t _dc_get_message_indent();
void   _dc_vfmessage(FILE* fp, char* msg, va_list args);
void   _dc_fmessage(FILE* fp, char* msg, ...);
void   _dc_message(char* msg, ...);
void   _dc_panic(char* msg, ...);

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
