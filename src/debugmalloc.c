/* This is file debugmalloc.c.

Copyright 2013 Louis Strous

This file is part of LUX.

LUX is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation, either version 3 of the License, or (at your
option) any later version.

LUX is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with LUX.  If not, see <http://www.gnu.org/licenses/>.
*/
/* HEADERS */
#include <ctype.h>
#include <malloc.h>
#include <stddef.h>
#include <stdio.h>
/* END HEADERS */
#include "debugmalloc.h"
#include "luxMap.h"

static void *my_malloc_hook(size_t, const void *);
static void my_free_hook(void *, const void *);
static void *my_realloc_hook(void *, size_t, const void *);
static void *my_memalign_hook(size_t, size_t, const void *);
static void *(*old_malloc_hook)(size_t, const void *);
static void *(*old_realloc_hook)(void *, size_t, const void *);
static void *(*old_memalign_hook)(size_t, size_t, const void *);
static void (*old_free_hook)(void *, const void *);

unsigned long num_allocs = 1;
unsigned long net_allocs = 0;

static luxMap mallocmap = NULL;

#define MALLOCMAP 1

void my_init_hook(void) {
  old_malloc_hook = __malloc_hook;
  old_free_hook = __free_hook;
  old_realloc_hook = __realloc_hook;
  old_memalign_hook = __memalign_hook;
#if MALLOCMAP
  mallocmap = luxMap_create_pi();
#endif
  __malloc_hook = my_malloc_hook;
  __free_hook = my_free_hook;
  __realloc_hook = my_realloc_hook;
  __memalign_hook = my_memalign_hook;
}

static void *my_malloc_hook(size_t size, const void *caller) {
  __malloc_hook = old_malloc_hook;
  __free_hook = old_free_hook;
  __realloc_hook = old_realloc_hook;
  __memalign_hook = old_memalign_hook;
  void *result = malloc(size);
  old_malloc_hook = __malloc_hook;
  old_free_hook = __free_hook;
  old_realloc_hook = __realloc_hook;
  old_memalign_hook = __memalign_hook;
#if MALLOCMAP
  if (result)
    luxMap_add_pi(mallocmap, result, num_allocs);
#endif
  if (result) {
    num_allocs++;
    net_allocs++;
  }
  __malloc_hook = my_malloc_hook;
  __free_hook = my_free_hook;
  __realloc_hook = my_realloc_hook;
  __memalign_hook = my_memalign_hook;
  return result;
}

static void *my_memalign_hook(size_t align, size_t size, const void *caller) {
  void *result;

  __malloc_hook = old_malloc_hook;
  __free_hook = old_free_hook;
  __realloc_hook = old_realloc_hook;
  __memalign_hook = old_memalign_hook;
  result = memalign(align, size);
  old_malloc_hook = __malloc_hook;
  old_free_hook = __free_hook;
  old_realloc_hook = __realloc_hook;
  old_memalign_hook = __memalign_hook;
#if MALLOCMAP
  if (result)
    luxMap_add_pi(mallocmap, result, num_allocs);
#endif
  if (result) {
    num_allocs++;
    net_allocs++;
  }
  __malloc_hook = my_malloc_hook;
  __free_hook = my_free_hook;
  __realloc_hook = my_realloc_hook;
  __memalign_hook = my_memalign_hook;
  return result;
}

static void my_free_hook(void *ptr, const void *caller) {
  __malloc_hook = old_malloc_hook;
  __free_hook = old_free_hook;
  __realloc_hook = old_realloc_hook;
  __memalign_hook = old_memalign_hook;
#if MALLOCMAP
  if (ptr)
    luxMap_remove_pi(mallocmap, ptr);
#endif
  free(ptr);
  old_malloc_hook = __malloc_hook;
  old_free_hook = __free_hook;
  old_realloc_hook = __realloc_hook;
  old_memalign_hook = __memalign_hook;
  if (ptr) {
    if (!net_allocs--) {
      puts("debugmalloc.c(my_free_hook): illegal decrement of net_allocs from 0.");
    }
  }
  __malloc_hook = my_malloc_hook;
  __free_hook = my_free_hook;
  __realloc_hook = my_realloc_hook;
  __memalign_hook = my_memalign_hook;
}

static void *my_realloc_hook(void *ptr, size_t size, const void *caller) {
  void *result;

  __malloc_hook = old_malloc_hook;
  __free_hook = old_free_hook;
  __realloc_hook = old_realloc_hook;
  __memalign_hook = old_memalign_hook;
  result = realloc(ptr, size);
  old_malloc_hook = __malloc_hook;
  old_free_hook = __free_hook;
  old_realloc_hook = __realloc_hook;
  old_memalign_hook = __memalign_hook;
#if MALLOCMAP
  if (ptr || size) {
    if (ptr)
      luxMap_remove_pi(mallocmap, ptr);
    if (size)
      luxMap_add_pi(mallocmap, result, num_allocs);
  }
#endif
  if (!ptr && size) {
    num_allocs++;
    net_allocs++;
  } else if (ptr && !size)
    if (!net_allocs--) {
      puts("debugmalloc.c(my_realloc_hook): illegal decrement of net_allocs from 0.");
    }
  __malloc_hook = my_malloc_hook;
  __free_hook = my_free_hook;
  __realloc_hook = my_realloc_hook;
  __memalign_hook = my_memalign_hook;
  return result;
}

char *mallocmap_to_text(void)
{
  void *(*keep_malloc_hook)(size_t, const void *);
  void *(*keep_realloc_hook)(void *, size_t, const void *);
  void *(*keep_memalign_hook)(size_t, size_t, const void *);
  void (*keep_free_hook)(void *, const void *);
  keep_malloc_hook = __malloc_hook;
  keep_realloc_hook = __realloc_hook;
  keep_memalign_hook = __memalign_hook;
  keep_free_hook = __free_hook;
  __malloc_hook = old_malloc_hook;
  __free_hook = old_free_hook;
  __realloc_hook = old_realloc_hook;
  __memalign_hook = old_memalign_hook;
  char *result = luxMap_to_text(mallocmap);
  __malloc_hook = keep_malloc_hook;
  __free_hook = keep_free_hook;
  __realloc_hook = keep_realloc_hook;
  __memalign_hook = keep_memalign_hook;
  return result;
}

#if MALLOCMAP
void dump_allocs(Int max_count)
{
  void *(*keep_malloc_hook)(size_t, const void *);
  void *(*keep_realloc_hook)(void *, size_t, const void *);
  void *(*keep_memalign_hook)(size_t, size_t, const void *);
  void (*keep_free_hook)(void *, const void *);
  keep_malloc_hook = __malloc_hook;
  keep_realloc_hook = __realloc_hook;
  keep_memalign_hook = __memalign_hook;
  keep_free_hook = __free_hook;
  __malloc_hook = old_malloc_hook;
  __free_hook = old_free_hook;
  __realloc_hook = old_realloc_hook;
  __memalign_hook = old_memalign_hook;

  luxMap map = luxMap_create_ip();
  luxIter iter = luxIter_create(mallocmap);
  while (iter) {
    if (!luxMap_add_unique_ip(map, luxIter_value_i(iter),
			      luxIter_key_p(iter))) {
      printf("MULTIPLE OCCURRENCE of num_allocs = %d\nmallocmap:\n%s\n",
	     luxIter_value_i(iter), luxMap_to_text(mallocmap));
    }
    iter = luxIter_next(iter);
  }
  iter = luxIter_create(map);
  Int i = 0;
  Int n = luxMap_key_count(map);
  if (max_count > 0)
    while (n > max_count) {
      iter = luxIter_next(iter);
      n--;
    }
  printf(" #: num_alloc pointer\n");
  while (iter) {
    i++;
    printf("%2d: %9d %p ", i, luxIter_key_i(iter), luxIter_value_p(iter));
    unsigned char *p = luxIter_value_p(iter);
    Int j;
    for (j = 0; j < 12; j++)
      putchar(isprint(p[j])? p[j]: '.');
    putchar(' ');
    for (j = 0; j < 12; j++)
      printf("%02x ", p[j]);
    putchar('\n');
    
    iter = luxIter_next(iter);
  }
  luxMap_delete(map);

  __malloc_hook = keep_malloc_hook;
  __free_hook = keep_free_hook;
  __realloc_hook = keep_realloc_hook;
  __memalign_hook = keep_memalign_hook;
}
#endif

Int check_net_allocs(unsigned long expected)
{
  Int bad = 0;

  void *(*keep_malloc_hook)(size_t, const void *);
  void *(*keep_realloc_hook)(void *, size_t, const void *);
  void *(*keep_memalign_hook)(size_t, size_t, const void *);
  void (*keep_free_hook)(void *, const void *);
  keep_malloc_hook = __malloc_hook;
  keep_realloc_hook = __realloc_hook;
  keep_memalign_hook = __memalign_hook;
  keep_free_hook = __free_hook;
  __malloc_hook = old_malloc_hook;
  __free_hook = old_free_hook;
  __realloc_hook = old_realloc_hook;
  __memalign_hook = old_memalign_hook;

  if (net_allocs != luxMap_key_count(mallocmap)) {
    printf("\nDISCREPANCY: net_allocs = %lu but mallocmap key count = %d.\n",
	   net_allocs, luxMap_key_count(mallocmap));
    bad = 1;
  }
  if (net_allocs != expected) {
    printf("\nExpected %lu but found %lu allocations.\n",
	   expected, net_allocs);
    bad = 1;
  }
#if MALLOCMAP
  if (bad) {
    printf("Allocation table (at most the last 10):\n");
    dump_allocs(10);
  }
#endif
  __malloc_hook = keep_malloc_hook;
  __free_hook = keep_free_hook;
  __realloc_hook = keep_realloc_hook;
  __memalign_hook = keep_memalign_hook;
  return bad;
}

