#define _GNU_SOURCE
/* HEADERS */
#include <malloc.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
/* END HEADERS */

#include <sys/types.h>		/* for wait(), fork() */
#include <sys/wait.h>		/* for wait() */
#include <unistd.h>		/* for fork() */
#include <stdlib.h>		/* for exit() */

void my_init_hook(void);

/* Override initializing hook from the C library. */
void (*__malloc_initialize_hook)(void) = my_init_hook;

#include "test_declarations.h"

static jmp_buf dojmp;

Int signals = 0;

static void report_fault(Int sig) {
  extern Int assertion_count;
  extern char *assertion_file;
  extern Int assertion_line;
  extern char *assertion_func;

  printf("\n%s:%d (%s) [%d] {sig=%d}: %s\n",
	 assertion_file? assertion_file: "<unknown file>",
	 assertion_line,
	 assertion_func? assertion_func: "<unknown function>",
	 assertion_count, sig, strsignal(sig));
  signals++;
  longjmp(dojmp, 1);
}

void testcleanup(void) {
}

Int main(Int narg, char *argv[]) {
  Int bad = 0;
  extern Int num_assertions;

  signal(SIGSEGV, report_fault);

  testcleanup();

#include "test_performing.h"

  bad += signals;
  printf("\n%u Test(s), %d Failure(s), %d Signal(s)\n", num_assertions,
	 bad, signals);

  return bad != 0;
}
