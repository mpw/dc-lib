/* ============================================================================
 *  rm_emu.c
 * ============================================================================

 *  Author:         (c) 2010 the rm team
 *  License:        See the end of this file for license information
 *  Created:        July 21, 2010
 *  Note:           x86 emulation unit

 *  Last changed:   $Date: 2011/01/06 17:24:42 $
 *  Changed by:     $Author: alex $
 *  Revision:       $Revision: 1.10 $
*/

#include "_rm_private.h"


/*=======================================
    CODE CACHE SIMULATOR FUNCS
========================================*/

#if 0       /***** VERSIONE FUNZIONANTE SENZA INLINE PATCH (se si utilizzano le inline patch gli handler sono NULL e si ha un AV) *****/
#define MOV_REG_MEM(reg_name, reg_id) \
    void mov_##reg_name##_mem (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data) { \
        /* loads reg_name register (offset reg_id in gregs struct as documented in sys/ucontext.h)... */ \
        context->uc_mcontext.gregs[reg_id] = *(unsigned*)rm_get_inactive_ptr(addr); \
        \
        /* advances EIP register (offset 14 in gregs struct as documented in sys/ucontext.h)... */ \
        context->uc_mcontext.gregs[14] += instr_size; \
        \
        g_read_handler(addr, 4); \
    }

#define MOV_MEM_REG(reg_name, reg_id) \
    void mov_mem_##reg_name (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data) { \
        /* writes reg_name (offset reg_id in gregs struct as documented in sys/ucontext.h) to mem location... */ \
        *(unsigned*)rm_get_inactive_ptr(addr) = context->uc_mcontext.gregs[reg_id]; \
        \
        /* advances EIP register (offset 14 in gregs struct as documented in sys/ucontext.h)... */ \
        context->uc_mcontext.gregs[14] = context->uc_mcontext.gregs[14] + instr_size; \
        \
        g_write_handler(addr, 4); \
    }


#define CMP_REG_MEM(reg_name, reg_id) \
    void cmp_##reg_name##_mem (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data) { \
        /* emulates instruction and modifies flags... */ \
        unsigned dest, source, flags; \
        dest = context->uc_mcontext.gregs[reg_id]; \
        source = *(unsigned*)rm_get_inactive_ptr(addr); \
        flags = context->uc_mcontext.gregs[16]; \
        __asm__ __volatile__ ( \
            "pushf;" \
            "push   %%ecx;" \
            "popf;" \
            "cmpl   %%ebx, %%eax;" \
            "pushf;" \
            "pop    %%ecx;" \
            "popf;" \
            : "=c" (flags) \
            : "a" (dest), "b" (source), "c" (flags) \
        ); \
        context->uc_mcontext.gregs[16] = flags; \
        \
        /* advances EIP register (offset 14 in gregs struct as documented in sys/ucontext.h)... */ \
        context->uc_mcontext.gregs[14] += instr_size; \
        \
        g_read_handler(addr, 4); \
    }

#define CMP_MEM_REG(reg_name, reg_id) \
    void cmp_mem_##reg_name (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data) { \
        /* emulates instruction and modifies flags... */ \
        unsigned dest, source, flags; \
        dest = *(unsigned*)rm_get_inactive_ptr(addr); \
        source = context->uc_mcontext.gregs[reg_id]; \
        flags = context->uc_mcontext.gregs[16]; \
        __asm__ __volatile__ ( \
            "pushf;" \
            "push   %%ecx;" \
            "popf;" \
            "cmpl   %%ebx, %%eax;" \
            "pushf;" \
            "pop    %%ecx;" \
            "popf;" \
            : "=c" (flags) \
            : "a" (dest), "b" (source), "c" (flags) \
        ); \
        context->uc_mcontext.gregs[16] = flags; \
        \
        /* advances EIP register (offset 14 in gregs struct as documented in sys/ucontext.h)... */ \
        context->uc_mcontext.gregs[14] += instr_size; \
        \
        g_read_handler(addr, 4); \
    }


#define ADD_REG_MEM(reg_name, reg_id) \
    void add_##reg_name##_mem (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data) { \
        /* emulates instruction and modifies destination and flags... */ \
        unsigned dest, source, flags; \
        dest = context->uc_mcontext.gregs[reg_id]; \
        source = *(unsigned*)rm_get_inactive_ptr(addr); \
        flags = context->uc_mcontext.gregs[16]; \
        __asm__ __volatile__ ( \
            "pushf;" \
            "push   %%ecx;" \
            "popf;" \
            "addl   %%ebx, %%eax;" \
            "pushf;" \
            "pop    %%ecx;" \
            "popf;" \
            : "=a" (dest), "=c" (flags) \
            : "a" (dest), "b" (source), "c" (flags) \
        ); \
        context->uc_mcontext.gregs[reg_id] = dest; \
        context->uc_mcontext.gregs[16] = flags; \
        \
        /* advances EIP register (offset 14 in gregs struct as documented in sys/ucontext.h)... */ \
        context->uc_mcontext.gregs[14] += instr_size; \
        \
        g_read_handler(addr, 4); \
    }

#define ADD_MEM_REG(reg_name, reg_id) \
    void add_mem_##reg_name (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data) { \
        /* emulates instruction and modifies destination and flags... */ \
        unsigned dest, source, flags; \
        dest = *(unsigned*)rm_get_inactive_ptr(addr); \
        source = context->uc_mcontext.gregs[reg_id]; \
        flags = context->uc_mcontext.gregs[16]; \
        __asm__ __volatile__ ( \
            "pushf;" \
            "push   %%ecx;" \
            "popf;" \
            "addl   %%ebx, %%eax;" \
            "pushf;" \
            "pop    %%ecx;" \
            "popf;" \
            : "=a" (dest), "=c" (flags) \
            : "a" (dest), "b" (source), "c" (flags) \
        ); \
        *(unsigned*)rm_get_inactive_ptr(addr) = dest; \
        context->uc_mcontext.gregs[16] = flags; \
        \
        /* advances EIP register (offset 14 in gregs struct as documented in sys/ucontext.h)... */ \
        context->uc_mcontext.gregs[14] += instr_size; \
        \
        g_read_handler(addr, 4); \
        g_write_handler(addr, 4); \
    }


#define SUB_REG_MEM(reg_name, reg_id) \
    void sub_##reg_name##_mem (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data) { \
        /* emulates instruction and modifies destination and flags... */ \
        unsigned dest, source, flags; \
        dest = context->uc_mcontext.gregs[reg_id]; \
        source = *(unsigned*)rm_get_inactive_ptr(addr); \
        flags = context->uc_mcontext.gregs[16]; \
        __asm__ __volatile__ ( \
            "pushf;" \
            "push   %%ecx;" \
            "popf;" \
            "subl   %%ebx, %%eax;" \
            "pushf;" \
            "pop    %%ecx;" \
            "popf;" \
            : "=a" (dest), "=c" (flags) \
            : "a" (dest), "b" (source), "c" (flags) \
        ); \
        context->uc_mcontext.gregs[reg_id] = dest; \
        context->uc_mcontext.gregs[16] = flags; \
        \
        /* advances EIP register (offset 14 in gregs struct as documented in sys/ucontext.h)... */ \
        context->uc_mcontext.gregs[14] += instr_size; \
        \
        g_read_handler(addr, 4); \
    }

#define SUB_MEM_REG(reg_name, reg_id) \
    void sub_mem_##reg_name (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data) { \
        /* emulates instruction and modifies destination and flags... */ \
        unsigned dest, source, flags; \
        dest = *(unsigned*)rm_get_inactive_ptr(addr); \
        source = context->uc_mcontext.gregs[reg_id]; \
        flags = context->uc_mcontext.gregs[16]; \
        __asm__ __volatile__ ( \
            "pushf;" \
            "push   %%ecx;" \
            "popf;" \
            "subl   %%ebx, %%eax;" \
            "pushf;" \
            "pop    %%ecx;" \
            "popf;" \
            : "=a" (dest), "=c" (flags) \
            : "a" (dest), "b" (source), "c" (flags) \
        ); \
        *(unsigned*)rm_get_inactive_ptr(addr) = dest; \
        context->uc_mcontext.gregs[16] = flags; \
        \
        /* advances EIP register (offset 14 in gregs struct as documented in sys/ucontext.h)... */ \
        context->uc_mcontext.gregs[14] += instr_size; \
        \
        g_read_handler(addr, 4); \
        g_write_handler(addr, 4); \
    }


MOV_REG_MEM(eax, 11)
MOV_REG_MEM(ebx, 8)
MOV_REG_MEM(ecx, 10)
MOV_REG_MEM(edx, 9)
MOV_REG_MEM(esi, 5)
MOV_REG_MEM(edi, 4)

MOV_MEM_REG(eax, 11)
MOV_MEM_REG(ebx, 8)
MOV_MEM_REG(ecx, 10)
MOV_MEM_REG(edx, 9)
MOV_MEM_REG(esi, 5)
MOV_MEM_REG(edi, 4)

void mov_mem_imm (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data) {
    /* writes immediate (cached) operand to mem location... */
    *(unsigned*)rm_get_inactive_ptr(addr) = imm_data;
    
    /* advances EIP register (offset 14 in gregs struct as documented in sys/ucontext.h)... */
    context->uc_mcontext.gregs[14] = context->uc_mcontext.gregs[14] + instr_size;
    
    g_write_handler(addr, 4);
}


CMP_REG_MEM(eax, 11)
CMP_REG_MEM(ebx, 8)
CMP_REG_MEM(ecx, 10)
CMP_REG_MEM(edx, 9)
CMP_REG_MEM(esi, 5)
CMP_REG_MEM(edi, 4)

CMP_MEM_REG(eax, 11)
CMP_MEM_REG(ebx, 8)
CMP_MEM_REG(ecx, 10)
CMP_MEM_REG(edx, 9)
CMP_MEM_REG(esi, 5)
CMP_MEM_REG(edi, 4)

void cmp_mem_imm (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data) {
    /* emulates instruction and modifies flags... */
    unsigned dest, source, flags;
    dest = *(unsigned*)rm_get_inactive_ptr(addr);
    source = imm_data;
    flags = context->uc_mcontext.gregs[16];
    __asm__ __volatile__ (
        "pushf;"
        "push   %%ecx;"
        "popf;"
        "cmpl   %%ebx, %%eax;"
        "pushf;"
        "pop    %%ecx;"
        "popf;"
        : "=c" (flags)
        : "a" (dest), "b" (source), "c" (flags)
    );
    context->uc_mcontext.gregs[16] = flags;
    
    /* advances EIP register (offset 14 in gregs struct as documented in sys/ucontext.h)... */
    context->uc_mcontext.gregs[14] += instr_size;
    
    g_read_handler(addr, 4);
}


ADD_REG_MEM(eax, 11)
ADD_REG_MEM(ebx, 8)
ADD_REG_MEM(ecx, 10)
ADD_REG_MEM(edx, 9)
ADD_REG_MEM(esi, 5)
ADD_REG_MEM(edi, 4)

ADD_MEM_REG(eax, 11)
ADD_MEM_REG(ebx, 8)
ADD_MEM_REG(ecx, 10)
ADD_MEM_REG(edx, 9)
ADD_MEM_REG(esi, 5)
ADD_MEM_REG(edi, 4)

void add_mem_imm (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data) {
    /* emulates instruction and modifies destination and flags... */
    unsigned dest, source, flags;
    dest = *(unsigned*)rm_get_inactive_ptr(addr);
    source = imm_data;
    flags = context->uc_mcontext.gregs[16];
    __asm__ __volatile__ (
        "pushf;"
        "push   %%ecx;"
        "popf;"
        "addl   %%ebx, %%eax;"
        "pushf;"
        "pop    %%ecx;"
        "popf;"
        : "=a" (dest), "=c" (flags)
        : "a" (dest), "b" (source), "c" (flags)
    );
    *(unsigned*)rm_get_inactive_ptr(addr) = dest;
    context->uc_mcontext.gregs[16] = flags;
    
    /* advances EIP register (offset 14 in gregs struct as documented in sys/ucontext.h)... */
    context->uc_mcontext.gregs[14] += instr_size;
    
    g_read_handler(addr, 4);
    g_write_handler(addr, 4);
}


SUB_REG_MEM(eax, 11)
SUB_REG_MEM(ebx, 8)
SUB_REG_MEM(ecx, 10)
SUB_REG_MEM(edx, 9)
SUB_REG_MEM(esi, 5)
SUB_REG_MEM(edi, 4)

SUB_MEM_REG(eax, 11)
SUB_MEM_REG(ebx, 8)
SUB_MEM_REG(ecx, 10)
SUB_MEM_REG(edx, 9)
SUB_MEM_REG(esi, 5)
SUB_MEM_REG(edi, 4)

void sub_mem_imm (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data) {
    /* emulates instruction and modifies destination and flags... */
    unsigned dest, source, flags;
    dest = *(unsigned*)rm_get_inactive_ptr(addr);
    source = imm_data;
    flags = context->uc_mcontext.gregs[16];
    __asm__ __volatile__ (
        "pushf;"
        "push   %%ecx;"
        "popf;"
        "subl   %%ebx, %%eax;"
        "pushf;"
        "pop    %%ecx;"
        "popf;"
        : "=a" (dest), "=c" (flags)
        : "a" (dest), "b" (source), "c" (flags)
    );
    *(unsigned*)rm_get_inactive_ptr(addr) = dest;
    context->uc_mcontext.gregs[16] = flags;
    
    /* advances EIP register (offset 14 in gregs struct as documented in sys/ucontext.h)... */
    context->uc_mcontext.gregs[14] += instr_size;
    
    g_read_handler(addr, 4);
    g_write_handler(addr, 4);
}
#endif



#define MOV_REG_MEM(reg_name, reg_id) \
    void mov_##reg_name##_mem (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data) { \
        printf("*** mov_reg_mem emulation: TO DO ***\n"); \
        printf(">> addr=%p - EIP=%p\n", addr, (void*)context->uc_mcontext.gregs[14]); \
        exit(1); \
    }

#define MOV_MEM_REG(reg_name, reg_id) \
    void mov_mem_##reg_name (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data) { \
        printf("*** mov_mem_reg emulation: TO DO ***\n"); \
        printf(">> addr=%p - EIP=%p\n", addr, (void*)context->uc_mcontext.gregs[14]); \
        exit(1); \
    }


#define CMP_REG_MEM(reg_name, reg_id) \
    void cmp_##reg_name##_mem (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data) { \
        printf("*** cmp_reg_mem emulation: TO DO ***\n"); \
        printf(">> addr=%p - EIP=%p\n", addr, (void*)context->uc_mcontext.gregs[14]); \
        exit(1); \
    }

#define CMP_MEM_REG(reg_name, reg_id) \
    void cmp_mem_##reg_name (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data) { \
        printf("*** cmp_mem_reg emulation: TO DO ***\n"); \
        printf(">> addr=%p - EIP=%p\n", addr, (void*)context->uc_mcontext.gregs[14]); \
        exit(1); \
    }


#define ADD_REG_MEM(reg_name, reg_id) \
    void add_##reg_name##_mem (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data) { \
        printf("*** add_reg_mem emulation: TO DO ***\n"); \
        printf(">> addr=%p - EIP=%p\n", addr, (void*)context->uc_mcontext.gregs[14]); \
        exit(1); \
    }

#define ADD_MEM_REG(reg_name, reg_id) \
    void add_mem_##reg_name (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data) { \
        printf("*** add_mem_reg emulation: TO DO ***\n"); \
        printf(">> addr=%p - EIP=%p\n", addr, (void*)context->uc_mcontext.gregs[14]); \
        exit(1); \
    }


#define SUB_REG_MEM(reg_name, reg_id) \
    void sub_##reg_name##_mem (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data) { \
        printf("*** sub_reg_mem emulation: TO DO ***\n"); \
        printf(">> addr=%p - EIP=%p\n", addr, (void*)context->uc_mcontext.gregs[14]); \
        exit(1); \
    }

#define SUB_MEM_REG(reg_name, reg_id) \
    void sub_mem_##reg_name (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data) { \
        printf("*** sub_mem_reg emulation: TO DO ***\n"); \
        printf(">> addr=%p - EIP=%p\n", addr, (void*)context->uc_mcontext.gregs[14]); \
        exit(1); \
    }


MOV_REG_MEM(eax, 11)
MOV_REG_MEM(ebx, 8)
MOV_REG_MEM(ecx, 10)
MOV_REG_MEM(edx, 9)
MOV_REG_MEM(esi, 5)
MOV_REG_MEM(edi, 4)

MOV_MEM_REG(eax, 11)
MOV_MEM_REG(ebx, 8)
MOV_MEM_REG(ecx, 10)
MOV_MEM_REG(edx, 9)
MOV_MEM_REG(esi, 5)
MOV_MEM_REG(edi, 4)

void mov_mem_imm (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data) {
    printf("*** mov_mem_imm emulation: TO DO ***\n");
    printf(">> addr=%p - EIP=%p\n", addr, (void*)context->uc_mcontext.gregs[14]);
    exit(1);
}


CMP_REG_MEM(eax, 11)
CMP_REG_MEM(ebx, 8)
CMP_REG_MEM(ecx, 10)
CMP_REG_MEM(edx, 9)
CMP_REG_MEM(esi, 5)
CMP_REG_MEM(edi, 4)

CMP_MEM_REG(eax, 11)
CMP_MEM_REG(ebx, 8)
CMP_MEM_REG(ecx, 10)
CMP_MEM_REG(edx, 9)
CMP_MEM_REG(esi, 5)
CMP_MEM_REG(edi, 4)

void cmp_mem_imm (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data) {
    printf("*** cmp_mem_imm emulation: TO DO ***\n");
    printf(">> addr=%p - EIP=%p\n", addr, (void*)context->uc_mcontext.gregs[14]);
    exit(1);
}


ADD_REG_MEM(eax, 11)
ADD_REG_MEM(ebx, 8)
ADD_REG_MEM(ecx, 10)
ADD_REG_MEM(edx, 9)
ADD_REG_MEM(esi, 5)
ADD_REG_MEM(edi, 4)

ADD_MEM_REG(eax, 11)
ADD_MEM_REG(ebx, 8)
ADD_MEM_REG(ecx, 10)
ADD_MEM_REG(edx, 9)
ADD_MEM_REG(esi, 5)
ADD_MEM_REG(edi, 4)

void add_mem_imm (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data) {
    printf("*** add_mem_imm emulation: TO DO ***\n");
    printf(">> addr=%p - EIP=%p\n", addr, (void*)context->uc_mcontext.gregs[14]);
    exit(1);
}


SUB_REG_MEM(eax, 11)
SUB_REG_MEM(ebx, 8)
SUB_REG_MEM(ecx, 10)
SUB_REG_MEM(edx, 9)
SUB_REG_MEM(esi, 5)
SUB_REG_MEM(edi, 4)

SUB_MEM_REG(eax, 11)
SUB_MEM_REG(ebx, 8)
SUB_MEM_REG(ecx, 10)
SUB_MEM_REG(edx, 9)
SUB_MEM_REG(esi, 5)
SUB_MEM_REG(edi, 4)

void sub_mem_imm (void* addr, ucontext_t* context, unsigned instr_size, unsigned imm_data) {
    printf("*** sub_mem_imm emulation: TO DO ***\n");
    printf(">> addr=%p - EIP=%p\n", addr, (void*)context->uc_mcontext.gregs[14]);
    exit(1);
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
