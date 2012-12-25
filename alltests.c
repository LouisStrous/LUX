#include <malloc.h> /* for __malloc_initialize_hook(1) malloc(1) */
#include <stddef.h> /* for NULL(1) */
#include <stdio.h> /* for FILE(10) printf(1) puts(1) */
#include "debugmalloc.h"

/* Override initializing hook from the C library. */
void (*__malloc_initialize_hook)(void) = my_init_hook;

Int main(Int narg, char *argv[]) {
  Int bad = 0;
  extern Int num_assertions;

  Int test_hershey(void);

  bad += test_hershey();
  printf("\n%u Test(s), %d Failure(s)\n", num_assertions, bad);
  return bad != 0;
}
