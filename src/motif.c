/* This is file motif.c.

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
 /* lux interface for using motif widgets, the general plan is that most
 lux widgets will be dialogs that are children of a top level shell that
 contains a button to interrupt motif (to allow normal command entry).
 Other top level shells can also be supported but are not in the present
 program. The creation and destruction of lots of widgets means that a
 means of managing the lux data base must be developed */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include "lux_structures.h"
#include "install.h"
#include <stdarg.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xm/MwmUtil.h>
#include <Xm/Xm.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>
#include <Xm/Command.h>
#include <Xm/CascadeB.h>
#include <Xm/CascadeBG.h>
#include <Xm/Label.h>
#include <Xm/LabelG.h>
#include <Xm/MessageB.h>
#include <Xm/List.h>
#include <Xm/Form.h>
#include <Xm/Text.h>
#include <Xm/Scale.h>
#include <Xm/ScrolledW.h>
#include <Xm/ScrollBar.h>
#include <Xm/DrawingA.h>
#include <Xm/Frame.h>
#include <Xm/RowColumn.h>
#include <Xm/TextF.h>
#include <Xm/ArrowB.h>
#include <Xm/ToggleBG.h>
#include <Xm/ToggleB.h>
#include <Xm/SelectioB.h>
#include <Xm/Separator.h>
#include <Xm/FileSB.h>
#include <sys/stat.h>
 /*following is icon bitmap, currently same as in xport but want to change */
#include "lux_bitmap.xbm"

 extern	struct sym_desc sym[];

extern Colormap	colorMap;	/* from color.c */

 /* functions */
 void activateCB();
 void browse_callback(), button_callback(), quit_callback(), scale_callback();
 void textfield_callback(), lux_callback_execute(), radio_which();
 void radio_callback(), textfield_which(), menu_which(), scroll_callback();
 void selectionbox_cb(), command_callback(), fselect_callback();
 void fhelp_callback(), fcancel_callback(), lux_xminit(), bcancel_callback();
 void wprint(char *, ...), draw_in_callback(), draw_re_callback(), draw_ex_callback();
 void color_not_available(), font_not_available();
void destroy_cb(Widget, XtPointer, void *);		/* LS 20jan99 */
 Widget PostDialog(Widget parent, Int dialog_type, char *msg);
 Widget	lux_command_widget;
 
/*#define MAXWIDGETS       4000 */
/*#define MAXPIXMAPS       20*/
 Int	motif_flag = 1, motif_realized_flag = 0, motif_init_flag = 0;
 Int	motif_called_widget = -1; /* set by call backs */
 Int	xtloop_running = 0, input_modal_flag = 0;
Int ck_widget_count(void);
 static XmStringCharSet cset = (XmStringCharSet) XmSTRING_DEFAULT_CHARSET;
 static	XmString exit_button_text;
 static	Int	nlist, n_widgets = 0, toplevel;
 static XtAppContext   app_context;
 static	XEvent	event;
 Widget	lux_widget_id[MAXWIDGETS], text_output;
 static	Pixmap	lux_pixmap_id[MAXPIXMAPS];
 static	char	*list_item = {"$LIST_ITEM"};
 static	char	*selected_file = {"$SELECTED_FILE"};
 static	char	*command_item = {"$COMMAND_LINE"};
 static	char	*list_item_position = {"$LIST_ITEM_POSITION"};
 static	char	*scale_value = {"$SCALE_VALUE"};
 static	char	*scroll_value = {"$SCROLL_VALUE"};
 static	char	*textfield_number = {"$TEXTFIELD_NUMBER"};
 static	char	*textfield_value = {"$TEXTFIELD_VALUE"};
 /*
 static	char	*radio_button = {"$RADIO_BUTTON"};
 static	char	*check_button = {"$CHECK_BUTTON"};
 */
 Int	radio_button, radio_state;
 static	char	*option_value = {"$OPTION_VALUE"};
 static	XFontStruct	*font = NULL;
 static	XmFontList	fontlist = NULL;
 static	Display	*disp;
static Colormap	cmap;
 static	XColor  colorcell, rgb_def;
 static Arg	wargs[20];	/* shared by many routines */
 static	Int	n;		/* arg counter */
 static	Int	radiobox = 1;	/* an internal flag */
  /*------------------------------------------------------------------------- */
Widget xmgettopshell(wg)
 Widget wg;
 {
 while (wg && !XtIsWMShell(wg))  wg = XtParent(wg);
 return wg;
 }
 /*------------------------------------------------------------------------- */
Widget xmtoplevel_form(width, height, title, hs, vs, mx, my)
 /* creates a toplevel form widget, does not popup, returns the form
 widget created inside the toplevel */
 Int	width, height, hs, vs, mx, my;
 char	*title;
 {
 Widget	toplevel_form, form;
 XmString	wtitle;

 wtitle = XmStringCreateSimple(title);
 n=0;
 XtSetArg(wargs[n], XmNdeleteResponse, XmUNMAP); n++;
 XtSetArg(wargs[n], XmNallowShellResize, True); n++;
 XtSetArg(wargs[n], XmNcolormap, colorMap); n++; /* added LS 12mar99 */
 toplevel_form = XtAppCreateShell(NULL, "Class",
	topLevelShellWidgetClass, disp, wargs, n);
 
 n=0;
 XtSetArg(wargs[n], XmNwidth, width); n++;
 XtSetArg(wargs[n], XmNheight, height); n++;
 XtSetArg(wargs[n], XmNdialogTitle, wtitle); n++;
 XtSetArg(wargs[n], XmNautoUnmanage, False); n++;
 XtSetArg(wargs[n], XmNresizePolicy, XmRESIZE_ANY); n++;
 XtSetArg(wargs[n], XmNmarginWidth, mx); n++;
 XtSetArg(wargs[n], XmNmarginHeight, my); n++;
 XtSetArg(wargs[n], XmNhorizontalSpacing, hs); n++;
 XtSetArg(wargs[n], XmNverticalSpacing, vs); n++;
 form = XmCreateForm(toplevel_form, "form", wargs, n);
 XmStringFree(wtitle);
 XtManageChild(form);
 return form;
 }
 /*--------------------------------------------------------------------------*/
Widget xmtoplevel_board(width, height, title, mx, my)
 /* creates a toplevel form widget, does not popup, returns the form
 widget created inside the toplevel */
 Int	width, height, mx, my;
 char	*title;
 {
 Widget	toplevel_board, board;
 XmString	wtitle;
 wtitle = XmStringCreateSimple(title);
 n=0;
 XtSetArg(wargs[n], XmNdeleteResponse, XmUNMAP); n++;
 XtSetArg(wargs[n], XmNallowShellResize, False); n++;
 XtSetArg(wargs[n], XmNcolormap, colorMap); n++; /* added LS 12mar99 */
 toplevel_board = XtAppCreateShell(NULL, "Class",
	topLevelShellWidgetClass, disp, wargs, n);
 
 n=0;
 XtSetArg(wargs[n], XmNwidth, width); n++;
 XtSetArg(wargs[n], XmNheight, height); n++;
 XtSetArg(wargs[n], XmNdialogTitle, wtitle); n++;
 XtSetArg(wargs[n], XmNautoUnmanage, False); n++;
 XtSetArg(wargs[n], XmNresizePolicy, XmRESIZE_GROW); n++;
 XtSetArg(wargs[n], XmNmarginWidth, mx); n++;
 XtSetArg(wargs[n], XmNmarginHeight, my); n++;
 board = XmCreateBulletinBoard(toplevel_board, "board", wargs, n);
 XmStringFree(wtitle);
 XtManageChild(board);
 return board;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmgettopshell(narg, ps)
 /* top = xmgettopshell (widget) */
 Int     narg, ps[];
 {
 Int	w;
 Widget wg;
 if ( get_widget_id( ps[0], &w) != 1 ) return -1;
 wg = lux_widget_id[w];
 while (wg && !XtIsWMShell(wg))  wg = XtParent(wg);

 return 1;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmmessage(narg, ps) /* (string,[font,color,ix,iy]) */
 /* a temporary widget, does not return a widget id, used as an lux subr */
 /* 5/29/95 disable the menu and resize */
 Int     narg, ps[];
 {
 Int	iq, menus, ix, iy;
 Position	x, y;
 char *s;
 Widget dialog, parent;
 XmString text, title;
 /* pops a motif message dialog with the string passed */
 if (ck_motif() != 1) return -1;
 iq = ps[0];
 if (sym[iq].class != 2) { return execute_error(70); }
 s = (char *) sym[iq].spec.array.ptr;
 parent = lux_widget_id[toplevel];
 n = 0;
 dialog = XmCreateMessageDialog(parent, "dialog", NULL, 0);
 text = XmStringCreateLtoR(s, cset);
 title = XmStringCreateLtoR("Alert!", cset);
 XtSetArg(wargs[n], XmNdialogType, XmDIALOG_MESSAGE); n++;
 XtSetArg(wargs[n], XmNmessageString, text); n++;
 XtSetArg(wargs[n], XmNdialogTitle, title); n++;
 XtSetArg(wargs[n], XmNnoResize, True); n++;
 XtSetArg(wargs[n], XmNmessageAlignment, XmALIGNMENT_CENTER); n++;

 /* now the optional font and color */
 if (narg > 1) {
  if (set_textfontlist( ps[1] ) != 1) return -1; /* gets fontlist from string */
 }
 if (narg > 2) {
  if (setup_colors (ps[2] ) != 1) return -1; }	/* do colors based on bg */
 XtSetValues(dialog, wargs, n);
 /* junk the cancel and help buttons */
 XtUnmanageChild(XmMessageBoxGetChild(dialog, XmDIALOG_CANCEL_BUTTON));
 XtUnmanageChild(XmMessageBoxGetChild(dialog, XmDIALOG_HELP_BUTTON));
 XtAddCallback(dialog, XmNokCallback, (XtCallbackProc) XtDestroyWidget, NULL);
 /* disable some of the menu functions */
 XtVaGetValues (dialog, XmNmwmFunctions, &menus, NULL);
 menus &= ~MWM_FUNC_RESIZE;
 /*printf("menus = %#x, MWM_FUNC_RESIZE = %#x\n", menus, MWM_FUNC_RESIZE);*/
 XtVaSetValues (dialog, XmNmwmFunctions, menus, NULL);
 /* change the message font */
 if (narg > 1) {
  n = 0;
  if (set_fontlist( ps[1] ) != 1) return -1; /* gets fontlist from string */
  XtSetValues(XmMessageBoxGetChild(dialog, XmDIALOG_MESSAGE_LABEL), wargs, n);
 }

 XtManageChild(dialog);
 XmStringFree(text);
 XmStringFree(title);
 if (narg < 4) return 1;
 n = 0;
 /* optional screen position */
 /* this looks stupid but changing the position before managing doesn't
 seem to work */
 if (narg > 3) {
 	if (int_arg_stat(ps[3], &ix) != 1) return -1;
	x = ix;
	XtSetArg(wargs[n], XmNx, x); n++; }
 if (narg > 4) {
 	if (int_arg_stat(ps[4], &iy) != 1) return -1;
	y = iy;
	XtSetArg(wargs[n], XmNy, y); n++; }
 XtSetValues(dialog, wargs, n);
 return 1;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmprompt(narg, ps)	/* see next line for args */
 /* (string, default, call_back, modal_flag, [font,color,ix,iy]) */
 /* a temporary widget, does not return a widget id, used as an lux subr */
 Int     narg, ps[];
 {
 Int	iq, nsym, modal_flag = 0, ix, iy;
 char *s, *sd, *sm;
 Widget dialog, parent;
 XmString text;
 Position	x, y;
 /* pops a motif message dialog with the string passed */
 if (ck_motif() != 1) return -1;
 iq = ps[0];	if (sym[iq].class != 2) { return execute_error(70); }
 sm = (char *) sym[iq].spec.array.ptr;
 iq = ps[1];	if (sym[iq].class != 2) { return execute_error(70); }
 sd = (char *) sym[iq].spec.array.ptr;
 /* get callback string */
 iq = ps[2];
 if (sym[iq].class != 2) { return execute_error(70); }
 s = (char *) sym[iq].spec.array.ptr;
 if (strlen(s) == 0 )  nsym = 0;  else	nsym = lux_execute_symbol(s,1);
 parent = lux_widget_id[toplevel];
 n = 0;
 dialog = XmCreatePromptDialog(parent, "prompt", NULL, 0);
 if (narg > 3)  if (int_arg_stat(ps[3], &modal_flag) != 1) return -1;
 if (modal_flag != 0)
  { XtSetArg(wargs[n], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL); n++;}

 text = XmStringCreateLtoR(sm, cset);
 XtSetArg(wargs[n], XmNselectionLabelString, text); n++;
 XtSetValues(dialog, wargs, n);
 /* junk the help button */
 XtUnmanageChild(XmSelectionBoxGetChild(dialog, XmDIALOG_HELP_BUTTON));
 /* load a default entry in the text field */
 XmTextSetString(XmSelectionBoxGetChild(dialog, XmDIALOG_TEXT), sd);
 n = 0;
 if (narg > 4) {
  if (set_fontlist( ps[4] ) != 1) return -1; /* gets fontlist from string */
 }
 XtSetValues(XmSelectionBoxGetChild(dialog, XmDIALOG_OK_BUTTON), wargs, n);
 XtSetValues(XmSelectionBoxGetChild(dialog, XmDIALOG_CANCEL_BUTTON), wargs, n);
 XtSetValues(XmSelectionBoxGetChild(dialog, XmDIALOG_SELECTION_LABEL), wargs, n);
 if (narg > 5) {
  if (setup_colors (ps[5] ) != 1) return -1; }	/* do colors based on bg */
 XtSetValues(XmSelectionBoxGetChild(dialog, XmDIALOG_TEXT), wargs, n);
 /* need to change the way prompt works so that more is done here in C,
 for the moment we use the same lux callback for OK and cancel, the
 user has to make sense of it if possible! Both destroy the widget. */
 XtAddCallback(dialog, XmNcancelCallback, selectionbox_cb, (XtPointer) nsym);
 XtAddCallback(dialog, XmNokCallback, selectionbox_cb, (XtPointer) nsym);
 XtManageChild(dialog);
 XmStringFree(text);
 if (narg < 7) return 1;
 n = 0;
 /* optional screen position */
 /* this looks stupid but changing the position before managing doesn't
 seem to work */
 if (narg > 6) {
 	if (int_arg_stat(ps[6], &ix) != 1) return -1;
	x = ix;
	XtSetArg(wargs[n], XmNx, x); n++; }
 if (narg > 7) {
 	if (int_arg_stat(ps[7], &iy) != 1) return -1;
	y = iy;
	XtSetArg(wargs[n], XmNy, y); n++; }
 XtSetValues(dialog, wargs, n);
 return 1;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmcommand(narg, ps) /*see next line for args */
 /*(parent, callback, nvisible, [nmax, prompt,font,color])*/
 /* returns widget id, an lux function */
 /* creates a command widget, original intent is to experiment
 with this to develop a widget command interface for lux */
 Int     narg, ps[];
 {
 Int	iq, nsym, parent, nvisible, hmax = 100;
 char	*s;
 Int	command, result_sym;
 XmString	prompt;
 Widget		wg;
 /* command widget interface */
 if (ck_motif() != 1) return -1;
 /* get parent widget */
 if ( get_widget_id( ps[0], &parent) != 1 ) return -1;
 /* get callback string */
 iq = ps[1];
 if (sym[iq].class != 2) { return execute_error(70); }
 s = (char *) sym[iq].spec.array.ptr;
 if (strlen(s) == 0 )  nsym = 0;  else	nsym = lux_execute_symbol(s,1);
 if (int_arg_stat(ps[2], &nvisible) != 1) return -1;
 if (narg > 3) { if (int_arg_stat(ps[3], &hmax) != 1) return -1; }
 
 n = 0;
 XtSetArg(wargs[n], XmNhistoryVisibleItemCount, nvisible); n++;
 XtSetArg(wargs[n], XmNhistoryMaxItems, hmax); n++;
 /* the optional prompt */
 if (narg > 4) {
 iq = ps[4];
 if (sym[iq].class != 2) { return execute_error(70); }
 s = (char *) sym[iq].spec.array.ptr;
 prompt = XmStringCreateSimple( s);
 XtSetArg(wargs[n], XmNpromptString, prompt); n++;
 }
 /* now the optional font and color */
 /* these have to set some of the children widgets, not the command widget */
 if (ck_widget_count() < 0) return -1;
 command = n_widgets++;
 lux_widget_id[command] = XmCreateCommand(lux_widget_id[parent],
 	"command",wargs, n);
 XtAddCallback(lux_widget_id[command], XmNcommandEnteredCallback,
                command_callback, (XtPointer) nsym);
 /* now the optional font and color */
 /* these have to set some of the children widgets, not the command widget */
 /* we don't set the color for the prompt */
 n = 0;
 if (narg > 5) {
  if (set_fontlist( ps[5] ) != 1) return -1; /* gets fontlist from string */
 }
 if (narg > 6) {
  if (setup_colors (ps[6] ) != 1) return -1; }	/* do colors based on bg */
 /* if n is still 0, no color or font, so skip the following section */
 if (n != 0) {
 wg = XmCommandGetChild(lux_widget_id[command], XmDIALOG_COMMAND_TEXT);
 XtSetValues(wg, wargs, n);
 wg = XmCommandGetChild(lux_widget_id[command], XmDIALOG_HISTORY_LIST);
 XtSetValues(wg, wargs, n);
 /* the prompt label just gets the font, so re-do the font args */
 n = 0;
 if (narg > 5) {
  if (set_fontlist( ps[5] ) != 1) return -1; /* gets fontlist from string */
  wg = XmCommandGetChild(lux_widget_id[command], XmDIALOG_PROMPT_LABEL);
  XtSetValues(wg, wargs, n);
 }
 }
 /* now that everybody is ready */

 XtManageChild(lux_widget_id[command]);
 XmStringFree(prompt);
 result_sym = scalar_scratch(2);		/*for widget id */
 sym[result_sym].spec.scalar.l = command;
 return result_sym;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmdialog_board(narg, ps)	/* see next line for args */
 /* (parent, width, height, title, [ix, iy, lr_margin, tb_margin,
 resize_flag]) */
 /* note that the close_flag idea didn't work, code for it commented out, the
 last arg. is now the resize_flag,   5/18/96 */
 /* 5/29/95,  added ix, iy for positioning on screen */
 /* create a dialog bulletin board widget, usually the main container */
 /* returns widget id, an lux function */
 Int     narg, ps[];
 {
 Int	dx, dy, parent, board, result_sym, iq, ix, iy, mx=0, my=0;
 Int	resize_flag = 0, ck_widget_count(void);
 XmString	title;
 Widget		wb;
 Position	x, y;
 char	*s;
 if (ck_motif() != 1) return -1;
 /* get parent widget */
 if ( get_widget_id( ps[0], &parent) != 1 ) return -1;
 /* get the arguments */
 if (int_arg_stat(ps[1], &dx) != 1) return -1;
 if (int_arg_stat(ps[2], &dy) != 1) return -1;
 iq = ps[3];
 if (sym[iq].class != 2) { return execute_error(70); }
 s = (char *) sym[iq].spec.array.ptr;
 title = XmStringCreateSimple( s);
 n = 0;
 XtSetArg(wargs[n], XmNwidth, dx); n++;
 XtSetArg(wargs[n], XmNheight, dy); n++;
 XtSetArg(wargs[n], XmNdialogTitle, title); n++;
 XtSetArg(wargs[n], XmNautoUnmanage, False); n++;
 /* for boards, we probably don't want resize_any but may want resize_grow */
 /* XtSetArg(wargs[n], XmNresizePolicy, XmRESIZE_ANY); n++; */
 XtSetArg(wargs[n], XmNresizePolicy, XmRESIZE_GROW); n++;
 /* set margins */
 if (narg >6 )  {if (int_arg_stat(ps[6], &mx) != 1) return -1;}
 XtSetArg(wargs[n], XmNmarginWidth, mx); n++;
 if (narg >7 )  {if (int_arg_stat(ps[7], &my) != 1) return -1;}
 XtSetArg(wargs[n], XmNmarginHeight, my); n++;
 /*if (narg >6) {XtSetArg(wargs[n], XmNdeleteResponse, XmDO_NOTHING); n++;}*/
 /* XtSetArg(wargs[n], XmNborderWidth, (Dimension) 10); n++; */
 /* optional screen position */
 if (narg > 4) {
 	if (int_arg_stat(ps[4], &ix) != 1) return -1;
	x = ix;
	XtSetArg(wargs[n], XmNx, x); n++; }
 if (narg > 5) {
 	if (int_arg_stat(ps[5], &iy) != 1) return -1;
	y = iy;
	XtSetArg(wargs[n], XmNy, y); n++; }
 if (narg > 8) {
  if (int_arg_stat(ps[8], &resize_flag) != 1) return -1; }
 if (resize_flag == 0) {
   XtSetArg(wargs[n], XmNnoResize, True);
   n++;				/* put in body of if statement. LS 1apr99 */
 }
 if (ck_widget_count() < 0) return -1;
 board = n_widgets++;
 lux_widget_id[board] = wb =
 	XmCreateBulletinBoardDialog(lux_widget_id[parent], "board",wargs, n);
 
 /* this didn't work
 if (narg > 8) {
  if (int_arg_stat(ps[8], &close_flag) != 1) return -1; }
 if (close_flag == 0) {
 XtVaGetValues (wb, XmNmwmFunctions, &funs, NULL);
 funs &= ~MWM_FUNC_CLOSE;
 printf("funs = %#x, MWM_FUNC_CLOSE = %#x\n", funs, MWM_FUNC_CLOSE);
 XtVaSetValues (wb, XmNmwmFunctions, funs, NULL);
 }
 end of old close_flag code */
 XmStringFree(title);
 result_sym = scalar_scratch(2);		/*for widget id */
 sym[result_sym].spec.scalar.l = board;
 return result_sym;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmdialog_form(narg, ps)	/* see next line for args */
 /* (parent, width, height, title, [ hspace, vspace, lr_margin, tb_margin) */
 /* 5/19/96 */
 /* create a dialog form widget, usually the main container */
 /* returns widget id, an lux function */
 Int     narg, ps[];
 {
 Int	dx, dy, parent, form, result_sym, iq, hs=0, vs=0, mx=0, my=0;
 XmString	title;
 Widget		wb;
 char	*s;
 if (ck_motif() != 1) return -1;
 /* get parent widget */
 if ( get_widget_id( ps[0], &parent) != 1 ) return -1;
 /* get the arguments */
 if (int_arg_stat(ps[1], &dx) != 1) return -1;
 if (int_arg_stat(ps[2], &dy) != 1) return -1;
 iq = ps[3];
 if (sym[iq].class != 2) { return execute_error(70); }
 s = (char *) sym[iq].spec.array.ptr;
 title = XmStringCreateSimple( s);
 n = 0;
 XtSetArg(wargs[n], XmNwidth, dx); n++;
 XtSetArg(wargs[n], XmNheight, dy); n++;
 XtSetArg(wargs[n], XmNdialogTitle, title); n++;
 XtSetArg(wargs[n], XmNautoUnmanage, False); n++;
 XtSetArg(wargs[n], XmNresizePolicy, XmRESIZE_ANY); n++;
 /* set margins */
 if (narg >6 )  {if (int_arg_stat(ps[6], &mx) != 1) return -1;}
 XtSetArg(wargs[n], XmNmarginWidth, mx); n++;
 if (narg >7 )  {if (int_arg_stat(ps[7], &my) != 1) return -1;}
 XtSetArg(wargs[n], XmNmarginHeight, my); n++;
 /* horizontal and vertical spacings */
 if (narg > 4) { if (int_arg_stat(ps[4], &hs) != 1) return -1; }
 if (narg > 5) { if (int_arg_stat(ps[5], &vs) != 1) return -1; }
 XtSetArg(wargs[n], XmNhorizontalSpacing, hs); n++;
 XtSetArg(wargs[n], XmNverticalSpacing, vs); n++;
 if (ck_widget_count() < 0) return -1;
 form = n_widgets++;
 lux_widget_id[form] = wb =
 	XmCreateFormDialog(lux_widget_id[parent], "form",wargs, n);
 
 XmStringFree(title);
 result_sym = scalar_scratch(2);		/*for widget id */
 sym[result_sym].spec.scalar.l = form;
 return result_sym;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmtoplevel_form(narg, ps)	/* see next line for args */
 /* (width, height, title, [ hspace, vspace, lr_margin, tb_margin) */
 /* 12/26/97 */
 /* create a toplevel form widget, does not popup, returns the form
 widget created inside the toplevel */
 /* returns widget id, an lux function */
 /* similar to lux_xmdialog_form except that this is toplevel, do not use
 manage and unmanage, instead use popup and popdown */
 Int     narg, ps[];
 {
 Int	dx, dy, form, result_sym, iq, hs=0, vs=0, mx=0, my=0;
 char	*s;
 if (ck_motif() != 1) return -1;
 /* get the arguments */
 if (int_arg_stat(ps[0], &dx) != 1) return -1;
 if (int_arg_stat(ps[1], &dy) != 1) return -1;
 iq = ps[2];
 if (sym[iq].class != 2) { return execute_error(70); }
 s = (char *) sym[iq].spec.array.ptr;
 if (narg >5 )  {if (int_arg_stat(ps[5], &mx) != 1) return -1;}
 if (narg >6 )  {if (int_arg_stat(ps[6], &my) != 1) return -1;}
 /* horizontal and vertical spacings */
 if (narg > 3) { if (int_arg_stat(ps[3], &hs) != 1) return -1; }
 if (narg > 4) { if (int_arg_stat(ps[4], &vs) != 1) return -1; }
 if (ck_widget_count() < 0) return -1;
 form = n_widgets++;
 lux_widget_id[form] = xmtoplevel_form(dx, dy, s, hs, vs, mx, my);
 
 result_sym = scalar_scratch(2);		/*for widget id */
 sym[result_sym].spec.scalar.l = form;
 return result_sym;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmtoplevel_board(narg, ps)	/* see next line for args */
 /* (width, height, title, [lr_margin, tb_margin]) */
 /* 5/17/98 */
 /* create a toplevel board widget, does not popup, returns the form
 widget created inside the toplevel */
 /* returns widget id, an lux function */
 /* similar to lux_xmdialog_board except that this is toplevel, do not use
 manage and unmanage, instead use popup and popdown */
 Int     narg, ps[];
 {
 Int	dx, dy, board, result_sym, iq, mx=0, my=0;
 char	*s;
 if (ck_motif() != 1) return -1;
 /* get the arguments */
 if (int_arg_stat(ps[0], &dx) != 1) return -1;
 if (int_arg_stat(ps[1], &dy) != 1) return -1;
 iq = ps[2];
 if (sym[iq].class != 2) { return execute_error(70); }
 s = (char *) sym[iq].spec.array.ptr;
 if (narg > 3) { if (int_arg_stat(ps[3], &mx) != 1) return -1; }
 if (narg > 4) { if (int_arg_stat(ps[4], &my) != 1) return -1; }
 if (ck_widget_count() < 0) return -1;
 board = n_widgets++;
 lux_widget_id[board] = xmtoplevel_board(dx, dy, s, mx, my);
 result_sym = scalar_scratch(2);		/*for widget id */
 sym[result_sym].spec.scalar.l = board;
 return result_sym;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmboard(narg, ps)	/* (parent, [width, height, lr_margin, tb_margin]) */
 /* create a managed bulletin board widget */
 /* returns widget id, an lux function */
 Int     narg, ps[];
 {
 Int	dx=0, dy=0, parent, board, result_sym, mx=0, my=0;
 if (ck_motif() != 1) return -1;
 /* get parent widget */
 if ( get_widget_id( ps[0], &parent) != 1 ) return -1;
 /* get the arguments */
 
 if (narg >1 )  {if (int_arg_stat(ps[1], &dx) != 1) return -1; }
 if (narg >2 )  {if (int_arg_stat(ps[2], &dy) != 1) return -1; }
 n = 0;
 XtSetArg(wargs[n], XmNwidth, dx); n++;
 XtSetArg(wargs[n], XmNheight, dy); n++;
 /* set margins */
 if (narg >3 )  {if (int_arg_stat(ps[3], &mx) != 1) return -1;}
 XtSetArg(wargs[n], XmNmarginWidth, mx); n++;
 if (narg >4 )  {if (int_arg_stat(ps[4], &my) != 1) return -1;}
 XtSetArg(wargs[n], XmNmarginHeight, my); n++;
 if (ck_widget_count() < 0) return -1;
 board = n_widgets++;
 lux_widget_id[board] =
 	XmCreateBulletinBoard(lux_widget_id[parent], "board",wargs, n);
 XtManageChild(lux_widget_id[board]);
 result_sym = scalar_scratch(2);		/*for widget id */
 sym[result_sym].spec.scalar.l = board;
 return result_sym;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmform(narg, ps)	/* (parent, [width,height,hspace,vspace,mx,my]) */
 /* returns widget id, an lux function */
 Int     narg, ps[];
 {
 Int	dx=0, dy=0, parent, form, result_sym, hs=0, vs=0, mx=0, my=0;
 if (ck_motif() != 1) return -1;
 /* get parent widget */
 if ( get_widget_id( ps[0], &parent) != 1 ) return -1;
 /* get the arguments */
 if (narg >1 )  { if (int_arg_stat(ps[1], &dx) != 1) return -1; }
 if (narg >2 )  { if (int_arg_stat(ps[2], &dy) != 1) return -1; }
 if (narg >3 )  { if (int_arg_stat(ps[3], &hs) != 1) return -1; }
 if (narg >4 )  { if (int_arg_stat(ps[4], &vs) != 1) return -1; }
 if (narg >5 )  { if (int_arg_stat(ps[5], &mx) != 1) return -1; }
 if (narg >6 )  { if (int_arg_stat(ps[6], &my) != 1) return -1; }
 n = 0;
 XtSetArg(wargs[n], XmNwidth, dx); n++;
 XtSetArg(wargs[n], XmNheight, dy); n++;
 XtSetArg(wargs[n], XmNresizable, TRUE); n++;
 XtSetArg(wargs[n], XmNhorizontalSpacing, hs); n++;
 XtSetArg(wargs[n], XmNverticalSpacing, vs); n++;
 /* set margins */
 if (narg >5 )  {if (int_arg_stat(ps[5], &mx) != 1) return -1;}
 XtSetArg(wargs[n], XmNmarginWidth, mx); n++;
 if (narg >6 )  {if (int_arg_stat(ps[6], &my) != 1) return -1;}
 XtSetArg(wargs[n], XmNmarginHeight, my); n++;
 if (ck_widget_count() < 0) return -1;
 form = n_widgets++;
 lux_widget_id[form] = XmCreateForm(lux_widget_id[parent], "form",wargs, n);
 XtManageChild(lux_widget_id[form]);
 result_sym = scalar_scratch(2);		/*for widget id */
 sym[result_sym].spec.scalar.l = form;
 return result_sym;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmframe(narg, ps) /* (parent, [lr_margin, tb_margin, width, type]) */
 /* create a frame widget */
 /* returns widget id, an lux function */
 Int     narg, ps[];
 {
 Int	dx, parent, frame, result_sym, type, mx, my;
 if (ck_motif() != 1) return -1;
 /* get parent widget */
 n = 0;
 if ( get_widget_id( ps[0], &parent) != 1 ) return -1;
 if (narg >1 )  {
 	if (int_arg_stat(ps[1], &mx) != 1) return -1;
	XtSetArg(wargs[n], XmNmarginWidth, mx); n++; }
 if (narg >2 )  {
 	if (int_arg_stat(ps[2], &my) != 1) return -1;
	XtSetArg(wargs[n], XmNmarginHeight, my); n++; }
 /* get the width and shadow type */
 if (narg >3 )  {
 	if (int_arg_stat(ps[3], &dx) != 1) return -1;
	XtSetArg(wargs[n], XmNshadowThickness, dx); n++; }
 if (narg >4 )  {
 	if (int_arg_stat(ps[4], &type) != 1) return -1;
  	switch (type) {
  case 0: XtSetArg(wargs[n], XmNshadowType, XmSHADOW_ETCHED_IN); n++; break;
  case 1: XtSetArg(wargs[n], XmNshadowType, XmSHADOW_ETCHED_OUT); n++; break;
  case 2: XtSetArg(wargs[n], XmNshadowType, XmSHADOW_IN); n++; break;
  case 3: XtSetArg(wargs[n], XmNshadowType, XmSHADOW_OUT); n++; break;
  	}
 }
 if (ck_widget_count() < 0) return -1;
 frame = n_widgets++;
 lux_widget_id[frame] =
 	XmCreateFrame(lux_widget_id[parent], "frame",wargs, n);
 XtManageChild(lux_widget_id[frame]);
 result_sym = scalar_scratch(2);		/*for widget id */
 sym[result_sym].spec.scalar.l = frame;
 return result_sym;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmseparator(narg, ps) /* (parent, [orient, margin, type]) */
 /* orient = 0 for horozontal, otherwise vertical
 margin is width or height
 type is 1 for simple line, 0 for empty space, 2 for Double line, 3 for dotted
 line, 5 for etched line */
 /* returns widget id, an lux function */
 Int     narg, ps[];
 {
 Int	margin, parent, sep, result_sym, type=1, orient=0;
 if (ck_motif() != 1) return -1;
 /* get parent widget */
 n = 0;
 if ( get_widget_id( ps[0], &parent) != 1 ) return -1;
 if (narg >1 )  {
 	if (int_arg_stat(ps[1], &orient) != 1) return -1;
	if (orient == 0) XtSetArg(wargs[n], XmNorientation, XmHORIZONTAL);
	else  XtSetArg(wargs[n], XmNorientation, XmVERTICAL); n++; }
 if (narg >2 )  {
 	if (int_arg_stat(ps[2], &margin) != 1) return -1;
	XtSetArg(wargs[n], XmNmargin, margin); n++; }
 /* get the type */
 if (narg >3 )  {
 	if (int_arg_stat(ps[3], &type) != 1) return -1;
	type = MAX(type, 0);	type = MIN(type,8);
	XtSetArg(wargs[n], XmNseparatorType, type); n++;
 }
 if (ck_widget_count() < 0) return -1;
 sep = n_widgets++;
 lux_widget_id[sep] =
 	XmCreateSeparator(lux_widget_id[parent], "separator",wargs, n);
 XtManageChild(lux_widget_id[sep]);
 result_sym = scalar_scratch(2);		/*for widget id */
 sym[result_sym].spec.scalar.l = sep;
 return result_sym;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmcolumns(narg, ps)	/* (parent, width, height, [ncolumns],[pack_flag]) */
 /* returns widget id, an lux function */
 Int     narg, ps[];
 { return rows_or_columns(narg, ps, 0); }
 /*------------------------------------------------------------------------- */
Int lux_xmrows(narg, ps)	/* (parent, width, height, [nrows],[pack_flag]) */
 /* returns widget id, an lux function */
 Int     narg, ps[];
 { return rows_or_columns(narg, ps, 1); }
 /*------------------------------------------------------------------------- */
Int rows_or_columns(narg, ps,mode)	/*(parent, width, height, [ncolumns],[p_flag]) */
 /* returns widget id, an lux function */
 Int     narg, ps[], mode;
 {
 Int	dx, dy, parent,rowcolumn, result_sym, ncol, p_flag = 0;
 if (ck_motif() != 1) return -1;
 /* get parent widget */
 if ( get_widget_id( ps[0], &parent) != 1 ) return -1;
 /* get the arguments */
 if (int_arg_stat(ps[1], &dx) != 1) return -1;
 if (int_arg_stat(ps[2], &dy) != 1) return -1;
 if (narg > 3) if (int_arg_stat(ps[3], &ncol) != 1) return -1;
 if (narg > 4) if (int_arg_stat(ps[4], &p_flag) != 1) return -1;
 n = 0;
 XtSetArg(wargs[n], XmNwidth, dx); n++;
 XtSetArg(wargs[n], XmNheight, dy); n++;
 printf("p_flag = %d\n", p_flag);
 if (p_flag) XtSetArg(wargs[n], XmNpacking, XmPACK_COLUMN);
 	else XtSetArg(wargs[n], XmNpacking, XmPACK_TIGHT);
 n++;
 if (mode == 0) XtSetArg(wargs[n], XmNorientation, XmVERTICAL);
 	else	XtSetArg(wargs[n], XmNorientation, XmHORIZONTAL);
 
 n++;
 if (narg > 3) { XtSetArg(wargs[n], XmNnumColumns, ncol); n++; }
 if (ck_widget_count() < 0) return -1;
 rowcolumn = n_widgets++;
 lux_widget_id[rowcolumn] =
 	XmCreateRowColumn(lux_widget_id[parent], "rowcolumn",wargs, n);
 XtManageChild(lux_widget_id[rowcolumn]);
 result_sym = scalar_scratch(2);		/*for widget id */
 sym[result_sym].spec.scalar.l = rowcolumn;
 return result_sym;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmscrolledwindow(narg, ps)	/* (parent, width, height) */
 /* create a scrolled window */
 /* returns widget id, an lux function */
 Int     narg, ps[];
 {
 Int	dx, dy, parent, scroll, result_sym;
 if (ck_motif() != 1) return -1;
 /* get parent widget */
 if ( get_widget_id( ps[0], &parent) != 1 ) return -1;
 /* get the arguments */
 if (int_arg_stat(ps[1], &dx) != 1) return -1;
 if (int_arg_stat(ps[2], &dy) != 1) return -1;
 n = 0;
 XtSetArg(wargs[n], XmNwidth, dx); n++;
 XtSetArg(wargs[n], XmNheight, dy); n++;
 XtSetArg(wargs[n], XmNscrollingPolicy, XmAUTOMATIC); n++;
 if (ck_widget_count() < 0) return -1;
 scroll = n_widgets++;
 lux_widget_id[scroll] =
 	XmCreateScrolledWindow(lux_widget_id[parent], "scroll",wargs, n);
 XtManageChild(lux_widget_id[scroll]);
 result_sym = scalar_scratch(2);		/*for widget id */
 sym[result_sym].spec.scalar.l = scroll;
 return result_sym;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmscrolledwindowapp(narg, ps)	/* (parent,hmargin,vmargin,space) */
 /* create a scrolled window, application controlled */
 /* returns widget id, an lux function */
 Int     narg, ps[];
 {
 Int	mx, my, parent, scroll, result_sym, space;
 if (ck_motif() != 1) return -1;
 /* get parent widget */
 if ( get_widget_id( ps[0], &parent) != 1 ) return -1;
 n = 0;
 if (narg >1 )  {
 	if (int_arg_stat(ps[1], &mx) != 1) return -1;
	XtSetArg(wargs[n], XmNscrolledWindowMarginWidth, mx); n++; }
 if (narg >2 )  {
 	if (int_arg_stat(ps[2], &my) != 1) return -1;
	XtSetArg(wargs[n], XmNscrolledWindowMarginHeight, my); n++; }
 if (narg >3 )  {
 	if (int_arg_stat(ps[2], &space) != 1) return -1;
	XtSetArg(wargs[n], XmNspacing, space); n++; }
 if (ck_widget_count() < 0) return -1;
 scroll = n_widgets++;
 lux_widget_id[scroll] =
 	XmCreateScrolledWindow(lux_widget_id[parent], "scroll",wargs, n);
 XtManageChild(lux_widget_id[scroll]);
 result_sym = scalar_scratch(2);		/*for widget id */
 sym[result_sym].spec.scalar.l = scroll;
 return result_sym;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmhscrollbar(narg, ps)/*(parent,max,slidersize,dpage,callback,[drag]) */
 /* creates a horizontal scroll widget, returns widget id */
 Int     narg, ps[];
 {
 return lux_int_xmscrollbar(narg, ps, 1);	/* the 1 indicates horizontal */
 }
 /*------------------------------------------------------------------------- */
Int lux_xmvscrollbar(narg, ps)/*(parent,max,slidersize,dpage,callback,[drag]) */
 /* creates a vertical scroll widget, returns widget id */
 Int     narg, ps[];
 {
 return lux_int_xmscrollbar(narg, ps, 0);	/* the 0 indicates vertical */
 }
 /*------------------------------------------------------------------------- */
Int lux_int_xmscrollbar(narg, ps, mode)  /* internal, mode = 1 for horizontal */
 /*(parent,max,slidersize,dpage,callback,[drag]) */
 /* note that the default increment of 1 is used since the max can always
 be set so that 1 is the single step size, of course, we may change this
 someday for more flexibility */
 Int     narg, ps[], mode;
 {
 Int	iq, nsym, w, scroll, result_sym, slidersize, max, dpage;
 Int	drag_flag=0;
 char *s;
 if (ck_motif() != 1) return -1;
 /* get parent widget */
 if ( get_widget_id( ps[0], &w) != 1 ) return -1;
 /* get max and slider size*/
 if (int_arg_stat(ps[1], &max) != 1) return -1;
 if (int_arg_stat(ps[2], &slidersize) != 1) return -1;
 /* get page size for "page" jumps */
 if (int_arg_stat(ps[3], &dpage) != 1) return -1;
 /* get callback string */
 iq = ps[4];
 if (sym[iq].class != 2) { return execute_error(70); }
 s = (char *) sym[iq].spec.array.ptr;
 if (strlen(s) == 0 )  nsym = 0;  else	nsym = lux_execute_symbol(s,1);
 if (narg > 5) if (int_arg_stat(ps[5], &drag_flag) != 1) return -1;
 n = 0;
 XtSetArg(wargs[n], XmNmaximum, max); n++;
 XtSetArg(wargs[n], XmNsliderSize, slidersize); n++;
 XtSetArg(wargs[n], XmNpageIncrement, dpage); n++;
 if (mode) { XtSetArg(wargs[n], XmNorientation, XmHORIZONTAL); } else {
 	XtSetArg(wargs[n], XmNorientation, XmVERTICAL); } ; n++;
 if (ck_widget_count() < 0) return -1;
 scroll = n_widgets++;
 lux_widget_id[scroll] = XmCreateScrollBar(lux_widget_id[w], "scroll", wargs, n);
 XtManageChild(lux_widget_id[scroll]);
 XtAddCallback(lux_widget_id[scroll], XmNvalueChangedCallback,
                scroll_callback, (XtPointer) nsym);
 if (drag_flag != 0) {
 XtAddCallback(lux_widget_id[scroll], XmNdragCallback,
                scroll_callback, (XtPointer) nsym);
	}
 result_sym = scalar_scratch(2);		/*for widget id */
 sym[result_sym].spec.scalar.l = scroll;
 return result_sym;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmscrollbarsetvalues(narg, ps)/*(widget,value,max,slidersize,dpage) */
 /* changes the slider size and range (and dpage) */
 /* if a value is 0 or < 0, we assume it isn't supposed to be changed */
 Int     narg, ps[];
 {
 Int	w, value=-1, slidersize=-1, max=-1, dpage=-1;
 if (ck_motif() != 1) return -1;
 if (get_widget_id(ps[0], &w) != 1) return -1;
 n = 0;
 if (narg>1) { if (int_arg_stat(ps[1], &value) != 1) return -1; }
 if (narg>2) { if (int_arg_stat(ps[2], &max) != 1) return -1; }
 if (narg>3) { if (int_arg_stat(ps[3], &slidersize) != 1) return -1; }
 if (narg>4) { if (int_arg_stat(ps[4], &dpage) != 1) return -1; }

 if (value >= 0)		{ XtSetArg(wargs[n], XmNvalue, value); n++; }
 if (max>0)		{ XtSetArg(wargs[n], XmNmaximum, max); n++; }
 if (slidersize>0)	{  XtSetArg(wargs[n], XmNsliderSize, slidersize); n++; }
 if (dpage>0)		{ XtSetArg(wargs[n], XmNpageIncrement, dpage); n++; }
 
 if (n>0)	XtSetValues(lux_widget_id[w], wargs, n);
 return 1;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmdrawingarea(narg, ps)  /*(parent,lux_win,w,h,input_cb,resize_cb,ex_cb) */
 /* create a drawing window */
 /* returns widget id, an lux function */
 /* note that we can't get the window number until after the widgets
 are realized */
 Int     narg, ps[];
 {
 Widget	wdraw;
 Int	dx=0, dy=0, parent, draw, result_sym, iq, lux_win, nsym;
 char	*s;
 extern	Int	drawingareas[];
 extern	Int	ck_window();
 extern	 Window  win[];
 if (ck_motif() != 1) return -1;
 /* get parent widget */
 if ( get_widget_id( ps[0], &parent) != 1 ) return -1;
 /* get the arguments */
 if (int_arg_stat(ps[1], &lux_win) != 1) return -1;
 /* we want the lux X window to be positive and in range */
 /* we do not currently allow a negative lux window which is supposed to be
 a pixmap rather than a displayable window */
 if (lux_win<0 || ck_window(lux_win)<0 ) {
 	printf("XMDRAWINGAREA - illegal lux X window number %d\n", lux_win);
	return -1; }
 if (narg>2) { if (int_arg_stat(ps[2], &dx) != 1) return -1; }
 if (narg>3) { if (int_arg_stat(ps[3], &dy) != 1) return -1; }
 /* if the lux X window already exists, we have to delete it since the motif
 widget will create a new X window */
 if ( win[lux_win] != 0 ) lux_xdelete(1, &lux_win);
 n = 0;
 XtSetArg(wargs[n], XmNwidth, dx); n++;
 XtSetArg(wargs[n], XmNheight, dy); n++;
 XtSetArg(wargs[n], XmNresizePolicy, XmRESIZE_ANY); n++;
 if (ck_widget_count() < 0) return -1;
 draw = n_widgets++;
 lux_widget_id[draw] = wdraw =
 	XmCreateDrawingArea(lux_widget_id[parent], "draw",wargs, n);
 /* callbacks (if any), first the input callback */
 if (narg>4) {
 Int	*pt;
 iq = ps[4];
 if (sym[iq].class != 2) { return execute_error(70); }
 s = (char *) sym[iq].spec.array.ptr;
 if (strlen(s) == 0 )  nsym = 0;  else	nsym = lux_execute_symbol(s,1);
 /* instead of just passing the execution symbol, we make a little
 array containing it and the lux X window # and pass the pointer */
 pt = (Int *) malloc(2*sizeof(Int));
 *pt = nsym;	*(pt+1) = lux_win;
 XtAddCallback(wdraw, XmNinputCallback, draw_in_callback, (XtPointer) pt);
 }
 if (narg>5) {
 Int	*pt;
 iq = ps[5];
 if (sym[iq].class != 2) { return execute_error(70); }
 s = (char *) sym[iq].spec.array.ptr;
 if (strlen(s) == 0 )  nsym = 0;  else	nsym = lux_execute_symbol(s,1);
 /* instead of just passing the exection sysmbol, we make a little
 array containing it and the lux X window # and pass the pointer */
 pt = (Int *) malloc(2*sizeof(Int));
 *pt = nsym;	*(pt+1) = lux_win;
 XtAddCallback(wdraw, XmNresizeCallback, draw_re_callback, (XtPointer) pt);
 }
 if (narg>6) {
 Int	*pt;
 iq = ps[6];
 if (sym[iq].class != 2) { return execute_error(70); }
 s = (char *) sym[iq].spec.array.ptr;
 if (strlen(s) == 0 )  nsym = 0;  else	nsym = lux_execute_symbol(s,1);
 /* instead of just passing the exection sysmbol, we make a little
 array containing it and the lux X window # and pass the pointer */
 pt = (Int *) malloc(2*sizeof(Int));
 *pt = nsym;	*(pt+1) = lux_win;
 XtAddCallback(wdraw, XmNexposeCallback, draw_ex_callback, (XtPointer) pt);
 }
 XtManageChild(lux_widget_id[draw]);

 result_sym = scalar_scratch(2);		/*for widget id */
 sym[result_sym].spec.scalar.l = drawingareas[lux_win] = draw;
#ifdef DEPRECATED
 printf("created widget %d, %d\n", draw, (uint32_t) wdraw);
#endif
 /* the X window won't actually be created until the manager of this drawing
 area is managed, the final setups of the window are therefore deferred until
 the first access via the xport.c routines. In there, the normal checking
 will find that this is a new window associated with a drawing area in
 lux_xcreat */
 return result_sym;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmdrawinglink(narg, ps)	/* (widget, lux_win) */
 /* link a drawing area widget to an lux window */
 /* a subroutine */
 Int     narg, ps[];
 {
 extern	Window  win[];
 extern	 GC     gc[];
 extern	 Int     wd[], ht[];
 Int	w;
 uint32_t	valuemask=0;
 Display	*display;
 Bool	status;
 XWindowAttributes	wat;
 XSetWindowAttributes	setwat;
 Int	lux_win, screen_num;
 if (ck_motif() != 1) return -1;
 /* get widget */
 if ( get_widget_id( ps[0], &w) != 1 ) return -1;
 /* get the arguments */
 if (int_arg_stat(ps[1], &lux_win) != 1) return -1;
 display = XtDisplay(lux_widget_id[w]);
 screen_num = DefaultScreen(display);
 win[lux_win] = XtWindowOfObject(lux_widget_id[w]);
 printf("win[lux_win] = %ld\n", win[lux_win]);
 status = XGetWindowAttributes(display, win[lux_win], &wat);
 valuemask |= CWBackPixel;
 setwat.background_pixel = WhitePixel(display, screen_num);
 valuemask |= CWBackingStore;
 setwat.backing_store = WhenMapped; /*note that Always screws up things
					  not sure why yet */
 XChangeWindowAttributes(display, win[lux_win], valuemask, &setwat);
 /* set some of the lux parameters, assuming a window and not a pixmap
 for early testing */
 ht[lux_win] = wat.width;	wd[lux_win] = wat.height;
 gc[lux_win] = XCreateGC(display, RootWindow(display,screen_num),
 	0,NULL);
 XSetForeground(display, gc[lux_win], BlackPixel(display, screen_num));

 XSelectInput( display, win[lux_win], ExposureMask | KeyPressMask | 
    ButtonPressMask | StructureNotifyMask | FocusChangeMask );
 /*
 cursor = XCreateFontCursor(display, XC_crosshair);
 XDefineCursor(display, win[lux_win], cursor);
 XFreeCursor(display, cursor);
 */
 /*printf("win[lux_win] = %d, ht[lux_win] = %d, wd[lux_win] = %d\n",
 	win[lux_win], ht[lux_win], wd[lux_win]); */
 return 1;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmtextfromfile(narg, ps)/*(parent, filename, [rows, cols, font,color])*/
 /* returns widget id, an lux function */
 Int     narg, ps[];
 {
 Int	iq, i, text, result_sym, parent, rows = 24, cols = 80;
 char *filename, *buffer;
 FILE		*fin;
 struct	stat	file_info;
 if (ck_motif() != 1) return -1;
 /* get parent widget */
 if ( get_widget_id( ps[0], &parent) != 1 ) return -1;
 /* open the file */
 iq = ps[1];
 if (sym[iq].class != 2) { return execute_error(70); }
 filename = (char *) sym[iq].spec.array.ptr;
 if ((fin = fopen(filename, "r")) == NULL)
 	{ printf("could not open text file\n");  return -1; }
 if (stat(filename, &file_info) != 0)
 	{ printf("could not get file status\n");  return -1; }
 buffer = (char *) XtMalloc( file_info.st_size + 1);
 if (buffer == (char *) NULL)
 	{ printf("could not allocate buffer for text widget\n");  return -1; }
 i = fread(buffer,1,file_info.st_size, fin);
 buffer[file_info.st_size] = 0;
 fclose(fin);
 
 if (narg > 2) if (int_arg_stat(ps[2], &rows) != 1) return -1;
 if (narg > 3) if (int_arg_stat(ps[3], &cols) != 1) return -1;
 
 n = 0;
 XtSetArg(wargs[n], XmNrows, rows); n++;
 XtSetArg(wargs[n], XmNcolumns, cols); n++;
 /* now the optional font and color */
 if (narg > 4) {
  if (set_fontlist( ps[4] ) != 1) return -1; /* gets fontlist from string */
 }
 if (narg > 5) {
  if (setup_colors (ps[5] ) != 1) return -1; }	/* do colors based on bg */
 XtSetArg(wargs[n], XmNscrollHorizontal, True); n++;
 XtSetArg(wargs[n], XmNscrollVertical, True); n++;
 XtSetArg(wargs[n], XmNscrollLeftSide, True); n++;
 XtSetArg(wargs[n], XmNeditable, True); n++;
 XtSetArg(wargs[n], XmNeditMode, XmMULTI_LINE_EDIT); n++;
 XtSetArg(wargs[n], XmNresizable, TRUE); n++;
 if (ck_widget_count() < 0) return -1;
 text = n_widgets++;
 lux_widget_id[text] = XmCreateScrolledText(lux_widget_id[parent],
 	"text",wargs, n);
 XmTextSetString(lux_widget_id[text], buffer);
 XtFree(buffer);
 XtManageChild(lux_widget_id[text]);
 result_sym = scalar_scratch(2);		/*for widget id */
 sym[result_sym].spec.scalar.l = text;
 return result_sym;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmtext(narg, ps)/*(parent, rows, cols, [font,color])*/
 /* creates an empty text widget, scrollable */
 /* returns widget id, an lux function */
 /* note that the text widget is returned, to position this, you must
 use the scrolled text widget which is the parent of the returned one */
 Int     narg, ps[];
 {
 Int	text, result_sym, parent, rows = 24, cols = 80;
 if (ck_motif() != 1) return -1;
 /* get parent widget */
 if ( get_widget_id( ps[0], &parent) != 1 ) return -1;
 
 if (narg > 1) if (int_arg_stat(ps[1], &rows) != 1) return -1;
 if (narg > 2) if (int_arg_stat(ps[2], &cols) != 1) return -1;
 
 n = 0;
 XtSetArg(wargs[n], XmNrows, rows); n++;
 XtSetArg(wargs[n], XmNcolumns, cols); n++;
 /* now the optional font and color */
 if (narg > 3) {
  if (set_fontlist( ps[3] ) != 1) return -1; /* gets fontlist from string */
 }
 if (narg > 4) {
  if (setup_colors (ps[4] ) != 1) return -1; }	/* do colors based on bg */
 XtSetArg(wargs[n], XmNscrollHorizontal, True); n++;
 XtSetArg(wargs[n], XmNscrollVertical, True); n++;
 XtSetArg(wargs[n], XmNscrollLeftSide, True); n++;
 XtSetArg(wargs[n], XmNeditable, True); n++;
 XtSetArg(wargs[n], XmNeditMode, XmMULTI_LINE_EDIT); n++;
 if (ck_widget_count() < 0) return -1;
 text = n_widgets++;
 lux_widget_id[text] = XmCreateScrolledText(lux_widget_id[parent],
 	"text",wargs, n);
 XtManageChild(lux_widget_id[text]);
 result_sym = scalar_scratch(2);		/*for widget id */
 sym[result_sym].spec.scalar.l = text;
 return result_sym;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmtextbox(narg, ps)/*(parent, rows, cols, [font,color])*/
 /* creates an empty text widget, not scrolled, write only, not editable */
 /* returns widget id, an lux function */
 Int     narg, ps[];
 {
 Int	text, result_sym, parent, rows = 24, cols = 80;
 if (ck_motif() != 1) return -1;
 /* get parent widget */
 if ( get_widget_id( ps[0], &parent) != 1 ) return -1;
 
 if (narg > 1) if (int_arg_stat(ps[1], &rows) != 1) return -1;
 if (narg > 2) if (int_arg_stat(ps[2], &cols) != 1) return -1;
 
 n = 0;
 XtSetArg(wargs[n], XmNrows, rows); n++;
 XtSetArg(wargs[n], XmNcolumns, cols); n++;
 /* now the optional font and color */
 if (narg > 3) {
  if (set_fontlist( ps[3] ) != 1) return -1; /* gets fontlist from string */
 }
 if (narg > 4) {
  if (setup_colors (ps[4] ) != 1) return -1; }	/* do colors based on bg */
 XtSetArg(wargs[n], XmNeditable, False); n++;
 XtSetArg(wargs[n], XmNeditMode, XmMULTI_LINE_EDIT); n++;
 XtSetArg(wargs[n], XmNcursorPositionVisible, False); n++;
 if (ck_widget_count() < 0) return -1;
 text = n_widgets++;
 lux_widget_id[text] = XmCreateText(lux_widget_id[parent],
 	"text",wargs, n);
 XtManageChild(lux_widget_id[text]);
 result_sym = scalar_scratch(2);		/*for widget id */
 sym[result_sym].spec.scalar.l = text;
 return result_sym;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmtextappend(narg, ps)	/* (widget_id, string, [lf_flag]) */
 /* adds string to the text widget, this is a subroutine */
 /* if lf_flag is present and ne 0, then a line feed is included */
 Int     narg, ps[];
 {
 Int	iq, w, lflag=0, mq;
 XmTextPosition		i;
 char *s, *s2;
 if ( get_widget_id( ps[0], &w) != 1 ) return -1;
 iq = ps[1];
 if (sym[iq].class != 2) { return execute_error(70); }
 s = (char *) sym[iq].spec.array.ptr;
 i = XmTextGetLastPosition(lux_widget_id[w]);
 if (narg > 2) if (int_arg_stat(ps[2], &lflag) != 1) return -1;
 if (lflag == 0) XmTextInsert(lux_widget_id[w], i, s); else {
 /* you want a line feed too !, more complicated */
 mq = sym[iq].spec.array.bstore;	/* includes null */
 s2 = (char *) malloc(mq+1);			/* for the copy */
 bcopy(s, s2, mq-1);
 *(s2 + mq-1) = 10;
 *(s2 + mq) = 0;
 XmTextInsert(lux_widget_id[w], i, s2);
 free(s2);
 }
 return 1;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmtextgetstring(narg, ps) /* (widget) */
 /* gets a copy of the text contents as a string */
 Int     narg, ps[];
 {
 Int	w, mq, result_sym;
 char    *s, *p, *pt;
 if (ck_motif() != 1) return -1;
 if (get_widget_id(ps[0], &w) != 1) return -1;
 
 pt = XmTextGetString(lux_widget_id[w]);
 mq = strlen(pt);
 result_sym = string_scratch(mq);		/*for resultant string */
 s = (char *) sym[result_sym].spec.array.ptr;
 p = pt;
 while (mq--) *s++ = *p++;
 *s = 0;	/* ending null */
 XtFree(pt);
 return result_sym;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmtextsetstring(narg, ps) /* (widget, value) */
 Int     narg, ps[];
 {
 Int	w, iq;
 char    *s;
  if (ck_motif() != 1) return -1;
 if (get_widget_id(ps[0], &w) != 1) return -1;
 /* get the string */
 iq = ps[1];
 if (sym[iq].class != 2) return execute_error(70);
 s = (char *) sym[iq].spec.array.ptr;
 
 XmTextSetString(lux_widget_id[w], s);
 return 1;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmtextsetposition(narg, ps) /* (widget, position) */
 Int     narg, ps[];
 {
 Int	w, i;
  if (ck_motif() != 1) return -1;
 if (get_widget_id(ps[0], &w) != 1) return -1;
 if (int_arg_stat(ps[1], &i) != 1) return -1;
 
 XmTextSetInsertionPosition(lux_widget_id[w], (XmTextPosition) i);
 return 1;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmtextgetlastposition(narg, ps) /* (widget) */
 Int     narg, ps[];
 {
 Int	w, i, result_sym;
  if (ck_motif() != 1) return -1;
 if (get_widget_id(ps[0], &w) != 1) return -1;
 
 i = XmTextGetLastPosition(lux_widget_id[w]);
 result_sym = scalar_scratch(2);		/*for position */
 sym[result_sym].spec.scalar.l = i;
 return result_sym;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmtextgetinsertposition(narg, ps) /* (widget) */
 Int     narg, ps[];
 {
 Int	w, i, result_sym;
  if (ck_motif() != 1) return -1;
 if (get_widget_id(ps[0], &w) != 1) return -1;
 
 i = XmTextGetInsertionPosition(lux_widget_id[w]);
 result_sym = scalar_scratch(2);		/*for position */
 sym[result_sym].spec.scalar.l = i;
 return result_sym;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmtextgetselection(narg, ps) /* (widget, [left, right]) */
 /* returns the primary selection as a string, optionally returns the
 position */
 Int     narg, ps[];
 {
 Int	w, mq, result_sym;
 char    *s, *p, *pt;
 
 if (ck_motif() != 1) return -1;
 if (get_widget_id(ps[0], &w) != 1) return -1;
 
 pt = XmTextGetSelection(lux_widget_id[w]);
 mq = strlen(pt);
 result_sym = string_scratch(mq);		/*for resultant string */
 s = (char *) sym[result_sym].spec.array.ptr;
 p = pt;
 while (mq--) *s++ = *p++;
 *s = 0;	/* ending null */
 XtFree(pt);
 if (narg > 1) {
 /* get the position, this is optional */
 XmTextPosition	left;
 XmTextPosition	right;
 Int	i1, i2;
 if (narg !=3) execute_error(55);
 XmTextGetSelectionPosition (lux_widget_id[w], &left, &right);
 i1 = (Int) left;
 i2 = (Int) right;
 redef_scalar(ps[1], 2, &i1);
 redef_scalar(ps[2], 2, &i2);
 }
 return result_sym;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmtextreplace(narg, ps) /* (widget, i1, i2, s) */
 Int     narg, ps[];
 {
 Int	w, iq, i1, i2;
 char    *s;
  if (ck_motif() != 1) return -1;
 if (get_widget_id(ps[0], &w) != 1) return -1;
 if (int_arg_stat(ps[1], &i1) != 1) return -1;
 if (int_arg_stat(ps[2], &i2) != 1) return -1;
 if (narg > 3) {
   iq = ps[3];
   if (sym[iq].class != 2) { return execute_error(70); }
   s = (char *) sym[iq].spec.array.ptr;
 } else s = NULL;
 XmTextReplace(lux_widget_id[w],(XmTextPosition) i1,(XmTextPosition) i2, s);
 return 1;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmtexterase(narg, ps)	/* (widget_id) */
 /* erases all the text in a text widget */
 Int     narg, ps[];
 {
 Int	w;
 char *s = {""};
 if ( get_widget_id( ps[0], &w) != 1 ) return -1;
 XmTextSetString(lux_widget_id[w], s);
 return 1;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmtextfield(narg, ps) /* (parent, text, length, callback, ... see next line
	[font, color, lostfocus_callback, gotfocus_callback]) */
 /* creates a textfield widget, returns widget id */
 /* 9/14/96 added the optional focus callbacks */
 Int     narg, ps[];
 {
 Int	iq, nsym, w, textfield, result_sym, length;
 char *s, *ls;
 if (ck_motif() != 1) return -1;
 /* get parent widget */
 if ( get_widget_id( ps[0], &w) != 1 ) return -1;
 /* get label string */
 iq = ps[1];
 if (sym[iq].class != 2) { return execute_error(70); }
 ls = (char *) sym[iq].spec.array.ptr;
 if (int_arg_stat(ps[2], & length) != 1) return -1;
 /* get callback string */
 iq = ps[3];
 if (sym[iq].class != 2) { return execute_error(70); }
 s = (char *) sym[iq].spec.array.ptr;
 if (strlen(s) == 0 )  nsym = 0;  else	nsym = lux_execute_symbol(s,1);

 n = 0;
 XtSetArg(wargs[n], XmNmaxLength, length); n++;
 /* now the optional font and color */
 if (narg > 4) {
  if (set_fontlist( ps[4] ) != 1) return -1; /* gets fontlist from string */
 }
 if (narg > 5) {
  if (setup_colors (ps[5] ) != 1) return -1; }	/* do colors based on bg */
  
 if (ck_widget_count() < 0) return -1;
 textfield = n_widgets++;
 lux_widget_id[textfield] =
 	XmCreateTextField(lux_widget_id[w], "textfield", wargs, n);
 if (nsym) XtAddCallback(lux_widget_id[textfield], XmNactivateCallback,
                textfield_callback, (XtPointer) nsym);
 /* check for optional callbacks */
 if (narg > 6) {
 iq = ps[6];
 if (sym[iq].class != 2) { return execute_error(70); }
 s = (char *) sym[iq].spec.array.ptr;
 if (strlen(s) == 0 )  nsym = 0;  else	nsym = lux_execute_symbol(s,1);
 if (nsym) XtAddCallback(lux_widget_id[textfield], XmNlosingFocusCallback,
	textfield_callback, (XtPointer) nsym);
 }
 if (narg > 7) {
 iq = ps[7];
 if (sym[iq].class != 2) { return execute_error(70); }
 s = (char *) sym[iq].spec.array.ptr;
 if (strlen(s) == 0 )  nsym = 0;  else	nsym = lux_execute_symbol(s,1);
 if (nsym) XtAddCallback(lux_widget_id[textfield], XmNfocusCallback,
	textfield_callback, (XtPointer) nsym);
 }

 XmTextSetString(lux_widget_id[textfield], ls);
 XtManageChild(lux_widget_id[textfield]);
 result_sym = scalar_scratch(2);		/*for widget id */
 sym[result_sym].spec.scalar.l = textfield;
 return result_sym;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmtextfieldsetstring(narg, ps) /* (widget, string, [cursor]) */
 Int     narg, ps[];
 {
 Int	w, iq, cflag=0, n;
 char    *s;
  if (ck_motif() != 1) return -1;
 if (get_widget_id(ps[0], &w) != 1) return -1;
 /* get the string */
 iq = ps[1];
 if (sym[iq].class != 2) return execute_error(70);
 s = (char *) sym[iq].spec.array.ptr;
 
 XmTextFieldSetString(lux_widget_id[w], s);
 /* set the cursor at the end if cursor argument is not 0 */
 if (narg > 2) { if (int_arg_stat(ps[2], &cflag) != 1) return -1;
 if (cflag !=0) {
 n = sym[iq].spec.array.bstore - 1;
 XmTextFieldSetInsertionPosition(lux_widget_id[w], (XmTextPosition) n);
 }
 }
  return 1;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmtextfieldseteditable(narg, ps) /* (widget, value) */
 Int     narg, ps[];
 {
 Int	w, iq;
  if (ck_motif() != 1) return -1;
 if (get_widget_id(ps[0], &w) != 1) return -1;
 /* get the condition */
 if (int_arg_stat(ps[1], &iq) != 1) return -1;
 
 if (iq != 0) XmTextFieldSetEditable(lux_widget_id[w], True);  else
 	XmTextFieldSetEditable(lux_widget_id[w], False);
 return 1;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmtextfieldsetmaxlength(narg, ps) /* (widget, value) */
 Int     narg, ps[];
 {
 Int	w, iq;
  if (ck_motif() != 1) return -1;
 if (get_widget_id(ps[0], &w) != 1) return -1;
 /* get the condition */
 if (int_arg_stat(ps[1], &iq) != 1) return -1;
 
 XmTextFieldSetMaxLength(lux_widget_id[w], iq);
 return 1;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmtextseteditable(narg, ps) /* (widget, value) */
 Int     narg, ps[];
 {
 Int	w, iq;
  if (ck_motif() != 1) return -1;
 if (get_widget_id(ps[0], &w) != 1) return -1;
 /* get the condition */
 if (int_arg_stat(ps[1], &iq) != 1) return -1;
 
 if (iq != 0) XmTextSetEditable(lux_widget_id[w], True);  else
 	XmTextSetEditable(lux_widget_id[w], False);
 return 1;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmtextfieldgetstring(narg, ps) /* (widget) */
 Int     narg, ps[];
 {
 Int	w, mq, result_sym;
 char    *s, *p, *pt;
 if (ck_motif() != 1) return -1;
 if (get_widget_id(ps[0], &w) != 1) return -1;
 
 pt = XmTextFieldGetString(lux_widget_id[w]);
 mq = strlen(pt);
 result_sym = string_scratch(mq);		/*for resultant string */
 s = (char *) sym[result_sym].spec.array.ptr;
 p = pt;
 while (mq--) *s++ = *p++;
 *s = 0;	/* ending null */
 XtFree(pt);
 return result_sym;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmtextfieldarray(narg, ps)  /* lots of arguments */
 /* (parent,callback,font,color,nxl,nxm,nxt,ny,l1,[l2, l3 ...], [len]) */
 /* nxl is horiz. size of labels, nxm is margin, nxt is horiz. text size */
 /* the font and color preceed the labels to allow easier parsing, if
 these are null strings or scalars, you get the defaults */
 /* added 8/7/95, if the last argument is a scalar, it is the allowed
 length of each field, otherwise 10 is used as a default */
 Int     narg, ps[];
 {
 Int	iq, nsym, i, w, text, result_sym, j, k, *wids, nq;
 Int	nxl, nxm, nxt, ny, ix, iy, ix2, length;
 char *s;
 Widget	wg;
 struct	ahead	*h;
 XmString	ms;
 if (ck_motif() != 1) return -1;

 iq = ps[narg-1];	length = 10;
 if (sym[iq].class != 2) {
 /* check if a scalar, otherwise an error */
 if (int_arg_stat(iq, &length) != 1) {
  printf("XMTEXTFIELDARRAY, last arg neither a string nor a scalar\n");
  return -1;
 }
 narg--;	/* decrease for the new # of labels */
 }

 if (int_arg_stat(ps[4], &nxl) != 1) return -1;
 if (int_arg_stat(ps[5], &nxm) != 1) return -1;
 if (int_arg_stat(ps[6], &nxt) != 1) return -1;
 if (int_arg_stat(ps[7], &ny) != 1) return -1;

 /* get parent widget */
 if ( get_widget_id( ps[0], &w) != 1 ) return -1;
 /* note that callback is first here to provide for unknown # of labels */
 /* get callback string */
 iq = ps[1];
 if (sym[iq].class != 2) { return execute_error(70); }
 s = (char *) sym[iq].spec.array.ptr;
 if (strlen(s) == 0 )  nsym = 0;  else	nsym = lux_execute_symbol(s,1);

 /* before getting the labels, create a board widget as a container, we
 pass back this ID as first element in returned array */
 n = 0;
 wg = XmCreateBulletinBoard(lux_widget_id[w], "board",wargs, n);
 
 i = narg - 8;	j = 8;
 /* i is the count of labels, we are going to store the widget id's of the
 adjacent textfields in an array returned, but the first element will be the
 parent bulletin board widget so the the array will be i+1 long */
 nq=i+1;
 result_sym = array_scratch(2, 1, &nq );
 h = (struct ahead *) sym[result_sym].spec.array.ptr;
 wids = (Int *) ((char *)h + sizeof(struct ahead));
 if (ck_widget_count() < 0) return -1;
 text = n_widgets++;
 lux_widget_id[text] = wg;
 *wids++ = text;
 iy = 10;	/* set top y to default board margin */
 ix = nxm;	ix2 = nxl + 2*nxm;
 /* now the semi-optional font */
 n = 0;	/* be careful, n is bumped by set_fontlist and setup_colors */
 /* 3/21/98 - modified to not set colors of labels, hence 2 loops */
 if (set_fontlist( ps[2] ) != 1) return -1; /* gets fontlist from string */
 while (i--) {
 /* note that each of these use the font set above, no color here */
 /* first the label */
 /* get label string */
 iq = ps[j++];
 if (sym[iq].class != 2) { return execute_error(70); }
 s = (char *) sym[iq].spec.array.ptr;
 ms = XmStringCreateLtoR(s, cset);
 XtSetArg(wargs[n], XmNlabelString, ms); n++;
 XtSetArg(wargs[n], XmNx, ix); n++;
 XtSetArg(wargs[n], XmNy, iy); n++;
 XtSetArg(wargs[n], XmNwidth, nxl); n++;
 XtSetArg(wargs[n], XmNheight, ny); n++;
 XtSetArg(wargs[n], XmNalignment, XmALIGNMENT_END); n++;
 XtManageChild(XmCreateLabel(wg, "label", wargs, n));
 XmStringFree(ms);
 n = n - 6;	iy += ny;
 }
 
 /* now the textfield */
 n = 0;	/* be careful, n is bumped by set_fontlist and setup_colors */
 /* 3/21/98 - modified to not set colors of labels, hence 2 loops */
 if (set_fontlist( ps[2] ) != 1) return -1; /* gets fontlist from string */
 if (setup_colors (ps[3] ) != 1) return -1; /* do colors based on bg */
 i = narg - 8; k = 0;
 iy = 10;	/* set top y to default board margin */
 while (i--) {
 if (ck_widget_count() < 0) return -1;
 text = n_widgets++;
 *wids++ = text;
 XtSetArg(wargs[n], XmNx, ix2); n++;
 XtSetArg(wargs[n], XmNy, iy); n++;
 XtSetArg(wargs[n], XmNmaxLength, length); n++;
 XtSetArg(wargs[n], XmNwidth, nxt); n++;
 XtSetArg(wargs[n], XmNheight, ny); n++;
 lux_widget_id[text] =
 	XmCreateTextField(wg, "textfield", wargs, n);
 XtManageChild(lux_widget_id[text]);
 
 n = n -4;	iy += ny;

 /* call backs, 2 of them, one sets the textfield number */
 /* note that the textfield_which call back is activate, valuechanged
 didn't prove very useful */
 /*XtAddCallback(lux_widget_id[text], XmNvalueChangedCallback,
 textfield_which, k); */
 XtAddCallback(lux_widget_id[text], XmNactivateCallback, textfield_which,
 	(XtPointer) k);
 k++;
 XtAddCallback(lux_widget_id[text], XmNactivateCallback,
                textfield_callback, (XtPointer) nsym);
 }
 XtManageChild(wg);
 return result_sym;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmlistfromfile(narg, ps) /*(parent, callback, filename,
	nvisible,[font,color])*/
 /* returns widget id, an lux function */
 Int     narg, ps[];
 {
 Int	iq, nsym, i, parent, nvisible;
 char *s, *filename;
 char		line[200], *p;
 FILE		*fin;
 Int	list, result_sym;
 XmString	ms;
 /* list widget interface */
 if (ck_motif() != 1) return -1;
 /* get parent widget */
 if ( get_widget_id( ps[0], &parent) != 1 ) return -1;
 /* get callback string */
 iq = ps[1];
 if (sym[iq].class != 2) { return execute_error(70); }
 s = (char *) sym[iq].spec.array.ptr;
 if (strlen(s) == 0 )  nsym = 0;  else	nsym = lux_execute_symbol(s,1);
 /* printf("nsym for call back = %d\n", nsym); */
 /* open the file */
 iq = ps[2];
 if (sym[iq].class != 2) { return execute_error(70); }
 filename = (char *) sym[iq].spec.array.ptr;
 if ((fin = fopen(filename, "r")) == NULL)
 	{ printf("could not open list file\n"); fclose(fin);
	return 2; } /* for bad file, return a -1 as widget result */
 if (int_arg_stat(ps[3], &nvisible) != 1) return -1;
 
 nlist = 0;
 n = 0;
 XtSetArg(wargs[n], XmNselectionPolicy, XmBROWSE_SELECT); n++;
 XtSetArg(wargs[n], XmNlistSizePolicy, XmRESIZE_IF_POSSIBLE); n++;
 XtSetArg(wargs[n], XmNvisibleItemCount, nvisible); n++;
 /* now the optional font and color */
 if (narg > 4) {
  if (set_fontlist( ps[4] ) != 1) return -1; /* gets fontlist from string */
 }
 if (narg > 5) {
  if (setup_colors (ps[5] ) != 1) return -1; }	/* do colors based on bg */
 if (ck_widget_count() < 0) return -1;
 list = n_widgets++;
 lux_widget_id[list] = XmCreateScrolledList(lux_widget_id[parent],
 	"list",wargs, n);
 /* load the lines as items */
 while ( fgets(line, 200, fin) != NULL) {
  /* strip out the new line or we get a symbol in list */
  p = line;	i = 200;
  while ( i--) { if (*p++ == '\n') { *(p-1) = 0; break; } }
  ms = XmStringCreate(line, cset);
  XmListAddItemUnselected(lux_widget_id[list], ms, 0);
  XmStringFree(ms);
  nlist++;
 }
 fclose(fin);
 XtManageChild(lux_widget_id[list]);
 XtAddCallback(lux_widget_id[list], XmNbrowseSelectionCallback,
                browse_callback, (XtPointer) nsym);
 result_sym = scalar_scratch(2);		/*for widget id */
 sym[result_sym].spec.scalar.l = list;
 return result_sym;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmaddfiletolist(narg, ps) /*(list_widget, filename)*/
 /* an lux function, returns error if file not readable */
 Int     narg, ps[];
 {
 Int	iq, i, w;
 char *filename;
 char		line[200], *p;
 FILE		*fin;
 XmString	ms;
 if ( get_widget_id( ps[0], &w) != 1 ) return -1;

 /* open the file */
 iq = ps[1];
 if (sym[iq].class != 2) { return execute_error(70); }
 filename = (char *) sym[iq].spec.array.ptr;
 if ((fin = fopen(filename, "r")) == NULL)
 	{ printf("could not open list file: %s\n", filename); fclose(fin);
	return 4; } /* for bad file, return a 0 */
 
 /* load the lines as items */
 while ( fgets(line, 200, fin) != NULL) {
  /* strip out the new line or we get a symbol in list */
  p = line;	i = 200;
  while ( i--) { if (*p++ == '\n') { *(p-1) = 0; break; } }
  ms = XmStringCreate(line, cset);
  XmListAddItemUnselected(lux_widget_id[w], ms, 0);
  XmStringFree(ms);
 }
 fclose(fin);
 return 1;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmlistsubr(narg, ps) /*(list_widget)*/
 /* an lux subr, loads given list widget with internal subr names */
 Int     narg, ps[];
 {
 extern Int	num_lux_subr;
 extern	 struct	lux_subr_struct	lux_subr[];
 Int	i, list_widget;
 char		line[100];
 XmString	ms;
 /* get list widget */
 if ( get_widget_id( ps[0], &list_widget) != 1 ) return -1;
 
 /* load the subr names as items */
 for (i=0; i < num_lux_subr; i++ ) {
  sprintf(line, "%s",lux_subr[i].name);
  ms = XmStringCreate(line, cset);
  XmListAddItemUnselected(lux_widget_id[list_widget], ms, 0);
  XmStringFree(ms);
 }
 return 1;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmlistfunc(narg, ps) /*(list_widget)*/
 /* an lux subr, loads given list widget with internal func names */
 Int     narg, ps[];
 {
 extern Int	num_lux_func;
 extern	 struct	lux_subr_struct	lux_func[];
 Int	i, list_widget;
 char		line[100];
 XmString	ms;
 /* get list widget */
 if ( get_widget_id( ps[0], &list_widget) != 1 ) return -1;
 
 /* load the subr names as items */
 /* note that we skip over the specials, currently 9 of them */
 for (i=9; i < num_lux_func; i++ ) {
  sprintf(line, "%s", lux_func[i].name);
  ms = XmStringCreate(line, cset);
  XmListAddItemUnselected(lux_widget_id[list_widget], ms, 0);
  XmStringFree(ms);
 }
 return 1;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmlist(narg, ps) /*(parent, callback, nvisible,[font,color, resize_flag])*/
 /* returns widget id, an lux function */
 /* creates an empty list, nvisible is the vertical size in lines,
 resize_flag chooses the list size policy (see below) */
 Int     narg, ps[];
 {
 Int	iq, nsym, parent, nvisible, resize_flag=0;
 char *s;
 Int	list, result_sym;
 /* list widget interface */
 if (ck_motif() != 1) return -1;
 /* get parent widget */
 if ( get_widget_id( ps[0], &parent) != 1 ) return -1;
 /* get callback string */
 iq = ps[1];
 if (sym[iq].class != 2) { return execute_error(70); }
 s = (char *) sym[iq].spec.array.ptr;
 if (strlen(s) == 0 )  nsym = 0;  else	nsym = lux_execute_symbol(s,1);
 /* printf("nsym for call back = %d\n", nsym);*/
 if (int_arg_stat(ps[2], &nvisible) != 1) return -1;
 
 n = 0;
 XtSetArg(wargs[n], XmNselectionPolicy, XmBROWSE_SELECT); n++;
 XtSetArg(wargs[n], XmNvisibleItemCount, nvisible); n++;
 /* now the optional font and color */
 if (narg > 3) {
  if (set_fontlist( ps[3] ) != 1) return -1; /* gets fontlist from string */
 }
 if (narg > 4) {
  if (setup_colors (ps[4] ) != 1) return -1; }	/* do colors based on bg */
 if (narg > 5) if (int_arg_stat(ps[5], &resize_flag) != 1) return -1;
 switch (resize_flag) {
 case 1: XtSetArg(wargs[n], XmNlistSizePolicy, XmCONSTANT); n++; break;
 case 2: XtSetArg(wargs[n], XmNlistSizePolicy, XmVARIABLE); n++; break;
 default: XtSetArg(wargs[n], XmNlistSizePolicy, XmRESIZE_IF_POSSIBLE);n++;break;
 }
 if (ck_widget_count() < 0) return -1;
 list = n_widgets++;
 lux_widget_id[list] = XmCreateScrolledList(lux_widget_id[parent],
 	"list",wargs, n);
 XtManageChild(lux_widget_id[list]);
 XtAddCallback(lux_widget_id[list], XmNbrowseSelectionCallback,
                browse_callback, (XtPointer) nsym);
 result_sym = scalar_scratch(2);		/*for widget id */
 sym[result_sym].spec.scalar.l = list;
 return result_sym;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmlistadditem(narg, ps)	/* (widget_id, string, [position]) */
 /* adds string to the list widget, this is a subroutine */
 Int     narg, ps[];
 {
 Int	iq, i, w;
 char *s;
 XmString	ms;
 if ( get_widget_id( ps[0], &w) != 1 ) return -1;
 iq = ps[1];
 if (sym[iq].class != 2) { return execute_error(70); }
 s = (char *) sym[iq].spec.array.ptr;
 /* get position option, default is 0 (at end) */
 i = 0;
 if (narg > 2) {
 	if (int_arg_stat(ps[2], & i) != 1) return -1; }
 ms = XmStringCreate(s, cset);
 XmListAddItemUnselected(lux_widget_id[w], ms, i);
 /* also make this the last visible item */
 /* note that XmListAddItemUnselected doesn't return a status */
 XmStringFree(ms);
 return 1;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmlistdeleteitem(narg, ps)	/* (widget_id, position) */
 /* delete the input position from list, this is a subroutine */
 Int     narg, ps[];
 {
 Int	i, w;
 if ( get_widget_id( ps[0], &w) != 1 ) return -1;
 if (int_arg_stat(ps[1], & i) != 1) return -1;
 XmListDeletePos(lux_widget_id[w], i);
 return 1;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmlistdeleteall(narg, ps)	/* (widget_id) */
 /* delete all items in a list, this is a subroutine */
 Int     narg, ps[];
 {
 Int	w;
 if ( get_widget_id( ps[0], &w) != 1 ) return -1;
 XmListDeleteAllItems(lux_widget_id[w]);
 return 1;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmlistcount(narg, ps)	/* (widget_id) */
 /* get the count in a list, a function which returns count */
 Int     narg, ps[];
 {
 Int	i, w, result_sym;
 if ( get_widget_id( ps[0], &w) != 1 ) return -1;
 n = 0;
 XtSetArg(wargs[n], XmNitemCount, &i); n++;
 XtGetValues(lux_widget_id[w], wargs, n);
 result_sym = scalar_scratch(2);
 sym[result_sym].spec.scalar.l = i;
 return result_sym;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmlistselect(narg, ps)	/* (widget_id, position, cb_flag) */
 /* selects position in list and callback is notified if cb_flag > 0
 if the cb_flag is 2, then position is not set */
 Int     narg, ps[];
 {
 Int	i, w, cb_flag = 0;
 Widget	wg;
 if ( get_widget_id( ps[0], &w) != 1 ) return -1;
 if (int_arg_stat(ps[1], &i) != 1) return -1;
 if (narg > 2) if (int_arg_stat(ps[2], &cb_flag) != 1) return -1;
 wg = lux_widget_id[w];
 XmListDeselectAllItems(wg);
 if (cb_flag != 2) XmListSetPos(wg, i);
 if (cb_flag > 0) XmListSelectPos(wg, i, True);
 return 1;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmlabel(narg, ps)	/* (parent, label, [font, color, size_flag]) */
 /* creates a label widget, returns widget id */
 /* 7/26/96 size_flag added, sets XmNrecomputeSize to true/false (1/0) */
 Int     narg, ps[];
 {
 Int	iq, w, label, result_sym, size_flag = 0;
 char *s, *pc;
 XmString	ms;
 if (ck_motif() != 1) return -1;
 /* get parent widget */
 if ( get_widget_id( ps[0], &w) != 1 ) return -1;
 /* get label string */
 iq = ps[1];
 if (sym[iq].class != 2) { return execute_error(70); }
 s = (char *) sym[iq].spec.array.ptr;
 ms = XmStringCreateLtoR(s, cset);
 n = 0;
 XtSetArg(wargs[n], XmNlabelString, ms); n++;
 if (narg > 2) {
  if (set_fontlist( ps[2] ) != 1) return -1; /* gets fontlist from string */
 }
 if (narg > 3) {
  /* try to figure out the color */
  /* a string ? */
  if ( sym[ ps[3] ].class != 2 ) return execute_error(70);
  pc = (char *) sym[ps[3] ].spec.array.ptr;
  if ( strlen(pc) > 0 ) {
  if ( XAllocNamedColor(disp, cmap, pc, &colorcell, &rgb_def) == 0 )
	 color_not_available(); else
   { XtSetArg(wargs[n], XmNforeground, colorcell.pixel); n++; }
 } }
 if (narg > 4) {
 if (int_arg_stat(ps[4], &size_flag) != 1) return -1;
 }
 if (size_flag == 0 ) {	XtSetArg(wargs[n], XmNrecomputeSize, False); n++;
 	} else {	XtSetArg(wargs[n], XmNrecomputeSize, True); n++; }
 if (ck_widget_count() < 0) return -1;
 label = n_widgets++;
 lux_widget_id[label] = XmCreateLabel(lux_widget_id[w], "label", wargs, n);
 XtManageChild(lux_widget_id[label]);
 XmStringFree(ms);
 result_sym = scalar_scratch(2);		/*for widget id */
 sym[result_sym].spec.scalar.l = label;
 return result_sym;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmfileselect(narg, ps) /*(parent, title, ok_callback, (more on next line)
 [ok_label, help_callback, cancel_callback, font, button_font, color, dir,
 help_label, cancel_label] )*/
 /* returns widget id, an lux function */
 /* creates a file selection dialog, you get the file name from the
 callback routine
 10/3/95 add another paramter to be the directory mask */
 Int     narg, ps[];
 {
 Int	iq, nsym, parent;
 Int	nsym_help=0, nsym_cancel=0;
 char *s;
 Int	fselect, result_sym;
 XmString	title, ms, directory;
 Widget		dialog;
 /* list widget interface */
 if (ck_motif() != 1) return -1;
 /* get parent widget */
 if ( get_widget_id( ps[0], &parent) != 1 ) return -1;
 /* get title */
 iq = ps[1];
 if (sym[iq].class != 2) { return execute_error(70); }
 s = (char *) sym[iq].spec.array.ptr;
 title = XmStringCreateSimple(s);
 /* get callback string */
 iq = ps[2];
 if (sym[iq].class != 2) { return execute_error(70); }
 s = (char *) sym[iq].spec.array.ptr;
 if (strlen(s) == 0 )  nsym = 0;  else	nsym = lux_execute_symbol(s,1);
 n = 0;
 XtSetArg(wargs[n], XmNdialogTitle, title); n++;
 /* now the optional font and color, but font only good for children (below) */
 if (narg > 8) {
  if (setup_colors (ps[8] ) != 1) return -1; }	/* do colors based on bg */
 if (ck_widget_count() < 0) return -1;
 fselect = n_widgets++;
 if (narg > 9) {
 iq = ps[9];
 if (sym[iq].class != 2) { return execute_error(70); }
 s = (char *) sym[iq].spec.array.ptr;
 directory = XmStringCreateSimple(s);
 XtSetArg(wargs[n], XmNdirectory, directory); n++;
 }
 dialog =
 lux_widget_id[fselect] = XmCreateFileSelectionDialog(lux_widget_id[parent],
 	"fselect",wargs, n);
 XmStringFree(title);
 if (narg > 9) XmStringFree(directory);
 /* set up the OK label, if passed */
 if (narg > 3) {
 iq = ps[3];
 if (sym[iq].class != 2) { return execute_error(70); }
 s = (char *) sym[iq].spec.array.ptr;
 ms = XmStringCreateLtoR(s, cset);
 n = 0;
 XtSetArg(wargs[n], XmNlabelString, ms); n++;
 XtSetValues(XmFileSelectionBoxGetChild(dialog, XmDIALOG_OK_BUTTON), wargs, n);
 /* also do the help button redefine now if specified */
  if (narg > 10) {
  iq = ps[10];
  if (sym[iq].class != 2) { return execute_error(70); }
  s = (char *) sym[iq].spec.array.ptr;
  ms = XmStringCreateLtoR(s, cset);
  n = 0;
  XtSetArg(wargs[n], XmNlabelString, ms); n++;
  XtSetValues(XmFileSelectionBoxGetChild(dialog,XmDIALOG_HELP_BUTTON),wargs,n);
  }
 /* optional call backs, a null string indicates no call back */
 if (narg > 4) {
 iq = ps[4];
 if (sym[iq].class != 2) { return execute_error(70); }
 s = (char *) sym[iq].spec.array.ptr;
 if (strlen(s) == 0 )  nsym_help = 0; else nsym_help = lux_execute_symbol(s,1);
 if (narg > 5) {
 iq = ps[5];
 if (sym[iq].class != 2) { return execute_error(70); }
 s = (char *) sym[iq].spec.array.ptr;
 if (strlen(s) == 0) nsym_cancel = 0; else nsym_cancel=lux_execute_symbol(s,1);
 
 /* need to set fonts, colors for children */
 n = 0;
 if (narg > 6) {
  if (set_fontlist( ps[6] ) != 1) return -1; /* gets fontlist from string */

 if (narg > 8) {
  if (setup_colors (ps[8] ) != 1) return -1; }	/* do colors based on bg */
 XtSetValues(XmFileSelectionBoxGetChild(dialog, XmDIALOG_DIR_LIST), wargs, n);
 XtSetValues(XmFileSelectionBoxGetChild(dialog, XmDIALOG_FILTER_TEXT), wargs, n);
 XtSetValues(XmFileSelectionBoxGetChild(dialog, XmDIALOG_TEXT), wargs, n);
 XtSetValues(XmFileSelectionBoxGetChild(dialog, XmDIALOG_LIST), wargs, n);
 XtSetValues(XmFileSelectionBoxGetChild(dialog, XmDIALOG_FILTER_LABEL),wargs,n);
 XtSetValues(XmFileSelectionBoxGetChild(dialog,XmDIALOG_DIR_LIST_LABEL),wargs,n);
 XtSetValues(XmFileSelectionBoxGetChild(dialog, XmDIALOG_LIST_LABEL), wargs, n);
 XtSetValues(XmFileSelectionBoxGetChild(dialog,XmDIALOG_SELECTION_LABEL),wargs,n);
 /* another font for the buttons, different children */
 if (narg > 7) {
  n = 0;
  if (set_fontlist( ps[7] ) != 1) return -1; /* gets fontlist from string */
  XtSetValues(XmFileSelectionBoxGetChild(dialog, XmDIALOG_OK_BUTTON), wargs, n);
  XtSetValues(XmFileSelectionBoxGetChild(dialog,XmDIALOG_CANCEL_BUTTON),wargs,n);
  XtSetValues(XmFileSelectionBoxGetChild(dialog,XmDIALOG_HELP_BUTTON),wargs,n);
  XtSetValues(XmFileSelectionBoxGetChild(dialog,XmDIALOG_APPLY_BUTTON),wargs,n);
 }}
 }}}
 XtAddCallback(dialog, XmNokCallback, fselect_callback, (XtPointer) nsym);
 if (nsym_help == 0)
 XtUnmanageChild(XmFileSelectionBoxGetChild(dialog, XmDIALOG_HELP_BUTTON));
  else
   XtAddCallback(dialog, XmNhelpCallback, fhelp_callback, (XtPointer) nsym_help);
 if (nsym_cancel == 0)
 XtUnmanageChild(XmFileSelectionBoxGetChild(dialog, XmDIALOG_CANCEL_BUTTON));
  else
 XtAddCallback(dialog,XmNcancelCallback,fcancel_callback,(XtPointer)nsym_cancel);
 XtManageChild(lux_widget_id[fselect]);
 result_sym = scalar_scratch(2);		/*for widget id */
 sym[result_sym].spec.scalar.l = fselect;
 return result_sym;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmfilegetlist(narg, ps)	/* (widget_id) */
 /* get the files in a file selection widget, puts them in a string array
 strarr = xmfilegetlist(widget)  where widget must be a file selection widget */
 Int     narg, ps[];
 {
 Int	w, result_sym, fcount, dim[8];
 XmString	*names;
 char	**q, *text, *strsave();
 
 if ( get_widget_id( ps[0], &w) != 1 ) return -1;
 n = 0;
 XtSetArg(wargs[n], XmNfileListItems, &names); n++;
 XtSetArg(wargs[n], XmNfileListItemCount, &fcount); n++;
 XtGetValues(lux_widget_id[w], wargs, n);
 dim[0] = fcount;
 result_sym = strarr_scratch(1, dim);
 q = (char **)((char *)sym[result_sym].spec.array.ptr+sizeof(struct ahead));
 while (fcount--) {
 if (XmStringGetLtoR (*names, cset, &text)) *q = strsave(text);
 q++; names++;
 }
 return result_sym;
 }
 /*------------------------------------------------------------------------- */
Int set_fontlist(nsym)
 Int	nsym;
 {
 char    *fontname;
 if (sym[nsym].class != 2) return execute_error(70);
 fontname = (char *) sym[nsym].spec.array.ptr;
 if ( strlen(fontname) <= 0 ) return 1;	/* a null, do nothing */
 /* note that font and fontlist are common and reused, may have a malloc
 leak here, not sure if these should be freed after each use */
 if ( (font = XLoadQueryFont(disp, fontname)) == NULL)
 	{ font_not_available();
	  return 1; }
 fontlist = XmFontListCreate(font, cset);
 XtSetArg(wargs[n], XmNfontList, fontlist); n++;
 return 1; 
 }
 /*------------------------------------------------------------------------- */
Int set_textfontlist(nsym)
 Int	nsym;
 {
 char    *fontname;
 if (sym[nsym].class != 2) return execute_error(70);
 fontname = (char *) sym[nsym].spec.array.ptr;
 if ( strlen(fontname) <= 0 ) return 1;	/* a null, do nothing */
 /* note that font and fontlist are common and reused, may have a malloc
 leak here, not sure if these should be freed after each use */
 if ( (font = XLoadQueryFont(disp, fontname)) == NULL)
 	{ font_not_available();
	  return 1; }
 fontlist = XmFontListCreate(font, cset);
 XtSetArg(wargs[n], XmNtextFontList, fontlist); n++;
 return 1; 
 }
 /*------------------------------------------------------------------------- */
Int set_labelfontlist(nsym)
 Int	nsym;
 {
 char    *fontname;
 if (sym[nsym].class != 2) return execute_error(70);
 fontname = (char *) sym[nsym].spec.array.ptr;
 if ( strlen(fontname) <= 0 ) return 1;
 /* note that font and fontlist are common and reused, may have a malloc
 leak here, not sure if these should be freed after each use */
 if ( (font = XLoadQueryFont(disp, fontname)) == NULL)
 	{ font_not_available();
	  return 1; }
 fontlist = XmFontListCreate(font, cset);
 XtSetArg(wargs[n], XmNlabelFontList, fontlist); n++;
 return 1; 
 }
 /*------------------------------------------------------------------------- */
Int set_defaultfontlist(nsym)
 Int	nsym;
 {
 char    *fontname;
 if (sym[nsym].class != 2) return execute_error(70);
 fontname = (char *) sym[nsym].spec.array.ptr;
 /* note that font and fontlist are common and reused, may have a malloc
 leak here, not sure if these should be freed after each use */
 if ( (font = XLoadQueryFont(disp, fontname)) == NULL)
 	{ font_not_available();
	  return 1; }
 fontlist = XmFontListCreate(font, cset);
 XtSetArg(wargs[n], XmNdefaultFontList, fontlist); n++;
 return 1; 
 }
/*------------------------------------------------------------------------- */
Int setup_colors(Int nsym)
{
  char	*pc;
  Pixel	ts, bs, fc, sc;
  Status	anaAllocNamedColor(char *, XColor **);
  XColor	*color;
  void	installPixel(Int);

  /* try to figure out the color */
  /* a string ? */
  if (!symbolIsString(nsym))
    return cerror(NEED_STR, nsym);
  pc = string_value(nsym);
  if (strlen(pc) <= 0)
    return 1;			/* a null, do nothing */
  if (!anaAllocNamedColor(pc, &color))
    return 1;

 /* now let motif choose the others */
  XmGetColors(XtScreen(lux_widget_id[toplevel]), colorMap, color->pixel,
 	&fc, &ts, &bs, &sc);

  /* update colors[] database.  If the pixel value already exists, then */
  /* do nothing.  If not, then store it in the database. */
  installPixel(fc);
  installPixel(ts);
  installPixel(bs);
  installPixel(sc);

 /* and set them */
  XtSetArg(wargs[n], XmNforeground, fc); n++; 
  XtSetArg(wargs[n], XmNbackground, color->pixel); n++; 
  XtSetArg(wargs[n], XmNborderColor, fc); n++; 
  XtSetArg(wargs[n], XmNtopShadowColor, ts); n++; 
  XtSetArg(wargs[n], XmNarmColor, sc); n++; 
  XtSetArg(wargs[n], XmNbottomShadowColor, bs); n++; 
  XtSetArg(wargs[n], XmNselectColor, sc); n++;
  return 1; 
 }
 /*------------------------------------------------------------------------- */
Int lux_xmbutton(narg, ps) /* (parent, label, callback, [font,color]) */
 /* creates a button widget, returns widget id */
 Int     narg, ps[];
 {
 Int	iq, nsym, w, push, result_sym;
 char *s;
 XmString	ms;
 if (ck_motif() != 1) return -1;
 /* get parent widget */
 if ( get_widget_id( ps[0], &w) != 1 ) return -1;
 /* get label string */
 iq = ps[1];
 if (sym[iq].class != 2) { return execute_error(70); }
 s = (char *) sym[iq].spec.array.ptr;
 ms = XmStringCreateLtoR(s, cset);
 /* get callback string */
 iq = ps[2];
 if (sym[iq].class != 2) { return execute_error(70); }
 s = (char *) sym[iq].spec.array.ptr;
 if (strlen(s) == 0 )  nsym = 0;  else	nsym = lux_execute_symbol(s,1);

 n = 0;
 /* now the optional font and color */
 if (narg > 3) {
  if (set_fontlist( ps[3] ) != 1) return -1; /* gets fontlist from string */
 }
 if (narg > 4) {
  if (setup_colors (ps[4] ) != 1) return -1; }	/* do colors based on bg */
 XtSetArg(wargs[n], XmNlabelString, ms); n++;
 if (ck_widget_count() < 0) return -1;
 push = n_widgets++;
 lux_widget_id[push] = XmCreatePushButton(lux_widget_id[w], s, wargs, n);
 XtManageChild(lux_widget_id[push]);
 XmStringFree(ms);
 XtAddCallback(lux_widget_id[push], XmNactivateCallback,
                button_callback, (XtPointer) nsym);
 result_sym = scalar_scratch(2);		/*for widget id */
 sym[result_sym].spec.scalar.l = push;
 return result_sym;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmgetpixmap(narg, ps) /* (id, pixmap_file) */
 /* reads a pixmap (bitmap) file and associates it with id, the
 maximum number of pixmaps is  MAXPIXMAPS with id's from 0 to MAXPIXMAPS-1 */
 Int     narg, ps[];
 {
 Int	iq, id;
 char *s;
 Pixel	fg, bg;
 if (ck_motif() != 1) return -1;
 if (int_arg_stat(ps[1], &id) != 1) return -1;
 if (id <0 || id >= MAXPIXMAPS) {
 	printf("XMGETPIXMAP: id out of range (0-%d)", MAXPIXMAPS-1);
	return -1;
	}
 /* get pixmap file name */
 iq = ps[1];
 if (sym[iq].class != 2) { return execute_error(70); }
 s = (char *) sym[iq].spec.array.ptr;
 /* use fg and bg from toplevel */
 XtVaGetValues (lux_widget_id[toplevel],
  XmNforeground, &fg,
  XmNbackground, &bg,
  NULL);
 lux_pixmap_id[id] = XmGetPixmap(XtScreen(lux_widget_id[toplevel]), s, fg, bg);
 if ( lux_pixmap_id[id] == 0) { printf("error reading pixmap file %s\n", s);
	return -1;	}
 return 1;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmsetpixmap(narg, ps) /* (widget, pixmap_file) */
 Int     narg, ps[];
 {
 Pixmap	pixmap;
 Pixel	fg, bg;
 Int	w, iq;
 char    *s;
 if (ck_motif() != 1) return -1;
 if (get_widget_id(ps[0], &w) != 1) return -1;
 /* get pixmap file name */
 iq = ps[1];
 if (sym[iq].class != 2) { return execute_error(70); }
 s = (char *) sym[iq].spec.array.ptr;
 /* use fg and bg from parent */
 XtVaGetValues (lux_widget_id[w],
  XmNforeground, &fg,
  XmNbackground, &bg,
  NULL);
 pixmap = XmGetPixmap(XtScreen(lux_widget_id[toplevel]), s, fg, bg);
 if ( pixmap == 0) { printf("error reading pixmap file %s\n", s);
	return -1;	}
 
 n = 0;
 XtSetArg(wargs[n], XmNlabelType, XmPIXMAP); n++;
 XtSetArg(wargs[n], XmNlabelPixmap, pixmap); n++;
 XtSetValues(lux_widget_id[w], wargs, n);
 return 1;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmpixmapbutton(narg, ps) /* (parent, pixmap_file, callback) */
 /* creates a button widget, returns widget id */
 Int     narg, ps[];
 {
 Int	iq, nsym, w, push, result_sym;
 char *s;
 Pixmap	pixmap;
 Pixel	fg, bg;
 if (ck_motif() != 1) return -1;
 /* get parent widget */
 if ( get_widget_id( ps[0], &w) != 1 ) return -1;
 /* get pixmap file name */
 iq = ps[1];
 if (sym[iq].class != 2) { return execute_error(70); }
 s = (char *) sym[iq].spec.array.ptr;
 /* use fg and bg from parent */
 XtVaGetValues (lux_widget_id[w],
  XmNforeground, &fg,
  XmNbackground, &bg,
  NULL);
 pixmap = XmGetPixmap(XtScreen(lux_widget_id[toplevel]), s, fg, bg);
 if ( pixmap == 0) { printf("error reading pixmap file %s\n", s);
	return -1;	}
  
 /* get callback string */
 iq = ps[2];
 if (sym[iq].class != 2) { return execute_error(70); }
 s = (char *) sym[iq].spec.array.ptr;
 if (strlen(s) == 0 )  nsym = 0;  else	nsym = lux_execute_symbol(s,1);

 n = 0;
 XtSetArg(wargs[n], XmNlabelType, XmPIXMAP); n++;
 XtSetArg(wargs[n], XmNlabelPixmap, pixmap); n++;
 if (ck_widget_count() < 0) return -1;
 push = n_widgets++;
 lux_widget_id[push] = XmCreatePushButton(lux_widget_id[w], "button", wargs, n);
 XtManageChild(lux_widget_id[push]);
 XtAddCallback(lux_widget_id[push], XmNactivateCallback,
                button_callback, (XtPointer) nsym);
 result_sym = scalar_scratch(2);		/*for widget id */
 sym[result_sym].spec.scalar.l = push;
 return result_sym;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmradiobox(narg, ps) /* (parent,callback,font,color,l1,[l2, l3 ...]) */
 /* creates a simple radio box widget, returns array of widget id's for
 toggles and the container widget as the first value */
 /* the font and color preceed the labels to allow easier parsing, if
 these are null strings or scalars, you get the defaults */
 /* 2/3/95 addition, if the last argument is a scalar, it is used as
 the # of columns, otherwise there is only one column */
 Int     narg, ps[];
 {
 Int	iq, nsym, i, w, radio, result_sym, j, k, *wids, nq, ncolumns;
 char *s;
 Widget	wg;
 struct	ahead	*h;
 if (ck_motif() != 1) return -1;
 /* get parent widget */
 if ( get_widget_id( ps[0], &w) != 1 ) return -1;
 /* note that callback is first here to provide for unknown # of labels */
 /* get callback string */
 iq = ps[1];
 if (sym[iq].class != 2) { return execute_error(70); }
 s = (char *) sym[iq].spec.array.ptr;
 if (strlen(s) == 0 )  nsym = 0;  else	nsym = lux_execute_symbol(s,1);

 /* check the last arg., if not a string, we try to use it as the number of
 columns */

 iq = ps[narg-1];	ncolumns = 1;
 if (sym[iq].class != 2) {
 /* check if a scalar, otherwise an error */
 if (int_arg_stat(iq, &ncolumns) != 1) {
  printf("XMRADIOBOX, last arg neither a string nor a scalar\n");
  return -1;
 }
 narg--;	/* decrease for the new # of labels */
 }

 /* before getting the labels, create the radio box widget, we
 pass back this ID as first element in returned array */
 n = 0;
 if ( radiobox == 0 ) XtSetArg(wargs[n], XmNradioBehavior, False); n++;
 XtSetArg(wargs[n], XmNnumColumns, ncolumns); n++;
 wg = XmCreateRadioBox(lux_widget_id[w], "radio_box",wargs,n);
 
 /* now the semi-optional font and color */
 n = 0;	/* be careful, n is bumped by set_fontlist and setup_colors
 	and is then then used for each of the toggle buttons */
 if (set_fontlist( ps[2] ) != 1) return -1; /* gets fontlist from string */
 if (setup_colors (ps[3] ) != 1) return -1; /* do colors based on bg */
 i = narg - 4;	j = 4; k = 0;
 /* i is the count of toggle buttons, we are going to store the widget id's
 for these in an array returned, but the first element will be the
 parent row/column widget so the the array will be i+1 long */
 nq=i+1;
 result_sym = array_scratch(2, 1, &nq );
 h = (struct ahead *) sym[result_sym].spec.array.ptr;
 wids = (Int *) ((char *)h + sizeof(struct ahead));
 if (ck_widget_count() < 0) return -1;
 radio = n_widgets++;
 lux_widget_id[radio] = wg;
 *wids++ = radio;
 while (i--) {
 /* get label string */
 iq = ps[j++];
 if (sym[iq].class != 2) { return execute_error(70); }
 s = (char *) sym[iq].spec.array.ptr;
 if (ck_widget_count() < 0) return -1;
 radio = n_widgets++;
 *wids++ = radio;
 /* note that each of these use the font and color set above */
 lux_widget_id[radio] = XtCreateManagedWidget( s, xmToggleButtonGadgetClass, wg,
 	wargs, n);
 XtAddCallback(lux_widget_id[radio], XmNvalueChangedCallback, radio_which,
 	(XtPointer) k);
 k++;
 XtAddCallback(lux_widget_id[radio], XmNvalueChangedCallback, radio_callback,
 	(XtPointer) nsym);
 }
 XtManageChild(wg);
 return result_sym;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmtogglegetstate(narg, ps) /* (widget) */
 Int     narg, ps[];
 {
 Int	w, result_sym, iq;
 if (ck_motif() != 1) return -1;
 if (get_widget_id(ps[0], &w) != 1) return -1;
 result_sym = scalar_scratch(2);		/* state */
 iq = 0;
 if ( XmToggleButtonGadgetGetState(lux_widget_id[w]) ) iq = 1;
 result_sym = scalar_scratch(2);		/* state */
 sym[result_sym].spec.scalar.l = iq;
 return result_sym;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmtogglesetstate(narg, ps) /* (widget, state, [notify]) */
 /* this is a subroutine call, state is 0 or non-zero for True/False */
 /* added a notify option, default is to notify which means the callback
    gets executed */
 Int     narg, ps[];
 {
 Int	w, iq;
 Boolean state, notify = True;
 if (ck_motif() != 1) return -1;
 if (get_widget_id(ps[0], &w) != 1) return -1;
 if (int_arg_stat(ps[1], &iq) != 1) return -1;
 if (iq) state = True; else state = False;
 if (narg > 2) { if (int_arg_stat(ps[2], &iq) != 1) return -1;
  if (iq) notify = True; else notify = False; }
 XmToggleButtonGadgetSetState(lux_widget_id[w], state, notify);
 return 1;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmcheckbox(narg, ps) /* (parent,callback,font,color,l1,[l2, l3 ...]) */
 /* this works very much like a radio box except for a few details, so
 we just set a flag here and call lux_xmradiobox(narg, ps) */
 /* note that there is no management of which buttons are set at this level */
 /* the user will have to keep track of that from the callbacks or by
 using the getstate call */
 Int     narg, ps[];
 {
 Int	iq;
 radiobox = 0;
 iq = lux_xmradiobox(narg, ps);
 radiobox = 1; /* reset default */
 return iq;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmpixmapoptionmenu(narg, ps) /* (parent,callback,font,color,s,p1,[p2 ...]) */
 /* creates a simple option menu widget that uses pixmaps for the button
 labels, all items must be pixmaps, s is the title for this option */
 /* based on original xmoptionmenu, the arguments p1, p2, ... are
 file names for the pixmaps (they must be strings), note that font and color
 arguments are still present but used only for the label s
 6/26/94 addition - if the last argument is a scalar instead of a string,
 it is used to set the default menu item, otherwise this is the first on
 the list (first is 0, etc) */
 Int     narg, ps[];
 {
 Int	iq, nsym, i, w, option, result_sym, j, k, *wids, nq, dmenu;
 char *s;
 Widget	wg, optionsubmenu, woption;
 XmString	ms;
 Pixmap	pixmap;
 Pixel	fg, bg;
 struct	ahead	*h;
 /* get parent widget and some checks */
 if (ck_motif() != 1) return -1;
 if ( get_widget_id( ps[0], &w) != 1 ) return -1;
 /* note that callback is first here to provide for unknown # of labels */
 /* get callback string */
 iq = ps[1];
 if (sym[iq].class != 2) { return execute_error(70); }
 s = (char *) sym[iq].spec.array.ptr;
 if (strlen(s) == 0 )  nsym = 0;  else	nsym = lux_execute_symbol(s,1);

 /* now the semi-optional font and color */
 n = 0;	/* be careful, n is bumped by set_fontlist and setup_colors
 	and is then then used for each of the toggle buttons */
 if (set_fontlist( ps[2] ) != 1) return -1; /* gets fontlist from string */
 if (set_labelfontlist( ps[2] ) != 1) return -1; /* gets fontlist from string */
 if (setup_colors(ps[3] ) != 1) return -1; /* do colors based on bg */

 /* create the pulldown menu, this is not the widget passed back */
 optionsubmenu = XmCreatePulldownMenu(lux_widget_id[w], "optionsubmenu",wargs,n);

 /* use fg and bg from parent */
 XtVaGetValues (lux_widget_id[w],
  XmNforeground, &fg,
  XmNbackground, &bg,
  NULL);

 /* check the last arg., if not a string, we try to use it as the number of
 the menu item to use as the default */

 iq = ps[narg-1];	dmenu = 0;
 if (sym[iq].class != 2) {
 /* check if a scalar, otherwise an error */
 if (int_arg_stat(iq, &dmenu) != 1) {
  printf("XMOPTIONMENU, last arg neither a string nor a scalar\n");
  return -1;
 }
 narg--;	/* decrease for the new # of labels */
 dmenu = MAX(dmenu ,0);		dmenu = MIN(dmenu, narg - 6);
 }
 
 i = narg - 5;	j = 5; k = 0;
 /* i is the count of push buttons, we are going to store the widget id's
 for these in an array returned, but the first element will be the
 parent row/column widget, also (at the end) we store the widgets for
 the option label and the current selection (a cascade button) so the the
 array will be i+3 long */
 nq=i+3;
 result_sym = array_scratch(2, 1, &nq );
 h = (struct ahead *) sym[result_sym].spec.array.ptr;
 wids = (Int *) ((char *)h + sizeof(struct ahead));
 wg = optionsubmenu;
 while (i--) {
 /* get filename string */
 iq = ps[j++];
 if (sym[iq].class != 2) { return execute_error(70); }
 s = (char *) sym[iq].spec.array.ptr;
 /* setup for pixmap buttons */
 n = 0;
 XtSetArg(wargs[n], XmNlabelType, XmPIXMAP); n++;
 pixmap = XmGetPixmap(XtScreen(lux_widget_id[toplevel]), s, fg, bg);
 if ( pixmap == 0) { printf("error reading pixmap file %s\n", s);
	return -1;	}
 XtSetArg(wargs[n], XmNlabelPixmap, pixmap); n++;
 if (ck_widget_count() < 0) return -1;
 option = n_widgets++;
 /* note that each of these use the font and color set above */
 
 lux_widget_id[option] = XtCreateManagedWidget( s, xmPushButtonGadgetClass, wg,
 	wargs, n);
 
 XtAddCallback(lux_widget_id[option], XmNactivateCallback, menu_which,
 	(XtPointer) k);
 k++;
 XtAddCallback(lux_widget_id[option], XmNactivateCallback, button_callback,
 	(XtPointer) nsym);
 wids[k] = option;  /* first one is k=1, k=0 element set later */
 /* printf("k, option, lux_widget_id[option] = %d %d %d\n",k,
 	option, lux_widget_id[option] ); */
 }
 /* get option label */
 n = 0;
 if (set_fontlist( ps[2] ) != 1) return -1; /* gets fontlist from string */
 if (set_labelfontlist( ps[2] ) != 1) return -1; /* gets fontlist from string */
 if (setup_colors (ps[3] ) != 1) return -1; /* do colors based on bg */
 iq = ps[4];
 if (sym[iq].class != 2) { return execute_error(70); }
 s = (char *) sym[iq].spec.array.ptr;
 ms = XmStringCreateLtoR(s, cset);
 XtSetArg(wargs[n], XmNlabelString, ms); n++;
 XtSetArg(wargs[n], XmNsubMenuId, optionsubmenu); n++;
 XtSetArg(wargs[n], XmNmenuHistory, lux_widget_id[wids[dmenu+1]]); n++;
 woption = XmCreateOptionMenu(lux_widget_id[w],"option_menu", wargs, n);
 if (ck_widget_count() < 0) return -1;
 option = n_widgets++;
 lux_widget_id[option] = woption;
 *wids = option;
 /* now we can get the internally created widgets and set their fonts */
 /* for the pixmap version, we only do this for the label gadget but
 we still get the widget id for the button */
 wg = XmOptionButtonGadget(woption);
 /* XtSetValues(wg, wargs, n); */
 if (ck_widget_count() < 0) return -1;
 k++; option = n_widgets++; lux_widget_id[option] =wg; wids[k] = option;
 wg = XmOptionLabelGadget(woption);
 XtSetValues(wg, wargs, n);
 if (ck_widget_count() < 0) return -1;
 k++; option = n_widgets++; lux_widget_id[option] =wg; wids[k] = option;
 XtManageChild(woption);
 XmStringFree(ms);
 return result_sym;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmoptionmenu(narg, ps) /* (parent,callback,font,color,s,l1,[l2 ...]) */
 /* creates a simple option menu widget, s is the title for this option */
 /* the font and color preceed the labels to allow easier parsing, if
 these are null strings or scalars, you get the defaults, the option label
 is s and the labels for each button are l1, l2 ...
 6/26/94 addition - if the last argument is a scalar instead of a string,
 it is used to set the default menu item, otherwise this is the first on
 the list (first is 0, etc) */
 Int     narg, ps[];
 {
 Int	iq, nsym, i, w, option, result_sym, j, k, *wids, nq, dmenu;
 char *s;
 Widget	wg, optionsubmenu, woption;
 XmString	ms;
 struct	ahead	*h;
 /* get parent widget and some checks */
 if (ck_motif() != 1) return -1;
 if ( get_widget_id( ps[0], &w) != 1 ) return -1;
 /* note that callback is first here to provide for unknown # of labels */
 /* get callback string */
 iq = ps[1];
 if (sym[iq].class != 2) { return execute_error(70); }
 s = (char *) sym[iq].spec.array.ptr;
 if (strlen(s) == 0 )  nsym = 0;  else	nsym = lux_execute_symbol(s,1);

  /* now the semi-optional font and color */
 n = 0;	/* be careful, n is bumped by set_fontlist and setup_colors
 	and is then then used for each of the toggle buttons */
 if (set_fontlist( ps[2] ) != 1) return -1; /* gets fontlist from string */
 if (set_labelfontlist( ps[2] ) != 1) return -1; /* gets fontlist from string */
 if (setup_colors (ps[3] ) != 1) return -1; /* do colors based on bg */

 /* create the pulldown menu, this is not the widget passed back */
 optionsubmenu = XmCreatePulldownMenu(lux_widget_id[w], "optionsubmenu",wargs,n);

 /* check the last arg., if not a string, we try to use it as the number of
 the menu item to use as the default */

 iq = ps[narg-1];	dmenu = 0;
 if (sym[iq].class != 2) {
 /* check if a scalar, otherwise an error */
 if (int_arg_stat(iq, &dmenu) != 1) {
  printf("XMOPTIONMENU, last arg neither a string nor a scalar\n");
  return -1;
 }
 narg--;	/* decrease for the new # of labels */
 dmenu = MAX(dmenu ,0);		dmenu = MIN(dmenu, narg - 6);
 }
 
 i = narg - 5;	j = 5; k = 0;
 /* i is the count of push buttons, we are going to store the widget id's
 for these in an array returned, but the first element will be the
 parent row/column widget, also (at the end) we store the widgets for
 the option label and the current selection (a cascade button) so the the
 array will be i+3 long */
 nq=i+3;
 result_sym = array_scratch(2, 1, &nq );
 h = (struct ahead *) sym[result_sym].spec.array.ptr;
 wids = (Int *) ((char *)h + sizeof(struct ahead));
 while (i--) {
 /* get label string */
 iq = ps[j++];
 if (sym[iq].class != 2) { return execute_error(70); }
 s = (char *) sym[iq].spec.array.ptr;
 if (ck_widget_count() < 0) return -1;
 option = n_widgets++;
 /* note that each of these use the font and color set above */

 /* sorry, motif does not support cascade buttons in option menus so we
 can't have sub menus (toggles also not supported) */

 wg = XtCreateManagedWidget( s, xmPushButtonGadgetClass, optionsubmenu,wargs, n);
 XtAddCallback(wg, XmNactivateCallback, menu_which, (XtPointer) k);
 XtAddCallback(wg, XmNactivateCallback, button_callback, (XtPointer) nsym);
 k++;
 wids[k] = option;  /* first one is k=1, k=0 element set later */
 /* printf("k, option, lux_widget_id[option] = %d %d %d\n",k,
 	option, lux_widget_id[option] ); */
 lux_widget_id[option] = wg;
 }
 /* get option label */
 iq = ps[4];
 if (sym[iq].class != 2) { return execute_error(70); }
 s = (char *) sym[iq].spec.array.ptr;
 ms = XmStringCreateLtoR(s, cset);
 n = 0;
 /* setting the color here will apply it to both label and button and the
 entire widget area, not desired. No way to just do the button! */
 XtSetArg(wargs[n], XmNlabelString, ms); n++;
 XtSetArg(wargs[n], XmNsubMenuId, optionsubmenu); n++;
 XtSetArg(wargs[n], XmNmenuHistory, lux_widget_id[wids[dmenu+1]]); n++;
 woption = XmCreateOptionMenu(lux_widget_id[w],"option_menu", wargs, n);
 if (ck_widget_count() < 0) return -1;
 option = n_widgets++;
 lux_widget_id[option] = woption;
 *wids = option;
 /* now we can get the internally created widgets and set their fonts, we
 can't set their colors unfortunately */
 n = 0;
 if (set_labelfontlist( ps[2] ) != 1) return -1; /* gets fontlist from string */
 wg = XmOptionButtonGadget(woption);
 XtSetValues(wg, wargs, n);
 if (ck_widget_count() < 0) return -1;
 k++; option = n_widgets++; lux_widget_id[option] =wg; wids[k] = option;
 n = 0;
 if (set_fontlist( ps[2] ) != 1) return -1; /* gets fontlist from string */
 wg = XmOptionLabelGadget(woption);
 XtSetValues(wg, wargs, n);
 if (ck_widget_count() < 0) return -1;
 k++; option = n_widgets++; lux_widget_id[option] =wg; wids[k] = option;
 XtManageChild(woption);
 XmStringFree(ms);
 return result_sym;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmgetoptionselection(narg, ps)
 /* arg is row/column widget from an option menu */
 Int     narg, ps[];
 {
 Int	w, result_sym, wm;
 if ( get_widget_id( ps[0], &w) != 1 ) return -1;
 /* this should be a row/column type but we aren't checking here */
 if (ck_widget_count() < 0) return -1;
 wm = n_widgets++; 
 n = 0;
 XtSetArg(wargs[n], XmNmenuHistory, &lux_widget_id[wm]); n++;
 XtGetValues(lux_widget_id[w], wargs, n);
 result_sym = scalar_scratch(2);		/*for widget id */
 sym[result_sym].spec.scalar.l = wm;
 return result_sym;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmsetoptionselection(narg, ps)
 /* 2 args, row/column widget from an option menu and widget for the
 menu item it should be set to */
 Int     narg, ps[];
 {
 Int	w, wm;
 if ( get_widget_id( ps[0], &w) != 1 ) return -1;
 if ( get_widget_id( ps[1], &wm) != 1 ) return -1;
 n = 0;
 XtSetArg(wargs[n], XmNmenuHistory, lux_widget_id[wm]); n++;
 XtSetValues(lux_widget_id[w], wargs, n);
 return 1;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmmenubar(narg, ps) /* (parent,font,color,l1,[l2 ...]) */
 /* creates a menu bar widget, you will need pulldowns to attach */
 /* the font and color preceed the labels to allow easier parsing, if
 these are null strings or scalars, you get the defaults
 the labels for each cascade button are l1, l2 ...
 an array of widget id's is returned, the first is the menubar
 widget followed by each cascade button, we need the latter for attaching
 pulldowns to these cascade buttons */
 Int     narg, ps[];
 {
 Int	iq, i, w, result_sym, j, k, *wids, nq, menu;
 char *s;
 Widget	wg, menu_bar;
 XmString	ms;
 struct	ahead	*h;
 /* get parent widget and some checks */
 if (ck_motif() != 1) return -1;
 if ( get_widget_id( ps[0], &w) != 1 ) return -1;
 if (ck_widget_count() < 0) return -1;
 menu = n_widgets++;
 
 /* create the menu bar */
 n = 0;	/* be careful, n is bumped by set_fontlist and setup_colors
 	and is then then used for each of the toggle buttons */
 if (setup_colors (ps[2] ) != 1) return -1; /* do colors based on bg */
 menu_bar = XmCreateMenuBar(lux_widget_id[w], "menu_bar",wargs,n);
 lux_widget_id[menu] = menu_bar;


 /* now the semi-optional font and color for the cascade buttons */
 if (set_fontlist( ps[1] ) != 1) return -1; /* gets fontlist from string */
 if (set_labelfontlist( ps[1] ) != 1) return -1; /* gets fontlist from string */

 i = narg - 3;
 j = 3;		nq = i + 1;
 /* i is the count of cascade buttons */
 result_sym = array_scratch(2, 1, &nq );
 h = (struct ahead *) sym[result_sym].spec.array.ptr;
 wids = (Int *) ((char *)h + sizeof(struct ahead));
 wids[0] =  menu;
 k = 1;
 while (i--) {
 if (ck_widget_count() < 0) return -1;
 menu = n_widgets++;
 /* get label string */
 iq = ps[j++];
 if (sym[iq].class != 2) { return execute_error(70); }
 s = (char *) sym[iq].spec.array.ptr;
 ms = XmStringCreateLtoR(s, cset);
 /* note that each of these use the font and color set above, plus label */
 XtSetArg(wargs[n], XmNlabelString, ms);
 wg = lux_widget_id[menu] = XmCreateCascadeButton(menu_bar, "menu", wargs, n+1);
 XtManageChild(wg);
 XmStringFree(ms);
 wids[k++] = menu;
 }
 /* make the last one a "help" menu, this means it get shoved to far right */
 n = 0;
 XtSetArg(wargs[n], XmNmenuHelpWidget, wg); n++;
 XtSetValues(menu_bar, wargs, n);
 XtManageChild(menu_bar);
 return result_sym;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmpulldownmenu(narg, ps) /* (parent,casc,callback,font,color,l1,[l2 ...]) */
 /* creates a pulldown menu widget, you first need a menubar to attach to,
 the parent must be a menubar widget, the casc is the cascade button
 widget id that this pulldown is attached to, these are the addition
 items in the widget array returned by xmmenubar
 the callback is for all items, a global index is used to determine
 which menu button was activated */
 /* the font and color preceed the labels to allow easier parsing, if
 these are null strings or scalars, you get the defaults
 the labels for each cascade button are l1, l2 ... */
 /* returns the widget number for the pulldown and each button */
 Int     narg, ps[];
 {
 Int	iq, nsym, i, w, result_sym, j, k, nq, c_button, wc, *wids;
 char *s;
 Widget	wg, pull_down;
 struct	ahead	*h;
 /* get parent widget and some checks */
 if (ck_motif() != 1) return -1;
 if ( get_widget_id( ps[0], &w) != 1 ) return -1;
 
 /* get the cascade button widget */
 if (int_arg_stat(ps[1], &c_button) != 1) {
  printf("XMPULLDOWNMENU, cascade widget id is not a scalar\n");
  return -1;
 }
 /* note that callback is here to provide for unknown # of labels */
 /* get callback string */
 iq = ps[2];
 if (sym[iq].class != 2) { return execute_error(70); }
 s = (char *) sym[iq].spec.array.ptr;
 if (strlen(s) == 0 )  nsym = 0;  else	nsym = lux_execute_symbol(s,1);
 /* create the pulldown menu */
 n = 0;
 XtSetArg(wargs[n], XmNtearOffModel, XmTEAR_OFF_ENABLED); n++;
 /* turns out the color has to be defined here */
 if (setup_colors (ps[4] ) != 1) return -1; /* do colors based on bg */
 pull_down = XmCreatePulldownMenu(lux_widget_id[w], "pulldown",wargs,n);
 if (ck_widget_count() < 0) return -1;
 wc  = n_widgets++;
 lux_widget_id[wc] = pull_down;
 /* set the XmNsubMenuId for the cascade button widget */
 if (!XtIsSubclass(lux_widget_id[c_button], xmCascadeButtonWidgetClass) &&
 	!XtIsSubclass(lux_widget_id[c_button], xmCascadeButtonGadgetClass))
	{ printf("XMPULLDOWNMENU, cascade widget not a cascade!\n");
	return -1;}
 XtSetArg(wargs[0], XmNsubMenuId, pull_down);
 XtSetValues(lux_widget_id[c_button], wargs, 1);

 /* now the semi-optional font for the push buttons */
 n = 0;	/* be careful, n is bumped by set_fontlist
 	and is then then used for each of the toggle buttons */
 if (set_fontlist( ps[3] ) != 1) return -1; /* gets fontlist from string */
 if (set_labelfontlist( ps[3] ) != 1) return -1; /* gets fontlist from string */

 i = narg - 5;
 j = 5;	k = 0;
 /* i is the count of menu push buttons */
 nq = i + 1;
 result_sym = array_scratch(2, 1, &nq );
 h = (struct ahead *) sym[result_sym].spec.array.ptr;
 wids = (Int *) ((char *)h + sizeof(struct ahead));
 wids[0] =  wc;
 while (i--) {
 /* get label string */
 iq = ps[j++];
 if (sym[iq].class != 2) { return execute_error(70); }
 s = (char *) sym[iq].spec.array.ptr;
 /* if the string starts with a >, we interpret as a cascade that will
 (maybe) be attached to a pull right menu */
 /* note that each of these use the font and color set above */
 if (*s == '>') {
 s++;
 /*printf("got a pull right\n");*/
 wg = XtCreateManagedWidget(s, xmCascadeButtonGadgetClass, pull_down, wargs, n);
 } else {
 wg = XtCreateManagedWidget(s, xmPushButtonGadgetClass, pull_down, wargs, n);
 XtAddCallback(wg, XmNactivateCallback, menu_which, (XtPointer) k);
 XtAddCallback(wg, XmNactivateCallback, button_callback, (XtPointer) nsym);
 }
 k++;
 if (ck_widget_count() < 0) return -1;
 wc  = n_widgets++;
 lux_widget_id[wc] = wg;
 wids[k] = wc;
 }
 return result_sym;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmgetwidgetaddress(narg, ps)
 /* (widget) */
 Int     narg, ps[];
 {
 Int	w, result_sym, wm;
 if ( get_widget_id( ps[0], &w) != 1 ) return -1;
 wm = (Int) lux_widget_id[w]; 
 result_sym = scalar_scratch(2);		/*for widget id */
 sym[result_sym].spec.scalar.l = wm;
 return result_sym;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmarrow(narg, ps) /* (parent, direction, callback, [color]) */
 /* creates an arrow widget, returns widget id */
 Int     narg, ps[];
 {
 Int	iq, nsym, w, arrow, result_sym, mode;
 char	*s;
 if (ck_motif() != 1) return -1;
 /* get parent widget */
 if ( get_widget_id( ps[0], &w) != 1 ) return -1;
 /* get direction */
 if (int_arg_stat(ps[1], &mode) != 1) return -1;
 /* get callback string */
 iq = ps[2];
 if (sym[iq].class != 2) { return execute_error(70); }
 s = (char *) sym[iq].spec.array.ptr;
 if (strlen(s) == 0 )  nsym = 0;  else	nsym = lux_execute_symbol(s,1);

 n = 0;
 switch (mode) {
 case 0: XtSetArg(wargs[n], XmNarrowDirection, XmARROW_LEFT); break;
 case 1: XtSetArg(wargs[n], XmNarrowDirection, XmARROW_RIGHT); break;
 case 2: XtSetArg(wargs[n], XmNarrowDirection, XmARROW_UP); break;
 default: XtSetArg(wargs[n], XmNarrowDirection, XmARROW_DOWN); break;
 }
 n++;
 /* now the optional color */
 if (narg > 3) {
  if (setup_colors (ps[3] ) != 1) return -1; }	/* do colors based on bg */
 if (ck_widget_count() < 0) return -1;
 arrow = n_widgets++;
 lux_widget_id[arrow] = XmCreateArrowButton(lux_widget_id[w], s, wargs, n);
 XtManageChild(lux_widget_id[arrow]);
 XtAddCallback(lux_widget_id[arrow], XmNactivateCallback,
                button_callback, (XtPointer) nsym);
 result_sym = scalar_scratch(2);		/*for widget id */
 sym[result_sym].spec.scalar.l = arrow;
 return result_sym;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmhscale(narg, ps)/*(parent,min,max,dshift,callback,[show,size,drag]) */
 /* creates a horizontal scale widget, returns widget id */
 Int     narg, ps[];
 {
 return lux_int_xmscale(narg, ps, 1);	/* the 1 indicates horizontal */
 }
 /*------------------------------------------------------------------------- */
Int lux_xmvscale(narg, ps)/*(parent,min,max,dshift,callback,[show,size,drag]) */
 /* creates a vertical scale widget, returns widget id */
 Int     narg, ps[];
 {
 return lux_int_xmscale(narg, ps, 0);	/* the 0 indicates vertical */
 }
 /*------------------------------------------------------------------------- */
Int lux_int_xmscale(narg, ps, mode)	/* internal, mode = 1 for horizontal */
 Int     narg, ps[], mode;
 {
 Int	iq, nsym, w, scale, result_sym, min, max, dshift, show, size;
 Int	drag_flag=0;
 char *s;
 if (ck_motif() != 1) return -1;
 /* get parent widget */
 if ( get_widget_id( ps[0], &w) != 1 ) return -1;
 /* get min and max */
 if (int_arg_stat(ps[1], &min) != 1) return -1;
 if (int_arg_stat(ps[2], &max) != 1) return -1;
 /* get decimal shift */
 if (int_arg_stat(ps[3], &dshift) != 1) return -1;
 /* get callback string */
 iq = ps[4];
 if (sym[iq].class != 2) { return execute_error(70); }
 s = (char *) sym[iq].spec.array.ptr;
 if (strlen(s) == 0 )  nsym = 0;  else	nsym = lux_execute_symbol(s,1);
 show = 1;	size = 0;
 if (narg > 5) if (int_arg_stat(ps[5], &show) != 1) return -1;
 if (narg > 6) if (int_arg_stat(ps[6], &size) != 1) return -1;
 if (narg > 7) if (int_arg_stat(ps[7], &drag_flag) != 1) return -1;
 n = 0;
 if (show != 0 ) XtSetArg(wargs[n], XmNshowValue, True);
 	else	 XtSetArg(wargs[n], XmNshowValue, False);	n++;
 XtSetArg(wargs[n], XmNmaximum, max); n++;
 XtSetArg(wargs[n], XmNminimum, min); n++;
 XtSetArg(wargs[n], XmNdecimalPoints, dshift); n++;
 if (size != 0 && mode == 0) { XtSetArg(wargs[n], XmNscaleHeight, size); n++; }
 if (mode) { XtSetArg(wargs[n], XmNorientation, XmHORIZONTAL); } else {
 	XtSetArg(wargs[n], XmNorientation, XmVERTICAL); } ; n++;
 if (mode) { XtSetArg(wargs[n], XmNprocessingDirection, XmMAX_ON_RIGHT);
  } else { XtSetArg(wargs[n], XmNprocessingDirection, XmMAX_ON_TOP); } n++;
 if (ck_widget_count() < 0) return -1;
 scale = n_widgets++;
 lux_widget_id[scale] = XmCreateScale(lux_widget_id[w], "scale", wargs, n);
 XtManageChild(lux_widget_id[scale]);
 XtAddCallback(lux_widget_id[scale], XmNvalueChangedCallback,
                scale_callback, (XtPointer) nsym);
 if (drag_flag != 0) {
 XtAddCallback(lux_widget_id[scale], XmNdragCallback,
                scale_callback, (XtPointer) nsym);
	}
 result_sym = scalar_scratch(2);		/*for widget id */
 sym[result_sym].spec.scalar.l = scale;
 return result_sym;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmscalesetvalue(narg, ps) /* (widget, value) */
 Int     narg, ps[];
 {
 Int	w, value;
  if (ck_motif() != 1) return -1;
 if (get_widget_id(ps[0], &w) != 1) return -1;
 if (int_arg_stat(ps[1], &value) != 1) return -1;
 
 XmScaleSetValue(lux_widget_id[w], value);
 return 1;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmscalegetvalue(narg, ps) /* (widget) */
 Int     narg, ps[];
 {
 Int	w, result_sym, value;
  if (ck_motif() != 1) return -1;
 if (get_widget_id(ps[0], &w) != 1) return -1;
 
 XmScaleGetValue(lux_widget_id[w], &value);
 result_sym = scalar_scratch(2);		/* for returned value */
 sym[result_sym].spec.scalar.l = value;
 return result_sym;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmscaleresetlimits(narg, ps) /* (widget, min, max,[dshift]) */
 Int     narg, ps[];
 {
 Int	w, min, max, dshift;
  if (ck_motif() != 1) return -1;
 if (get_widget_id(ps[0], &w) != 1) return -1;
 /* get min and max */
 if (int_arg_stat(ps[1], &min) != 1) return -1;
 if (int_arg_stat(ps[2], &max) != 1) return -1;
 /* get decimal shift, optional */
 if (narg>3)  { if (int_arg_stat(ps[3], &dshift) != 1) return -1; }
 
 XtUnmanageChild(lux_widget_id[w]);
 n = 0;
 XtSetArg(wargs[n], XmNminimum, min); n++;
 XtSetArg(wargs[n], XmNmaximum, max); n++;
 if (narg>3)  { XtSetArg(wargs[n], XmNdecimalPoints, dshift); n++; }
 XtSetValues(lux_widget_id[w], wargs, n);
 XtManageChild(lux_widget_id[w]);
 return 1;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmattach(narg, ps)
 /* (widget, ref_widget, left, right, top, bottom) */
 /* top, etc are 1 for attachment on that side and 0 for no change */
 /* -1 for attach opposite */
 /* the parent should be a form widget, use ref_widget = 0 to make attachments
 to the parent, otherwise ref_widget must be a sibling widget (with the same
 parent */
 Int     narg, ps[];
 {
 Int	w1, w2, attach[4], i;
 if (ck_motif() != 1) return -1;
 /* get the arguments */
 if (get_widget_id(ps[0], &w1) != 1) return -1;
 if (get_widget_id(ps[1], &w2) != 1) return -1;
 for (i=0; i < 4; i++) {
	if (int_arg_stat(ps[i+2], &attach[i]) != 1) return -1; }
 /* try to include both attaches to a parent form and attaches to sibling widgets
 by checking if ref_widget is 0, which we assume to mean parent */
 n = 0;
 if (w2 == 0)
  {	/* assuming we want to attach to some side(s) of parent */
   if (attach[0]) { XtSetArg(wargs[n], XmNleftAttachment, XmATTACH_FORM); n++; }
   if (attach[1]) { XtSetArg(wargs[n], XmNrightAttachment, XmATTACH_FORM); n++; }
   if (attach[3]) { XtSetArg(wargs[n], XmNbottomAttachment, XmATTACH_FORM); n++;}
   if (attach[2]) { XtSetArg(wargs[n], XmNtopAttachment, XmATTACH_FORM); n++; }
 /* printf("in attach, mark 3, w1,n = %d %d\n", w1, n); */
   if (n != 0 ) XtSetValues(lux_widget_id[w1], wargs, n);
 /* printf("in attach, mark 4\n"); */
  } else {
  /* assuming we are attaching to a sibling widget */
   if (attach[2]) {
   	if (attach[2] < 0) {
	XtSetArg(wargs[n], XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET); n++;
   	} else {
	XtSetArg(wargs[n], XmNtopAttachment, XmATTACH_WIDGET); n++; }
   	XtSetArg(wargs[n], XmNtopWidget, lux_widget_id[w2]); n++; }
   if (attach[1]) {
   	if (attach[1] < 0) {
	XtSetArg(wargs[n], XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET); n++;
   	} else {
	XtSetArg(wargs[n], XmNrightAttachment, XmATTACH_WIDGET); n++; }
   	XtSetArg(wargs[n], XmNrightWidget, lux_widget_id[w2]); n++; }
   if (attach[3]) {
   	if (attach[3] < 0) {
	XtSetArg(wargs[n], XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET); n++;
   	} else {
	XtSetArg(wargs[n], XmNbottomAttachment, XmATTACH_WIDGET); n++; }
   	XtSetArg(wargs[n], XmNbottomWidget, lux_widget_id[w2]); n++; }
   if (attach[0]) {
   	if (attach[0] < 0) {
	XtSetArg(wargs[n], XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET); n++;
   	} else {
	XtSetArg(wargs[n], XmNleftAttachment, XmATTACH_WIDGET); n++; }
   	XtSetArg(wargs[n], XmNleftWidget, lux_widget_id[w2]); n++; }
   if (n != 0 ) XtSetValues(lux_widget_id[w1], wargs, n);
   }
 return 1;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmattach_relative(narg, ps)
 /* (widget, left, right, top, bottom) */
 /* top, etc are relative positions (in percent) for attachment on that side */
 /* the parent should be a form widget */
 Int     narg, ps[];
 {
 Int	w1, attach[4], i;
 if (ck_motif() != 1) return -1;
 /* get the arguments */
 if (get_widget_id(ps[0], &w1) != 1) return -1;
 for (i=0; i < 4; i++) {
	if (int_arg_stat(ps[i+1], &attach[i]) != 1) return -1; }
 n = 0;
 XtSetArg(wargs[n], XmNleftAttachment, XmATTACH_POSITION); n++;
 XtSetArg(wargs[n], XmNleftPosition, attach[0]); n++;
 XtSetArg(wargs[n], XmNrightAttachment, XmATTACH_POSITION); n++;
 XtSetArg(wargs[n], XmNrightPosition, attach[1]); n++;
 XtSetArg(wargs[n], XmNbottomAttachment, XmATTACH_POSITION); n++;
 XtSetArg(wargs[n], XmNbottomPosition, attach[3]); n++;
 XtSetArg(wargs[n], XmNtopAttachment, XmATTACH_POSITION); n++;
 XtSetArg(wargs[n], XmNtopPosition, attach[2]); n++;
 XtSetValues(lux_widget_id[w1], wargs, n);
 return 1;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmposition(narg, ps) /* (widget, x, y, [sx, sy]) */
 /* note that (x,y) position is not set if values are negative */
 Int     narg, ps[];
 {
 Int	w1, x, y, dx, dy;
 if (ck_motif() != 1) return -1;
 /* get the arguments */
 if (get_widget_id(ps[0], &w1) != 1) return -1;
 if (int_arg_stat(ps[1], &x) != 1) return -1;
 if (int_arg_stat(ps[2], &y) != 1) return -1;
 if (narg > 3) if (int_arg_stat(ps[3], &dx) != 1) return -1;
 if (narg > 4) if (int_arg_stat(ps[4], &dy) != 1) return -1;
 
 n = 0;
 if (x >= 0) { XtSetArg(wargs[n], XmNx, x); n++; }
 if (y >= 0) { XtSetArg(wargs[n], XmNy, y); n++; }
 if (narg > 3)  { XtSetArg(wargs[n], XmNwidth, dx); n++; }
 if (narg > 4)  { XtSetArg(wargs[n], XmNheight, dy); n++; }
 XtSetValues(lux_widget_id[w1], wargs, n);
 return 1;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmsize(narg, ps) /* (widget, sx, sy) */
 /* just reset the sizes */
 Int     narg, ps[];
 {
 Int	w1, dx, dy;
 if (ck_motif() != 1) return -1;
 /* get the arguments */
 if (get_widget_id(ps[0], &w1) != 1) return -1;
 if (int_arg_stat(ps[1], &dx) != 1) return -1;
 if (int_arg_stat(ps[2], &dy) != 1) return -1;
 n = 0;
 if (dx > 0) { XtSetArg(wargs[n], XmNwidth, dx); n++; }
 if (dy > 0) { XtSetArg(wargs[n], XmNheight, dy); n++; }
 XtSetValues(lux_widget_id[w1], wargs, n);
 return 1;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmresizepolicy(narg, ps) /* (widget, flag) */
 /* 3 policies available, 0 = none, 1 = grow only, 2 = any */
 Int     narg, ps[];
 {
 Int	w1, flag;
 if (ck_motif() != 1) return -1;
 /* get the arguments */
 if (get_widget_id(ps[0], &w1) != 1) return -1;
 if (int_arg_stat(ps[1], &flag) != 1) return -1;
 n = 0;
 switch (flag) {
 case 0: XtSetArg(wargs[n], XmNresizePolicy, XmRESIZE_NONE); break;
 case 1: XtSetArg(wargs[n], XmNresizePolicy, XmRESIZE_GROW); break;
 default: XtSetArg(wargs[n], XmNresizePolicy, XmRESIZE_ANY); break;
 }
 n++;
 XtSetValues(lux_widget_id[w1], wargs, n);
 return 1;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmsetmodal(narg, ps) /* (widget, modal_flag) */
 /* use with care, 0 sets modeless (turns off modal), 1 makes the widget
 modal */
 Int     narg, ps[];
 {
 Int	w1, modal_flag;
 if (ck_motif() != 1) return -1;
 /* get the arguments */
 if (get_widget_id(ps[0], &w1) != 1) return -1;
 if (int_arg_stat(ps[1], &modal_flag) != 1) return -1;
 
 n = 0;
 if (modal_flag != 0)
  { XtSetArg(wargs[n], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL); n++;}
  else
  { XtSetArg(wargs[n], XmNdialogStyle, XmDIALOG_MODELESS); n++;}
 XtSetValues(lux_widget_id[w1], wargs, n);
 return 1;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmsetdirectory(narg, ps) /* (widget, directory_path) */
 /* can only be used for file select widgets */
 Int     narg, ps[];
 {
 Int	w1;
 char *s;
 XmString	directory;
 if (ck_motif() != 1) return -1;
 /* get the arguments */
 if (get_widget_id(ps[0], &w1) != 1) return -1;
 if (!XtIsSubclass(lux_widget_id[w1], xmFileSelectionBoxWidgetClass))
	{ printf("XMSETDIRECTORY, widget not a file selection box!\n");
	return -1;}
 if (sym[ps[1]].class != 2) { return execute_error(70); }
 s = (char *) sym[ps[1]].spec.array.ptr;
 directory = XmStringCreateSimple(s);
 n = 0;
 XtSetArg(wargs[n], XmNdirectory, directory); n++;
 XtSetValues(lux_widget_id[w1], wargs, n);
 return 1;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmborderwidth(narg, ps) /* (widget, width) */
 /* manipulate the border */
 Int     narg, ps[];
 {
 Int	w1, wq;
 if (ck_motif() != 1) return -1;
 /* get the arguments */
 if (get_widget_id(ps[0], &w1) != 1) return -1;
 if (int_arg_stat(ps[1], &wq) != 1) return -1;
 
 n = 0;
 XtSetArg(wargs[n], XmNborderWidth, wq); n++;
 XtSetValues(lux_widget_id[w1], wargs, n);
 return 1;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmgetwidgetsize(narg, ps)
 /* a subroutine; i.e, xmgetwidgetsize, widget, dx, dy  returns dx, dy */
 Int     narg, ps[];
 {
 Int	w, dxx, dyy;
 Dimension	dx, dy;
 if ( get_widget_id( ps[0], &w) != 1 ) return -1;
 XtSetArg(wargs[0], XmNwidth, &dx);
 XtSetArg(wargs[1], XmNheight, &dy);
 XtGetValues(lux_widget_id[w], wargs, 2);
 dxx = (Int) dx;	dyy = (Int) dy;
 redef_scalar(ps[1], 2, &dxx);
 redef_scalar(ps[2], 2, &dyy);
 return 1;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmgetwidgetposition(narg, ps)
 /* a subroutine; i.e, xmgetwidgetposition, widget, x, y  returns x, y */
 Int     narg, ps[];
 {
 Int	w, dxx, dyy;
 Dimension	dx, dy;
 if ( get_widget_id( ps[0], &w) != 1 ) return -1;
 XtSetArg(wargs[0], XmNx, &dx);
 XtSetArg(wargs[1], XmNy, &dy);
 XtGetValues(lux_widget_id[w], wargs, 2);
 dxx = (Int) dx;	dyy = (Int) dy;
 redef_scalar(ps[1], 2, &dxx);
 redef_scalar(ps[2], 2, &dyy);
 return 1;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmalignment(narg, ps) /* (widget, mode) */
 /* makes sense for label and button widgets or their parents */
 Int     narg, ps[];
 {
 Int	w, mode;
 if (ck_motif() != 1) return -1;
 /* get the widget */
 if (get_widget_id(ps[0], &w) != 1) return -1;
 if (int_arg_stat(ps[1], &mode) != 1) return -1;
 
 n = 0;
 XtSetArg(wargs[n], XmNisAligned, True); n++;
 switch (mode) {
 case 0: XtSetArg(wargs[n], XmNalignment, XmALIGNMENT_BEGINNING); break;
 case 1: XtSetArg(wargs[n], XmNalignment, XmALIGNMENT_CENTER); break;
 case 2: XtSetArg(wargs[n], XmNalignment, XmALIGNMENT_END); break;
 }
 n++;
 XtSetValues(lux_widget_id[w], wargs, n);
 return 1;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmsensitive(narg, ps) /* (widget, mode) */
 /* use to make labels or button insensitive or sensitive */
 /* this doesn't make sense for all types of widgets */
 Int     narg, ps[];
 {
 Int	w, mode;
 if (ck_motif() != 1) return -1;
 /* get the widget */
 if (get_widget_id(ps[0], &w) != 1) return -1;
 if (int_arg_stat(ps[1], &mode) != 1) return -1;
 
 n = 1;
 if (mode != 0) XtSetArg(wargs[n], XmNsensitive, True); else
 	XtSetArg(wargs[n], XmNsensitive, False);
 XtSetValues(lux_widget_id[w], wargs, n);
 return 1;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmfont(narg, ps) /* (widget, font_string) */
 Int     narg, ps[];
 {
 Int	w, iq;
 char    *fontname;
  if (ck_motif() != 1) return -1;
 if (get_widget_id(ps[0], &w) != 1) return -1;
 /* get the font string */
 iq = ps[1];
 if (sym[iq].class != 2) return execute_error(70);
 fontname = (char *) sym[iq].spec.array.ptr;
 /* note that font and fontlist are common and reused, may have a malloc
 leak here, not sure if these should be freed after each use */
 if ( (font = XLoadQueryFont(disp, fontname)) == NULL)
  { font_not_available();  return 1; }
 
 n = 0;
 fontlist = XmFontListCreate(font, cset);
 XtSetArg(wargs[n], XmNfontList, fontlist); n++;
 XtSetValues(lux_widget_id[w], wargs, n);
 return 1;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmsetlabel(narg, ps) /* (widget, string) */
 Int     narg, ps[];
 {
 Int	w, iq;
 char    *s;
 XmString	ms;
 if (ck_motif() != 1) return -1;
 if (get_widget_id(ps[0], &w) != 1) return -1;
 /* get the string */
 iq = ps[1];
 if (sym[iq].class != 2) return execute_error(70);
 s = (char *) sym[iq].spec.array.ptr;
 ms = XmStringCreateLtoR(s, cset);
 n = 0;
 XtSetArg(wargs[n], XmNlabelString, ms); n++;
 XtSetValues(lux_widget_id[w], wargs, n);
 XmStringFree(ms);
 return 1;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmsettitle(narg, ps) /* (widget, string) */
 Int     narg, ps[];
 {
 Int	w, iq;
 char    *s;
 XmString	title;
 if (ck_motif() != 1) return -1;
 if (get_widget_id(ps[0], &w) != 1) return -1;
 /* get the string */
 iq = ps[1];
 if (sym[iq].class != 2) return execute_error(70);
 s = (char *) sym[iq].spec.array.ptr;
 title = XmStringCreateLtoR(s, cset);
 n = 0;
 XtSetArg(wargs[n], XmNdialogTitle, title); n++;
 XtSetValues(lux_widget_id[w], wargs, n);
 XmStringFree(title);
 return 1;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmsetmnemonic(narg, ps) /* (widget, string) */
 /* string should be a single character, we take only the first if not */
 Int     narg, ps[];
 {
 Int	w, iq;
 char    *s;
 if (ck_motif() != 1) return -1;
 if (get_widget_id(ps[0], &w) != 1) return -1;
 /* get the string */
 iq = ps[1];
 if (sym[iq].class != 2) return execute_error(70);
 s = (char *) sym[iq].spec.array.ptr;
 n = 0;
 XtSetArg(wargs[n], XmNmnemonic, *s); n++;
 XtSetValues(lux_widget_id[w], wargs, n);
 return 1;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmsetmargins(narg, ps) /* (widget, xm, ym) */
 Int     narg, ps[];
 {
 Int	w, xm, ym;
 if (ck_motif() != 1) return -1;
 if (get_widget_id(ps[0], &w) != 1) return -1;
 if (int_arg_stat(ps[1], &xm) != 1) return -1;
 if (int_arg_stat(ps[2], &ym) != 1) return -1;
 n = 0;
 XtSetArg(wargs[n], XmNmarginWidth, xm); n++;
 XtSetArg(wargs[n], XmNmarginHeight, ym); n++;
 XtSetValues(lux_widget_id[w], wargs, n);
 return 1;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmtextsetrowcolumnsize(narg, ps) /* (widget, rows, columns) */
 /* reset the number of rows and columns for a text image */
 Int     narg, ps[];
 {
 Int	w, rows, cols;
 if (ck_motif() != 1) return -1;
 if (get_widget_id(ps[0], &w) != 1) return -1;
 if (int_arg_stat(ps[1], &rows) != 1) return -1;
 if (int_arg_stat(ps[2], &cols) != 1) return -1;

 n = 0;
 XtSetArg(wargs[n], XmNrows, rows); n++;
 XtSetArg(wargs[n], XmNcolumns, cols); n++;
 XtSetValues(lux_widget_id[w], wargs, n);
 return 1;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmforegroundcolor(narg, ps) /* (widget, color) */
 Int     narg, ps[];
 { return colorset(narg, ps, 1); }
 /*------------------------------------------------------------------------- */
Int lux_xmbackgroundcolor(narg, ps) /* (widget, color) */
 Int     narg, ps[];
 { return colorset(narg, ps, 0); }
 /*------------------------------------------------------------------------- */
Int lux_xmtopshadowcolor(narg, ps) /* (widget, color) */
 Int     narg, ps[];
 { return colorset(narg, ps, 2); }
 /*------------------------------------------------------------------------- */
Int lux_xmbottomshadowcolor(narg, ps) /* (widget, color) */
 Int     narg, ps[];
 { return colorset(narg, ps, 3); }
 /*------------------------------------------------------------------------- */
Int lux_xmselectcolor(narg, ps) /* (widget, color) */
 Int     narg, ps[];
 { return colorset(narg, ps, 4); }
 /*------------------------------------------------------------------------- */
Int lux_xmarmcolor(narg, ps) /* (widget, color) */
 Int     narg, ps[];
 { return colorset(narg, ps, 5); }
 /*------------------------------------------------------------------------- */
Int lux_xmbordercolor(narg, ps) /* (widget, color) */
 Int     narg, ps[];
 { return colorset(narg, ps, 6); }
 /*------------------------------------------------------------------------- */
Int colorset(narg, ps, mode) /* internal routine */
 Int     narg, ps[], mode;
 {
 Int	w, iq;
 char    *pc;
 if (ck_motif() != 1) return -1;
 if (get_widget_id(ps[0], &w) != 1) return -1;
 /* get the color string */
 iq = ps[1];
 if (sym[iq].class != 2) return execute_error(70);
 /* try to figure out the color */
 /* a string ? */
 if ( sym[ ps[1] ].class != 2 ) return execute_error(70);
 pc = (char *) sym[ps[1] ].spec.array.ptr;
 if ( strlen(pc) <= 0 ) return 1;	/* a null, do nothing */
 if ( XAllocNamedColor(disp, cmap, pc, &colorcell, &rgb_def) == 0 )
	{ color_not_available(); return 1; }
 
 n = 0;
 switch (mode)
 {
 case  0: XtSetArg(wargs[n], XmNbackground, colorcell.pixel); break;
 case  1: XtSetArg(wargs[n], XmNforeground, colorcell.pixel); break;
 case  2: XtSetArg(wargs[n], XmNtopShadowColor, colorcell.pixel); break;
 case  3: XtSetArg(wargs[n], XmNbottomShadowColor, colorcell.pixel); break;
 case  4: XtSetArg(wargs[n], XmNselectColor, colorcell.pixel); break;
 case  5: XtSetArg(wargs[n], XmNarmColor, colorcell.pixel); break;
 case  6: XtSetArg(wargs[n], XmNborderColor, colorcell.pixel); break;
 default: return execute_error(3);
 }
 n++;
 XtSetValues(lux_widget_id[w], wargs, n);
 return 1;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmsetcolors(narg, ps) /* (widget, bg_color) */
 Int     narg, ps[];
 /* given a new background color for a widget, set it and an "appropiate"
 set of the other colors */
 {
 Int	w, iq;
 if (ck_motif() != 1) return -1;
 if (get_widget_id(ps[0], &w) != 1) return -1;
 /* get the color string */
 iq = ps[1];
 n = 0;
 setup_colors (iq);
 XtSetValues(lux_widget_id[w], wargs, n);
 return 1;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmquery(narg, ps) /* (widget) */
 Int     narg, ps[];
 /* a subroutine, gets pointer location in this widget and for root window */
 {
 Int	w, status;
 Window	qroot, qchild;
 Window win;
 extern  Int	xcoord, ycoord, root_x, root_y;
 extern  uint32_t    kb;

 if ( get_widget_id( ps[0], &w) != 1 ) return -1;
 win = XtWindowOfObject(lux_widget_id[w]);
 if (win == 0) {
 printf("XTWINDOW: no such window, invalid object or object not yet realized\n");
 return -1; }
 status = XQueryPointer(disp, win, &qroot, &qchild, &root_x,
 &root_y,&xcoord, &ycoord, &kb);
 /*printf("qroot, qchild = %d, %d\n", (Int) qroot, (Int) qchild);*/
 return 1;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmraise(narg, ps) /* (widget) */
 Int     narg, ps[];
 /* a subroutine, gets window for this widget and does a xraise on it */
 {
 Int	w;
 Window win;

 if ( get_widget_id( ps[0], &w) != 1 ) return -1;
 win = XtWindowOfObject(lux_widget_id[w]);
 if (win == 0) {
 printf("XTWINDOW: no such window, invalid object or object not yet realized\n");
 return -1; }
 XRaiseWindow(disp, win);
 return 1;
 }
 /*------------------------------------------------------------------------- */
Int lux_xtparent(narg, ps)
 /* (widget) */
 Int     narg, ps[];
 {
 Int	w, result_sym, parent;
 if ( get_widget_id( ps[0], &w) != 1 ) return -1;
 if (ck_widget_count() < 0) return -1;
 parent = n_widgets++;
 lux_widget_id[parent] = XtParent(lux_widget_id[w]);
 result_sym = scalar_scratch(2);		/*for widget id */
 sym[result_sym].spec.scalar.l = parent;
 return result_sym;
 }
 /*------------------------------------------------------------------------- */
Int lux_xtwindow(narg, ps)
 /* (widget) */
 Int     narg, ps[];
 {
 Int	w, result_sym;
 Window wid;
 if ( get_widget_id( ps[0], &w) != 1 ) return -1;
 wid = XtWindowOfObject(lux_widget_id[w]);
 if (wid == 0) {
 printf("XTWINDOW: no such window, invalid object or object not yet realized\n");
 return -1; }
 result_sym = scalar_scratch(2);		/*for window id */
 sym[result_sym].spec.scalar.l = wid;
 return result_sym;
 }
 /*------------------------------------------------------------------------- */
Int lux_xtmanage(narg, ps)
 /* (widget) */
 Int     narg, ps[];
 {
 Int	w;
 if ( get_widget_id( ps[0], &w) != 1 ) return -1;
 XtManageChild(lux_widget_id[w]);
 return 1;
 }
 /*------------------------------------------------------------------------- */
Int lux_xtunmanage(narg, ps)
 /* (widget) */
 Int     narg, ps[];
 {
 Int	w;
 if ( get_widget_id( ps[0], &w) != 1 ) return -1;
 XtUnmanageChild(lux_widget_id[w]);
 return 1;
 }
 /*------------------------------------------------------------------------- */
Int lux_xtpopup(narg, ps)
 /* xtpopup, widget */
 Int     narg, ps[];
 {
 Int	w;
 if ( get_widget_id( ps[0], &w) != 1 ) return -1;
 XtPopup(xmgettopshell(lux_widget_id[w]), XtGrabNone);
 return 1;
 }
 /*------------------------------------------------------------------------- */
Int lux_xtpopdown(narg, ps)
 /* xtpopdown, widget */
 Int     narg, ps[];
 {
 Int	w;
 if ( get_widget_id( ps[0], &w) != 1 ) return -1;
 XtPopdown(xmgettopshell(lux_widget_id[w]));
 return 1;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmdestroy(narg, ps)
 Int     narg, ps[];
 {
 Int	w;
 if ( get_widget_id( ps[0], &w) != 1 ) return -1;
 XtDestroyWidget(lux_widget_id[w]);
 return 1;
 }
 /*------------------------------------------------------------------------- */
Int lux_xmset_text_output(narg, ps) /* assigns a widget text_output */
 /* this takes an already defined widget number in the LUX environment and
 assigns it to the text_output widget used by wprint */
 Int     narg, ps[];
 {
 Int	w;
 if ( get_widget_id( ps[0], &w) != 1 ) return -1;
 text_output = lux_widget_id[w];
 return 1;
 }
 /*------------------------------------------------------------------------- */
Int get_widget_id( nsym, w)
 Int nsym, *w;
 {
 /* verify that symbol is an integer */
 if (int_arg_stat(nsym, w) != 1) {
 	printf("widget ID was not a scalar or undefined\n");
 	return -1; }
 if ( *w < 0 || *w >= n_widgets )
 	{ printf("invalid widget ID %d, valid range 0 to %d\n", *w, n_widgets-1);
	return -1; }
 return 1;
 }
 /*------------------------------------------------------------------------- */
void lux_xminit(void) 	/* the motif initialization */
{
 extern	Int	display_width, display_height;
 Widget   exit_button;
 Screen	*screen;
 Int	argc = 1;
 Pixmap  icon_pixmap;
 char *argv = {"lux motif"};
 extern Int	connect_flag, depth;	/* from color.c */
 extern Display	*display;
 extern Visual	*visual;

 toplevel = 0;
 n_widgets++;

 /* we want to set our own colormap.  The Motif Programmers' Guide says */
 /* not to use XtAppInitialize, but rather to do all the steps ourselves */
 /* and to give the colormap as an argument to XtAppCreateShell.  LS 12mar99 */

 /* make sure we're connected */
 if (!connect_flag)
   setup_x();

 XtToolkitInitialize();
 app_context = XtCreateApplicationContext();
 /* setting a widget name and class is good policy -- just to be safe. */
 XtDisplayInitialize(app_context, display, "lux", "Lux", NULL, 0,
		     &argc, &argv);
 disp = display;		/* instead of replacing all "disp"s. */
 lux_widget_id[toplevel] =
   XtAppCreateShell("lux", "Lux", applicationShellWidgetClass, disp, NULL, 0);

 exit_button_text = XmStringCreateLtoR("interrupt motif",
				       XmSTRING_DEFAULT_CHARSET);
 icon_pixmap = XCreateBitmapFromData(disp, DefaultRootWindow(display),
				     icon_bitmap_bits, icon_bitmap_width,
				     icon_bitmap_height);

 n=0;
 XtSetArg(wargs[n], XmNiconPixmap, icon_pixmap); n++;

 /* NOTE: we *must* define the visual and depth or we get BadMatch */
 /* errors when using visuals and depths other than the default ones! */
 /* LS 31mar99 - with thanks to Chad M. Fraleigh */
 XtSetArg(wargs[n], XmNcolormap, colorMap); n++; /* added LS 12mar99 */
 XtSetArg(wargs[n], XmNvisual, visual); n++; /* added LS 31mar99 */
 XtSetArg(wargs[n], XmNdepth, depth); n++; /* added LS 31mar99 */
 cmap = colorMap;		/* instead of replacing all cmaps */


 /* XtSetArg (wargs[n],XmNiconic, True);	n++; */
 XtSetValues(lux_widget_id[toplevel], wargs, n);

 n=0;
 if ((font =
      XLoadQueryFont(disp, "-adobe-helvetica-bold-r-normal--14*")) == NULL) 
   font_not_available(); 
 else {
   fontlist = XmFontListCreate(font, cset);
   XtSetArg(wargs[n], XmNfontList, fontlist); n++;
 }
 XtSetArg(wargs[n],XmNlabelString, exit_button_text); n++;
 XtSetArg(wargs[n],XmNwidth, 200);	n++;
 XtSetArg(wargs[n],XmNheight, 50);	n++;

 exit_button = XtCreateManagedWidget("exit_button", xmPushButtonWidgetClass,
				     lux_widget_id[toplevel], wargs, n);
 XtAddCallback(exit_button, XmNactivateCallback, quit_callback, NULL);
 XmStringFree(exit_button_text);
 /* put in a particular place */
 screen = XtScreen(lux_widget_id[toplevel]);
 display_width = WidthOfScreen(screen);
 display_height = HeightOfScreen(screen);
 n = 0;
 XtSetArg(wargs[n], XmNx, (Position) (display_width-200)); n++;
 XtSetArg(wargs[n], XmNy, (Position) 10); n++;
 XtSetValues(lux_widget_id[toplevel], wargs, n);
}
 /*------------------------------------------------------------------------- */
Int ck_motif()
 {
 if (motif_init_flag == 0) { lux_xminit(); motif_init_flag = 1; }
 return 1;
 }
 /*------------------------------------------------------------------------- */
Int ck_widget_count()
 {
 Int	mx;
 mx = MAXWIDGETS;
 if ( (n_widgets +1000) > mx) {
 	printf("WARNING - widget table is getting large, n = %d\n", n_widgets);
	if ( (n_widgets +10) > mx) {
	printf("TOO MANY WIDGETS, widget creation now turned off, n = %d\n",
		n_widgets); return -1; } }
 return 1;
 }
 /*------------------------------------------------------------------------- */
Int lux_xtloop(narg, ps)
 Int     narg, ps[];
 {
 extern Int	ck_events();
 /* have we been realized ? */
 if ( motif_realized_flag != 1) {
	XtRealizeWidget(lux_widget_id[toplevel]);
	motif_realized_flag = 1; }

 /* 6/13/95, accepts a single argument and sets !motif to the value */
 
 motif_flag = 1;
 if (narg>0) if (int_arg_stat(ps[0], &motif_flag) != 1) return -1;
 /* 8/26/94 changed again
 if motif_flag = 1 (!motif in lux) we just loop until someone resets !motif
 if motif_flag = 0, we just check if anything pending
 if motif_flag = 2, we force a flush and update display and then do
 	anything pending
 the 0 case allows a quick execution for a check in a loop, the
 flush and update slow it down a lot
 */
 xtloop_running = 1;	/* to indicate we are running in here */
 switch (motif_flag)
 {
 case 1:
  for (;;) { if (motif_flag == 0) break;
  ck_events();
  XtAppNextEvent(app_context, &event);
  /* printf("event type was: %d\n", event.type); */
  XtDispatchEvent(&event);
  } break;
 case 2:
  XFlush(disp);
  XmUpdateDisplay(lux_widget_id[toplevel]);
 case 0:
  /* while (XtAppPending(app_context) != NULL) { */
  while (XtAppPending(app_context) & XtIMXEvent) {
  XtAppNextEvent(app_context, &event);
  XtDispatchEvent(&event);
  } break;
 }
 
 /*printf("escaped motif\n");*/
 xtloop_running = 0;	/* to indicate we are done in here */
 return 1;
 }
 /*------------------------------------------------------------------------- */
void color_not_available()
 {
 printf("color not available, your widget may look funny\n");
 }
 /*------------------------------------------------------------------------- */
void font_not_available()
 {
 printf("font not available, your widget may look funny\n");
 }
 /*------------------------------------------------------------------------- */
void browse_callback(w, ptq, call_data)
 Widget     w;
 XtPointer	ptq;
 XmListCallbackStruct *call_data;
 {
 Int	mq;
 char *text, *p, *s;
 Int	result_position, result_string;
 switch (call_data->reason) {
  case XmCR_BROWSE_SELECT:
   XmStringGetLtoR (call_data->item, cset, &text);
   break;
  default:
   printf("unexpected reason = %d\n", call_data->reason);
  }
 mq = strlen(text);
 result_string = find_sym(list_item);
 redef_string(result_string, mq);
 p = (char *) sym[result_string].spec.array.ptr;	s = text;
 while (mq--) *p++ = *s++;	*p = 0;			/*load it */
 result_position = find_sym(list_item_position);
 redef_scalar(result_position, 2, &call_data->item_position);
 XtFree(text);
 (void) lux_callback_execute( (Int) ptq);
 }
 /*------------------------------------------------------------------------- */
void fselect_callback(w, ptq, call_data)
 Widget     w;
 XmFileSelectionBoxCallbackStruct *call_data;
 XtPointer	ptq;
 {
 Int	mq;
 Int	result_string;
 char *filename, *p, *s;
 switch (call_data->reason) {
  case XmCR_OK:
   XmStringGetLtoR (call_data->value, cset, &filename);
   break;
  default:
   printf("fselect_callback, unexpected reason = %d\n", call_data->reason);
  }
 mq = strlen(filename);
 result_string = find_sym(selected_file);
 redef_string(result_string, mq);
 p = (char *) sym[result_string].spec.array.ptr;
 s = filename;
 while (mq--) *p++ = *s++;	*p = 0;			/*load it */
 XtFree(filename);

 (void) lux_callback_execute( (Int) ptq);
 }
 /*------------------------------------------------------------------------- */
void fhelp_callback(w, ptq, call_data)
 Widget     w;
 XmFileSelectionBoxCallbackStruct *call_data;
 XtPointer	ptq;
 {
 Int	mq;
 Int	result_string;
 char *filename, *p, *s;

 XmStringGetLtoR (call_data->value, cset, &filename);
 mq = strlen(filename);
 result_string = find_sym(selected_file);
 redef_string(result_string, mq);
 p = (char *) sym[result_string].spec.array.ptr;	s = filename;
 while (mq--) *p++ = *s++;	*p = 0;			/*load it */
 XtFree(filename);
 (void) lux_callback_execute( (Int) ptq);
 }
 /*------------------------------------------------------------------------- */
void fcancel_callback(w, ptq, call_data)
 Widget     w;
 XmFileSelectionBoxCallbackStruct *call_data;
 XtPointer	ptq;
 {
 (void) lux_callback_execute( (Int) ptq);
 }
 /*------------------------------------------------------------------------- */
void bcancel_callback(w, ptq, call_data)
 Widget     w;
 XmAnyCallbackStruct *call_data;
 XtPointer	ptq;
 {
 (void) lux_callback_execute( (Int) ptq);
 }
 /*------------------------------------------------------------------------- */
void command_callback(w, ptq, call_data)
 /* similar to browse_callback but need a different structure */
 Widget     w;
 XmCommandCallbackStruct *call_data;
 XtPointer	ptq;
 {
 Int	mq;
 char *text, *p, *s;
 Int	result_string;
 switch (call_data->reason) {
  case XmCR_COMMAND_ENTERED:
   XmStringGetLtoR (call_data->value, cset, &text);
   break;
  default:
   printf("unexpected reason = %d\n", call_data->reason);
  }
 mq = strlen(text);
 result_string = find_sym(command_item);
 redef_string(result_string, mq);
 p = (char *) sym[result_string].spec.array.ptr;	s = text;
 while (mq--) *p++ = *s++;	*p = 0;			/*load it */

 (void) lux_callback_execute( (Int) ptq);
 }
 /*------------------------------------------------------------------------- */
void lux_callback_execute(nsym)
 Int nsym;
 {
 Int iq;
 if (nsym > 0) { iq = execute(nsym);
 	if (iq <= 0) printf("problem with callback execution\n"); }
 }
 /*------------------------------------------------------------------------- */
void button_callback(w, ptq, call_data)
 Widget	w;
 caddr_t	call_data;
 XtPointer	ptq;
 {
 (void) lux_callback_execute( (Int) ptq);
 }
 /*------------------------------------------------------------------------- */
void selectionbox_cb(w, ptq, call_data)
 Widget	w;
 XmSelectionBoxCallbackStruct	*call_data;
 XtPointer	ptq;
 {
 Int	result_string, mq;
 char *text, *p, *s;
 /* get the value in the textfield */
 XmStringGetLtoR (call_data->value, cset, &text);
 mq = strlen(text);
 result_string = find_sym(textfield_value);
 redef_string(result_string, mq);
 p = (char *) sym[result_string].spec.array.ptr;	s = text;
 while (mq--) *p++ = *s++;	*p = 0;			/*load it */
 XtFree(text);
 (void) lux_callback_execute( (Int) ptq);
 zap(run_block_number((Int) ptq)); /* get rid of callback - LS 20jan99 */
 }
 /*------------------------------------------------------------------------- */
void radio_which(w, ptq, state)
 Widget	w;
 XmToggleButtonCallbackStruct	*state;
 XtPointer	ptq;
 {
 /*
 printf("radio_which widget = %d\n", w);
 printf("%s: %s\n", XtName(w), state->set ? "on" : "off");
 */
 if (state->set) radio_state = 1; else radio_state = 0;
 radio_button = (Int) ptq;
 }
 /*------------------------------------------------------------------------- */
void radio_callback(w, ptq, state)
 Widget	w;
 XmToggleButtonCallbackStruct	*state;
 XtPointer	ptq;
 {
 /* 7/11/95, changed to callback lux routine for either state set on or off */
 (void) lux_callback_execute( (Int) ptq);
 }
 /*------------------------------------------------------------------------- */
void menu_which(w, ptq, state)
 Widget	w;
 XmToggleButtonCallbackStruct	*state;
 XtPointer	ptq;
 {
 Int	iq, which;
 /* printf("menu_which called\n"); */
 iq = find_sym(option_value);
 which = (Int) ptq;
 redef_scalar(iq, 2, &which);
 }
 /*------------------------------------------------------------------------- */
void scale_callback(w, ptq, call_data)
 Widget	w;
 XmScaleCallbackStruct *call_data;
 XtPointer	ptq;
 {
 /* check if a bogus call */
 if ( call_data != (XmScaleCallbackStruct *) NULL)
  {
  redef_scalar(find_sym(scale_value), 2, &call_data->value);
 (void) lux_callback_execute( (Int) ptq);
  }
 }
 /*------------------------------------------------------------------------- */
void scroll_callback(w, ptq, call_data)
 Widget	w;
 XmScaleCallbackStruct *call_data;
 XtPointer	ptq;
 {
 /* check if a bogus call */
 if ( call_data != (XmScaleCallbackStruct *) NULL)
  {
  redef_scalar(find_sym(scroll_value), 2, &call_data->value);
 (void) lux_callback_execute( (Int) ptq);
  }
 }
 /*------------------------------------------------------------------------- */
void draw_in_callback(w, ptq, call_data)
 Widget	w;
 XmDrawingAreaCallbackStruct *call_data;
 Int	*ptq;
 {
 extern	Int	lux_keycode, lux_button, last_wid, xcoord, ycoord;
 extern	Int	root_x, root_y, lux_keystate, lux_keysym;
 Int	nc;
 XEvent  *report = call_data->event;
 char	buffer[16];
 KeySym	keysym;
 /*assume no key and no button down until we find different */
 lux_keycode = lux_button = lux_keysym = lux_keystate = 0;
 last_wid = *(ptq+1);	/* which lux X window the event occurred in */
 /* check type of input */
 switch (report->xany.type) {
  case ButtonPress:
    lux_button = report->xbutton.button;
  case ButtonRelease:
    xcoord = report->xbutton.x;
    ycoord = report->xbutton.y;
    root_x = report->xbutton.x_root;
    root_y = report->xbutton.y_root;
    lux_keystate = report->xbutton.state;
    break;
  case KeyPress:
    lux_keycode = report->xkey.keycode;
    /* note that keycode can be used to distinquish between press and release */
  case KeyRelease:
    xcoord = report->xkey.x;
    ycoord = report->xkey.y;
    root_x = report->xkey.x_root;
    root_y = report->xkey.y_root;
    lux_keystate = report->xkey.state;
    nc = XLookupString(&(report->xkey), buffer, 15, &keysym, NULL);
    buffer[nc] = '\0';
    /*printf("nc = %d, string = %s\n", nc, buffer);*/
    lux_keysym = (Int) keysym;
    break;
 }
 (void) lux_callback_execute( *ptq);
 }
 /*------------------------------------------------------------------------- */
void draw_re_callback(w, ptq, call_data)
 Widget	w;
 XmDrawingAreaCallbackStruct *call_data;
 Int	*ptq;
 {
 extern	 Int    wd[], ht[], last_wid,  set_defw();
 extern  Float   xfac, yfac;
 extern  Int     ixhigh, iyhigh;
 Dimension	ww, wh;
 static	Int	kilroy;

 /* this callback is particularly likely to cause re-entrant problems in the
 lux code because the lux call backs may do some limit checking and hence
 re-size via lux_xmsize. This causes an immediate callback that gets here
 again. Hence we use a kilroy to ensure that the previous lux callback is
 done. If not, we just return. We could do other stuff as long as the
 lux_execute isn't done.
 */
 last_wid = *(ptq+1);	/* which lux X window the event occurred in */
 /* the new size is not available in the call_data structure, so we
 just call set_defw in xport.c which gets the new size and sets a bunch
 of plot context parameters and last_wid*/
 /* 5/27/96 unfortunately the XGetWindowAttributes in set_defw does not
 always get the latest size, this almost always happens when downsizing,
 a subsequent call does get it right so it seems to be a delay of some
 sort, hence we try to get the widget size here which we hope is up to
 date and duplicate the rest of set_defw here */
 /* set_defw(*(ptq+1)); */
 XtVaGetValues (w,
 XmNwidth,  &ww,
 XmNheight, &wh,
 NULL);
 wd[last_wid] = ww;    xfac = ww;
 ht[last_wid] = wh;    yfac = wh;
 /*printf("resize from drawing area, %d %d, kilroy = %d\n", ww, wh, kilroy);*/
 ixhigh = xfac - 1;  iyhigh = yfac - 1;
 xfac = yfac = MIN( xfac, yfac);
 if (kilroy) return;
 kilroy = 1;
 (void) lux_callback_execute( *ptq);
 kilroy = 0;
 }
 /*------------------------------------------------------------------------- */
void draw_ex_callback(w, ptq, call_data)
 Widget	w;
 XmDrawingAreaCallbackStruct *call_data;
 Int	*ptq;
 {
 extern	Int	last_wid;
 static	Int	kilroy;

 /* 5/28/96 - added, tried to handle expose events in the same callbacks
 that do re-sizing but problems */
 /* this callback is particularly likely to cause re-entrant problems in the
 lux code because the lux call backs may do some limit checking and hence
 re-size via lux_xmsize. This causes an immediate callback that gets here
 again. Hence we use a kilroy to ensure that the previous lux callback is
 done. If not, we just return. We could do other stuff as long as the
 lux_execute isn't done.
 */
 last_wid = *(ptq+1);	/* which lux X window the event occurred in */
 /*printf("expose from drawing area, kilroy = %d\n", kilroy);*/
 if (kilroy) return;
 kilroy = 1;
 (void) lux_callback_execute( *ptq);
 kilroy = 0;
 }
 /*------------------------------------------------------------------------- */
void textfield_which(w, ptq, state)
 Widget	w;
 XmToggleButtonCallbackStruct	*state;
 XtPointer	ptq;
 {
 Int	iq, which;
 iq = find_sym(textfield_number);
 which = (Int) ptq;
 redef_scalar(iq, 2, &which);
 }
 /*------------------------------------------------------------------------- */
void textfield_callback(w, ptq, call_data)
 /* used for XmNactivateCallback,  XmNlosingFocusCallback, and XmNFocusCallback
 but the lux symbol will generally be different for these 3 cases */
 /* 9/14/96 added checking when either entering or leaving text field,
 note that the text contents are copied, might want to re-think this since it
 could also be read at the lux level, but if we always want to load it,
 it is more efficient to do it here */
 /* also note that we don't take advantage of the extra stuff available
 for some callbacks and just use XmAnyCallbackStruct */
 Widget	w;
 XmAnyCallbackStruct *call_data;
 XtPointer	ptq;
 {
 Int	result_string, mq;
 char *text, *p, *s;
 /* get the value in the textfield */
 text = XmTextFieldGetString(w);
 mq = strlen(text);
 result_string = find_sym(textfield_value);
 redef_string(result_string, mq);
 p = (char *) sym[result_string].spec.array.ptr;	s = text;
 while (mq--) *p++ = *s++;	*p = 0;			/*load it */
 XtFree(text);
 (void) lux_callback_execute( (Int) ptq);
 }
 /*------------------------------------------------------------------------- */
void quit_callback(w, client_data, call_data)
 Widget     w;
 caddr_t	client_data, call_data;
 {
 printf("Exit selected.\n");
 motif_flag = 0;	/* this stops xtloop */
 }
/*------------------------------------------------------------------------- */
void motif_command_callback(Widget w, XtPointer ptq,
			    XmCommandCallbackStruct *call_data)
 /* this needs to be re-entrant, the parser call here may cause
 this routine to be called, so be careful */
{
 char	*text;

 switch (call_data->reason) {
   case XmCR_COMMAND_ENTERED:
     XmStringGetLtoR(call_data->value, cset, &text);
     break;
   default:
     printf("unexpected reason = %d\n", call_data->reason);
     return;
 }
 compileString(text);
 /* need to test input_modal_flag again because it could have changed */
 if (input_modal_flag)
   motif_flag = 0;		/* turn off xtloop */
}
/*------------------------------------------------------------------------- */
Int lux_motif_input(narg, ps) /* get our commands via motif widget */
 /* when called, command input is allowed via a command widget,
 somewhat complicated because of multi-line commands, we want to lock out
 other motif callbacks in the middle of same */
 /* jul 9, 1995, modified, no longer makes its own dialog board, hence
 it needs to be put in one (needs a parent), allows more flexibility */
 /* arguments are: parent, width, nvisible, hmax, font, color */
 Int     narg, ps[];
 {
 /* first create a managed bulletin board dialog widget */
 Int	iq, parent, nvisible = 20, hmax = 100, width=600;
 struct	ahead	*h;
 Int	result_sym, nq, *wids;
 XmString	prompt;
 Widget		commandw, wg;
 if (ck_motif() != 1) return -1;
 /* get parent widget */
 if ( get_widget_id( ps[0], &parent) != 1 ) return -1;

 /* command widget */
 nq = 4;
 result_sym = array_scratch(2, 1, &nq);
 h = (struct ahead *) sym[result_sym].spec.array.ptr;
 wids = (Int *) ((char *)h + sizeof(struct ahead));
 
 if (narg > 1) { if (int_arg_stat(ps[1], &width) != 1) return -1; }
 if (narg > 2) { if (int_arg_stat(ps[2], &nvisible) != 1) return -1; }
 if (narg > 3) { if (int_arg_stat(ps[3], &hmax) != 1) return -1; }
 
 n = 0;
 XtSetArg(wargs[n], XmNhistoryVisibleItemCount, nvisible); n++;
 XtSetArg(wargs[n], XmNhistoryMaxItems, hmax); n++;
 XtSetArg(wargs[n], XmNwidth, width);	n++;
 prompt = XmStringCreateSimple( "command line");
 XtSetArg(wargs[n], XmNpromptString, prompt); n++;
 commandw = XmCreateCommand(lux_widget_id[parent], "command",wargs, n);
 if (ck_widget_count() < 0) return -1;
 iq = n_widgets++;	wids[0] = iq;	lux_widget_id[iq] = commandw;
 lux_command_widget = commandw;
 XtAddCallback(commandw, XmNcommandEnteredCallback,
                (XtCallbackProc) motif_command_callback, 0);
 /* now the optional font and color */
 /* these have to set some of the children widgets, not the command widget */
 /* we don't set the color for the prompt */
 n = 0;
 if (narg > 4) {
  if (set_fontlist( ps[4] ) != 1) return -1; /* gets fontlist from string */
 }
 if (narg > 5) {
  if (setup_colors (ps[5] ) != 1) return -1; }	/* do colors based on bg */
 /* if n is still 0, no color or font */
 wg = XmCommandGetChild(commandw, XmDIALOG_COMMAND_TEXT);
 if (ck_widget_count() < 0) return -1;
 iq = n_widgets++;	wids[1] = iq;	lux_widget_id[iq] = wg;

 if (n != 0) { XtSetValues(wg, wargs, n); }
 wg = XmCommandGetChild(commandw, XmDIALOG_HISTORY_LIST);
 if (ck_widget_count() < 0) return -1;
 iq = n_widgets++;	wids[3] = iq;	lux_widget_id[iq] = wg;
 if (n != 0) { XtSetValues(wg, wargs, n); }
 /* the prompt label just gets the font, so re-do the font args */
 n = 0;
 wg = XmCommandGetChild(commandw, XmDIALOG_PROMPT_LABEL);
 if (ck_widget_count() < 0) return -1;
 iq = n_widgets++;	wids[2] = iq;	lux_widget_id[iq] = wg;
 if (narg > 4) {
  if (set_fontlist( ps[4] ) != 1) return -1; /* gets fontlist from string */
  XtSetValues(wg, wargs, n);
 }
 /* now that everybody is ready */

 XtManageChild(commandw);
 XmStringFree(prompt);
 return result_sym;
 }
 /*------------------------------------------------------------------------- */
void wprint(char *fmt, ...)
{
 char	msgbuf[256];	/* should be coordinated with maximum size elsewhere */
 Int	i;
 va_list args;
 
 va_start(args, fmt);
 (void) vsprintf(msgbuf, fmt, args);
 va_end(args);
 i = XmTextGetLastPosition(text_output);
 XmTextInsert(text_output, i, msgbuf);
}
 /*------------------------------------------------------------------------- */
Int lux_test_wprint(narg,ps)
 Int	narg,ps[];
 {
 wprint("a test of wprint\n");
 wprint("now with a value = %d\n", 13);
 return 1;
 }
 /*------------------------------------------------------------------------- */
void destroy_cb(Widget w, XtPointer ptq, void *call_data)
{
  printf("zapping %1d (%s)\n", (Int) ptq, symbolIdent((Int) ptq,0));
  zap((Int) ptq);
}
/*------------------------------------------------------------------------- */
void xminfo(Int w)
{
  Int	minwidth, maxwidth, minheight, maxheight, basewidth, baseheight;
  Dimension	width, height;

  if (w < 0 || w >= n_widgets) {
    printf("Invalid widget number %d; valid range 0 through %d\n",
	   w, n_widgets);
    return;
  }
  n = 0;
  XtSetArg(wargs[n], XmNwidth, &width); n++;
  XtSetArg(wargs[n], XmNheight, &height); n++;
  XtSetArg(wargs[n], XmNminWidth, &minwidth); n++;
  XtSetArg(wargs[n], XmNmaxWidth, &maxwidth); n++;
  XtSetArg(wargs[n], XmNminHeight, &minheight); n++;
  XtSetArg(wargs[n], XmNmaxHeight, &maxheight); n++;
  XtSetArg(wargs[n], XmNbaseWidth, &basewidth); n++;
  XtSetArg(wargs[n], XmNbaseHeight, &baseheight); n++;
  XtGetValues(lux_widget_id[w], wargs, n);
  printf("Widget %d out of: %d\n", w, n_widgets);
  printf("XmNwidth     : %u\n", width);
  printf("XmNheight    : %u\n", height);
  printf("XmNminWidth  : ");
  if (minwidth == XtUnspecifiedShellInt)
    printf("Unspecified\n");
  else
    printf("%d\n", minwidth);
  printf("XmNmaxWidth  : ");
  if (maxwidth == XtUnspecifiedShellInt)
    printf("Unspecified\n");
  else
    printf("%d\n", maxwidth);
  printf("XmNminHeight : ");
  if (minheight == XtUnspecifiedShellInt)
    printf("Unspecified\n");
  else
    printf("%d\n", minheight);
  printf("XmNmaxHeight : ");
  if (maxheight == XtUnspecifiedShellInt)
    printf("Unspecified\n");
  else
    printf("%d\n", maxheight);
  printf("XmNbaseWidth : ");
  if (basewidth == XtUnspecifiedShellInt)
    printf("Unspecified\n");
  else
    printf("%d\n", basewidth);
  printf("XmNbaseHeight: ");
  if (baseheight == XtUnspecifiedShellInt)
    printf("Unspecified\n");
  else
    printf("%d\n", baseheight);
}
/*------------------------------------------------------------------------- */
Int lux_xminfo(Int narg, Int ps[])
{
  Int	w;
  Int	minwidth, maxwidth, minheight, maxheight, basewidth, baseheight;
  Dimension	width, height;

  if (get_widget_id(ps[0], &w) == LUX_ERROR)
    return LUX_ERROR;

  if (w < 0 || w >= n_widgets) {
    printf("Invalid widget number %d; valid range 0 through %d\n",
	   w, n_widgets);
    return;
  }
  n = 0;
  XtSetArg(wargs[n], XmNwidth, &width); n++;
  XtSetArg(wargs[n], XmNheight, &height); n++;
  XtSetArg(wargs[n], XmNminWidth, &minwidth); n++;
  XtSetArg(wargs[n], XmNmaxWidth, &maxwidth); n++;
  XtSetArg(wargs[n], XmNminHeight, &minheight); n++;
  XtSetArg(wargs[n], XmNmaxHeight, &maxheight); n++;
  XtSetArg(wargs[n], XmNbaseWidth, &basewidth); n++;
  XtSetArg(wargs[n], XmNbaseHeight, &baseheight); n++;
  XtGetValues(lux_widget_id[w], wargs, n);
  printf("Widget %d out of: %d\n", w, n_widgets);
  printf("XmNwidth     : %u\n", width);
  printf("XmNheight    : %u\n", height);
  printf("XmNminWidth  : ");
  if (minwidth == XtUnspecifiedShellInt)
    printf("Unspecified\n");
  else
    printf("%d\n", minwidth);
  printf("XmNmaxWidth  : ");
  if (maxwidth == XtUnspecifiedShellInt)
    printf("Unspecified\n");
  else
    printf("%d\n", maxwidth);
  printf("XmNminHeight : ");
  if (minheight == XtUnspecifiedShellInt)
    printf("Unspecified\n");
  else
    printf("%d\n", minheight);
  printf("XmNmaxHeight : ");
  if (maxheight == XtUnspecifiedShellInt)
    printf("Unspecified\n");
  else
    printf("%d\n", maxheight);
  printf("XmNbaseWidth : ");
  if (basewidth == XtUnspecifiedShellInt)
    printf("Unspecified\n");
  else
    printf("%d\n", basewidth);
  printf("XmNbaseHeight: ");
  if (baseheight == XtUnspecifiedShellInt)
    printf("Unspecified\n");
  else
    printf("%d\n", baseheight);
  return LUX_OK;
}
/*------------------------------------------------------------------------- */
