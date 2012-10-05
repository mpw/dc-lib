#include <stdio.h>
#include <stdlib.h>
#include "rm.h"

#define LIBC   0
#define RM     1
#define CUSTOM 2
#define ALLOCATOR RM

#if ALLOCATOR == LIBC
#define alloc malloc
#elif ALLOCATOR == RM
#define alloc rm_malloc
#elif ALLOCATOR == CUSTOM
#define alloc mymalloc
#endif

//num nodes...
#define N 511

#define lvalue(addr,type) (*(rm_is_reactive(addr) ? ((type*)rm_get_inactive_ptr(addr)) : ((type*)addr)))

enum {PLUS, MINUS, TIMES, DIV};

typedef long T;

typedef struct tagnode_t{
  T num;
  int op;
  struct tagnode_t* left;
  struct tagnode_t* right;
} node_t;

void read_h (void *addr, size_t size)
{ printf("*** read %p\n", addr); }

void write_h (void *addr, size_t size)
{ printf("*** written %p\n", addr); }

#if ALLOCATOR == CUSTOM
#define MAX 65536
void* mymalloc(size_t s) {
  void* res;
  static char* ptr = NULL;
  static char* limit = NULL;
  if (ptr == NULL) {
    ptr = (char*)rm_malloc(MAX);
    if (ptr == NULL) exit(printf("[mymalloc] rm_malloc error\n"));
    limit = ptr + MAX;
  }
  if (ptr == limit) exit(printf("[mymalloc] Out of memory error\n"));
  res = (void*)ptr;
  ptr += s;
  return res;
}
#endif

T f(T a, T b, int op){
  int res;
       if (op == PLUS)  res = a + b; 
  else if (op == MINUS) res = a - b; 
  else if (op == TIMES) res = a * b; 
  else                  res = a + b; 
  return res;
}

T op_h(node_t* p, T v_l, T v_r) {

  node_t* left = lvalue(&p->left, node_t*);
  node_t* right = lvalue(&p->right, node_t*);

  T r_l = lvalue(&left->num, T);
  T r_r = lvalue(&right->num, T);

  int op = lvalue(&p->op,int);

  T res = f(v_l, v_r, op);

  lvalue(&p->num, T) = f(r_l, r_r, op);

  printf ("[%p] left: c=%ld (s=%ld [%p]), right: c=%ld (s=%ld [%p]) -- Op %d: c=%ld (s=%ld) %s\n", 
    p, v_l, r_l, left, v_r, r_r, right, op, res, lvalue(&p->num, T), res != lvalue(&p->num, T) ? "- ERROR" : "");

  return res;
}

// leaf constructor
node_t* exptrees_leaf (T val) {
  node_t* p = alloc(sizeof(node_t));
  if (p==NULL) exit(printf("rm_malloc error\n"));
  lvalue(&p->num, T) = val;
  lvalue(&p->left, node_t*) = lvalue(&p->right, node_t*) = NULL;
  return p;
}

// node constructor
node_t* exptrees_node (int op) {
  node_t* p = alloc(sizeof(node_t));
  if (p==NULL) exit(printf("rm_malloc error\n"));
  lvalue(&p->op, int) = op;
  return p;
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

node_t* exptrees_random(int num_nodes) {
  node_t* root;
  exptrees_random_rec (num_nodes, &root);
  return root;
}

void exptrees_print (node_t* root, int tab, int depth, int* map)
{
  if (root==NULL) return;

  int i;
  for (i=0; i<depth*tab; ++i) printf ( (i%tab == 0 && i>=tab && map[i/tab-1]) ? "|" :" ");

  if (root->left==NULL && root->right==NULL)
    printf ("%ld \n", root->num);
  else {
	     if (root->op == PLUS)  printf ("%ld=+\n", root->num);
	else if (root->op == MINUS) printf ("%ld=-\n", root->num);
	else if (root->op == TIMES) printf ("%ld=*\n", root->num);
	else                        printf ("%ld=+\n", root->num);
  }
  
  map[depth] = 1;
  exptrees_print (root->left,  tab, depth+1, map);
  map[depth] = 0;
  exptrees_print (root->right, tab, depth+1, map);
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

T exptrees_conv_eval_verbose (node_t *root)
{
  T res, left_val, right_val;
  
  if (root->left==NULL && root->right==NULL)
    return root->num;
  
  left_val  = exptrees_conv_eval_verbose (root->left);
  right_val = exptrees_conv_eval_verbose (root->right);

  if (root->op==PLUS)  
    res=left_val + right_val;
  else if (root->op==MINUS)
    res=left_val - right_val;
  else if (root->op==TIMES)
    res=left_val * right_val;
  else
    res=left_val + right_val;
  
  printf ("[%p] left=%ld(%ld) [%p], right=%ld(%ld) [%p] -- Op %d -- %ld(%ld)\n", 
    root, left_val, root->left->num, root->left, right_val, root->right->num, root->right, root->op, res, root->num);
  
  return res;
}

int main ()
{
  node_t *theTreeRoot, **theLeafPtrArray;
  int theNumLeaves, i;
  T theBackup, theFirstResult, theFirstConvResult;
  struct timeval tvBegin, tvEnd;
  double theLoopTime;

  if (-1==rm_init (read_h, write_h, NULL, NULL, 4, 4))
  {
    printf ("rm_init() failed!\n");
    return 0;
  }

  printf("Logging initial rm state (res=%d)\n", rm_make_dump_file("dump_init.txt"));
  
  //prints input size...
  printf ("\nN = %d\n", N);

  //initializes rand generator...
  //srand((unsigned)time (NULL));
  srand (123);
  
  //creates exptree...
  theTreeRoot = exptrees_random (N);

  #if 1
  //counts leaves...  
  theNumLeaves=exptrees_count_leaves (theTreeRoot);
  printf ("Num leaves = %d\n\n", theNumLeaves);
  #endif
  
  //prints tree structure (for debugging purposes)...
  #if 1
  printf ("Tree structure:\n");
  int* map = (int*)calloc(sizeof(int)*100,1);
  exptrees_print (theTreeRoot, 4, 0, map);
  printf ("\n\n");
  #endif

  #if 1
  //result of first eval...
  printf ("Result of tree evaluation = %ld\n", theTreeRoot->num);
  theFirstResult=theTreeRoot->num;
  theFirstConvResult=exptrees_conv_eval_verbose (theTreeRoot);
  printf ("Result of conventional eval = %ld\n\n", theFirstConvResult);
  #endif

  printf("Logging final rm state (res=%d)\n", rm_make_dump_file("dump_final.txt"));

  return 0;
}
