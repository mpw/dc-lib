/* ============================================================================
 *  _rm_private.h
 * ============================================================================

 *  Author:         (c) 2010 the rm team
 *  License:        See the end of this file for license information
 *  Created:        July 21, 2010
 *  Note:           rm internals

 *  Last changed:   $Date: 2011/01/31 12:21:04 $
 *  Changed by:     $Author: alex $
 *  Revision:       $Revision: 1.14 $
*/


#ifndef ___rm_private__
#define ___rm_private__

#include "rm.h"
#include "_rm_config.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

//#include <sys/ptrace.h>
//#include <sys/wait.h>
//#include <sys/user.h>
#include <sys/mman.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
//#include <sys/ipc.h>
//#include <sys/shm.h>
#include <ucontext.h>
//#include <sys/time.h>

#include <glib.h>
#include <libelf.h>
#include <gelf.h>

#if rm_DISASSEMBLER == rm_LIBDISASM
/*libdisasm main include file...*/
#include <libdis.h>
#else
/*distorm main include file...*/
#include <distorm.h>
#include <mnemonics.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*typedefs...*/
typedef struct tagrm_TCODECACHEDATA {
    void (*sim_func) (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data);
    unsigned instr_size;
    unsigned imm_data; /*required by MOV <MEM>,#const instr...*/
} rm_CODECACHEDATA;

/*hash table for code cache...*/
GHashTable* g_cchash;

/*global handler pointers...*/
rm_h g_read_handler;
rm_h g_write_handler;

/*pagesize...*/
unsigned int g_pagesize;

/*original sigaction...*/
struct sigaction g_orig_sigaction;


//CODE CACHE SIMULATOR FUNC prototypes...
void mov_eax_mem (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data);
void mov_ebx_mem (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data);
void mov_ecx_mem (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data);
void mov_edx_mem (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data);
void mov_esi_mem (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data);
void mov_edi_mem (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data);

void mov_mem_eax (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data);
void mov_mem_ebx (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data);
void mov_mem_ecx (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data);
void mov_mem_edx (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data);
void mov_mem_esi (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data);
void mov_mem_edi (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data);

void mov_mem_imm (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data);


void cmp_eax_mem (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data);
void cmp_ebx_mem (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data);
void cmp_ecx_mem (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data);
void cmp_edx_mem (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data);
void cmp_esi_mem (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data);
void cmp_edi_mem (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data);

void cmp_mem_eax (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data);
void cmp_mem_ebx (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data);
void cmp_mem_ecx (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data);
void cmp_mem_edx (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data);
void cmp_mem_esi (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data);
void cmp_mem_edi (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data);

void cmp_mem_imm (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data);


void add_eax_mem (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data);
void add_ebx_mem (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data);
void add_ecx_mem (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data);
void add_edx_mem (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data);
void add_esi_mem (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data);
void add_edi_mem (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data);

void add_mem_eax (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data);
void add_mem_ebx (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data);
void add_mem_ecx (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data);
void add_mem_edx (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data);
void add_mem_esi (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data);
void add_mem_edi (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data);

void add_mem_imm (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data);


void sub_eax_mem (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data);
void sub_ebx_mem (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data);
void sub_ecx_mem (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data);
void sub_edx_mem (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data);
void sub_esi_mem (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data);
void sub_edi_mem (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data);

void sub_mem_eax (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data);
void sub_mem_ebx (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data);
void sub_mem_ecx (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data);
void sub_mem_edx (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data);
void sub_mem_esi (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data);
void sub_mem_edi (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data);

void sub_mem_imm (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data);


/** Sets variable errno and prints debug error string.
    \param code an error code (defined in rm.h)
    \param format a string describing the error
*/
void _rm_error(int code, char* format, ...);


/** Segmentation fault handler.
    \param signum 
    \param mysiginfo 
    \param mypointer 
*/
void _rm_segvhandler(int signum, siginfo_t* mysiginfo, void* mypointer);


/** Install patch for current instruction (the one addressed by the program counter of mycontext).
    \param mycontext the context
    \return 0 if the instruction cannot be patched, 1 if the instruction is successfully patched, -1 if some error occurs
*/
int _rm_patch_install(ucontext_t* mycontext);


/** Initialize structures needed for patching.
    \return 0 if initialization was successful, -1 otherwise
*/
int _rm_patch_init(void);


/** Initialize reactive memory allocator.
    \return 0 if initialization was successful, -1 otherwise
*/
int _rm_alloc_init(void);


/** Get memory usage.
    \return total number of memory bytes in use by the executable heap
*/
size_t _rm_exec_heap_used_bytes(void);


/** Expands the executable heap by incr bytes, where incr is a non-negative integer.
    \param incr non-negative integer.
    \return generic pointer to the first byte of the newly allocated heap area
*/
void* _rm_exec_sbrk(int incr);


/** Initialize reactive heap.
    \return 0 if initialization was successful, -1 otherwise
*/
int _rm_heap_init(void);


/** Expands the reactive heap by incr bytes, where incr is a non-negative integer.
    The semantics is identical to the Unix sbrk function, except that rm_sbrk 
    accepts only a non-negative integer argument.
    \param incr non-negative integer.
    \return generic pointer to the first byte of the newly allocated heap area
*/
void* _rm_sbrk(int incr);


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
