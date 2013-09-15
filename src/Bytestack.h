/* This is file Bytestack.h.

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
/** \file Bytestack.h Declarations for a stack of bytes whose size
 * grows as needed when more data is pushed unto it. */

#ifndef HAVE_BYTESTACK_H_
#define HAVE_BYTESTACK_H_

/* HEADERS */
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>		/* for ssize_t */
/* END HEADERS */

/** Defines a Byte stack, as a pointer to an opaque structure. */
typedef struct Bytestack_struct *Bytestack;

/** Defines an unsigned integral type for an index into a Byte
    stack. The value #BYTESTACK_INDEX_ERROR indicates an error state.
    Other values indicate locations in a Byte stack. */
typedef ssize_t Bytestack_index;

Bytestack Bytestack_create(void);
void Bytestack_delete(Bytestack bytestack);
char *Bytestack_pop(Bytestack bytestack, Bytestack_index index);
char *Bytestack_pop_all(Bytestack bytestack);
Int Bytestack_restore(Bytestack bytestack, Int size);
Bytestack_index Bytestack_push_text(Bytestack bytestack,
				    const char *text);
char *Bytestack_strcpy(Bytestack bytestack,
		       const char *text);
char *Bytestack_strcat(Bytestack bytestack,
		       const char *text);
Bytestack_index Bytestack_push_data(Bytestack bytestack,
				    const void *begin,
				    const void *end);
#define Bytestack_push_var(b, v) Bytestack_push_data((b), &(v), &(v) + 1)
char *Bytestack_temp_text(Bytestack bytestack,
			  const char *text);
void *Bytestack_temp_data(Bytestack bytestack,
			  const void *begin,
			  const void *end);
Bytestack_index Bytestack_sprintf(Bytestack bytestack,
				  const char *format, ...);
Bytestack_index Bytestack_vsprintf(Bytestack bytestack,
				   const char *format, va_list ap);
char *Bytestack_temp_sprintf(Bytestack bytestack,
			     const char *format, ...);
char *temp_sprintf(const char *format, ...);
char *Bytestack_temp_vsprintf(Bytestack bytestack,
			      const char *format, va_list ap);
char *Bytestack_peek(Bytestack bytestack,
			Bytestack_index index);
char *Bytestack_peek_all(Bytestack bytestack);
Bytestack_index Bytestack_top(Bytestack bytestack);
size_t Bytestack_bytes(Bytestack bytestack, Bytestack_index index);

char *Bytestack_quotify(Bytestack bytestack,
			Bytestack_index bi1, Bytestack_index bi2);

#if HAVE_COOKIE_IO_FUNCTIONS_T
FILE *Bytestack_open(Bytestack stack, const char *opentype);
#endif

#endif
