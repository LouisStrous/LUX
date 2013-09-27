/* This is file menu.c.

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
/* LUX routines dealing with (X window) menus */
/* file menu.c  LS  Started 17apr93.
   contains routines for creating and manipulating menus in windows */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xos.h>
#include <X11/cursorfont.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "install.h"
#include "action.h"
#include "editor.h"

#define BLACK           1
#define WHITE           0
#define NONE		100

#define BORDER_WIDTH	4

#define TEXTMENU	1

Int	lux_replace(Int, Int), define_menu(Int, Int, Int, Int []),
  redefine_menu(Int, Int []), coordTrf(Float *, Float *, Int, Int),
  set_defw(Int);
void	printfw(char *, ...);

extern Display	*display;
extern Int	screen_num, black_pixel, white_pixel;
extern Double	last_time;
Int	theMenu, theItem;

struct Menu {
  Int          n_items;		/* number of menu panes (including title) */
  char         **text;		/* pointer to item strings */
  Window       *window;		/* pointer to item windows */
};
typedef        struct Menu     Menu;
/* title is # 0 */

Menu	menu[MAXMENU];
XFontStruct	*menu_font_info;
char	*menu_font_name = "fixed", menu_setup_done = 0;
Window	menu_win[MAXMENU], inverted_pane = NONE;
GC	menugc, menurgc;
Cursor	cursor;
Int	last_menu, menu_item, fontheight, fontwidth;
Float	menu_x, menu_y;
Int	text_menus = 0;

void	delete_menu(Int);
/*------------------------------------------------------------------------*/
Int menu_setup(void)
/* setup for menus.  executed once before first menu is created */
{
 Int	i;

 setup_x();
 menu_font_info = XLoadQueryFont(display, menu_font_name);
 if (!menu_font_info)
   return luxerror("Cannot find font %s\n", 0, menu_font_name);
 fontheight = menu_font_info->max_bounds.ascent
            + menu_font_info->max_bounds.descent;
				/* scale !XMENU with size of largest */
				/* char in font */
 fontwidth = menu_font_info->max_bounds.width;
 menugc = XCreateGC(display, RootWindow(display, screen_num), 0, NULL);
 XSetForeground(display, menugc, BlackPixel(display, screen_num));
 XSetFont(display, menugc, menu_font_info->fid);
 menurgc = XCreateGC(display, RootWindow(display, screen_num), 0, NULL);
 XSetForeground(display, menurgc, WhitePixel(display, screen_num));
 XSetFont(display, menurgc, menu_font_info->fid);
 for (i = 0; i < MAXMENU; i++)
   menu_win[i] = 0;
 menu_setup_done = 1;
 return LUX_OK;
}
/*------------------------------------------------------------------------*/
Int placeEvent(XEvent *event)
/* returns 1 if the event occurred in one of our menus;  0 otherwise. */
/* if 1, then stores the menu number in theMenu and the item number in */
/* theItem.  KeyPress and PointerMotion events always have theItem = -1. */
/* if 0, then both theMenu and theItem are -1. */
{
 Int	i, j;

 theMenu = -1;
 theItem = -1;
 for (i = 0; i < MAXMENU; i++) {
   if (menu_win[i] == event->xany.window) {
     theMenu = i;
     return 1;
   }
   for (j = 0; j < menu[i].n_items - 1; j++)
     if (event->xany.window == menu[i].window[j]) {
       theMenu = i;
       theItem = j;
       return 1;
     }
 }
 return 0;
} 
/*------------------------------------------------------------------------*/
void paint_pane(Int menu_num, Int menu_item, Int mode)
/* paints a menu item */
{
 Int	x = 2, y;
 GC	gc;
 Window	window;

 window = menu[menu_num].window[menu_item - 1];
 switch (mode) {
   case BLACK:
     XSetWindowBackground(display, window, BlackPixel(display, screen_num));
     gc = menurgc;
     break;
   case WHITE:
     XSetWindowBackground(display, window, WhitePixel(display, screen_num));
     gc = menugc;
     break;
 }
    /* clearing repaints the background */
 XClearWindow(display, window);
   /* find out index of window for label text */
 y = menu_font_info->max_bounds.ascent;
    /* the string length is necessary because strings for
        XDrawString may not be NULL terminated */
 XDrawString(display, window, gc, x, y + 2, menu[menu_num].text[menu_item],
	     strlen(menu[menu_num].text[menu_item]));
 return;
}
/*------------------------------------------------------------------------*/
Int lux_menu_hide(Int narg, Int ps[])
/* remove menu from window, but keep in memory */
/* if argument is -1, then hide all */
{
 Int	num, i1, i2;

 num = int_arg(ps[0]);
 if (num < -1 || num >= MAXMENU) 
   return luxerror("Menu number out of range", 0);
 if (!text_menus) {
   if (num < 0) {
     i1 = 0;
     i2 = MAXMENU;
   } else {
     i1 = num;
     i2 = num + 1;
   }
   for (num = i1; num < i2; num++)
     if (menu[num].n_items) XUnmapWindow(display, menu_win[num]); /* unmap */
   XFlush(display);
 }
 return LUX_OK;
}
 /*------------------------------------------------------------------------*/
Int lux_menu_kill(Int narg, Int ps[])
/* remove menu from window and from memory
   argument -1 -> remove all menus */
{
 Int	num, i1, i2;

 num = int_arg(ps[0]);
 if (num < -1 || num >= MAXMENU)
   return luxerror("Menu number out of range", 0);
 if (num < 0) {
   i1 = 0;
   i2 = MAXMENU;
 } else {
   i1 = num;
   i2 = num + 1;
 }
	/* note: a nonexistant menu is guaranteed to have menu_win[] == 0 */
 for (num = i1; num < i2; num++)
   if (menu_win[num])
     delete_menu(num);
 if (!text_menus)
   XFlush(display);
 return LUX_OK;
} 
 /*------------------------------------------------------------------------*/
Bool menuButtonPress(Display *display, XEvent *event, XPointer arg)
/* returns True if the event is a ButtonPress, EnterNotify, or LeaveNotify */
/* in one of our menus, False otherwise */
{
  Int	num;

  if (arg)
    num = *((Int *) arg);
  else
    num = -1;
  switch (event->type) {
    case ButtonPress: case EnterNotify: case LeaveNotify:
      return (placeEvent(event) && (num < 0 || theMenu == num))?
	True: False;
    default:
      return False;
  }
}
 /*------------------------------------------------------------------------*/
Bool menuKeyPress(Display *display, XEvent *event, XPointer arg)
/* returns True if the event is a KeyPress */
/* in one of our menus, False otherwise */
{
  Int	num;

  if (arg) num = *((Int *) arg); else num = -1;
  switch (event->type) {
    case KeyPress:
      return (placeEvent(event) && (num < 0 || theMenu == num))?
	True: False;
    default:
      return False;
  }
}
/*------------------------------------------------------------------------*/
Bool menuEvent(Display *display, XEvent *event, XPointer arg)
/* returns True if the event was in one of our menus, False otherwise */
{
  return placeEvent(event)? True: False;
}
/*------------------------------------------------------------------------*/
Int lux_check_menu(Int narg, Int ps[])
/* checks event buffer for any pending menu selections */
/* function: returns 1 if pending, 0 if not pending, error if illegal */
{
 Int	num;
 XEvent	event;

 if (text_menus)
   return luxerror("lux_check_menu does not yet support text menus!", 0);
 if (narg >= 1)
   num = int_arg(*ps);
 else
   num = -1;
 if (num < 0 || num >= MAXMENU)
   return luxerror("Menu number %1d out of range (0:%1d)", 0, num, MAXMENU - 1);
 if (!menu_win[num])
   return luxerror("Selected menu %1d does not exist", 0, num);
 if (XCheckIfEvent(display, &event, menuButtonPress,
		   (XPointer) ((num < 0)? NULL: &num))) {
   switch (event.type) {
     case EnterNotify:
       paint_pane(theMenu, theItem, BLACK);
       break;
     case LeaveNotify:
       paint_pane(theMenu, theItem, WHITE);
       break;
     default:
       XPutBackEvent(display, &event);
   }
   return LUX_ONE;
 } else
   return LUX_ZERO;
} 
 /*------------------------------------------------------------------------*/
char getMenuChar(Int menu_num)
/* reads the next character from menu #menu_num */
{
  XEvent	event;
  char	buffer;

  if (text_menus)
    return luxerror("getMenuChar not yet upgraded for text menus!", 0);
  while (1) {
    XIfEvent(display, &event, menuKeyPress,
	     (menu_num >= 0)? (XPointer) &menu_num: NULL);
    XLookupString(&event.xkey, &buffer, 1, NULL, NULL);
    if (buffer == '\r')
      buffer = '\n';
    return buffer;
  }
}
 /*------------------------------------------------------------------------*/
char *readPane(Int menu_num, Int item_num, char *query)
{
  extern Int	scrat[];
  char	*current, *old, *answer;
  Int	y, c = 'x', revert_to;
  Window	focusWindow;
  
  /* make item text pointer point at "current" instead of "old", */
  /* so that paint_pane finds the text that is being entered */
  current = (char *) scrat;
  old = menu[menu_num].text[item_num];
  menu[menu_num].text[item_num] = current;
  strcpy(current, query);
  query = current;
  current += strlen(current);
  strcat(query, "?_");
  answer = current;
  y = menu_font_info->max_bounds.ascent;
  XMapWindow(display, menu_win[menu_num]);
  XGetInputFocus(display, &focusWindow, &revert_to);
  XSetInputFocus(display, menu_win[menu_num], RevertToParent, CurrentTime);
  c = '\0';
  while (c != '\n') {
    XSetWindowBackground(display, menu[menu_num].window[item_num - 1],
			 WhitePixel(display,screen_num));
    XClearWindow(display, menu[menu_num].window[item_num - 1]);
    XDrawString(display, menu[menu_num].window[item_num - 1],
		menugc, 2, y + 2, query, strlen(query));
    XFlush(display);
    c = getMenuChar(menu_num);
    switch (c) {
      default:
	if (!isprint(c))
	  break;
	if (!current[1])
	  strcpy(&current[1], "_");
	*current++ = c;
	break;
      case '\n':
	*current = '\0';
	break;
#if FIXKEYCODES
      case '\b': case DEL:
	if (current > answer) {
	  memmove(current - 1, current, strlen(current - 1));
	  current--;
	}
	break;
      case BKC: case LAR:
	if (current > answer)
	  current--;
	break;
      case FWC: case RAR:
	if (current[1])
	  current++;
	break;
#endif
    }
  }
  XSetWindowBackground(display, menu[menu_num].window[item_num - 1],
		       WhitePixel(display,screen_num)); 
  XClearWindow(display, menu[menu_num].window[item_num - 1]);
  menu[menu_num].text[item_num] = old; /* restore old text */
  XDrawString(display, menu[menu_num].window[item_num - 1], menugc, 2, y + 2,
	      old, strlen(old));
  XSetInputFocus(display, focusWindow, revert_to, CurrentTime);
  XFlush(display);
  return answer;
}
/*---------------------------------------------------------------------*/
Int lux_menu_read(Int narg, Int ps[])
     /* read string through menu item; 
	Syntax:  menuread,menu#,item#,query,answer */
{
  Int	menu_num, item_num, result;
  char	*query, *answer;

  if (text_menus)
    return luxerror("lux_menu_read not yet upgraded for text menus!", 0);
  menu_num = int_arg(*ps++);
  if (menu_num < 0 || menu_num >= MAXMENU) 
    return luxerror("Menu number %1d out of range", 0, menu_num);
  if (!menu_win[menu_num])
    return luxerror("Selected menu %1d does not exist", 0, menu_num);
  item_num = int_arg(*ps++);
  if (item_num < 1 || item_num >= menu[menu_num].n_items)
    return luxerror("Selected item %1d of menu %1d does not exist", 0,
		 item_num, menu_num);
  query = string_value(ps[2]);
  answer = strsave(readPane(menu_num, menu_item, query));
  result = string_scratch(strlen(answer));
  string_value(result) = answer;
  return lux_replace(*ps, result);
}
 /*------------------------------------------------------------------------*/
Int lux_wait_for_menu(Int narg, Int ps[])
/* grabs pointer and waits for menu input */
{
 Int	mode;
 XEvent	event;

 if (text_menus)
   return luxerror("lux_wait_for_menu not yet upgraded for text menus!", 0);
 if (narg == 1)
   mode = int_arg(ps[0]);
 else
   mode = 0;
 /* if mode is nonzero, then empty buffer of ButtonPress events
    before waiting for input  - i.e. discard anything clicked before */
 if (mode)
   while (XCheckIfEvent(display, &event, menuButtonPress, NULL));
 while (1) {
   XIfEvent(display, &event, menuEvent, NULL);
   switch (event.type) {
     case Expose:
       paint_pane(theMenu, theItem, WHITE);
       break;
     case EnterNotify:
       paint_pane(theMenu, theItem, BLACK);
       break;
     case LeaveNotify:
       paint_pane(theMenu, theItem, WHITE);
       break;
     case ButtonPress:
       last_menu = theMenu;
       menu_item = theItem;
       last_time = (Double) event.xbutton.time/1000.0;
       menu_x = (Float) event.xbutton.x/((Float) fontwidth);
       menu_y = (Float) event.xbutton.y/((Float) fontheight);
       paint_pane(theMenu, theItem, BLACK);
       paint_pane(theMenu, theItem, WHITE);
       XFlush(display);
       return LUX_OK;
   }
   return LUX_OK;
 }
}
 /*------------------------------------------------------------------------*/
Int lux_menu_pop(Int narg, Int ps[])
/* as lux_menu, but fix x and y according to mouse position */
{
 Int	x, y, i, xd, yd;
 uint32_t	kb;
 Window	qroot, qchild;

 if (text_menus)
   return luxerror("lux_menu_pop not yet upgraded for text menus!", 0);
 if (menu_setup() < 0)
   return LUX_ERROR;
	/* get current mouse position in root window */
 XQueryPointer(display, RootWindow(display, screen_num), &qroot, &qchild,
	       &x, &y, &xd, &yd, &kb);
 i = define_menu(x, y, narg, ps);
 return i;
}
/*------------------------------------------------------------------------*/
Int createMenu(Int num, Int x, Int y, Int nItem, char **item)
{
  Int	i, direction, ascent, descent, menu_width = 0,
    border_width = BORDER_WIDTH, item_height = 0, menu_height;
  char	*title, *text;
  XCharStruct	overall;
  XSetWindowAttributes	attributes;
  XWindowAttributes	w_attributes;

  delete_menu(num);
  menu[num].n_items = nItem;
  if (!(menu[num].text = (char **) Malloc(nItem*sizeof(char *))))
    return cerror(ALLOC_ERR, 0);
  if (!(menu[num].window = (Window *) Malloc(nItem*sizeof(Window)))) {
    Free(menu[num].text);
    return cerror(ALLOC_ERR, 0);
  }
  for (i = 0; i < nItem; i++) {
    menu[num].text[i] = strsave(*item? *item: "");
    item++;
  }
  title = menu[num].text[0];
  if (text_menus)
    menu_win[num] = TEXTMENU;
  else {
    for (i = 0; i < nItem; i++) {
      text = menu[num].text[i];
      XTextExtents(menu_font_info, text, strlen(text), &direction,
		   &ascent, &descent, &overall);
      if (overall.width > menu_width)
	menu_width = overall.width;
    }
    menu_width += border_width;
    item_height = fontheight + 2 + border_width;
    menu_height = item_height*(nItem - 1);
    attributes.background_pixel = white_pixel;
    attributes.backing_store = Always;
    attributes.bit_gravity = StaticGravity;
    attributes.event_mask = KeyPressMask;
    menu_win[num] =
      XCreateWindow(display, RootWindow(display, screen_num),
		    x, y, menu_width, menu_height, border_width,
		    CopyFromParent, 0, DefaultVisual(display, screen_num),
		    CWBackingStore | CWBitGravity | CWBackPixel | CWEventMask,
		    &attributes);
    XSetIconName(display, menu_win[num], title);
    XStoreName(display, menu_win[num], title);
    XMapWindow(display, menu_win[num]);	/* now XGetWindowAttributes returns
					   actual width */
    XFlush(display);
    XGetWindowAttributes(display, menu_win[num], &w_attributes);
    menu_width = w_attributes.width;	/* items as wide as window */
    attributes.event_mask = ButtonPressMask | EnterWindowMask
      | LeaveWindowMask;
    for (i = 1; i < nItem; i++) {
      menu[num].window[i - 1] =
	XCreateWindow(display, menu_win[num], 0, item_height*(i - 1),
		      menu_width, item_height, 1, CopyFromParent, 0,
		      DefaultVisual(display, screen_num), CWBackingStore
		      | CWBitGravity | CWBackPixel | CWEventMask,
		      &attributes);
    }
    XMapSubwindows(display, menu_win[num]);
    cursor = XCreateFontCursor(display, XC_left_ptr);
    XDefineCursor(display, menu_win[num], cursor);
  }
  return LUX_OK;
}
/*------------------------------------------------------------------------*/
Int lux_menu(Int narg, Int ps[])
/* try at menus.  Syntax:
   MENU,num,x,y,title,item1 [,item2...]             (re)create menu
   MENU,num,title,item1 [,item2...]                  repaint menu
   MENU,num [,x,y]                                   redisplay menu
 Note that in a repaint, the menu does not change its size!
*/
{
 Int	num, i, n_items, iq, x = 0, y = 0;

 num = int_arg(ps[0]);		/* menu number */
 if (num < 0 || num >= MAXMENU)
   return luxerror("Menu number %1d out of range", 0, num);
 if (narg == 1 && !menu_win[num]) /* MENU,num  case */
   return luxerror("Requested menu %1d does not exist", 0, num);
 if (narg > 1) {
   iq = ps[1];
   if (symbolIsNumerical(symbol_type(iq))) { /* no string: MENU,num,x,y,... */
     if (narg == 2) 
       return luxerror("Need x and y coordinates", 0);
     x = int_arg(iq);
     y = int_arg(ps[2]);
     i = 2;			/* skip num,x,y */
     ps[2] = ps[0];		/* pass menu number to define_menu */
   } else
     i = 0;			/* MENU,num,title,item1,...   skip num */
   n_items = narg - i - 1;	/* # text arguments */
   if (!menu_setup_done && menu_setup() != 1)
     return LUX_ERROR;
   if (n_items == 0 || i == 0) {
     if (!menu_win[num])
       return luxerror("Requested menu %1d does not exist", 0, num);
     else if (i == 2) {
       if (!text_menus)
	 XMoveWindow(display, menu_win[num], x, y);
     } else if (n_items && (!redefine_menu(n_items + 1, ps + i)))
       return LUX_ERROR;
   } else if (!define_menu(x, y, n_items + 1, ps + i))
     return LUX_ERROR;
 }
 return LUX_OK;
}
/*------------------------------------------------------------------------*/
Int redefine_menu(Int narg, Int ps[])
/* store new menu items in existing Menu struct. 
   ps[]:  menu_number, menu_item, ... */
{
 Int	num, n_new, i;
 char	string_array = 0, *text, *zilch = "", **ptr, *title;

 num = int_arg(ps[0]);
 if (narg == 2
     && symbol_class(ps[1]) == LUX_ARRAY
     && symbol_type(ps[1]) == LUX_STRING_ARRAY) {
   string_array = 1;		/* string array, one allowed */
   ptr = (char **) array_data(ps[1]);
   narg = array_size(ps[1]);
 } else
   narg--;
 n_new = menu[num].n_items;
 if (narg < n_new)
   n_new = narg;
 for (i = 0; i < n_new; i++) {
   Free(menu[num].text[i]);	/* delete individual entries */
   if (string_array) {
     if (*ptr)
       text = *ptr++; 
     else {
       text = zilch;
       ptr++;
     }
   } else if (!(text = string_arg(*++ps))) {
     puts("Menu item must be a string");
     text = zilch;
   }
   if (i)
     menu[num].text[i - 1] = strsave(text);
   else
     title = text;
 }
 if (!text_menus) {
   XSetIconName(display, menu_win[num], title);
   XStoreName(display, menu_win[num], title);
 }
 return LUX_OK;
}
/*------------------------------------------------------------------------*/
Int lux_menu_item(Int narg, Int ps[])
/* change a single menu item
   Syntax: MenuItem,menu,item,text
   LS 29apr93 */
{
 Int	num, item;
 char	*text;

 if (text_menus)
   return luxerror("lux_menu_item not yet upgraded for text menus!", 0);
 num = int_arg(ps[0]);
 if (num < 0 || num >= MAXMENU)
   return luxerror("Menu number %1d out of range (0:%1d)", 0, num, MAXMENU - 1);
 if (!menu_win[num])
   return luxerror("Requested menu %1d does not exist", 0, num);
 item = int_arg(ps[1]);
 if (item < 0 || item >= menu[num].n_items)
  return luxerror("Menu item number %1d out of range (0:%1d)", 0, item,
	       menu[num].n_items - 1);
 text = string_arg(ps[2]);
 Free(menu[num].text[item]);
 menu[num].text[item] = strsave(text);
 if (item > 0)
   paint_pane(num, item, WHITE);
 /* else change icon title:  still to be done! */
 XFlush(display);
 return LUX_OK;
}
 /*------------------------------------------------------------------------*/ 
Int define_menu(Int x, Int y, Int narg, Int ps[])
/* store menu items in Menu struct.  menu number must be valid!
   also finds appropriate menu sizes, creates windows (but doesn't map
   them to the screen)  ps[]:  menu_num, menu_item, ...
   returns LUX_OK if successful, LUX_ERROR if an error occurred */
{
  Int	num, i;
  char	string_array = 0, **item, *text;
  pointer	ptr;

  num = int_arg(ps[0]);
  if (narg == 2
      && symbol_class(ps[1]) == LUX_ARRAY
      && array_type(ps[1]) == LUX_STRING_ARRAY) {
    string_array = 1;			/* string array, one allowed */
    narg = array_size(ps[1]);
    ptr.l = array_data(ps[1]);
  } else
    narg--;
  if (narg < 2) 		/* just a title */
    return luxerror("Need at least one item in menu", 0);
  if (!(item = (char **) Malloc(narg*sizeof(char *))))
    return cerror(ALLOC_ERR, 0);
  for (i = 0; i < narg; i++) {		/* all menu items and title */
    if (string_array) {
      if (*ptr.sp)
	text = *ptr.sp++; 
      else {
	text = NULL;
	ptr.sp++;
      }
    }
    else if (!(text = string_arg(*++ps))) {
      puts("Menu item %1d is not a string -- substitute empty string");
      text = NULL;
    }
    *item++ = text;
  }
  item -= narg;
  if (createMenu(num, x, y, narg, item) == LUX_ERROR) 
    return LUX_ERROR;
  if (text_menus) {
    printf("Menu %1d: %s\n", num, menu[num].text[0]);
    for (i = 1; i < menu[num].n_items; i++)
      printf("%2d - %s\n", i, menu[num].text[i]);
  } else {
    XMapWindow(display, menu_win[num]); /* display window */
    XRaiseWindow(display, menu_win[num]);
    for (i = 1; i < menu[num].n_items; i++)
      paint_pane(num, i, WHITE);
    XFlush(display);
  }
  return LUX_OK;
}
 /*------------------------------------------------------------------------*/
void delete_menu(Int num)
/* delete a menu.  assumes that menu number <num> is legal. */
{
 Int	i;
 XEvent	e;

 if (!menu_win[num])
   return;			/* nothing to do */
 for (i = 0; i < menu[num].n_items; i++) {
   if (text_menus && i)
     while (XCheckWindowEvent(display, menu[num].window[i], ~NoEventMask,
			      &e));
   Free(menu[num].text[i]);		/* delete individual entries */
 }
 Free(menu[num].text);			/* delete entry pointer list */
 if (!text_menus) {
   Free(menu[num].window);		/* delete window pointer list */
   XDestroyWindow(display, menu_win[num]);
 }
 menu_win[num] = 0;			/* guaranteed for nonexistant menus */
}
 /*------------------------------------------------------------------------*/
#define X_KEYPRESS	1
#define X_BUTTONPRESS	4
#define X_BUTTONRELEASE	8
#define X_POINTERMOTION	16
#define X_ENTERWINDOW	32
#define X_LEAVEWINDOW	64
#define X_ALLWINDOWS	128
#define X_ALLMENUS	256
#define X_DESELECT	512
#define X_SOURCES	(X_ALLWINDOWS | X_ALLMENUS)
#define X_ALLEVENTS	(X_KEYPRESS | X_BUTTONPRESS | \
			X_BUTTONRELEASE | X_POINTERMOTION | X_ENTERWINDOW | \
			X_LEAVEWINDOW)
#define X_WINDOW	256
#define X_MENU		512
static Int	XRegisteredWindow[MAXWINDOWS], XRegisteredMenu[MAXMENU];
static Int	eventCode[] = {
  X_KEYPRESS, X_BUTTONPRESS, X_BUTTONRELEASE,
  X_POINTERMOTION, X_ENTERWINDOW, X_LEAVEWINDOW
};
static char	*XEventName[] = {
  "key press", "mouse button press", "mouse button release",
  "mouse motion", "mouse enters window", "mouse leaves window",
};
static char	anythingRegistered = 0;
static Int	N_XMask = sizeof(eventCode)/sizeof(Int);
Int	lux_event, eventSource;
extern Int	lux_button, root_x, root_y, xcoord, ycoord, last_wid,
		ht[], wd[];
extern Float	xhair, yhair;
extern Window	win[];

Int lux_register_event(Int narg, Int ps[])
     /* registers event types that XLOOP must act on */
     /* syntax:  XREGISTER,event_mask,window,menu,item */
     /* a negative event_mask unregisters;  a zero mask clears. */
{
  Int	type, addEvent, i, j, temp, iq, *windows, *menus, nWindow, nMenu,
  	sources;
  char	started;

  if (!narg && !internalMode) {	/* no arguments; show current */
    /* first see which events are selected for all windows */
    addEvent = ~0;
    temp = 0;
    for (i = 0; i < MAXWINDOWS; i++) {
      if (addEvent)
	addEvent &= XRegisteredWindow[i];
      temp |= XRegisteredWindow[i];
    }
    if (!temp)
      printw("No events registered for any window.\n");
    else {			/* report individual events */
      for (i = 0; i < MAXWINDOWS; i++) {
	temp = XRegisteredWindow[i] & ~addEvent;
	started = 0;
	if (temp) {
	  for (j = 0; j < N_XMask; j++)
	    if (temp & eventCode[j]) {
	      if (!started) {
		printfw("Window %d: %s", i, XEventName[j]);
		started = 1;
	      }
	      else
		printfw(", %s", XEventName[j]);
	    }
	  printw("\n");
	}
      }
				/* report all common events */
      if (addEvent) {
	puts("Events registered for all defined windows:");
	started = 0;
	for (i = 0; i < N_XMask; i++)
	  if (addEvent & eventCode[i]) {
	    if (!started) {
	      printf(" %s", XEventName[i]);
	      started = 1;
	    } else
	      printf(", %s", XEventName[i]);
	  }
	putchar('\n');
      }
    }
    /* now do the menus */
    addEvent = ~0;
    temp = 0;
    for (i = 0; i < MAXMENU; i++) {
      if (addEvent)
	addEvent &= XRegisteredMenu[i];
      temp |= XRegisteredMenu[i];
    }
    if (!temp)
      printw("No events registered for any menu.\n");
    else {			/* report individual events */
      for (i = 0; i < MAXMENU; i++) {
	temp = XRegisteredMenu[i] & ~addEvent;
	started = 0;
	if (temp) {
	  for (j = 0; j < N_XMask; j++)
	    if (temp & eventCode[j]) {
	      if (!started) {
		printfw("Menu %d: %s", i, XEventName[j]);
		started = 1;
	      } else
		printfw(", %s", XEventName[j]);
	    }
	  printw("\n");
	}
      }
				/* report all common events */
      if (addEvent) {
	puts("Events registered for all defined menus:");  started = 0;
	for (i = 0; i < N_XMask; i++)
	  if (addEvent & eventCode[i]) {
	    if (!started) {
	      printf(" %s", XEventName[i]);
	      started = 1;
	    } else
	      printf(", %s", XEventName[i]);
	  }
	putchar('\n'); }
    }
    return LUX_OK;
  }
				/* install new */
  if (narg)
    type = int_arg(*ps);
  else
    type = 0;			/* event codes */
  if (type < 0)
    type = -type | X_DESELECT;
  if (internalMode)
    type |= internalMode;
  if (narg >= 2 && ps[1]) {	/* window #s */
    iq = lux_long(1, &ps[1]);
    switch (symbol_class(iq)) {
      case LUX_SCAL_PTR:
	iq = dereferenceScalPointer(iq); /* fall-thru */
      case LUX_SCALAR:
	windows = &sym[iq].spec.scalar.l;
	nWindow = 1;
	break;
      case LUX_ARRAY:
	windows = array_data(iq);
	nWindow = array_size(iq);
	break;
      default:
	return cerror(ILL_CLASS, ps[1]);
    }
  } else
    nWindow = 0;
  if (narg >= 3 && ps[2]) {	/* menu #s */
    iq = lux_long(1, &ps[1]);
    switch (symbol_class(iq)) {
      case LUX_SCALAR:
	menus = &scalar_value(iq).l;
	nMenu = 1;
	break;
      case LUX_ARRAY:
	menus = array_data(iq);
	nMenu = array_size(iq);
	break;
      default:
	return cerror(ILL_CLASS, ps[1]);
    }
  } else
    nMenu = 0;
  if (!type) {
    for (i = 0; i < MAXWINDOWS; i++)
      XRegisteredWindow[i] = 0;
    for (i = 0; i < MAXMENU; i++)
      XRegisteredMenu[i] = 0;
    anythingRegistered = 0;
    return LUX_OK;
  }
  if (type & X_DESELECT) {
    temp = 1;
    type &= ~X_DESELECT;
  } else
    temp = 0;
  sources = (type & X_SOURCES);
  if (sources)
    type &= ~X_SOURCES;
				/* now do the registration */
  if (!nWindow && ((!sources && narg < 2) || sources & X_ALLWINDOWS)) {
    for (i = 0; i < MAXWINDOWS; i++)
      if (temp)
	XRegisteredWindow[i] &= ~type;
      else
	XRegisteredWindow[i] |= type;
  } else while (nWindow--) {
    if (*windows >= 0 && *windows < MAXWINDOWS) {
      if (temp)
	XRegisteredWindow[*windows] &= ~type;
      else
	XRegisteredWindow[*windows] |= type;
    } else
      printfw("Warning: window number %d out of range\n", *windows);
    windows++;
  }
  if (!nMenu && ((!sources && narg < 2) || sources & X_ALLMENUS)) {
    for (i = 0; i < MAXMENU; i++)
      if (temp)
	XRegisteredMenu[i] &= ~type;
      else
	XRegisteredMenu[i] |= type;
  } else while (nMenu--) {
    if (*menus >= 0 && *menus < MAXMENU) {
      if (temp)
	XRegisteredMenu[*menus] &= ~type;
      else
	XRegisteredMenu[*menus] |= type;
    } else
      printfw("Warning: menu number %d out of range\n", *menus);
    menus++;
  }
				/* determine if anything is registered */
  anythingRegistered = 0;
  for (i = 0; i < MAXWINDOWS; i++)
    if (XRegisteredWindow[i]) {
      anythingRegistered = 1;
      break;
    }
  if (!anythingRegistered)
    for (i = 0; i < MAXMENU; i++)
      if (XRegisteredMenu[i]) {
	anythingRegistered = 1;
	break;
      }
  return LUX_OK;
}
 /*------------------------------------------------------------------------*/
#define XLOOP_WINDOW	-1
#define XLOOP_MENU	-2
#define XLOOP_NONE	-3
Int lux_xloop(Int narg, Int ps[])
     /* LUX interface to X window manager. */
     /* waits for a registered event, returns event type in !EVENT_TYPE */
     /* important global variables:  button -> mouse button #; */
     /* lux_event -> event type;  xcoord, ycoord -> pointer position in */
     /* window;  menu_item -> menu item;  root_x, root_y -> pointer */
     /* position in root window;  last_wid -> window;  last_menu -> menu; */
     /* menu_x, menu_y -> pointer position in menu item; */
     /* last_time -> time of last event;  event_source -> source of event */
     /* returns appropriate values only if a registered event was caught. */
     /* treats EnterNotify and LeaveNotify events on menu items regardless */
     /* of whether such events were selected. */
{
  XEvent	event;
  Int	type, arg, i, j, mask;
  char	status = 0, buffer[4];
  KeySym	keysym;
  Window	w;
  extern Int	lux_keycode, lux_button, lux_keysym, lux_keystate;

  if (narg)
    arg = int_arg(*ps);		/* if 1 then flush event queue first */
  else
    arg = 0;
  if (!anythingRegistered) {	/* nothing was registered: return */
    eventSource = 0;
    lux_event = 0;
    return LUX_OK;
  }
  if (!menu_setup_done && menu_setup() != LUX_OK)
    return LUX_ERROR;
  status = 0;
  lux_keycode = lux_button = lux_keysym = lux_keystate = 0;
  /* clean out the event queue if required */
  if (arg)
    while (XCheckMaskEvent(display, ~0, &event));
  /* now treat all events until one of the requested kinds occurs */
  while (!status) {
    XNextEvent(display, &event);
    w = event.xany.window;
    j = XLOOP_NONE;
    status = 0;
    mask = 0;
    /* figure out which LUX window or menu the event occurred in, if any */
    /* results: status = 1 if in an LUX window or menu, then i = number */
    /* of the window or menu, j = XLOOP_WINDOW if a window, XLOOP_MENU */
    /* if a menu envelope, the item number if a menu item.  status = 0 */
    /* if not in an LUX window or menu. */
    if (w) {
      for (i = 0; i < MAXWINDOWS; i++)
	if (win[i] && w == win[i]) {
	  status = 1;
	  j = XLOOP_WINDOW;
	  mask = XRegisteredWindow[i];
	  break;
	}
      if (!status)
	for (i = 0; i < MAXMENU; i++) {
	  if (menu_win[i]) {
	    if (w == menu_win[i]) {
	      status = 1;
	      j = XLOOP_MENU;
	      mask = XRegisteredMenu[i];
	      break;
	    }
	    for (j = 0; j < menu[i].n_items - 1; j++)
	      if (w == menu[i].window[j]) {
		status = 1;
		mask = XRegisteredMenu[i];
		j++;
		break;
	      }
	    if (status) break;
	  }
	}
    }
    if (status && (((j == XLOOP_WINDOW && !XRegisteredWindow[i])
		    || (j >= 0 && !XRegisteredMenu[i])))) status = 0;
    /* now know if the right kind of source was caught; still need */
    /* to treat all kinds */
    switch (event.type) {
      case ButtonPress:
	if (mask & X_BUTTONPRESS)
	  type = X_BUTTONPRESS;
	else
	  status = 0;
	if (status) {
	  lux_button = event.xbutton.button;
	  root_x = event.xbutton.x_root;
	  root_y = event.xbutton.y_root;
	  last_time = (Double) event.xbutton.time/1000.0;
	  lux_keystate = event.xbutton.state;
	  switch (j) {
	    case XLOOP_WINDOW:	/* button press in an LUX window */
	      eventSource = i | X_WINDOW;
	      xhair = event.xbutton.x;
	      yhair = event.xbutton.y;
	      coordTrf(&xhair, &yhair, LUX_X11, LUX_DEV);
	      xcoord = (Int) xhair;
	      ycoord = (Int) yhair;
	      coordTrf(&xhair, &yhair, LUX_DEV, LUX_DVI);
	      break;
	    case XLOOP_MENU:	/* button press in LUX menu envelope */
	      return luxerror("?? button press in enveloping menu window?", 0);
	    default:		/* button press in LUX menu item */
	      menu_x = (Float) event.xbutton.x / (Float) fontwidth;
	      menu_y = (Float) event.xbutton.y / (Float) fontheight;
	      eventSource = i | X_MENU;
	      last_menu = i;
	      menu_item = j;
	  }
	}
	if (j == X_WINDOW)
	  set_defw(i);		/* set LUX window focus */
	break;
      case KeyPress:
	if (mask & X_KEYPRESS)
	  type = X_KEYPRESS;
	else
	  status = 0;
	if (status) {
	  lux_keycode = event.xkey.keycode;
	  lux_keystate = event.xkey.state;
	  i = XLookupString(&event.xkey, buffer, 15, &keysym, NULL);
	  buffer[i] = '\0';
	  lux_keysym = (Int) keysym;
	  root_x = event.xkey.x_root;
	  root_y = event.xkey.y_root;
	  last_time = (Double) event.xkey.time/1000.0;
	  switch (j) {
	    case XLOOP_WINDOW:	/* key press in an LUX window */
	      eventSource = i | X_WINDOW;
	      break;
	    case XLOOP_MENU:	/* key press in LUX menu envelope */
	      eventSource = i | X_MENU;
	      last_menu = i;
	      break;
	    default:		/* key press in LUX menu item */
	      return luxerror("?? key press in menu item?", 0);
	  }
	}
	break;
      case MotionNotify:
	/* remove all pointer motion events - we really only want the */
	/* last one */
	while (XCheckMaskEvent(display, PointerMotionMask, &event));
	if (mask & X_POINTERMOTION)
	  type = X_POINTERMOTION;
	else
	  status = 0;
	if (status) {
	  root_x = event.xmotion.x_root;
	  root_y = event.xmotion.y_root;
	  last_time = (Double) event.xmotion.time/1000.0;
	  switch (j) {
	    case XLOOP_WINDOW:
	      xhair = event.xbutton.x;
	      yhair = event.xbutton.y;
	      coordTrf(&xhair, &yhair, LUX_X11, LUX_DEV);
	      xcoord = (Int) xhair;
	      ycoord = (Int) yhair;
	      coordTrf(&xhair, &yhair, LUX_DEV, LUX_DVI);
	      eventSource = i | X_WINDOW;
	      break;
	    case XLOOP_MENU:
	      return luxerror("?? pointer motion in menu envelope?", 0);
	    default:
	      return luxerror("?? pointer motion in menu item", 0);
	  }
	}
	break;
      case EnterNotify:
	if (mask & X_ENTERWINDOW)
	  type = X_ENTERWINDOW;
	else
	  status = 0;
	switch (j) {
	  case XLOOP_WINDOW:	/* entering an LUX window */
	    if (status) {
	      last_time = (Double) event.xcrossing.time/1000.0;
	      eventSource = i | X_WINDOW;
	    }
	    break;
	  case XLOOP_MENU:	/* entering a menu envelope */
	    return luxerror("?? pointer enters menu envelope?", 0);
	  default:		/* entering a menu item */
	    if (status) {
	      last_menu = i;
	      menu_item = j;
	      eventSource = i | X_MENU;
	      last_time = (Double) event.xcrossing.time/1000.0;
	    }
	    paint_pane(i, j, BLACK);
	    XFlush(display);
	    break;
	}
	break;
      case LeaveNotify:
	if (mask & X_LEAVEWINDOW)
	  type = X_LEAVEWINDOW;
	else
	  status = 0;
	switch (j) {
	  case XLOOP_WINDOW:	/* leaving an LUX window */
	    if (status) {
	      eventSource = i | X_WINDOW;
	      last_time = (Double) event.xcrossing.time/1000.0;
	    }
	    break;
	  case XLOOP_MENU:	/* entering a menu envelope */
	    return luxerror("?? pointer leaves menu envelope?", 0);
	  default:		/* leaving a menu item */
	    if (status) {
	      last_menu = i;
	      menu_item = j;
	      eventSource = i | X_MENU;
	      last_time = (Double) event.xcrossing.time/1000.0;
	    }
	    paint_pane(i, j, WHITE);
	    XFlush(display);
	    break;
	}
	break;
      case ConfigureNotify:
	if (j == X_WINDOW) {
	  wd[i] = event.xconfigure.width;
	  ht[i] = event.xconfigure.height;
	  set_defw(i);
	}
	status = 0;
	break;
      default:			/* other event types are ignored */
	status = 0;
	break;
      }
  }
  lux_event = type;
  return LUX_OK;
}
 /*------------------------------------------------------------------------*/
char *eventName(Int type)
/* returns name of LUX X event */
{
  static char	eventHashTable[] = {
    3, 6, 5, 7, 0, 1, 7, 2, 4
  };
  Int	hash;

  if (type < 0)
    type = 9;
  hash = (type + 3)%11;
  if (hash > 8)
    hash = 3;
  hash = eventHashTable[hash];
  if (hash == 7 || eventCode[hash] != type)
    return "unknown";
  return XEventName[hash];
}
 /*------------------------------------------------------------------------*/
Int lux_event_name(Int narg, Int ps[])
{
  Int	type, result;
  
  type = int_arg(*ps);
  result = string_scratch(strlen(eventName(type)) + 1);
  string_value(result) = strsave(eventName(type));
  return result;
}
 /*------------------------------------------------------------------------*/
