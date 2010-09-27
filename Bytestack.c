/** \file Bytestack.c A stack of bytes whose size grows as needed
    when more data is pushed unto it. */

#include "Bytestack.h"
/* HEADERS */
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
/* END HEADERS */

/** Internal representation of a stack of bytes whose size grows as
    needed when more data is pushed unto it.  This structure is not
    exposed; use <code>Bytestack</code> instead. */
struct Bytestack_struct {
  /** The beginning of the stack data. */
  char *begin;
  /** The current location within the stack, represented as the offset
      from the beginning.  Newly pushed data will begin here. */
  size_t cur;
  /** The greatest offset of any data byte stored in the stack.
      Positions beyond this one have not yet been used. */
  size_t max;
  /** The current capacity of the stack, represented as the offset
      from the beginning.  \a max cannot exceed \a size. */
  size_t size;
};

/** The default byte stack. */
static Bytestack stack = NULL;

/** Returns the default byte stack.  The default byte stack is
    (re)created if it does not currently exist. */
static Bytestack
default_Bytestack(void) 
{
  if (!stack)
    stack = Bytestack_create();
  return stack;
}

/** Creates an empty byte stack.
 * 
 * @return The byte stack, or \c NULL if a problem occurred.
 *
 * The caller is responsible for (eventually) deleting the byte stack
 * again, using Bytestack_delete().
 */
Bytestack 
Bytestack_create(void)
{
  Bytestack b;

  b = (Bytestack) malloc(sizeof(*b));
  if (b) {
    b->size = 256;
    b->cur = 0;
    b->max = 0;
    b->begin = (char *) malloc((size_t) b->size);
    if (!b->begin) {
      free(b);
      b = NULL;
    }
    *b->begin = '\0';
  }
  return b;
}

/**
 * Deletes a byte stack.
 *
 * @param b the byte stack to delete.  May be \c NULL, which refers to
 * the default byte stack.
 *
 * @see Bytestack_create()
 */
void 
Bytestack_delete(Bytestack b) 
{
  if (!b)
    b = default_Bytestack();
  if (b) {
    free(b->begin);
    free(b);
  }
  if (b == stack)
    stack = NULL;		/* undefined again */
}

/** Adds a minimum number of bytes of room to the byte stack.

    @param b the byte stack to enlarge.  May be \c NULL, which refers
    to the default byte stack.

    @param n the minimum number of bytes to add.  More bytes than this
    may in fact be added.

    @return 0 upon success, non-zero upon error. */
static int 
enlarge(Bytestack b, size_t n) 
{
  char *p;

  if (!b)
    b = default_Bytestack();
  n = (((n - 1)/256) + 1)*256;
  p = realloc(b->begin, (size_t) (b->size + n));
  if (p) {
    b->size += n;
    b->begin = p;
    return 0;
  } else
    return 1;
}

/**
 * Pushes text unto a byte stack.
 * 
 * @param b the byte stack to push unto.  May be \c NULL, which
 * indicates the default byte stack.
 * 
 * @param text the text to push.
 *
 * Pushes bytes from \p text until but excluding the next following
 * null byte unto byte stack \p b.
 *
 * @return The byte index that indicates the beginning of the pushed
 * text on the byte stack, or #BYTESTACK_INDEX_ERROR if an error
 * occurred.
 */
Bytestack_index 
Bytestack_push_text(Bytestack b,
		       const char *text) 
{
  return Bytestack_push_data(b, text, NULL);
}

char *
Bytestack_strcpy(Bytestack b,
		 const char *text)
{
  Bytestack_pop_all(b);
  return Bytestack_strcat(b, text);
}

char *
Bytestack_strcat(Bytestack b,
		 const char *text)
{
  return Bytestack_peek(b, Bytestack_push_text(b, text));
}

/**
 * Pushes data unto a byte stack.
 * 
 * @param b the byte stack to push unto.  May be \c NULL, which
 * indicates the default byte stack.
 * 
 * @param begin the first byte to push.
 * 
 * @param end one beyond the last byte to push.  Must be greater than
 * \p begin.
 *
 * Pushes bytes from \p begin until just before \p end unto byte stack
 * \p b.
 *
 * @return The byte index that indicates the beginning of the pushed
 * text on the byte stack, or #BYTESTACK_INDEX_ERROR if an error
 * occurred.
 */
Bytestack_index 
Bytestack_push_data(Bytestack b,
		       const void *begin,
		       const void *end) 
{
  size_t n;
  ssize_t p;

  if (!begin)
    return -1;
  if (!b)
    b = default_Bytestack();
  if (!end)
    end = strchr(begin, '\0');
  if (end < begin)
    return -1;
  if (end == begin) {
    b->begin[b->cur] = '\0';
    return b->cur;
  }
  n = (size_t) (end - begin);
  p = -1;
  if ((b->cur + n < b->size	/* enough room */
       || enlarge(b, n + 1) == 0) /* or enlarged sufficiently */
      && end >= begin) {
    memcpy(b->begin + b->cur, begin, n);
    b->begin[b->cur + n] = '\0'; /* always end with a '\0' */
    p = b->cur;
    b->cur += n;
    if (b->cur > b->max)
      b->max = b->cur;
  }
  return p;
}

/**
 * Pops data from a byte stack.
 * 
 * @param b the byte stack to pop from.  May be \c NULL, which
 * indicates the default byte stack.
 * 
 * @param index the byte stack index that indicates the beginning of
 * the data to pop.
 *
 * Pops data from a byte stack.  All data from location \p index
 * through the end of stack \p b (i.e., the most recently pushed data)
 * is popped.  It remains on the byte stack but is flagged as
 * available for overwriting.  It will be overwritten by the next call
 * to any function that pushes new data unto the byte stack.  This
 * means that popped data is only guaranteed to be available at the
 * returned location until the next data is pushed unto the stack.
 * Also, the memory holding the data remains owned by the byte stack
 * and must not be modified or freed by the caller.
 *
 * @return a pointer to the beginning of the popped data.
 */
char *
Bytestack_pop(Bytestack b, Bytestack_index index) 
{
  if (!b)
    b = default_Bytestack();
  if (index < 0 || index > b->max)
    return NULL;
  b->begin[b->cur] = '\0';
  b->cur = index;
  return b->begin + b->cur;
}

char *Bytestack_pop_all(Bytestack b)
{
  return Bytestack_pop(b, 0);
}

/**
 * Restores data to a byte stack.
 * 
 * @param b the byte stack.  May be \c NULL, which indicates the
 * default byte stack.
 * 
 * @param size the number of bytes to restore.  Must be non-negative.
 *
 * Shifts the stack pointer over the indicated number \p size of
 * bytes, so that that many bytes just beyond the stack pointer that
 * were earlier pushed and popped are again considered to be part of
 * the byte stack \p b.
 *
 * It is not possible to 'restore' bytes that haven't actually been
 * pushed unto the stack.
 *
 * @return 0 upon success, 1 upon failure (e.g., when \p size is
 * negative or larger than the number of bytes available for
 * restoring).
 */
int 
Bytestack_restore(Bytestack b, int size) 
{
  if (!b)
    b = default_Bytestack();
  if (size < 0 || size + b->cur > b->max)
    return 1;
  b->cur += size;
  return 0;
}

/**
 * Temporarily stores a text string on a byte stack.
 * 
 * @param stack the byte stack.  May be \c NULL, which indicates the
 * default byte stack.
 * 
 * @param text the text to temporarily store on the byte stack.
 *
 * Pushes and immediately pops the \p text from byte stack \p stack
 * again.  In effect, a temporary copy of the text is created, which
 * is retained only until the next data is pushed unto the byte stack.
 *
 * The memory holding the temporary copy remains owned by the byte
 * stack and must not be changed or freed by the caller.
 *
 * @return a pointer to the temporary copy of the text.
 */
char *
Bytestack_temp_text(Bytestack stack, const char *text) 
{
  return Bytestack_pop(stack, Bytestack_push_text(stack, text));
}

/**
 * Temporarily stores data on a byte stack.
 * 
 * @param stack the byte stack.  May be \c NULL, which indicates the
 * default byte stack.
 * 
 * @param begin the location of the first byte to store.
 * @param end one beyond the location of the last byte to store.
 *
 * Pushes and immediately pops the data from the byte stack \p stack
 * again.  In effect, a temporary copy of the data is created, which
 * is retained only until the next data is pushed unto the byte stack.
 *
 * The memory holding the temporary copy remains owned by the byte
 * stack and must not be changed or freed by the caller.
 *
 * @return a pointer to the beginning of the temporary copy of the
 * data.
 */
void *
Bytestack_temp_data(Bytestack stack, const void *begin,
		       const void *end) 
{
  return Bytestack_pop(stack, Bytestack_push_data(stack, begin, end));
}

/**
 * Temporarily stores text on a byte stack.
 * 
 * @param stack the byte stack.  May be \c NULL, which indicates the
 * default byte stack.
 * 
 * @param fmt the printf-style format string to guide the printing of
 * text to the byte stack.
 * 
 * @param ap the list of arguments to print.
 *
 * Pushes (under guidance of the format \p fmt) and immediately pops
 * the text from the byte stack again.  In effect, a temporary copy of
 * the text is created, which is retained only until the next data is
 * pushed unto the byte stack.
 *
 * The memory holding the temporary copy remains owned by the byte
 * stack and must not be changed or freed by the caller.
 *
 * @return a pointer to the temporary copy of the text.
 */
char *
Bytestack_temp_vsprintf(Bytestack stack, const char *fmt, va_list ap) 
{
  return Bytestack_pop(stack, Bytestack_vsprintf(stack, fmt, ap));
}

/**
 * Temporarily stores text on a byte stack.
 * 
 * @param stack the byte stack.  May be \c NULL, which indicates the
 * default byte stack.
 * 
 * @param fmt the printf-style format string to guide the printing of
 * text to the byte stack.
 *
 * Pushes (under guidance of the format \p fmt) and immediately pops
 * the text from the byte stack again.  In effect, a temporary copy of
 * the text is created, which is retained only until the next data is
 * pushed unto the byte stack.
 *
 * The memory holding the temporary copy remains owned by the byte
 * stack and must not be changed or freed by the caller.
 *
 * @return a pointer to the temporary copy of the text.
 */
char *
Bytestack_temp_sprintf(Bytestack stack, const char *fmt, ...) 
{
  va_list ap;

  va_start(ap, fmt);
  char *result = Bytestack_temp_vsprintf(stack, fmt, ap);
  va_end(ap);
  return result;
}

char *temp_sprintf(const char *format, ...)
{
  va_list ap;

  va_start(ap, format);
  char *result = Bytestack_temp_vsprintf(NULL, format, ap);
  va_end(ap);
  return result;
}

/**
 * Returns the top of the stack.
 * 
 * @param b the byte stack.  May be \c NULL, which indicates the
 * default byte stack.
 *
 * Returns the top of the byte stack, where the next pushed data will
 * be stored.
 *
 * @return the byte stack index of the top of the byte stack.
 */
Bytestack_index 
Bytestack_top(Bytestack b) 
{
  if (!b)
    b = default_Bytestack();
  return b? b->cur: -1;
}

/**
 * Peeks at byte stack data without popping it.
 * 
 * @param b the byte stack.  May be \c NULL, which indicates the
 * default byte stack.
 * 
 * @param index the byte stack index of the data to peek at.
 *
 * Returns a pointer to some data on the stack \p b, without popping
 * the data from the stack.
 *
 * @return a pointer to the data in the stack.  This pointer is only
 * guaranteed to remain valid until the next data is pushed unto the
 * byte stack.
 */
char *
Bytestack_peek(Bytestack b, Bytestack_index index) 
{
  if (!b)
    b = default_Bytestack();
  if (index < 0 || index > b->max)
    return NULL;
  return b->begin + index;
}

char *
Bytestack_peek_all(Bytestack b)
{
  return Bytestack_peek(b, 0);
}

size_t
Bytestack_bytes(Bytestack b, Bytestack_index bi)
{
  return Bytestack_peek(b, Bytestack_top(b)) - Bytestack_peek(b, bi);
}

/**
 * Prints data unto a byte stack.
 * 
 * @param b the byte stack.  May be \c NULL, which indicates the
 * default byte stack.
 * 
 * @param fmt the printf-style format string to guide the printing.
 *
 * Prints text to the byte stack \p b under guidance of the format
 * string \p fmt.
 *
 * @return the byte stack index that points at the beginning of the
 * newly printed data.
 */
Bytestack_index 
Bytestack_sprintf(Bytestack b, const char *fmt, ...) 
{
  va_list ap;

  va_start(ap, fmt);
  Bytestack_index result = Bytestack_vsprintf(b, fmt, ap);
  va_end(ap);
  return result;
}

/**
 * Prints data unto a byte stack.
 * 
 * @param b the byte stack.  May be \c NULL, which indicates the
 * default byte stack.
 * 
 * @param fmt: the printf-style format string to guide the printing.
 * 
 * @param ap: the list of arguments to print.
 *
 * Prints text to the byte stack \p b under guidance of the format
 * string \p fmt.
 *
 * @return the byte stack index that points at the beginning of the
 * newly printed data.
 */
Bytestack_index 
Bytestack_vsprintf(Bytestack b, const char *fmt, va_list ap)
{
  size_t n1, n2;
  Bytestack_index index;

  if (!b)
    b = default_Bytestack();
  index = Bytestack_top(b);
  n1 = b->size - b->cur;
  n2 = vsnprintf(b->begin + b->cur, n1, fmt, ap);
  if (n2 >= n1) {
    enlarge(b, n2 - n1);
    vsnprintf(b->begin + b->cur, n1, fmt, ap);
  }
  b->cur += n2;
  if (b->cur > b->max)
    b->max = b->cur;
  return index;
}

char *
Bytestack_quotify(Bytestack b, Bytestack_index bi1, Bytestack_index bi2)
{
  char *p;
  size_t c, ccur;
  Bytestack_index i;

  if (!b)
    b = default_Bytestack();
  if (!bi2)
    bi2 = Bytestack_top(b);
  if (bi1 < 0 || bi2 < bi1 || bi2 > b->max)
    return NULL;
  p = Bytestack_peek(b, bi1);
  c = 1;
  for (i = bi1; i <= b->cur; i++) {
    switch (*p++) {
    case '\\':
    case '\"':
      c++;
      break;
    }
  }
  if (b->cur < bi1)
    ccur = 0;
  else if (bi2 <= b->cur)
    ccur = c + 1;
  else
    ccur = c;
  for (i = b->cur + 1; i < bi2; i++) {
    switch (*p++) {
    case '\\':
    case '\"':
      c++;
      break;
    }
  }
  c++;
  /* we need to insert c extra characters */
  if (b->cur + c < b->size
      || enlarge(b, c + 1) == 0) {
    p = Bytestack_peek(b, bi2) + c;
    b->cur += ccur;
    if (b->cur > b->max)
      b->max = b->cur;
    *p = p[-c];
    *--p = '\"';
    p--;
    c--;
    for (i = bi1; i < bi2; i++) {
      *p = p[-c];
      switch (*p) {
      case '\\':
      case '\"':
	*--p = '\\';
	c--;
	break;
      }
      p--;
    }
    *p = '\"';
  }
  return p;
}

char *
Bytestack_unescapify(Bytestack b, Bytestack_index bi)
{
  char *p, *q, *r;
  size_t n, i, c, ccur;

  if (!b)
    b = default_Bytestack();
  if (bi < 0 || bi >= b->max)
    return NULL;
  p = q = r = Bytestack_peek(b, bi);
  c = ccur = 0;
  if (*p) {
    while (*p && bi < b->cur) {
      if (*p == '\\') {
	c++;
	p++;
	bi++;
      }
      *q++ = *p++;
      bi++;
    }
    ccur = c;
    while (*p) {
      if (*p == '\\') {
	c++;
	p++;
	bi++;
      }
      *q++ = *p++;
      bi++;
    }
    if (p[-1])
      *q = '\0';
  }
  b->cur -= ccur;
  return r;
}

#if HAVE_COOKIE_IO_FUNCTIONS_T

/** \defgroup bytestackstream Custom stream for ByteStack -- GNU-specific
 *
 * This group of items defines a custom stream that accesses a byte
 * stack.  It uses GNU-specific stuff and is used in the unit tests
 * for Bytestack.c.
 */
/*@{*/

#include <stdio.h>
#include <unistd.h>

/** A stream cookie for Bytestack.  The cookie stores the identity
    of the byte stack and the current position within the byte
    stack. */
typedef struct {
  Bytestack stack;		/**< The byte stack */
  Bytestack_index index;	/**< The byte stack index */
} *Bytestack_cookie;

/** Reads from a byte stack stream.
 *
 * @param cookie the stream cookie.
 *
 * @param buffer the buffer to store the data read from the stream.
 *
 * @param size the maximum number of bytes to read from the stream.
 *
 * Reads up to \p size bytes from the stream identified by the \p
 * cookie and stores the read data in the \p buffer, which must be big
 * enough.
 *
 * @return the number of read bytes.
 * */
static ssize_t 
Bytestack_stream_read(void *cookie, char *buffer, size_t size)
{
  Bytestack_cookie lc = (Bytestack_cookie) cookie;
  Bytestack stack = lc->stack;
  if (lc->index + size > stack->max + 1)
    size = stack->max + 1 - lc->index;
  if (size) {
    memcpy(buffer, stack->begin + lc->index - 1, size);
    lc->index += size;
  }
  return size;
}

/** Writes to a byte stack stream.
 *
 * @param cookie the stream cookie.
 *
 * @param buffer the buffer from where to read the data to be written
 * to the stream.
 *
 * @param size the number of bytes to write to the stream from the
 * buffer.
 *
 * Reads \p size bytes from the \p buffer and writes them to the
 * stream identified by the \p cookie.
 * 
 * @return the number of bytes written.
 */
static ssize_t 
Bytestack_stream_write(void *cookie, const char *buffer,
			  size_t size)
{
  Bytestack_cookie lc = (Bytestack_cookie) cookie;
  Bytestack stack = lc->stack;
  if (lc->index + size > stack->size)
    if (enlarge(stack, stack->size - lc->index - size))
      size = stack->size + 1 - lc->index;
  memcpy(stack->begin + lc->index - 1, buffer, size);
  lc->index += size;
  stack->begin[lc->index - 1] = '\0';
  if (lc->index > stack->max + 1)
    stack->max = lc->index - 1;
  return size;
}

/** Seeks a location in a byte stack stream.
 *
 * @param cookie the stream cookie.
 *
 * @param position the position to seek, in the style of fseek.
 *
 * @param whence the origin from where to measure the position to
 * seek, in the style of fseek.
 *
 * Seeks position \p position relative to the location indicated by \p
 * whence in the byte stack stream identified by \p cookie.
 *
 * @return 0 for success, -1 for failure.
 */
static int 
Bytestack_stream_seek(void *cookie, off64_t *position, int whence)
{
  Bytestack_cookie lc = (Bytestack_cookie) cookie;
  Bytestack stack = lc->stack;
  off64_t i;

  switch (whence) {
  case SEEK_SET:
    i = *position;
    break;
  case SEEK_CUR:
    i = *position + lc->index - 1;
    break;
  case SEEK_END:
    i = *position + stack->max;
    break;
  default:
    return -1;
  }
  if (i < 0 || i >= stack->max)
    return -1;
  *position = i;
  lc->index = i + 1;
  return 0;
}

static void Bytestack_delete_cookie(Bytestack_cookie cookie);

/** Close a byte stack stream.
 *
 * @param cookie the stream cookie.
 *
 * Closes the byte stack stream identified by the \p cookie.
 *
 * @return 0
 */
static int 
Bytestack_stream_close(void *cookie)
{
  Bytestack_cookie lc = (Bytestack_cookie) cookie;
  Bytestack_delete_cookie(lc);
  return 0;
}

/** Identifies functions that implement activities relevant for a
 *  custom stream.
 */
cookie_io_functions_t Bytestack_stream_funcs = 
  {
    Bytestack_stream_read,
    Bytestack_stream_write,
    Bytestack_stream_seek,
    Bytestack_stream_close
  };

/** Creates a byte stack stream cookie.
 *
 * @param stack the byte stack for which to create a cookie.
 *
 * @return the cookie, or \c NULL if an error occurred.
 */
static Bytestack_cookie 
Bytestack_create_cookie(Bytestack stack)
{
  if (!stack)
    stack = default_Bytestack();
  Bytestack_cookie cookie = malloc(sizeof(*cookie));
  if (cookie) {
    cookie->stack = stack;
    if (cookie->stack)
      cookie->index = 0;
    else {
      free(cookie);
      cookie = NULL;
    }
  }
  return cookie;
}

/** Delete a byte stack stream cookie.
 *
 * @param cookie the byte stack stream cookie to delete.
 */
static void 
Bytestack_delete_cookie(Bytestack_cookie cookie)
{
  if (cookie)
    free(cookie);
}

/**
 * Opens a byte stack as a \c FILE.
 * 
 * @param stack the byte stack.  May be \c NULL, which indicates the
 * default byte stack.
 * 
 * @param opentype the type of access that is requested, similar to
 * the access specified in calls to \c fopen, e.g., "r" for reading,
 * "w" for writing.
 *
 * Opens a byte stack as a \c FILE, so that it can be read from and
 * written to as a stream.  The caller is responsible for closing the
 * stream again, using the standard \c fclose function.
 *
 * \warning This feature is specific to the GNU C Compiler and may not
 * be supported by other C compilers.
 *
 * @return the \c FILE pointer.
 */
FILE *
Bytestack_open(Bytestack stack, const char *opentype)
{
  Bytestack_cookie cookie = Bytestack_create_cookie(stack);
  return cookie? fopencookie(cookie, opentype, Bytestack_stream_funcs): 
    NULL;
}

/*@}*/

#endif
