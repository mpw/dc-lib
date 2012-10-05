/* ============================================================================
 *  rm_av.c
 * ============================================================================

 *  Author:         (c) 2010 the rm team
 *  License:        See the end of this file for license information
 *  Created:        July 21, 2010
 *  Note:           segmentation fault handler

 *  Last changed:   $Date: 2011/01/06 17:24:41 $
 *  Changed by:     $Author: alex $
 *  Revision:       $Revision: 1.13 $
*/

#include "_rm_private.h"


/* ----------------------------------------------------------------------------
 *  _rm_segvhandler
 * ----------------------------------------------------------------------------
*/
void _rm_segvhandler(int signum, siginfo_t* mysiginfo, void* mypointer) {
    rm_CODECACHEDATA* the_cache_data;
    ucontext_t* mycontext;
    int patch_ret;
    
    /*retrieves faulty context...*/
    mycontext = (ucontext_t*)mypointer;
    
    //if access violation is not in reactive memory
    if(!rm_is_reactive(mysiginfo->si_addr)) {
        
#if rm_DEBUG == 1
        printf("[rm] Access violation to non-reactive address %p\n", (void*)mysiginfo->si_addr);
        printf("[rm] Offending instruction %p\n", (void*)mycontext->uc_mcontext.gregs[14]);
#endif
        
        // launch original AV handler
        #if 0
        if(g_orig_sigaction.sa_flags & SA_SIGINFO == SA_SIGINFO) {
            g_orig_sigaction.sa_sigaction(signum, mysiginfo, mypointer);
        }
        else {
            g_orig_sigaction.sa_handler(signum);
        }
        #endif

        //************* meglio solo return e demandare tutta la responsabilità all'handler originale? **********************************
        printf("[rm] Segmentation fault\n");
        exit(1);
    }
    
    //CODE CACHE STUFF HERE...
    //gets instr data...
    the_cache_data = g_hash_table_lookup(g_cchash, (void*)mycontext->uc_mcontext.gregs[14]);
    //if EIP is in cache...
    if(the_cache_data) {
        
        //printf ("Using cached code for instr %p\n", (void*)mycontext->uc_mcontext.gregs[14]);
#if rm_STAT == 1
        g_stats.exec_cache_instr_count++;
#endif

        //simulates instr (including handlers, if any)...
        the_cache_data->sim_func(mysiginfo->si_addr, mycontext, the_cache_data->instr_size, the_cache_data->imm_data);
        
        return;
    }
    
    /*patches the instruction...*/
    patch_ret = _rm_patch_install(mycontext);
    if(patch_ret == -1) {
        exit(1);
    }
    if(patch_ret == 1) {
        return;
    }
    
    
    //if we get here then we could not patch nor simulate instruction...
    printf("[rm] Unable to handle instruction!\n");
    printf("[rm] Address: %p\n", (char*)mycontext->uc_mcontext.gregs[14]);
    
#if rm_DEBUG == 1
#if rm_DISASSEMBLER == rm_LIBDISASM
    int LINE_SIZE = 100;
    char line[LINE_SIZE];
    int size;
    x86_insn_t insn;
    
    /*initializes libdisasm...*/
    x86_init(opt_none, NULL, NULL);
    
    /* disassemble address */
    size = x86_disasm((unsigned char*)mycontext->uc_mcontext.gregs[14], rm_SINGLE_INSN_MAX_SIZE, 0, 0, &insn);
    
    /* print instruction size */
    printf("[rm] Instruction size: %d\n", size);
    
    if(size) {
        /* print instruction */
        x86_format_insn(&insn, line, LINE_SIZE, att_syntax);
        printf("[rm] Instruction: %s\n", line);
        
        /* print instruction opcode */
        printf("[rm] Instruction opcode: ");
        int i;
        for(i=0; i<size; i++) {
            printf("%02x ", *(unsigned char*)(mycontext->uc_mcontext.gregs[14] + i));
        }
        printf("\n");
    }
    else {
        printf("[rm] Instruction: invalid instruction\n");
    }
    
        /*uninitializes libdisasm...*/
    x86_cleanup();

#else

    _DInst insn[1];         /* instructions */
    unsigned int instructions_count = 0;
    _DecodedInst inst;      /* instruction (string) */
    
    _CodeInfo ci;
    ci.code = (unsigned char*)mycontext->uc_mcontext.gregs[14];
    ci.codeLen = rm_SINGLE_INSN_MAX_SIZE;
    ci.codeOffset = mycontext->uc_mcontext.gregs[14];
    ci.dt = Decode32Bits;
    ci.features = DF_NONE;
    
    /* disassemble address */
    distorm_decompose(&ci, insn, 1, &instructions_count);
    
    if(instructions_count != 1 || insn[0].flags == FLAG_NOT_DECODABLE) {
        printf("[rm] Instruction: invalid instruction\n");
    }
    else {
        /* print instruction size */
        printf("[rm] Instruction size: %d\n", insn[0].size);
        
        /* print instruction */
        distorm_format(&ci, &insn[0], &inst);
        printf("[rm] Instruction: %s\t%s\n", inst.mnemonic.p, inst.operands.p);
        
        /* print instruction opcode */
        printf("[rm] Instruction opcode: ");
        int i;
        for(i=0; i<insn[0].size; i++) {
            printf("%02x ", *(unsigned char*)(mycontext->uc_mcontext.gregs[14] + i));
        }
        printf("\n");
    }
#endif
#endif
    
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
