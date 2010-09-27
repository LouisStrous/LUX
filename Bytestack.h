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

/** Defines a byte stack, as a pointer to an opaque structure. */
typedef struct Bytestack_struct *Bytestack;

/** Defines an unsigned integral type for an index into a byte
    stack. The value #BYTESTACK_INDEX_ERROR indicates an error state.
    Other values indicate locations in a byte stack. */
typedef ssize_t Bytestack_index;

Bytestack Bytestack_create(void);
void Bytestack_delete(Bytestack bytestack);
char *Bytestack_pop(Bytestack bytestack, Bytestack_index index);
char *Bytestack_pop_all(Bytestack bytestack);
int Bytestack_restore(Bytestack bytestack, int size);
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
