#ifndef HAVE_HERSHEY_H
#define HAVE_HERSHEY_H

enum { HERSHEY_DRAW, HERSHEY_MOVE, HERSHEY_END, HERSHEY_ERR = -1 };

typedef Int hershey_handle;

hershey_handle hershey_exists(Int hershey_char);
Int hershey_coords(hershey_handle *handle, Int *x, Int *y);
Int hershey_max_handle(void);
Int hershey_max_char(void);
char *hershey_set_filename(char *filename);

#endif

