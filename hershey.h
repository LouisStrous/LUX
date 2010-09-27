#ifndef HAVE_HERSHEY_H
#define HAVE_HERSHEY_H

enum { HERSHEY_DRAW, HERSHEY_MOVE, HERSHEY_END, HERSHEY_ERR = -1 };

typedef int hershey_handle;

hershey_handle hershey_exists(int hershey_char);
int hershey_coords(hershey_handle *handle, int *x, int *y);
int hershey_max_handle(void);
int hershey_max_char(void);
char *hershey_set_filename(char *filename);

#endif

