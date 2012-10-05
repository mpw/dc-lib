/* ============================================================================
 *  rm_heap.c
 * ============================================================================

 *  Author:         (c) 2010 the rm team
 *  License:        See the end of this file for license information
 *  Created:        July 21, 2010
 *  Note:           reactive heap manager

 *  Last changed:   $Date: 2011/01/31 12:21:06 $
 *  Changed by:     $Author: alex $
 *  Revision:       $Revision: 1.12 $
*/

#include "_rm_private.h"

#include <assert.h>


/* private variables */
static char *rm_start_brk;  /* points to first byte of heap */
static char *rm_brk;        /* points to last byte of heap */
static char *rm_max_addr;   /* largest legal heap address */
static char *rm_max_rec_addr;


/* ----------------------------------------------------------------------------
 *  rm_get_first_word
 * ----------------------------------------------------------------------------
*/
void* rm_get_first_word(void** shadow_rec_ptr) {
    if(rm_brk == (void*)rm_heap_START_BRK) {
        *shadow_rec_ptr = NULL;
        return NULL;
    }
    void* the_first_word = (void*)(rm_heap_START_BRK + rm_OFFSET);
    *shadow_rec_ptr = rm_get_shadow_rec(the_first_word);
    return the_first_word;
}


/* ----------------------------------------------------------------------------
 *  rm_get_next_word
 * ----------------------------------------------------------------------------
*/
void* rm_get_next_word(void* word, void** shadow_rec_ptr) {
    void* the_next_word = word + g_shadow_wordsize;
    if(the_next_word >= (void*)(rm_brk + rm_OFFSET)) {
        *shadow_rec_ptr = NULL;
        return NULL;
    }
    *shadow_rec_ptr = rm_get_shadow_rec(the_next_word);
    return the_next_word;
}


/* ----------------------------------------------------------------------------
 *  rm_heap_alloc_bytes
 * ----------------------------------------------------------------------------
*/
size_t rm_heap_alloc_bytes(void) {
    return (size_t)(rm_max_addr - rm_max_rec_addr);
}


/* ----------------------------------------------------------------------------
 *  rm_heap_alloc_lo
 * ----------------------------------------------------------------------------
*/
void* rm_heap_alloc_lo(void) {
    return (void*)rm_max_rec_addr;
}


/* ----------------------------------------------------------------------------
 *  rm_heap_alloc_hi
 * ----------------------------------------------------------------------------
*/
void* rm_heap_alloc_hi(void) {
    return (void*)(rm_max_addr - 1);
}


/* ----------------------------------------------------------------------------
 *  _rm_heap_init
 * ----------------------------------------------------------------------------
*/
int _rm_heap_init(void) {
    unsigned int the_records_size;
    
    /* allocate the storage we will use to model the available VM */
    if((rm_start_brk = (char*)mmap((void*)rm_heap_START_BRK, rm_heap_START_NUM_OF_PAGES*getpagesize(), PROT_READ|PROT_WRITE, MAP_FIXED|MAP_PRIVATE|MAP_ANONYMOUS, -1, 0)) == MAP_FAILED) {
        _rm_error(rm_NO_MEMORY, "_rm_heap_init: mmap error\n");
        return -1;
    }
    
    rm_max_addr = rm_start_brk + rm_heap_START_NUM_OF_PAGES*getpagesize();	/* max legal heap address */
    rm_brk = rm_start_brk;							/* heap is empty initially */
    
    /* allocate the storage for shadow records */
    the_records_size = rm_heap_START_NUM_OF_PAGES*getpagesize() / g_shadow_wordsize * g_shadow_rec_size;
    if((rm_max_rec_addr = (char*)mmap((void*)rm_heap_START_BRK-the_records_size, the_records_size, PROT_READ|PROT_WRITE, MAP_FIXED|MAP_PRIVATE|MAP_ANONYMOUS, -1, 0)) == MAP_FAILED) {
        _rm_error(rm_NO_MEMORY, "_rm_heap_init: mmap error\n");
        return -1;
    }
    
    return 0;
}


/* ----------------------------------------------------------------------------
 *  _rm_sbrk
 * ----------------------------------------------------------------------------
*/
void* _rm_sbrk(int incr) {
    char* old_brk = rm_brk;
    
    if((incr < 0)) {
        _rm_error(rm_NO_MEMORY, "_rm_sbrk: negative increment\n");
        return (void*)-1;
    }
    
    if((rm_brk + incr) > rm_max_addr) {
        unsigned int size_to_add, the_records_size;
        size_to_add = (((rm_brk + incr) - rm_max_addr) + (getpagesize()-1)) & ~(getpagesize()-1);
        if(mmap(rm_max_addr, size_to_add, PROT_READ|PROT_WRITE, MAP_FIXED|MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) == MAP_FAILED) {
            _rm_error(rm_NO_MEMORY, "_rm_sbrk: ran out of memory\n");
            return (void*)-1;
        }
        
        rm_max_addr += size_to_add;  /* max legal heap address */
        
        the_records_size = size_to_add / g_shadow_wordsize * g_shadow_rec_size;
        if((rm_max_rec_addr = (char *)mmap(rm_max_rec_addr-the_records_size, the_records_size, PROT_READ|PROT_WRITE, MAP_FIXED|MAP_PRIVATE|MAP_ANONYMOUS, -1, 0)) == MAP_FAILED) {
            _rm_error(rm_NO_MEMORY, "_rm_sbrk: ran out of memory\n");
            return (void*)-1;
        }
    }
    
    rm_brk += incr;
    return (void*)old_brk;
}


/* ----------------------------------------------------------------------------
 *  rm_heap_lo
 * ----------------------------------------------------------------------------
*/
void* rm_heap_lo(void) {
    return (void*)rm_start_brk;
}


/* ----------------------------------------------------------------------------
 *  rm_heap_hi
 * ----------------------------------------------------------------------------
*/
void* rm_heap_hi(void) {
    return (void*)(rm_brk - 1);
}


/* ----------------------------------------------------------------------------
 *  rm_heapsize
 * ----------------------------------------------------------------------------
*/
size_t rm_heapsize(void) {
    return (size_t)(rm_brk - rm_start_brk);
}


/* ----------------------------------------------------------------------------
 *  rm_pagesize
 * ----------------------------------------------------------------------------
*/
size_t rm_pagesize(void) {
    return (size_t)getpagesize();
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
