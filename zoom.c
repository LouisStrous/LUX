/* File zoom.c */
/* Image browser by L. Strous */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include "install.h"
#include "action.h"
static char rcsid[] __attribute__ ((unused)) =
 "$Id: zoom.c,v 4.0 2001/02/07 20:37:08 strous Exp $";

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xos.h>
#include <X11/cursorfont.h>

#define WHITE           0
#define BLACK           1

struct Menu {
  int          n_items;		/* number of menu panes (including title) */
  char         **text;		/* pointer to item strings */
  Window       *window;		/* pointer to item windows */
};
typedef        struct Menu     Menu;

extern Display	*display;
extern Menu	menu[MAXMENU];
extern Window	menu_win[], win[];
extern int	ht[], wd[];
extern int	fontwidth;

int	extractNumerical(pointer, pointer, int, int, int *, int *, int, int *),
  ana_xerase(int, int []);
void	paint_pane(int, int, int), delete_menu(int);
/*--------------------------------------------------------------------------*/
void value_string(char *trgt, pointer image, int type, int indx)
{
  switch (type) {
    case ANA_BYTE:
      sprintf(trgt, "Value:% 10d", image.b[indx]);
      break;
    case ANA_WORD:
      sprintf(trgt, "Value:% 10d", image.w[indx]);
      break;
    case ANA_LONG:
      sprintf(trgt, "Value:% 10d", image.l[indx]);
      break;
    case ANA_FLOAT:
      sprintf(trgt, "Value:% #9.4g", image.f[indx]);
      break;
    case ANA_DOUBLE:
      sprintf(trgt, "Value:% #9.4g", image.d[indx]);
      break;
  }	/* end of switch (type) */
}
/*--------------------------------------------------------------------------*/

enum menuItems { ZOOM_TITLE, ZOOM_MAG, ZOOM_STD, ZOOM_TWO,
		 ZOOM_RECENTER, ZOOM_MOVE, ZOOM_CONTRAST,
		 ZOOM_CVALUES, ZOOM_THREE, ZOOM_CHFRAME, ZOOM_PLAY,
		 ZOOM_AXES, ZOOM_FOLLOW, ZOOM_COORDS, ZOOM_FRAME,
		 ZOOM_VALUE, ZOOM_QUIT };

#define ZOOM_PROFILE	-1

float	zoom_xc = 0.0, zoom_yc = 0.0, zoom_mag = 0.0;
double	zoom_clo = 0.0, zoom_chi = 0.0;
int	zoom_frame = 0;

int ana_zoom(int narg, int ps[])
/* ZOOM,image[,bitmap] */
{
  extern int	menu_setup_done, last_wid, threeColors;
  extern scalar	lastmin, lastmax;
  int	createMenu(int num, int x, int y, int nItem, char **item),
    menu_setup(void), ana_xport(int, int []),
    tvraw(pointer data, int type, int nx, int ny, float x1, float x2,
	  float y1, float y2, float sx, float sy, int wid, float *mag,
	  int mode, double clo, double chi, byte *bitmap1, byte *bitmap2),
    threecolors(float *, int);
  int	i, ntext, ndim, *dims, wid, mid, x = 0, y = 0, selected, j, type,
    nx, ny, sx, sy, sx0, sy0, ww, hw, nframe, stride, profile = -1,
    step[MAX_DIMS], coords[2*MAX_DIMS], sdims[3],
    axes[2] = {0, 1}, follow = 0, play = 0, loop, offset, mousepos = 1;
  int	minmax(int *data, int nelem, int type);
  float	x1, x2, y1, y2, dx, dy, z, colorRange = 1.0;
  pointer	data, image, bitmapdata1, bitmap1, bitmapdata2,
    bitmap2;
  char	**zoomText,
    *text[] = { "Zoom            ",
		"Magnify:    1.00",
		"Standard Zoom   ",
		"Zoom  In (2) Out",
		"Recenter        ",
		"Move            ",
		"Fix Contrast Let",
		"       0       0",
		"Default Colors  ",
		"<< <  Step  > >>",
		"Play   <  X  >  ",
		"Dims:     0    1",
		"Mouse Position  ",
		"X:     0 Y:    0",
		"Frame:         0",
		"Value:          ",
		"Quit            " };
  char	*eventName(int), *readPane(int menuid, int menu_item, char *query);
  XEvent	event;

  if (numerical(ps[0], &dims, &ndim, NULL, &data) < 0)
    return ANA_ERROR;
  if (ndim < 2)
    return anaerror("Need at least two dimensions", ps[0]);
  ntext = sizeof(text)/sizeof(char *);
  wid = last_wid;
  type = array_type(ps[0]);

  if (narg > 1 && ps[1]) {
    if (!symbolIsNumericalArray(ps[1])
	|| array_size(ps[1]) != array_size(ps[0]))
      return cerror(INCMP_ARG, ps[1]);
    bitmapdata1.v = array_data(ana_byte(1, &ps[1]));
  } else
    bitmapdata1.v = NULL;

  if (narg > 2 && ps[2]) {
    if (!symbolIsNumericalArray(ps[2])
	|| array_size(ps[2]) != array_size(ps[0]))
      return cerror(INCMP_ARG, ps[2]);
    bitmapdata2.v = array_data(ana_byte(1, &ps[2]));
  } else
    bitmapdata2.v = NULL;

  nx = sdims[0] = dims[axes[0]];
  ny = sdims[1] = dims[axes[1]];
  nframe = sdims[2] = array_size(ps[0])/(nx*ny);
  if (zoom_frame >= nframe)
    zoom_frame = nframe;

  if (win[wid]) {		/* window exists already */
    sx = sx0 = 0.5*wd[wid];
    sy = sy0 = 0.5*ht[wid];
  } else {			/* window will have dimensions 512x512 */
    sx = sx0 = 256;
    sy = sy0 = 256;
  }
  if (zoom_mag == 0 || zoom_xc < 0 || zoom_yc < 0 || zoom_xc >= nx
      || zoom_yc >= ny) {
    zoom_xc = 0.5*nx;
    zoom_yc = 0.5*ny;
    zoom_mag = (int) ((2*sx)/nx);
    i = (int) ((2*sy)/ny);
    if (i < zoom_mag)
      zoom_mag = i;
    if (!zoom_mag)
    zoom_mag = 1.0;
  } else if (zoom_mag <= 0.0) {
    zoom_mag = (int) ((2*sx)/nx);
    i = (int) ((2*sy)/ny);
    if (i < zoom_mag)
      zoom_mag = i;
    if (!zoom_mag)
      zoom_mag = 1.0;
  }
  x1 = zoom_xc - sx/zoom_mag;
  x2 = zoom_xc + sx/zoom_mag;
  y1 = zoom_yc - sy/zoom_mag;
  y2 = zoom_yc + sy/zoom_mag;
  if (x1 < 0)
    x1 = 0;
  else if (x1 > nx - 1)
    x1 = nx - 1;
  if (x2 < 1)
    x2 = 1;
  else if (x2 > nx)
    x2 = nx;
  if (y1 < 0)
    y1 = 0;
  else if (y1 > ny - 1)
    y1 = ny;
  if (y2 < 1)
    y2 = 1;
  else if (y2 > ny)
    y2 = ny;

  if (zoom_frame < 0)
    zoom_frame = 0;
  else if (zoom_frame >= nframe)
    zoom_frame = nframe - 1;
  
  if ((internalMode & 1) == 0)	/* /oldcontrast */
    zoom_clo = zoom_chi = 0.0;
  /* prepare internalMode for tvraw() */
  internalMode = TV_CENTER;

  stride = ana_type_size[type];
  image.b = malloc(nx*ny*stride);
  bitmap1.b = bitmapdata1.v? malloc(nx*ny): NULL;
  bitmap2.b = bitmapdata2.v? malloc(nx*ny): NULL;
  coords[0] = 0;
  coords[1] = nx - 1;
  coords[2] = 0;
  coords[3] = ny - 1;
  j = zoom_frame;
  for (i = 2; i < ndim; i++) {
    coords[2*i] = coords[2*i + 1] = j % dims[i];
    j /= dims[i];
  }
  step[0] = 1;
  for (i = 1; i < ndim; i++)
    step[i] = step[i - 1]*dims[i - 1];
  offset = 0;
  for (i = 0; i < ndim; i++)
    if (i != axes[0] && i != axes[1])
      offset += coords[2*i]*step[i];
  if (extractNumerical(data, image, type, ndim, dims, coords, 2, axes) < 0
      || (bitmap1.b
	  && extractNumerical(bitmapdata1, bitmap1, ANA_BYTE, ndim, dims,
			      coords, 2, axes) < 0)
      || (bitmap2.b
	  && extractNumerical(bitmapdata2, bitmap2, ANA_BYTE, ndim, dims,
			      coords, 2, axes) < 0)) {
    free(image.b);
    free(bitmap1.b);		/* it's OK to free something if it is
				   equal to NULL, so we do not have to
				   check if bitmap1.b is NULL here. */
    free(bitmap2.b);
    return ANA_ERROR;
  }
  ana_xport(0, NULL);
  if ((bitmap1.b || bitmap2.b)
      && !threeColors) {	/* must install three-colors color table */
    z = 1.0;
    threecolors(&z, 1);
  }
  if (tvraw(image, type, nx, ny, x1, x2 - 1, y1, y2 - 1,
	    sx, sy, wid, &zoom_mag, 0, zoom_clo, zoom_chi,
	    bitmap1.b, bitmap2.b) == ANA_ERROR) {
    free(image.b);
    free(bitmap1.b);
    free(bitmap2.b);
    return ANA_ERROR;
  }
  ww = wd[wid];
  hw = ht[wid];

  if (!menu_setup_done && menu_setup() != ANA_OK) {
    free(image.b);
    free(bitmap1.b);
    free(bitmap2.b);
    return ANA_ERROR;
  }
  for (mid = 0; mid < MAXMENU; mid++)
    if (!menu_win[mid])
      break;
  if (mid == MAXMENU)
    return anaerror("No more menus available", 0);
  zoomText = (char **) Malloc(ntext*sizeof(char *));
  if (!zoomText)
    return cerror(ALLOC_ERR, 0);
  for (i = 0; i < ntext; i++) {
    zoomText[i] = strsave(text[i]);
    if (!zoomText[i])
      return cerror(ALLOC_ERR, 0);
  }
  if (createMenu(mid, 0, 0, ntext, zoomText) == ANA_ERROR) {
    free(image.b);
    free(bitmap1.b);
    free(bitmap2.b);
    return ANA_ERROR;
  }
  for (i = 0; i < ntext; i++)
    free(zoomText[i]);
  free(zoomText);
  XMapWindow(display, menu_win[mid]); /* display window */
  XRaiseWindow(display, menu_win[mid]);
  x = zoom_xc;
  y = zoom_yc;
  sprintf(menu[mid].text[ZOOM_COORDS], "X:%5d  Y:%5d", x, y);
  if (x >= 0 && x < nx && y >= 0 && y < ny)
    value_string(menu[mid].text[ZOOM_VALUE], image, type,
		 x + y*nx);
  sprintf(menu[mid].text[ZOOM_FRAME], "Frame:  %8d", zoom_frame);
  sprintf(menu[mid].text[ZOOM_MAG], "Magnify: %7.2f", zoom_mag);
  if (threeColors)
    sprintf(menu[mid].text[ZOOM_THREE], "Colors: %8g", 1.0);
  for (i = 1; i < menu[mid].n_items; i++)
    paint_pane(mid, i, WHITE);
  XFlush(display);

  do {
    do {
      if (play) {
	loop = (XCheckMaskEvent(display, ButtonPressMask | PointerMotionMask
				| EnterWindowMask | LeaveWindowMask, &event)
		== True)? 1: 0;
	if (!loop)
	  event.xany.window = 0;
      } else {
	XMaskEvent(display, ButtonPressMask | PointerMotionMask
		   | EnterWindowMask | LeaveWindowMask, &event);
	loop = 0;
      }

      if (event.xany.window) {
	if (event.xany.window == win[wid]) { /* current ANA window */
	  switch (event.type) {
	    case MotionNotify:	/* pointer movement */
	      /* remove all pointer motion events - we really only want the */
	      /* last one */
	      if (mousepos) {
		while (XCheckMaskEvent(display, PointerMotionMask, &event));
		x = event.xbutton.x;
		y = hw - event.xbutton.y;
		x = (x - sx)/zoom_mag + zoom_xc;
		y = (y - sy)/zoom_mag + zoom_yc;
		if (x < 0)
		  x = 0;
		else if (x >= nx)
		  x = nx - 1;
		if (y < 0)
		  y = 0;
		else if (y >= ny)
		  y = ny - 1;
		sprintf(menu[mid].text[ZOOM_COORDS], "X:%5d  Y:%5d", x, y);
		paint_pane(mid, ZOOM_COORDS, WHITE);
		if (x < nx && y < ny) {
		  value_string(menu[mid].text[ZOOM_VALUE], image, type,
			       x + y*nx);
		  paint_pane(mid, ZOOM_VALUE, WHITE);
		} /* end of if (x < nx && y < ny) */
	      }
	      if (follow) {
		while (XCheckMaskEvent(display, PointerMotionMask, &event));
		dx = event.xbutton.x - sx;
		dy = hw - event.xbutton.y - sy;
		z = sqrt(dx*dx + dy*dy);
		if (z) {
		  dx /= z;
		  dy /= z;
		}
		z = z/50 - 1;
		if (z > 0) {
		  dx *= z;
		  dy *= z;
		  x1 = zoom_xc + dx - sx/zoom_mag;
		  x2 = zoom_xc + dx + sx/zoom_mag;
		  y1 = zoom_yc + dy - sy/zoom_mag;
		  y2 = zoom_yc + dy + sy/zoom_mag;
		  sx = sx0;
		  sy = sy0;
		  if (x1 < 0)
		    x1 = 0;
		  else if (x1 > nx - 1)
		    x1 = nx - 1;
		  if (x2 < 1)
		    x2 = 1;
		  else if (x2 > nx)
		    x2 = nx;
		  if (y1 < 0)
		    y1 = 0;
		  else if (y1 > ny - 1)
		    y1 = ny;
		  if (y2 < 1)
		    y2 = 1;
		  else if (y2 > ny)
		    y2 = ny;
		  zoom_xc += dx;
		  zoom_yc += dy;
		  if (tvraw(image, type, nx, ny, x1, x2 - 1, y1, y2 - 1, sx,
			    sy, wid, &zoom_mag, TV_CENTER, zoom_clo,
			    zoom_chi, bitmap1.b, bitmap2.b) < 0) {
		    free(image.b);
		    free(bitmap1.b);
		    free(bitmap2.b);
		    return ANA_ERROR;
		  }
		}
	      }		
	      break;
	    case ButtonPress:
	      switch (selected) {
		case ZOOM_RECENTER:
		  dx = (event.xbutton.x - sx)/zoom_mag;
		  dy = (hw - event.xbutton.y - sy)/zoom_mag;
		  x1 = zoom_xc + dx - sx/zoom_mag;
		  x2 = zoom_xc + dx + sx/zoom_mag;
		  y1 = zoom_yc + dy - sy/zoom_mag;
		  y2 = zoom_yc + dy + sy/zoom_mag;
		  if (x1 < 0)
		    x1 = 0;
		  if (x2 > nx)
		    x2 = nx;
		  if (y1 < 0)
		    y1 = 0;
		  if (y2 > ny)
		    y2 = ny;
		  zoom_xc += dx;
		  zoom_yc += dy;
		  if (tvraw(image, type, nx, ny, x1, x2 - 1, y1, y2 - 1,
			    sx, sy, wid, &zoom_mag, TV_CENTER, zoom_clo,
			    zoom_chi, bitmap1.b, bitmap2.b) < 0) {
		    free(image.b);
		    free(bitmap1.b);
		    free(bitmap2.b);
		    return ANA_ERROR;
		  }
		  sprintf(menu[mid].text[ZOOM_RECENTER], "Recenter        ");
		  paint_pane(mid, ZOOM_RECENTER, WHITE);
		  selected = 0;
		  break;
		case ZOOM_MOVE:
		  follow = 0;
		  sprintf(menu[mid].text[ZOOM_MOVE], "Move            ");
		  paint_pane(mid, ZOOM_MOVE, WHITE);
		  selected = 0;
		  break;
		case ZOOM_FOLLOW:
		  mousepos = 0;
		  sprintf(menu[mid].text[ZOOM_FOLLOW], "Fixed Position  ");
		  paint_pane(mid, ZOOM_FOLLOW, WHITE);
		  selected = 0;
		  break;
	      }
	      break;
	  } /* end of switch (event.type) */
	} else {			/* the zoom menu? */
	  for (j = 0; j < menu[mid].n_items; j++)
	    if (event.xany.window == menu[mid].window[j])
	      break;
	  if (j == menu[mid].n_items)
	    continue;		/* no match */
	  j++;
	  switch (event.type) {
	    case EnterNotify:
	      paint_pane(mid, j, BLACK);
	      break;
	    case LeaveNotify:
	      paint_pane(mid, j, WHITE);
	      break;
	    case ButtonPress:
	      switch (j) {
		case ZOOM_MAG:	/* magnification */
		  zoom_mag = atof(readPane(mid, ZOOM_MAG, "Magnify"));
		  if (zoom_mag < 0)
		    zoom_mag = -zoom_mag;
		  if (!zoom_mag)
		    zoom_mag = 1;
		  x1 = zoom_xc - sx/zoom_mag;
		  x2 = zoom_xc + sx/zoom_mag;
		  y1 = zoom_yc - sy/zoom_mag;
		  y2 = zoom_yc + sy/zoom_mag;
		  if (x1 < 0)
		    x1 = 0;
		  else if (x1 > nx)
		    x1 = nx;
		  if (x2 < 0)
		    x2 = 0;
		  if (x2 > nx)
		    x2 = nx;
		  if (y1 < 0)
		    y1 = 0;
		  else if (y1 > ny)
		    y1 = ny;
		  if (y2 < 0)
		    y2 = 0;
		  if (y2 > ny)
		    y2 = ny;
		  if (tvraw(image, type, nx, ny, x1, x2 - 1, y1, y2 - 1,
			    sx, sy, wid, &zoom_mag, TV_CENTER, zoom_clo, zoom_chi,
			    bitmap1.b, bitmap2.b) < 0) {
		    free(image.b);
		    free(bitmap1.b);
		    free(bitmap2.b);
		    return ANA_ERROR;
		  }
		  sprintf(menu[mid].text[ZOOM_MAG], "Magnify: %7.2f",
			  zoom_mag);
		  paint_pane(mid, ZOOM_MAG, BLACK);
		  break;
		case ZOOM_STD:	/* standard zoom */
		  x1 = y1 = 0;
		  x2 = nx;
		  y2 = ny;
		  zoom_xc = 0.5*nx;
		  zoom_yc = 0.5*ny;
		  zoom_mag = 1.0;
		  if (tvraw(image, type, nx, ny, x1, x2 - 1, y1, y2 - 1,
			    sx, sy, wid, &zoom_mag, TV_CENTER, zoom_clo,
			    zoom_chi, bitmap1.b, bitmap2.b) < 0) {
		    free(image.b);
		    free(bitmap1.b);
		    free(bitmap2.b);
		    return ANA_ERROR;
		  }
		  sprintf(menu[mid].text[ZOOM_MAG], "Magnify: %7.2f",
			  zoom_mag);
		  paint_pane(mid, ZOOM_MAG, WHITE);
		  break;
		case ZOOM_TWO:	/* zoom in/out by a factor of 2 */
		  z = (float) event.xbutton.x/((float) fontwidth);
		  if (z < 10.5) {	/* zoom in */
		    x1 = zoom_xc - sx/zoom_mag;
		    x2 = zoom_xc + sx/zoom_mag;
		    y1 = zoom_yc - sy/zoom_mag;
		    y2 = zoom_yc + sy/zoom_mag;
		    x1 = (x1 + zoom_xc)*0.5;
		    x2 = (x2 + zoom_xc)*0.5;
		    y1 = (y1 + zoom_yc)*0.5;
		    y2 = (y2 + zoom_yc)*0.5;
		    zoom_mag *= 2;
		  } else {	/* zoom out */
		    x1 = zoom_xc - sx/zoom_mag;
		    x2 = zoom_xc + sx/zoom_mag;
		    y1 = zoom_yc - sy/zoom_mag;
		    y2 = zoom_yc + sy/zoom_mag;
		    x1 = 2*x1 - zoom_xc;
		    x2 = 2*x2 - zoom_xc;
		    y1 = 2*y1 - zoom_yc;
		    y2 = 2*y2 - zoom_yc;
		    zoom_mag /= 2;
		  }
		  if (x1 < 0)
		    x1 = 0;
		  if (x2 > nx)
		    x2 = nx;
		  if (y1 < 0)
		    y1 = 0;
		  if (y2 > ny)
		    y2 = ny;
		  sprintf(menu[mid].text[ZOOM_MAG], "Magnify: %7.2f",
			  zoom_mag);
		  paint_pane(mid, ZOOM_MAG, WHITE);
		  if (tvraw(image, type, nx, ny, x1, x2 - 1, y1, y2 - 1,
			    sx, sy, wid, &zoom_mag, TV_CENTER, zoom_clo,
			    zoom_chi, bitmap1.b, bitmap2.b) < 0) {
		    free(image.b);
		    free(bitmap1.b);
		    free(bitmap2.b);
		    return ANA_ERROR;
		  }
		  break;
		case ZOOM_RECENTER:
		  sprintf(menu[mid].text[ZOOM_RECENTER], "> Select Now");
		  paint_pane(mid, ZOOM_RECENTER, BLACK);
		  selected = ZOOM_RECENTER;
		  break;
		case ZOOM_MOVE:
		  follow = !follow;
		  if (follow)
		    sprintf(menu[mid].text[ZOOM_MOVE], "> Moving...     ");
		  else 
		    sprintf(menu[mid].text[ZOOM_MOVE], "Move            ");
		  paint_pane(mid, ZOOM_MOVE, WHITE);
		  selected = ZOOM_MOVE;
		  break;
		case ZOOM_PROFILE:
		  profile++;
		  if (profile == ndim)
		    profile = -1;
		  if (profile >= 0)
		    sprintf(menu[mid].text[ZOOM_PROFILE], "Profile %1d",
			    profile);
		  else
		    sprintf(menu[mid].text[ZOOM_PROFILE], "Profile");
		  paint_pane(mid, ZOOM_PROFILE, BLACK);
		  break;
		case ZOOM_CONTRAST:
		  i = event.xbutton.x/fontwidth;
		  if (i < 8) {	/* take from current image */
		    minmax(image.l, nx*ny, type);
		    switch (type) {
		      case ANA_BYTE:
			zoom_clo = (double) lastmin.b;
			zoom_chi = (double) lastmax.b;
			break;
		      case ANA_WORD:
			zoom_clo = (double) lastmin.w;
			zoom_chi = (double) lastmax.w;
			break;
		      case ANA_LONG:
			zoom_clo = (double) lastmin.l;
			zoom_chi = (double) lastmax.l;
			break;
		      case ANA_FLOAT:
			zoom_clo = (double) lastmin.f;
			zoom_chi = (double) lastmax.f;
			break;
		      case ANA_DOUBLE:
			zoom_clo = (double) lastmin.d;
			zoom_chi = (double) lastmax.d;
			break;
		    }
		  } else
		    zoom_clo = zoom_chi = 0.0;
		  sprintf(menu[mid].text[ZOOM_CVALUES], "%8.4g%8.4g",
			  zoom_clo, zoom_chi);
		  paint_pane(mid, ZOOM_CVALUES, WHITE);
		  if (tvraw(image, type, nx, ny, x1, x2 - 1, y1, y2 - 1,
			    sx, sy, wid, &zoom_mag, TV_CENTER, zoom_clo,
			    zoom_chi, bitmap1.b, bitmap2.b) < 0) {
		    free(image.b);
		    free(bitmap1.b);
		    free(bitmap2.b);
		    return ANA_ERROR;
		  }
		  break;
		case ZOOM_CVALUES:
		  i = event.xbutton.x/fontwidth;
		  if (i < 8) 	/* take from current image */
		    zoom_clo = atof(readPane(mid, ZOOM_CVALUES, "Low"));
		  else
		    zoom_chi = atof(readPane(mid, ZOOM_CVALUES, "High"));
		  sprintf(menu[mid].text[ZOOM_CVALUES], "%8.4g%8.4g",
			  zoom_clo, zoom_chi);
		  paint_pane(mid, ZOOM_CVALUES, WHITE);
		  if (tvraw(image, type, nx, ny, x1, x2 - 1, y1, y2 - 1,
			    sx, sy, wid, &zoom_mag, TV_CENTER,
			    zoom_clo, zoom_chi,
			    bitmap1.b, bitmap2.b) < 0) {
		    free(image.b);
		    free(bitmap1.b);
		    free(bitmap2.b);
		    return ANA_ERROR;
		  }
		  break;
		case ZOOM_THREE:
		  colorRange = atof(readPane(mid, ZOOM_THREE, "Range"));
		  if (colorRange < -1)
		    colorRange = -1;
		  else if (colorRange > 1)
		    colorRange = 1;
		  sprintf(menu[mid].text[ZOOM_THREE], "Colors: %8g",
			  colorRange);
		  threecolors(&colorRange, 1);
		  paint_pane(mid, ZOOM_THREE, WHITE);
		case ZOOM_CHFRAME:
		  i = event.xbutton.x/fontwidth;
		  if (i < 2 && nframe > 5) { /* go back fast */
		    zoom_frame -= 5;
		    if (zoom_frame < 0)
		      zoom_frame += nframe;
		  } else if (i < 8) { /* go back */
		    zoom_frame--;
		    if (zoom_frame < 0)
		      zoom_frame += nframe;
		  } else if (i > 14 && nframe > 5) { /* go forward fast */
		    zoom_frame += 5;
		    if (zoom_frame >= nframe)
		      zoom_frame -= nframe;
		  } else {	/* go forward */
		    zoom_frame++;
		    if (zoom_frame == nframe)
		      zoom_frame = 0;
		  }
		  j = zoom_frame;
		  offset = 0;
		  for (i = 0; i < ndim; i++)
		    if (axes[0] != i && axes[1] != i) {
		      coords[2*i] = coords[2*i + 1] = j % dims[i];
		      offset += coords[2*i]*step[i];
		      j /= dims[i];
		    }
		  if (extractNumerical(data, image, type, ndim, dims, coords,
				       2, axes) < 0
		      || (bitmapdata1.v
			  && extractNumerical(bitmapdata1, bitmap1, ANA_BYTE,
					      ndim, dims, coords, 2, axes)
			  < 0)
		      || (bitmapdata2.v
			  && extractNumerical(bitmapdata2, bitmap2, ANA_BYTE,
					      ndim, dims, coords, 2, axes)
			  < 0)) {
		    free(image.b);
		    free(bitmap1.b);
		    free(bitmap2.b);
		    return ANA_ERROR;
		  }
		  if (tvraw(image, type, nx, ny, x1, x2 - 1, y1, y2 - 1,
			    sx, sy, wid, &zoom_mag, TV_CENTER, zoom_clo,
			    zoom_chi, bitmap1.b, bitmap2.b) < 0) {
		    free(image.b);
		    free(bitmap1.b);
		    free(bitmap2.b);
		    return ANA_ERROR;
		  }
		  sprintf(menu[mid].text[ZOOM_FRAME], "Frame:  %8d",
			  zoom_frame);
		  paint_pane(mid, ZOOM_FRAME, WHITE);
		  value_string(menu[mid].text[ZOOM_VALUE], image, type,
			       x + y*nx);
		  paint_pane(mid, ZOOM_VALUE, WHITE);
		  break;
		case ZOOM_FOLLOW:
		  if (mousepos) {
		    sprintf(menu[mid].text[ZOOM_FOLLOW], "> Select Place");
		    paint_pane(mid, ZOOM_FOLLOW, BLACK);
		    selected = ZOOM_FOLLOW;
		  } else {
		    sprintf(menu[mid].text[ZOOM_FOLLOW], "Mouse Position");
		    paint_pane(mid, ZOOM_FOLLOW, BLACK);
		    mousepos = 1;
		  }
		  break;
		case ZOOM_AXES:
		  if (axes[0] != axes[1]) {
		    for (i = 0; i < ndim; i++) /* determine current coords */
		      if (axes[0] == i)
			coords[2*i] = x;
		      else if (axes[1] == i)
			  coords[2*i] = y;
		      else {
			coords[2*i] = zoom_frame % dims[i];
			zoom_frame /= dims[i];
		      }
		  }

		  i = event.xbutton.x/fontwidth;
		  if (i < 11) {	/* first dimension */
		    axes[0]++;
		    if (axes[0] == ndim)
		      axes[0] = 0;
		  } else {	/* second dimension */
		    axes[1]++;
		    if (axes[1] == ndim)
		      axes[1] = 0;
		  }
		  if (axes[0] == axes[1])
		    sprintf(menu[mid].text[ZOOM_AXES], "Dims* %5d%5d", axes[0],
			    axes[1]);
		  else {
		    sprintf(menu[mid].text[ZOOM_AXES], "Dims: %5d%5d", axes[0],
			    axes[1]);
		    nx = dims[axes[0]];
		    ny = dims[axes[1]];
		    nframe = array_size(ps[0])/(nx*ny);
		    /* now determine x, y, frame in the new system */
		    x = zoom_xc = coords[2*axes[0]];
		    y = zoom_yc = coords[2*axes[1]];
		    zoom_frame = 0;
		    for (i = ndim - 1; i >= 0; i--)
		      if (axes[0] != i && axes[1] != i)
			zoom_frame = zoom_frame*dims[i] + coords[2*i];

		    x1 = zoom_xc - 0.5*ww/zoom_mag;
		    x2 = zoom_xc + 0.5*ww/zoom_mag;
		    y1 = zoom_yc - 0.5*hw/zoom_mag;
		    y2 = zoom_yc + 0.5*hw/zoom_mag;
		    if (x1 < 0) {
		      x2 -= x1;
		      x1 = 0;
		    } else if (x1 > nx - 1)
		      x1 = nx - 1;
		    if (x2 < 1)
		      x2 = 1;
		    if (x2 > nx) {
		      x1 += nx - x2;
		      if (x1 < 0)
			x1 = 0;
		      x2 = nx;
		    }
		    if (y1 < 0) {
		      y2 -= y1;
		      y1 = 0;
		    }
		    if (y1 > ny - 1)
		      y1 = ny - 1;
		    if (y2 < 1)
		      y2 = 1;
		    if (y2 > ny) {
		      y1 += ny - y2;
		      if (y1 < 0)
			y1 = 0;
		      y2 = ny;
		    }
		    zoom_xc = (x1 + x2)*0.5;
		    zoom_yc = (y1 + y2)*0.5;
		      
		    offset = 0;
		    for (i = 0; i < ndim; i++)
		      if (axes[0] == i) {
			coords[2*i] = 0;
			coords[2*i + 1] = nx - 1;
		      } else if (axes[1] == i) {
			coords[2*i] = 0;
			coords[2*i + 1] = ny - 1;
		      } else {
			coords[2*i + 1] = coords[2*i];
			offset += coords[2*i]*step[i];
		      }

		    image.b = Realloc(image.b, nx*ny*stride);
		    if (!image.b)
		      return cerror(ALLOC_ERR, 0);
		    if (extractNumerical(data, image, type, ndim, dims,
					 coords, 2, axes) < 0
			|| (bitmapdata1.v
			    && extractNumerical(bitmapdata1, bitmap1, ANA_BYTE,
						ndim, dims, coords, 2, axes)
			    < 0)
			|| (bitmapdata2.v
			    && extractNumerical(bitmapdata2, bitmap2, ANA_BYTE,
						ndim, dims, coords, 2, axes)
			    < 0)) {
		      free(image.b);
		      free(bitmap1.b);
		      free(bitmap2.b);
		      return ANA_ERROR;
		    }
		    if (tvraw(image, type, nx, ny, x1, x2 - 1, y1, y2 - 1,
			    sx, sy, wid, &zoom_mag, TV_CENTER, zoom_clo,
			    zoom_chi, bitmap1.b, bitmap2.b)
			< 0) {
		      free(image.b);
		      free(bitmap1.b);
		      free(bitmap2.b);
		      return ANA_ERROR;
		    }
		    sprintf(menu[mid].text[ZOOM_FRAME], "Frame:  %8d",
			    zoom_frame);
		    paint_pane(mid, ZOOM_FRAME, WHITE);
		    sprintf(menu[mid].text[ZOOM_COORDS], "X:%5d  Y:%5d", x,
			    y);
		    paint_pane(mid, ZOOM_COORDS, WHITE);
		    value_string(menu[mid].text[ZOOM_VALUE], image, type,
				 zoom_xc + zoom_yc*nx);
		    paint_pane(mid, ZOOM_VALUE, WHITE);
		  }
		  paint_pane(mid, ZOOM_AXES, BLACK);
		  break;
		case ZOOM_PLAY:
		  i = event.xbutton.x/fontwidth;
		  if (i < 9) 	/* play backwards */
		    play--;
		  else if (i > 11)
		    play++;
		  else
		    play = 0;
		  if (play >= nframe)
		    play = nframe - 1;
		  else if (play <= -nframe)
		    play = 1 - nframe;
		  switch (play) {
		    case -2:
		      strcpy(menu[mid].text[ZOOM_PLAY], "Play  <<< X  >  ");
		      break;
		    case -1:
		      strcpy(menu[mid].text[ZOOM_PLAY], "Play   << X  >  ");
		      break;
		    case 0:
		      strcpy(menu[mid].text[ZOOM_PLAY], "Play   <  X  >  ");
		      break;
		    case 1:
		      strcpy(menu[mid].text[ZOOM_PLAY], "Play   <  X >>  ");
		      break;
		    case 2:
		      strcpy(menu[mid].text[ZOOM_PLAY], "Play   <  X >>> ");
		      break;
		    default:
		      if (play > 0)
			strcpy(menu[mid].text[ZOOM_PLAY], "Play   <  X >>>>");
		      else
			strcpy(menu[mid].text[ZOOM_PLAY], "Play <<<< X  >  ");
		      break;
		  }
		  paint_pane(mid, ZOOM_PLAY, BLACK);
		  break;
		case ZOOM_QUIT:
		  delete_menu(mid);
		  XFlush(display);
		  return 1;
	      } /* end of switch (j) */
	      break;
	  } /* end of switch (event.type) */
	}	/* end of if (event.xany.window == win[wid]) */
      } /* end of if (event.xany.window) */
    } while (loop);
    
    if (play && axes[0] != axes[1]) {
      zoom_frame += play;
      if (zoom_frame < 0)
	zoom_frame += nframe;
      else if (zoom_frame >= nframe)
	zoom_frame -= nframe;
      j = zoom_frame;
      offset = 0;
      for (i = 0; i < ndim; i++)
	if (axes[0] != i && axes[1] != i) {
	  coords[2*i] = coords[2*i + 1] = j % dims[i];
	  offset += coords[2*i]*step[i];
	  j /= dims[i];
	}
      if (extractNumerical(data, image, type, ndim, dims,
			   coords, 2, axes) < 0
	  || (bitmapdata1.v
	      && extractNumerical(bitmapdata1, bitmap1, ANA_BYTE,
				  ndim, dims, coords, 2, axes) < 0)
	  || (bitmapdata2.v
	      && extractNumerical(bitmapdata2, bitmap2, ANA_BYTE,
				  ndim, dims, coords, 2, axes) < 0)) {
	free(image.b);
	free(bitmap1.b);
	free(bitmap2.b);
	return ANA_ERROR;
      }
      if (tvraw(image, type, nx, ny, x1, x2 - 1, y1, y2 - 1, sx, sy,
		wid, &zoom_mag, TV_CENTER, zoom_clo, zoom_chi, bitmap1.b,
		bitmap2.b) < 0) {
	free(image.b);
	free(bitmap1.b);
	free(bitmap2.b);
	return ANA_ERROR;
      }
      if (play) {
	sprintf(menu[mid].text[ZOOM_FRAME], "Frame:  %8d", zoom_frame);
	paint_pane(mid, ZOOM_FRAME, WHITE);
	sprintf(menu[mid].text[ZOOM_COORDS], "X:%5d  Y:%5d", x, y);
	paint_pane(mid, ZOOM_COORDS, WHITE);
	if (x < nx && y < ny) {
	  value_string(menu[mid].text[ZOOM_VALUE], image, type,
		       x + y*nx);
	  paint_pane(mid, ZOOM_VALUE, WHITE);
	} /* end of if (x < nx && y < ny) */
      }
    }
  } while (1);
}
/*--------------------------------------------------------------------------*/
int tvzoom(int narg, int ps[])
{
  int	*dims, ndim, axes[2] = {0, 1}, sdims[3], nx, ny, wid;
  int	coords[4], offset, type, sx, sy, i, stride, step[2];
  extern int	last_wid;
  int	tvraw(pointer data, int type, int nx, int ny, float x1, float x2,
	      float y1, float y2, float sx, float sy, int wid, float *mag,
	      int mode, double clo, double chi, byte *bitmap1, byte *bitmap2);

  pointer	data, image, bitmapdata, bitmap;
  float	x1, x2, y1, y2;

  if (numerical(ps[0], &dims, &ndim, NULL, &data) < 0)
    return ANA_ERROR;
  if (ndim != 2)
    return cerror(NEED_2D_ARR, ps[0]);
  type = array_type(ps[0]);

  wid = last_wid;

  if (narg > 1 && ps[1]) {
    if (symbol_class(ps[1]) != ANA_ARRAY
	|| array_size(ps[1]) != array_size(ps[0])
	|| !isNumericalType(array_type(ps[1])))
      return cerror(INCMP_ARG, ps[1]);
    bitmapdata.v = array_data(ana_byte(1, &ps[1]));
  } else
    bitmapdata.v = NULL;

  nx = sdims[0] = dims[axes[0]];
  ny = sdims[1] = dims[axes[1]];

  if (win[wid]) {		/* window exists already */
    sx = 0.5*wd[wid];
    sy = 0.5*ht[wid];
  } else {			/* window will have dimensions 512x512 */
    sx = 256;
    sy = 256;
  }
  if (zoom_mag == 0 || zoom_xc < 0 || zoom_yc < 0 || zoom_xc >= nx
      || zoom_yc >= ny) {
    zoom_xc = 0.5*nx;
    zoom_yc = 0.5*ny;
    zoom_mag = (int) ((2*sx)/nx);
    i = (int) ((2*sy)/ny);
    if (i < zoom_mag)
      zoom_mag = i;
    if (!zoom_mag)
    zoom_mag = 1.0;
  } else if (zoom_mag <= 0.0) {
    zoom_mag = (int) ((2*sx)/nx);
    i = (int) ((2*sy)/ny);
    if (i < zoom_mag)
      zoom_mag = i;
    if (!zoom_mag)
      zoom_mag = 1.0;
  }
  x1 = zoom_xc - sx/zoom_mag;
  x2 = zoom_xc + sx/zoom_mag;
  y1 = zoom_yc - sy/zoom_mag;
  y2 = zoom_yc + sy/zoom_mag;
  if (x1 < 0)
    x1 = 0;
  else if (x1 > nx)
    x1 = nx;
  if (x2 < 0)
    x2 = 0;
  else if (x2 > nx)
    x2 = nx;
  if (y1 < 0)
    y1 = 0;
  else if (y1 > ny)
    y1 = ny;
  if (y2 < 0)
    y2 = 0;
  else if (y2 > ny)
    y2 = ny;
  
  stride = ana_type_size[type];
  image.b = malloc(nx*ny*stride);
  bitmap.b = bitmapdata.v? malloc(nx*ny): NULL;
  coords[0] = 0;
  coords[1] = nx - 1;
  coords[2] = 0;
  coords[3] = ny - 1;
  step[0] = 1;
  for (i = 1; i < ndim; i++)
    step[i] = step[i - 1]*dims[i - 1];
  offset = 0;
  for (i = 0; i < ndim; i++)
    if (i != axes[0] && i != axes[1])
      offset += coords[2*i]*step[i];
  if (extractNumerical(data, image, type, ndim, dims, coords, 2, axes) < 0
      || (bitmap.b
	  && extractNumerical(bitmapdata, bitmap, ANA_BYTE, ndim, dims,
			      coords, 2, axes) < 0)) {
    free(image.b);
    free(bitmap.b);		/* it's OK to free something if it is
				   equal to NULL, so we do not have to
				   check if bitmap.b is NULL here. */
    return ANA_ERROR;
  }
  if (tvraw(image, type, nx, ny, x1, x2 - 1, y1, y2 - 1,
	    sx, sy, wid, &zoom_mag, TV_CENTER, zoom_clo, zoom_chi,
	    bitmap.b, NULL) == ANA_ERROR) {
    free(image.b);
    free(bitmap.b);
    return ANA_ERROR;
  }
  return ANA_OK;
}
/*--------------------------------------------------------------------------*/
