/* This is file debugmalloc.cc.

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
#include <map>
/* END HEADERS */

static void *my_malloc_hook(size_t, const void *);
static void my_free_hook(void *, const void *);
static void *my_realloc_hook(void *, size_t, const void *);
static void *my_memalign_hook(size_t, size_t, const void *);

unsigned long num_allocs = 1;
unsigned long net_allocs = 0;

static std::map<void*,int> map_pi; // pointer to allocation counter
static std::map<int,void*> map_ip; // allocation counter to pointer
static std::map<void*,size_t> map_size; // pointer to allocated size

struct hook_state {
  void *(*malloc_hook)(size_t, const void *);
  void *(*realloc_hook)(void *, size_t, const void *);
  void *(*memalign_hook)(size_t, size_t, const void *);
  void (*free_hook)(void *, const void *);
};

static struct hook_state originals;

struct hook_state save_hooks(void)
{
  struct hook_state state;
  state.malloc_hook = __malloc_hook;
  state.free_hook = __free_hook;
  state.realloc_hook = __realloc_hook;
  state.memalign_hook = __memalign_hook;
  return state;
}

void install_mine(void)
{
  __malloc_hook = my_malloc_hook;
  __free_hook = my_free_hook;
  __realloc_hook = my_realloc_hook;
  __memalign_hook = my_memalign_hook;
}

void install_hooks(struct hook_state state)
{
  __malloc_hook = state.malloc_hook;
  __free_hook = state.free_hook;
  __realloc_hook = state.realloc_hook;
  __memalign_hook = state.memalign_hook;
}

void my_init_hook(void) {
  originals = save_hooks();
  install_mine();
}

void install_malloc_hooks(void) __attribute__((constructor));
void install_malloc_hooks(void)
{
  __malloc_initialize_hook = my_init_hook;
}

static void *my_malloc_hook(size_t size, const void *caller) {
  struct hook_state state = save_hooks();
  install_hooks(originals);
  void *result = malloc(size);
  if (result) {
    if (map_pi.find(result) != map_pi.end())
      printf("Memory address %p already registered for allocation %d when attempting to register it for allocation %d.  Double allocation or problem in debugmalloc.cc\n", result, map_pi[result], num_allocs);
    map_pi[result] = num_allocs;
    map_ip[num_allocs] = result;
    map_size[result] = size;
    num_allocs++;
    net_allocs++;
  }
  install_hooks(state);
  return result;
}

static void *my_memalign_hook(size_t align, size_t size, const void *caller) {
  void *result;

  struct hook_state state = save_hooks();
  install_hooks(originals);
  result = memalign(align, size);
  if (result) {
    if (map_pi.find(result) != map_pi.end())
      printf("Memory address %p already registered for allocation %d when attempting to register it for allocation %d.  Double allocation or problem in debugmalloc.cc\n", result, map_pi[result], num_allocs);
    map_pi[result] = num_allocs;
    map_ip[num_allocs] = result;
    map_size[result] = size;
    num_allocs++;
    net_allocs++;
  }
  install_hooks(state);
  return result;
}

static void my_free_hook(void *ptr, const void *caller) {
  if (ptr) {    
    struct hook_state state = save_hooks();
    install_hooks(originals);
    std::map<void*,int>::iterator it = map_pi.find(ptr);
    if (it != map_pi.end()) {
      int na = map_pi[ptr];
      map_pi.erase(it);
      map_ip.erase(na);
      map_size.erase(it->first);
      if (!net_allocs)
        puts("debugmalloc.cc(my_free_hook): illegal decrement of net_allocs from 0.");
      else
        --net_allocs;
    }
    // many mallocs predate the installation of these hooks (for
    // setup), so there are many addresses of allocated memory that
    // are not included in map_pi and map_ip.  We don't complain if
    // some address isn't known to us.
    free(ptr);
    install_hooks(state);
  }
}

static void *my_realloc_hook(void *ptr, size_t size, const void *caller) {
  void *result;

  struct hook_state state = save_hooks();
  install_hooks(originals);
  result = realloc(ptr, size);
  if (ptr || size) {
    if (ptr) {
      std::map<void*,int>::iterator it = map_pi.find(ptr);
      if (it != map_pi.end()) {
        int na = map_pi[ptr];
        map_pi.erase(it);
        map_ip.erase(na);
        map_size.erase(it->first);
        if (!net_allocs)
          puts("debugmalloc.cc(my_realloc_hook): Illegal decrement of net_allocs from 0.");
        else
          --net_allocs;
      }
    }
    if (size) {
      map_pi[result] = num_allocs;
      map_ip[num_allocs] = result;
      map_size[result] = size;
      ++net_allocs;
      ++num_allocs;
    }
  }
  install_hooks(state);
  return result;
}

extern "C" {
  void dump_allocs(int);
};

void dump_allocs(int max_count)
{
  struct hook_state state = save_hooks();
  install_hooks(originals);

  std::map<int,void*>::iterator iter = map_ip.begin();
  int i = 0;
  int n = map_ip.size();
  if (max_count > 0)
    while (n > max_count) {
      ++iter;
      --n;
    }
  printf("Current allocation count = %d\n", num_allocs);
  printf(" #: num_alloc size pointer\n");
  while (iter != map_ip.end()) {
    ++i;
    printf("%2d: %9d %5d %p ", i, iter->first, map_size[iter->second], 
           iter->second);
    unsigned char *p = (unsigned char*) iter->second;
    int j;
    for (j = 0; j < 12; j++)
      putchar(isprint(p[j])? p[j]: '.');
    putchar(' ');
    for (j = 0; j < 12; j++)
      printf("%02x ", p[j]);
    putchar('\n');
    
    ++iter;
  }

  install_hooks(state);
}

int check_net_allocs(unsigned long expected)
{
  int bad = 0;

  struct hook_state state = save_hooks();
  install_hooks(originals);
  if (net_allocs != map_pi.size()) {
    printf("\nDISCREPANCY: net_allocs = %lu but map_pi key count = %d.\n",
	   net_allocs, map_pi.size());
    bad = 1;
  }
  if (net_allocs != expected) {
    printf("\nExpected %lu but found %lu allocations.\n",
	   expected, net_allocs);
    bad = 1;
  }
  if (bad) {
    printf("Allocation table (at most the last 10):\n");
    dump_allocs(10);
  }
  install_hooks(state);
  return bad;
}

