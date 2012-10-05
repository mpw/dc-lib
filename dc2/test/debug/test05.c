// =====================================================================
//  dc/test/test05.c
// =====================================================================

//  Author:         (C) 2010-2011 Camil Demetrescu and Andrea Ribichini
//  License:        See the end of this file for license information
//  Created:        January 12, 2011
//  Module:         dc

//  Last changed:   $Date: 2011/03/03 16:36:23 $
//  Changed by:     $Author: demetres $
//  Revision:       $Revision: 1.3 $


#include "dc.h"
#include "rm.h"
#include "dc_inspect.h"

// typedefs
typedef enum {PLUS, MINUS, TIMES, DIV} op_t;

typedef long T;

typedef struct tagnode_t {
    T num;
    op_t op;
    struct tagnode_t* left;
    struct tagnode_t* right;
} node_t;
 
 
// macros
#define DUMP_TREE           1
#define CHECK_CORRECTNESS   1
#define MALLOC              dc_malloc
#define N                   8
#define TRIALS              1


// global vars
int index_g;


// function prototypes
node_t* exptrees_random(int num_nodes);
int exptrees_count_leaves(node_t *root);
void exptrees_fill_leaf_ptr_array_ext(node_t *root, 
                                      node_t **leaf_ptr_array);
void exptrees_fill_leaf_ptr_array(node_t *root, 
                                  node_t **leaf_ptr_array);
T exptrees_conv_eval(node_t *root);


// constraint
void op_h(node_t* p) {
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

		dc_new_cons((dc_cons_f)op_h, node, NULL);
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

int exptrees_count_leaves (node_t *root) {
	if (root==NULL)
		return 0;
  
	if (root->left==NULL && root->right==NULL)
		return 1;
	else
		return exptrees_count_leaves (root->left) + 
              exptrees_count_leaves (root->right);
}

void exptrees_fill_leaf_ptr_array_ext(node_t *root, 
                                      node_t **leaf_ptr_array) {
    index_g = 0;
    exptrees_fill_leaf_ptr_array(root, leaf_ptr_array);
}

// set index_g=0 before calling!!!
void exptrees_fill_leaf_ptr_array(node_t *root, 
                                  node_t **leaf_ptr_array) {

	if (root==NULL)
		return;
  
	if (root->left==NULL && root->right==NULL) {    
		leaf_ptr_array[index_g]=root;
		index_g++;
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
		return exptrees_conv_eval (root->left) + 
               exptrees_conv_eval (root->right);
	if (root->op==MINUS)
		return exptrees_conv_eval (root->left) - 
               exptrees_conv_eval (root->right);
	if (root->op==TIMES)
		return exptrees_conv_eval (root->left) * 
               exptrees_conv_eval (root->right);
	if (root->op==DIV)
		return exptrees_conv_eval (root->left) + 
               exptrees_conv_eval (root->right);

    return 0;
}


int main () {

	node_t *root, **leaf_array;
	int num_leaves, i;
	T first_result;

    dc_init();

    rm_make_dump_file("test05-start-"OPT".log");

	// initializes rand generator...
	srand(123);

	// creates exptree...
	root = exptrees_random(N);

	// prints input size...
	printf("\nN = %d\n", N);

	// counts leaves
	num_leaves = exptrees_count_leaves(root);
	printf("num leaves = %d\n\n", num_leaves);

	// prints tree structure (for debugging purposes)...
    #if DUMP_TREE
	printf ("tree structure:\n");
	exptrees_print (root);
	printf ("\n\n");
    #endif

	// result of initial eval
	printf ("result of tree evaluation = %ld\n", root->num);

	first_result = root->num;

    #if CHECK_CORRECTNESS
	T first_conv_result = exptrees_conv_eval(root);
	printf ("result of conventional eval = %ld\n", first_conv_result);
    if (root->num != first_conv_result) {
        printf("evaluation error.");
        exit(1);
    }
    #endif

	// allocates mem for leaf pointer array...
	leaf_array = (node_t**)malloc(num_leaves*sizeof(node_t*));

	// fills leaf pointer array...
	index_g = 0;
	exptrees_fill_leaf_ptr_array(root, leaf_array);

	// test loop...
    int j;
    for (j = 0; j < TRIALS; j++) {

        printf ("running test loop %d\n", j);
        for (i = 0; i < num_leaves; i++) {

            T backup = (leaf_array[i])->num;

            dc_begin_at();
            (leaf_array[i])->num = 100;
            dc_end_at();

            #if CHECK_CORRECTNESS
            if (exptrees_conv_eval(root) != root->num) {
                printf("wrong result of intermediate "
                       "tree eval = %ld\n\n", 
                   root->num);
                exit(1);
            }
            #endif

            dc_begin_at();
            (leaf_array[i])->num = backup;
            dc_end_at();

            if (first_result != root->num) {
                printf ("new tree value at iteration %d: %ld\n", 
                        i, root->num);
                printf ("conventional value: %ld\n", 
                        exptrees_conv_eval(root));
            } 
    
            if (i == num_leaves-1 && first_result != root->num)
                printf ("new tree value in test loop = %ld\n\n", 
                        root->num);
        }
    }

    // dump DC data structures
    dc_dump_cons_list(NULL);
    dc_dump_cell_list(NULL);

	// just checking
	printf("final tree value = %ld\n", root->num);
    printf("number of constraints: %u\n", dc_get_num_cons());

	// cleanup
	free(leaf_array);
  
	return 0;
}


// Copyright (C) 2010-2011 Camil Demetrescu and Andrea Ribichini

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
