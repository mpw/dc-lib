/* ============================================================================
 *  rm_init.c
 * ============================================================================

 *  Author:         (c) 2010 the rm team
 *  License:        See the end of this file for license information
 *  Created:        July 21, 2010
 *  Note:           reactive memory manager init

 *  Last changed:   $Date: 2012/02/03 18:07:44 $
 *  Changed by:     $Author: ribbi $
 *  Revision:       $Revision: 1.9 $
*/

#include "_rm_private.h"

#if rm_STAT == 1
rm_stats g_stats;
#endif

void* g_patch_table[rm_patch_num][rm_size_num];
size_t g_size_table[rm_patch_num][rm_size_num];

/*global variables*/
size_t g_shadow_rec_size;
size_t g_shadow_wordsize;

/* ----------------------------------------------------------------------------
 *  _rm_error
 * ----------------------------------------------------------------------------
*/
void _rm_error(int code, char* format, ...) {
#if rm_DEBUG == 1
    printf("[rm ERROR] ");
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
#endif
    errno = code;
}


/* ----------------------------------------------------------------------------
 *  rm_init
 * ----------------------------------------------------------------------------
*/
int rm_init(rm_h read_handler, rm_h write_handler, 
            void* patch_table[rm_patch_num][rm_size_num], size_t size_table[rm_patch_num][rm_size_num], 
            size_t shadow_rec_size, size_t shadow_wordsize) {
    
    struct sigaction mysigaction;
    sigset_t myset;
    
    /*gets pagesize...*/
    g_pagesize = getpagesize();
    
    /*copies handlers to global vars...*/
    g_read_handler = read_handler;
    g_write_handler = write_handler;
    
    /*copies variables for inline patching to global vars...*/
    if(patch_table && size_table) {
        int i, j;
        for(i=0; i<rm_patch_num; i++) {
            for(j=0; j<rm_size_num; j++) {
                g_patch_table[i][j] = patch_table[i][j];
                g_size_table[i][j] = size_table[i][j];
            }
        }
    }
    
    /*copies shadow sizes to global vars...*/
    g_shadow_rec_size = shadow_rec_size;
    g_shadow_wordsize = shadow_wordsize;
    
    /*initializes patching system...*/
    if(_rm_patch_init() == -1)
        return -1;
    
    /*initializes reactive heap...*/
    if(_rm_heap_init() == -1)
        return -1;
    
    /*initializes reactive memory allocator...*/
    if(_rm_alloc_init() == -1)
        return -1;
    
    /*installs segment violation handler...*/
    sigemptyset(&myset);
    mysigaction.sa_handler = NULL;
    mysigaction.sa_sigaction = _rm_segvhandler;
    mysigaction.sa_mask = myset;
    mysigaction.sa_flags = SA_SIGINFO|SA_NODEFER;
    mysigaction.sa_restorer = NULL;
    if (-1 == sigaction (SIGSEGV, &mysigaction, &g_orig_sigaction)) {
        _rm_error(rm_SIGACTION_FAIL, "rm_init: sigaction() failed!\n");
        return -1;
    }
    else {
        //printf("SIGSEGV handler installed!\n");
    }
    
    return 0;
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
