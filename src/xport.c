/* This is file xport.c.

Copyright 2013 Louis Strous, Richard Shine

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
/* File xport.c */
/* LUX routines dealing with (X window) data windows. */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>		/* key symbol definitions LS 23nov92 */
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "install.h"
#include <time.h>
#include <float.h>
#include <limits.h>
#include "action.h"
 /*following is icon bitmap */
#include "lux_bitmap.xbm"

#define BITMAPDEPTH     1
#define TOO_SMALL       0
#define BIG_ENOUGH      1
#define WHITE_BACKGR	2

int32_t	coordTrf(float *, float *, int32_t, int32_t),
  checkCoordSys(int32_t mode, int32_t defaultmode);

int32_t	setup_x(void), anaAllocNamedColor(char *, XColor **);

extern  int32_t     scalemin, scalemax, setup, connect_flag, visualClass;
extern  float   xfac, yfac, xlimit, ylimit;
extern  int32_t     ixlow, iylow, ixhigh, iyhigh, threeColors;
char    *display_name = NULL;
int32_t     last_wid = 0;

double  last_time;
int32_t     xcoord, ycoord, lux_button, lux_keycode, lux_keypress, root_x, root_y,
	preventEventFlush = 0, lux_keystate, lux_keysym;
uint32_t	kb;
float	xhair, yhair, tvscale = 1.0;
Window  win[MAXWINDOWS];
/* the size of each of the windows and pixmaps for our use */

int32_t     wd[MAXWINDOWS], ht[MAXWINDOWS], wdmap[MAXPIXMAPS], htmap[MAXPIXMAPS];
int32_t     xerrors;
extern Atom	wm_delete;
GC      gc[MAXWINDOWS], gcmap[MAXPIXMAPS];
int32_t	drawingareas[MAXWINDOWS]; /* used in motif.c */
XFontStruct     *font_info[MAXWINDOWS];
XImage  *xi;
Pixmap  icon_pixmap, maps[MAXPIXMAPS], back_pixmap;
extern Colormap        colorMap;
int32_t	xold, yold;
float     tvix, tviy, tvixb, tviyb;
extern GC	gcnot;

/* from color.c */
extern unsigned long	black_pixel, white_pixel, *pixels;
extern Visual	*visual;
extern uint32_t	depth;
extern int32_t	private_colormap, screen_num;
extern uint32_t	display_width, display_height, display_cells, nColors;
extern Display	*display;

extern char   *strsave(char  *s), *visualNames[];

int32_t	freeCellIndex, nFreeCells;

#ifdef MOTIF
#include <X11/Intrinsic.h>
extern Widget	lux_widget_id[MAXWIDGETS];
#endif

/* revised color handling (LS 18jan94): */
/* data -> color index -> pixel value -> screen */
/*  * SCALE     *  COLORPIXEL  *    TVRAW   *   */
/*  * SCALE     *            TVMAP          *   */
/*  *                 TV                    *   */

/*--------------------------------------------------------------------------*/
int32_t xerr(Display *display, XErrorEvent *err)
/* our own non-fatal X window error routine */
/* essentially copied from manual */
/* Expanded 9 oct 97 - LS */
{
  char    msg[80];
  
  XGetErrorText(display, err->error_code, msg, 80);
  printwf("X window error: %s\n", msg);
  printwf("Request code: %1d (%1d)\n", err->request_code,
	  err->minor_code);
  xerrors += 1;
  return LUX_ERROR;
}
 /*--------------------------------------------------------------------------*/
void eventType(int32_t type)
/* reports the name corresponding to the event type */
{
  static unsigned long	count = 0;

  count++;
  printf("%d: ", count);
  switch (type) {
    case KeyPress: 
      puts("KeyPress");
      break;
    case KeyRelease:
      puts("KeyRelease");
      break;
    case ButtonPress:
      puts("ButtonPress");
      break;
    case ButtonRelease:
      puts("ButtonRelease");
      break;
    case MotionNotify:
      puts("MotionNotify");
      break;
    case EnterNotify:
      puts("EnterNotify");
      break;
    case LeaveNotify:
      puts("LeaveNotify");
      break;
    case FocusIn:
      puts("FocusIn");
      break;
    case FocusOut:
      puts("FocusOut");
      break;
    case KeymapNotify:
      puts("KeymapNotify");
      break;
    case Expose:
      puts("Expose");
      break;
    case GraphicsExpose: 
      puts("GraphicsExpose");
      break;
    case NoExpose:
      puts("NoExpose");
      break;
    case CirculateRequest:
      puts("CirculateRequest");
      break;
    case ConfigureRequest:
      puts("ConfigureRequest");
      break;
    case MapRequest:
      puts("MapRequest");
      break;
    case ResizeRequest:
      puts("ResizeRequest");
      break;
    case CirculateNotify:
      puts("CirculateNotify");
      break;
    case ConfigureNotify:
      puts("ConfigureNotify");
      break;
    case CreateNotify:
      puts("CreateNotify");
      break;
    case DestroyNotify:
      puts("DestroyNotify");
      break;
    case GravityNotify:
      puts("GravityNotify");
      break;
    case MapNotify:
      puts("MapNotify");
      break;
    case MappingNotify:
      puts("MappingNotify");
      break;
    case ReparentNotify:
      puts("ReparentNotify");
      break;
    case UnmapNotify:
      puts("UnmapNotify");
      break;
    case VisibilityNotify:
      puts("VisibilityNotify");
      break;
    case ColormapNotify:
      puts("ColormapNotify");
      break;
    case ClientMessage:
      puts("ClientMessage");
      break;
    case PropertyNotify:
      puts("PropertyNotify");
      break;
    case SelectionClear:
      puts("SelectionClear");
      break;
    case SelectionNotify:
      puts("SelectionNotify");
      break;
    case SelectionRequest:
      puts("SelectionRequest");
      break;
  }
}
 /*--------------------------------------------------------------------------*/
void xsynchronize(int32_t status)
     /* sets (status != 0) or resets (status == 0) X-window event */
     /* synchronization.  If set, all X events are treated in the order */
     /* in which the commands are specified.  If reset, then X events */
     /* are treated in the most efficient order -- in this case, error */
     /* messages may appear a while after the LUX command that generates */
     /* them has (apparently) finished, but the overall execution */
     /* of X window commands is much faster. */
     /* if you want to find out which LUX command generates an X window */
     /* error, then set this synchronization. */
{
  if (setup_x() == LUX_ERROR)	/* make sure we're connected */
    return;
  XSynchronize(display, status? True: False);
  printf("X synchronization %sset.\n", status? "": "re");
}
 /*--------------------------------------------------------------------------*/
int32_t lux_show_visuals(int32_t narg, int32_t ps[])
{
  XVisualInfo	*vInfo, vTemplate;
  extern char	*visualNames[];
  int32_t	nVisual, i, mask, j;

  if (!connect_flag && setup_x() < 0)
    return LUX_ERROR;

  mask = VisualScreenMask;
  vTemplate.screen = screen_num;
  
  vInfo = XGetVisualInfo(display, mask, &vTemplate, &nVisual);
  if (!vInfo)
    nVisual = 0;
  
  if (!nVisual)
    return luxerror("No color representation is available", 0);

  printf("%1d visuals available on this screen\n", nVisual);
  
  printf("%6s %11s %2s %3s %6s %6s %6s %3s\n", "number", "class", "d",
	 "csz", "red", "green", "blue", "bpc");
  for (i = 0; i < nVisual; i++) {
    printf("%6d %11s %2d %3d %06x %06x %06x %3d\n", i + 1,
	   visualNames[vInfo[i].class], vInfo[i].depth,
	   vInfo[i].colormap_size, vInfo[i].red_mask,
	   vInfo[i].green_mask, vInfo[i].blue_mask,
	   vInfo[i].bits_per_rgb);
    if (vInfo[i].visual == visual)
      j = i;
  }
  printf("current visual: %d\n", j + 1);
  
#ifdef DEBUG
  printf("current width, height, cells, depth = %d %d %d %d\n",
	display_width, display_height, display_cells,
	depth);
#endif

  XFree(vInfo);
  return 1;
}
/*--------------------------------------------------------------------------*/
int32_t lux_xclose(int32_t narg, int32_t ps[])
/* close connection to window manager: close and destroy all windows, */
/* pixmaps, color maps, etc.  LS 30jul96 */
{
  int32_t	i;
  extern int32_t	menu_setup_done;
  void	disconnect_x();

  if (connect_flag) {
    for (i = 0; i < MAXPIXMAPS; i++)
      maps[i] = 0;
    for (i = 0; i < MAXWINDOWS; i++)
      win[i] = 0;
    menu_setup_done = 0;
    disconnect_x();
  }
  return 1;
}
/*--------------------------------------------------------------------------*/
int32_t ck_area(int32_t wid, int32_t *xpos, int32_t *ypos, int32_t *width, int32_t *height)
/* checks whether the specified DEV area lies within window <wid>.
   If OK, then returns 1.  If the area lies partially in the window,
   then the parameters are adjusted so the area is wholly inside the
   window, and 1 is returned.  If the area is wholly outside the window,
   or the window does not exist, then 0 is returned.  LS 16apr93 */
{
 int32_t	dwd, dht;

 if (wid >= 0) {
   if (!win[wid])
     return 0;			/* window does not exist */
   dwd = wd[wid];
   dht = ht[wid];
 } else {
   if (!maps[-wid])
     return 0;			/* pixmap does not exist */
   dwd = wdmap[-wid];
   dht = htmap[-wid];
 }
 if (*xpos + *width < 0 || *ypos + *height < 0
     || *xpos >= dwd || *ypos >= dht
     || *width <= 0 || *height <= 0)
   return 0;			/* wholly outside */
 if (*xpos + *width >= dwd)
   *width = dwd - *xpos;
 if (*ypos + *height >= dht)
   *height = dht - *ypos;
 if (*xpos < 0) {
   *width += *xpos;
   *xpos = 0;
 }
 if (*ypos < 0) {
   *height += *ypos;
   *ypos = 0;
 }
 return 1;
}
 /*--------------------------------------------------------------------------*/
int32_t lux_xtvlct(int32_t narg, int32_t ps[])
/* load color table, scaling to available range */
/* expect 3 arrays for RGB */
{
  int32_t	i, n, nmin = INT32_MAX, iq;
  float	*p[3];
  void	storeColorTable(float *, float *, float *, int32_t, int32_t);

  for (i = 0; i < 3; i++) {
    if (!symbolIsNumericalArray(ps[i]))
      return cerror(NEED_NUM_ARR, ps[i]);
    iq = lux_float(1, ps + i);	/* convert to FLOAT if necessary */
    n = array_size(iq);
    if (n < nmin)
      nmin = n;
    p[i] = (float *) array_data(iq);
  }
  if (setup_x() == LUX_ERROR)
    return LUX_ERROR;
  storeColorTable(p[0], p[1], p[2], nmin, !(internalMode & 1));
  return LUX_OK;
}
/*--------------------------------------------------------------------------*/
int32_t lux_xopen(int32_t narg, int32_t ps[])
/* sets or changes the display name for x setup and/or sets or changes the */
/* color map default.  LS 30jul96 */
{
  extern int32_t	select_visual;

  if (narg && ps[0]) {			/* set display name */
    if (symbol_class(ps[0]) != LUX_STRING)
      return cerror(NEED_STR, *ps);
    if (display_name)		/* may already be connected: disconnect old */
      lux_xclose(0, NULL);
    if (string_size(ps[0]) == 0) /* empty string -> default ($DISPLAY) */
      display_name = NULL;
    else			/* explicit name */
      display_name = strsave(string_value(ps[0]));
  }
  switch (internalMode & 3) {
    case 1:			/* /PRIVATE_COLORS */
      if (private_colormap != 1)	
	lux_xclose(0, NULL);	/* get rid of old */
      private_colormap = 1;
      break;
    case 2: case 0:		/* /DEFAULT_COLORMAP */
      if (private_colormap != 0)
	lux_xclose(0, NULL);	/* get rid of old */
      private_colormap = 0; 
      break;
    default:
      return luxerror("Illegal keyword combination", 0);
  }
  if (internalMode & 4) {	/* /SELECTVISUAL */
    if (connect_flag)
      lux_xclose(0, NULL);
    select_visual = 1;
  } else
    select_visual = 0;
  return setup_x();
}
/*--------------------------------------------------------------------------*/
int32_t lux_xexist(int32_t narg, int32_t ps[])/* return 1 if window exists */
 /* argument is port # */
{
  int32_t     wid;

  if (int_arg_stat(ps[0], &wid) != 1)
    return LUX_ERROR;
  if (wid < 0) {
    if (maps[-wid] != 0)
      return LUX_ONE;
  } else {                                /* window case */
    if (win[wid] != 0)
      return LUX_ONE;
  }
  /* no such, return an lux 0 */
  return LUX_ZERO;
}
/*--------------------------------------------------------------------------*/
int32_t lux_xport(int32_t narg, int32_t ps[])	/* open a window or pixmap */
 /* arguments are port #, width , height (default is 512x512) */
 /* position x, y, window title (<128 chars), icon title (<16 chars) */
 /* might be nice to have some commands to change things like the background
 color and pattern and cursor */
 /* added titles  LS 13apr93 */
/* check validity of window number  LS 14jul2000 */
{
  int32_t     wid, mapid, xpos = 0, ypos = 0, pflag, n;
  uint32_t	width, height;
  char	*wtitle = NULL, *ititle = NULL;
  int32_t	ck_window(int32_t), set_defw(int32_t), lux_xdelete(int32_t, int32_t []),
    lux_xcreat(int32_t, uint32_t, uint32_t, int32_t, int32_t, int32_t, char *,
	       char *);
  
  if (narg > 0)
    wid = int_arg(ps[0]);
  else
    wid = last_wid;
  if (ck_window(wid) != LUX_OK)
    return LUX_ERROR;

  width = height = 512;
  pflag = 0;
  if (narg > 1)
    width = int_arg(ps[1]);      /* get width if specified */
  if (narg > 2)
    height = int_arg(ps[2]);     /* get height if specified */
  if (narg > 3) {
    pflag = 1;
    xpos = int_arg(ps[3]);
  }
  if (narg > 4)
    ypos = int_arg(ps[4]);
  if (narg > 5)
    wtitle = string_arg(ps[5]);
  if (narg > 6)
    ititle = string_arg(ps[6]);
  if (ck_window(wid) != 1)
    return LUX_ERROR;
  if (wid < 0) {
			 		 /* pixmap case */
    /* check if pixmap already created, if so and no size, just set last_wid */
    mapid = - wid;
    if (maps[mapid] != 0) {
      if (narg < 2) {
	set_defw(wid);
	return 1;
      } else
	lux_xdelete(1, ps);
    }
  } else {                                /* window case */
    /* check if window already created, if so and no size, just set last_wid */
    if (win[wid] != 0) {
      switch (narg) {
	default:
	  if (height != ht[wid])
	    break;
	case 2:
	  if (width != wd[wid])
	    break;
	case 1: case 0:
	  set_defw(wid);
	  return 1;
      }
      lux_xdelete(1,ps);
    }
  }
		 /* if we get here, we now create the window or pixmap */
  n = lux_xcreat(wid, height, width, xpos, ypos, pflag, wtitle, ititle);
  if (n > 0)
    set_defw(wid);
  return n;
}
 /*--------------------------------------------------------------------------*/
int32_t ck_window(int32_t wid)
/* only checks if a window value is in allowed range */
{
  if (wid >= MAXWINDOWS || wid <= -MAXPIXMAPS)
    return luxerror("Illegal window or pixmap %1d; allowed range %1d -- %1d",
		 0, wid, -MAXPIXMAPS+1, MAXWINDOWS-1);
  else
    return LUX_OK;
}
 /*--------------------------------------------------------------------------*/
int32_t ck_events(void)         /* checks events for focus and size changes */
 /* updates the last_wid and any window sizes */
 /* will also change last_win to a window with a button or key pressed even
 if focus not changed; i.e., LUX's focus for a plot or tv can be changed
 without changing the X focus
 also note that the LUX "focus" will always be the last LUX window and ignores
 other windows including the xterm (or whatever) that LUX is running in */
{
   XEvent  report;
   int32_t     nev, i, j, iq;
   Window wq;
   int32_t	set_defw(int32_t);
   
   if (setup_x() < 0)
     return LUX_ERROR;
   XFlush(display);
   nev = XPending(display);
   /* printf("number of events = %d\n",nev); */
   if (nev <= 0 || preventEventFlush)
     return 1;
   for (i = 0; i < nev; i++) {
     XNextEvent(display, &report);
     switch (report.type) {
     case ButtonPress:
       wq = report.xbutton.window;
       break;
     case KeyPress:
       wq = report.xkey.window;
       break;
     case ConfigureNotify:
       wq = report.xconfigure.window;
       break;
     case ClientMessage:
       if ((Atom) report.xclient.data.l[0] == wm_delete)
	 wq = report.xclient.window;
       else
	 wq = 0;
       break;
       /*
     case Expose:
       wq = !report.xexpose.count? report.xexpose.window: 0;
       break;
       */
     default:
       /* we must put these events back into the queue or else */
       /* exposure and map events for widgets &c may not be serviced! */
       /* XPutBackEvent(display, &report); LS 1apr99 */
       wq = 0;
       break;
     }
     /* which LUX window ? */
     if (wq != 0) {
       iq = -1;
       for (j=0;j<MAXWINDOWS;j++) {
	 if (win[j] == wq) {
	   iq = j;
	   break;
	 }
       }
       if (iq == -1)		/* probably clicked in LUX menu    LS 1jun93 */
	 XPutBackEvent(display, &report);	/* save for later */
       else {				/* clicked in LUX window */
	 if (report.type == ConfigureNotify) {
	   wd[iq] = report.xconfigure.width;
	   ht[iq] = report.xconfigure.height;
	 } else if (report.type == ClientMessage) {
	   XDestroyWindow(display, report.xclient.window);
	   win[iq] = 0;
	   XFlush(display);
	 } else {
	   last_wid = iq;
	   set_defw(iq);
	 }
       }
     }
   }
   return  1;
}
/*-------------------------------------------------------------------------*/
int32_t set_defw(int32_t wid)
/* assumes window is defined, sets last_wid and plotting context */
{
  int32_t     mapid;

  last_wid = wid;
  if (wid < 0 ) {		/* pixmap case */
    mapid = - wid;
    xfac = wdmap[mapid];	/* width */
    yfac = htmap[mapid];	/* height */
  } else {			/* window case */
    xfac = wd[wid];		/* width */
    yfac = ht[wid];		/* height */
  }
  ixlow = 0;
  iylow = 0;
  ixhigh = xfac - 1;
  iyhigh = yfac - 1;
  return  1;
}
/*--------------------------------------------------------------------------*/
int32_t lux_xcreat(int32_t wid, uint32_t height, uint32_t width, int32_t xpos,
	       int32_t ypos, int32_t pflag, char *wtitle, char *ititle)
 /* might be nice to have some commands to change things like the background
 color and pattern and cursor */
{
 uint32_t     border_width = 1;
 uint32_t     valuemask;
 char    window_title[128], *window_name = window_title;
 char    icon_title[16], *icon_name = icon_title;
 char    snum[3];
 XTextProperty window, icon;
 XSizeHints      size_hints;
 XWMHints        wm_hints;
 XSetWindowAttributes    attributes;
 Cursor cursor;
 int32_t     mapid;

 /* start execution here */
 if (setup_x() < 0)
   return LUX_ERROR;
 
 if (ck_window(wid) != LUX_OK)
   return LUX_ERROR;

 if (wid >= 0) {                 /* window case */
#ifdef MOTIF
   /* if this window is associated with a motif drawing area, then the
    window will already be created (we check) and we just modify it a
    bit and setup the gc */
   if (drawingareas[wid] == 0) {
     /* not a drawing area, do all setups */
#endif
     wd[wid] = width;
     ht[wid] = height;
     *window_name = '\0';
     if (!wtitle || (wtitle && *wtitle != '-')) { /* need "lux xx" in title */
       sprintf( snum, "%d", wid);
       snum[2] = 0;
       strcpy(window_name, "lux ");
       strcat(window_name, snum);
     }
     if (wtitle) {		/* user specified window name */
       if (*wtitle == '-')	/* skip initial - */
	 wtitle++;
       else
	 strcat(window_name, ": ");
       strncat(window_name, wtitle, 127 - strlen(window_name)); /* add title */
     }
     *icon_name = '\0';
     if (!ititle || (ititle && *ititle != '-')) { /* need "lux xx" in title */
       sprintf( snum, "%d", wid);
       snum[2] = 0;
       strcpy(icon_name, "lux ");
       strcat(icon_name, snum);
     }
     if (ititle) {		/* user specified window name */
       if (*ititle == '-')	/* skip initial - */
	 ititle++;
       else
	 strcat(icon_name, ": ");
       strncat(icon_name, ititle, 15 - strlen(icon_name)); /* add title */
     }
     
     /* NOTE: need border pixel or else XCreateWindow generates a */
     /* BadMatch error when using a visual other than the default one */
     valuemask = CWBackPixel | CWColormap | CWBackingStore | CWBitGravity
       | CWBorderPixel;
     attributes.background_pixel =
       (setup & WHITE_BACKGR)?  white_pixel: black_pixel;
     attributes.border_pixel = black_pixel;
     attributes.backing_store = Always;
     attributes.colormap = colorMap;
     /* note that contents still disappear when window is iconized, may need
	to use our own pixmap to store and not count on server */
     attributes.bit_gravity = StaticGravity;
     
     /*
     win[wid] = XCreateWindow(display, RootWindow(display, screen_num), xpos,
			      ypos, width, height, border_width, depth,
			      InputOutput, visual, valuemask, &attributes);
     */

     win[wid] = XCreateWindow(display, RootWindow(display, screen_num), xpos,
			      ypos, width, height, border_width, depth,
			      InputOutput, visual, valuemask, &attributes);
     if (!win[wid])
       return luxerror("Could not create requested window", 0);
     icon_pixmap =
       XCreateBitmapFromData(display, win[wid], icon_bitmap_bits,
			     icon_bitmap_width, icon_bitmap_height);
     
     wm_hints.initial_state = NormalState;
     wm_hints.input = True;
     wm_hints.icon_pixmap = icon_pixmap;
     wm_hints.flags = StateHint | IconPixmapHint | InputHint;
     
     if (pflag)
       size_hints.flags = USPosition | USSize;
     else
       size_hints.flags = PPosition | PSize;
     
     XStringListToTextProperty(&icon_name, 1, &icon);
     XStringListToTextProperty(&window_name, 1, &window);
     
     XSetWMProperties(display, win[wid], &window, &icon, NULL, 0, &size_hints,
		      &wm_hints, NULL);
     
     XSetWMProtocols(display, win[wid], &wm_delete, 1);	

     XSelectInput(display, win[wid], KeyPressMask | ButtonPressMask 
		  | ButtonReleaseMask | PointerMotionMask | ExposureMask |
		  FocusChangeMask | EnterWindowMask | LeaveWindowMask);

#ifdef MOTIF
   } else {     
     /* a motif drawing area, if window not already created, return an error */
     
     /* assume we are using same display for X and motif */
     if ((win[wid] = XtWindowOfObject(lux_widget_id[drawingareas[wid]])) == 0) 
       return luxerror("cannot get window number from motif drawing area", 0);
     if (XGetWindowAttributes(display, win[wid], &wat) == False)
       return luxerror("cannot get window attributes for motif drawing area", 0);
     ht[wid] = wat.width;
     wd[wid] = wat.height;
     valuemask = CWBackPixel;	/* was |= but valuemask was not yet defined */
     attributes.background_pixel = white_pixel;
     valuemask |= CWBackingStore;
     /* attributes.backing_store = WhenMapped; / note that Always screws up things
					      not sure why yet */
     attributes.backing_store = Always;
     XChangeWindowAttributes(display, win[wid], valuemask, &attributes);
     XSelectInput(display, win[wid], ExposureMask | KeyPressMask
		  | ButtonPressMask | ExposureMask | FocusChangeMask);
   }
#endif
   /* this part is again the same for window and widget */
 {
   XGCValues	xgcv;
   unsigned long	valuemask;

   xgcv.foreground = (setup & WHITE_BACKGR)? black_pixel: white_pixel;
   xgcv.background = (setup & WHITE_BACKGR)? white_pixel: black_pixel;
   xgcv.plane_mask = AllPlanes;
   valuemask = GCPlaneMask | GCForeground | GCBackground;

   gc[wid] = XCreateGC(display, win[wid], valuemask, &xgcv);
 }
   
   cursor = XCreateFontCursor(display, XC_crosshair);
   XDefineCursor(display, win[wid], cursor);
   XFreeCursor(display, cursor);
   XMapWindow(display, win[wid]);
   XFlush(display);
 } else {                                /* pixmap case */
   mapid = -wid;
   if (maps[mapid] == 0)
     maps[mapid] = XCreatePixmap(display, RootWindow(display,screen_num),
				 width, height, 8);
   wdmap[mapid] = width;
   htmap[mapid] = height;
   gcmap[mapid] = XCreateGC(display, maps[mapid], 0, NULL);
   XSetForeground(display, gcmap[mapid],
		  setup & WHITE_BACKGR? black_pixel: white_pixel);
   XSetBackground(display, gcmap[mapid],
		  setup & WHITE_BACKGR? white_pixel: black_pixel);
 }
 set_defw(wid);
 return 1;
}
/*--------------------------------------------------------------------------*/
int32_t lux_xsetinputs(int32_t narg, int32_t ps[])		/* set the input mask */
{
  int32_t	wid;

  if (ck_events() != 1)
    return LUX_ERROR;
  if (int_arg_stat(ps[0], &wid) != 1)
    return LUX_ERROR;
  if (ck_window(wid) != 1)
    return LUX_ERROR;
  XSelectInput(display, win[wid], KeyPressMask | ButtonPressMask);
  return 1;
}
 /*-------------------------------------------------------------------------*/
int32_t lux_xcursor(int32_t narg, int32_t ps[]) /* set the cursor */
{
  int32_t    wid, iq;
  Cursor cursor;
  XColor	*cfore, *cback;
  char	*cfore_default = {"red"};
  char	*cback_default = {"black"};
  char	*pc;

  if (ck_events() != 1
      || int_arg_stat(ps[0], &wid) != 1
      || int_arg_stat(ps[1], &iq) != 1
      || ck_window(wid) != 1)
    return LUX_ERROR;
  cursor = XCreateFontCursor(display, iq);
  if (narg > 2) {
    /* get foreground color */
    if (symbol_class(ps[2]) != LUX_STRING)
      return cerror(NEED_STR, ps[2]);
    pc = string_value(ps[2]);
  } else
    pc = cfore_default;
  if (anaAllocNamedColor(pc, &cfore) != 1)
    return luxerror("error in background color", 0);
  if (narg > 3) {
    /* get background color */
    if (symbol_class(ps[3]) != LUX_STRING)
      return cerror(NEED_STR, ps[3]);
    pc = string_value(ps[3]);
  } else
    pc = cback_default; 
  if (anaAllocNamedColor(pc, &cback) != 1)
    return luxerror("error in background color", 0);
  XRecolorCursor(display, cursor, cfore, cback);
  XDefineCursor(display, win[wid], cursor);
  XFreeCursor(display, cursor);
  XFlush(display);
  return LUX_OK;
}
/*--------------------------------------------------------------------------*/
int32_t lux_xsetbackground(int32_t narg, int32_t ps[])/* set the background color */
 /* setbackground [, win#] , color */
/* made first argument optional.  Also check that window/pixmap */
/* really exists before attempting to change its background color */
/* LS 13jul2000 */
/* allow three-element array for <color> to specify RGB values. LS 14jul2000 */
{
  int32_t    wid, iq;
  char	*pc;
  float	*value;
  XColor	*color;
  
  ck_events();
  if (narg == 1)
    wid = last_wid;
  else
    wid = int_arg(*ps++);
  if (ck_window(wid) != 1)
    return LUX_ERROR;
  if ((wid >= 0 && !win[wid]) || (wid < 0 && !maps[-wid]))
    lux_xcreat(wid, 512, 512, 0, 0, 0, NULL, NULL); 

  /* try to figure out the color */
  switch (symbol_class(*ps)) {
    case LUX_STRING:
      pc = string_value(*ps);
      if (anaAllocNamedColor(pc, &color) == 0)
	goto setbg_1;
      break;
    case LUX_ARRAY:
      if (symbolIsRealArray(*ps) && array_size(*ps) == 3) {
	iq = lux_float(1, ps);	/* ensure FLOAT */
	value = array_data(iq);
	sprintf(pc = curScrat, "rgbi:%g/%g/%g", value[0], value[1], value[2]);
	if (anaAllocNamedColor(pc, &color) == 0)
	  goto setbg_1;
	if (iq != *ps)
	  zap(iq);		/* not needed anymore */
      } else return luxerror("Need string or 3-element real array for color specification", 0);
      break;
    default:
      return cerror(ILL_CLASS, *ps);
  }

  if (wid < 0) {		/* pixmap case */
    XSetBackground(display, gcmap[-wid], color->pixel);
    XSetWindowBackground(display, maps[-wid], color->pixel);
  } else {			/* window case */
    XSetBackground(display, gc[wid], color->pixel);
    XSetWindowBackground(display,win[wid], color->pixel);
  }
  XFlush(display);
  return LUX_OK;

  setbg_1:
  return luxerror("error in foreground color", 0);
}
/*--------------------------------------------------------------------------*/
int32_t lux_xsetforeground(int32_t narg, int32_t ps[])/* set the foreground color */
 /* setforeground [, win#] , color */
/* made first argument optional.  Also check that window/pixmap */
/* really exists before attempting to change its foreground color */
/* LS 13jul2000 */
/* allow three-element array for <color> to specify RGB values. LS 14jul2000 */
{
  int32_t    wid, iq;
  char	*pc;
  float	*value;
  XColor	*color;
  
  ck_events();
  if (narg == 1)
    wid = last_wid;
  else
    wid = int_arg(*ps++);
  if (ck_window(wid) != 1)
    return LUX_ERROR;
  if ((wid >= 0 && !win[wid]) || (wid < 0 && !maps[-wid]))
    lux_xcreat(wid, 512, 512, 0, 0, 0, NULL, NULL); 

  /* try to figure out the color */
  switch (symbol_class(*ps)) {
    case LUX_STRING:
      pc = string_value(*ps);
      if (anaAllocNamedColor(pc, &color) == 0)
	goto setfg_1;
      break;
    case LUX_ARRAY:
      if (symbolIsRealArray(*ps) && array_size(*ps) == 3) {
	iq = lux_float(1, ps);	/* ensure FLOAT */
	value = array_data(iq);
	sprintf(pc = curScrat, "rgbi:%g/%g/%g", value[0], value[1], value[2]);
	if (anaAllocNamedColor(pc, &color) == 0)
	  goto setfg_1;
	if (iq != *ps)
	  zap(iq);		/* not needed anymore */
      } else return luxerror("Need string or 3-element real array for color specification", 0);
      break;
    default:
      return cerror(ILL_CLASS, *ps);
  }

  if (wid < 0) {		/* pixmap case */
    XSetForeground(display, gcmap[-wid], color->pixel);
  } else {			/* window case */
    XSetForeground(display, gc[wid], color->pixel);
  }
  XFlush(display);
  return LUX_OK;

  setfg_1:
  return luxerror("error in foreground color", 0);
}
/*--------------------------------------------------------------------------*/
int32_t lux_xdelete(int32_t narg, int32_t ps[]) /* delete a window or a pixmap */
{
  int32_t    wid;
  
  ck_events();
  wid = int_arg( ps[0] );
  if (ck_window(wid) != 1)
    return -1;
  if (wid < 0 )  {		/* pixmap case */
    XFreePixmap(display, maps[-wid]);
    maps[-wid] = 0;
  } else {			/* window case */
    XDestroyWindow(display, win[wid]);
    win[wid] = 0;
  }
  XFlush(display);		/* or it won't vanish for a long time */
  return 1;
 }
 /*--------------------------------------------------------------------------*/
int32_t lux_xerase(int32_t narg, int32_t ps[])
     /* erase a window */
     /* pixmap support added.  LS 8oct97 */
{
 int32_t    wid, xx, yx, wx, hx, old, cs;
 float	x, y, w, h;
 XGCValues	values;

 ck_events();

 x = y = h = w = 0.0;		/* defaults */
 wid = last_wid;
 switch (narg) { 
   case 0:			/* OK */
     break;
   case 1:
     wid = int_arg(ps[0]);	/* window number */
     break;
   case 4:
     x = float_arg(ps[0]);
     y = float_arg(ps[1]);
     w = float_arg(ps[2]);
     h = float_arg(ps[3]);
     break;
   case 5:
     wid = int_arg(ps[0]);	/* window number */
     x = float_arg(ps[1]);
     y = float_arg(ps[2]);
     w = float_arg(ps[3]);
     h = float_arg(ps[4]);
   default:
     return cerror(ILL_ARG_LIST, 0);
 }
 if (ck_window(wid) != 1)
   return LUX_ERROR;

 if (wid < 0) {			/* pixmap */
   if (!maps[-wid]) {
     last_wid = wid;
     if (lux_xport(0, NULL) < 0)
       return LUX_ERROR;
   }
 } else {			/* window */
   if (!win[wid]) {
     last_wid = wid;
     if (lux_xport(0, NULL) < 0)
       return LUX_ERROR;	/* window didn't exists */
   }
 }

 if (!w || !h) {		/* not yet specified -- or illegal */
   /* we erase the whole image */
   xx = 0;
   yx = 0;
   if (wid < 0) {
     wx = wdmap[-wid];		/* take window dimension */
     hx = htmap[-wid];		/* take window dimension */
   } else {
     wx = wd[wid];
     hx = ht[wid];
   }
 } else {
   w = x + w;
   h = y + h;
   cs = (internalMode & 7);
   if ((setup & 4) && cs == LUX_DEV)
     cs = LUX_X11;
   coordTrf(&x, &y, cs, LUX_X11);
   coordTrf(&w, &h, cs, LUX_X11);
   xx = (int32_t) x;
   yx = (int32_t) y;
   wx = (int32_t) fabs(w - x);
   hx = (int32_t) fabs(h - y);
 }

 if (!ck_area(wid, &xx, &yx, &wx, &hx)) /* area wholly outside window */
   return 1;			/* do nothing */

 if (wid >= 0)
   XClearArea(display, win[wid], xx, yx, wx, hx, False);
 else {
   XGetGCValues(display, gcmap[-wid], GCFunction, &values);
   old = values.function;
   values.function = GXclear;
   XChangeGC(display, gcmap[-wid], GCFunction, &values);
   XCopyArea(display, maps[-wid], maps[-wid], gcmap[-wid], xx, yx, wx, hx,
	     xx, yx);
   values.function = old;
   XChangeGC(display, gcmap[-wid], GCFunction, &values);
 }
 XFlush(display);		/* or it won't happen for a long time */
 return 1;
}
 /*------------------------------------------------------------------------*/
int32_t lux_xsetaction(int32_t narg, int32_t ps[])
/* This routine associates a certain copy action with a window.
   Codes 1 and 2 select sprite action.  A sprite disappears
   without a trace if drawn for a second time.  LS 14apr93 */
/* syntax:  xsetaction [,code,window]
    code defaults to 1, window to last_win */
{
  int32_t	wid = last_wid, code = 1;
  static int32_t	function_code[] = {
	GXcopy, GXxor, GXequiv, GXclear, GXset, GXnoop, GXinvert, GXand,
	GXandReverse, GXandInverted, GXor, GXorReverse, GXorInverted,
	GXnand, GXnor, GXcopyInverted };

  if (narg) code = int_arg(ps[0]);			/* action code */
  if (code < 0 || code > 15)
  { puts("Invalid action.  Valid range: 0 thru 15.");
    return -1; }
  if (narg > 1) wid = int_arg(ps[1]);			/* window number */
  if (ck_window(wid) != 1) return -1;
  code = function_code[code];
  /* does window exist? If not create a default size */
  if ( win[wid] == 0 ) lux_xcreat(wid, 512, 512, 0, 0, 0, NULL, NULL);
  if (wid >= 0)
    XSetFunction(display, gc[wid], code);
  else
    XSetFunction(display, gcmap[-wid], code);
  return 1;
}
 /*------------------------------------------------------------------------*/
int32_t reverseYImage(int32_t iq)
/* returns a copy of 2D image <iq> reversed in the y direction */
{
  int32_t	ps[2];
  int32_t	lux_reverse(int32_t, int32_t *);

  if (symbol_class(iq) != LUX_ARRAY
      || array_num_dims(iq) != 2)
    return cerror(NEED_2D_ARR, iq);
  ps[0] = iq;
  ps[1] = LUX_ONE;
  return lux_reverse(2, ps);
}
/*------------------------------------------------------------------------*/
int32_t lux_xtv_general(int32_t narg, int32_t ps[], int32_t mode)
{
  pointer	data;
  int32_t	type, nx, ny, wid;
  float	x, y;
  int32_t	tvraw(pointer data, int32_t type, int32_t nx, int32_t ny, float x1, float x2,
	  float y1, float y2, float sxf, float syf, int32_t wid, float *mag,
	  int32_t mode, double clo, double chi, uint8_t *bitmap1, uint8_t *bitmap2);

  if (internalMode & TV_24) {
    mode |= TV_24;
    if (!symbolIsNumericalArray(ps[0]) /* <image> */
	|| isComplexType(array_type(ps[0]))
	|| array_num_dims(ps[0]) != 3
	|| array_dims(ps[0])[2] != 3)
      return luxerror("Need 3D array with 3rd dimension = 3", ps[0]);
  } else {
    mode &= ~TV_24;
    if (!symbolIsNumericalArray(ps[0]) /* <image> */
	|| isComplexType(array_type(ps[0]))
	|| array_num_dims(ps[0]) != 2)
      return cerror(NEED_2D_ARR, ps[0]);
  }

  nx = array_dims(ps[0])[0];
  ny = array_dims(ps[0])[1];
  type = array_type(ps[0]);
  data.v = array_data(ps[0]);

  if (narg > 1 && ps[1]) {	/* <x> */
    if (!symbolIsScalar(ps[1]))
      return cerror(NEED_SCAL, ps[1]);
    x = float_arg(ps[1]);
  } else
    x = 0;

  if (narg > 2 && ps[2]) {	/* <y> */
    if (!symbolIsScalar(ps[2]))
      return cerror(NEED_SCAL, ps[2]);
    y = float_arg(ps[2]);
  } else
    y = 0;

  if (narg > 3 && ps[3]) {	/* <window> */
    if (!symbolIsScalar(ps[3]))
      return cerror(NEED_SCAL, ps[3]);
    wid = int_arg(ps[3]);
  } else
    wid = last_wid;

  if (narg > 4 && ps[4]) {	/* <scale> */
    if (!symbolIsScalar(ps[4]))
      return cerror(NEED_SCAL, ps[4]);
    tvscale = float_arg(ps[4]);
    if (tvscale == 1)
      mode |= TV_SCALE;
  } else
    tvscale = 0.0;

  return tvraw(data, type, nx, ny, 0, nx - 1, 0, ny - 1, x, y, wid, &tvscale,
	       mode, 0.0, 0.0, NULL, NULL);
}
/*------------------------------------------------------------------------*/
int32_t lux_xtvraw(int32_t narg, int32_t ps[])
/* scales non-LUX_INT8 arrays; displays on screen */
/* NOTE: in older versions of LUX the coordinates of the image were
 counted from the upper left-hand corner of the screen, and the
 position specified for the image was that of the upper left-hand
 corner of the image.  We have now changed that so that the
 coordinates are counted from the lower left-hand corner, and the
 position specified for the image is the position of its lower
 left-hand corner, too, so that TV coordinates and device-dependent
 XYMOV coordinates are now the same.  The old behavior is still
 available: execute SET,/ULIMCOORDS.  LS mar98 */
{
  return lux_xtv_general(narg, ps, TV_RAW);
}
/*------------------------------------------------------------------------*/
int32_t tvraw(pointer data, int32_t type, int32_t nx, int32_t ny, float x1, float x2,
	  float y1, float y2, float sxf, float syf, int32_t wid, float *mag,
	  int32_t mode, double clo, double chi, uint8_t *bitmap1, uint8_t *bitmap2)
/* display data in a window. */
/* data: pointer to the start of the data */
/* type: data type */
/* nx, ny: data dimensions */
/* x1, x2, y1, y2: coordinates of the desired image part (px) */
/* sx, sy: target coordinates (coordinate system determined by <mode> as
   in internalMode of TV and TV3). */
/* wid: target window (assumed > 0) */
/* mag: desired magnification factor */
/* mode: if 0 -> (sx,sy) indicate position of lower left-hand corner;
         if 1 -> (sx,sy) indicate position of center
         if &2 -> 24-bit color specification; the data is assumed to
          have dimensions (nx,ny,3) with (*,*,0) the red, (*,*,1)
	  the green, and (*,*,2) the blue value for each pixel. */
/* it is assumed here that the arguments are consistent! */
{
  pointer	image, image0;
  int32_t	hq, wq, nxx, nyy, ix, iy, xsrc, ysrc, indx, i, toscreen, maxdim,
    iq, sx, sy, bpp, s;
  scalar	min, max, value, factor, offset;
  float	fx, fy, fx2, fy2, magx, magy, nxxf, nyyf;
  extern float	postXBot, postXTop, postYBot, postYTop,
    zoom_mag, zoom_xc, zoom_yc, zoom_clo, zoom_chi, wxb, wxt, wyb, wyt;
  extern int32_t	lunplt, bits_per_pixel;
  extern unsigned long	red_mask, green_mask, blue_mask;
  int32_t	lux_threecolors(int32_t, int32_t []), checkCoordSys(int32_t, int32_t),
    coordTrf(float *, float *, int32_t, int32_t),
    postgray(char *, int32_t, int32_t, float, float, float, float, int32_t),
    postcolor(char *, int32_t, int32_t, float, float, float, float, int32_t);
  
  toscreen = (internalMode & (TV_SCREEN | TV_POSTSCRIPT | TV_PDEV));
  switch (toscreen) {
    case TV_SCREEN: case 0:
      toscreen = 1;
      break;
    case TV_POSTSCRIPT:
      toscreen = 0;
      break;
    case TV_PDEV:
      toscreen = lunplt? 0: 1;
      break;
    default:
      return luxerror("Cannot specify /SCREEN and /POSTSCRIPT at the same time",
		   0);
  }

  if (connect_flag) {
    if (wid >= 0 && win[wid]) {	/* existent window */
      hq = ht[wid];
      wq = wd[wid];
    } else if (wid < 0 && maps[-wid]) {	/* existent pixmap */
      hq = htmap[-wid];
      wq = wdmap[-wid];
    } else {			/* defaults */
      hq = yfac = ny;
      wq = xfac = nx;
    }
  } else {			/* defaults */
    hq = yfac = ny;
    wq = xfac = nx;
  }

  bpp = bits_per_pixel? bits_per_pixel: 8; /* bits_per_pixel is zero if we */
					   /* haven't connected to the X */
					   /* server (e.g., tv,x,/post) */

  if (!*mag)
    *mag = 1;

  if (internalMode & TV_ZOOM) {	/* use ZOOM parameters instead */
    sxf = wq*0.5;		/* zoom target coordinates: image center */
    syf = hq*0.5;
    sx = (int32_t) sxf;
    sy = (int32_t) syf;
    *mag = zoom_mag;		/* zoom scale */
    internalMode |= TV_CENTER;	/* zoom coordinates indicate image center */
    internalMode = (internalMode & ~63) | LUX_DEV;
				/* zoom coordinates are in DEV */
    clo = zoom_clo;		/* zoom contrast */
    chi = zoom_chi;
    x1 = zoom_xc - 0.5*wq/(*mag); /* zoom image part */
    x2 = zoom_xc + 0.5*wq/(*mag);
    y1 = zoom_yc - 0.5*hq/(*mag);
    y2 = zoom_yc + 0.5*hq/(*mag);
    if (x2 < 0)			/* restrict desired part to existent image */
      x2 = 1;
    else if (x2 >= nx)
      x2 = nx - 1;
    if (x1 < 0)
      x1 = 0;
    else if (x1 >= nx - 1)
      x1 = nx - 2;
    if (y2 < 0)
      y2 = 1;
    else if (y2 >= ny)
      y2 = ny - 1;
    if (y1 < 0)
      y1 = 0;
    else if (y1 >= ny - 1)
      y1 = ny - 2;
    if (y2 < 0)
      y2 = 1;
    else if (y2 >= ny)
      y2 = ny - 1;
    magx = magy = *mag;
  } else if (internalMode & TV_PLOTWINDOW) {
    magx = wq*(wxt - wxb)/nx;
    magy = hq*(wyt - wyb)/ny;
    if (magx <= 0 || magy <= 0) {
      puts("WARNING - plot window has zero dimensions.  Use unit scale");
      magx = magy = 1.0;
    }
  } else if ((mode & TV_SCALE) || (setup & 32)) {
				/* set magnification to standard value */
    *mag = MIN((float) hq/ny, (float) wq/nx);
    if (*mag >= 1)
      *mag = (int32_t) *mag;
    else
      *mag = 1.0/((int32_t) 1.0/ *mag);
    magx = magy = *mag;
  } else if ((x2 - x1 + 1)**mag > 2*wq || (y2 - y1 + 1)**mag > 2*hq) {
    puts(
      "WARNING - scaled image is significantly bigger than target window.");
    puts(" -- reducing scale factor to accommodate.");
    *mag = MIN((float) hq/ny, (float) wq/nx);
    if (*mag >= 1)
      *mag = (int32_t) *mag;
    else
      *mag = 1.0/((int32_t) 1.0/ *mag);
    magx = magy = *mag;
  } else
    magx = magy = *mag;

  nxxf = (x2 + 1 - x1)*magx; /* scaled dimensions */
  nyyf = (y2 + 1 - y1)*magy;
  nxx = (int32_t) nxxf;
  nyy = (int32_t) nyyf;

  if (internalMode & TV_PLOTWINDOW) {
    sxf = wxb * wq;
    syf = (1 - wyt) * hq;
    sx = (int32_t) sxf;
    sy = (int32_t) syf;
  } else {
    i = (internalMode & 7);
    if (toscreen		/* displaying on screen */
	&& (setup & 4)		/* X11 coordinates */
	&& i == LUX_DEV)	/* have DEV coordinates */
      i = LUX_X11;		/* interpret as X11 */
    coordTrf(&sxf, &syf, i, LUX_DEV); /* transform to DEV */
    sx = (int32_t) sxf;
    sy = (int32_t) syf;
    /* calculate coordinates of image's upper right-hand corner in X11
       coordinates */
    switch (internalMode & TV_CENTER) {	/* /CENTER */
      case 0:			/* (sx,sy) indicate lower left-hand corner */
	if (setup & 4)
	  sy = hq - 1 - sy;
	else
	  sy = hq - nyy - sy;
	break;
      case TV_CENTER:		/* (sx,sy) indicate image center position */
	sx = sx - nxx*0.5;
	if (setup & 4)
	  sy = sy - nyy*0.5;
	else
	  sy = hq - sy - nyy*0.5;
	break;
    }
  }

  if (toscreen) {
    /* does window or pixmap exist?  If not, create to fit image */
    if ((wid >= 0 && win[wid] == 0)
	|| (wid < 0 && maps[-wid] == 0)) {
      lux_xcreat(wid, hq, wq, 0, 0, 0, NULL, NULL);
    }
    if (!ck_area(wid, &sx, &sy, &nxx, &nyy)) {
      puts("The image falls completely outside the window");
      return 1;
    }
  }

  if (mode & TV_24) {
    if (visualPrimariesAreLinked(visual->class) && toscreen)
      return luxerror("The %s visual that you are using is incapable\nof displaying 3-component images.\n", 0, visualNames[visual->class]);
    s = nx*ny;
  }

  /* we interpret <type> as follows:
     > 0  -> the data type of the image; we must scale the image contrast
     0,-1 -> image contains BYTE data already scaled to pixel indices
     -2   -> image contains BYTE data already scaled to raw pixel values */
  
  if (!(mode & (TV_MAP | TV_RAW))) {
    /* must scale the image contrast */
    if (clo == chi) {		/* no contrast -> use full contrast in */
				/* current image */
      /* determine min/max */
      if (mode & TV_24) {		/* three bytes per pixel */
	switch (type) {
	  case LUX_INT8:
	    min.b = UINT8_MAX;
	    max.b = 0;
	    for (iy = y1; iy <= y2; iy++)
	      for (ix = x1; ix <= x2; ix++) {
		value.b = data.b[ix + iy*nx];
		if (value.b < min.b)
		  min.b = value.b;
		if (value.b > max.b)
		  max.b = value.b;
		value.b = data.b[ix + iy*nx + s];
		if (value.b < min.b)
		  min.b = value.b;
		if (value.b > max.b)
		  max.b = value.b;
		value.b = data.b[ix + iy*nx + 2*s];
		if (value.b < min.b)
		  min.b = value.b;
		if (value.b > max.b)
		  max.b = value.b;
	      }
	    if (max.b == min.b)
	      factor.f = 1;
	    else if (threeColors)
	      factor.f = (256/3)/((float) max.b - min.b);
	    else
	      factor.f = 255/((float) max.b - min.b);
	    offset.f = (float) min.b;
	    break;
	  case LUX_INT16:
	    min.w = INT16_MAX;
	    max.w = -INT16_MAX;
	    for (iy = y1; iy <= y2; iy++)
	      for (ix = x1; ix <= x2; ix++) {
		value.w = data.w[ix + iy*nx];
		if (value.w < min.w)
		  min.w = value.w;
		if (value.w > max.w)
		  max.w = value.w;
		value.w = data.w[ix + iy*nx + s];
		if (value.w < min.w)
		  min.w = value.w;
		if (value.w > max.w)
		  max.w = value.w;
		value.w = data.w[ix + iy*nx + 2*s];
		if (value.w < min.w)
		  min.w = value.w;
		if (value.w > max.w)
		  max.w = value.w;
	      }
	    if (max.w == min.w)
	      factor.f = 1;
	    else if (threeColors)
	      factor.f = (256/3)/((float) max.w - min.w);
	    else
	      factor.f = 255/((float) max.w - min.w);
	    offset.f = (float) min.w;
	    break;
	  case LUX_INT32:
	    min.l = INT32_MAX;
	    max.l = -INT32_MAX;
	    for (iy = y1; iy <= y2; iy++)
	      for (ix = x1; ix <= x2; ix++) {
		value.l = data.l[ix + iy*nx];
		if (value.l < min.l)
		  min.l = value.l;
		if (value.l > max.l)
		  max.l = value.l;
		value.l = data.l[ix + iy*nx + s];
		if (value.l < min.l)
		  min.l = value.l;
		if (value.l > max.l)
		  max.l = value.l;
		value.l = data.l[ix + iy*nx + 2*s];
		if (value.l < min.l)
		  min.l = value.l;
		if (value.l > max.l)
		  max.l = value.l;
	      }
	    if (max.l == min.l)
	      factor.f = 1;
	    else if (threeColors)
	      factor.f = (256/3)/((float) max.l - min.l);
	    else
	      factor.f = 255/((float) max.l - min.l);
	    offset.f = (float) min.l;
	    break;
	  case LUX_INT64:
	    min.q = INT64_MAX;
	    max.q = -INT64_MAX;
	    for (iy = y1; iy <= y2; iy++)
	      for (ix = x1; ix <= x2; ix++) {
		value.q = data.q[ix + iy*nx];
		if (value.q < min.q)
		  min.q = value.q;
		if (value.q > max.q)
		  max.q = value.q;
		value.q = data.q[ix + iy*nx + s];
		if (value.q < min.q)
		  min.q = value.q;
		if (value.q > max.q)
		  max.q = value.q;
		value.q = data.q[ix + iy*nx + 2*s];
		if (value.q < min.q)
		  min.q = value.q;
		if (value.q > max.q)
		  max.q = value.q;
	      }
	    if (max.q == min.q)
	      factor.f = 1;
	    else if (threeColors)
	      factor.f = (256/3)/((float) max.q - min.q);
	    else
	      factor.f = 255/((float) max.q - min.q);
	    offset.f = (float) min.q;
	    break;
	  case LUX_FLOAT:
	    min.f = FLT_MAX;
	    max.f = -FLT_MAX;
	    for (iy = y1; iy <= y2; iy++)
	      for (ix = x1; ix <= x2; ix++) {
		value.f = data.f[ix + iy*nx];
		if (value.f < min.f)
		  min.f = value.f;
		if (value.f > max.f)
		  max.f = value.f;
		value.f = data.f[ix + iy*nx + s];
		if (value.f < min.f)
		  min.f = value.f;
		if (value.f > max.f)
		  max.f = value.f;
		value.f = data.f[ix + iy*nx + 2*s];
		if (value.f < min.f)
		  min.f = value.f;
		if (value.f > max.f)
		  max.f = value.f;
	      }
	    if (max.f == min.f)
	      factor.f = 1;
	    else if (threeColors)
	      factor.f = (256/3)/((float) max.f - min.f);
	    else
	      factor.f = 255/((float) max.f - min.f);
	    offset.f = (float) min.f;
	    break;
	  case LUX_DOUBLE:
	    min.d = DBL_MAX;
	    max.d = -DBL_MAX;
	    for (iy = y1; iy <= y2; iy++)
	      for (ix = x1; ix <= x2; ix++) {
		value.d = data.d[ix + iy*nx];
		if (value.d < min.d)
		  min.d = value.d;
		if (value.d > max.d)
		  max.d = value.d;
		value.d = data.d[ix + iy*nx + s];
		if (value.d < min.d)
		  min.d = value.d;
		if (value.d > max.d)
		  max.d = value.d;
		value.d = data.d[ix + iy*nx + 2*s];
		if (value.d < min.d)
		  min.d = value.d;
		if (value.d > max.d)
		  max.d = value.d;
	      }
	    if (max.d == min.d)
	      factor.f = 1;
	    else if (threeColors)
	      factor.f = (256/3)/((float) max.d - min.d);
	    else
	      factor.f = 255/((float) max.d - min.d);
	    offset.f = (float) min.d;
	    break;
	}
      } else {			/* one uint8_t per pixel */
	switch (type) {
	  case LUX_INT8:
	    min.b = UINT8_MAX;
	    max.b = 0;
	    for (iy = y1; iy <= y2; iy++)
	      for (ix = x1; ix <= x2; ix++) {
		value.b = data.b[ix + iy*nx];
		if (value.b < min.b)
		  min.b = value.b;
		if (value.b > max.b)
		  max.b = value.b;
	      }
	    if (max.b == min.b)
	      factor.f = 1;
	    else if (threeColors)
	      factor.f = (256/3)/((float) max.b - min.b);
	    else
	      factor.f = ((float) scalemax - scalemin)/((float) max.b - min.b);
	    offset.f = (float) min.b;
	    break;
	  case LUX_INT16:
	    min.w = INT16_MAX;
	    max.w = -INT16_MAX;
	    for (iy = y1; iy <= y2; iy++)
	      for (ix = x1; ix <= x2; ix++) {
		value.w = data.w[ix + iy*nx];
		if (value.w < min.w)
		  min.w = value.w;
		if (value.w > max.w)
		  max.w = value.w;
	      }
	    if (max.w == min.w)
	      factor.f = 1;
	    else if (threeColors)
	      factor.f = (256/3)/((float) max.w - min.w);
	    else
	      factor.f = ((float) scalemax - scalemin)/((float) max.w - min.w);
	    offset.f = (float) min.w;
	    break;
	  case LUX_INT32:
	    min.l = INT32_MAX;
	    max.l = -INT32_MAX;
	    for (iy = y1; iy <= y2; iy++)
	      for (ix = x1; ix <= x2; ix++) {
		value.l = data.l[ix + iy*nx];
		if (value.l < min.l)
		  min.l = value.l;
		if (value.l > max.l)
		  max.l = value.l;
	      }
	    if (max.l == min.l)
	      factor.f = 1;
	    else if (threeColors)
	      factor.f = (256/3)/((float) max.l - min.l);
	    else
	      factor.f = ((float) scalemax - scalemin)/((float) max.l - min.l);
	    offset.f = (float) min.l;
	    break;
	  case LUX_INT64:
	    min.q = INT64_MAX;
	    max.q = -INT64_MAX;
	    for (iy = y1; iy <= y2; iy++)
	      for (ix = x1; ix <= x2; ix++) {
		value.q = data.q[ix + iy*nx];
		if (value.q < min.q)
		  min.q = value.q;
		if (value.q > max.q)
		  max.q = value.q;
	      }
	    if (max.q == min.q)
	      factor.f = 1;
	    else if (threeColors)
	      factor.f = (256/3)/((float) max.q - min.q);
	    else
	      factor.f = ((float) scalemax - scalemin)/((float) max.q - min.q);
	    offset.f = (float) min.q;
	    break;
	  case LUX_FLOAT:
	    min.f = FLT_MAX;
	    max.f = -FLT_MAX;
	    for (iy = y1; iy <= y2; iy++)
	      for (ix = x1; ix <= x2; ix++) {
		value.f = data.f[ix + iy*nx];
		if (value.f < min.f)
		  min.f = value.f;
		if (value.f > max.f)
		  max.f = value.f;
	      }
	    if (max.f == min.f)
	      factor.f = 1;
	    else if (threeColors)
	      factor.f = (256/3)/(max.f - min.f);
	    else
	      factor.f = (scalemax - scalemin)/(max.f - min.f);
	    offset.f = min.f;
	    break;
	  case LUX_DOUBLE:
	    min.d = DBL_MAX;
	    max.d = -DBL_MAX;
	    for (iy = y1; iy <= y2; iy++)
	      for (ix = x1; ix <= x2; ix++) {
		value.d = data.d[ix + iy*nx];
		if (value.d < min.d)
		  min.d = value.d;
		if (value.d > max.d)
		  max.d = value.d;
	      }
	    if (max.d == min.d)
	      factor.d = 1;
	    else if (threeColors)
	      factor.d = (256/3)/(max.d - min.d);
	    else
	      factor.d = (scalemax - scalemin)/(max.d - min.d);
	    offset.d = min.d;
	    break;
	}
      }
    } else { 			/* have explicit contrast range */
      if (type == LUX_DOUBLE) {
	if (threeColors)
	  factor.d = (256/3)/(chi - clo);
	else
	  factor.d = (scalemax - scalemin)/(chi - clo);
	offset.d = clo;
      } else {
	if (threeColors)
	  factor.f = (256/3)/(chi - clo);
	else
	  factor.f = (scalemax - scalemin)/(chi - clo);
	offset.f = (float) clo;
      }
    }
  }

  /* set the globals to corners of image */
  tvix = sx;
  tvixb = sx + nxx;
  tviy = hq - sy - nyy;
  tviyb = hq - sy;
  
  /* update bounding box */
  if (postXBot > tvix/(float) wq)
    postXBot = tvix/(float) wq;
  if (postXTop < tvixb/(float) wq)
    postXTop = tvixb/(float) wq;
  if (postYBot > tviy/(float) hq)
    postYBot = tviy/(float) hq;
  if (postYTop < tviyb/(float) hq)
    postYTop = tviyb/(float) hq;
  /* if we're sending the image to a postscript file, then we can scale
     it to arbitrary size by specifying different desired corner positions
     on the canvas; we do not actually need to change the number of
     elements in the image.  We already know what we need to determine
     the final dimensions and position of the image, so we can modify
     the remaining parameters so that not more data that necessary is
     written to the file */
  if (!toscreen) {		/* to postscript file */
    magx = magy = 1;		/* unit magnification */
    nxx = nx;			/* original dimensions */
    nyy = ny;
  }
  image.b = image0.b = malloc(nxx*nyy*(bpp/8)*((mode & TV_24)? 3: 1));
  if (!image.b)
    return cerror(ALLOC_ERR, 0);
  
  /* now generate the image data */
  if (!(mode & (TV_RAW | TV_MAP))) {
    if (setup & 4) {		/* "reversed" images */
      if (mode & TV_24) {	/* 24-bit colors */
	switch (type) {
	  case LUX_INT8:
	    for (iy = 0; iy < nyy; iy--)
	      for (ix = 0; ix < nxx; ix++) {
		xsrc = x1 + ix/magx;
		ysrc = y1 + iy/magy;
		indx = (data.b[xsrc + ysrc*nx] - offset.f)*factor.f;
		if (indx < scalemin)
		  indx = scalemin;
		else if (indx > scalemax)
		  indx = scalemax;
		switch (bpp) {
		  case 8:
		    *image.b = (pixels[indx] & red_mask);
		    break;
		  case 16:
		    *image.w = (pixels[indx] & red_mask);
		    break;
		  case 32:
		    *image.l = (pixels[indx] & red_mask);
		    break;
		} /* end of switch (bpp) */
		indx = (data.b[xsrc + ysrc*nx + s] - offset.f)*factor.f;
		if (indx < scalemin)
		  indx = scalemin;
		else if (indx > scalemax)
		  indx = scalemax;
		switch (bpp) {
		  case 8:
		    *image.b |= (pixels[indx] & green_mask);
		    break;
		  case 16:
		    *image.w |= (pixels[indx] & green_mask);
		    break;
		  case 32:
		    *image.l |= (pixels[indx] & green_mask);
		    break;
		} /* end of switch (bpp) */
		indx = (data.b[xsrc + ysrc*nx + 2*s] - offset.f)*factor.f;
		if (indx < scalemin)
		  indx = scalemin;
		else if (indx > scalemax)
		  indx = scalemax;
		switch (bpp) {
		  case 8:
		    *image.b++ |= (pixels[indx] & blue_mask);
		    break;
		  case 16:
		    *image.w++ |= (pixels[indx] & blue_mask);
		    break;
		  case 32:
		    *image.l++ |= (pixels[indx] & blue_mask);
		    break;
		} /* end of switch (bpp) */
	      }	/* end of for (ix...) */
	    break;
	  case LUX_INT16:
	    for (iy = 0; iy < nyy; iy--)
	      for (ix = 0; ix < nxx; ix++) {
		xsrc = x1 + ix/magx;
		ysrc = y1 + iy/magy;
		indx = (data.w[xsrc + ysrc*nx] - offset.f)*factor.f;
		if (indx < scalemin)
		  indx = scalemin;
		else if (indx > scalemax)
		  indx = scalemax;
		switch (bpp) {
		  case 8:
		    *image.b = (pixels[indx] & red_mask);
		    break;
		  case 16:
		    *image.w = (pixels[indx] & red_mask);
		    break;
		  case 32:
		    *image.l = (pixels[indx] & red_mask);
		    break;
		} /* end of switch (bpp) */
		indx = (data.w[xsrc + ysrc*nx + s] - offset.f)*factor.f;
		if (indx < scalemin)
		  indx = scalemin;
		else if (indx > scalemax)
		  indx = scalemax;
		switch (bpp) {
		  case 8:
		    *image.b |= (pixels[indx] & green_mask);
		    break;
		  case 16:
		    *image.w |= (pixels[indx] & green_mask);
		    break;
		  case 32:
		    *image.l |= (pixels[indx] & green_mask);
		    break;
		} /* end of switch (bpp) */
		indx = (data.w[xsrc + ysrc*nx + 2*s] - offset.f)*factor.f;
		if (indx < scalemin)
		  indx = scalemin;
		else if (indx > scalemax)
		  indx = scalemax;
		switch (bpp) {
		  case 8:
		    *image.b++ |= (pixels[indx] & blue_mask);
		    break;
		  case 16:
		    *image.w++ |= (pixels[indx] & blue_mask);
		    break;
		  case 32:
		    *image.l++ |= (pixels[indx] & blue_mask);
		    break;
		} /* end of switch (bpp) */
	      }	/* end of for (ix...) */
	    break;
	  case LUX_INT32:
	    for (iy = 0; iy < nyy; iy--)
	      for (ix = 0; ix < nxx; ix++) {
		xsrc = x1 + ix/magx;
		ysrc = y1 + iy/magy;
		indx = (data.l[xsrc + ysrc*nx] - offset.f)*factor.f;
		if (indx < scalemin)
		  indx = scalemin;
		else if (indx > scalemax)
		  indx = scalemax;
		switch (bpp) {
		  case 8:
		    *image.b = (pixels[indx] & red_mask);
		    break;
		  case 16:
		    *image.w = (pixels[indx] & red_mask);
		    break;
		  case 32:
		    *image.l = (pixels[indx] & red_mask);
		    break;
		} /* end of switch (bpp) */
		indx = (data.l[xsrc + ysrc*nx + s] - offset.f)*factor.f;
		if (indx < scalemin)
		  indx = scalemin;
		else if (indx > scalemax)
		  indx = scalemax;
		switch (bpp) {
		  case 8:
		    *image.b |= (pixels[indx] & green_mask);
		    break;
		  case 16:
		    *image.w |= (pixels[indx] & green_mask);
		    break;
		  case 32:
		    *image.l |= (pixels[indx] & green_mask);
		    break;
		} /* end of switch (bpp) */
		indx = (data.l[xsrc + ysrc*nx + 2*s] - offset.f)*factor.f;
		if (indx < scalemin)
		  indx = scalemin;
		else if (indx > scalemax)
		  indx = scalemax;
		switch (bpp) {
		  case 8:
		    *image.b++ |= (pixels[indx] & blue_mask);
		    break;
		  case 16:
		    *image.w++ |= (pixels[indx] & blue_mask);
		    break;
		  case 32:
		    *image.l++ |= (pixels[indx] & blue_mask);
		    break;
		} /* end of switch (bpp) */
	      }	/* end of for (ix...) */
	    break;
	  case LUX_INT64:
	    for (iy = 0; iy < nyy; iy--)
	      for (ix = 0; ix < nxx; ix++) {
		xsrc = x1 + ix/magx;
		ysrc = y1 + iy/magy;
		indx = (data.q[xsrc + ysrc*nx] - offset.f)*factor.f;
		if (indx < scalemin)
		  indx = scalemin;
		else if (indx > scalemax)
		  indx = scalemax;
		switch (bpp) {
		  case 8:
		    *image.b = (pixels[indx] & red_mask);
		    break;
		  case 16:
		    *image.w = (pixels[indx] & red_mask);
		    break;
		  case 32:
		    *image.l = (pixels[indx] & red_mask);
		    break;
		} /* end of switch (bpp) */
		indx = (data.q[xsrc + ysrc*nx + s] - offset.f)*factor.f;
		if (indx < scalemin)
		  indx = scalemin;
		else if (indx > scalemax)
		  indx = scalemax;
		switch (bpp) {
		  case 8:
		    *image.b |= (pixels[indx] & green_mask);
		    break;
		  case 16:
		    *image.w |= (pixels[indx] & green_mask);
		    break;
		  case 32:
		    *image.l |= (pixels[indx] & green_mask);
		    break;
		} /* end of switch (bpp) */
		indx = (data.q[xsrc + ysrc*nx + 2*s] - offset.f)*factor.f;
		if (indx < scalemin)
		  indx = scalemin;
		else if (indx > scalemax)
		  indx = scalemax;
		switch (bpp) {
		  case 8:
		    *image.b++ |= (pixels[indx] & blue_mask);
		    break;
		  case 16:
		    *image.w++ |= (pixels[indx] & blue_mask);
		    break;
		  case 32:
		    *image.l++ |= (pixels[indx] & blue_mask);
		    break;
		} /* end of switch (bpp) */
	      }	/* end of for (ix...) */
	    break;
	  case LUX_FLOAT:
	    for (iy = 0; iy < nyy; iy--)
	      for (ix = 0; ix < nxx; ix++) {
		xsrc = x1 + ix/magx;
		ysrc = y1 + iy/magy;
		indx = (data.f[xsrc + ysrc*nx] - offset.f)*factor.f;
		if (indx < scalemin)
		  indx = scalemin;
		else if (indx > scalemax)
		  indx = scalemax;
		switch (bpp) {
		  case 8:
		    *image.b = (pixels[indx] & red_mask);
		    break;
		  case 16:
		    *image.w = (pixels[indx] & red_mask);
		    break;
		  case 32:
		    *image.l = (pixels[indx] & red_mask);
		    break;
		} /* end of switch (bpp) */
		indx = (data.f[xsrc + ysrc*nx + s] - offset.f)*factor.f;
		if (indx < scalemin)
		  indx = scalemin;
		else if (indx > scalemax)
		  indx = scalemax;
		switch (bpp) {
		  case 8:
		    *image.b |= (pixels[indx] & green_mask);
		    break;
		  case 16:
		    *image.w |= (pixels[indx] & green_mask);
		    break;
		  case 32:
		    *image.l |= (pixels[indx] & green_mask);
		    break;
		} /* end of switch (bpp) */
		indx = (data.f[xsrc + ysrc*nx + 2*s] - offset.f)*factor.f;
		if (indx < scalemin)
		  indx = scalemin;
		else if (indx > scalemax)
		  indx = scalemax;
		switch (bpp) {
		  case 8:
		    *image.b++ |= (pixels[indx] & blue_mask);
		    break;
		  case 16:
		    *image.w++ |= (pixels[indx] & blue_mask);
		    break;
		  case 32:
		    *image.l++ |= (pixels[indx] & blue_mask);
		    break;
		} /* end of switch (bpp) */
	      }	/* end of for (ix...) */
	    break;
	  case LUX_DOUBLE:
	    for (iy = 0; iy < nyy; iy--)
	      for (ix = 0; ix < nxx; ix++) {
		xsrc = x1 + ix/magx;
		ysrc = y1 + iy/magy;
		indx = (data.d[xsrc + ysrc*nx] - offset.f)*factor.f;
		if (indx < scalemin)
		  indx = scalemin;
		else if (indx > scalemax)
		  indx = scalemax;
		switch (bpp) {
		  case 8:
		    *image.b = (pixels[indx] & red_mask);
		    break;
		  case 16:
		    *image.w = (pixels[indx] & red_mask);
		    break;
		  case 32:
		    *image.l = (pixels[indx] & red_mask);
		    break;
		} /* end of switch (bpp) */
		indx = (data.d[xsrc + ysrc*nx + s] - offset.f)*factor.f;
		if (indx < scalemin)
		  indx = scalemin;
		else if (indx > scalemax)
		  indx = scalemax;
		switch (bpp) {
		  case 8:
		    *image.b |= (pixels[indx] & green_mask);
		    break;
		  case 16:
		    *image.w |= (pixels[indx] & green_mask);
		    break;
		  case 32:
		    *image.l |= (pixels[indx] & green_mask);
		    break;
		} /* end of switch (bpp) */
		indx = (data.d[xsrc + ysrc*nx + 2*s] - offset.f)*factor.f;
		if (indx < scalemin)
		  indx = scalemin;
		else if (indx > scalemax)
		  indx = scalemax;
		switch (bpp) {
		  case 8:
		    *image.b++ |= (pixels[indx] & blue_mask);
		    break;
		  case 16:
		    *image.w++ |= (pixels[indx] & blue_mask);
		    break;
		  case 32:
		    *image.l++ |= (pixels[indx] & blue_mask);
		    break;
		} /* end of switch (bpp) */
	      }	/* end of for (ix...) */
	    break;
	} /* end of switch (type) */
      }	/* end of if (mode & TV_24) */
      else if (threeColors) {	/* have three-color colortable */
	switch (type) {
	  case LUX_INT8:
	    for (iy = 0; iy < nyy; iy++)
	      for (ix = 0; ix < nxx; ix++) {
		xsrc = x1 + ix/magx;
		ysrc = y1 + iy/magy;
		indx = (data.b[xsrc + ysrc*nx] - offset.f)*factor.f;
		if (indx < 0)
		  indx = 0;
		else if (indx >= 256/3)
		  indx = 256/3 - 1;
		if (bitmap1 && bitmap1[xsrc + ysrc*nx])
		  indx += 256/3;
		else if (bitmap2 && bitmap2[xsrc + ysrc*nx])
		  indx += 2*(256/3);
		switch (bpp) {
		  case 8:
		    *image.b++ = toscreen? pixels[indx]: indx;
		    break;
		  case 16:
		    *image.w++ = toscreen? pixels[indx]: indx;
		    break;
		  case 32:
		    *image.l++ = toscreen? pixels[indx]: indx;
		    break;
		} /* end of switch (bpp) */
	      }	/* end of for (ix...) */
	    break;
	  case LUX_INT16:
	    for (iy = 0; iy < nyy; iy++)
	      for (ix = 0; ix < nxx; ix++) {
		xsrc = x1 + ix/magx;
		ysrc = y1 + iy/magy;
		indx = (data.w[xsrc + ysrc*nx] - offset.f)*factor.f;
		if (indx < 0)
		  indx = 0;
		else if (indx >= 256/3)
		  indx = 256/3 - 1;
		if (bitmap1 && bitmap1[xsrc + ysrc*nx])
		  indx += 256/3;
		else if (bitmap2 && bitmap2[xsrc + ysrc*nx])
		  indx += 2*(256/3);
		switch (bpp) {
		  case 8:
		    *image.b++ = toscreen? pixels[indx]: indx;
		    break;
		  case 16:
		    *image.w++ = toscreen? pixels[indx]: indx;
		    break;
		  case 32:
		    *image.l++ = toscreen? pixels[indx]: indx;
		    break;
		} /* end of switch (bpp) */
	      }	/* end of for (ix...) */
	    break;
	  case LUX_INT32:
	    for (iy = 0; iy < nyy; iy++)
	      for (ix = 0; ix < nxx; ix++) {
		xsrc = x1 + ix/magx;
		ysrc = y1 + iy/magy;
		indx = (data.l[xsrc + ysrc*nx] - offset.f)*factor.f;
		if (indx < 0)
		  indx = 0;
		else if (indx >= 256/3)
		  indx = 256/3 - 1;
		if (bitmap1 && bitmap1[xsrc + ysrc*nx])
		  indx += 256/3;
		else if (bitmap2 && bitmap2[xsrc + ysrc*nx])
		  indx += 2*(256/3);
		switch (bpp) {
		  case 8:
		    *image.b++ = toscreen? pixels[indx]: indx;
		    break;
		  case 16:
		    *image.w++ = toscreen? pixels[indx]: indx;
		    break;
		  case 32:
		    *image.l++ = toscreen? pixels[indx]: indx;
		    break;
		} /* end of switch (bpp) */
	      }	/* end of for (ix...) */
	    break;
	  case LUX_INT64:
	    for (iy = 0; iy < nyy; iy++)
	      for (ix = 0; ix < nxx; ix++) {
		xsrc = x1 + ix/magx;
		ysrc = y1 + iy/magy;
		indx = (data.q[xsrc + ysrc*nx] - offset.f)*factor.f;
		if (indx < 0)
		  indx = 0;
		else if (indx >= 256/3)
		  indx = 256/3 - 1;
		if (bitmap1 && bitmap1[xsrc + ysrc*nx])
		  indx += 256/3;
		else if (bitmap2 && bitmap2[xsrc + ysrc*nx])
		  indx += 2*(256/3);
		switch (bpp) {
		  case 8:
		    *image.b++ = toscreen? pixels[indx]: indx;
		    break;
		  case 16:
		    *image.w++ = toscreen? pixels[indx]: indx;
		    break;
		  case 32:
		    *image.l++ = toscreen? pixels[indx]: indx;
		    break;
		} /* end of switch (bpp) */
	      }	/* end of for (ix...) */
	    break;
	  case LUX_FLOAT:
	    for (iy = 0; iy < nyy; iy++)
	      for (ix = 0; ix < nxx; ix++) {
		xsrc = x1 + ix/magx;
		ysrc = y1 + iy/magy;
		indx = (data.f[xsrc + ysrc*nx] - offset.f)*factor.f;
		if (indx < 0)
		  indx = 0;
		else if (indx >= 256/3)
		  indx = 256/3 - 1;
		if (bitmap1 && bitmap1[xsrc + ysrc*nx])
		  indx += 256/3;
		else if (bitmap2 && bitmap2[xsrc + ysrc*nx])
		  indx += 2*(256/3);
		switch (bpp) {
		  case 8:
		    *image.b++ = toscreen? pixels[indx]: indx;
		    break;
		  case 16:
		    *image.w++ = toscreen? pixels[indx]: indx;
		    break;
		  case 32:
		    *image.l++ = toscreen? pixels[indx]: indx;
		    break;
		} /* end of switch (bpp) */
	      }	/* end of for (ix) */
	    break;
	  case LUX_DOUBLE:
	    for (iy = 0; iy < nyy; iy++)
	      for (ix = 0; ix < nxx; ix++) {
		xsrc = x1 + ix/magx;
		ysrc = y1 + iy/magy;
		indx = (data.d[xsrc + ysrc*nx] - offset.f)*factor.f;
		if (indx < 0)
		  indx = 0;
		else if (indx >= 256/3)
		  indx = 256/3 - 1;
		if (bitmap1 && bitmap1[xsrc + ysrc*nx])
		  indx += 256/3;
		else if (bitmap2 && bitmap2[xsrc + ysrc*nx])
		  indx += 2*(256/3);
		switch (bpp) {
		  case 8:
		    *image.b++ = toscreen? pixels[indx]: indx;
		    break;
		  case 16:
		    *image.w++ = toscreen? pixels[indx]: indx;
		    break;
		  case 32:
		    *image.l++ = toscreen? pixels[indx]: indx;
		    break;
		} /* end of switch (bpp) */
	      }	/* end of for (ix) */
	    break;
	} /* end of switch (type) */
      } /* end of if (mode & TV_24) else if (threeColors) */
      else switch (type) {	/* ordinary color table */
	case LUX_INT8:
	  for (iy = 0; iy < nyy; iy++)
	    for (ix = 0; ix < nxx; ix++) {
	      xsrc = x1 + ix/magx;
	      ysrc = y1 + iy/magy;
	      indx = (data.b[xsrc + ysrc*nx] - offset.f)*factor.f
		+ scalemin;
	      if (indx < scalemin)
		indx = scalemin;
	      else if (indx > scalemax)
		indx = scalemax;
	      switch (bpp) {
		case 8:
		  *image.b++ = toscreen? pixels[indx]: indx;
		  break;
		case 16:
		  *image.w++ = toscreen? pixels[indx]: indx;
		  break;
		case 32:
		  *image.l++ = toscreen? pixels[indx]: indx;
		  break;
	      }	/* end of switch (bpp) */
	    } /* end of for (ix) */
	  break;
	case LUX_INT16:
	  for (iy = 0; iy < nyy; iy++)
	    for (ix = 0; ix < nxx; ix++) {
	      xsrc = x1 + ix/magx;
	      ysrc = y1 + iy/magy;
	      indx = (data.w[xsrc + ysrc*nx] - offset.f)*factor.f
		+ scalemin;
	      if (indx < scalemin)
		indx = scalemin;
	      else if (indx > scalemax)
		indx = scalemax;
	      switch (bpp) {
		case 8:
		  *image.b++ = toscreen? pixels[indx]: indx;
		  break;
		case 16:
		  *image.w++ = toscreen? pixels[indx]: indx;
		  break;
		case 32:
		  *image.l++ = toscreen? pixels[indx]: indx;
		  break;
	      }	/* end of switch (bpp) */
	    } /* end of for (ix) */
	  break;
	case LUX_INT32:
	  for (iy = 0; iy < nyy; iy++)
	    for (ix = 0; ix < nxx; ix++) {
	      xsrc = x1 + ix/magx;
	      ysrc = y1 + iy/magy;
	      indx = (data.l[xsrc + ysrc*nx] - offset.f)*factor.f
		+ scalemin;
	      if (indx < scalemin)
		indx = scalemin;
	      else if (indx > scalemax)
		indx = scalemax;
	      switch (bpp) {
		case 8:
		  *image.b++ = toscreen? pixels[indx]: indx;
		  break;
		case 16:
		  *image.w++ = toscreen? pixels[indx]: indx;
		  break;
		case 32:
		  *image.l++ = toscreen? pixels[indx]: indx;
		  break;
	      }	/* end of switch (bpp) */
	    } /* end of for (ix) */
	  break;
	case LUX_INT64:
	  for (iy = 0; iy < nyy; iy++)
	    for (ix = 0; ix < nxx; ix++) {
	      xsrc = x1 + ix/magx;
	      ysrc = y1 + iy/magy;
	      indx = (data.q[xsrc + ysrc*nx] - offset.f)*factor.f
		+ scalemin;
	      if (indx < scalemin)
		indx = scalemin;
	      else if (indx > scalemax)
		indx = scalemax;
	      switch (bpp) {
		case 8:
		  *image.b++ = toscreen? pixels[indx]: indx;
		  break;
		case 16:
		  *image.w++ = toscreen? pixels[indx]: indx;
		  break;
		case 32:
		  *image.l++ = toscreen? pixels[indx]: indx;
		  break;
	      }	/* end of switch (bpp) */
	    } /* end of for (ix) */
	  break;
	case LUX_FLOAT:
	  for (iy = 0; iy < nyy; iy++)
	    for (ix = 0; ix < nxx; ix++) {
	      xsrc = x1 + ix/magx;
	      ysrc = y1 + iy/magy;
	      indx = (data.f[xsrc + ysrc*nx] - offset.f)*factor.f
		+ scalemin;
	      if (indx < scalemin)
		indx = scalemin;
	      else if (indx > scalemax)
		indx = scalemax;
	      switch (bpp) {
		case 8:
		  *image.b++ = toscreen? pixels[indx]: indx;
		  break;
		case 16:
		  *image.w++ = toscreen? pixels[indx]: indx;
		  break;
		case 32:
		  *image.l++ = toscreen? pixels[indx]: indx;
		  break;
	      }	/* end of switch (bpp) */
	    } /* end of for (ix) */
	  break;
	case LUX_DOUBLE:
	  for (iy = 0; iy < nyy; iy++)
	    for (ix = 0; ix < nxx; ix++) {
	      xsrc = x1 + ix/magx;
	      ysrc = y1 + iy/magy;
	      indx = (data.d[xsrc + ysrc*nx] - offset.d)*factor.d
		+ scalemin;
	      if (indx < scalemin)
		indx = scalemin;
	      else if (indx > scalemax)
		indx = scalemax;
	      switch (bpp) {
		case 8:
		  *image.b++ = toscreen? pixels[indx]: indx;
		  break;
		case 16:
		  *image.w++ = toscreen? pixels[indx]: indx;
		  break;
		case 32:
		  *image.l++ = toscreen? pixels[indx]: indx;
		  break;
	      }	/* end of switch (bpp) */
	    } /* end of for (ix) */
	  break;
      }	/* end if (mode & TV_24) else if (threeColors) else switch (type) */
    } /* end of if (setup & 4) */
    else {			/* standard image */
      if (mode & TV_24) {	/* 24-bit colors */
	switch (type) {
	  case LUX_INT8:
	    for (iy = nyy - 1; iy >= 0; iy--)
	      for (ix = 0; ix < nxx; ix++) {
		xsrc = x1 + ix/magx;
		ysrc = y1 + iy/magy;
		indx = (data.b[xsrc + ysrc*nx] - offset.f)*factor.f;
		if (indx < 0)
		  indx = 0;
		else if (indx > 255)
		  indx = 255;
		switch (bpp) {
		  case 8:
		    *image.b = (pixels[indx] & red_mask);
		    break;
		  case 16:
		    *image.w = (pixels[indx] & red_mask);
		    break;
		  case 32:
		    *image.l = (pixels[indx] & red_mask);
		    break;
		} /* end of switch (bpp) */
		indx = (data.b[xsrc + ysrc*nx + s] - offset.f)*factor.f;
		if (indx < 0)
		  indx = 0;
		else if (indx > 255)
		  indx = 255;
		switch (bpp) {
		  case 8:
		    *image.b |= (pixels[indx] & green_mask);
		    break;
		  case 16:
		    *image.w |= (pixels[indx] & green_mask);
		    break;
		  case 32:
		    *image.l |= (pixels[indx] & green_mask);
		    break;
		} /* end of switch (bpp) */
		indx = (data.b[xsrc + ysrc*nx + 2*s] - offset.f)*factor.f;
		if (indx < 0)
		  indx = 0;
		else if (indx > 255)
		  indx = 255;
		switch (bpp) {
		  case 8:
		    *image.b++ |= (pixels[indx] & blue_mask);
		    break;
		  case 16:
		    *image.w++ |= (pixels[indx] & blue_mask);
		    break;
		  case 32:
		    *image.l++ |= (pixels[indx] & blue_mask);
		    break;
		} /* end of switch (bpp) */
	      }	/* end of for (ix) */
	    break;
	  case LUX_INT16:
	    for (iy = nyy - 1; iy >= 0; iy--)
	      for (ix = 0; ix < nxx; ix++) {
		xsrc = x1 + ix/magx;
		ysrc = y1 + iy/magy;
		indx = (data.w[xsrc + ysrc*nx] - offset.f)*factor.f;
		if (indx < 0)
		  indx = 0;
		else if (indx > 255)
		  indx = 255;
		switch (bpp) {
		  case 8:
		    *image.b = (pixels[indx] & red_mask);
		    break;
		  case 16:
		    *image.w = (pixels[indx] & red_mask);
		    break;
		  case 32:
		    *image.l = (pixels[indx] & red_mask);
		    break;
		} /* end of switch (bpp) */
		indx = (data.w[xsrc + ysrc*nx + s] - offset.f)*factor.f;
		if (indx < 0)
		  indx = 0;
		else if (indx > 255)
		  indx = 255;
		switch (bpp) {
		  case 8:
		    *image.b |= (pixels[indx] & green_mask);
		    break;
		  case 16:
		    *image.w |= (pixels[indx] & green_mask);
		    break;
		  case 32:
		    *image.l |= (pixels[indx] & green_mask);
		    break;
		} /* end of switch (bpp) */
		indx = (data.w[xsrc + ysrc*nx + 2*s] - offset.f)*factor.f;
		if (indx < 0)
		  indx = 0;
		else if (indx > 255)
		  indx = 255;
		switch (bpp) {
		  case 8:
		    *image.b++ |= (pixels[indx] & blue_mask);
		    break;
		  case 16:
		    *image.w++ |= (pixels[indx] & blue_mask);
		    break;
		  case 32:
		    *image.l++ |= (pixels[indx] & blue_mask);
		    break;
		} /* end of switch (bpp) */
	      }	/* end of for (ix) */
	    break;
	  case LUX_INT32:
	    for (iy = nyy - 1; iy >= 0; iy--)
	      for (ix = 0; ix < nxx; ix++) {
		xsrc = x1 + ix/magx;
		ysrc = y1 + iy/magy;
		indx = (data.l[xsrc + ysrc*nx] - offset.f)*factor.f;
		if (indx < 0)
		  indx = 0;
		else if (indx > 255)
		  indx = 255;
		switch (bpp) {
		  case 8:
		    *image.b = (pixels[indx] & red_mask);
		    break;
		  case 16:
		    *image.w = (pixels[indx] & red_mask);
		    break;
		  case 32:
		    *image.l = (pixels[indx] & red_mask);
		    break;
		} /* end of switch (bpp) */
		indx = (data.l[xsrc + ysrc*nx + s] - offset.f)*factor.f;
		if (indx < 0)
		  indx = 0;
		else if (indx > 255)
		  indx = 255;
		switch (bpp) {
		  case 8:
		    *image.b |= (pixels[indx] & green_mask);
		    break;
		  case 16:
		    *image.w |= (pixels[indx] & green_mask);
		    break;
		  case 32:
		    *image.l |= (pixels[indx] & green_mask);
		    break;
		} /* end of switch (bpp) */
		indx = (data.l[xsrc + ysrc*nx + 2*s] - offset.f)*factor.f;
		if (indx < 0)
		  indx = 0;
		else if (indx > 255)
		  indx = 255;
		switch (bpp) {
		  case 8:
		    *image.b++ |= (pixels[indx] & blue_mask);
		    break;
		  case 16:
		    *image.w++ |= (pixels[indx] & blue_mask);
		    break;
		  case 32:
		    *image.l++ |= (pixels[indx] & blue_mask);
		    break;
		} /* end of switch (bpp) */
	      }	/* end of for (ix) */
	    break;
	  case LUX_INT64:
	    for (iy = nyy - 1; iy >= 0; iy--)
	      for (ix = 0; ix < nxx; ix++) {
		xsrc = x1 + ix/magx;
		ysrc = y1 + iy/magy;
		indx = (data.q[xsrc + ysrc*nx] - offset.f)*factor.f;
		if (indx < 0)
		  indx = 0;
		else if (indx > 255)
		  indx = 255;
		switch (bpp) {
		  case 8:
		    *image.b = (pixels[indx] & red_mask);
		    break;
		  case 16:
		    *image.w = (pixels[indx] & red_mask);
		    break;
		  case 32:
		    *image.l = (pixels[indx] & red_mask);
		    break;
		} /* end of switch (bpp) */
		indx = (data.q[xsrc + ysrc*nx + s] - offset.f)*factor.f;
		if (indx < 0)
		  indx = 0;
		else if (indx > 255)
		  indx = 255;
		switch (bpp) {
		  case 8:
		    *image.b |= (pixels[indx] & green_mask);
		    break;
		  case 16:
		    *image.w |= (pixels[indx] & green_mask);
		    break;
		  case 32:
		    *image.l |= (pixels[indx] & green_mask);
		    break;
		} /* end of switch (bpp) */
		indx = (data.q[xsrc + ysrc*nx + 2*s] - offset.f)*factor.f;
		if (indx < 0)
		  indx = 0;
		else if (indx > 255)
		  indx = 255;
		switch (bpp) {
		  case 8:
		    *image.b++ |= (pixels[indx] & blue_mask);
		    break;
		  case 16:
		    *image.w++ |= (pixels[indx] & blue_mask);
		    break;
		  case 32:
		    *image.l++ |= (pixels[indx] & blue_mask);
		    break;
		} /* end of switch (bpp) */
	      }	/* end of for (ix) */
	    break;
	  case LUX_FLOAT:
	    for (iy = nyy - 1; iy >= 0; iy--)
	      for (ix = 0; ix < nxx; ix++) {
		xsrc = x1 + ix/magx;
		ysrc = y1 + iy/magy;
		indx = (data.f[xsrc + ysrc*nx] - offset.f)*factor.f;
		if (indx < 0)
		  indx = 0;
		else if (indx > 255)
		  indx = 255;
		switch (bpp) {
		  case 8:
		    *image.b = (pixels[indx] & red_mask);
		    break;
		  case 16:
		    *image.w = (pixels[indx] & red_mask);
		    break;
		  case 32:
		    *image.l = (pixels[indx] & red_mask);
		    break;
		} /* end of switch (bpp) */
		indx = (data.f[xsrc + ysrc*nx + s] - offset.f)*factor.f;
		if (indx < 0)
		  indx = 0;
		else if (indx > 255)
		  indx = 255;
		switch (bpp) {
		  case 8:
		    *image.b |= (pixels[indx] & green_mask);
		    break;
		  case 16:
		    *image.w |= (pixels[indx] & green_mask);
		    break;
		  case 32:
		    *image.l |= (pixels[indx] & green_mask);
		    break;
		} /* end of switch (bpp) */
		indx = (data.f[xsrc + ysrc*nx + 2*s] - offset.f)*factor.f;
		if (indx < 0)
		  indx = 0;
		else if (indx > 255)
		  indx = 255;
		switch (bpp) {
		  case 8:
		    *image.b++ |= (pixels[indx] & blue_mask);
		    break;
		  case 16:
		    *image.w++ |= (pixels[indx] & blue_mask);
		    break;
		  case 32:
		    *image.l++ |= (pixels[indx] & blue_mask);
		    break;
		} /* end of switch (bpp) */
	      }	/* end of for (ix) */
	    break;
	  case LUX_DOUBLE:
	    for (iy = nyy - 1; iy >= 0; iy--)
	      for (ix = 0; ix < nxx; ix++) {
		xsrc = x1 + ix/magx;
		ysrc = y1 + iy/magy;
		indx = (data.d[xsrc + ysrc*nx] - offset.f)*factor.f;
		if (indx < 0)
		  indx = 0;
		else if (indx > 255)
		  indx = 255;
		switch (bpp) {
		  case 8:
		    *image.b = (pixels[indx] & red_mask);
		    break;
		  case 16:
		    *image.w = (pixels[indx] & red_mask);
		    break;
		  case 32:
		    *image.l = (pixels[indx] & red_mask);
		    break;
		} /* end of switch (bpp) */
		indx = (data.d[xsrc + ysrc*nx + s] - offset.f)*factor.f;
		if (indx < 0)
		  indx = 0;
		else if (indx > 255)
		  indx = 255;
		switch (bpp) {
		  case 8:
		    *image.b |= (pixels[indx] & green_mask);
		    break;
		  case 16:
		    *image.w |= (pixels[indx] & green_mask);
		    break;
		  case 32:
		    *image.l |= (pixels[indx] & green_mask);
		    break;
		} /* end of switch (bpp) */
		indx = (data.d[xsrc + ysrc*nx + 2*s] - offset.f)*factor.f;
		if (indx < 0)
		  indx = 0;
		else if (indx > 255)
		  indx = 255;
		switch (bpp) {
		  case 8:
		    *image.b++ |= (pixels[indx] & blue_mask);
		    break;
		  case 16:
		    *image.w++ |= (pixels[indx] & blue_mask);
		    break;
		  case 32:
		    *image.l++ |= (pixels[indx] & blue_mask);
		    break;
		} /* end of switch (bpp) */
	      }	/* end of for (ix) */
	    break;
	} /* end of switch (type) */
      }	/* end of if (mode & TV_24) */
      else if (threeColors) {	/* have three-color colortable */
	switch (type) {
	  case LUX_INT8:
	    for (iy = nyy - 1; iy >= 0; iy--)
	      for (ix = 0; ix < nxx; ix++) {
		xsrc = x1 + ix/magx;
		ysrc = y1 + iy/magy;
		indx = (data.b[xsrc + ysrc*nx] - offset.f)*factor.f;
		if (indx < 0)
		  indx = 0;
		else if (indx >= 256/3)
		  indx = 256/3 - 1;
		if (bitmap1 && bitmap1[xsrc + ysrc*nx])
		  indx += 256/3;
		else if (bitmap2 && bitmap2[xsrc + ysrc*nx])
		  indx += 2*(256/3);
		switch (bpp) {
		  case 8:
		    *image.b++ = toscreen? pixels[indx]: indx;
		    break;
		  case 16:
		    *image.w++ = toscreen? pixels[indx]: indx;
		    break;
		  case 32:
		    *image.l++ = toscreen? pixels[indx]: indx;
		    break;
		} /* end of switch (bpp) */
	      }	/* end of for (ix) */
	    break;
	  case LUX_INT16:
	    for (iy = nyy - 1; iy >= 0; iy--)
	      for (ix = 0; ix < nxx; ix++) {
		xsrc = x1 + ix/magx;
		ysrc = y1 + iy/magy;
		indx = (data.w[xsrc + ysrc*nx] - offset.f)*factor.f;
		if (indx < 0)
		  indx = 0;
		else if (indx >= 256/3)
		  indx = 256/3 - 1;
		if (bitmap1 && bitmap1[xsrc + ysrc*nx])
		  indx += 256/3;
		else if (bitmap2 && bitmap2[xsrc + ysrc*nx])
		  indx += 2*(256/3);
		switch (bpp) {
		  case 8:
		    *image.b++ = toscreen? pixels[indx]: indx;
		    break;
		  case 16:
		    *image.w++ = toscreen? pixels[indx]: indx;
		    break;
		  case 32:
		    *image.l++ = toscreen? pixels[indx]: indx;
		    break;
		} /* end of switch (bpp) */
	      }	/* end of if (ix) */
	    break;
	  case LUX_INT32:
	    for (iy = nyy - 1; iy >= 0; iy--)
	      for (ix = 0; ix < nxx; ix++) {
		xsrc = x1 + ix/magx;
		ysrc = y1 + iy/magy;
		indx = (data.l[xsrc + ysrc*nx] - offset.f)*factor.f;
		if (indx < 0)
		  indx = 0;
		else if (indx >= 256/3)
		  indx = 256/3 - 1;
		if (bitmap1 && bitmap1[xsrc + ysrc*nx])
		  indx += 256/3;
		else if (bitmap2 && bitmap2[xsrc + ysrc*nx])
		  indx += 2*(256/3);
		switch (bpp) {
		  case 8:
		    *image.b++ = toscreen? pixels[indx]: indx;
		    break;
		  case 16:
		    *image.w++ = toscreen? pixels[indx]: indx;
		    break;
		  case 32:
		    *image.l++ = toscreen? pixels[indx]: indx;
		    break;
		} /* end of switch (bpp) */
	      }	/* end of if (ix) */
	    break;
	  case LUX_INT64:
	    for (iy = nyy - 1; iy >= 0; iy--)
	      for (ix = 0; ix < nxx; ix++) {
		xsrc = x1 + ix/magx;
		ysrc = y1 + iy/magy;
		indx = (data.q[xsrc + ysrc*nx] - offset.f)*factor.f;
		if (indx < 0)
		  indx = 0;
		else if (indx >= 256/3)
		  indx = 256/3 - 1;
		if (bitmap1 && bitmap1[xsrc + ysrc*nx])
		  indx += 256/3;
		else if (bitmap2 && bitmap2[xsrc + ysrc*nx])
		  indx += 2*(256/3);
		switch (bpp) {
		  case 8:
		    *image.b++ = toscreen? pixels[indx]: indx;
		    break;
		  case 16:
		    *image.w++ = toscreen? pixels[indx]: indx;
		    break;
		  case 32:
		    *image.l++ = toscreen? pixels[indx]: indx;
		    break;
		} /* end of switch (bpp) */
	      }	/* end of if (ix) */
	    break;
	  case LUX_FLOAT:
	    for (iy = nyy - 1; iy >= 0; iy--)
	      for (ix = 0; ix < nxx; ix++) {
		xsrc = x1 + ix/magx;
		ysrc = y1 + iy/magy;
		indx = (data.f[xsrc + ysrc*nx] - offset.f)*factor.f;
		if (indx < 0)
		  indx = 0;
		else if (indx >= 256/3)
		  indx = 256/3 - 1;
		if (bitmap1 && bitmap1[xsrc + ysrc*nx])
		  indx += 256/3;
		else if (bitmap2 && bitmap2[xsrc + ysrc*nx])
		  indx += 2*(256/3);
		switch (bpp) {
		  case 8:
		    *image.b++ = toscreen? pixels[indx]: indx;
		    break;
		  case 16:
		    *image.w++ = toscreen? pixels[indx]: indx;
		    break;
		  case 32:
		    *image.l++ = toscreen? pixels[indx]: indx;
		    break;
		} /* end of switch (bpp) */
	      }	/* end of if (ix) */
	    break;
	  case LUX_DOUBLE:
	    for (iy = nyy - 1; iy >= 0; iy--)
	      for (ix = 0; ix < nxx; ix++) {
		xsrc = x1 + ix/magx;
		ysrc = y1 + iy/magy;
		indx = (data.d[xsrc + ysrc*nx] - offset.f)*factor.f;
		if (indx < 0)
		  indx = 0;
		else if (indx >= 256/3)
		  indx = 256/3 - 1;
		if (bitmap1 && bitmap1[xsrc + ysrc*nx])
		  indx += 256/3;
		else if (bitmap2 && bitmap2[xsrc + ysrc*nx])
		  indx += 2*(256/3);
		switch (bpp) {
		  case 8:
		    *image.b++ = toscreen? pixels[indx]: indx;
		    break;
		  case 16:
		    *image.w++ = toscreen? pixels[indx]: indx;
		    break;
		  case 32:
		    *image.l++ = toscreen? pixels[indx]: indx;
		    break;
		} /* end of switch (bpp) */
	      }	/* end of if (ix) */
	    break;
	} /* end of switch (type) */
      }	/* end of if (mode & TV_24) else if (threeColors) */
      else switch (type) {	/* ordinary color table */
	case LUX_INT8:
	  for (iy = nyy - 1; iy >= 0; iy--)
	    for (ix = 0; ix < nxx; ix++) {
	      xsrc = x1 + ix/magx;
	      ysrc = y1 + iy/magy;
	      indx = (data.b[xsrc + ysrc*nx] - offset.f)*factor.f
		+ scalemin;
	      if (indx < scalemin)
		indx = scalemin;
	      else if (indx > scalemax)
		indx = scalemax;
	      switch (bpp) {
		case 8:
		  *image.b++ = toscreen? pixels[indx]: indx;
		  break;
		case 16:
		  *image.w++ = toscreen? pixels[indx]: indx;
		  break;
		case 32:
		  *image.l++ = toscreen? pixels[indx]: indx;
		  break;
	      }	/* end of switch (bpp) */
	    } /* end of for (ix) */
	  break;
	case LUX_INT16:
	  for (iy = nyy - 1; iy >= 0; iy--)
	    for (ix = 0; ix < nxx; ix++) {
	      xsrc = x1 + ix/magx;
	      ysrc = y1 + iy/magy;
	      indx = (data.w[xsrc + ysrc*nx] - offset.f)*factor.f
		+ scalemin;
	      if (indx < scalemin)
		indx = scalemin;
	      else if (indx > scalemax)
		indx = scalemax;
	      switch (bpp) {
		case 8:
		  *image.b++ = toscreen? pixels[indx]: indx;
		  break;
		case 16:
		  *image.w++ = toscreen? pixels[indx]: indx;
		  break;
		case 32:
		  *image.l++ = toscreen? pixels[indx]: indx;
		  break;
	      }	/* end of switch (bpp) */
	    } /* end of for (ix) */
	  break;
	case LUX_INT32:
	  for (iy = nyy - 1; iy >= 0; iy--)
	    for (ix = 0; ix < nxx; ix++) {
	      xsrc = x1 + ix/magx;
	      ysrc = y1 + iy/magy;
	      indx = (data.l[xsrc + ysrc*nx] - offset.f)*factor.f
		+ scalemin;
	      if (indx < scalemin)
		indx = scalemin;
	      else if (indx > scalemax)
		indx = scalemax;
	      switch (bpp) {
		case 8:
		  *image.b++ = toscreen? pixels[indx]: indx;
		  break;
		case 16:
		  *image.w++ = toscreen? pixels[indx]: indx;
		  break;
		case 32:
		  *image.l++ = toscreen? pixels[indx]: indx;
		  break;
	      }	/* end of switch (bpp) */
	    } /* end of for (ix) */
	  break;
	case LUX_INT64:
	  for (iy = nyy - 1; iy >= 0; iy--)
	    for (ix = 0; ix < nxx; ix++) {
	      xsrc = x1 + ix/magx;
	      ysrc = y1 + iy/magy;
	      indx = (data.q[xsrc + ysrc*nx] - offset.f)*factor.f
		+ scalemin;
	      if (indx < scalemin)
		indx = scalemin;
	      else if (indx > scalemax)
		indx = scalemax;
	      switch (bpp) {
		case 8:
		  *image.b++ = toscreen? pixels[indx]: indx;
		  break;
		case 16:
		  *image.w++ = toscreen? pixels[indx]: indx;
		  break;
		case 32:
		  *image.l++ = toscreen? pixels[indx]: indx;
		  break;
	      }	/* end of switch (bpp) */
	    } /* end of for (ix) */
	  break;
	case LUX_FLOAT:
	  for (iy = nyy - 1; iy >= 0; iy--)
	    for (ix = 0; ix < nxx; ix++) {
	      xsrc = x1 + ix/magx;
	      ysrc = y1 + iy/magy;
	      indx = (data.f[xsrc + ysrc*nx] - offset.f)*factor.f
		+ scalemin;
	      if (indx < scalemin)
		indx = scalemin;
	      else if (indx > scalemax)
		indx = scalemax;
	      switch (bpp) {
		case 8:
		  *image.b++ = toscreen? pixels[indx]: indx;
		  break;
		case 16:
		  *image.w++ = toscreen? pixels[indx]: indx;
		  break;
		case 32:
		  *image.l++ = toscreen? pixels[indx]: indx;
		  break;
	      }	/* end of switch (bpp) */
	    } /* end of for (ix) */
	  break;
	case LUX_DOUBLE:
	  for (iy = nyy - 1; iy >= 0; iy--)
	    for (ix = 0; ix < nxx; ix++) {
	      xsrc = x1 + ix/magx;
	      ysrc = y1 + iy/magy;
	      indx = (data.d[xsrc + ysrc*nx] - offset.d)*factor.d
		+ scalemin;
	      if (indx < scalemin)
		indx = scalemin;
	      else if (indx > scalemax)
		indx = scalemax;
	      switch (bpp) {
		case 8:
		  *image.b++ = toscreen? pixels[indx]: indx;
		  break;
		case 16:
		  *image.w++ = toscreen? pixels[indx]: indx;
		  break;
		case 32:
		  *image.l++ = toscreen? pixels[indx]: indx;
		  break;
	      }	/* end of switch (bpp) */
	    } /* end of for (ix) */
	  break;
      }	/* end of if (mode & TV_24) else if (threeColors) else switch (type) */
    } /* end of if (setup & 4) else */
  } /* end of if (!(mode & (TV_RAW | TV_MAP))) */
  else if (mode & TV_MAP) {	/* must map to raw pixel values */
    if (setup & 4) {
      if (mode & TV_24) {
	for (iy = 0; iy < nyy; iy++)
	  for (ix = 0; ix < nxx; ix++) {
	    xsrc = x1 + ix/magx;
	    ysrc = y1 + iy/magy;
	    indx = data.b[xsrc + ysrc*nx];
	    if (indx < scalemin)
	      indx = scalemin;
	    else if (indx > scalemax)
	      indx = scalemax;
	    switch (bpp) {
	      case 8:
		*image.b = (pixels[indx] & red_mask);
		break;
	      case 16:
		*image.w = (pixels[indx] & red_mask);
		break;
	      case 32:
		*image.l = (pixels[indx] & red_mask);
		break;
	    } /* end of switch (bpp) */
	    indx = data.b[xsrc + ysrc*nx + s];
	    if (indx < scalemin)
	      indx = scalemin;
	    else if (indx > scalemax)
	      indx = scalemax;
	    switch (bpp) {
	      case 8:
		*image.b |= (pixels[indx] & green_mask);
		break;
	      case 16:
		*image.w |= (pixels[indx] & green_mask);
		break;
	      case 32:
		*image.l |= (pixels[indx] & green_mask);
		break;
	    } /* end of switch (bpp) */
	    indx = data.b[xsrc + ysrc*nx + 2*s];
	    if (indx < scalemin)
	      indx = scalemin;
	    else if (indx > scalemax)
	      indx = scalemax;
	    switch (bpp) {
	      case 8:
		*image.b++ = (pixels[indx] & blue_mask);
		break;
	      case 16:
		*image.w++ = (pixels[indx] & blue_mask);
		break;
	      case 32:
		*image.l++ = (pixels[indx] & blue_mask);
		break;
	    } /* end of switch (bpp) */
	  } /* end of for (ix) */
      }	/* end of if (mode & TV_24) */
      else {
	for (iy = 0; iy < nyy; iy++)
	  for (ix = 0; ix < nxx; ix++) {
	    xsrc = x1 + ix/magx;
	    ysrc = y1 + iy/magy;
	    indx = data.b[xsrc + ysrc*nx];
	    if (indx < scalemin)
	      indx = scalemin;
	    else if (indx > scalemax)
	      indx = scalemax;
	    switch (bpp) {
	      case 8:
		*image.b++ = toscreen? pixels[indx]: indx;
		break;
	      case 16:
		*image.w++ = toscreen? pixels[indx]: indx;
		break;
	      case 32:
		*image.l++ = toscreen? pixels[indx]: indx;
		break;
	    } /* end of switch (bpp) */
	  } /* end of for (ix) */
      }	/* end of if (mode & TV_24) else */
    } /* end of if (setup & 4) */
    else {
      if (mode & TV_24) {
	for (iy = nyy - 1; iy >= 0; iy--)
	  for (ix = 0; ix < nxx; ix++) {
	    xsrc = x1 + ix/magx;
	    ysrc = y1 + iy/magy;
	    indx = data.b[xsrc + ysrc*nx];
	    if (indx < scalemin)
	      indx = scalemin;
	    else if (indx > scalemax)
	      indx = scalemax;
	    switch (bpp) {
	      case 8:
		*image.b = (pixels[indx] & red_mask);
		break;
	      case 16:
		*image.w = (pixels[indx] & red_mask);
		break;
	      case 32:
		*image.l = (pixels[indx] & red_mask);
		break;
	    } /* end of switch (bpp) */
	    indx = data.b[xsrc + ysrc*nx + s];
	    if (indx < scalemin)
	      indx = scalemin;
	    else if (indx > scalemax)
	      indx = scalemax;
	    switch (bpp) {
	      case 8:
		*image.b |= (pixels[indx] & green_mask);
		break;
	      case 16:
		*image.w |= (pixels[indx] & green_mask);
		break;
	      case 32:
		*image.l |= (pixels[indx] & green_mask);
		break;
	    } /* end of switch (bpp) */
	    indx = data.b[xsrc + ysrc*nx + 2*s];
	    if (indx < scalemin)
	      indx = scalemin;
	    else if (indx > scalemax)
	      indx = scalemax;
	    switch (bpp) {
	      case 8:
		*image.b++ |= (pixels[indx] & blue_mask);
		break;
	      case 16:
		*image.w++ |= (pixels[indx] & blue_mask);
		break;
	      case 32:
		*image.l++ |= (pixels[indx] & blue_mask);
		break;
	    } /* end of switch (bpp) */
	  } /* end of for (ix) */
      }	/* end of if (mode & TV_24) */
      else {
	for (iy = nyy - 1; iy >= 0; iy--)
	  for (ix = 0; ix < nxx; ix++) {
	    xsrc = x1 + ix/magx;
	    ysrc = y1 + iy/magy;
	    indx = data.b[xsrc + ysrc*nx];
	    if (indx < scalemin)
	      indx = scalemin;
	    else if (indx > scalemax)
	      indx = scalemax;
	    switch (bpp) {
	      case 8:
		*image.b++ = toscreen? pixels[indx]: indx;
		break;
	      case 16:
		*image.w++ = toscreen? pixels[indx]: indx;
		break;
	      case 32:
		*image.l++ = toscreen? pixels[indx]: indx;
	      break;
	    } /* end of switch (bpp) */
	  } /* end of for (ix) */
      }	/* end of if (mode & TV_24) else */
    } /* end of if (setup & 4) else */
  } /* end of if (!(mode & (TV_RAW | TV_MAP))) else if (mode & TV_MAP) */
  else {			/* must extract raw pixel values */
    if (setup & 4) {
      if (mode & TV_24) {
	for (iy = 0; iy < nyy; iy++)
	  for (ix = 0; ix < nxx; ix++) {
	    xsrc = x1 + ix/magx;
	    ysrc = y1 + iy/magy;
	    switch (bpp) {
	      case 8:
		*image.b++ = (data.b[xsrc + ysrc*nx] & red_mask)
		  | (data.b[xsrc + ysrc*nx + s] & green_mask)
		  | (data.b[xsrc + ysrc*nx + 2*s] & blue_mask);
		break;
	      case 16:
		*image.w++ = (data.w[xsrc + ysrc*nx] & red_mask)
		  | (data.w[xsrc + ysrc*nx + s] & green_mask)
		  | (data.w[xsrc + ysrc*nx + 2*s] & blue_mask);
		break;
	      case 32:
		*image.l++ = (data.l[xsrc + ysrc*nx] & red_mask)
		  | (data.l[xsrc + ysrc*nx + s] & green_mask)
		  | (data.l[xsrc + ysrc*nx + 2*s] & blue_mask);
		break;
	    } /* end of switch (bpp) */
	  } /* end of for (ix) */
      }	/* end of if (mode & TV_24) */
      else {
	for (iy = 0; iy < nyy; iy++)
	  for (ix = 0; ix < nxx; ix++) {
	    xsrc = x1 + ix/magx;
	    ysrc = y1 + iy/magy;
	    switch (bpp) {
	      case 8:
		*image.b++ = data.b[xsrc + ysrc*nx];
		break;
	      case 16:
		*image.w++ = data.w[xsrc + ysrc*nx];
		break;
	      case 32:
		*image.l++ = data.l[xsrc + ysrc*nx];
		break;
	    } /* end of switch (bpp) */
	  } /* end of for (ix) */
      }	/* end of if (mode & TV_24) else */
    } /* end of if (setup & 4) */
    else {
      if (mode & TV_24) {
	for (iy = nyy - 1; iy >= 0; iy--)
	  for (ix = 0; ix < nxx; ix++) {
	    xsrc = x1 + ix/magx;
	    ysrc = y1 + iy/magy;
	    switch (bpp) {
	      case 8:
		*image.b++ = (data.b[xsrc + ysrc*nx] & red_mask)
		  | (data.b[xsrc + ysrc*nx + s] & green_mask)
		  | (data.b[xsrc + ysrc*nx + 2*s] & blue_mask);
		break;
	      case 16:
		*image.w++ = (data.w[xsrc + ysrc*nx] & red_mask)
		  | (data.w[xsrc + ysrc*nx + s] & green_mask)
		  | (data.w[xsrc + ysrc*nx + 2*s] & blue_mask);
		break;
	      case 32:
		*image.l++ = (data.l[xsrc + ysrc*nx] & red_mask)
		  | (data.l[xsrc + ysrc*nx + s] & green_mask)
		  | (data.l[xsrc + ysrc*nx + 2*s] & blue_mask);
		break;
	    } /* end of switch (bpp) */
	  } /* end of for (ix) */
      }	/* end of if (mode & TV_24) */
      else {
	for (iy = nyy - 1; iy >= 0; iy--)
	  for (ix = 0; ix < nxx; ix++) {
	    xsrc = x1 + ix/magx;
	    ysrc = y1 + iy/magy;
	    switch (bpp) {
	      case 8:
		*image.b++ = data.b[xsrc + ysrc*nx];
		break;
	      case 16:
		*image.w++ = data.w[xsrc + ysrc*nx];
		break;
	      case 32:
		*image.l++ = data.l[xsrc + ysrc*nx];
		break;
	    } /* end of switch (bpp) */
	  } /* end of for (ix) */
      } /* end of if (mode & TV_24) else */
    } /* end of if (setup & 4) else */
  } /* end of if (!(mode & (TV_RAW | TV_MAP))) else if (mode & TV_MAP) else */
	
  if (toscreen) {
    /* create image structure */
    xi = XCreateImage(display, visual, depth, ZPixmap, 0, (char *) image0.b,
		      nxx, nyy, bpp, 0);
/*    if (!XInitImage(xi))
      return luxerror("Error initializing image", 0); */
    if (wid >= 0)		/* window */
      XPutImage(display, win[wid], gc[wid], xi, 0, 0, sx, sy, nxx, nyy);
    else			/* pixmap */
      XPutImage(display, maps[-wid], gcmap[-wid], xi, 0, 0, sx, sy, nxx, nyy);
    /* now dealloc the image structure but not the data */
    xi->data = NULL;
    XDestroyImage(xi);
    XFlush(display);
    iq = LUX_OK;
  } else {			/* to postscript */
    /* in older versions of LUX, when DVI coordinates were used to specify */
    /* positions in a postscript figure, they were exported to the file */
    /* without any additional transformations.  This meant that if the */
    /* LUX window whose contents you were trying to copy to a postscript */
    /* file was not square, then the postscript figure would have an aspect */
    /* ratio different from what you saw in the LUX window, and if you */
    /* tried to combine images with lines, then the images would retain */
    /* their original aspect ratio while the line objects did not.  Here */
    /* we try to work things so that you get the same aspect ratio on */
    /* screen as on paper.  LS 23mar98 */
    maxdim = MAX(hq, wq);
    fx = (float) sxf/maxdim;
    fy = (float) syf/maxdim;
    fx2 = (float) (sxf + nxxf)/maxdim;
    fy2 = (float) (syf + nyyf)/maxdim;
    /* NOTE: must modify this to support 16 and 32-bit color indices */
    if (mode & TV_24)
      iq = postcolor((void *) image0.b, nx, ny, fx, fx2, fy, fy2, 2);
    else
      iq = postgray((char *) image0.b, nx, ny, fx, fx2, fy, fy2, 2);
  }

  free(image0.b);

  return iq;
}
 /*------------------------------------------------------------------------*/
int32_t lux_colorpixel(int32_t narg, int32_t ps[])
/* maps color indices to pixel values */
{
  pointer	p, q;
  int32_t	n, iq, bpp, type;
  extern int32_t	bits_per_pixel;
  extern unsigned long	red_mask, green_mask, blue_mask;

  bpp = bits_per_pixel? bits_per_pixel: 
    ((internalMode & TV_24)? 24: 8); /* bits_per_pixel is zero if we haven't
					yet connected to the X server (e.g.,
					tv,x,/post) */
  switch (bpp) {
    case 8:
      type = LUX_INT8;
      break;
    case 16:
      type = LUX_INT16;
      break;
    case 24:
      type = LUX_INT32;
      break;
    default:
      return luxerror("Illegal bpp in lux_colorpixel()", 0);
  }

  switch (symbol_class(*ps)) {
    case LUX_ARRAY:
      n = array_size(*ps);
      if (internalMode & TV_24) {
	if (n % 3)
	  return luxerror("Need multiple of 3 elements for 24-bit color treatment", *ps);
	n /= 3;
	if (array_dims(*ps)[array_num_dims(*ps) - 1] == 3)
	  iq = array_scratch(type, array_num_dims(*ps) - 1, array_dims(*ps));
	else
	  iq = array_scratch(type, 1, &n);
      } else {
	if (isFreeTemp(*ps) && array_type(*ps) == type)
	  iq = *ps;		/* use input as output */
	else			/* need a new variable */
	  iq = array_clone(*ps, type);
      }
      p.l = array_data(*ps);
      q.b = array_data(iq);
      break;
    case LUX_SCALAR:
      if (internalMode & TV_24)
	return luxerror("Need multiple of 3 elements for 24-bit color treatment",
		     *ps);
      n = 1;
      p.l = &scalar_value(*ps).l;
      iq = scalar_scratch(type);
      q.b = &scalar_value(iq).b;
      break;
    default:
      return cerror(ILL_CLASS, *ps);
  }

  switch (bpp) {
    case 8:
      if (internalMode & TV_24)
	switch (symbol_type(*ps)) {
	  case LUX_INT8:
	    while (n--)
	      *q.b++ = (pixels[(int32_t) *p.b] & red_mask)
		| (pixels[(int32_t) p.b[n]] & green_mask)
		| (pixels[(int32_t) p.b[2*n]] & blue_mask);
	    break;
	  case LUX_INT16:
	    while (n--)
	      *q.b++ = (pixels[(int32_t) *p.w] & red_mask)
		| (pixels[(int32_t) p.w[n]] & green_mask)
		| (pixels[(int32_t) p.w[2*n]] & blue_mask);
	    break;
	  case LUX_INT32:
	    while (n--)
	      *q.b++ = (pixels[(int32_t) *p.l] & red_mask)
		| (pixels[(int32_t) p.l[n]] & green_mask)
		| (pixels[(int32_t) p.l[2*n]] & blue_mask);
	    break;
	  case LUX_INT64:
	    while (n--)
	      *q.b++ = (pixels[(int32_t) *p.q] & red_mask)
		| (pixels[(int32_t) p.q[n]] & green_mask)
		| (pixels[(int32_t) p.q[2*n]] & blue_mask);
	    break;
	  case LUX_FLOAT:
	    while (n--)
	      *q.b++ = (pixels[(int32_t) *p.f] & red_mask)
		| (pixels[(int32_t) p.f[n]] & green_mask)
		| (pixels[(int32_t) p.f[2*n]] & blue_mask);
	    break;
	  case LUX_DOUBLE:
	    while (n--)
	      *q.b++ = (pixels[(int32_t) *p.d] & red_mask)
		| (pixels[(int32_t) p.d[n]] & green_mask)
		| (pixels[(int32_t) p.d[2*n]] & blue_mask);
	    break;
	} else switch (symbol_type(*ps)) {
	  case LUX_INT8:
	    while (n--)
	      *q.b++ = pixels[(int32_t) *p.b++];
	    break;
	  case LUX_INT16:
	    while (n--)
	      *q.b++ = pixels[(int32_t) ((uint8_t) *p.w++)];
	    break;
	  case LUX_INT32:
	    while (n--)
	      *q.b++ = pixels[(int32_t) ((uint8_t) *p.l++)];
	    break;
          case LUX_INT64:
	    while (n--)
	      *q.b++ = pixels[(int32_t) ((uint8_t) *p.q++)];
	    break;
	  case LUX_FLOAT:
	    while (n--)
	      *q.b++ = pixels[(int32_t) ((uint8_t) *p.f++)];
	    break;
	  case LUX_DOUBLE:
	    while (n--)
	      *q.b++ = pixels[(int32_t) ((uint8_t) *p.d++)];
	    break;
	}
      break;
    case 16:
      if (internalMode & TV_24)
	switch (symbol_type(*ps)) {
	  case LUX_INT8:
	    while (n--)
	      *q.w++ = (pixels[(int32_t) *p.b] & red_mask)
		| (pixels[(int32_t) p.b[n]] & green_mask)
		| (pixels[(int32_t) p.b[2*n]] & blue_mask);
	    break;
	  case LUX_INT16:
	    while (n--)
	      *q.w++ = (pixels[(int32_t) *p.w] & red_mask)
		| (pixels[(int32_t) p.w[n]] & green_mask)
		| (pixels[(int32_t) p.w[2*n]] & blue_mask);
	    break;
	  case LUX_INT32:
	    while (n--)
	      *q.w++ = (pixels[(int32_t) *p.l] & red_mask)
		| (pixels[(int32_t) p.l[n]] & green_mask)
		| (pixels[(int32_t) p.l[2*n]] & blue_mask);
	    break;
	  case LUX_INT64:
	    while (n--)
	      *q.w++ = (pixels[(int32_t) *p.q] & red_mask)
		| (pixels[(int32_t) p.q[n]] & green_mask)
		| (pixels[(int32_t) p.q[2*n]] & blue_mask);
	    break;
	  case LUX_FLOAT:
	    while (n--)
	      *q.w++ = (pixels[(int32_t) *p.f] & red_mask)
		| (pixels[(int32_t) p.f[n]] & green_mask)
		| (pixels[(int32_t) p.f[2*n]] & blue_mask);
	    break;
	  case LUX_DOUBLE:
	    while (n--)
	      *q.w++ = (pixels[(int32_t) *p.d] & red_mask)
		| (pixels[(int32_t) p.d[n]] & green_mask)
		| (pixels[(int32_t) p.d[2*n]] & blue_mask);
	    break;
	} else switch (symbol_type(*ps)) {
	  case LUX_INT8:
	    while (n--)
	      *q.w++ = pixels[(int32_t) *p.b++];
	    break;
	  case LUX_INT16:
	    while (n--)
	      *q.w++ = pixels[(int32_t) ((uint8_t) *p.w++)];
	    break;
	  case LUX_INT32:
	    while (n--)
	      *q.w++ = pixels[(int32_t) ((uint8_t) *p.l++)];
	    break;
	  case LUX_INT64:
	    while (n--)
	      *q.w++ = pixels[(int32_t) ((uint8_t) *p.q++)];
	    break;
	  case LUX_FLOAT:
	    while (n--)
	      *q.w++ = pixels[(int32_t) ((uint8_t) *p.f++)];
	    break;
	  case LUX_DOUBLE:
	    while (n--)
	      *q.w++ = pixels[(int32_t) ((uint8_t) *p.d++)];
	    break;
	}
      break;
    case 24:
      if (internalMode & TV_24)
	switch (symbol_type(*ps)) {
	  case LUX_INT8:
	    while (n--)
	      *q.l++ = (pixels[(int32_t) *p.b] & red_mask)
		| (pixels[(int32_t) p.b[n]] & green_mask)
		| (pixels[(int32_t) p.b[2*n]] & blue_mask);
	    break;
	  case LUX_INT16:
	    while (n--)
	      *q.l++ = (pixels[(int32_t) *p.w] & red_mask)
		| (pixels[(int32_t) p.w[n]] & green_mask)
		| (pixels[(int32_t) p.w[2*n]] & blue_mask);
	    break;
	  case LUX_INT32:
	    while (n--)
	      *q.l++ = (pixels[(int32_t) *p.l] & red_mask)
		| (pixels[(int32_t) p.l[n]] & green_mask)
		| (pixels[(int32_t) p.l[2*n]] & blue_mask);
	    break;
	  case LUX_INT64:
	    while (n--)
	      *q.l++ = (pixels[(int32_t) *p.q] & red_mask)
		| (pixels[(int32_t) p.q[n]] & green_mask)
		| (pixels[(int32_t) p.q[2*n]] & blue_mask);
	    break;
	  case LUX_FLOAT:
	    while (n--)
	      *q.l++ = (pixels[(int32_t) *p.f] & red_mask)
		| (pixels[(int32_t) p.f[n]] & green_mask)
		| (pixels[(int32_t) p.f[2*n]] & blue_mask);
	    break;
	  case LUX_DOUBLE:
	    while (n--)
	      *q.l++ = (pixels[(int32_t) *p.d] & red_mask)
		| (pixels[(int32_t) p.d[n]] & green_mask)
		| (pixels[(int32_t) p.d[2*n]] & blue_mask);
	    break;
	} else switch (symbol_type(*ps)) {
	  case LUX_INT8:
	    while (n--)
	      *q.l++ = pixels[(int32_t) *p.b++];
	    break;
	  case LUX_INT16:
	    while (n--)
	      *q.l++ = pixels[(int32_t) ((uint8_t) *p.w++)];
	    break;
	  case LUX_INT32:
	    while (n--)
	      *q.l++ = pixels[(int32_t) ((uint8_t) *p.l++)];
	    break;
	  case LUX_INT64:
	    while (n--)
	      *q.l++ = pixels[(int32_t) ((uint8_t) *p.q++)];
	    break;
	  case LUX_FLOAT:
	    while (n--)
	      *q.l++ = pixels[(int32_t) ((uint8_t) *p.f++)];
	    break;
	  case LUX_DOUBLE:
	    while (n--)
	      *q.l++ = pixels[(int32_t) ((uint8_t) *p.d++)];
	    break;
	}
      break;
  }
  return iq;
}
 /*------------------------------------------------------------------------*/
int32_t lux_xtv(int32_t narg, int32_t ps[])
/* displays an image, properly colored for the current color map, on screen.
   LS 18jan94 */
{
  return lux_xtv_general(narg, ps, (narg && symbol_type(ps[0]) == LUX_INT8)? TV_MAP: 0);
}
 /*------------------------------------------------------------------------*/
int32_t lux_xtvmap(int32_t narg, int32_t ps[])
/* displays an image of color indices, mapped to proper pixel values,
  on screen.  LS 18jan94 */
{
  return lux_xtv_general(narg, ps, TV_MAP);
}
 /*------------------------------------------------------------------------*/
int32_t lux_xcopy(int32_t narg, int32_t ps[])
 /* 1/8/92 modified to treat negative numbers as pixmaps and 0 and >0 as
   displayed windows */
 /* needs lots of checking which isn't implemented yet */
 /* also just assumes 512x512 for testing */
 {
 int32_t     id1, id2, ixs, iys, ixd, iyd, w, h, ws, hs;
 Drawable        *src, *dest;
 GC      *cgc;

 ck_events();
 id1 = int_arg( ps[0] );		/* source window */
 id2 = int_arg( ps[1] );		/* destination window */
 ixs = iys = ixd = iyd = 0;
 if (ck_window(id1) != 1) return -1;
 if (ck_window(id2) != 1) return -1;
 if (id1 < 0 ) { src = &(maps[-id1]);
	 ws = wdmap[-id1];  hs = htmap[-id1]; }
	 else { src = &(win[id1]);
	 ws = wd[id1];  hs = ht[id1]; }  
 if (id2 < 0 ) { dest = &(maps[-id2]); cgc = &gcmap[-id2]; }
	 else { dest = &(win[id2]); cgc = &gc[id2]; }
 w = ws; h = hs;
 if (narg > 2) ixs = int_arg( ps[2] );	/* source x1 */
 if (narg > 3) iys = int_arg( ps[3] );	/* source y1 */
 if (narg > 4) w = int_arg( ps[4] );	/* source width */
 if (narg > 5) h = int_arg( ps[5] );	/* source height */
 if (narg > 6) ixd = int_arg( ps[6] );	/* destination x1 */
 if (narg > 7) iyd = int_arg( ps[7] );	/* destination y1 */
 if ( src == NULL ) { printf("source drawable not defined\n");
		 return -1; }
 if ( dest == NULL ) { printf("dest drawable not defined\n");
		 return -1; }
 /* make w and h fit both */
 if (!ck_area(id1, &ixs, &iys, &w, &h))
 { puts("xcopy - source area completely out of window"); return 1; }
 if (!ck_area(id2, &ixd, &iyd, &w, &h))
 { puts("xcopy - destination area completely out of window"); return 1; }
 XCopyArea( display, *src, *dest, *cgc, ixs, iys, w, h, ixd, iyd);
 XFlush(display);
 return 1;
 }
 /*------------------------------------------------------------------------*/
int32_t lux_xevent(int32_t narg, int32_t ps[])
{
 XEvent  report;
 int32_t     nev, i;

 XFlush(display);
 nev = XPending( display);
 printf("number of events = %d\n",nev);
 if (nev <= 0 ) return 1;
 for (i=0;i<nev;i++) {
 XNextEvent(display, &report);
 printf("report.type = %d\n",report.type);
 switch (report.type) {
  case ButtonPress:
 printf("button down, window = %ld\n", report.xbutton.window);
 printf("x and y = %d %d\n",report.xbutton.x,report.xbutton.y);
 printf("button # %d\n", report.xbutton.button);
 printf("time = %x\n", (uint32_t) report.xbutton.time);
 break;
  case ButtonRelease:
 printf("button up, window = %ld\n", report.xbutton.window);
 printf("x and y = %d %d\n",report.xbutton.x,report.xbutton.y);
 printf("button # %d\n", report.xbutton.button);
 printf("time = %x\n", (uint32_t) report.xbutton.time);
 break;
  case KeyPress:
 printf("key press, window = %ld\n", report.xkey.window);
 printf("x and y = %d %d\n",report.xkey.x,report.xkey.y);
 printf("keycode # %d\n", report.xkey.keycode);
 printf("time = %x\n", (uint32_t) report.xkey.time);
 break;
  case FocusIn:
 printf("focus in, window = %ld\n", report.xfocus.window);
 printf("type, mode, detail = %d %d %d\n",report.xfocus.type,report.xfocus.mode,
  report.xfocus.detail);
 break;
  case FocusOut:
 printf("focus out, window = %ld\n", report.xfocus.window);
 printf("type, mode, detail = %d %d %d\n",report.xfocus.type,report.xfocus.mode,
  report.xfocus.detail);
 break;
  case ConfigureNotify:
 printf("ConfigureNotify, window = %ld\n", report.xconfigure.window);
 printf("x and y = %d %d\n",report.xconfigure.x,report.xconfigure.y);
 printf("w and h = %d %d\n",report.xconfigure.width,report.xconfigure.height);
 break;
  default:
 printf("an event not in current list\n");
 break;
 }
 }
 return 1;
 }
 /*------------------------------------------------------------------------*/
int32_t lux_xpurge(narg,ps)	/* just throw away any pending X events */
 int32_t narg, ps[];
 {
 XEvent  report;
 int32_t     nev, i;

 XFlush(display);
 nev = XPending( display);
 for (i=0;i<nev;i++) {
 XNextEvent(display, &report);
 }
 return 1;
 }
 /*------------------------------------------------------------------------*/
int32_t lux_xplace(int32_t narg, int32_t ps[])
 /* response to a key or button press in an lux window and note the
 time and position */
{
 int32_t	i, cs, nc;
 KeySym	keysym;
 char	buffer[16];
 int32_t	coordTrf(float *, float *, int32_t, int32_t);
 extern int32_t	lux_keysym, lux_keystate;

 XEvent  report;

 if (ck_events() != LUX_OK)
   return LUX_ERROR;
	/* wait for a button or key press */
	/* changed to a loop because may catch event for an LUX menu */
	/* LS 10jun93 */
 do {
   XMaskEvent(display, KeyPressMask | ButtonPressMask, &report);
	/* which window ? */
   last_wid = -1;
   for (i = 0; i < MAXWINDOWS; i++)
     if (win[i] == report.xbutton.window) {
       last_wid = i;
       break;
     }
 } while (last_wid == -1);
	 /* preset key and button to 0, only one of these will be set */
 lux_keycode = lux_keysym = lux_button = 0;
 xhair = report.xbutton.x;
 yhair = report.xbutton.y;
 coordTrf(&xhair, &yhair, LUX_X11, LUX_DEV);
 xcoord = (int32_t) xhair;
 ycoord = (int32_t) yhair;
 cs = (internalMode & 7);
 if (!cs)
   cs = LUX_DVI;
 coordTrf(&xhair, &yhair, LUX_DEV, cs);
 last_time = (double) report.xbutton.time / 1000.0; /* time in seconds */
 switch (report.type) {
 case ButtonPress:
   lux_button = report.xbutton.button;
   lux_keystate = report.xbutton.state;
   break;
 case KeyPress:	/* keycode translation LS 1jun93 */
   lux_keycode = report.xkey.keycode;
   lux_keystate = report.xkey.state;
   nc = XLookupString(&(report.xkey), buffer, 15, &keysym, NULL);
   buffer[nc] = '\0';
   lux_keysym = (int32_t) keysym;
   break;
 }
	/* return parameters if they were arguments */
 switch (narg) {
 case 2:
   if (redef_scalar(ps[1], LUX_INT32, &ycoord) != 1)
     return cerror(ALLOC_ERR, ps[1]);
 case 1:
   if (redef_scalar(ps[0], LUX_INT32, &xcoord) != 1)
     return cerror(ALLOC_ERR, ps[0]);
 }
 return 1;
}
 /*------------------------------------------------------------------------*/
int32_t xwindow_plot(int32_t ix, int32_t iy, int32_t mode)
/* drawing on X-window */
/* added pixmap case - LS 8oct97 */
/* added alternateDash - LS 16oct98 */
{
 int32_t     wid, lux_xpen(int32_t, float);
 extern int32_t	alternateDash, current_pen;
 extern float	current_gray;

 wid = last_wid;
 if ((wid >= 0 && !win[wid]) || (wid < 0 && !maps[-wid]))
   lux_xcreat(wid, 512, 512, 0, 0, 0, NULL, NULL); 

 if (mode == 0) {
   if (alternateDash)
     lux_xpen(current_pen, (float) 1 - current_gray);
   else {
     xold = ix;
     yold = iy;
     return 1;
   }
 }

 if (wid >= 0)			/* window */
   XDrawLine(display, win[wid], gc[wid], xold, yold, ix, iy);
 else				/* pixmap */
   XDrawLine(display, maps[-wid], gcmap[-wid], xold, yold, ix, iy);
 if (!mode)			/* we also have alternateDash */
   lux_xpen(current_pen, current_gray);
 xold = ix;
 yold = iy;
 XFlush(display);
 return 1;
}
 /*------------------------------------------------------------------------*/
int32_t lux_xflush()
 {
 XFlush(display);       return 1;
 }
 /*------------------------------------------------------------------------*/
int32_t lux_xpen(int32_t pen, float gray)
{
  int32_t	wid;
  extern int32_t	standardGray;
  XColor	seek_color, *return_color;
  XColor	*anaFindBestRGB(XColor *, int32_t);

  /* lines on screen with a given non-unity pen width appear much
     fatter than the corresponding lines on paper, so reduce screen
     pen width. */
/* LS 12jul94 */
 if (pen)
   pen /= 2; 
 ck_events();
 wid = last_wid;
 /* does window exist? If not create a default size */
 if ((wid >= 0 && !win[wid]) || (wid < 0 && !maps[-wid]))
   lux_xcreat(wid, 512, 512, 0, 0, 0, NULL, NULL);
 XSetLineAttributes(display, gc[wid], pen, LineSolid, CapRound,
		    JoinRound);
 if (gray < 0)
   gray = 0;
 else if (gray > 1)
   gray = 1;

 if ((setup & WHITE_BACKGR) && !standardGray)
   gray = 1 - gray;
 
 seek_color.red = seek_color.green = seek_color.blue = gray*65535;
 return_color = anaFindBestRGB(&seek_color, 1);
 XSetForeground(display, gc[wid], return_color->pixel);
 return 1;
}
 /*------------------------------------------------------------------------*/
int32_t lux_xfont(narg, ps)
                                        /* set font for a window */
 int32_t     narg, ps[];
 {
 int32_t      wid, iq;
 char    *fontname;
 /* first arg is a string which should be a font name */
 ck_events();
 iq = ps[0];
 if (sym[iq].class != 2) return cerror(NEED_STR, iq);
 fontname = (char *) sym[iq].spec.array.ptr;
 wid = last_wid;
 if (narg > 1) wid = int_arg( ps[1]);
 /* does window exist? If not create a default size */
 if ( win[wid] == 0 ) lux_xcreat(wid, 512, 512, 0, 0, 0, NULL, NULL);
 if ( (font_info[wid] = XLoadQueryFont(display, fontname)) == NULL)
  { printf("font does not exist: %s\n", fontname);  return -1; }
 XSetFont( display, gc[wid], font_info[wid]->fid);
 return 1;
 }
 /*------------------------------------------------------------------------*/
int32_t lux_xlabel(narg, ps)
                                        /* user labels */
 int32_t     narg, ps[];
 {
 int32_t     wid, iq, ix, iy, len;
 char    *s;
 /* first arg is a string which will be the label */
 ck_events();
 iq = ps[0];
 if (sym[iq].class != 2) return cerror(NEED_STR, iq);
 s = (char *) sym[iq].spec.array.ptr;
 len = sym[iq].spec.array.bstore - 1;
 ix = int_arg( ps[1]);   iy = int_arg( ps[2]);
 wid = last_wid;
 if (narg > 3) { wid = int_arg( ps[3]);  last_wid = wid; }
 /* does window exist? If not create a default size */
 if ( win[wid] == 0 ) lux_xcreat(wid, 512, 512, 0, 0, 0, NULL, NULL);
 XDrawString( display, win[wid], gc[wid], ix, iy, s, len);
 XFlush(display);
 return 1;
 }
 /*------------------------------------------------------------------------*/
int32_t lux_xlabelwidth(narg, ps)
                                        /* user label width */
 int32_t     narg, ps[];
 {
 int32_t     wid, iq, len, result_sym;
 char    *s;
 /* arg is a string which will be the label */
 ck_events();
 iq = ps[0];
 if (sym[iq].class != 2) return cerror(NEED_STR, iq);
 s = (char *) sym[iq].spec.array.ptr;
 len = sym[iq].spec.array.bstore - 1;
 wid = last_wid;
 iq = XTextWidth(font_info[wid], s, len );
 result_sym = scalar_scratch(2);
 sym[result_sym].spec.scalar.l = iq;
 return result_sym;
 }
 /*------------------------------------------------------------------------*/
int32_t xlabel(char *s, int32_t ix, int32_t iy)
                                        /* internal call for labels */
{
  int32_t     len, wid;

  len = strlen(s);
  wid = last_wid;
  XDrawString( display, win[wid], gc[wid], ix, iy, s, len);
  return 1;
}
 /*------------------------------------------------------------------------*/
int32_t xlabelwidth(s)
                                        /* internal call for labels */
 char    *s;
 {
 int32_t     len, wid;
 len = strlen(s);
 wid = last_wid;
 /* does window exist? If not create a default size */
 if ( win[wid] == 0 ) lux_xcreat(wid, 512, 512, 0, 0, 0, NULL, NULL);
 return  XTextWidth(font_info[wid], s, len );
 }
 /*------------------------------------------------------------------------*/
int32_t lux_xtvread(int32_t narg, int32_t ps[])
{
 /* read from an X window or pixmap and create a uint8_t array */
 int32_t  nx, ny, ix, iy, wid, result_sym, dim[2], w, hh, type;
 pointer    ptr;
 Drawable        *src;
 int32_t	lux_zerof(int32_t, int32_t *);
 extern int32_t	bits_per_pixel;
 int32_t lux_colorstogrey(int32_t, int32_t []);
 int32_t lux_delete(int32_t, int32_t []);

 /* the input arguments are all scalars */
 ck_events();
 wid = last_wid;
 ix = iy = 0;
 if (narg == 1)
   wid = int_arg(ps[0]);
 else if (narg > 4)
   wid = int_arg(ps[4]);
 if (wid >= 0) {		/* window */
   nx = wd[wid];
   ny = ht[wid];
 } else {			/* pixmap */
   nx = wdmap[-wid];
   ny = htmap[-wid];
 }
 if (narg >= 2) {
   ix = int_arg(ps[0]);
   iy = int_arg(ps[1]);
 }
 if (narg >= 4) {
   nx = int_arg(ps[2]);
   ny = int_arg(ps[3]);
 }
 /* check this window's size and decide whether real or mapped */
 if (wid < 0) {			/* pixmap */
   src = &(maps[-wid]);
   w = wdmap[-wid];
   hh = htmap[-wid];
 } else {			/* window */
   src = &(win[wid]);
   w = wd[wid];
   hh = ht[wid];
 }
 if (*src == 0)
   return luxerror("non-existent window in tvread: %d", 0, wid);
 nx = MIN(nx, w - ix);
 ny = MIN(ny, hh - iy);
 if (nx < 0 || ny < 0)
   return luxerror("xtvread - off screen", 0);
 dim[0] = nx;
 dim[1] = ny;
 switch (bits_per_pixel) {
   case 8:
     type = LUX_INT8;
     break;
   case 16:
     type = LUX_INT16;
     break;
   case 32:
     type = LUX_INT32;
     break;
 }
 result_sym = array_scratch(type, 2, dim);
 result_sym = lux_zerof(1, &result_sym);  /* zero it in case not filled */
 ptr.b = array_data(result_sym);
 /* note that we create our own image and use XGetSubImage */
 xi = XCreateImage(display, visual, depth, ZPixmap, 0, (char *) ptr.b,
		   nx, ny, bits_per_pixel, 0);
 XGetSubImage(display, *src, ix, iy, nx, ny, 0xffffffffUL, ZPixmap, xi, 0, 0);
 xi->data = NULL;
 XDestroyImage(xi);
 XFlush(display);
 if (internalMode & 1) {
   int32_t lux_colorstogrey(int32_t narg, int32_t ps[]);
   int32_t lux_delete(int32_t narg, int32_t ps[]);
   if (lux_colorstogrey(1, &result_sym) != LUX_OK) {
     lux_delete(1, &result_sym);
     return LUX_ERROR;
   }
 }
 return reverseYImage(result_sym);
}
/*------------------------------------------------------------------------*/
int32_t lux_xquery(int32_t narg, int32_t ps[])
{
  int32_t	xquery(int32_t, int32_t []);

  xquery(narg, ps);
  return 1;
}
 /*------------------------------------------------------------------------*/
int32_t lux_xquery_f(int32_t narg, int32_t ps[])
{
 int32_t	result;
 int32_t	xquery(int32_t, int32_t []);

 result = scalar_scratch(LUX_INT32);
 sym[result].spec.scalar.l = xquery(narg, ps);
 return result;
}
/*------------------------------------------------------------------------*/
int32_t xquery(int32_t narg, int32_t ps[])
/* note time and position of mouse */
{
  int32_t	wid;
  Window	qroot, qchild;

  wid = narg? int_arg(ps[0]): last_wid;
  if (!win[wid] && lux_xport(1, &wid) == LUX_ERROR)
    return LUX_ERROR;
  XQueryPointer(display, win[wid], &qroot, &qchild, &root_x,
                &root_y, &xcoord, &ycoord, &kb);
  /* Note:  status seems to always be True, even if the pointer isn't
     actually in the proper window.  Fix our own: */
  if (xcoord >= 0 && ycoord >= 0 && xcoord < wd[wid] && ycoord < ht[wid])
    return 1;
  return 0;
}
/*------------------------------------------------------------------------*/
Bool windowButtonPress(Display *display, XEvent *event, XPointer arg)
/* returns True if the event is a ButtonPress in an LUX window,
   False otherwise */
{
  int32_t	i, num;
  extern Window	win[];
  
  if (event->type != ButtonPress) return False;
  if (arg) num = *((int32_t *) arg); else num = -1;
  for (i = (num < 0)? 0: num; i < (num < 0? MAXWINDOWS: num + 1); i++)
    /* if no arg, then check all windows, else just indicated one */
    if (win[i] && event->xbutton.window == win[i]) return True;
  return False;
}
/*------------------------------------------------------------------------*/
int32_t lux_check_window(int32_t narg, int32_t ps[])
     /* checks event buffer for any pending window selections */
{
  int32_t	num, w, i;
  XEvent	event;
  
  if (setup_x() < 0)
    return LUX_ERROR;
  if (narg >= 1) num = int_arg(*ps); else num = -1;
  if (XCheckIfEvent(display, &event, windowButtonPress,
		    (XPointer) ((num < 0)? NULL: &num)))
  { w = event.xbutton.window;
    for (i = 0; i < MAXWINDOWS; i++)
      if (win[i] == w) { last_wid = i;  break; }
    xcoord = event.xbutton.x;
    ycoord = event.xbutton.y;
    lux_button = event.xbutton.button;
    last_time = (double) event.xbutton.time / 1000.0;
    return 1; }
  return 4;
} 
/*------------------------------------------------------------------------*/
int32_t lux_xraise(int32_t narg, int32_t ps[]) /* raise (popup) a window */
{
  int32_t    wid;

  wid = int_arg( ps[0] );
  if (ck_window(wid) != 1)
    return -1;
  /* a no-op for a pixmap */
  if ( wid >= 0 )
  { if ( win[wid] == 0 )
      return luxerror("No such window", ps[0]);
    XRaiseWindow(display, win[wid]);
  }
  return 1;
}
/*------------------------------------------------------------------------*/
int32_t lux_xanimate(int32_t narg, int32_t ps[])
/* XANIMATE,data [, x, y, FR1=fr1, FR2=fr2, FRSTEP=frstep] [, /TIME, */
/* /REPEAT] */
{
  int32_t	wid, nx, ny, ix, iy, *dims, nFrame, i, nnx, nny, fr1, fr2, frs;
  double	ts, tc;
  struct timeval	tp;
  struct timezone	tzp;
  pointer	data;

  if (symbol_class(ps[0]) != LUX_ARRAY)
    return cerror(NEED_ARR, ps[0]);
  if (array_num_dims(ps[0]) < 3)
    return luxerror("XANIMATE needs data with at least 3 dimensions", ps[0]);
  wid = last_wid;		/* window */
  if (wid < 0)
    return luxerror("XANIMATE does not work in pixmaps", ps[0]);
  if (narg > 1 && ps[1])
    ix = int_arg(ps[1]);
  if (narg > 2 && ps[2])
    iy = int_arg(ps[2]);
  if (narg > 3 && ps[3])
    fr1 = int_arg(ps[3]);
  else
    fr1 = 0;
  dims = array_dims(ps[0]);
  nnx = nx = dims[0];
  nny = ny = dims[1];
  nFrame = array_size(ps[0])/(nx*ny);
  if (fr1 < 0 || fr1 >= nFrame - 1)
    return luxerror("Start frame number out of range: %1d vs. (0:%1d)",
		 ps[3], fr1, 0, nFrame - 2);
  data.l = array_data(ps[0]);
  ix = 0;
  iy = 0;
  if (narg > 4 && ps[4])
    fr2 = int_arg(ps[4]);
  else
    fr2 = nFrame - 1;
  if (fr2 < fr1 || fr2 >= nFrame)
    return luxerror("End frame number out of range: %1d vs. (%1d:%1d)",
		 ps[4], fr2, fr1 + 1, nFrame - 1);
  if (narg > 5 && ps[5])
    frs = int_arg(ps[5]);
  else
    frs = 1;
  if (frs < 1)
    return luxerror("Illegal frame step size: %1d", ps[5], frs);

  /* does window exist?  If not create to fit image */
  if (win[wid] == 0)
    lux_xcreat(wid, iy + ny, ix + nx, 0, 0, 0, NULL, NULL);
  if (!ck_area(wid, &ix, &iy, &nnx, &nny))
    return luxerror("XANIMATE - completely off window", 0);

  xi = XCreateImage(display, visual, 8, ZPixmap, 0, (char *) data.b, nx,
		    ny*nFrame, 8, 0);

  if (internalMode & 1)
  { gettimeofday(&tp, &tzp);
    ts = tp.tv_sec + tp.tv_usec*1e-6;
    tc = clock(); }
  for (i = fr1; i <= fr2; i += frs)
    XPutImage(display, win[wid], gc[wid], xi, 0, i*ny, ix, iy, nnx, nny);
  if (internalMode & 1)
  { gettimeofday(&tp, &tzp);
    ts = tp.tv_sec + tp.tv_usec*1e-6 - ts;
    tc = (clock() - tc)/CLOCKS_PER_SEC;
    printf("XANIMATE systime: %1.2f sec, %1.2f frames/sec, %1g px/sec\n", ts,
	   (fr2 - fr1 + 1)/ts, nnx*nny*(fr2 - fr1 + 1)/(ts*frs));
    printf("         cputime: %1.2f sec, %1.2f frames/sec, %1g px/sec\n", tc,
	   (fr2 - fr1 + 1)/(tc*frs), nnx*nny*(fr2 - fr1 + 1)/(tc*frs)); }

  xi->data = NULL;
  XDestroyImage(xi);
  XFlush(display);
  return 1;
}
/*---------------------------------------------------------*/
int32_t lux_xzoom(int32_t narg, int32_t ps[])
/* ZOOM,image [,x,y,window] */
{
  XEvent	event;
  int32_t	wid, i, type, nx, ny;
  pointer	ptr;

  if (symbol_class(ps[0]) != LUX_ARRAY)
    return cerror(NEED_ARR, ps[0]);
  ck_events();
  wid = last_wid;
  if (wid < 0)
    return luxerror("Cannot use ZOOM on pixmaps", 0);
  if (lux_xtv(narg, ps) < 0)	/* display image */
    return LUX_ERROR;		/* some error */
  ptr.l = array_data(ps[0]);
  type = array_type(ps[0]);
  nx = *array_dims(ps[0]);	/* width */
  ny = array_dims(ps[0])[1];	/* height */

  /* first remove all events pertaining to this window so we start with */
  /* a clean slate */
  while (XCheckWindowEvent(display, win[wid],
			   PointerMotionMask | ButtonPressMask, &event));
  
  while (1) {			/* loop */
    XWindowEvent(display, win[wid],
                 PointerMotionMask | ButtonPressMask, &event);
    switch (event.type) {
    case MotionNotify:
      /* remove all pointer motion events - we only want the last one */
      while (XCheckWindowEvent(display, win[wid], PointerMotionMask, &event));
      printf("\r %10d %10d ", event.xmotion.x, event.xmotion.y);
      if (event.xmotion.x < nx && event.xmotion.y < ny) {
	i = event.xmotion.x + event.xmotion.y*nx;
	switch (type) {
	case LUX_INT8:
	  printf("%10d", ptr.b[i]);
	  break;
	case LUX_INT16:
	  printf("%10d", ptr.w[i]);
	  break;
	case LUX_INT32:
	  printf("%10d", ptr.l[i]);
	  break;
	case LUX_INT64:
	  printf("%10jd", ptr.q[i]);
	  break;
	case LUX_FLOAT:
	  printf("%10f", ptr.f[i]);
	  break;
	case LUX_DOUBLE:
	  printf("%10g", ptr.d[i]);
	  break;
	}
      }
      fflush(stdout);
      break;
    case ButtonPress:
      /* done. */
      putchar('\n');
      return 1;
    }
  }
}
/*---------------------------------------------------------*/
int32_t tvplanezoom = 1;
int32_t lux_xtvplane(int32_t narg, int32_t ps[])
 /* use negative zoom factors for compression, not real fast */
 /* 9/21/96 allow 2-D arrays also but verify that plane is 0 */
 /* tvplane, cube, in, ix,iy, window
 similar to tv but extracts a 2-D image (plane) from a data cube,
 avoids an extra memory transfer, works like tv, cube(*,*,in),ix,iy window
 5/27/96 The original tvplane used ix,iy in the same way as tv but now we
 change it to mean an offset in the cube rather than one in the output
 window. Hence the output window offset is always (0,0) unless we decide
 to add more parameters. This is better adapted for scrollable windows
 where we want to fill the window but vary the part of the image displayed
 Also, drop support for pixmaps, this routine only makes sense for displayable
 windows. Also, don't set the !tvix,!tvixb stuff for this routine.
 */
{
  int32_t iq, nx, ny, nz, nd, ix=0, iy=0, wid, hq, wq, ip;
  int32_t	zoom_sym, ns, ms;
  uint8_t	*ptr, *ptr2, *ptr0;
  static uint8_t *subfree;
  int32_t zoomer2(uint8_t *, int32_t, int32_t, int32_t *, int32_t *, int32_t *, int32_t),
    zoomer3(uint8_t *, int32_t, int32_t, int32_t *, int32_t *, int32_t *, int32_t),
    zoomer4(uint8_t *, int32_t, int32_t, int32_t *, int32_t *, int32_t *, int32_t),
    zoomer8(uint8_t *, int32_t, int32_t, int32_t *, int32_t *, int32_t *, int32_t),
    zoomer16(uint8_t *, int32_t, int32_t, int32_t *, int32_t *, int32_t *, int32_t),
    compress2(uint8_t *, int32_t, int32_t, int32_t *, int32_t *, int32_t *, int32_t),
    compress4(uint8_t *, int32_t, int32_t, int32_t *, int32_t *, int32_t *, int32_t);

  if (ck_events() != 1)
    return LUX_ERROR;
  iq = ps[0];
  wid = last_wid;
  if (wid < 0)
    return luxerror("TVPLANE - pixmaps (negative window #'s) not supported", 0);
  if (symbol_class(iq) != LUX_ARRAY)
    return cerror(NEED_ARR, iq);
  nd = array_num_dims(iq);
  if ((nd != 3 && nd != 2)
      || array_type(iq) != LUX_INT8)
    return luxerror("TVPLANE - array must be 2-D or 3-D BYTE", iq);
  nx = array_dims(iq)[0];
  ny = array_dims(iq)[1];
  if (nd == 3)
    nz = array_dims(iq)[2];
  else
    nz = 1;
  ix = iy = ip = 0;
  if (narg > 1)
    ip = int_arg(ps[1]);
  if (ip >= nz || ip < 0) 
    return luxerror("TVPLANE - out of range plane (%d)", ps[1]);
  if (narg > 2)
    ix = int_arg(ps[2]);
  if (narg > 3)
    iy = int_arg(ps[3]);
  if (narg > 4)
    wid = int_arg(ps[4]);
  /* does window exist? We must have a predefined window */
  if (win[wid] == 0)
    return luxerror("TVPLANE - window must be pre-defined, %d\n", narg > 4? ps[4]: 0, wid);

  /* get pointer to the unzoomed image */
  ptr = (uint8_t *) array_data(iq) + ip*nx*ny;

  /* how much do we need ? depends on destination window and zoom */
  wq = wd[wid];
  hq = ht[wid];
  if (tvplanezoom > 1) {
    wq = wq/tvplanezoom;
    hq  = hq/tvplanezoom;
  }
  if (tvplanezoom < 0) {
    wq = wq*ABS(tvplanezoom);
    hq = hq*ABS(tvplanezoom);
  }
 /* can't be too big */
  wq = MIN(wq, nx);
  hq = MIN(hq, ny);
  /* check this range, adjust ix if necessary */
  if (ix + wq >= nx)
    ix = nx - wq;
  if (iy + hq >= ny)
    iy = ny - hq;

  /* now extract this subarea if it is a subarea*/
  if (ix != 0 || iy != 0 || nx != wq || ny != hq) {
    /* 2 obvious upgrades, if zooming, the extraction and zoom can be
       combined and the data could be re-oriented while extracting */
    uint8_t	*sub, *p;
    int32_t	m = hq, n, stride;
    
    p = ptr;
    sub = ptr = malloc(wq*hq);
    p = p + ix + iy*nx;
    stride = nx - wq;
    while (m--) {
      n = wq;
      while (n--)
	*sub++ = *p++;
      p += stride;
    }
    nx = wq;
    ny = hq;
    subfree = ptr;
  }
  /* are we zooming? */
  if (tvplanezoom != 1) {
    /* now call zoomerx to get the expanded version */
    switch (tvplanezoom) {
      case 2:
	zoomer2(ptr, nx, ny, &zoom_sym, &ns, &ms, 0);
	break;
      case 3:
	zoomer3(ptr, nx, ny, &zoom_sym, &ns, &ms, 0);
	break;
      case 4:
	zoomer4(ptr, nx, ny, &zoom_sym, &ns, &ms, 0);
	break;
      case 8:
	zoomer8(ptr, nx, ny, &zoom_sym, &ns, &ms, 0);
	break;
      case 16:
	zoomer16(ptr, nx, ny, &zoom_sym, &ns, &ms, 0);
	break;
      case -2:
	compress2(ptr, nx, ny, &zoom_sym, &ns, &ms, 0);
	break;
      case -4:
	compress4(ptr, nx, ny, &zoom_sym, &ns, &ms, 0);
	break;
      default:
	return luxerror("TVPLANE - illegal zoom factor %1d; only -4,-2,1,2,3,4,8,16 allowed", 0, tvplanezoom);
    }
    /* get pointer to this now, assume that ns and ms are the proper
       dimension */
    ptr = array_data(zoom_sym);
    nx = ns;
    ny = ms;
  }

  /* extra stuff for LHS:
     RHS SCALE() returns raw pixel values, but LHS SCALE() returns pixel
     index values, which must be put through COLORPIXEL() to be transformed
     into raw pixel values.  */
  if (!subfree)
    /* we're looking at the original image, which we are not supposed
       to modify */
    subfree = ptr2 = malloc(nx*ny);
  else
    ptr2 = ptr;

  ptr0 = ptr2;
  wq = nx*ny;			/* size of final image */
  while (wq--) 
    *ptr2++ = pixels[(uint8_t) *ptr++];

		 /* create image structure */
  xi = XCreateImage(display, visual, depth, ZPixmap, 0, (char *) ptr0,
		    nx, ny, 8, 0);
  XPutImage(display, win[wid], gc[wid], xi, 0, 0, 0, 0, nx, ny);
  /* now dealloc the image structure but not the data */
  xi->data = NULL;
  XDestroyImage(xi);
  if (subfree)
    free(subfree);
  subfree = NULL;
  XFlush(display);
  return 1;
}
/*---------------------------------------------------------*/
int32_t lux_threecolors(int32_t narg, int32_t ps[])
/* installs color table with three domains */
/* THREECOLORS,arg */
/* the color table consists of a grey domain, a red domain, and a blue
   domain.  The grey domain runs from 0 through threeColors - 1,
   the red domain from threeColors through 2*threeColors - 1, and
   the blue domain from 2*threeColors through 3*threeColors - 1.
   LS 12nov98 */
{
  float	fraction, *list;
  int32_t	threecolors(float *, int32_t);

  if (ck_events() != LUX_OK)
    return LUX_ERROR;

  if (!narg || symbol_class(ps[0]) == LUX_SCALAR) {
    if (narg)
      fraction = float_arg(ps[0]);
    else
      fraction = 1.0;
    return threecolors(&fraction, 1);
  }

  if (!symbolIsNumericalArray(ps[0]))
    return cerror(ILL_CLASS, ps[0]);
  if (array_size(ps[0]) != 9)
    return luxerror("Need 9 elements in array", ps[0]);
  list = array_data(lux_float(1, ps));
  return threecolors(list, 9);
}
/*---------------------------------------------------------*/
int32_t lux_tv3(int32_t narg, int32_t ps[])
/* TV3,<image>[,<bitmap1>,<bitmap2>] */
{
  int32_t	iq, nx, ny, mode, i, wid;
  float	fx, fy;
  uint8_t	*bitmap1, *bitmap2;
  pointer	data;
  int32_t	coordTrf(float *, float *, int32_t, int32_t);

  if (!symbolIsNumericalArray(ps[0]) /* <image> */
      || isComplexType(array_type(ps[0]))
      || array_num_dims(ps[0]) != 2)
    return cerror(NEED_2D_ARR, ps[0]);
  data.b = array_data(ps[0]);
  nx = array_dims(ps[0])[0];
  ny = array_dims(ps[0])[1];

  if (narg > 1 && ps[1]) {	/* <bitmap1> */
    if (symbolIsScalar(ps[1]))
      bitmap1 = NULL;
    else if (symbolIsNumericalArray(ps[1])
	     && array_size(ps[1]) == array_size(ps[0])) {
      iq = lux_byte(1, &ps[1]);	/* ensure BYTE */
      bitmap1 = array_data(iq);
    } else
      return cerror(INCMP_ARG, ps[1]);
  } else
    bitmap1 = NULL;

  if (narg > 2 && ps[2]) {	/* <bitmap2> */
    if (symbolIsScalar(ps[2]))
      bitmap2 = NULL;
    else if (symbolIsNumericalArray(ps[2])
	     && array_size(ps[2]) == array_size(ps[0])) {
      iq = lux_byte(1, &ps[2]);
      bitmap2 = array_data(iq);
    } else
      return cerror(INCMP_ARG, ps[2]);
  } else
    bitmap2 = NULL;
  
  if (narg > 3 && ps[3]) {	/* <x> */
    if (!symbolIsScalar(ps[3]))
      return cerror(NEED_SCAL, ps[3]);
    fx = float_arg(ps[3]);
  } else
    fx = 0;
  
  if (narg > 4 && ps[4]) {	/* <y> */
    if (!symbolIsScalar(ps[4]))
      return cerror(NEED_SCAL, ps[4]);
    fy = float_arg(ps[4]);
  } else
    fy = 0;

  wid = last_wid;
  if (narg > 5 && ps[5]) {	/* window */
    if (!symbolIsScalar(ps[5]))
      return cerror(NEED_SCAL, ps[5]);
    wid = int_arg(ps[5]);
  }

  mode = internalMode;
  internalMode = 0;		/* or it may interfere with lux_scale() which
				   is called by tvraw() */

  if (narg > 6 && ps[6]) {	/* <scale> */
    if (!symbolIsScalar(ps[6]))
      return cerror(NEED_SCAL, ps[6]);
    tvscale = float_arg(ps[6]);
    if (tvscale == 1)
      mode |= TV_SCALE;
  } else
    tvscale = 0.0;

  if (!threeColors)
    lux_threecolors(0, NULL);
  i = tvraw(data, array_type(ps[0]), array_dims(ps[0])[0],
	    array_dims(ps[0])[1], 0, nx - 1, 0, ny - 1, fx, fy,
	    wid, &tvscale, mode, 0.0, 0.0, bitmap1, bitmap2);
  return i;
}
/*---------------------------------------------------------*/
int32_t invert_flag = 0;
int32_t lux_xdrawline(int32_t narg, int32_t ps[])
/* subroutine, call is xdrawline, x1, y1, x2, y2 where the arguments can
   be scalars or arrays but all must match in length */
/* used for X window drawing with lower overhead than xymov calls */
/* better for interactive graphics */
{
  int32_t     wid, ixs, iys, ix, iy;
  Drawable	dq;
  GC		gq;

  wid = last_wid;
  if (narg > 4) {
    if (int_arg_stat(ps[4], &wid) != 1)
      return LUX_ERROR;
  }
  if (wid < 0 ) {
    dq = maps[-wid];
    if (dq == 0) {
      lux_xcreat(wid, 512, 512, 0, 0, 0, NULL, NULL);
      dq = maps[-wid];		/* added LS 22mar99 */
    } else
      set_defw(wid);
    gq = gcmap[-wid];
  } else {
    dq = win[wid];
    if (dq == 0) {
      lux_xcreat(wid, 512, 512, 0, 0, 0, NULL, NULL);
      dq = win[wid];		/* added LS 22mar99 */
    } else
      set_defw(wid);
    gq = gc[wid];
  }
  /* note that the set_defw should be done before getting the other
     arguments in case any of them use the window limit symbols */
  /* the arguments can be scalars (class 1 or 8) or arrays (class 4), check
     first for not class 4 as first step */
  if (symbol_class(ps[0]) != LUX_ARRAY) {
    /* assume all scalars, int_arg_stat will complain if that doesn't happen,
       it will also handle the class 8 for us */
    if (int_arg_stat(ps[0], &ixs) != 1
	|| int_arg_stat(ps[1], &iys) != 1
	|| int_arg_stat(ps[2], &ix) != 1
	|| int_arg_stat(ps[3], &iy) != 1)
      return LUX_ERROR;
    
    switch (invert_flag) {
      case 0:
	XDrawLine(display, dq, gq, ixs, iys, ix, iy);
	break;
      case 1:
	XDrawLine(display, dq, gcnot, ixs, iys, ix, iy);
	break;
    }
  } else {
    /* here we expect 4 arrays of the same size */
    int32_t	*px1, *px2, *py1, *py2, n, nx, iq;

    iq = lux_long(1, &ps[0]);
    nx = array_size(iq);
    px1 = array_data(iq);

    if (symbol_class(ps[1]) != LUX_ARRAY)
      return cerror(NEED_ARR, ps[1]);
    iq = lux_long(1, &ps[1]);
    n = array_size(iq);
    if (n != nx)
      return cerror(INCMP_ARG, ps[1]);
    py1 = array_data(iq);
    
    if (symbol_class(ps[2]) != LUX_ARRAY)
      return cerror(INCMP_ARG, iq);
    iq = lux_long(1, &ps[2]);
    n = array_size(iq);
    if (n != nx)
      return cerror(INCMP_ARG, ps[2]);
    px2 = array_data(iq);;
    
    if (symbol_class(ps[3]) != LUX_ARRAY)
      return cerror(NEED_ARR, iq);
    iq = lux_long(1, &ps[3]);
    n = array_size(iq);
    if (n != nx)
      return cerror(INCMP_ARG, ps[3]);
    py2 = array_data(iq);

    /* now just loop some xdrawline calls, we don't know if the lines
       connect */
    switch (invert_flag) {
      case 0:
	while (n--)
	  XDrawLine(display, dq, gq, *px1++, *py1++, *px2++, *py2++);
	break;
      case 1:
	while (n--)
	  XDrawLine(display, dq, gcnot, *px1++, *py1++, *px2++, *py2++);
	break;
    }
  }
  return LUX_OK;
}
/*---------------------------------------------------------*/
int32_t lux_xinvertline(int32_t narg, int32_t ps[])
/* used for X window drawing with lower overhead than xymov calls */
/* better for interactive graphics */
{
  int32_t	iq;

  invert_flag = 1;
  iq = lux_xdrawline(narg, ps);
  invert_flag = 0;
  return iq;
}
/*------------------------------------------------------------------------*/
int32_t lux_xinvertarc(int32_t narg, int32_t ps[])
{
  int32_t	iq;
  int32_t	lux_xdrawarc(int32_t, int32_t []);

  invert_flag = 1;
  iq = lux_xdrawarc(narg, ps);
  invert_flag = 0;
  return iq;
}
/*------------------------------------------------------------------------*/
int32_t lux_xdrawarc(int32_t narg, int32_t ps[])
/* subroutine, call is xdrawarc, x1, y1, w, h, [a1, a2, win] */
{
  int32_t     wid, ixs, iys, w, h, xa1, xa2;
  float	a1, a2;
  Drawable	dq;
  GC		gq;

  wid = last_wid;
  if (narg > 6 && int_arg_stat(ps[6], &wid) != 1)
    return LUX_ERROR;
  if (wid < 0) {
    dq = maps[-wid];
    if (dq == 0)
      lux_xcreat(wid, 512, 512, 0, 0, 0, NULL, NULL);
    else
      set_defw(wid);
    gq = gcmap[-wid];
  } else {
    dq = win[wid];
    if (dq == 0)
      lux_xcreat(wid, 512, 512, 0, 0, 0, NULL, NULL);
    else
      set_defw(wid);
    gq = gc[wid];
  }
  /* note that the set_defw should be done before getting the other
     arguments in case any of them use the window limit symbols */
  /* assume all scalars, int_arg_stat will complain if that doesn't happen, it
     will also handle the class 8 for us */
  if (int_arg_stat(ps[0], &ixs) != LUX_OK
      || int_arg_stat(ps[1], &iys) != LUX_OK
      || int_arg_stat(ps[2], &w) != LUX_OK
      || int_arg_stat(ps[3], &h) != LUX_OK)
    return LUX_ERROR;
  a1 = 0.0;
  a2 = 360.0;
  if (narg > 4 && float_arg_stat(ps[4], &a1) != LUX_OK)
    return LUX_ERROR;
  if (narg > 5 && float_arg_stat(ps[5], &a2) != LUX_OK)
    return LUX_ERROR;
  /* convert these fp angles into X units */
  xa1 = (int32_t) 64.*a1;
  xa2 = (int32_t) 64.*a2;

  switch (invert_flag) {
    case 0:
      XDrawArc(display, dq, gq, ixs, iys, w, h, xa1, xa2);
      break;
    case 1:
      XDrawArc(display, dq, gcnot, ixs, iys, w, h, xa1, xa2);
      break;
  }
  return LUX_OK;
}
/*---------------------------------------------------------*/
