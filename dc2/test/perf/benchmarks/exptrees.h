/* =====================================================================
 *  exptrees.h
 * =====================================================================

 *  Author:         (c) 2010 Camil Demetrescu, Andrea Ribichini
 *  License:        See the end of this file for license information
 *  Created:        October 13, 2010
 *  Module:         dc/test

 *  Last changed:   $Date: 2012/09/25 14:40:24 $
 *  Changed by:     $Author: ribbi $
 *  Revision:       $Revision: 1.2 $
*/


typedef enum {PLUS,MINUS,TIMES,DIV} op_t;

typedef long T;

typedef struct tagnode_t {
    T num;
    op_t op;
    struct tagnode_t* left;
    struct tagnode_t* right;
} node_t;
 
node_t* exptrees_random(int num_nodes);
int exptrees_count_leaves (node_t *root);
void exptrees_fill_leaf_ptr_array_ext (node_t *root, node_t **leaf_ptr_array);
void exptrees_fill_leaf_ptr_array (node_t *root, node_t **leaf_ptr_array);
T exptrees_conv_eval (node_t *root);

/* Copyright (C) 2010 Camil Demetrescu, Andrea Ribichini

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
