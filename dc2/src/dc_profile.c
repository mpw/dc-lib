// =====================================================================
//  dc/src/dc_profile.c
// =====================================================================

//  Author:         (C) 2011 Camil Demetrescu
//  License:        See the end of this file for license information
//  Created:        February 5, 2011
//  Module:         dc

//  Last changed:   $Date: 2011/03/19 12:35:11 $
//  Changed by:     $Author: demetres $
//  Revision:       $Revision: 1.23 $


#include "dc_globals.h"
#include "dc_profile.h"
#include "dc_inspect.h"
#include "profile.h"
#include "pool.h"


// ---------------------------------------------------------------------
//  dc_profile_on
// ---------------------------------------------------------------------
int dc_profile_on() {
    #if DC_PROFILE
    return 1;
    #else
    return 0;
    #endif
}


// ---------------------------------------------------------------------
//  dc_fetch_profile_info
// ---------------------------------------------------------------------
void dc_fetch_profile_info(dc_profile* buffer) {

    #ifdef DC_PROFILE

    // copy profiling record
    *buffer = dc_curr_profile;

    // complete profiling record
    buffer->vm_peak = get_vm_peak();
    buffer->rmem_size = rm_heap_alloc_bytes() / 1024;
    buffer->cons_peak_size = 
        pool_get_num_blocks(dc_cons_pool_g)*
            (sizeof(dc_cons)-sizeof(dc_cons_prof))/1024;
    buffer->dep_peak_size = 
        pool_get_num_blocks(dc_dep_pool_g)*sizeof(dc_dep)/1024;
    buffer->cell_peak_size = 
        pool_get_num_blocks(dc_cell_pool_g)*sizeof(dc_cell)/1024;

    buffer->dep_peak_num  = pool_get_num_blocks(dc_dep_pool_g);
    buffer->cell_peak_num = pool_get_num_blocks(dc_cell_pool_g);

    // iterate over dependency objects
    buffer->dep_num = 0;
    buffer->stale_dep_num = 0;
    pool_iterator_t* it = 
        pool_iterator_new(dc_dep_pool_g, dc_dep_pool_free_list_g);
    if (it == NULL) _dc_panic("dc_fetch_profile_info failed");
    dc_dep* dep;
    while ((dep = (dc_dep*)pool_iterator_next_block(it)) != NULL) {
        buffer->dep_num++;
        if (!_dc_is_up_to_date(dep)) buffer->stale_dep_num++;
    }
    pool_iterator_delete(it);
    #else
    _dc_panic("dc_fetch_profile_info() called with DC_PROFILE macro "
              "undefined");
    #endif
}


// ---------------------------------------------------------------------
//  dc_get_cons_exec
// ---------------------------------------------------------------------
unsigned long long dc_get_cons_exec(dc_cons* cons) {
    #ifdef DC_PROFILE
    return cons->prof.num_exec;
    #else
    return 0;
    #endif
}


// ---------------------------------------------------------------------
//  dc_dump_profile_diff
// ---------------------------------------------------------------------
void dc_dump_profile_diff(FILE* fp, 
                          dc_profile* start, dc_profile* end) {

    unsigned long long num_solver_sessions_diff = 
        end->num_solver_sessions - start->num_solver_sessions;
    unsigned long long num_exec_cons_diff = 
        end->num_exec_cons - start->num_exec_cons;
    unsigned long long num_distinct_exec_cons_diff = 
        end->num_distinct_exec_cons - start->num_distinct_exec_cons;
    unsigned long long cons_peak_num_diff = 
        end->cons_peak_num - start->cons_peak_num;
    unsigned long long num_new_cons_diff = 
        end->num_new_cons - start->num_new_cons;
    unsigned long long num_del_cons_diff = 
        end->num_del_cons - start->num_del_cons;
    unsigned long long num_first_read_diff = 
        end->num_first_read - start->num_first_read;
    unsigned long long num_first_write_diff = 
        end->num_first_write - start->num_first_write;
    unsigned long long peak_num_reactive_blocks_diff =
        end->peak_num_reactive_blocks - start->peak_num_reactive_blocks;
    unsigned long long num_alloc_diff = 
        end->num_alloc - start->num_alloc;
    unsigned long long num_free_diff = 
        end->num_free - start->num_free;
    unsigned long vm_peak_diff = 
        end->vm_peak - start->vm_peak;
    unsigned long rmem_size_diff = 
        end->rmem_size - start->rmem_size;

    dc_printf("--number of solver sessions in window: %llu\n", 
        num_solver_sessions_diff);

    // dump constraint executions stats
    dc_printf("--number of executed cons in window: %llu\n", 
        num_exec_cons_diff);
    dc_printf("  number of distinct executed cons in window: %llu\n",
        num_distinct_exec_cons_diff);
    dc_printf("  average number of cons exec per session "
             "in window: %.2f\n",
        num_solver_sessions_diff == 0 ? 
            0.0 : (double)num_exec_cons_diff / 
                          num_solver_sessions_diff);
    dc_printf("  average number of cons re-exec per session "
              "in window: %.2f\n",
        num_solver_sessions_diff == 0 ? 
            0.0 : (double)(num_exec_cons_diff - 
                    num_distinct_exec_cons_diff) / 
                           num_solver_sessions_diff);

    // dump constraints stats
    dc_printf("--peak number of cons: %llu (+%llu in window)\n", 
        end->cons_peak_num, cons_peak_num_diff);
    dc_printf("  total number of cons: %llu (%s%lld in window)\n", 
        end->num_new_cons - end->num_del_cons, 
        num_new_cons_diff >= num_del_cons_diff ? "+" : "", 
        (long long)num_new_cons_diff - num_del_cons_diff);
    dc_printf("  number of created cons in window: %llu\n", 
        num_new_cons_diff);
    dc_printf("  number of deleted cons in window: %llu\n", 
        num_del_cons_diff);

    // dump reactive blocks stats
    dc_printf("--peak number of reactive blocks: "
              "%llu (+%llu in window)\n", 
        end->peak_num_reactive_blocks, peak_num_reactive_blocks_diff);
    dc_printf("  total number of allocated reactive blocks: "
              "%llu (%s%lld in window)\n", 
        end->num_alloc - end->num_free, 
        num_alloc_diff >= num_free_diff ? "+" : "", 
        (long long)num_alloc_diff - num_free_diff);
    dc_printf("  number of reactive blocks allocations "
              "in window: %llu\n", 
        num_alloc_diff);
    dc_printf("  number of reactive blocks deallocations "
              "in window: %llu\n", 
        num_free_diff);

    // dump reactive memory access stats
    dc_printf("--number of first read ops in window: %llu\n", 
        num_first_read_diff);

    dc_printf("  number of first write ops in window: %llu\n", 
        num_first_write_diff);

    // dump memory size stats
    dc_printf("--virtual memory peak: %lu KB (+%lu KB in window)\n", 
        end->vm_peak, vm_peak_diff);

    dc_printf("--reactive memory size: %lu KB (+%lu KB in window)\n", 
        end->rmem_size, rmem_size_diff);

    dc_printf("--cons peak size: %lu KB (+%lu KB in window)\n", 
        end->cons_peak_size, 
        end->cons_peak_size - start->cons_peak_size);

    dc_printf("--cell peak size: %lu KB (+%lu KB in window)\n", 
        end->cell_peak_size, 
        end->cell_peak_size - start->cell_peak_size);

    dc_printf("--dep peak size: %lu KB (+%lu KB in window)\n", 
        end->dep_peak_size, 
        end->dep_peak_size - start->dep_peak_size);

    dc_printf("--cell peak num: %lu (+%lu in window)\n", 
        end->cell_peak_num,
        end->cell_peak_num - start->cell_peak_num);

    dc_printf("--dep peak num: %lu (+%lu in window)\n", 
        end->dep_peak_num, 
        end->dep_peak_num - start->dep_peak_num);

    dc_printf("--dep num: %lu (%s%ld in window)\n", 
        end->dep_num, 
        (end->dep_num >= start->dep_num) ? "+":"",
        end->dep_num - start->dep_num);

    dc_printf("--stale dep num: %lu (%s%ld in window)\n", 
        end->stale_dep_num, 
        (end->stale_dep_num >= start->stale_dep_num) ? "+":"",
        end->stale_dep_num - start->stale_dep_num);

    dc_printf("--stale cleanup num: %llu (+%llu in window)\n", 
        end->stale_cleanup_num, 
        end->stale_cleanup_num - start->stale_cleanup_num);


    // dump sampling info
    dc_printf("--normal samp: %llu\n", 
        end->normal_samples   - start->normal_samples);

    dc_printf("--solver samp: %llu\n", 
        end->solver_samples   - start->solver_samples);

    dc_printf("--cons exec samp: %llu\n", 
        end->cons_exec_samples   - start->cons_exec_samples);
}


// ---------------------------------------------------------------------
//  dc_profile_diff
// ---------------------------------------------------------------------
void dc_profile_diff(dc_profile* start, 
                     dc_profile* end, 
                     dc_profile* diff) {

    #define d(f) diff->f = end->f - start->f
    d(num_solver_sessions);
    d(num_exec_cons);
    d(num_distinct_exec_cons);
    d(cons_peak_size);
    d(cons_peak_num);
    d(num_new_cons);
    d(num_del_cons);
    d(num_first_read);
    d(num_first_write);
    d(peak_num_reactive_blocks);
    d(num_alloc);
    d(num_free);
    d(vm_peak);
    d(rmem_size);
    d(cell_peak_size);
    d(cell_peak_num);
    d(dep_peak_size);
    d(dep_peak_num);
    d(dep_num);
    d(stale_dep_num);
    d(stale_cleanup_num);
    d(normal_samples);
    d(solver_samples);
    d(cons_exec_samples);
    #undef d
}


// ---------------------------------------------------------------------
//  _dc_init_profiler
// ---------------------------------------------------------------------
void _dc_init_profiler() {
}


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
