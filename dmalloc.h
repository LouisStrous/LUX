/* declares Free, Malloc, Calloc, and Realloc if DEBUG is set, and define
   macros with all-lowercase letters to point at the capitalized version;
   or redefines the capitalized versions as the standard versions
   if DEBUG is not set.
   The idea is that the user uses the standard calling forms and gets either
   the standard behavior (if DEBUG is not set) or the "enhanced" behavior
   (if DEBUG is set).  The capitalized versions may still be used in older
   parts of the code, and these will be replaced by the standard forms
   LS 9jul95 31jul98 */

#ifndef COMPILING_DEBUG_C
#ifdef DEBUG
#include <stddef.h>
#include <stdio.h>
  void Free(void *), *Malloc(size_t), *Calloc(size_t, size_t),
       *Realloc(void *, size_t);
  FILE *Fopen(const char *, const char *), *Tmpfile(void);
  Int  Fclose(FILE *);
#define free	Free
#define malloc	Malloc
#define calloc	Calloc
#define realloc	Realloc
#define fopen	Fopen
#define tmpfile	Tmpfile
#define fclose	Fclose
#else
#include <stdlib.h>
#include <stdio.h>
#define Free	free
#define Malloc	malloc
#define Calloc	calloc
#define Realloc	realloc
#define Fopen	fopen
#define Tmpfile	tmpfile
#define Fclose	fclose
#endif
#endif

#if MALLOPT
#include <sys/types.h>
#include <malloc.h>
#endif
