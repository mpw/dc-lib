// =====================================================================
//  dc/include/dc_inspect.h
// =====================================================================

//  Author:         (C) 2010-2011 Camil Demetrescu
//  License:        See the end of this file for license information
//  Created:        January 3, 2010
//  Module:         dc

//  Last changed:   $Date: 2011/02/07 12:52:21 $
//  Changed by:     $Author: demetres $
//  Revision:       $Revision: 1.5 $


#ifndef __dc_debug__
#define __dc_debug__

#include "dc.h"
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif


// ---------------------------------------------------------------------
//  dc_fprintf
// ---------------------------------------------------------------------
/** Print message to output stream prefixed by current execution 
 *  context. The execution context may be either "[dc: normal]" if 
 *  dc_printf() is called in normal execution mode, or 
 *  "[dc: cons t @ x]" if dc_printf() is called during the t-th 
 *  execution of a constraint, which happens to be constraint x.
 * @param fp file pointer
 * @param fmt format string with the same conventions of the printf
 *            family, followed by a variable optional number of 
 *            arguments
*/
void dc_fprintf(FILE* fp, char* fmt, ...);


// ---------------------------------------------------------------------
//  dc_printf
// ---------------------------------------------------------------------
/** Print message to stdout prefixed by current execution context. 
 *  Similar to dc_fprintf(), except that dc_printf() prints to stdout.
 * @param fmt format string with the same conventions of the printf
 *            family, followed by a variable optional number of 
 *            arguments
*/
void dc_printf(char* fmt, ...);


// ---------------------------------------------------------------------
//  dc_is_scheduled
// ---------------------------------------------------------------------
/** Check if a constraint is currently scheduled for execution.
    @param cons constraint
    @return non-zero, if the constraint is scheduled, zero otherwise
*/
int dc_is_scheduled(dc_cons* cons);


// ---------------------------------------------------------------------
//  dc_get_num_cons
// ---------------------------------------------------------------------
/** Get current number of constraints.
    Runs in O(p*(1+1/DC_MEM_POOL_PAGE_SIZE)-n) time, where p is the peak
    number of allocated constraints and n is the current number of 
    constraints returned by this function. DC_MEM_POOL_PAGE_SIZE is a
    constant defined in dc_defs.h (currently 1000).
    @return current number of constraints
*/
size_t dc_get_num_cons();


// ---------------------------------------------------------------------
//  dc_dump_wr_cell_list
// ---------------------------------------------------------------------
/** Dump to file the list of reactive cells written during the current 
    constraint execution.
    @param fp file descriptor
*/
void dc_dump_wr_cell_list(FILE* fp);


// ---------------------------------------------------------------------
//  dc_dump_rd_cell_list
// ---------------------------------------------------------------------
/** Dump to file the list of reactive cells read during the current 
    constraint execution.
    @param fp file descriptor
*/
void dc_dump_rd_cell_list(FILE* fp);


// ---------------------------------------------------------------------
//  dc_dump_cell
// ---------------------------------------------------------------------
/** Dump to file cell info. If the cell is reactive, the operation
    dumps all information kept by DC about the cell.
    @param fp file descriptor
    @param addr address of cell
*/
void dc_dump_cell(FILE* fp, void* addr);


// ---------------------------------------------------------------------
//  dc_dump_cell_list
// ---------------------------------------------------------------------
/** Dump to file list of all reactive cells ever accessed since program
    startup.
    @param fp file descriptor
*/
void dc_dump_cell_list(FILE* fp);


// ---------------------------------------------------------------------
//  dc_dump_cons_list
// ---------------------------------------------------------------------
/** Dump to file list of all currently allocated constraints.
    @param fp file descriptor
*/
void dc_dump_cons_list(FILE* fp);


// ---------------------------------------------------------------------
//  dc_dump_cons
// ---------------------------------------------------------------------
/** Dump to file constraint information.
    @param fp file descriptor
    @param cons constraint
*/
void dc_dump_cons(FILE* fp, dc_cons* cons);


// ---------------------------------------------------------------------
//  dc_dump_group_list
// ---------------------------------------------------------------------
/** Dump to file list of all currently allocated groups.
    @param fp file descriptor
*/
void dc_dump_group_list(FILE* fp);


// ---------------------------------------------------------------------
//  dc_dump_group
// ---------------------------------------------------------------------
/** Dump to file group information.
    @param fp file descriptor
    @param group group
*/
void dc_dump_group(FILE* fp, dc_group* group);


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
