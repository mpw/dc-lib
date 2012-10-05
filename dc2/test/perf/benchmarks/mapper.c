/* =====================================================================
 *  mapper.c
 * =====================================================================

 *  Author:         (c) 2011 Camil Demetrescu
 *  License:        See the end of this file for license information
 *  Created:        January 30, 2011
 *  Module:         dc/test/perf/benchmarks

 *  Last changed:   $Date: 2011/01/31 19:40:46 $
 *  Changed by:     $Author: demetres $
 *  Revision:       $Revision: 1.6 $
*/

#include <stdio.h>
#include <stdlib.h>
#include "mapper.h"
#include "dc.h"

// macros
#ifndef DEBUG
#define DEBUG 2
#endif

// mapper object
struct mapper_s {
    rlist*       in;
    rlist*       out;
    mapper_map_t map;
};

// constraint object
typedef struct mnode {
    dc_cons* cons;
    mapper*  m;
    rnode*   out;
} mnode;


// private functions
static void panic(char* msg);
static void ins(mapper* m, rnode* prev);
static void rem(mapper* m, rnode* prev);
static void cons(rnode* node);


// ---------------------------------------------------------------------
// mapper_new
// ---------------------------------------------------------------------
mapper* mapper_new(rlist* in, rlist* out, mapper_map_t map) {

    mapper* m = malloc(sizeof(mapper));
    if (m == NULL) panic("out of memory");
    m->in  = in;
    m->out = out;
    m->map = map;
    rlist_remove_all(out);
    rlist_subscribe(in, m, (rlist_ins_t)ins, (rlist_rem_t)rem);

    dc_begin_at();
    rnode *pred, *node;
    for (pred = NULL, node = rlist_first(in); 
         node != NULL; 
         node = rlist_next(node)) {

        ins(m, pred);
        pred = node;
    }
    dc_end_at();

    return m;
}


// ---------------------------------------------------------------------
// mapper_delete
// ---------------------------------------------------------------------
void mapper_delete(mapper* m) {
    rnode* node;
    for (node = rlist_first(m->in); 
         node != NULL; 
         node = rlist_next(node)) {

        mnode* c = (mnode*)rlist_param(node);
        dc_del_cons(c->cons);
        free(c);
    }
    rlist_unsubscribe(m->in);
    free(m);
}


// ---------------------------------------------------------------------
//  ins
// ---------------------------------------------------------------------
static void ins(mapper* m, rnode* prev) {

    rnode *in, *out;

    if (prev == NULL) {
        in  = rlist_first(m->in);
        out = rlist_insert_first(m->out, rlist_val(in));
    }
    else {
        in  = prev->next;
        out = rlist_insert_next(m->out, 
                    ((mnode*)rlist_param(prev))->out, rlist_val(in));
    }

    #if DEBUG > 0
    printf("[mapper %p] ins node %p as succ of %p with val %d\n", 
        m, in, prev, rlist_val(in));
    #endif

    mnode* c = malloc(sizeof(mnode));
    if (c == NULL) panic("out of memory");

    rlist_set_param(in, c);

    c->out    = out;
    c->m      = m;
    c->cons   = dc_new_cons((dc_cons_f)cons, in, NULL);
}


// ---------------------------------------------------------------------
//  rem
// ---------------------------------------------------------------------
static void rem(mapper* m, rnode* prev) {
    mnode* c;

    #if DEBUG > 0
    printf("[mapper %p] removing successor of node %p\n", m, prev);
    #endif

    if (prev == NULL) {
        c = (mnode*)rlist_param(rlist_first(m->in));
        rlist_remove_first(m->out);
    }
    else {
        c = (mnode*)rlist_param(prev->next);
        rlist_remove_next(m->out, ((mnode*)rlist_param(prev))->out);
    }

    dc_del_cons(c->cons);
    free(c);
}


// ---------------------------------------------------------------------
//  cons
// ---------------------------------------------------------------------
static void cons(rnode* node) {

    mnode* c = (mnode*)rlist_param(node);

    int val = c->m->map(rlist_val(node));

    #if DEBUG > 0
    printf("[mapper %p] updating node %p to %d\n", c->m, node, val);
    #endif

    rlist_set_val(c->out, val);

}


// ---------------------------------------------------------------------
//  panic
// ---------------------------------------------------------------------
static void panic(char* msg) {
    fprintf(stderr, "[mapper] %s\n", msg);
    exit(1);
}


/* Copyright (C) 2011 Camil Demetrescu

 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 
 * USA
*/
