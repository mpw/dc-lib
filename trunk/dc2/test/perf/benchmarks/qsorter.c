/* =====================================================================
 *  qsorter.c
 * =====================================================================

 *  Author:         (C) 2011 Camil Demetrescu
 *  License:        See the end of this file for license information
 *  Created:        February 1, 2010
 *  Module:         dc/test/perf/benchmarks

 *  Last changed:   $Date: 2011/02/06 14:40:54 $
 *  Changed by:     $Author: demetres $
 *  Revision:       $Revision: 1.12 $
*/

#include <stdio.h>
#include <stdlib.h>

#include "dc.h"
#include "dc_inspect.h"
#include "qsorter.h"
#include "splitter.h"
#include "joiner.h"


// macros
#ifndef DEBUG
#define DEBUG 2
#endif


// intermediate lists
typedef struct {
    rlist*    l_us;   // unsorted left half
    rlist*    r_us;   // unsorted right half
    rlist*    l_s;    // sorted left half
    rlist*    r_s;    // sorted right half
    splitter* s;      // splitter
    joiner*   j;      // joiner
    qsorter*  l;      // left half sorter
    qsorter*  r;      // right half sorter
} inter;


// object structure
struct qsorter_s {
    dc_cons*  cons;
    rlist*    in;     // input unsorted list
    rlist*    out;    // output sorted list
    inter*    i;      // intermediate lists
};


// ---------------------------------------------------------------------
//  dump_sorter_rec
// ---------------------------------------------------------------------
static void dump_sorter_rec(qsorter* s, int indent) {

    if (s->i != NULL) {
        dc_printf("%*s[qsorter %p] :\n", indent, "", s);
        dc_printf("%*s    <in> : ", indent, "");
        rlist_print(s->in);
        dc_printf("%*s    [splitter %p] ", indent, "", s->i->s);
        printf(" (pivot = %d)\n", rlist_val(rlist_first(s->in)));
        dump_sorter_rec(s->i->l, indent + 4);
        dump_sorter_rec(s->i->r, indent + 4);
        dc_printf("%*s    [joiner %p]\n", indent, "", s->i->j);
        dc_printf("%*s    <out> : ", indent, "");
        rlist_print(s->out);
    }
    else {
        dc_printf("%*s[qsorter %p] <in/out> : ", indent, "", s);
        rlist_print(s->out);
    }
}


// ---------------------------------------------------------------------
//  qsorter_dump
// ---------------------------------------------------------------------
void qsorter_dump(qsorter* s) {
    dump_sorter_rec(s, 0);
}


// ---------------------------------------------------------------------
//  tree_dump_rec
// ---------------------------------------------------------------------
static void tree_dump_rec(qsorter* s, int indent) {
    int j;
    for (j=0; j<indent; j++)  printf("%c",(j%4)?' ':'.');
    for (j=0; j<rlist_length(s->in); j++) printf("*");
    printf(" (%d",j);

    if (s->i != NULL) {
        printf(" = %u + ", 
            (s->i->l==NULL) ? 0 : rlist_length(s->i->l->in));
        printf("%u)", 
            (s->i->r==NULL) ? 0 : rlist_length(s->i->r->in));
        printf(" [pivot = %d]\n",rlist_val(rlist_first(s->in)));
        tree_dump_rec(s->i->l, indent + 4);
        tree_dump_rec(s->i->r, indent + 4);
    }
    else printf(")\n");
}


// ---------------------------------------------------------------------
//  qsorter_tree_dump
// ---------------------------------------------------------------------
void qsorter_tree_dump(qsorter* s) {
    tree_dump_rec(s, 0);
}


// ---------------------------------------------------------------------
//  sorter_depth_rec
// ---------------------------------------------------------------------
static int sorter_depth_rec(qsorter* s, int depth, int *num_leaves,
                            int *leaves_depth) {
    if (s->i != NULL) {
        int left_depth = sorter_depth_rec(s->i->l, depth + 1, 
                                            num_leaves, leaves_depth);
        int right_depth = sorter_depth_rec(s->i->r, depth + 1, 
                                            num_leaves, leaves_depth);
        if (left_depth > right_depth) return left_depth+1;
        else return right_depth+1;
    }
    else {
        (*num_leaves)++;
        (*leaves_depth) += depth;
        return depth;
    }
}


// ---------------------------------------------------------------------
//  qsorter__depth
// ---------------------------------------------------------------------
int qsorter_depth(qsorter* s, int *num_leaves, int *leaves_depth) {
    *num_leaves = 0;
    *leaves_depth = 0;
    return sorter_depth_rec(s, 0, num_leaves, leaves_depth);
}


// ---------------------------------------------------------------------
//  qsorter_cons
// ---------------------------------------------------------------------
static void qsorter_cons(qsorter* s) {

    #if DEBUG > 0
    dc_printf("[qsorter %p] exec cons - input list: ", s);
    dc_begin_no_log();
    rlist_print(s->in);
    dc_end_no_log();
    #endif

    // base step: input list is empty, or it contains only one item
    if (rlist_first(s->in) == NULL || 
        rlist_next(rlist_first(s->in)) == NULL) {

        dc_begin_no_log();

        // update the output list so that it is equal to the input list 
        if (rlist_first(s->in) != NULL) {
            if (rlist_first(s->out) == NULL)
                rlist_insert_first(s->out, 
                                   rlist_val(rlist_first(s->in)));
            else {
                rlist_set_val(rlist_first(s->out), 
                              rlist_val(rlist_first(s->in)));
                while (rlist_next(rlist_first(s->out)) != NULL)
                    rlist_remove_next(s->out, rlist_first(s->out));
            }
        }
        else while (rlist_first(s->out) != NULL) 
                rlist_remove_first(s->out);

        // dispose of intermediate lists, if any
        if (s->i != NULL) {

            #if DEBUG > 0
            dc_printf("[qsorter %p] deleting sub-sorters\n", s);
            #endif

            joiner_delete(s->i->j);
            qsorter_delete(s->i->l);
            qsorter_delete(s->i->r);
            splitter_delete(s->i->s);
            rlist_delete(s->i->l_us);
            rlist_delete(s->i->r_us);
            rlist_delete(s->i->l_s);
            rlist_delete(s->i->r_s);
            free(s->i);
            s->i = NULL;
        }

        dc_end_no_log();
    }

    // list contains more than one item: maintain intermediate lists
    else if (s->i == NULL) {

        #if DEBUG > 0
        dc_printf("[qsorter %p] creating sub-sorters\n", s);
        #endif

        dc_begin_no_log();

        s->i = (inter*)malloc(sizeof(inter));

        s->i->l_us = rlist_new();
        s->i->r_us = rlist_new();
        s->i->l_s  = rlist_new();
        s->i->r_s  = rlist_new();
        s->i->s    = splitter_new(s->in, s->i->l_us, s->i->r_us);
        s->i->l    = qsorter_new(s->i->l_us, s->i->l_s);
        s->i->r    = qsorter_new(s->i->r_us, s->i->r_s);
        s->i->j    = joiner_new(s->i->l_s, 
                        &rlist_first(s->in), s->i->r_s, s->out);

        dc_end_no_log();
    }

    #if DEBUG > 2
    dc_printf("[qsorter %p] -- end cons\n", s);
    #endif
}


// ---------------------------------------------------------------------
// qsorter_new
// ---------------------------------------------------------------------
qsorter* qsorter_new(rlist* in, rlist* out) {

    // create sorter object
    qsorter* s = (qsorter*)malloc(sizeof(qsorter));
    if (s == NULL) return NULL;

    #if DEBUG > 0
    dc_printf("[qsorter %p] creating for input list: ", s);
    rlist_print(in);
    #endif

    // setup in and out lists
    s->in  = in;
    s->out = out;

    // no intermediate lists
    s->i = NULL;

    // create constraint
    s->cons = dc_new_cons((dc_cons_f)qsorter_cons, s, NULL);

    return s;
}


// ---------------------------------------------------------------------
// qsorter_delete
// ---------------------------------------------------------------------
void qsorter_delete(qsorter* s) {

    #if DEBUG > 0
    dc_printf("[qsorter %p] deleting\n", s);
    #endif

    // delete constraint
    dc_del_cons(s->cons);

    // dispose of intermediate lists, if any
    if (s->i) {
        dc_begin_at();
        joiner_delete(s->i->j);
        qsorter_delete(s->i->l);
        qsorter_delete(s->i->r);
        splitter_delete(s->i->s);
        rlist_delete(s->i->l_us);
        rlist_delete(s->i->r_us);
        rlist_delete(s->i->l_s);
        rlist_delete(s->i->r_s);
        free(s->i);
        dc_end_at();
    }

    free(s);
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
