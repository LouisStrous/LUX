/* Compile this file as C; it won't compile as C++ because of the
   member called "class".  This is OK in C, but is a reserved keyword
   in C++ */
#include "config.h"
#if HAVE_LIBX11
#include <X11/Xlib.h>           /* for Visual */
#include <X11/Xutil.h>          /* for XVisualInfo */

int visualclass(Visual *v) {
  return v->class;
}

int xvisualinfoclass(XVisualInfo v) {
  return v.class;
}

#endif
