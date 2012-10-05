/* ============================================================================
 *  rm_exec_heap.c
 * ============================================================================

 *  Author:         (c) 2010 the rm team
 *  License:        See the end of this file for license information
 *  Created:        July 21, 2010
 *  Note:           executable heap manager

 *  Last changed:   $Date: 2010/08/23 23:28:34 $
 *  Changed by:     $Author: alex $
 *  Revision:       $Revision: 1.1 $
*/

#include "_rm_private.h"

//#include <assert.h>


/* private variables */
static char *rm_exec_brk = 0;
static char *rm_exec_max_addr = 0;
static size_t rm_exec_used_bytes = 0;


/* ----------------------------------------------------------------------------
 *  _rm_exec_heap_used_bytes
 * ----------------------------------------------------------------------------
*/
size_t _rm_exec_heap_used_bytes(void) {
    return rm_exec_used_bytes;
}


/* ----------------------------------------------------------------------------
 *  _rm_exec_sbrk
 * ----------------------------------------------------------------------------
*/
void* _rm_exec_sbrk(int incr) {
    char* old_exec_brk;
    
    if((incr < 0)) {
        _rm_error(rm_NO_MEMORY, "_rm_exec_sbrk: negative increment\n");
        return (void*)-1;
    }
    
    if((rm_exec_brk + incr) > rm_exec_max_addr) {
        unsigned int size_to_add;
        size_to_add = (incr + (getpagesize()-1)) & ~(getpagesize()-1);
        if((rm_exec_brk = (char*)mmap(0, size_to_add, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0)) == MAP_FAILED) {
            _rm_error(rm_NO_MEMORY, "_rm_exec_sbrk: ran out of memory\n");
            return (void*)-1;
        }
        
        rm_exec_max_addr = rm_exec_brk + size_to_add;
        rm_exec_used_bytes += size_to_add;
    }
    
    old_exec_brk = rm_exec_brk;
    rm_exec_brk += incr;
    return (void*)old_exec_brk;
}



/* Copyright (c) 2010 the rm team

 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
