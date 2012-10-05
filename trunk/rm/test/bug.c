#include <stdio.h>
#include <stdlib.h>
#include "rm.h"

#define alloc malloc

void read_h (void *addr, size_t size)
{ printf("*** read %p\n", addr); }

void write_h (void *addr, size_t size)
{ printf("*** written %p\n", addr); }


enum {PLUS, MINUS, TIMES, DIV};

typedef long T;

typedef struct tagnode_t{
  T num;
  int op;
  struct tagnode_t* left;
  struct tagnode_t* right;
} node_t;


#define lvalue(addr,type) (*(rm_is_reactive(addr) ? ((type*)rm_get_inactive_ptr(addr)) : ((type*)addr)))


T op_h(node_t* p, T v_l, T v_r) {
    return 0;
}

// leaf constructor
node_t* exptrees_leaf (T val) {
  return NULL;
}

// node constructor
node_t* exptrees_node (int op) {
    return NULL;
}


// build a random tree with num_nodes nodes, 
// the tree can have some extra nodes (probably just one).
T exptrees_random_rec(int num_nodes, node_t** dest) {  

  if (num_nodes > 2) {

    node_t* node = exptrees_node (rand() % 4);
    
    lvalue(dest, node_t*) = node;

    int n = num_nodes - 1;
    
    T v_l = exptrees_random_rec (n / 2, &node->left);
    T v_r = exptrees_random_rec (n / 2, &node->right);

    return op_h(node, v_l, v_r);
  }

  else if (num_nodes == 1) {
    T val = rand() % 2;
    node_t* node = exptrees_leaf (val);
    lvalue(dest, node_t*) = node;
    printf ("[%p] leaf c=%ld (s=%ld)\n", node, val, lvalue(&node->num, T));
    return val;
  }

  else if (num_nodes == 2) {
    return exptrees_random_rec (3, dest);
  }

  abort();
  return 0;
}

int main() {

    if (-1==rm_init (read_h, write_h, NULL, NULL, 4, 4))
    {
        printf ("rm_init() failed!\n");
        return 0;
    }

    printf("Logging initial rm state (res=%d)\n", rm_make_dump_file("dump_init.txt"));

    return 0;
}
