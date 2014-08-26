#include "config.h"

#if HAVE_LIBX11
#include <X11/Xlib.h>
#include <X11/Xutil.h>		/* for XVisualInfo */

int visualclass(Visual *v);
int xvisualinfoclass(XVisualInfo v);

#endif
