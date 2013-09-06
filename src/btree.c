/* This is file btree.c, part of the ANA distribution.  It contains */
/* wrappers for binary tree management routines so that (1) we can hide */
/* some global variables that are involved, and (2) we can take care */
/* of the memory allocation bookkeeping for the user.  LS 20may99 */

/* Usage:

   bt_initialize(compare, size): initializes the binary tree code for
   a new tree.  It removes any lingering traces of the previous tree,
   if any.  It must be called before the first node is entered into
   the tree.

   Int (*compare)(const void *, const void *): is a function that compares
   the two arguments and returns an integer greater than, equal to, or
   less than zero, depending on whether the first argument is to be
   considered greater than, equal to, or less than the second argument.

   size_t size: is the size in bytes of each node: in a contiguous region
   of memory, the beginning of each next node is separated from the
   beginning of each previous node by a multiple of this size. */

#include <search.h>		/* for tsearch(), tdelete(), twalk() */
#include <stdio.h>		/* for NULL */
#include <malloc.h>		/* for realloc(), malloc() */
#include <string.h>		/* for memcpy() */

/* NOTE: some configurations, including gcc 2.8.1 on SGI Irix 6.3, allow */
/* incrementing of pointers to void, but some others, including MIPS cc on */
/* SGI Irix 6.4, consider that to be an error.  These binary tree routines */
/* are designed to work on objects of any kind, so pointers to void are */
/* most appropriate.  I'll use pointers to char instead to prevent */
/* portability trouble. */

typedef char	node;

static void	*bt_root;
static Int	(*bt_compare)(const void *, const void *);
static void	**bt_blocks = NULL;
static void	*bt_curnode, **bt_curblock;
static Int	bt_nblocks = 0, bt_nfreeblocks = 0, bt_nfreenodes = 0;
static size_t	bt_nodesize;

void	bt_cleanup(void);

#define BT_BLOCK_SET	4096
#define BT_NODE_SET	4096

void *bt_nextnode(void)
/* returns a pointer to some free space in which to store a node of the */
/* binary tree. */
{
  if (bt_nfreenodes) {		/* still have some free nodes */
    bt_curnode = (char *) bt_curnode + bt_nodesize;/* advance pointer */
    bt_nfreenodes--;
    return bt_curnode;		/* return pointer to a free node */
  }
  /* if we get here then we need to allocate space for some more nodes */
  if (!bt_nfreeblocks) {	/* our block table is full */
    bt_nblocks += BT_BLOCK_SET;	/* add some block pointers */
    bt_nfreeblocks = BT_BLOCK_SET;
    /* resize: */
    bt_blocks = (void **) realloc(bt_blocks, bt_nblocks*sizeof(char *));
    /* point to one before the next avaialble block */
    bt_curblock = bt_blocks + (bt_nblocks - BT_BLOCK_SET - 1);
  }
  bt_nfreenodes = BT_NODE_SET;	/* add some space for new nodes */
  bt_nfreeblocks--;
  *++bt_curblock = (void *) malloc(bt_nfreenodes*bt_nodesize);
  bt_curnode = *(char **) bt_curblock;
  bt_nfreenodes--;
  return bt_curnode;
}

void bt_initialize(Int (*compare)(const void *, const void *), size_t size)
/* intialize for a new binary tree.  Only one can be active at any given */
/* time!  <compare> is a function that compares its two node arguments and */
/* returns a positive, zero, or negative number when the second node is */
/* to be considered greater than, equal to, or less than the first node. */
/* <size> is the separation in bytes between adjacent nodes in memory. */
{
  bt_root = NULL;		/* flags new tree */
  bt_compare = compare;		/* store comparison routine */
  bt_nodesize = size;		/* store node size */

  if (bt_blocks)		/* still have an old tree */
    bt_cleanup();		/* get rid of it */
  bt_nextnode();		/* get space for the first node */
}

void *bt_search(const void *key)
/* search for the node <*key> in the current tree.  If not found, then */
/* save the node and install it in the tree.  In any case, return a pointer */
/* to the corresponding branch of the tree. */
{
  void	*q;

  memcpy(bt_curnode, key, bt_nodesize);	/* save it */
  q = *(void **) tsearch(bt_curnode, &bt_root, bt_compare); /* find it and */
				/* return pointer */
  if (q == bt_curnode)		/* it's a new one */
    bt_nextnode();
  return q;
}

void *bt_delete(const void *key)
/* deletes the node <*key> from the binary tree, but does not remove it */
/* from the associated allocated memory. */
{
  return tdelete(key, &bt_root, bt_compare);
}

void bt_walk(void (*action)(const void *, VISIT, Int))
/* walk through the current tree in depth-first, left-to-right fashion */
/* and call routine <action> on each visited node.  <action> has three */
/* arguments: the address of the node that is visited; a key that indicates */
/* the type of visit ("preorder", "postorder", "endorder", "leaf"; defined */
/* in "search.h") depending on whether it is the first, second, or third */
/* visit to the node, or whether it is a leaf; and the level of the node, */
/* with the root being at level zero. */
{
  twalk(bt_root, action);
}

void bt_cleanup(void)
/* free up the allocated memory. */
{
  while (bt_curblock >= bt_blocks)
    free(*bt_curblock--);
  free(bt_blocks);
  bt_nfreenodes = bt_nfreeblocks = bt_nodesize = 0;
  bt_blocks = NULL;
}
