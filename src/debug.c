/* This is file debug.c.

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
/* File debug.c */
/* LUX routines for keeping a tab on memory allocation. */
/* The routines Free(), Malloc(), Calloc(), and Realloc() can be
 put in place of the corresponding no-capital-letter forms to keep
 tabs on memory allocations and to check for consistency. */
/* file containing routines helpful in debugging */
#define COMPILING_DEBUG_C
#if HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include "action.h"
#include "install.h"
#include <signal.h>
#include <string.h>
#include <unistd.h>

#define MAXALLOC	20000
#define FOUND		1
#define BEFORE		2
#define BETWEEN		3
#define BEYOND		4
#define FIRST		5

#define MY_DEBUG	1

static struct allocItem
{ void			*ptr;	/* pointer to allocated memory */
  size_t		size;	/* size of allocated memory */
  struct allocItem	*prev;	/* link to previous allocation element */
  struct allocItem	*next;	/* link to next allocation element */
  Int			context; /* context of symbol for which memory */
				 /* is allocated*/
  Int			line;	/* line number of symbol for which */
				/* memory is allocated*/
  Int			count;	/* current execution count */
  char			flag;
} allocList[MAXALLOC];

size_t	tSize;
Int	nAlloc = 0;

extern Int	setup, curLineNumber, nExecuted;
Int	checkChain(void), findAddress(void *, struct allocItem **),
  addressToSymbol(void *), lux_dump(Int, Int []);

char	*evbName(Int);

static FILE	*Fopen_ptr[FOPEN_MAX];
static char	*Fopen_name[FOPEN_MAX], *Fopen_type[FOPEN_MAX];
static Int	Fopen_index = 0;

Int checkListMessage(void *ptr, Int size, Int limit, Int symbol, char *msg)
{
  struct allocItem	*ap;
  char	*p;

  if (findAddress(ptr, &ap) == FOUND)
    ap->flag = 1;
  else if (size >= limit) {
    p = symbolProperName(symbol);
    printwf("%d bytes at address %p in <%d> (%s%c%s) not in alloc list\n",
	    size, ptr, symbol, p? p: "unnamed", msg? ' ': '\0', msg);
    return 0;
  }
  return 1;
}

Int checkOneSymbol(Int symbol, Int limit)
{
  char	regular, *p, ok;
  struct allocItem	*ap;
  extractSec	*eptr;
  Int	n, i;
  pointer	pp;

  regular = 0;			/* default: not a regular symbol */
  ok = 1;
  p = symbolProperName(symbol);
  if (p) {
    if (findAddress(p, &ap) == FOUND) /* we found the name */
      ap->flag = 1;
    else {
      if (strlen(p) >= limit)
	printwf("Name \"%p\" of symbol %d not in alloc list\n",
		p, symbol);
      ok = 0;			/* not everything is OK anymore */
     }
   }

  switch (symbol_class(symbol)) {
    case LUX_STRING: case LUX_ARRAY: case LUX_CARRAY: case LUX_CSCALAR:
    case LUX_CPLIST: case LUX_LIST: case LUX_CLIST: case LUX_PRE_CLIST:
    case LUX_INT_FUNC:
      regular = 1;
      break;
    case LUX_SCALAR: case LUX_UNUSED: case LUX_UNDEFINED: case LUX_FUNC_PTR:
    case LUX_ENUM: case LUX_TRANSFER: case LUX_POINTER: case LUX_KEYWORD:
    case LUX_PRE_RANGE: case LUX_BIN_OP: case LUX_IF_OP: case LUX_RANGE:
      break;
    case LUX_SCAL_PTR:
      if (scal_ptr_type(symbol) == LUX_TEMP_STRING)
	 regular = 1;
      break;
    case LUX_SUBROUTINE: case LUX_FUNCTION: case LUX_BLOCKROUTINE:
      if (routine_num_parameters(symbol) != 0
	  && !checkListMessage(routine_parameter_names(symbol),
			       routine_num_parameters(symbol)*sizeof(char *),
			       limit, symbol, "routine_parameter_names"))
	ok = 0;
      if (!checkListMessage(routine_parameters(symbol), (routine_num_parameters(symbol) + routine_num_statements(symbol))*sizeof(Word), limit, symbol, "routine_parameters"))
	ok = 0;
      break;
    case LUX_EXTRACT: case LUX_PRE_EXTRACT:
      if (symbol_class(symbol) == LUX_EXTRACT) {
	if (!checkListMessage(extract_ptr(symbol), symbol_memory(symbol),
			      limit, symbol, "extract_ptr"))
	  ok = 0;
	n = extract_num_sec(symbol);
	eptr = extract_ptr(symbol);
      } else {
	if (!checkListMessage(pre_extract_name(symbol),
			      strlen(pre_extract_name(symbol)),
			      limit, symbol, "pre_extract_name"))
	  ok = 0;
	if (!checkListMessage(pre_extract_data(symbol), sizeof(preExtract),
			      limit, symbol, "pre_extract_data"))
	  ok = 0;
	if (pre_extract_num_sec(symbol)
	    && !checkListMessage(pre_extract_ptr(symbol), pre_extract_num_sec(symbol)*sizeof(extractSec), limit, symbol, "pre_extract_ptr"))
	  ok = 0;
	n = pre_extract_num_sec(symbol);
	eptr = pre_extract_ptr(symbol);
      }
      while (n--) {
	switch (eptr->type) {
	  case LUX_RANGE:
	    if (!checkListMessage(eptr->ptr.w, eptr->number*sizeof(Word),
				  limit, symbol, "eptr->ptr.w"))
	      ok = 0;
	    break;
	  case LUX_LIST:
	    if (!checkListMessage(eptr->ptr.sp, eptr->number*sizeof(char *),
				  limit, symbol, "eptr->ptr.sp"))
	      ok = 0;
	    i = eptr->number;
	    pp.sp = eptr->ptr.sp;
	    while (i--) {
	      if (*pp.sp
		  && !checkListMessage(*pp.sp, sizeof(*pp.sp), limit,
				       symbol, "eptr->ptr.sp->"))
		ok = 0;
	      pp.sp++;
	    }
	    break;
	}
	eptr++;
      }
      break;
    case LUX_EVB:
      switch (evb_type(symbol)) {
	case EVB_INT_SUB: case EVB_USR_SUB: case EVB_CASE: case EVB_NCASE:
	case EVB_BLOCK: case EVB_INSERT: case EVB_FILE:
	  regular = 1;
	  break;
	case EVB_REPLACE: case EVB_IF: case EVB_RETURN: case EVB_FOR:
	case EVB_WHILE_DO: case EVB_DO_WHILE: case EVB_REPEAT:
	case EVB_USR_CODE:
	  break;
	default:
	  p = symbolProperName(symbol);
	  printwf("%s <%d>: EVB type %d (%s) not yet supported\n",
		  p? p: "(unnamed)", symbol, evb_type(symbol),
		  evbName(evb_type(symbol)));
	  
      }
      break;
    default:
      p = symbolProperName(symbol);
      printwf("%s <%d>: class %d (%s) not yet supported\n",
	      p? p: "(unnamed)", symbol, symbol_class(symbol),
	      className(symbol_class(symbol)));
      break;
  }
  if (regular && symbol_memory(symbol)) {
    if (!checkListMessage(symbol_data(symbol), symbol_memory(symbol),
			  limit, symbol, ""))
      ok = 0;
  }
  return ok;
}

char	checkListOk = 0;
Int checkList(Int narg, Int ps[])
/* checks all allocation entries against the LUX symbol tables and
   reports discrepancies.  NOTE: internal function and routine key lists
   are not regarded, so they result in spurious error messages.  These lists
   at present do not exceed 32 bytes in size, so a limit of 33 ought to take
   care of them. */
{
 Int	limit, i, nbad = 0, symbol;
 struct allocItem	*ap;
 char	*symbolProperName(Int);
 extern Int nSymbolStack;
 void	setPager(Int), resetPager(void);
 extern char	*theFormat, *ulib_path;

 if (narg)
   limit = int_arg(*ps);	/* lower limit for reporting */
 else
   limit = 0;
 ap = allocList;
 /* we set all "flag" entries to zero, indicating that the corresponding
    memory has not yet been located in any symbol. */
 while (ap->next) {
   ap->flag = 0;		/* default: not found */
   ap = ap->next;
 }
 
 setPager(0);			/* print by pages */

 checkListOk = 1;		/* so far everything still checks out */

 /* check all symbols */
 for (symbol = 0; symbol < NSYM; symbol++)
   if (checkOneSymbol(symbol, limit) == 0)
     checkListOk = 0;

 /* some special cases */
 if ((theFormat && 
      !checkListMessage(theFormat, strlen(theFormat) + 1, limit, 0,
			"theFormat"))
     || (ulib_path &&
	 !checkListMessage(ulib_path, strlen(ulib_path) + 1, limit, 0,
			   "ulib_name")))
   checkListOk = 0;

 /* check the symbol stack */
 for (i = 0; i < nSymbolStack; i++) {
   if (symbolStack[i]) {
     if (findAddress(symbolStack[i], &ap) == FOUND)
       ap->flag = 1;
     else {
       if (strlen(symbolStack[i]) >= limit)
	 printwf("Address %p at index %d (size %d) in symbolStack not in alloc list\n",
		symbolStack[i], i, strlen(symbolStack[i]));
       checkListOk = 0;
     }
   }
 } 

 /* now check the whole alloc list */
 ap = allocList;
 while (ap->next) {
   if (!ap->flag && ap->ptr && ap->size >= limit) {
     printwf("Alloc block of size %d at address %p not in any symbol\n",
	     ap->size, ap->ptr);
     printwf("[number %d; context %d; line %d; count %d]\n", ap - allocList,
	     ap->context, ap->line, ap->count);
     nbad++;
     checkListOk = 0;
   }
   ap = ap->next;
 }
 if (!checkListOk)
   printwf("%d alloc blocks unaccounted for.\n", nbad);
 resetPager();
 return 1;
}
/*------------------------------------------------------------------*/
Int findAddress(void *p, struct allocItem **ap)
/* finds address p in list.  Returns FOUND if found, BEYOND if beyond last
   item, FIRST if first item, BEFORE if before first item,
   and BETWEEN otherwise.
   returns pointer to item with found address or to the first
   item pointing beyond p, or to the last item, in *ap */
{
 struct allocItem	*ai;
 
 ai = allocList;
	/* find address p or pointer to first item beyond p */
 if (!allocList->next || allocList->next->ptr > p) {
   *ap = allocList;
   return FIRST;
 }
 while (ai->next && ai->ptr < p)
   ai = ai->next; 
 if (ap)
   *ap = ai;
 if (ai->ptr == p)
   return FOUND;
 if (!ai->next && p > ai->ptr)
   return BEYOND;
 return (ai->prev && p == ai->prev->ptr)? FOUND: BETWEEN;
}
/*-----------------------------------------------------------------------*/
Int freeAllocItemIndex = 1;
Int findFreeItem(void)
/* returns index to next free item in allocList, or -1 if none available */
{
 Int		oldindx;

 oldindx = freeAllocItemIndex;
 while (allocList[freeAllocItemIndex].size) {
   if (++freeAllocItemIndex == MAXALLOC)
     freeAllocItemIndex = 1;
   if (freeAllocItemIndex == oldindx)
     return -1;
 }
 return freeAllocItemIndex;
}
/*-----------------------------------------------------------------------*/
Int listAddress(void *address)
{
  struct allocItem	*ap;
  Int	result;
  
  result = findAddress(address, &ap);
  switch (result) {
    case FOUND: case FIRST:
      printf("found address %p at start of alloc item %d;\nsize %d; context %d; line %d; count %d\n", address, ap - allocList, ap->size, ap->context, ap->line, ap->count);
      return 0;
    case BETWEEN: case BEYOND: case BEFORE:
      if (result == BETWEEN)
	ap = ap->prev;
      if ((char *) ap->ptr + ap->size > (char *) address) {
	printf("found address %p at %d bytes beyond start of alloc item %d at %p;\ncontext %d; line %d; count %d\n", address, (char *) address - (char *) ap->ptr, ap - allocList, ap->ptr, ap->context, ap->line, ap->count);
	return 1;
      }
      printf("did not find address %p in alloc list\n", address);
      return 2;
  }
  return 3;
}
/*-----------------------------------------------------------------------*/
void Free(void *ptr)
/* Like standard free(), but keeping records for debugging */
{
  Int			found;
  struct allocItem	*ap;

  if (!ptr) return;
  found = findAddress(ptr, &ap);
  if (found != FOUND) {
    fprintf(stderr, "Free   - address %p not in list\n", ptr);
  } else {
    if (setup & MY_DEBUG)
      fprintf(stderr,
      "Free   - %7d at %p, (#%4d), total: %7d in %4d items\n",
       ap->size, ptr, ap - allocList, tSize - ap->size, nAlloc - 1); 
    nAlloc--;
    tSize -= ap->size;
    if (ap->prev)
      ap->prev->next = ap->next;
    else
      allocList[0].next = ap->next;
    if (ap->next)
      ap->next->prev = ap->prev;
    else
      if (ap->prev) ap->prev->next = NULL;
    ap->ptr = 0;
    ap->next = 0;
    ap->prev = 0;
    ap->size = 0;
    if (!nAlloc)
      allocList[0].next = NULL;
  }
  free(ptr);
  checkChain();
}
/*-----------------------------------------------------------------------*/
void *Malloc(size_t size)
/* malloc with stored information for consistency checks */
{
  void	*p;
  struct allocItem	*ap;
  Int	found, i;
  
  if (!size)
    return NULL;
  p = malloc(size);
  found = findAddress(p, &ap);
  if (found == FOUND) {
    fprintf(stderr, "Malloc - address %p already allocated\n",  p);
    return p;
  } else if (found == BETWEEN &&
	     (char *) ap->prev->ptr + ap->prev->size > (char *) p) {
    fprintf(stderr, 
	    "Malloc - address %p inside allocated block of size %d at %p\n",
	    p, ap->prev->size, ap->prev->ptr);
    return p;
  }
  if (found == BEYOND && (char *) ap->ptr + ap->size > (char *) p) {
    fprintf(stderr, 
	    "Malloc - address %p inside allocated block of size %d at %p\n",
	    p, ap->size, ap->ptr);
    return p;
  }
  i = findFreeItem();
  if (i == -1) {
    fprintf(stderr, "Malloc - no more room in alloc list\n");
    return p;
  }
  if (allocList[i].flag) 
    printf("allocation flag already set in element %d\n", i);
  nAlloc++;
  tSize += size;
  allocList[i].ptr = p;
  allocList[i].size = size;
  allocList[i].context = curContext;
  allocList[i].line = curLineNumber;
  allocList[i].count = nExecuted;
  switch (found) {
    case FIRST:
      allocList[i].next = allocList[0].next;
      if (allocList[0].next)
	allocList[0].next->prev = &allocList[i];
      allocList[0].next = &allocList[i];
      break;
    case BETWEEN:
      allocList[i].next = ap;
      allocList[i].prev = ap->prev;
      ap->prev->next = &allocList[i];
      ap->prev = &allocList[i];
      break;
    case BEYOND:
      allocList[i].prev = ap;
      ap->next = &allocList[i];
      break;
  }
  if (setup & MY_DEBUG)
    fprintf(stderr, "Malloc - %7d at %p, (#%4d), total: %7u in %4d items\n",
	    size, p, i, tSize, nAlloc);
  checkChain();
  return p;
}
/*-----------------------------------------------------------------------*/
void *Calloc(size_t nobj, size_t size)
/* calloc with stored information for consistency checks */
{
  void	*p;
  char	*c;
  Int	n;

  p = Malloc(n = nobj*size);
  c = (char *) p;
  while (n--)
    *c++ = 0;
  return p;
}
/*-----------------------------------------------------------------------*/
void *Realloc(void *p, size_t size)
/* realloc with stored information for consistency checks */
{
  Int	i, found;
  struct allocItem	*ai;
  void	*p2;

  /* realloc(NULL, size) is equivalent to malloc(size)
     realloc(p, 0) is equivalent to Free(p) */
  if (!p)
    return Malloc(size);
  if (!size) {
    Free(p);
    return NULL;
  }
  found = findAddress(p, &ai);
  switch (found) {
    case BEYOND:  case BEFORE:  case BETWEEN:  case FIRST:
      fprintf(stderr, "Realloc - address %p is not in the list\n",
	      p);
      return NULL;
    case FOUND:
      p2 = realloc(p, size);
      if (p == p2) {		/* sufficient space at original position */
	tSize += size;
        if (ai) {
	  tSize -= ai->size;
	  ai->size = size;
	}
	if (setup & MY_DEBUG)
	  fprintf(stderr, "Realloc - %7d at %p, (%4d), total: %7u in %4d items\n",
		  size, p2, ai - allocList, tSize, nAlloc);
      } else {
	if (ai) {
	  tSize -= ai->size;
	  if (ai->prev)
	    ai->prev->next = ai->next;
	  if (ai->next)
	    ai->next->prev = ai->prev;
	  ai->ptr = 0;
	  ai->next = 0;
	  ai->size = 0;
	}
	found = findAddress(p2, &ai);
	if (found == FOUND) {
	  fprintf(stderr, "Realloc - adress %p already allocated\n", p2);
	  return p2;
	} else if (found == BETWEEN &&
		   (char *) ai->prev->ptr + ai->prev->size > (char *) p2) {
	  fprintf(stderr, "Realloc - address %p inside allocated block of size %d at %p\n",
		  p2, ai->prev->size, ai->prev->ptr);
	  return p2;
	}
	if (found == BEYOND && (char *) ai->ptr + ai->size > (char *) p2) {
	  fprintf(stderr, "Realloc - address %p inside allocated block of size %d at %p\n",
		  p2, ai->size, ai->ptr);
	  return p2;
	}
	i = findFreeItem();
	if (i == -1) {
	  fprintf(stderr, "Realloc - no more room in alloc list\n");
	  return p2;
	}
	tSize += size;
	allocList[i].ptr = p2;
	allocList[i].size = size;
	allocList[i].context = curContext;
	allocList[i].line = curLineNumber;
	allocList[i].count = nExecuted;
	switch (found) {
	  case FIRST:
	    allocList[i].next = allocList[0].next;
	    if (allocList[0].next)
	      allocList[0].next->prev = &allocList[i];
	    allocList[0].next = &allocList[i];
	    break;
	  case BETWEEN:
	    allocList[i].next = ai;
	    allocList[i].prev = ai->prev;
	    ai->prev->next = &allocList[i];
	    ai->prev = &allocList[i];
	    break;
	  case BEYOND:
	    allocList[i].prev = ai;
	    ai->next = &allocList[i];
	    break;
	  }
	if (setup & MY_DEBUG)
	  fprintf(stderr, "Realloc - %7d at %p, (%4d), total: %7u in %4d items\n",
		  size, p2, ai - allocList, tSize, nAlloc);
      }
  }
  checkChain();
  return p2;
}
/*-----------------------------------------------------------------------*/
Int checkChain(void)
     /* checks the allocation list for consistency */
{
  struct allocItem	*ai;
  Int	count = 0;

  ai = allocList;
  while (ai->next) {
    count++;
    if (ai == ai->next) {
      luxerror("Unending loop in forward allocList at number %1d (%1d)",
	    0, count, (ai - allocList)/sizeof(struct allocItem));
      return -1;
    }
    ai = ai->next;
  }
  if (count != nAlloc) {
    luxerror("Number of forward allocation units %1d is unequal to target %1d",
	  count, nAlloc);
    return -1;
  }
  while (ai->prev) {
    count--;
    if (ai == ai->prev) {
      luxerror("Unending loop in backward allocList at number %1d (%1d)",
	    0, count, (ai - allocList)/sizeof(struct allocItem));
      return -1;
    }
    ai = ai->prev;
  }
  if (count > 1) {
    luxerror("Number of backward allocation units %1d is unequal to target %1d",
	  nAlloc - count + 1, nAlloc);
    return -1;
  }
  return 1;
}
/*-----------------------------------------------------------------------*/
Int findWorm(void)
/* checks the allocation list for internal consistency */
{
  struct allocItem	*ai, *ai2;
  char	suspect = 0;

  ai = allocList;
  while (ai->next) {
    if ((Int) ai->next < 0x10000000 || ai->flag ||
	(ai != allocList && (Int) ai->ptr < 0x10000000)) {
      printf("*** Suspect allocation element at %p\n", ai);
      if (ai->flag)
	suspect = 1;
      else
	return 1;
    }
    ai2 = ai;
    ai = ai->next;
    if (ai2 != ai->prev && ai2 != allocList) {
      printf("*** Broken link in allocation chain at %p\n", ai2);
      return 1;
    }
  }
  return suspect;
}
/*-----------------------------------------------------------------------*/
Int lux_whereisAddress(Int narg, Int ps[])
/* returns information on memory at a given address */
{
  Int	address, where, cut;
  struct allocItem	*ai;

  if (internalMode & 1) {
    cut = int_arg(*ps);		/* minimum size */
    ai = allocList;
    while (ai->next) {
      ai = ai->next;
      if (ai->size >= cut) {	/* got one */
	printf("Block of size %d at address %p\n", ai->size, ai->ptr);
	addressToSymbol(ai->ptr);
      }
    }
    return 1;
  }
  address = int_arg(*ps);
  where = findAddress((void *) address, &ai);
  switch (where) {
    case BEFORE: case FIRST:
      where = 0;
      break;
    case BETWEEN: case BEYOND:
      where = (address < (Int) ai->ptr ||
	       address >= (Int) ai->ptr + ai->size)? 0: 1;
      break;
    case FOUND:
      where = 1;
      break;
  }
  if (where) {
    printf("Address %x in block of size %d starting at %p.\n",
	   address, ai->size, ai->ptr);
    addressToSymbol(ai->ptr);
  } else
    printf("Address %x not in any allocated memory block.\n", address);
  return 1;
}
/*-----------------------------------------------------------------------*/
Int addressToSymbol(void *address)
{
  Int	iq, symbol = -1, i, n, j;
  array	*h;
  pointer	ptr;

  for (iq = NAMED_START; iq < NAMED_END; iq++) {
    switch (sym[iq].class) {
      case LUX_SCALAR: case LUX_SCAL_PTR:
	break;
      case LUX_STRING:
	if (address == sym[iq].spec.array.ptr)
	  symbol = iq;
	break;
      case LUX_ARRAY:
	h = HEAD(iq);
	if (sym[iq].type == LUX_TEMP_STRING)	/* a string array */
	{ ptr.sp = (char **) string_value(iq);
	  GET_SIZE(n, h->dims, h->ndim);
	  for (j = 0; j < n; j++)
	    if ((void *) *ptr.sp++ == address)
	    { printf("Block allocated for string #%1d (%s) in string array <%1d>:\n",
		     j, *--ptr.sp, iq);
	      symbol = iq;
	      break; }
	  if (symbol >= 0) break; }
	else			/* numerical array */
	  if (address == sym[iq].spec.array.ptr) symbol = iq;
	break;
      }
  }
  if (symbol < 0) for (iq = TEMPS_START; iq < TEMPS_END; iq++)
  { switch (sym[iq].class)
    { case LUX_SCALAR: case LUX_SCAL_PTR:
	break;
      case LUX_STRING:
	if (address == sym[iq].spec.array.ptr) symbol = iq;
	break;
      case LUX_ARRAY:
	h = HEAD(iq);
	if (sym[iq].type == LUX_TEMP_STRING)	/* a string array */
	{ ptr.sp = (char **) string_value(iq);
	  GET_SIZE(n, h->dims, h->ndim);
	  for (j = 0; j < n; j++)
	    if ((void *) *ptr.sp++ == address)
	    { printf("Block allocated for string #%1d (%s) in string array <%1d>:\n",
		     j, *--ptr.sp, iq);
	      symbol = iq;
	      break; }
	  if (symbol >= 0) break; }
	else			/* numerical array */
	  if (address == sym[iq].spec.array.ptr) symbol = iq;
	break;
      }
  }
  if (symbol < 0)
    printf("Address %p not found in checked symbols.\n", address);
  else lux_dump(1, &symbol);
  return symbol;
}
/*-----------------------------------------------------------------------*/
Int lux_newallocs(Int narg, Int ps[])
     /* NEWALLOCS reports on new allocations since the last call to */
     /* NEWALLOCS.  LS 11dec97 */
{
  struct allocItem	*ai;
  static Int	baseCount = 0, *baseExecs = NULL;
  Int	count, i;

  if (narg) {
    if (symbol_class(ps[0]) != LUX_SCALAR)
      return cerror(NEED_SCAL, ps[0]);
    count = int_arg(ps[0]);
    if (count <= 0)
      return luxerror("Need nonnegative scalar", ps[0]);
  } else count = 0;

  if (!baseCount && (internalMode & 1) == 0) {
    puts("No NEWALLOCS baseline set yet -- set now");
    if (count == 0) {
      puts("Assuming count of 1");
      count = 1;
    }
    internalMode = 1;
  }
  if (internalMode & 1) {	/* /RESET */
    if (baseExecs) {
      Free(baseExecs);
      baseExecs = NULL;
    }
    if (!count)
      count = 1;
    baseCount = count;
    baseExecs = (Int *) Malloc(baseCount*sizeof(Int));
    for (i = 0; i < baseCount; i++)
      baseExecs[i] = nExecuted;
  } else {			/* report */
    if (count >= baseCount) {
      printf("Reducing NEWALLOCS count to %1d\n", baseCount - 1);
      count = baseCount - 1;
    }
    ai = allocList;
    if (ai->next)
      ai = ai->next;
    while (ai) {
      if (ai->count > baseExecs[0]
	  && (!count || ai->count <= baseExecs[baseCount - 1 - count])) {
	printf("NEWALLOCS: %p; size %d; count %d; context %1d (%s); line %1d\n",
	       ai->ptr, ai->size, ai->count, ai->context,
	       symbolProperName(ai->context), ai->line);
	addressToSymbol(ai->ptr);
      }
      ai = ai->next;
    }
    memcpy(baseExecs, baseExecs + sizeof(Int), (baseCount - 1)*sizeof(Int));
    baseExecs[baseCount - 1] = nExecuted;
  }
  return 1;
}
/*-----------------------------------------------------------------------*/
Int squeeze(void)
{
  Int	brk, last;
  Word	*position;
  extern char	*firstbreak;
  struct allocItem	*ai;

  brk = (char *) sbrk(0) - firstbreak;
  ai = allocList;
  while (ai->next) ai = ai->next;
  last = (char *) ai->ptr - firstbreak;
  printf("last: %d;  break: %d;  allocated %d\n", last, brk, tSize);
  position = (Word *) calloc(nAlloc, sizeof(Word));
  return 1;
}
/*-----------------------------------------------------------------------*/
Int lux_squeeze(Int narg, Int ps[])
{
  struct allocItem	*ai;
  Int	iq, dim, *p;
  array	*h;
  
  dim = nAlloc*2;
  iq = array_scratch(LUX_LONG, 1, &dim);
  h = HEAD(iq);
  h->ndim = 2;
  h->dims[0] = 2;
  h->dims[1] = nAlloc;
  p = LPTR(h);
  ai = allocList;
  while (ai->next)
  { ai = ai->next;
    *p++ = (Int) ai->ptr;
    *p++ = ai->size; }
  return iq;
}
/*-----------------------------------------------------------------------*/
char *save(char *text)
{
  char *out;

  out = Malloc(strlen(text) + 1);
  if (!out) {
    puts("MALLOC error in save");
    return NULL;
  }
  strcpy(out, text);
  return out;
}

FILE *Fopen(const char *filename, const char *type)
{
  FILE	*fp;

  fp = fopen(filename, type);
  if (fp) {
    while (Fopen_ptr[Fopen_index])
      if (++Fopen_index == FOPEN_MAX)
	Fopen_index = 0;
    Fopen_ptr[Fopen_index] = fp;
    Fopen_name[Fopen_index] = save((char *) filename);
    Fopen_type[Fopen_index] = save((char *) type);
  }
  return fp;
}
/*-----------------------------------------------------------------------*/
FILE *Tmpfile(void)
{
  FILE	*fp;

  fp = tmpfile();
  if (fp) {
    while (Fopen_ptr[Fopen_index])
      if (++Fopen_index == FOPEN_MAX)
	Fopen_index = 0;
    Fopen_ptr[Fopen_index] = fp;
    Fopen_name[Fopen_index] = save("tmpfile");
    Fopen_type[Fopen_index] = save("wb+");
  }
  return fp;    
}
/*-----------------------------------------------------------------------*/
Int Fclose(FILE *stream)
{
  Int	n, i;

  n = fclose(stream);
  if (!n) {			/* closed OK */
    for (i = 0; i < FOPEN_MAX; i++)
      if (Fopen_ptr[i] == stream)
	break;
    if (i == FOPEN_MAX) {
      puts("ERROR - no record of closed file");
      return EOF;
    }
    Fopen_ptr[i] = NULL;
    Free(Fopen_name[i]);
    Free(Fopen_type[i]);
    Fopen_name[i] = Fopen_type[i] = 0;
  }
  return n;
}
/*-----------------------------------------------------------------------*/
Int show_files(Int narg, Int ps[])
{
  Int	i;

  puts("Open files:");
  for (i = 0; i < FOPEN_MAX; i++)
    if (Fopen_ptr[i])
      printf("%2d : %3s : %s\n",
	     i, Fopen_type[i], Fopen_name[i]);
  return 1;
}
/*-----------------------------------------------------------------------*/
#undef COMPILING_DEBUG_C
