// =====================================================================
//  dc/src/dc_asm.c
// =====================================================================

//  Author:         (C) 2011 Camil Demetrescu
//  License:        See the end of this file for license information
//  Created:        January 1, 2011
//  Module:         dc

//  Last changed:   $Date: 2011/01/25 16:07:56 $
//  Changed by:     $Author: demetres $
//  Revision:       $Revision: 1.15 $


#include "dc_globals.h"


// asm code labels
extern char dc_patch_start_RA;
extern char dc_patch_end_RA;

extern char dc_patch_start_RC;
extern char dc_patch_end_RC;

extern char dc_patch_start_WA;
extern char dc_patch_end_WA;

extern char dc_patch_start_WC;
extern char dc_patch_end_WC;

extern char dc_patch_start_RWA;
extern char dc_patch_end_RWA;

extern char dc_patch_start_RWC;
extern char dc_patch_end_RWC;


// asm code macros

// ---------------------------------------------------------------------
//  _DC_INLINE_PATCH_CODE_32_R
// ---------------------------------------------------------------------
// inline patch for read instructions (e.g, movl (%eax), %edx)
//    label: label identifying the patch code ("RA" or "RC")
//    reg:   register (eax or ecx) containing the reactive address
//           (does not have to be saved/restored)
//    reg2:  scratch register not to be modified (ecx or eax)

#define _DC_INLINE_PATCH_CODE_32_R(label, reg, reg2)                   \
    __asm__ __volatile__ (                                             \
        "dc_patch_start_R"label":"                                     \
            "cmpl   $0x0, dc_no_log_blocks_count_g;"                   \
            "jnz    dc_patch_end_R"label";"                            \
            "andl   $0xFFFFFFFC, %"reg";"                              \
            "subl   $0x3FFFFFFC, %"reg";"                              \
            "negl   %"reg";"                                           \
            "testb  $0x1, (%"reg");"                                   \
            "jne    dc_patch_end_R"label";"                            \
            "push   %"reg2";"                                          \
            "push   %edx;"                                             \
            "push   %"reg";"                                           \
            "movl   $_dc_read_handler, %"reg";"                        \
            "call   *%"reg";"                                          \
            "addl   $0x4, %esp;"                                       \
            "pop    %edx;"                                             \
            "pop    %"reg2";"                                          \
        "dc_patch_end_R"label":"                                       \
    );


// ---------------------------------------------------------------------
//  _DC_INLINE_PATCH_CODE_32_W
// ---------------------------------------------------------------------
// inline patch for write instructions (e.g, movl $0x1, (%eax))
//    label: label identifying the patch code ("RA" or "RC")
//    reg:   register (eax or ecx) containing the reactive address
//           (does not have to be saved/restored)
//    reg2:  scratch register not to be modified (ecx or eax)

#define _DC_INLINE_PATCH_CODE_32_W(label, reg, reg2)                   \
    __asm__ __volatile__ (                                             \
        "dc_patch_start_W"label":"                                     \
            "andl   $0xFFFFFFFC, %"reg";"                              \
            "subl   $0x3FFFFFFC, %"reg";"                              \
            "negl   %"reg";"                                           \
            "testb  $0x2, (%"reg");"                                   \
            "jne    dc_patch_end_W"label";"                            \
            "push   %"reg2";"                                          \
            "push   %edx;"                                             \
            "push   %"reg";"                                           \
            "movl   $_dc_write_handler, %"reg";"                       \
            "call   *%"reg";"                                          \
            "addl   $0x4, %esp;"                                       \
            "pop    %edx;"                                             \
            "pop    %"reg2";"                                          \
        "dc_patch_end_W"label":"                                       \
    );


// ---------------------------------------------------------------------
//  _DC_INLINE_PATCH_CODE_32_RW
// ---------------------------------------------------------------------
// inline patch for read+write instructions (e.g, addl $0x1, (%eax))
//    label: label identifying the patch code ("RWA" or "RWC")
//    reg:   register (eax or ecx) containing the reactive address
//           (does not have to be saved/restored)
//    reg2:  scratch register not to be modified (ecx or eax)

// the unified handling of read+write operations on the same reactive
// address allows it to reduce the total number of instructions that
// would be executed by performing separate operations:

// case 1 (normal mode, cell is first written): 
//     RW = 15 instructions, R+W = 2+13 = 15 instructions

// case 2 (normal mode, cell already written): 
//     RW = 7 instructions, R+W = 2+5 = 7 instructions

// case 3 (constraint mode, cell first read and first written): 
//     RW = 20 instructions, R+W = 15+13 = 28 instructions

// case 4 (constraint mode, cell already read and first written): 
//     RW = 17 instructions, R+W = 7+13 = 20 instructions

// case 5 (constraint mode, cell already read and already written): 
//     RW = 9 instructions, R+W = 7+5 = 12 instructions

// note: the case "cell first read and already written" is not possible
//       since no dependency is set from cells that are written and
//       subsequently read by the execution of a constraint

// note: the size of the unified RW patch code is also smaller 
//       than the sum of the sizes of the R/W patch codes: 
//          RW = 27 instructions, R+W = 15+13 = 28 instructions


#define _DC_INLINE_PATCH_CODE_32_RW(label, reg, reg2)                  \
    __asm__ __volatile__ (                                             \
        "dc_patch_start_RW"label":"                                    \
            "andl   $0xFFFFFFFC, %"reg";"                              \
            "subl   $0x3FFFFFFC, %"reg";"                              \
            "negl   %"reg";"                                           \
            \
            "cmpl   $0x0, dc_no_log_blocks_count_g;"                   \
            "jnz     dc_patch_write_RW"label";"                        \
            \
            "testb  $0x1, (%"reg");"                                   \
            "jne    dc_patch_write_RW"label";"                         \
            \
            "push   %"reg2";"                                          \
            "push   %edx;"                                             \
            "push   %"reg";"                                           \
            \
            "movl   $_dc_read_handler, %"reg";"                        \
            "call   *%"reg";"                                          \
            \
            "testb  $0x2, (%esp);"                                     \
            "jne    dc_patch_pop_RW"label";"                           \
            \
            "movl   $_dc_write_handler, %"reg";"                       \
            "call   *%"reg";"                                          \
            "jmp    dc_patch_pop_RW"label";"                           \
            \
        "dc_patch_write_RW"label":"                                    \
            "testb  $0x2, (%"reg");"                                   \
            "jne    dc_patch_end_RW"label";"                           \
            "push   %"reg2";"                                          \
            "push   %edx;"                                             \
            "push   %"reg";"                                           \
            "movl   $_dc_write_handler, %"reg";"                       \
            "call   *%"reg";"                                          \
            \
        "dc_patch_pop_RW"label":"                                      \
            "addl   $0x4, %esp;"                                       \
            "pop    %edx;"                                             \
            "pop    %"reg2";"                                          \
        "dc_patch_end_RW"label":"                                      \
    );


void __patch_code__() {
    _DC_INLINE_PATCH_CODE_32_R("A", "eax", "ecx")
    _DC_INLINE_PATCH_CODE_32_R("C", "ecx", "eax")
    _DC_INLINE_PATCH_CODE_32_W("A", "eax", "ecx")
    _DC_INLINE_PATCH_CODE_32_W("C", "ecx", "eax")
    _DC_INLINE_PATCH_CODE_32_RW("A", "eax", "ecx")
    _DC_INLINE_PATCH_CODE_32_RW("C", "ecx", "eax")
}


// ---------------------------------------------------------------------
//  _dc_init_patch_table
// ---------------------------------------------------------------------
int _dc_init_patch_table(void* patch_table[rm_patch_num][rm_size_num], 
                         size_t size_table[rm_patch_num][rm_size_num]) {

    // init 32-bit read event patch with eax reg
    patch_table[rm_read_eax_patch][rm_size_4] = &dc_patch_start_RA;
    size_table[rm_read_eax_patch][rm_size_4]  = 
        &dc_patch_end_RA - &dc_patch_start_RA;

    // init 32-bit read event patch with ecx reg
    patch_table[rm_read_ecx_patch][rm_size_4] = &dc_patch_start_RC;
    size_table[rm_read_ecx_patch][rm_size_4]  = 
        &dc_patch_end_RC - &dc_patch_start_RC;
    
    // init 32-bit write event patch with eax reg
    patch_table[rm_write_eax_patch][rm_size_4] = &dc_patch_start_WA;
    size_table[rm_write_eax_patch][rm_size_4]  = 
        &dc_patch_end_WA - &dc_patch_start_WA;

    // init 32-bit write event patch with ecx reg
    patch_table[rm_write_ecx_patch][rm_size_4] = &dc_patch_start_WC;
    size_table[rm_write_ecx_patch][rm_size_4]  = 
        &dc_patch_end_WC - &dc_patch_start_WC;

    // init 32-bit read+write event patch with eax reg
    patch_table[rm_rd_wr_eax_patch][rm_size_4] = &dc_patch_start_RWA;
    size_table[rm_rd_wr_eax_patch][rm_size_4]  = 
        &dc_patch_end_RWA - &dc_patch_start_RWA;

    // init 32-bit read+write event patch with ecx reg
    patch_table[rm_rd_wr_ecx_patch][rm_size_4] = &dc_patch_start_RWC;
    size_table[rm_rd_wr_ecx_patch][rm_size_4]  = 
        &dc_patch_end_RWC - &dc_patch_start_RWC;

    return 0;
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
