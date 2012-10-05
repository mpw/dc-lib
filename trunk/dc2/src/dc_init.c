// =====================================================================
//  dc/src/dc_init.c
// =====================================================================

//  Author:         (C) 2010-2011 Camil Demetrescu
//  License:        See the end of this file for license information
//  Created:        December 30, 2010
//  Module:         dc

//  Last changed:   $Date: 2011/03/03 16:36:23 $
//  Changed by:     $Author: demetres $
//  Revision:       $Revision: 1.16 $


#include "dc_globals.h"


// private function prototypes
static int  _dc_init_pools();
static void _dc_cleanup_pools();
static int  _dc_init_arrays();
static void _dc_cleanup_arrays();
static int  _dc_init_top_level_sched();
static void _dc_cleanup_top_level_sched();
static int  _dc_init_rm();


// ---------------------------------------------------------------------
//  dc_init
// ---------------------------------------------------------------------
int dc_init() {

    int res;

    // skip if DC has already been initialized
    if (dc_is_initialized_g) return 0;

    // initialize pools of dynamically allocated objects
    if ((res = _dc_init_pools()) != 0) return res;

    // initialize dynamic arrays
    if ((res = _dc_init_arrays()) != 0) goto cleanup;
    
    // initialize top-level scheduler
    if ((res = _dc_init_top_level_sched()) != 0) goto cleanup;

    // initialize reactive memory manager
    if ((res = _dc_init_rm()) != 0) goto cleanup;

    // init no-log blocks counter
    dc_no_log_blocks_count_g = 1;

    // init DC profiler
    #ifdef DC_PROFILE
    _dc_init_profiler();
    #endif

    // flag DC was initialized
    dc_is_initialized_g = 1;

    return 0;

  cleanup:
    _dc_cleanup_pools();
    _dc_cleanup_arrays();
    _dc_cleanup_top_level_sched();

    #if DC_DEBUG == 1
    _dc_panic("dc_init failed");
    #endif

    return res;
}


// ---------------------------------------------------------------------
//  _dc_init_pools
// ---------------------------------------------------------------------
int _dc_init_pools() {

    // init storage pool for newly created constraint nodes
    dc_init_cons_pool_g = 
        pool_init(DC_MEM_POOL_PAGE_SIZE, 
                  sizeof(dc_init_cons), 
                  &dc_init_cons_pool_free_list_g);
    if (dc_init_cons_pool_g == NULL) goto cleanup;

    // init storage pool for group objects
    dc_group_pool_g = 
        pool_init(DC_MEM_POOL_PAGE_SIZE, 
                  sizeof(dc_group), 
                  &dc_group_pool_free_list_g);
    if (dc_group_pool_g == NULL) goto cleanup;

    // init storage pool for constraint objects
    dc_cons_pool_g = 
        pool_init(DC_MEM_POOL_PAGE_SIZE, 
                  sizeof(dc_cons), 
                  &dc_cons_pool_free_list_g);
    if (dc_cons_pool_g == NULL) goto cleanup;

    // init storage pool for cell info objects
    dc_cell_pool_g = 
        pool_init(DC_MEM_POOL_PAGE_SIZE, 
                  sizeof(dc_cell), 
                  &dc_cell_pool_free_list_g);
    if (dc_cell_pool_g == NULL) goto cleanup;

    // init storage pool for dependency objects
    dc_dep_pool_g = 
        pool_init(DC_MEM_POOL_PAGE_SIZE, 
                  sizeof(dc_cons), 
                  &dc_dep_pool_free_list_g);
    if (dc_dep_pool_g == NULL) goto cleanup;

    return 0;

  cleanup:
    _dc_cleanup_pools();
    return -1;
}


// ---------------------------------------------------------------------
//  _dc_cleanup_pools
// ---------------------------------------------------------------------
void _dc_cleanup_pools() {

    if (dc_init_cons_pool_g != NULL)
        pool_cleanup(dc_init_cons_pool_g);

    if (dc_group_pool_g != NULL)
        pool_cleanup(dc_group_pool_g);

    if (dc_cons_pool_g != NULL)
        pool_cleanup(dc_cons_pool_g);

    if (dc_cell_pool_g != NULL)
        pool_cleanup(dc_cell_pool_g);

    if (dc_dep_pool_g != NULL)
        pool_cleanup(dc_dep_pool_g);

    dc_init_cons_pool_g = dc_group_pool_g = 
        dc_cons_pool_g = dc_cell_pool_g = 
            dc_dep_pool_g = NULL;
}


// ---------------------------------------------------------------------
//  _dc_init_arrays
// ---------------------------------------------------------------------
int _dc_init_arrays() {

    // setup initial sizes
    dc_sched_final_size_g = dc_sched_group_final_size_g = 
        DC_SCHED_FINAL_ARRAY_SIZE;
    dc_rd_cells_size_g = dc_wr_cells_size_g =
        DC_READ_WRITTEN_CELL_ARRAY_SIZE;

    // allocate array of scheduled final handlers
    dc_sched_final_g = 
        (dc_final*)malloc(dc_sched_final_size_g * 
                          sizeof(dc_final));
    if (dc_sched_final_g == NULL) goto cleanup;

    // allocate array of scheduled group final handlers
    dc_sched_group_final_g = 
        (dc_final*)malloc(dc_sched_group_final_size_g * 
                          sizeof(dc_final));
    if (dc_sched_group_final_g == NULL) goto cleanup;

    // allocate array of read cells
    dc_rd_cells_g = 
        (dc_ptr**)malloc(dc_rd_cells_size_g * sizeof(dc_ptr*));
    if (dc_rd_cells_g == NULL) goto cleanup;

    // allocate array of written cells
    dc_wr_cells_g = 
        (dc_ptr**)malloc(dc_wr_cells_size_g * sizeof(dc_ptr*));
    if (dc_wr_cells_g == NULL) goto cleanup;

    // allocate array of original values of written cells
    dc_wr_cells_val_g = 
        (dc_ui32*)malloc(dc_wr_cells_size_g * sizeof(dc_ui32));
    if (dc_wr_cells_val_g == NULL) goto cleanup;

    return 0;

  cleanup:
    _dc_cleanup_arrays();
    return -1;
}


// ---------------------------------------------------------------------
//  _dc_cleanup_arrays
// ---------------------------------------------------------------------
void _dc_cleanup_arrays() {

    if (dc_sched_final_g       != NULL) free(dc_sched_final_g);
    if (dc_sched_group_final_g != NULL) free(dc_sched_group_final_g);
    if (dc_rd_cells_g          != NULL) free(dc_rd_cells_g);
    if (dc_wr_cells_g          != NULL) free(dc_wr_cells_g);
    if (dc_wr_cells_val_g      != NULL) free(dc_wr_cells_val_g);

    dc_sched_final_g = dc_sched_group_final_g = NULL;
    dc_rd_cells_g = dc_wr_cells_g = NULL;
    dc_wr_cells_val_g = NULL;
    dc_rd_cells_size_g = dc_wr_cells_size_g = 0;
}


// ---------------------------------------------------------------------
//  _dc_init_top_level_sched
// ---------------------------------------------------------------------
int _dc_init_top_level_sched() {
    dc_top_level_scheduler_g = DC_SCHED_CREATE();
    if (dc_top_level_scheduler_g == NULL) return -1;
    return 0;
}


// ---------------------------------------------------------------------
//  _dc_cleanup_top_level_sched
// ---------------------------------------------------------------------
void _dc_cleanup_top_level_sched() {
    if (dc_top_level_scheduler_g == NULL) return;
    DC_SCHED_DESTROY(dc_top_level_scheduler_g);
    dc_top_level_scheduler_g = NULL;
}


// ---------------------------------------------------------------------
//  _dc_init_rm
// ---------------------------------------------------------------------
int _dc_init_rm() {

    // patch and size tables
    void* patch_table[rm_patch_num][rm_size_num] = { { NULL } };
    size_t size_table[rm_patch_num][rm_size_num] = { { 0 } };

    // init rm patch table
    _dc_init_patch_table(patch_table, size_table);
    
    // initialize rm
    rm_init(NULL, NULL, patch_table, size_table, 
        sizeof(void*), DC_REACTIVE_CELL_SIZE);

    return 0;
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
