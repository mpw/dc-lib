/* =====================================================================
 *  exptrees.c
 * =====================================================================

 *  Author:         (c) 2010 Camil Demetrescu, Andrea Ribichini
 *  License:        See the end of this file for license information
 *  Created:        October 13, 2010
 *  Module:         dc/test

 *  Last changed:   $Date: 2012/09/25 14:40:24 $
 *  Changed by:     $Author: ribbi $
 *  Revision:       $Revision: 1.2 $
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/times.h>
#include <sys/param.h>

#include "exptrees.h"
#include "dc.h"

#define MAKE_DUMP 0
//#define CHECK_CORRECTNESS 1
#define MALLOC dc_malloc

//num nodes...
//#define N 1000000


//global variables...
int g_index;
int g_num_att;

void op_h(node_t* p) {
  
	g_num_att++;
  
	T r_l = p->left->num;
	T r_r = p->right->num;

		 if (p->op == PLUS)  p->num = r_l + r_r; 
	else if (p->op == MINUS) p->num = r_l - r_r; 
	else if (p->op == TIMES) p->num = r_l * r_r; 
	else                     p->num = r_l + r_r; 
}

// leaf constructor
node_t* exptrees_leaf (T num) {
	node_t* p = MALLOC(sizeof(node_t));
	p->num = num;
	p->left = p->right = NULL;
	return p;
}

// node constructor
node_t* exptrees_node (op_t op) {
	node_t* p = MALLOC(sizeof(node_t));
	p->op = op;
	return p;
}

// build a random tree with num_nodes nodes, 
// the tree can have some extra nodes (probably just one).
int exptrees_random_rec(int num_nodes, node_t** dest) {  
	if (num_nodes > 2) {

		node_t* node = exptrees_node (rand() % 4);
		*dest = node;

		int n = num_nodes - 1;
    
		int n_l = exptrees_random_rec (n / 2, &node->left);
		int n_r = exptrees_random_rec (n - n_l, &node->right);

		dc_new_cons((dc_cons_f) op_h, node, NULL);
		//op_h(node);

		return (n_l + n_r + 1);
	}
	else if (num_nodes == 1) {
		*dest = exptrees_leaf (rand() % 100);
		return 1;
	}
	else if (num_nodes == 2) {
		return (exptrees_random_rec (3, dest));
	}

	return 0;
}


node_t* exptrees_random(int num_nodes) {
	node_t* root;
	exptrees_random_rec (num_nodes, &root);
	return root;
}


void exptrees_print (node_t* root) {
	if (root!=NULL) {
		
		printf ("(");
		exptrees_print (root->left);
		if (root->left==NULL && root->right==NULL)
			printf (" %ld ", root->num);
		else
		{
				 if (root->op == PLUS)  printf ("+");
			else if (root->op == MINUS) printf ("-");
			else if (root->op == TIMES) printf ("*");
			else                        printf ("/");
		}
		exptrees_print (root->right);
		printf (")");
	}  
}

int exptrees_count_leaves (node_t *root)
{
	if (root==NULL)
		return 0;
  
	if (root->left==NULL && root->right==NULL)
		return 1;
	else
		return exptrees_count_leaves (root->left) + exptrees_count_leaves (root->right);
}

void exptrees_fill_leaf_ptr_array_ext (node_t *root, node_t **leaf_ptr_array)
{
    g_index=0;
    exptrees_fill_leaf_ptr_array (root, leaf_ptr_array);
}

// set g_index=0 before calling!!!
void exptrees_fill_leaf_ptr_array (node_t *root, node_t **leaf_ptr_array)
{
	if (root==NULL)
		return;
  
	if (root->left==NULL && root->right==NULL) {    
		leaf_ptr_array[g_index]=root;
		g_index++;
	}
	else {
		exptrees_fill_leaf_ptr_array (root->left, leaf_ptr_array);
		exptrees_fill_leaf_ptr_array (root->right, leaf_ptr_array);
	}
}

T exptrees_conv_eval (node_t *root) {

	if (root->left==NULL && root->right==NULL)
		return root->num;
  
	if (root->op==PLUS)
		return exptrees_conv_eval (root->left) + exptrees_conv_eval (root->right);
	if (root->op==MINUS)
		return exptrees_conv_eval (root->left) - exptrees_conv_eval (root->right);
	if (root->op==TIMES)
		return exptrees_conv_eval (root->left) * exptrees_conv_eval (root->right);
	if (root->op==DIV)
		return exptrees_conv_eval (root->left) + exptrees_conv_eval (root->right);

    return 0;
}

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
