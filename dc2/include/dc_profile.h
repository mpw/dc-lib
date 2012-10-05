// =====================================================================
//  dc/include/dc_profile.h
// =====================================================================

//  Author:         (C) 2010-2011 Camil Demetrescu
//  License:        See the end of this file for license information
//  Created:        February 5, 2011
//  Module:         dc

//  Last changed:   $Date: 2011/03/05 23:10:26 $
//  Changed by:     $Author: demetres $
//  Revision:       $Revision: 1.14 $


#ifndef __dc_profile__
#define __dc_profile__


#include "dc.h"
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif


// ---------------------------------------------------------------------
//  dc_profile
// ---------------------------------------------------------------------
/** Structure specifying global DC profiling information. */
typedef struct {

    /** number of constraints solving sessions. A session starts when
     *  the execution mode switches from normal to constraint, and
     *  terminates when the execution mode of the underlying program
     *  is resumed in normal execution mode */
    unsigned long long num_solver_sessions;

    /** number of constraints executed since program startup */
    unsigned long long num_exec_cons;

    /** number of distinct constraint executions in each constraint
     *  solving session since program startup. This number differs
     *  from num_exec_cons since constraints executed more than once
     *  in the same solving session are only counted once in 
     *  num_distinct_exec_cons */
    unsigned long long num_distinct_exec_cons;

    /** peak space in KB needed to keep constraint objects */
    unsigned long cons_peak_size;

    /** peak number of constraints allocated simultaneously */
    unsigned long long cons_peak_num;

    /** number of constraint creations */
    unsigned long long num_new_cons;

    /** number of constraint deletions */
    unsigned long long num_del_cons;

    /** number of read operations that access for the first time
        a reactive cell during the execution of a constraint */
    unsigned long long num_first_read;

    /** number of write operations that access for the first time
        a reactive cell during the executiong of an atomic block,
        plus number of write operations that access a reactive cell
        in normal non-atomic execution mode */
    unsigned long long num_first_write;

    /** peak number of reactive blocks allocated simultaneously */
    unsigned long long peak_num_reactive_blocks;

    /** number of reactive blocks allocations */
    unsigned long long num_alloc;

    /** number of reactive blocks deallocations */
    unsigned long long num_free;

    /** process virtual memory peak in KB */
    unsigned long vm_peak;

    /** reactive memory size in KB */
    unsigned long rmem_size;

    /** peak space in KB needed for reactive cells bookkeeping */
    unsigned long cell_peak_size;

    /** peak number of reactive cell objects */
    unsigned long cell_peak_num;

    /** peak space in KB needed to keep constraint-cell dependencies */
    unsigned long dep_peak_size;

    /** peak number of cell-constraint dependency objects */
    unsigned long dep_peak_num;

    /** number of cell-constraint dependency objects (both stale and
     *  up to date) */
    unsigned long dep_num;

    /** number of stale cell-constraint dependency objects */
    unsigned long stale_dep_num;

    /** number of cleanup operations of stale dependency objects */
    unsigned long long stale_cleanup_num;

    /** samples taken in normal execution mode */
    unsigned long long normal_samples;

    /** samples taken in solver mode */
    unsigned long long solver_samples;

    /** samples taken in constraint execution mode */
    unsigned long long cons_exec_samples;

} dc_profile;


// ---------------------------------------------------------------------
//  dc_profile_on
// ---------------------------------------------------------------------
/** Check if DC provides profiling information. The flag tells whether
 *  the DC library was compiled with support for profiling 
 *  information. Collecting profiling information incurs some time and
 *  space overhead.
 *  @return non-zero if profiling information is available, 
 *          zero otherwise
*/
int dc_profile_on();


// ---------------------------------------------------------------------
//  dc_fetch_profile_info
// ---------------------------------------------------------------------
/** Copy current profiling info to buffer.
 *  @param buffer pointer to profiling info structure
*/
void dc_fetch_profile_info(dc_profile* buffer);


// ---------------------------------------------------------------------
//  dc_get_cons_exec
// ---------------------------------------------------------------------
/** Get number of executions of a constraint.
 *  @param cons constraint
 *  @return number of times the constraint has been executed since
 *          its creation.
*/
unsigned long long dc_get_cons_exec(dc_cons* cons);


// ---------------------------------------------------------------------
//  dc_profile_diff
// ---------------------------------------------------------------------
/** Compute differences between two profiling snapshots. 
 *  @param start initial profiling snapshot
 *  @param end final profiling snapshot
 *  @param diff field-by-field difference start - end
*/
void dc_profile_diff(dc_profile* start, 
                     dc_profile* end, 
                     dc_profile* diff);


// ---------------------------------------------------------------------
//  dc_dump_profile_diff
// ---------------------------------------------------------------------
/** Dump to output stream differences between two profiling snapshots. 
 *  @param fp file pointer
 *  @param start initial profiling snapshot
 *  @param end final profiling snapshot
*/
void dc_dump_profile_diff(FILE* fp,
                          dc_profile* start,
                          dc_profile* end);

#ifdef __cplusplus
}
#endif

#endif


// Copyright (C) 2011 Camil Demetrescu

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
