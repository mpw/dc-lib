/* =====================================================================
 *  joiner.c
 * =====================================================================

 *  Author:         (c) 2011 Camil Demetrescu
 *  License:        See the end of this file for license information
 *  Created:        January 30, 2011
 *  Module:         dc/test/perf/benchmarks

 *  Last changed:   $Date: 2011/02/05 08:02:30 $
 *  Changed by:     $Author: demetres $
 *  Revision:       $Revision: 1.12 $
*/

#include <stdio.h>
#include <stdlib.h>
#include "joiner.h"
#include "dc.h"
#include "dc_inspect.h"

// macros
#ifndef DEBUG
#define DEBUG 2
#endif

// joiner object (24 bytes on 32-bit platforms)
struct joiner_s {
    rlist*       in1;
    rlist*       in2;
    rlist*       out;
    rnode**      sep_in;
    rnode*       sep_out;
    dc_cons*     sep_cons;
};

// constraint object (12 bytes on 32-bit platforms)
typedef struct jnode {
    dc_cons* cons;
    joiner*  m;
    rnode*   out;
} jnode;


// private functions
static void        panic(char* msg);
static inline void make_jnode(joiner* m, rnode* in, rnode* out);
static void        ins1(joiner* m, rnode* prev);
static void        ins2(joiner* m, rnode* prev);
static void        rem1(joiner* m, rnode* prev);
static void        rem2(joiner* m, rnode* prev);
static void        cons(rnode* node);
static void        sep_cons(joiner* m);


// ---------------------------------------------------------------------
// joiner_new
// ---------------------------------------------------------------------
joiner* joiner_new(rlist* in1, rnode** sep_in, rlist* in2, rlist* out) {

    joiner* m = malloc(sizeof(joiner));
    if (m == NULL) panic("out of memory");

    #if DEBUG > 0
    dc_printf("[joiner %p] creating for input lists: \n", m);
    rlist_print(in1);
    rlist_print(in2);
    #endif

    m->in1     = in1;
    m->sep_in  = sep_in;
    m->sep_out = NULL;
    m->in2     = in2;
    m->out     = out;
    rlist_remove_all(out);
    rlist_subscribe(in1, m, (rlist_ins_t)ins1, (rlist_rem_t)rem1);
    rlist_subscribe(in2, m, (rlist_ins_t)ins2, (rlist_rem_t)rem2);

    dc_begin_at();
    rnode *node, *prev_out = NULL;

    // append nodes in first list
    for (node = rlist_first(in1); 
         node != NULL; 
         node = rlist_next(node)) {

        if (node == rlist_first(in1))
             prev_out = rlist_insert_first(out, rlist_val(node));
        else prev_out = rlist_insert_next(out, prev_out, 
                                          rlist_val(node));

        make_jnode(m, node, prev_out);
    }

    // append separator
    if (prev_out == NULL) 
         prev_out = rlist_insert_first(out, rlist_val(*sep_in));
    else prev_out = rlist_insert_next(out, prev_out, 
                                      rlist_val(*sep_in));
    m->sep_out = prev_out;
    m->sep_cons = dc_new_cons((dc_cons_f)sep_cons, m, NULL);
    if (m->sep_cons == NULL) panic("out of memory");

    // append nodes in second list
    for (node = rlist_first(in2); 
         node != NULL; 
         node = rlist_next(node)) {

        prev_out = rlist_insert_next(out, prev_out, rlist_val(node));

        make_jnode(m, node, prev_out);
    }

    dc_end_at();

    return m;
}


// ---------------------------------------------------------------------
// joiner_delete
// ---------------------------------------------------------------------
void joiner_delete(joiner* m) {
    rnode* node;

    #if DEBUG > 0
    dc_printf("[joiner %p] deleting\n", m);
    #endif

    for (node = rlist_first(m->in1); 
         node != NULL; 
         node = rlist_next(node)) {

        jnode* c = (jnode*)rlist_param(node);
        dc_del_cons(c->cons);
        free(c);
    }

    for (node = rlist_first(m->in2); 
         node != NULL; 
         node = rlist_next(node)) {

        jnode* c = (jnode*)rlist_param(node);
        dc_del_cons(c->cons);
        free(c);
    }

    dc_del_cons(m->sep_cons);

    rlist_unsubscribe(m->in1);
    rlist_unsubscribe(m->in2);

    free(m);
}


// ---------------------------------------------------------------------
//  make_jnode
// ---------------------------------------------------------------------
static inline void make_jnode(joiner* m, rnode* in, rnode* out) {
    jnode* c = malloc(sizeof(jnode));
    if (c == NULL) panic("out of memory");

    rlist_set_param(in, c);

    c->out    = out;
    c->m      = m;
    c->cons   = dc_new_cons((dc_cons_f)cons, in, NULL);
    if (c->cons == NULL) panic("out of memory");
}


// ---------------------------------------------------------------------
//  ins1
// ---------------------------------------------------------------------
static void ins1(joiner* m, rnode* prev) {

    rnode *in1, *out;

    if (prev == NULL) {
        in1 = rlist_first(m->in1);
        out = rlist_insert_first(m->out, rlist_val(in1));
    }
    else {
        in1 = prev->next;
        out = rlist_insert_next(m->out, 
                    ((jnode*)rlist_param(prev))->out, rlist_val(in1));
    }

    #if DEBUG > 0
    dc_printf("[joiner %p] ins node %p of list 1 "
           "as succ of %p with val %d\n", 
        m, in1, prev, rlist_val(in1));
    #endif

    make_jnode(m, in1, out);
}


// ---------------------------------------------------------------------
//  ins2
// ---------------------------------------------------------------------
static void ins2(joiner* m, rnode* prev) {

    rnode *in2, *out;

    if (prev == NULL) {
        in2 = rlist_first(m->in2);
        out = rlist_insert_next(m->out, m->sep_out, rlist_val(in2));
    }
    else {
        in2 = prev->next;
        out = rlist_insert_next(m->out, 
                    ((jnode*)rlist_param(prev))->out, rlist_val(in2));
    }

    #if DEBUG > 0
    dc_printf("[joiner %p] ins node %p of list 2 "
           "as succ of %p with val %d\n", 
        m, in2, prev, rlist_val(in2));
    #endif

    make_jnode(m, in2, out);
}


// ---------------------------------------------------------------------
//  rem1
// ---------------------------------------------------------------------
static void rem1(joiner* m, rnode* prev) {
    jnode* c;

    #if DEBUG > 0
    dc_printf("[joiner %p] removing successor of node %p in list 1\n", 
        m, prev);
    #endif

    if (prev == NULL) {
        c = (jnode*)rlist_param(rlist_first(m->in1));
        rlist_remove_first(m->out);
    }
    else {
        c = (jnode*)rlist_param(prev->next);
        rlist_remove_next(m->out, ((jnode*)rlist_param(prev))->out);
    }

    dc_del_cons(c->cons);
    free(c);
}


// ---------------------------------------------------------------------
//  rem2
// ---------------------------------------------------------------------
static void rem2(joiner* m, rnode* prev) {
    jnode* c;

    #if DEBUG > 0
    dc_printf("[joiner %p] removing successor of node %p in list 2\n", 
        m, prev);
    #endif

    if (prev == NULL) {
        c = (jnode*)rlist_param(rlist_first(m->in2));
        rlist_remove_next(m->out, m->sep_out);
    }
    else {
        c = (jnode*)rlist_param(prev->next);
        rlist_remove_next(m->out, ((jnode*)rlist_param(prev))->out);
    }

    dc_del_cons(c->cons);
    free(c);
}


// ---------------------------------------------------------------------
//  cons
// ---------------------------------------------------------------------
static void cons(rnode* node) {

    jnode* c = (jnode*)rlist_param(node);

    #if DEBUG > 0
    dc_printf("[joiner %p] exec cons - updating node %p to %d\n", 
        c->m, node, rlist_val(node));
    #endif

    rlist_set_val(c->out, rlist_val(node));
}


// ---------------------------------------------------------------------
//  sep_cons
// ---------------------------------------------------------------------
static void sep_cons(joiner* m) {

    if (*m->sep_in == NULL) return;

    #if DEBUG > 0
    dc_printf("[joiner %p] exec cons - updating separator to %d\n", 
        m, rlist_val(*m->sep_in));
    #endif

    rlist_set_val(m->sep_out, rlist_val(*m->sep_in));
}


// ---------------------------------------------------------------------
//  panic
// ---------------------------------------------------------------------
static void panic(char* msg) {
    dc_printf("[joiner] %s\n", msg);
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
