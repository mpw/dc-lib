/* =====================================================================
 *  merger.c
 * =====================================================================

 *  Author:         (c) 2011 Camil Demetrescu
 *  License:        See the end of this file for license information
 *  Created:        January 30, 2011
 *  Module:         dc/test/perf/benchmarks

 *  Last changed:   $Date: 2011/02/07 14:22:27 $
 *  Changed by:     $Author: demetres $
 *  Revision:       $Revision: 1.9 $
*/

#include <stdio.h>
#include <stdlib.h>
#include "merger.h"
#include "dc.h"
#include "dc_inspect.h"

// macros
#ifndef DEBUG
#define DEBUG 2
#endif

#define MERGER_CHECK 0

// constraint object (20 bytes on 32-bit platforms)
typedef struct mnode mnode;
struct mnode {
    dc_cons* cons;
    merger*  m;
    rnode*   out;
    mnode*   prev;
    mnode*   next;
};

// merger object (12 bytes on 32-bit platforms)
struct merger_s {
    rlist*       in1;
    rlist*       in2;
    rlist*       out;
    mnode*       first;
};


// private functions
static void          panic(char* msg);
static inline mnode* make_mnode(merger* m, rnode* in);
static void          ins1(merger* m, rnode* prev);
static void          ins2(merger* m, rnode* prev);
static inline void   ins(merger* m, rnode* prev, rnode* n);
static void          rem1(merger* m, rnode* prev);
static void          rem2(merger* m, rnode* prev);
static inline void   rem(merger* m, rnode* n);
static void          cons(rnode* node);


// ---------------------------------------------------------------------
//  check
// ---------------------------------------------------------------------
#if MERGER_CHECK
static void check(merger* m, char* msg) {

    rnode* p;
    mnode* c;

    if (m->first != NULL && m->first->prev != NULL)
        exit(printf("wrong prev field of first mnode in %s\n", msg));

    for (c = m->first; c != NULL; c = c->next) {
        if (c->next != NULL) {
            if (rlist_val(c->out) > rlist_val(c->next->out))
                exit(printf("wrong mnode list in %s\n", msg));
            if (c->next->prev != c) 
                exit(printf("wrong prev in %s\n", msg));
        }
    }

    for (p = rlist_first(m->out); p != NULL; p = rlist_next(p))
        if (rlist_next(p) != NULL && 
            rlist_val(p) > rlist_val(rlist_next(p))) {
            exit(printf("wrong out list in %s\n", msg));
        }
}
#endif


// ---------------------------------------------------------------------
// merger_new
// ---------------------------------------------------------------------
merger* merger_new(rlist* in1, rlist* in2, rlist* out) {

    merger* m = malloc(sizeof(merger));
    if (m == NULL) panic("out of memory");

    #if DEBUG > 0
    dc_printf("[merger %p] creating for input lists: \n", m);
    rlist_print(in1);
    rlist_print(in2);
    #endif

    m->in1     = in1;
    m->in2     = in2;
    m->out     = out;
    m->first   = NULL;
    rlist_remove_all(out);
    rlist_subscribe(in1, m, (rlist_ins_t)ins1, (rlist_rem_t)rem1);
    rlist_subscribe(in2, m, (rlist_ins_t)ins2, (rlist_rem_t)rem2);

    dc_begin_at();
    rnode *node1 = rlist_first(in1), 
          *node2 = rlist_first(in2), 
          *node,
          *out_node = NULL;
    mnode *curr, *prev = NULL;

    for (;;) {
        
        if (node2 == NULL) {
            if (node1 == NULL) break;
            node = node1;
            node1 = rlist_next(node1);            
        }
        else if (node1 == NULL) {
            node = node2;
            node2 = rlist_next(node2);
        }
        else if (rlist_val(node1) < rlist_val(node2)) {
            node = node1;
            node1 = rlist_next(node1);            
        }
        else {
            node = node2;
            node2 = rlist_next(node2);            
        }

        curr = make_mnode(m, node);
        curr->prev = prev;

        if (out_node == NULL) {
            m->first = curr;
            out_node = rlist_insert_first(out, rlist_val(node)); 
        }
        else {
            prev->next = curr;
            out_node = rlist_insert_next(out, out_node, 
                                         rlist_val(node));  
        }
        curr->out = out_node;
        prev = curr;
    }

    if (prev != NULL) prev->next = NULL;

    #if MERGER_CHECK
    check(m, "merger_new");
    #endif

    dc_end_at();

    return m;
}


// ---------------------------------------------------------------------
// merger_delete
// ---------------------------------------------------------------------
void merger_delete(merger* m) {

    #if DEBUG > 0
    dc_printf("[merger %p] deleting\n", m);
    #endif

    mnode* c = m->first;
    while (c != NULL) {
        mnode* dead = c;
        c = c->next;
        dc_del_cons(dead->cons);
        free(dead);
    }

    rlist_unsubscribe(m->in1);
    rlist_unsubscribe(m->in2);

    free(m);
}


// ---------------------------------------------------------------------
//  make_mnode
// ---------------------------------------------------------------------
static inline mnode* make_mnode(merger* m, rnode* in) {
    mnode* c = malloc(sizeof(mnode));
    if (c == NULL) panic("out of memory");

    rlist_set_param(in, c);

    c->m      = m;
    c->cons   = dc_new_cons((dc_cons_f)cons, in, NULL);
    if (c->cons == NULL) panic("out of memory");

    return c;
}


// ---------------------------------------------------------------------
//  ins1
// ---------------------------------------------------------------------
static void ins1(merger* m, rnode* prev) {
    ins(m, prev, prev == NULL ? rlist_first(m->in1) : rlist_next(prev));
}


// ---------------------------------------------------------------------
//  ins2
// ---------------------------------------------------------------------
static void ins2(merger* m, rnode* prev) {
    ins(m, prev, prev == NULL ? rlist_first(m->in2) : rlist_next(prev));
}


// ---------------------------------------------------------------------
//  ins
// ---------------------------------------------------------------------
static inline void ins(merger* m, rnode* prev, rnode* n) {

    mnode *c, *p, *q;
    int v;

    if (prev == NULL) {
        p = NULL;
        q = m->first;
    }
    else {
        p = (mnode*)rlist_param(prev);
        q = p->next;
    }

    v = rlist_val(n);

    #if DEBUG > 0
    dc_printf("[merger %p] inserting node %p as successor of %p "
              "with val %d\n", m, n, prev, v);
    #endif

    c = make_mnode(m, n);

    // seek proper position in sorted order
    while (q != NULL && rlist_val(q->out) < v) {
        p = q;
        q = q->next;
    }

    // add new node to output list
    if (p == NULL) {
        c->prev = NULL;
        c->next = m->first;
        m->first = c;
        if (c->next != NULL) c->next->prev = c;
        c->out = rlist_insert_first(m->out, v);
        #if MERGER_CHECK
        check(m, "ins - p == NULL");
        #endif
    }
    else {
        c->prev = p;
        c->next = q;
        p->next = c;
        if (q != NULL) q->prev = c;
        c->out = rlist_insert_next(m->out, p->out, v);
        #if MERGER_CHECK
        check(m, "ins - p != NULL");
        #endif
    }
}


// ---------------------------------------------------------------------
//  rem1
// ---------------------------------------------------------------------
static void rem1(merger* m, rnode* prev) {
    rem(m, prev == NULL ? rlist_first(m->in1) : rlist_next(prev));
}


// ---------------------------------------------------------------------
//  rem2
// ---------------------------------------------------------------------
static void rem2(merger* m, rnode* prev) {
    rem(m, prev == NULL ? rlist_first(m->in2) : rlist_next(prev));
}

// ---------------------------------------------------------------------
//  rem
// ---------------------------------------------------------------------
static inline void rem(merger* m, rnode* n) {

    mnode* c = (mnode*)rlist_param(n);

    dc_del_cons(c->cons);

    if (c->prev == NULL) {
        m->first = c->next;
        rlist_remove_first(m->out);
    }
    else {
        c->prev->next = c->next;
        rlist_remove_next(m->out, c->prev->out);
    }

    if (c->next != NULL) c->next->prev = c->prev;

    free(c);

    #if MERGER_CHECK
    check(m, "rem");
    #endif
}


// ---------------------------------------------------------------------
//  cons
// ---------------------------------------------------------------------
static void cons(rnode* node) {

    mnode* c = (mnode*)rlist_param(node);

    #if DEBUG > 0
    dc_printf("[merger %p] exec cons - updating node %p "
              "from %d to %d\n", 
        c->m, node, rlist_val(c->out), rlist_val(node));
    #endif

    // input and output nodes are equal: skip
    if (rlist_val(c->out) == rlist_val(node)) return;

    // output node may scroll to the left
    if (rlist_val(c->out) < rlist_val(node)) {
    }

    // output node may scroll to the right
    else {
    }

    rlist_set_val(c->out, rlist_val(node));

    #if MERGER_CHECK
    dc_begin_no_log();
    check(c->m, "cons");
    dc_end_no_log();
    #endif
}


// ---------------------------------------------------------------------
//  panic
// ---------------------------------------------------------------------
static void panic(char* msg) {
    dc_printf("[merger] %s\n", msg);
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
