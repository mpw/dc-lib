// =====================================================================
//  dc/test/test07.c
// =====================================================================

//  Author:         (C) 2011 Camil Demetrescu
//  License:        See the end of this file for license information
//  Created:        January 12, 2011
//  Module:         dc

//  Last changed:   $Date: 2011/01/19 15:43:07 $
//  Changed by:     $Author: demetres $
//  Revision:       $Revision: 1.2 $


#include "dc.h"
#include "rm.h"
#include "list.h"
#include "dc_inspect.h"

typedef struct {
    dc_cons* cons;
    size_t   prev_size;
    size_t*  size;
    node_t** head;
} i_list;

void cons(i_list* l);

i_list* i_list_new(size_t* size, node_t** head) {
    i_list* l = malloc(sizeof(i_list));
    l->prev_size = 0;
    l->size = size;
    l->head = head;
    l->cons = dc_new_cons((dc_cons_f)cons, l, NULL);
    return l;
}

void i_list_delete(i_list* l) {
    dc_del_cons(l->cons);
    free(l);
    list_remove_all(l->head);
}

void cons(i_list* l) {

    int i;
    
    // list size was increased: add nodes
    if (*l->size > l->prev_size) {
        for (i = 0; i < *l->size - l->prev_size; ++i)
            list_add_first(l->head, 0);
    }

    // list size was decreased: delete nodes
    else if (*l->size < l->prev_size) {
        for (i = 0; i < l->prev_size - *l->size; ++i)
            list_remove_first(l->head);
    }

    l->prev_size = *l->size;
    
    printf("list: ");
    list_print(*l->head);
}

int main() {

    dc_init();

    rm_make_dump_file("test07-start-"OPT".log");

    volatile size_t*   size = dc_malloc(sizeof(size_t));
    node_t** head = dc_malloc(sizeof(node_t*));

    *size = 5;

    i_list* l = i_list_new((size_t*)size, head);

    *size = 3;
    
    *size = 9;

    *size = 0;

    *size = 13;

    i_list_delete(l);

    rm_make_dump_file("test07-end-"OPT".log");

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
