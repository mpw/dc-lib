/* ============================================================================
 *  rm.h
 * ============================================================================

 *  Author:         (c) 2010 the rm team
 *  License:        See the end of this file for license information
 *  Created:        July 21, 2010

 *  Last changed:   $Date: 2012/02/03 18:07:44 $
 *  Changed by:     $Author: ribbi $
 *  Revision:       $Revision: 1.18 $
*/


#ifndef __rm__
#define __rm__

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif


#ifndef rm_STAT
#define rm_STAT     1
#endif

#ifndef rm_DEBUG
#define rm_DEBUG    1
#endif

/*disassembler to use*/
#define rm_LIBDISASM    0
#define rm_DISTORM      1
#ifndef rm_DISASSEMBLER
#define rm_DISASSEMBLER rm_LIBDISASM
#endif

#define rm_MAX_NOT_REACTIVE_MEMORY_ADDR_STR     "0xbfffffff"    // String for (rm_heap_START_BRK + rm_OFFSET - 1)
#define rm_OFFSET_STR                           "0x40000000"    // String for (rm_OFFSET)
#define rm_OFFSET                               0x40000000
#define rm_heap_START_BRK                       0x80000000

#if rm_STAT == 1
/*statistics*/
typedef struct {
    unsigned int patch_instr_number;
    unsigned int cache_instr_number;
    unsigned int exec_cache_instr_count;
    unsigned int actually_patched_instr_count;
    unsigned int actually_patched_BB_count;
    unsigned int BB_number;
    unsigned int BB_min_size;
    unsigned int BB_med_size;
    unsigned int BB_max_size;
    unsigned int BB_min_instr_number;
    unsigned int BB_med_instr_number;
    unsigned int BB_max_instr_number;
} rm_stats;

extern rm_stats g_stats;
#endif

/*error codes*/
enum {
    rm_CANNOT_OPEN_ELF = 1,
    rm_MPROTECT_FAIL = 2,
    rm_SIGACTION_FAIL = 3,
    rm_NO_MEMORY = 4,
    rm_CANNOT_OPEN_DUMP = 5
};

/*global variables for inline patching*/
enum {
    rm_read_eax_patch,
    rm_read_ecx_patch,
    rm_write_eax_patch,
    rm_write_ecx_patch,
    rm_rd_wr_eax_patch,
    rm_rd_wr_ecx_patch,
    rm_patch_num
};

enum {
    rm_size_1,
    rm_size_2,
    rm_size_4,
    rm_size_8,
    rm_size_num
};

extern void* g_patch_table[rm_patch_num][rm_size_num];
extern size_t g_size_table[rm_patch_num][rm_size_num];

/*global variables*/
extern size_t g_shadow_rec_size;
extern size_t g_shadow_wordsize;

/*macros*/
#define rm_get_addr_fast(rm_shadow_rec, word_t, shadow_rec_t) \
    ((void*)(rm_heap_START_BRK + rm_OFFSET - sizeof(word_t) + \
        (rm_heap_START_BRK - (unsigned int)rm_shadow_rec) *   \
            (sizeof(word_t)/sizeof(shadow_rec_t))))

#define rm_get_shadow_rec_fast(rm_addr, word_t, shadow_rec_t) \
    ((void *)(rm_heap_START_BRK-(((unsigned int)rm_addr-rm_OFFSET-rm_heap_START_BRK+ \
              sizeof(word_t))*(sizeof(shadow_rec_t)/sizeof(word_t)))))

#define rm_get_shadow_rec(rm_addr) ((void*)(rm_heap_START_BRK - ((((((unsigned int)rm_get_inactive_ptr(((unsigned int)(rm_addr)) & ~(g_shadow_wordsize-1))) - rm_heap_START_BRK) / g_shadow_wordsize) + 1) * g_shadow_rec_size)))
#define rm_is_reactive(addr) (((unsigned int)(addr)) >= (rm_heap_START_BRK + rm_OFFSET))
#define rm_get_inactive_ptr(rm_addr) (((void*)(rm_addr)) - rm_OFFSET)


/** Reactive memory read/write event handler.
    \param rm_addr reactive memory address accessed by read/write operation
    \param size size in bytes of memory word read or written
*/
typedef void (*rm_h) (void* rm_addr, size_t size);


/** Initialize reactive memory manager.
    \param read_handler handler for reactive memory reads
    \param write_handler handler for reactive memory writes
    \param patch_table table containing pointers to inline patch codes
    \param size_table table containing sizes of inline patch codes
    \param shadow_rec_size size of shadow memory records, in bytes. A value of
           zero means no shadowing is done by rm.
    \param shadow_wordsize size of aligned shadowed words, in bytes (must be a 
           power of two). Note: shadow_wordsize == 1 means every byte of reactive 
           memory is shadowed, shadow_wordsize == 2 means 16-bit words aligned to 
           16-bit boundaries are shadowed, etc.
    \return 0 if initialization was successful, -1 otherwise
*/
int rm_init(rm_h read_handler, rm_h write_handler, 
            void* patch_table[rm_patch_num][rm_size_num], size_t size_table[rm_patch_num][rm_size_num], 
            size_t shadow_rec_size, size_t shadow_wordsize);


/** Get address of first reactive word and pointer to its associated shadow record.
    \param shadow_rec_ptr 
    \return address of first reactive word
*/
void* rm_get_first_word(void** shadow_rec_ptr);


/** Given a reactive word, get address of next reactive word and pointer to its 
    associated shadow record.
    \param word address of a reactive word
    \param shadow_rec_ptr 
    \return address of next reactive word
*/
void* rm_get_next_word(void* word, void** shadow_rec_ptr);


/** Get memory usage.
    \return total number of memory bytes in use by the reactive heap
*/
size_t rm_heap_alloc_bytes(void);


/** \return generic pointer to the first allocated byte in the heap */
void* rm_heap_alloc_lo(void);


/** \return generic pointer to the last allocated byte in the heap */
void* rm_heap_alloc_hi(void);


/** \return generic pointer to the first byte in the heap */
void* rm_heap_lo(void);


/** \return generic pointer to the last byte in the heap */
void* rm_heap_hi(void);


/** \return current size of the reactive heap in bytes */
size_t rm_heapsize(void);


/** \return system's page size in bytes */
size_t rm_pagesize(void);


#if __DOXYGEN__
/** Get pointer to shadow record associated with the reactive word containing a 
    given address. To be implemented as a macro.
    \param rm_addr reactive memory address (precondition: rm_is_reactive(rm_addr) != 0)
    \return pointer to shadow record associated with word at address 
            (rm_addr & ~(shadow_wordsize-1))
*/
void* rm_get_shadow_rec(const void* rm_addr);


/** Check if a memory address is reactive.
    To be implemented as a macro.
    \param addr memory address
    \return non-zero if addr is reactive, zero otherwise
*/
int rm_is_reactive(const void* addr);


/** Get alias pointer to access reactive memory without generating read/write events.
    To be implemented as a macro.
    Note: rm_addr does not have to be aligned to 32-bit boundaries.
    \param rm_addr reactive memory address (precondition: rm_is_reactive(rm_addr) != 0)
    \return generic pointer to the data associated to address rm_addr. The only difference
            between rm_addr and the returned pointer is that reading or writing memory 
            using the latter does not generate read/write events.
*/
void* rm_get_inactive_ptr(const void* rm_addr);
#endif


#if rm_DEBUG == 1
/** Make a dump file containing disassembled code.
    \param filename file name
    \return -1 if some error occurs, 0 otherwise
*/
int rm_make_dump_file(char* filename);
#endif


/** Get memory usage.
    \return total number of memory bytes in use by the reactive memory manager
*/
size_t rm_used_bytes(void);


/** Allocate reactive memory block. Allocates a block of size bytes of reactive memory, 
    returning a pointer to the beginning of the block. The content of the newly 
    allocated block of memory is not initialized, remaining with indeterminate values.
    The semantics is identical to the C malloc function, except that rm_malloc 
    allocates a reactive memory block.
    \param size size of the memory block, in bytes
    \return On success, a pointer to the reactive memory block allocated by the 
            function. The type of this pointer is always void*, which can be cast 
            to the desired type of data pointer in order to be dereferenceable.
            If the function failed to allocate the requested block of memory, a 
            null pointer is returned.
*/
void* rm_malloc(size_t size);


/** Allocate space for array in reactive memory. Allocates a block of reactive memory 
    for an array of num elements, each of them size bytes long, and initializes all its 
    bits to zero. The effective result is the allocation of an zero-initialized memory 
    block of (num*size) bytes. Initialization does not generate memory write events.
    The semantics is identical to the C calloc function, except that rm_calloc 
    allocates a reactive memory block.
    \param num number of array elements to be allocated
    \param size size of array elements, in bytes
    \return a pointer to the reactive memory block allocated by the function. The type 
    of this pointer is always void*, which can be cast to the desired type of data pointer 
    in order to be dereferenceable. If the function failed to allocate the requested block 
    of memory, a NULL pointer is returned.
*/
void* rm_calloc(size_t num, size_t size);


/** Reallocate reactive memory block. The size of the memory block pointed to by the ptr 
    parameter is changed to the size bytes, expanding or reducing the amount of memory 
    available in the block. The function may move the memory block to a new location, in 
    which case the new location is returned. The content of the memory block is preserved 
    up to the lesser of the new and old sizes, even if the block is moved. If the new size 
    is larger, the value of the newly allocated portion is indeterminate. In case that ptr 
    is NULL, the function behaves exactly as malloc, assigning a new block of size bytes 
    and returning a pointer to the beginning of it. In case that the size is 0, the reactive
    memory previously allocated in ptr is deallocated as if a call to free was made, and a 
    NULL pointer is returned.
    The semantics is identical to the C realloc function, except that rm_realloc 
    reallocates a reactive memory block.
    \param addr memory address
    \return non-zero if addr is reactive, zero otherwise
*/
void* rm_realloc(void* ptr, size_t size);


/** Deallocate space in reactive memory. A block of reactive memory previously allocated 
    using a call to rm_malloc, rm_calloc, or rm_realloc is deallocated, making it available 
    again for further allocations. Notice that this function leaves the value of ptr unchanged, 
    hence it still points to the same (now invalid) location, and not to the null pointer.
    The semantics is identical to the C free function, except that rm_free 
    frees a reactive memory block.
    \param ptr pointer to a memory block previously allocated with malloc, calloc or realloc to 
           be deallocated. If a null pointer is passed as argument, no action occurs.
*/
void rm_free(void* ptr);


#ifdef __cplusplus
}
#endif

#endif


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
