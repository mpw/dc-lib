// =====================================================================
//  dc/src/dc_inspect.c
// =====================================================================

//  Author:         (C) 2011 Camil Demetrescu
//  License:        See the end of this file for license information
//  Created:        January 3, 2011
//  Module:         dc

//  Last changed:   $Date: 2011/02/07 12:52:21 $
//  Changed by:     $Author: demetres $
//  Revision:       $Revision: 1.8 $


#include "dc_inspect.h"
#include "dc_globals.h"


// private functions
static void _dc_dump_cell(FILE* fp, void* addr, dc_ptr* shadow_rec);
static void _dc_vfprintf(FILE* fp, char* fmt, va_list args);


// ---------------------------------------------------------------------
//  _dc_vfprintf
// ---------------------------------------------------------------------
void _dc_vfprintf(FILE* fp, char* fmt, va_list args) {
    dc_cons* curr_cons = dc_get_curr_cons();
    if (curr_cons == NULL)
         fprintf(stdout, "[dc | normal] ");
    else fprintf(stdout, "[dc | cons %u @ %p] ",
                 curr_cons->time_stamp, curr_cons);
	vfprintf(fp, fmt, args);
}


// ---------------------------------------------------------------------
//  dc_fprintf
// ---------------------------------------------------------------------
void dc_fprintf(FILE* fp, char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    _dc_vfprintf(fp, fmt, args);
    va_end(args);
}


// ---------------------------------------------------------------------
//  dc_printf
// ---------------------------------------------------------------------
void dc_printf(char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    _dc_vfprintf(stdout, fmt, args);
    va_end(args);
}


// ---------------------------------------------------------------------
//  dc_is_scheduled
// ---------------------------------------------------------------------
int dc_is_scheduled(dc_cons* cons) {
    return !_dc_is_unscheduled(cons);
}


// ---------------------------------------------------------------------
//  dc_get_num_cons
// ---------------------------------------------------------------------
size_t dc_get_num_cons() {
    return pool_get_num_used_blocks(
                dc_cons_pool_g, dc_cons_pool_free_list_g);
}


// ---------------------------------------------------------------------
//  dc_dump_wr_cell_list
// ---------------------------------------------------------------------
void dc_dump_wr_cell_list(FILE* fp) {

    _dc_fmessage(fp, 
        "list of cells written during current atomic block:");

    _dc_add_to_message_indent(_DC_MSG_TAB_SIZE);
 
    size_t i;
    for (i = 0; i < dc_wr_cells_count_g; ++i)
        _dc_fmessage(fp, "addr %p", 
            rm_get_addr_fast(dc_wr_cells_g[i], dc_ui32, dc_ptr));
 
    _dc_add_to_message_indent(-_DC_MSG_TAB_SIZE);
}


// ---------------------------------------------------------------------
//  dc_dump_rd_cell_list
// ---------------------------------------------------------------------
void dc_dump_rd_cell_list(FILE* fp) {

    _dc_fmessage(fp, 
        "list of addresses of reactive cells read during "
        "current constraint execution:");

    _dc_add_to_message_indent(_DC_MSG_TAB_SIZE);
 
    size_t i;
    for (i = 0; i < dc_rd_cells_count_g; ++i)
        _dc_fmessage(fp, "addr %p", 
            rm_get_addr_fast(dc_rd_cells_g[i], dc_ui32, dc_ptr));
 
    _dc_add_to_message_indent(-_DC_MSG_TAB_SIZE);
}


// ---------------------------------------------------------------------
//  dc_dump_cell
// ---------------------------------------------------------------------
void dc_dump_cell(FILE* fp, void* addr) {
    if (rm_is_reactive(addr))
         _dc_dump_cell(fp, addr,
            (dc_ptr*)rm_get_shadow_rec_fast(addr, dc_ui32, dc_ptr));
    else _dc_dump_cell(fp, addr, NULL);
}


// ---------------------------------------------------------------------
//  dc_dump_cell_list
// ---------------------------------------------------------------------
void dc_dump_cell_list(FILE* fp) {

    void* shadow_rec_ptr;

    _dc_fmessage(fp, "list of reactive cells accessed:");

    _dc_add_to_message_indent(_DC_MSG_TAB_SIZE);

    // iterate over reactive locations
    void* addr = rm_get_first_word(&shadow_rec_ptr);
    while (addr != NULL) {

        // skip reactive cells that have never been accessed
        if (*(dc_ptr*)shadow_rec_ptr != 0) 
            _dc_dump_cell(fp, addr, shadow_rec_ptr);

        // get next reactive address
        addr = rm_get_next_word(addr, &shadow_rec_ptr);
    }

    _dc_add_to_message_indent(-_DC_MSG_TAB_SIZE);
}


// ---------------------------------------------------------------------
//  dc_dump_cons_list
// ---------------------------------------------------------------------
void dc_dump_cons_list(FILE* fp) {

    dc_cons* cons;

    _dc_fmessage(fp, "list of constraints:");

    // create iterator
    pool_iterator_t* it = 
        pool_iterator_new(dc_cons_pool_g, dc_cons_pool_free_list_g);
    if (it == NULL) _dc_panic("dc_dump_cons_list failed");

    _dc_add_to_message_indent(_DC_MSG_TAB_SIZE);

    // iterate over constraints
    while ((cons = (dc_cons*)pool_iterator_next_block(it)) != NULL)
        dc_dump_cons(fp, cons);

    _dc_add_to_message_indent(-_DC_MSG_TAB_SIZE);

    // delete iterator
    pool_iterator_delete(it);
}


// ---------------------------------------------------------------------
//  dc_dump_cons
// ---------------------------------------------------------------------
void dc_dump_cons(FILE* fp, dc_cons* cons) {
    unsigned fflags = _dc_get_fflags(cons);
    _dc_fmessage(fp, 
        "cons %p: [fun=%p | param=%p | group=%p | final flags=%s | "
                  "time_stamp=%lu]",
        cons,
        cons->fun, 
        cons->param_next,
        _dc_get_group(cons),
        fflags == DC_CONS_FINAL_UNSCHED ? "unsched" :
        fflags == DC_CONS_FINAL_SCHED   ? "sched  " :
        fflags == DC_CONS_FINAL_CANC    ? "canc   " :
                                         "error  " ,
        cons->time_stamp
    );
}


// ---------------------------------------------------------------------
//  dc_dump_group_list
// ---------------------------------------------------------------------
void dc_dump_group_list(FILE* fp) {

    dc_group* group;

    _dc_fmessage(fp, "list of groups:");

    // create iterator
    pool_iterator_t* it = 
        pool_iterator_new(dc_group_pool_g, dc_group_pool_free_list_g);
    if (it == NULL) _dc_panic("dc_dump_group_list failed");

    _dc_add_to_message_indent(_DC_MSG_TAB_SIZE);

    // iterate over groups
    while ((group = (dc_group*)pool_iterator_next_block(it)) != NULL)
        dc_dump_group(fp, group);

    _dc_add_to_message_indent(-_DC_MSG_TAB_SIZE);

    // delete iterator
    pool_iterator_delete(it);
}


// ---------------------------------------------------------------------
//  dc_dump_group
// ---------------------------------------------------------------------
void dc_dump_group(FILE* fp, dc_group* group) {
    _dc_fmessage(fp, 
        "group %p: [flag=%s | cons_count=%lu | time_stamp=%lu | "
                   "sched_class=%p | sched_obj=%p]",
        group,
        _dc_get_self_trigger(group) ? 
            "self-trigger   " :
            "no self-trigger" ,
        _dc_get_cons_count(group),
        group->time_stamp,
        group->sched_class,
        group->sched_obj
    );
}


// ---------------------------------------------------------------------
//  _dc_dump_cell
// ---------------------------------------------------------------------
void _dc_dump_cell(FILE* fp, void* addr, dc_ptr* shadow_rec) {

    if (shadow_rec == NULL) {
        _dc_fmessage(fp, "addr %p: (not reactive)", addr);
        return;
    }

    int      flags = (*shadow_rec) & 0x3;
    dc_cell* cell  = (dc_cell*)((*shadow_rec) & ~0x3);

    if (cell == NULL) {
        _dc_fmessage(fp, "addr %p: [shadow_rec=%p] (never accessed)",
             addr, shadow_rec);
        return;
    }

    _dc_fmessage(fp, "addr %p: [shadow_rec=%p, cell_rec=%p, "
                     "flags=%s, val=%ld, countdown=%lu]",
         addr,
         shadow_rec,
         cell,
         flags == DC_UNTOUCHED_CELL ? "00(UT)" : 
         flags == DC_READ_CELL      ? "01(RD)" : 
         flags == DC_WRITTEN_CELL   ? "11(WR)" : 
                                      "ERROR " ,
         cell->val,
         cell->countdown
    );

    _dc_add_to_message_indent(_DC_MSG_TAB_SIZE);

    dc_dep* dep;
    for (dep = cell->first; dep != NULL; dep = dep->next) {
        _dc_fmessage(fp, "dep %p: [cons=%p, time_stamp=%lu (%s)]",
            dep,
            dep->cons,
            dep->time_stamp,
            dep->time_stamp < dep->cons->time_stamp ? 
                "stale" : "up to date"
        );
    }

    _dc_add_to_message_indent(-_DC_MSG_TAB_SIZE);
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
