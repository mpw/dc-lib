// =====================================================================
//  dc/include/dc_globals.h
// =====================================================================

//  Author:         (C) 2010-2011 Camil Demetrescu
//  License:        See the end of this file for license information
//  Created:        December 29, 2010
//  Module:         dc

//  Last changed:   $Date: 2011/04/02 14:14:42 $
//  Changed by:     $Author: demetres $
//  Revision:       $Revision: 1.18 $


#ifndef __dc_globals__
#define __dc_globals__

#include "dc_defs.h"
#include "pool.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __dc_extern__
#define __dc_extern__ extern
#endif


// ---------------------------------------------------------------------
//  global vars
// ---------------------------------------------------------------------

// global time counter, increased by one at each constraint execution
__dc_extern__ dc_ui32 dc_time_g;

// pointer to currently executed constraint (NULL in normal exec. mode);
// the variable is also used to determine the current execution mode
// ("normal" if dc_curr_cons_g == NULL, and "constraint" otherwise)
__dc_extern__ dc_cons* dc_curr_cons_g;

// number of currently open no-log blocks
// in "normal" execution mode, the counter is 1
__dc_extern__ dc_ui32  dc_no_log_blocks_count_g;

// pointer to currently executed group (NULL in normal exec. mode);
__dc_extern__ dc_group* dc_curr_group_g;

// number of currently open atomic blocks
__dc_extern__ dc_ui32 dc_atomic_blocks_count_g;

// flag that tells whether dc has been initialized (1=yes, 0=no)
__dc_extern__ dc_ui32 dc_is_initialized_g;

// pointer to top-level scheduler
__dc_extern__ void* dc_top_level_scheduler_g;

// global self-triggering flag: 1=enabled, 0=disabled
__dc_extern__ dc_ui32 dc_self_trigger_g;


// ---------------------------------------------------------------------
//  global dynamic arrays
// ---------------------------------------------------------------------

// array of scheduled final handlers:
//    the array maintains the list of final handlers to be executed
//    at the end of the current constraint solving session or at the
//    end of the current group execution
__dc_extern__ dc_final* dc_sched_final_g;
__dc_extern__ dc_final* dc_sched_group_final_g;

// size of array dc_sched_final_g and dc_sched_group_final_g
__dc_extern__ dc_ui32 dc_sched_final_size_g;
__dc_extern__ dc_ui32 dc_sched_group_final_size_g;

// number of final handlers scheduled in dc_sched_final_g and
// dc_sched_group_final_g
__dc_extern__ dc_ui32 dc_sched_final_count_g;
__dc_extern__ dc_ui32 dc_sched_group_final_count_g;

// array of addresses of read/written cells
//    the arrays maintain the list of addresses of reactive cells
//    accessed during the execution of the current constraint (read +
//    write) or atomic block (write)
__dc_extern__ dc_ptr** dc_rd_cells_g;
__dc_extern__ dc_ptr** dc_wr_cells_g;

// array of initial values of written cells (same size as dc_wr_cells_g)
//    the array maintains the initial values of all cells written
//    during the execution of the current constraint. At the end of
//    the execution of the constraint, the values in this array are
//    compared againts the values currently in reactive memory to check
//    which ones have actually changed by the execution of the 
//    constraint.
__dc_extern__ dc_ui32* dc_wr_cells_val_g;

// sizes of arrays dc_rd_cells_g and dc_wr_cells_g/dc_wr_cells_val_g
__dc_extern__ dc_ui32 dc_rd_cells_size_g;
__dc_extern__ dc_ui32 dc_wr_cells_size_g;

// number of read/written cells in dc_rd_cells_g and dc_wr_cells_g
// (these counters are zero at the beginning of each constraint
// execution)
__dc_extern__ dc_ui32 dc_rd_cells_count_g;
__dc_extern__ dc_ui32 dc_wr_cells_count_g;


// ---------------------------------------------------------------------
//  global linked lists
// ---------------------------------------------------------------------

// head and tail of temporary list of newly created constraints;
// the list contains all constraints created during the
// execution of the current constraint
__dc_extern__ dc_init_cons* dc_init_cons_temp_list_head_g;
__dc_extern__ dc_init_cons* dc_init_cons_temp_list_tail_g;

// head of the list of newly created constraints to be executed
__dc_extern__ dc_init_cons* dc_init_cons_list_head_g;


// ---------------------------------------------------------------------
//  global storage pools for dynamically-allocated objects
// ---------------------------------------------------------------------

// storage pool for newly created constraint nodes
__dc_extern__ pool_t* dc_init_cons_pool_g;
__dc_extern__ void*   dc_init_cons_pool_free_list_g;

// storage pool for group objects
__dc_extern__ pool_t* dc_group_pool_g;
__dc_extern__ void*   dc_group_pool_free_list_g;

// storage pool for constraint objects
__dc_extern__ pool_t* dc_cons_pool_g;
__dc_extern__ void*   dc_cons_pool_free_list_g;

// storage pool for cell info objects
__dc_extern__ pool_t* dc_cell_pool_g;
__dc_extern__ void*   dc_cell_pool_free_list_g;

// storage pool for dependency objects
__dc_extern__ pool_t* dc_dep_pool_g;
__dc_extern__ void*   dc_dep_pool_free_list_g;


// ---------------------------------------------------------------------
//  profiling info
// ---------------------------------------------------------------------

__dc_extern__ dc_profile dc_curr_profile;
__dc_extern__ dc_profile_auxdata dc_curr_profile_auxdata;

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
