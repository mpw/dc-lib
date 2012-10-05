/* =====================================================================
 *  rlist.h
 * =====================================================================

 *  Author:         (c) 2011 Camil Demetrescu
 *  License:        See the end of this file for license information
 *  Created:        January 30, 2011
 *  Module:         dc/test/perf/benchmarks

 *  Last changed:   $Date: 2011/02/08 14:26:25 $
 *  Changed by:     $Author: demetres $
 *  Revision:       $Revision: 1.9 $
*/


#ifndef __rlist__
#define __rlist__

#include "list.h"
#include <stddef.h>

// macros
#ifndef DEBUG
#define DEBUG 0
#endif


#define container_of(ptr, type, member) ( \
    (type*)((char*)ptr - offsetof(type, member)))

typedef struct rnode_s {
    int val;
    struct rnode_s* next;
    void* param;
} rnode;

typedef void (*rlist_ins_t)(void* observer, rnode* prev);
typedef void (*rlist_rem_t)(void* observer, rnode* prev);

typedef struct {
    rnode** first;
    void* observer;
    rlist_ins_t ins;
    rlist_rem_t rem;
} rlist;

rlist*  rlist_new();
rlist*  rlist_new_rand(int n, int max_val);
rlist*  rlist_new_sorted_rand(int n, int max_delta);
void    rlist_subscribe(rlist* list, 
                   void* observer, rlist_ins_t ins, rlist_rem_t rem);
void    rlist_unsubscribe(rlist* list);
void    rlist_delete(rlist* list);
rnode*  rlist_insert_first(rlist* list, int val);
rnode*  rlist_insert_next(rlist* list, rnode* node, int val);
void    rlist_remove_first(rlist* list);
void    rlist_remove_next(rlist* list, rnode* node);
void    rlist_remove_all(rlist* list);
rnode*  rlist_last(rlist* list);
size_t  rlist_length(rlist* list);
void    rlist_print(rlist* list);

#define rlist_first(list)        (*(list)->first)
#define rlist_next(node)         ((node)->next)
#define rlist_param(node)        ((node)->param)
#define rlist_set_param(node, p) ((node)->param = (p))
#define rlist_val(node)          ((node)->val)
#define rlist_set_val(node, v)   ((node)->val = (v))

#endif


/* Copyright (C) 2011 Camil Demetrescu

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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
*/
