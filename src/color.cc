/* This is file color.cc.

Copyright 2013-2014 Louis Strous

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
/* This file contains routines dealing with X11 color maps and visuals. */
/* Started 12 March 1999 by Louis Strous */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdlib.h>
#include <limits.h>
#include <string.h>		/* for memcpy */
#include <math.h>
#include "action.hh"
extern "C" {
#include "visualclass.h"
}
#include <X11/Xlib.h>
#include <X11/Xutil.h>		/* for XVisualInfo */

Display		*display;
int32_t		screen_num, connect_flag = 0, private_colormap = 0,
  threeColors = 0, foreground_pixel, colormin, colormax, nColors,
  select_visual = 0, bits_per_rgb, bits_per_pixel, colorIndexType,
  visualClass;
uint32_t	display_cells, depth, display_width, display_height,
  nColorCells, colorIndex;
unsigned long	*pixels, black_pixel, white_pixel, red_mask, green_mask,
  blue_mask, red_mask_bits, green_mask_bits, blue_mask_bits,
  red_mask_lower, green_mask_lower, blue_mask_lower;
Colormap	colorMap;
Visual		*visual;
XColor		*colors;
GC	gcnot;
Atom	wm_delete;

int32_t	xerr(Display *, XErrorEvent *), selectVisual(void);
XColor	*anaFindBestRGB(XColor *color, int32_t mode);
Status	anaAllocNamedColor(char const*, XColor **);

#define FBRGB_RAMP		1 /* color ramp */
#define FBRGB_INCIDENTAL	2 /* incidental colors */

/*
  Global variables:

  <display>: the current display
  <screen_num>:  the current screen number
  <connect_flag>: set to 1 if a connection to the X server is open, 0 if not
  <visual>:  the current visual
  <display_cells>: the maximum number of distinct colors in the current
    visual
  <depth>: the maximum number of color planes in the current visual
  <private_colormap>: set to 1 if we have a private colormap in the
    PseudoColor visual, 0 otherwise
  <display_width>: the width (in pixels) of the display
  <display_height>: the height (in pixels) of the display
  <colorMap>: the current color map
  <colors>:   the colors allocated by LUX
  <nColorCells>: the total number of color cells currently allocated by LUX
  <nColors>:  the number of color cells assigned to LUX's color ramp
  <pixels>:   the pixel values corresponding to the allocated colors
  <black_pixel>: the pixel value corresponding to black
  <white_pixel>: the pixel value corresponding to white
  <fg_pixel>: the pixel value corresponding to the foreground color
  <bg_pixel>: the pixel value corresponding to the background color */

/*
  Color model:

  For PseudoColor visuals: We distinguish between the "color ramp",
  used for display of data values, and "incidental colors", used for
  such things as foregrounds, backgrounds and borders.  Any color
  requested by name is considered to be an incidental color.

  We cannot grab all available color cells and set some apart for
  incidental colors, because widgets may request color cells outside
  of our own channels, and they might then not be satisfied.

  Our desires are as follows: to allocate at least 64 color cells for
  LUX's color ramp, and to have at least 32 cells available for
  incidental colors and requests by applications other than LUX.  This
  requires that at least 96 color cells be currently available (i.e.,
  not yet allocated by other applications).  If at least 96 color
  cells are available in the default colormap, then LUX allocates all
  but 32 of the available colorcells in that map.  Otherwise, LUX
  creates a private colormap and allocates all of its color cells.

  The selected colormap is available through global variable
  <colorMap>.  If the colormap is a private one, then
  <private_colormap> is set to 1, otherwise to 0.  The current visual
  is available through global variable <visual>.  The total number of
  color cells in the current colormap is available in <display_cells>.
  The number of color cells allocated by LUX at any given time is
  equal to <nColorCells>.  This number increases when incidental
  colors are allocated.

  <nColors> color cells are assigned to the color ramp.  These colors
  are initially filled with a linear greyscale ramp.  All colorcells
  are read-write, which means they can be changed on the fly, and any
  changes will have immediate impact on any graphical material already
  shown by LUX.

  Full color information is available through <colors>, which is an
  array of XColor structures.  Associated with each color is an index
  (referred to as the "color index") into the <colors> array, a pixel
  value which represents the color cell internally in X11, and
  intensity values for red, green, and blue (unsigned ints between 0
  and 65535, inclusive, for dark and bright, respectively).

  The pixel values of the fixed colors black and white are available
  as <black_pixel> and <white_pixel>, respectively.  If we're using
  the default colormap, then the associated color cells are not
  identified.  If we're using a private colormap, then two of the
  incidental color cells are reserved for them.

 */

/* The visual class macros are, from 0 - 5: StaticGray, GrayScale,
   StaticColor, PseudoColor, TrueColor, DirectColor */

char const* visualNames[] = { "StaticGray", "GrayScale", "StaticColor",
			   "PseudoColor", "TrueColor", "DirectColor" };


int32_t setup_x_visual(int32_t desiredVisualClass)
/* tries to open a connection to the X server and initializes a grey ramp */
/* colormap.  Returns LUX_OK and sets connect_flag to 1 on success; */
/* returns LUX_ERROR and sets connect_flag to 0 on failure.  If connect_flag */
/* is equal to 1 on entry, then a connection is already established and the */
/* routine returns LUX_OK immediately.  Sets some of the globals. */
/* LS 12mar99 */
{
  extern int32_t	scalemin, scalemax;
  Window	win;
  int32_t	i, j;
  XColor	*tempColor, rgb;
  unsigned long	n;
  extern char	*display_name;
  XVisualInfo	matchedVisual;

  if (connect_flag)		/* already did this earlier */
    return LUX_OK;
  
  /* 1. open the display */
  display = XOpenDisplay(display_name);
  if (!display)
    return luxerror("Could not connect to X display \"%s\"", 0, display_name);

  /* 2. set error handler */
  XSetErrorHandler(xerr);

  connect_flag = 1;		/* flag connection to X server */

  /* 3. get defaults */
  screen_num     = DefaultScreen(display);
  display_cells  = DisplayCells(display, screen_num);
  display_width  = DisplayWidth(display, screen_num); 
  display_height = DisplayHeight(display, screen_num);
  visual         = DefaultVisual(display, screen_num);

  colorMap       = DefaultColormap(display, screen_num);
  depth          = DisplayPlanes(display, screen_num);

  /* 4. select a visual */
  if (select_visual)
    selectVisual();
  else if (desiredVisualClass >= 0) {
    if (!XMatchVisualInfo(display, screen_num, depth, desiredVisualClass,
			  &matchedVisual))
      return luxerror("No %s visual at depth %d is available on this screen.",
		   0, visualNames[desiredVisualClass], depth);
    visual = matchedVisual.visual;
    depth = matchedVisual.depth;
    display_cells = matchedVisual.colormap_size;
    private_colormap = !(visual == DefaultVisual(display, screen_num));
    printf("Using %d-bit %s visual\n", depth, visualNames[visualclass(visual)]);
  } /* end of if (select_visual) else */

  bits_per_rgb = visual->bits_per_rgb;

  if (visualclass(visual) == DirectColor ||
      visualclass(visual) == TrueColor) {
    red_mask_bits = 0;
    red_mask_lower = -1;
    n = visual->red_mask;
    i = 0;
    do {
      if (red_mask_lower == -1 && (n & 1))
	red_mask_lower = i;
      red_mask_bits += (n & 1);
      n = n >> 1;
      i++;
    } while (n);
    green_mask_bits = 0;
    green_mask_lower = -1;
    n = visual->green_mask;
    i = 0;
    do {
      if (green_mask_lower == -1 && (n & 1))
	green_mask_lower = i;
      green_mask_bits += (n & 1);
      n = n >> 1;
      i++;
    } while (n);
    blue_mask_bits = 0;
    blue_mask_lower = -1;
    n = visual->blue_mask;
    i = 0;
    do {
      if (blue_mask_lower == -1 && (n & 1))
	blue_mask_lower = i;
      blue_mask_bits += (n & 1);
      n = n >> 1;
      i++;
    } while (n);
  } /* end of if (visual->class == DirectColor
       || visual->class == TrueColor) */

  /* we may need more than 8 bits to specify a color index */
  if (depth <= 8*sizeof(uint8_t))
    colorIndexType = LUX_INT8;
  else if (depth <= 8*sizeof(int32_t))
    colorIndexType = LUX_INT16;
  else
    colorIndexType = LUX_INT32;

  /* 5. get a colormap for the selected visual */
  /* for read-write colormaps, we first try to find enough color cells in */
  /* the default colormap.  We look for at least 96 color cells */
  /* First we figure out how many colorcells we have by trying to allocate */
  /* as many as possible.  If the number found is at least 96, then we */
  /* try to free some up for incidental colors and other applications. */
  /* If the number found is less than 96, then we go for a private */
  /* colormap and allocate 224 cells, leaving 32 for incidentals. */

  /* allocate as many as we may need */

  pixels = (long unsigned int*) malloc(display_cells*sizeof(unsigned long));
  if (!pixels) {
    XCloseDisplay(display);
    connect_flag = 0;
    return luxerror("Could not allocate memory for pixels in setup_x_visual()", 0);
  } /* end of if (!pixels) */

  switch (visualclass(visual)) {
    case PseudoColor: case StaticGray: case GrayScale: case StaticColor:
      if (display_cells <= 0x100)
	bits_per_pixel = 8;
      else if (display_cells <= 0x10000)
	bits_per_pixel = 16;
      else
	bits_per_pixel = 32;
      break;
    case DirectColor: case TrueColor:
      red_mask = visual->red_mask;
      green_mask = visual->green_mask;
      blue_mask = visual->blue_mask;
      bits_per_pixel = 0;
      n = red_mask | green_mask | blue_mask;
      while (n & 1) {
	bits_per_pixel++;
	n = n >> 1;
      }	/* end of while (n & 1) */
      if (bits_per_pixel <= 8)
	bits_per_pixel = 8;
      else if (bits_per_pixel <= 16)
	bits_per_pixel = 16;
      else
	bits_per_pixel = 32;
      break;
  } /* end of switch (visualclass(visual)) */

  if (visualIsRW(visualclass(visual))) {
    if (!private_colormap) {
      for (nColorCells = display_cells; nColorCells > 95; nColorCells--)
	if (XAllocColorCells(display, colorMap, False, NULL, 0, pixels,
			     nColorCells))
	  break;
      if (nColorCells < 96)
	private_colormap = 1;
    } /* end of if (!private_colormap) */
    if (private_colormap) {
      puts("Using private colormap.");
      colorMap = XCreateColormap(display, RootWindow(display, screen_num),
				 visual, AllocNone);
      /* On some systems, there seem to be fewer DirectColor color cells
	 available than visual->map_entries.  We better check how many
	 we can really get */
      for ( ; display_cells; display_cells--)
	if (XAllocColorCells(display, colorMap, False, NULL, 0, pixels,
			     display_cells)) {
	  printf("Got %d color cells.\n", display_cells);
	  break;
	}
      if (!display_cells) {
	printf("Could not allocate any color cells for the %s visual.\nUse a different one.\n", visualNames[visualclass(visual)]);
	XFreeColormap(display, colorMap);
	XCloseDisplay(display);
	free(pixels);
	return LUX_ERROR;
      }
      /* we may not want to get them all */
      XFreeColors(display, colorMap, pixels, display_cells, 0);

      /* for DirectColor and TrueColor we can construct incidental colors
	 reasonably well on the fly from the existing map, so we don't
	 leave any cells for those, particularly because the number of cells
	 can be very small, e.g. only 8 on a modest PC. */
      /* for the other visual classes, we leave some for incidental colors */
      if (visualPrimariesAreSeparate(visualclass(visual)))
	j = 0;
      else {
	j = display_cells/3;
	if (j < 2)
	  j = 2;
	else if (j > 32)
	  j = 32;
      }
      nColorCells = display_cells - j;
      if (!XAllocColorCells(display, colorMap, False, NULL, 0, pixels,
			    nColorCells)) {
	XCloseDisplay(display);
	connect_flag = 0;
	return
	  luxerror("Could not allocate %d cells in private colormap for %s visual.", 0, nColorCells, visualNames[visualclass(visual)]);
      }	/* end of if (!XAllocColorCells(...)) */
    } /* end of if (private_colormap) */
    else {
      /* we grabbed all available color cells and found that there */
      /* were more than 96 of them.  We free 32 of them so they */
      /* are available for incidental colors and other applications */
      nColorCells -= 32;
      XFreeColors(display, colorMap, pixels + nColorCells, 32, 0);
    } /* end of if (private_colormap) else */
    
    /* we now have <nColorCells> color cells from either the default
       or the private colormap.  We allocate an XColor entry for
       all color cells in the colormap, because we may allocate more
       colors later. */
    colors = (XColor*) malloc(display_cells*sizeof(XColor));
    if (!colors) {
      XCloseDisplay(display);
      connect_flag = 0;
      return
	luxerror("Could not allocate memory for XColor entries in setup_x()",
	      0);
    } /* end of (!colors) */
    
    nColors = nColorCells;	/* currently all of them are for the ramp */
    colormin = 0;
    colormax = nColors - 1;
    
    /* we fill the ramp colors with a default linear greyscale ramp */
    for (i = 0; i < nColors; i++) {
      colors[i].pixel = pixels[i];
      colors[i].red = colors[i].green = colors[i].blue =
	(i*0xffff)/(nColors - 1);
      colors[i].flags = DoRed | DoGreen | DoBlue;
    } /* end of for (i) */
    /* indicate that we are not using the remaining cells for LUX (yet) */
    for (i = nColors; i < display_cells; i++)
      colors[i].flags = 0;
    
    XStoreColors(display, colorMap, colors, nColorCells);
    /* Not all bits of the RGB values may have been significant for */
    /* specifying the colors; we want to know the exact RGB values */
    /* that were installed, so we call XQueryColors() */
    XQueryColors(display, colorMap, colors, nColorCells);

    /* now we look for black and white */
    /* NOTE: if we're dealing with the default colormap and we */
    /* take BlackPixel(display, screen_num) for the back pixel and */
    /* WhitePixel(display, screen_num) for the white pixel, then */
    /* we are unable to allocate incidental colors afterwards - */
    /* at least on our mips-sgi-irix6.3 system.  We just explicitly */
    /* allocate black and white, too. */
    
    /* get black */
    if (anaAllocNamedColor("black", &tempColor))
      black_pixel = tempColor->pixel;
    else
      black_pixel = colors[0].pixel; /* just pick one */
    if (anaAllocNamedColor("white", &tempColor))
      white_pixel = tempColor->pixel;
    else
      white_pixel = colors[1].pixel; /* just pick one */
    
    free(pixels);		/* don't need it anymore */
    if (private_colormap)
      XInstallColormap(display, colorMap);
    
    /* 6. set up a lookup table of fixed size into the colormap */
    /* our display routines are most portable if they do not need to worry */
    /* about the exact number of colorcells we got, so we use a lookup */
    /* table from a fixed range of LUX pixel values (we take display_cells) */
    /* into the underlying X11 pixel values */
    pixels = (unsigned long *) malloc(display_cells*sizeof(unsigned long));
    if (!pixels) {
      XCloseDisplay(display);
      connect_flag = 0;
      return luxerror("Could not allocate memory for pixels in setup_x()", 0);
    } /* end of if (!pixels) */
    for (i = 0; i < display_cells; i++)
      pixels[i] = colors[(i*(nColors - 1))/(display_cells - 1)].pixel;
  } /* end of if (visualIsRW(visualclass(visual))) */
  else {			/* read-only visuals */
    if (private_colormap)
      colorMap = XCreateColormap(display, RootWindow(display, screen_num),
				 visual, AllocNone);
    nColorCells = display_cells;
 
    /* 5. get black and white */
    anaAllocNamedColor("black", &tempColor);
    black_pixel = tempColor->pixel;
    anaAllocNamedColor("white", &tempColor);
    white_pixel = tempColor->pixel;

    /* 6. setup lookup table for color ramp */
    pixels = (unsigned long*) malloc(display_cells*sizeof(unsigned long));
    if (!pixels) {
      XCloseDisplay(display);
      connect_flag = 0;
      return luxerror("Could not allocate memory for pixels in setup_x()", 0);
    } /* end of if (!pixels) */
    rgb.flags = DoRed | DoGreen | DoBlue;
    for (i = 0; i < display_cells; i++) {
      rgb.red = rgb.green = rgb.blue = (i*0xffff)/(display_cells - 1);
      XAllocColor(display, colorMap, &rgb);
      pixels[i] = rgb.pixel;
    } /* end of for (i) */
    nColors = display_cells;
    colormin = 0;
    colormax = nColors - 1;

    colors = (XColor*) malloc(display_cells*sizeof(XColor));
    if (!colors) {
      XCloseDisplay(display);
      connect_flag = 0;
      return
	luxerror("Could not allocate memory for XColor entries in setup_x()",
	      0);
    } /* end of if (!colors) */
    for (i = 0; i < nColorCells; ++i) {
      XColor thisone = colors[i];
      thisone.pixel = pixels[i];
      XQueryColor(display, colorMap, &thisone);
      colors[i] = thisone;
    }
  } /* end of if (visualIsRW(visualclass(visual))) else */

  /* 7. set various defaults */
  scalemin = 0;
  scalemax = display_cells - 1;

  /* make a GC for drawing "not" lines in lux_xnotdraw */
  /* we must create a window with the selected visual first, so the GC */
  /* applies to those visuals, too */

 {
   unsigned long	valuemask;
   XSetWindowAttributes	att;

   att.background_pixel = black_pixel;
   att.border_pixel = black_pixel;
   att.colormap = colorMap;
   valuemask = CWBackPixel | CWColormap | CWBorderPixel;

   win = XCreateWindow(display, RootWindow(display, screen_num), 0, 0,
		       100, 100, 1, depth, InputOutput, visual,
		       valuemask, &att);
   gcnot = XCreateGC(display, win,0, NULL);
   XDestroyWindow(display, win); /* no longer needed */
 }
  XSetFunction(display, gcnot, GXinvert);

  wm_delete = XInternAtom(display, "WM_DELETE_WINDOW", False);
  
  return LUX_OK;
}
/*-------------------------------------------------------------------------*/
int32_t setup_x(void)
{
  return setup_x_visual(-1);
}
/*-------------------------------------------------------------------------*/
void disconnect_x(void)
/* disconnects gracefully from the X server and frees any lingering */
/* associated allocated memory.  Resets global connect_flag to 0. */
/* LS 12mar99 */
{
  if (!connect_flag)
    return;

  XCloseDisplay(display);
  connect_flag = 0;

  free(pixels);
  free(colors);
}
/*-------------------------------------------------------------------------*/
int32_t selectVisual(void)
/* allow the user to select a visual */
{
  XVisualInfo	*vInfo, vTemplate;
  int32_t	nVisual, i, mask, j;
  int32_t	getNewLine(char *, size_t, char const *, char);
  uint32_t	r, x;
  char const* name;

  mask = VisualScreenMask;
  vTemplate.screen = screen_num;

  vInfo = XGetVisualInfo(display, mask, &vTemplate, &nVisual);
  if (!vInfo)
    nVisual = 0;

  if (!nVisual)
    return luxerror("No color representation is available", 0);

  printf("There are %1d visuals available on this screen\n", nVisual);

  printf("%2s %11s %3s %4s %1s %1s %1s\n", "#", "class", "bit",
	 "lev", "r", "g", "b");
  j = -1;
  for (i = 0; i < nVisual; i++) {
    name = visualNames[xvisualinfoclass(vInfo[i])];

    printf("%2d %11s %3d %4d", i + 1, name, vInfo[i].depth,
	   vInfo[i].colormap_size);
    if (visualPrimariesAreSeparate(xvisualinfoclass(vInfo[i]))) {
      r = 0;
      x = vInfo[i].red_mask;
      do {
	r += (x & 1);
	x = x >> 1;
      } while (x);
      printf(" %1d", r);
      r = 0;
      x = vInfo[i].green_mask;
      do {
	r += (x & 1);
	x = x >> 1;
      } while (x);
      printf(" %1d", r);
      r = 0;
      x = vInfo[i].blue_mask;
      do {
	r += (x & 1);
	x = x >> 1;
      } while (x);
      printf(" %1d\n", r);
    } else
      putchar('\n');
    if (vInfo[i].visual == visual)	/* found current visual */
      j = i;
  }
  printf("current visual: %d\n", j + 1);

  printf("Your selection:\n");
  do {
    getNewLine(curScrat, NSCRAT, "  ?>", 0);
    visualClass = atol(curScrat);
    if (visualClass < 1 || visualClass > nVisual)
      printf("Selection %1d is invalid.  Select between 1 and %1d (inclusive).\n",
	     visualClass, nVisual);
  } while (visualClass < 1 || visualClass > nVisual);
  visualClass--;		/* visual class */
  visual = vInfo[visualClass].visual;
  depth = vInfo[visualClass].depth;
  display_cells = vInfo[visualClass].colormap_size;
  screen_num = vInfo[visualClass].screen; /* should not be necessary */
  /* if we're not using the default visual, then we'll certainly have */
  /* to create our own colormap */
  private_colormap = !(visual == DefaultVisual(display, screen_num));

  XFree(vInfo);
  return LUX_OK;
}
/*-------------------------------------------------------------------------*/
int32_t nextFreeColorsIndex(void)
/* returns an index to a free colors[] element. */
/* NOTE: this routine assumes there is such an element! */
{
  static int32_t	index = 0;

  while (colors[index].flags) {
    index++;
    if (index == display_cells)
      index = nColors;
  }
  return index;
}
/*-------------------------------------------------------------------------*/
void installPixel(int32_t pixel)
/* installs the indicated pixel value as one incidental color in the */
/* colors[] database if it does not already exist there. */
{
  int32_t	i;

  for (i = nColors; i < nColorCells; i++)
    if (colors[i].pixel == pixel)
      return;
  i = nextFreeColorsIndex();
  colors[i].pixel = pixel;
  colors[i].flags = DoRed | DoGreen | DoBlue;
  XQueryColor(display, colorMap, &colors[i]);
  nColorCells++;
}
/*-------------------------------------------------------------------------*/
Status anaAllocNamedColor(char const* colorName, XColor **return_color)
/* Returns the closest approximation to the specified <colorName>. */
/* If we're using a linked-primaries read-write visual (i.e., GrayScale
   or PseudoColor), then we have a separate set of incidental colors.
   If the requested color already exists among the incidental colors,
   then we return the matching incidental color.  If it doesn't yet
   exist, then we try to allocate a new color cell for it.  If this
   is successful, then we return the new color cell (with the requested
   color in it).  If not, then we return the closest match among the
   existing incidental colors.  If the color name does not match anything
   in the color database, then a message is printed and some arbitrary
   color is returned.

   If we're using a read-only or a separate-primaries visual (i.e.,
   TrueColor, DirectColor, StaticGray, or StaticColor), then we return
   the closest match to the requested color.

   A pointer to the assigned color is returned in <return_color>.
   Returns 0 if some fatal error occurs.
   LS 12mar99 - 17mar99 4oct99 */
{
  unsigned long	pixel;
  XColor	color, color2, *bestcolor;
  static XColor	rcolor;
  int32_t	index;

  if (visualclass(visual) == GrayScale || visualclass(visual) == PseudoColor) {
    /* figure out which RGB values to store */
    if (!XLookupColor(display, colorMap, colorName, &color2, &color)) {
      printf("Could not resolve color \"%s\"\nSubstituting another",
	     colorName);
      *return_color = &colors[nColorCells - 1]; /* take the last one */
      foreground_pixel = (*return_color)->pixel;
      return 1;
    } /* end of if (!XLookupColor(...)) */
    /* have rgb values */
    bestcolor = anaFindBestRGB(&color, FBRGB_INCIDENTAL);
    if (bestcolor->pad) {	/* an exact match */
      *return_color = bestcolor;
      foreground_pixel = bestcolor->pixel;
      return 1;
    } /* end of if (bestcolor->pad) */
    /* need to allocate a new cell */
    if (nColorCells < display_cells) { /* may have more cells */
      if (XAllocColorCells(display, colorMap, False, NULL, 0, &pixel, 1)) {
	/* found a R/W cell; now find an available XColor cell */
	index = nextFreeColorsIndex();
	/* store the RGB and pixel values */
	colors[index].red = color.red;
	colors[index].green = color.green;
	colors[index].blue = color.blue;
	colors[index].flags = DoRed | DoGreen | DoBlue;
	colors[index].pixel = pixel;
	nColorCells++;
	*return_color = &colors[index];
	XStoreColor(display, colorMap, colors + index);
	XQueryColor(display, colorMap, colors + index);
	foreground_pixel = pixel;
	return 1;
      }	/* end of if (XAllocColorCells(...)) */
    } /* end of if (nColorCells < display_cells) */
    /* if we get here then there are no more R/W cells available */
    printf("Cannot allocate color \"%s\"\n", colorName);
    puts("Using closest available incidental color instead");
    *return_color = bestcolor;
    foreground_pixel = bestcolor->pixel;
  } /* end of if (visualclass(visual) == GrayScale
       || visualclass(visual) == PseudoColor) */
  else if (visualIsRO(visualclass(visual))) {
    if (!XAllocNamedColor(display, colorMap, colorName, &rcolor, &color)) {
      printf("Could not resolve color \"%s\"\nSubstituting another",
	     colorName);
      XAllocColor(display, colorMap, &rcolor);
    }
    foreground_pixel = rcolor.pixel;
    *return_color = &rcolor;
  } /* end of if (visualclass(visual) == GrayScale
     || visualclass(visual) == PseudoColor) else if (visualIsRO(visualclass(visual)) */
  else {			/* DirectColor: find closest match */
    if (!XLookupColor(display, colorMap, colorName, &color2, &color)) {
      printf("Could not resolve color \"%s\"\nSubstituting another",
	     colorName);
      *return_color = &colors[nColorCells - 1]; /* take the last one */
      foreground_pixel = (*return_color)->pixel;
      return 1;
    } /* end of if (!XLookupColor(...)) */
    *return_color = anaFindBestRGB(&color, FBRGB_RAMP);
    foreground_pixel = (*return_color)->pixel;
  } /* end of if (visualclass(visual) == GrayScale
     || visualclass(visual) == PseudoColor) else if (visualIsRO(visualclass(visual))
     else */
  return 1;
}
/*-------------------------------------------------------------------------*/
XColor *anaFindBestRGB(XColor *color, int32_t mode)
/* finds the color in the current colormap that has RGB values closest to */
/* those in <color>, and returns a pointer to the associated color. */
/* <mode> is the logical sum of: */
/* FBRGB_RAMP -> check LUX's color ramp */
/* FBRGB_INCIDENTAL -> check LUX's incidental colors */
/* LS 12mar99 */
{
  uint32_t	i, best, i1, i2;
  float	mindist, dist, temp;	/* use float because uint32_t is not */
				/* big enough in all cases */

  if (visualPrimariesAreSeparate(visualclass(visual))) {
    XAllocColor(display, colorMap, color);
    return color;
  } /* end if (visualPrimariesAreSeparate(visualclass(visual))) */
  else {
    i1 = (mode & FBRGB_RAMP)? 0: nColors;
    i2 = (mode & FBRGB_INCIDENTAL)? nColorCells: nColors;
    if (i1 == i2) {
      /* no colorcells to check yet */
      /* we return the last of the color ramp cells, but set the .pad member */
      /* to zero to indicate we did not find a perfect match */
      best = nColors - 1;
      colors[best].pad = 0;
      return &colors[best];
    }
    temp = colors[i1].red - color->red;
    mindist = temp*temp;
    temp = colors[i1].green - color->green;
    mindist += temp*temp;
    temp = colors[i1].blue - color->blue;
    mindist += temp*temp;
    best = i1;
    for (i = i1 + 1; i < i2; i++) {
      temp = colors[i].red - color->red;
      dist = temp*temp;
      if (dist > mindist)
	continue;
      temp = colors[i].green - color->green;
      dist += temp*temp;
      if (dist > mindist)
	continue;
      temp = colors[i].blue - color->blue;
      dist += temp*temp;
      if (dist < mindist) {
	mindist = dist;
	best = i;
      }	/* end of if (dist < mindist) */
    } /* end of for (i1) */
    colors[best].pad = (mindist == 0)? 1: 0;	/* flag 1 if exact match */
    return &colors[best];
  } /* end of if (visualPrimariesAreSeparate(visualclass(visual))) else */
}
/*-------------------------------------------------------------------------*/
void storeColorTable(float *red, float *green, float *blue, int32_t nelem,
		     int32_t stretch)
/* stores a new color table in LUX's color map. */
/* <red>: points at an array of red color intensities between 0 and 1 */
/* <green>: points at an array of green color intensities between 0 and 1 */
/* <blue>: points at an array of blue color intensities between 0 and 1 */
/* <nelem>: the number of values in each array */
/* <stretch>: if non-zero, then the color array indices are stretched so */
/*   they cover <nColors> color map entries; otherwise the smaller of */
/*   <nelem> and <nColors> entries in the color map will be changed.  */
/* If any color value is not within the range 0 - 1, then wraparound occurs */
/* LS 12mar99 */
{
  uint32_t	i, j, k1, k2, n;
  XColor	rgb;

  if (stretch) {
    k1 = nColors - 1;
    k2 = nelem - 1;
    n = nColors;
  } else {
    n = MIN(nColors, nelem);
    k1 = k2 = 1;
  }
  if (visualIsRW(visualclass(visual))) {
    if (visualIsGray(visualclass(visual)))
      for (i = 0; i < n; i++) {
	j = (i*k2)/k1;
	colors[i].red = colors[i].green = colors[i].blue =
	  ((int32_t) floor((red[j] + green[j] + blue[j])/3.0*0xffff)) & 0xffff;
      }
    else
      for (i = 0; i < n; i++) {
	j = (i*k2)/k1;
	colors[i].red = ((int32_t) floor(red[j]*0xffff)) & 0xffff;
	colors[i].blue = ((int32_t) floor(blue[j]*0xffff)) & 0xffff;
	colors[i].green = ((int32_t) floor(green[j]*0xffff)) & 0xffff;
      }
    XStoreColors(display, colorMap, colors, n);
    XFlush(display);
  } else {
    rgb.flags = DoRed | DoGreen | DoBlue;
    if (visualIsGray(visualclass(visual)))
      for (i = 0; i < display_cells; i++) {
	j = (i*k2)/k1;
	rgb.red = rgb.green = rgb.blue =
	  ((int32_t) floor((red[j] + green[j] + blue[j])/3.0*0xffff)) & 0xffff;
	XAllocColor(display, colorMap, &rgb);
	pixels[i] = rgb.pixel;
      }
    else
      for (i = 0; i < display_cells; i++) {
	j = (i*k2)/k1;
	rgb.red = ((int32_t) floor(red[j]*0xffff)) & 0xffff;
	rgb.blue = ((int32_t) floor(blue[j]*0xffff)) & 0xffff;
	rgb.green = ((int32_t) floor(green[j]*0xffff)) & 0xffff;
	XAllocColor(display, colorMap, &rgb);
	pixels[i] = rgb.pixel;
      }
  }
}
/*-------------------------------------------------------------------------*/
int32_t	ck_window(int32_t);
int32_t getXcolor(char *colorname, XColor *color, int32_t alloc)
/* looks for the named color in the current colormap.  Returns the found */
/* color in <color>, which must be predefined.  If <alloc> is non-zero, then */
/* the color is placed in the current colormap.   Currently, no checks are */
/* made to see if the new color duplicates already existing ones. */
/* returns LUX_OK on success; LUX_ERROR on failure. */
/* LS 12mar99 */
{
  XColor	color2, *pcolor;
  extern int32_t	last_wid;
  extern GC	gc[], gcmap[];
  int32_t	lux_xcreat(int32_t, uint32_t, uint32_t, int32_t, int32_t, int32_t, char *,
		   char *);
  extern Window	win[];
  extern Pixmap	maps[];

  if (alloc) {
    if (anaAllocNamedColor(colorname, &pcolor) == LUX_ERROR)
      return LUX_ERROR;
    *color = *pcolor;
    if (ck_window(last_wid) == LUX_ERROR)
      return LUX_ERROR;
    if (last_wid < 0) {
      if (!maps[-last_wid]
	  && lux_xcreat(last_wid, 512, 512, 0, 0, 0, NULL, NULL) == LUX_ERROR)
	return LUX_ERROR;
      XSetForeground(display, gcmap[-last_wid], color->pixel);
    } else {
      if (!win[last_wid]
	  && lux_xcreat(last_wid, 512, 512, 0, 0, 0, NULL, NULL) == LUX_ERROR)
	return LUX_ERROR;
      XSetForeground(display, gc[last_wid], color->pixel);
    }
    return LUX_OK;
  } else
    return XLookupColor(display, colorMap, colorname, &color2, color);
}
/*-------------------------------------------------------------------------*/
int32_t threecolors(float *list, int32_t n)
/* If <n> is 0 or if <n> is 1 and <*list> is 0, then a standard
   greyscale table is installed; otherwise if <n> is 1 or 9 then a
   three-color color table is installed.  If <n> is 9, then the numbers
   are associated in groups of three with three sections in the color
   table.  The first number in each group of three governs red, the
   second one green, and the third one blue.  The numbers are restricted
   to the range of -1 to +1 (inclusive).  A positive number means that
   the intensity range from 0 through that number is included; a negative
   number means the intensity range from 1 + the number through 1 is
   included; a zero means the color is not included.  If <n> is 1, then
   <*list> is translated to a full nine-element list as follows:
   [ 1.0, 1.0, 1.0, -*list, 0.0, 0.0, 0.0, 0.0, -*list].  This yields
   a table with a grey section, a red section, and a blue section.
   LS 12nov98 */
{
  float	factor, tlist[9], off[3], fac[3];
  int32_t	i, j;

  switch (n) {
    case 0:			/* uninstall */
      break;
    case 1:
      if (!*list) {
	n = 0;
      } else {
	factor = *list;
	if (factor < -1)
	  factor = -1;
	else if (factor > 1)
	  factor = 1;
	tlist[0] = tlist[1] = tlist[2] = 1.0; /* grey */
	tlist[3] = tlist[8] = -factor;
	tlist[4] = tlist[7] = 0.0;
	tlist[5] = tlist[6] = 0.0;
	n = 9;
      }
      break;
    case 9:
      for (i = 0; i < 9; i++)
	if (list[i] < -1)
	  tlist[i] = -1;
	else if (list[i] > 1)
	  tlist[i] = 1;
	else
	  tlist[i] = list[i];
      break;
    default:
      return luxerror("Wrong # arguments to threecolors()", 0);
  }

  if (!n) {			/* uninstall */
    if (!threeColors)		/* nothing to do */
      return LUX_OK;
    factor = 65535.0/nColors;
    for (i = 0; i < nColors; i++)
      colors[i].red = colors[i].blue = colors[i].green = i*factor;
    for (i = 0; i < 256; i++)
      pixels[i] = colors[(i*nColors)/256].pixel;
    threeColors = 0;		/* standard grey colormap */
  } else {
    /* we install three color domains.  In colors[], each is nColors/3
       color cells.  In pixels[], each is 256/3 = 85 elements wide.
       We must ensure that the pixels[] domains correspond to the colors[]
       domains. */
    threeColors = 1;
    /* first we fix colors[] */
    factor = 65535.0/(nColors/3);
    /* first domain */
    for (j = 0; j < 3; j++)
      if (tlist[j] >= 0) {
	off[j] = 0.0;
	fac[j] = tlist[j]*factor;
      } else {
	off[j] = (1 + tlist[j])*65535.0;
	fac[j] = -tlist[j]*factor;
      }
    for (i = 0; i < nColors/3; i++) {
      colors[i].red = i*fac[0] + off[0];
      colors[i].green = i*fac[1] + off[1];
      colors[i].blue = i*fac[2] + off[2];
    }
    /* second domain */
    for (j = 0; j < 3; j++)
      if (tlist[j + 3] >= 0) {
	fac[j] = tlist[j + 3]*factor;
	off[j] = 0.0 - i*fac[j];
      } else {
	fac[j] = -tlist[j + 3]*factor;
	off[j] = (1 + tlist[j + 3])*65535.0 - i*fac[j];
      }
    for ( ; i < 2*(nColors/3); i++) {
      colors[i].red = i*fac[0] + off[0];
      colors[i].green = i*fac[1] + off[1];
      colors[i].blue = i*fac[2] + off[2];
    }
    /* third domain */
    for (j = 0; j < 3; j++)
      if (tlist[j + 6] >= 0) {
	fac[j] = tlist[j + 6]*factor;
	off[j] = 0.0 - i*fac[j];
      } else {
	fac[j] = -tlist[j + 6]*factor;
	off[j] = (1 + tlist[j + 6])*65535.0 - i*fac[j];
      }
    for ( ; i < 3*(nColors/3); i++) {
      colors[i].red = i*fac[0] + off[0];
      colors[i].green = i*fac[1] + off[1];
      colors[i].blue = i*fac[2] + off[2];
    }
    /* there may be one or two color cells left */
    for ( ; i < nColors; i++)
      colors[i].red = colors[i].green = colors[i].blue = 0;
    /* now we fix pixels[] */
    for (i = 0; i < 85; i++) {
      j = (i*(nColors/3))/85;
      pixels[i] = colors[j].pixel;
      pixels[85 + i] = colors[j + (nColors/3)].pixel;
      pixels[170 + i] = colors[j + 2*(nColors/3)].pixel;
    }
  }
  XStoreColors(display, colorMap, colors, nColors);

  XFlush(display);
  return LUX_OK;
}
/*---------------------------------------------------------*/
int32_t lux_colorComponents(int32_t narg, int32_t ps[])
/* colorcomponents,pixels,r,g,b */
/* takes raw pixel values <pixels> and returns the relative red, green, */
/* and blue components in <r>, <g>, and <b>, which range between 0 and 255. */
/* <pixels> must have type */
{
  uint8_t	*data;
  int32_t *q1, *q2, *q3;
  uint8_t	*red, *green, *blue;
  int32_t	nelem, i, step, pix;
  XColor	*color;

  if (!symbolIsNumericalArray(ps[0]))
    return cerror(ILL_CLASS, ps[0]);
  if (!isIntegerType(array_type(ps[0])))
    return luxerror("Can only decompose integer pixel values", ps[0]);
  if (lux_type_size[array_type(ps[0])]*8 != bits_per_pixel)
    return luxerror("Only arrays with %d bits per pixel are allowed with the current visual", ps[0], bits_per_pixel);

  if (!symbolIsNamed(ps[1]))
    return cerror(NEED_NAMED, ps[1]);
  if (!symbolIsNamed(ps[2]))
    return cerror(NEED_NAMED, ps[2]);
  if (!symbolIsNamed(ps[3]))
    return cerror(NEED_NAMED, ps[3]);

  data = (uint8_t*) array_data(ps[0]);
  nelem = array_size(ps[0]);
  to_scratch_array(ps[1], LUX_INT8, array_num_dims(ps[0]), array_dims(ps[0]));
  to_scratch_array(ps[2], LUX_INT8, array_num_dims(ps[0]), array_dims(ps[0]));
  to_scratch_array(ps[3], LUX_INT8, array_num_dims(ps[0]), array_dims(ps[0]));
  red = (uint8_t*) array_data(ps[1]);
  green = (uint8_t*) array_data(ps[2]);
  blue = (uint8_t*) array_data(ps[3]);
  step = bits_per_pixel/8;

  switch (visualclass(visual)) {
  case PseudoColor: case GrayScale: case StaticColor: case StaticGray:
    q1 = (int32_t*) malloc(display_cells*sizeof(int32_t));
    if (!q1)
      return cerror(ALLOC_ERR, 0);
    for (i = 0; i < display_cells; i++)
      if (colors[i].flags & (DoRed | DoGreen | DoBlue))
	q1[colors[i].pixel] = i;
    while (nelem--) {
      memcpy(&pix, data, step);
      color = &colors[q1[pix]];
      *red++ = color->red >> 8;
      *green++ = color->green >> 8;
      *blue++ = color->blue >> 8;
      data += step;
    }
    free(q1);
    break;
  case DirectColor:
    q1 = (int32_t*) malloc(display_cells*sizeof(int32_t));
    q2 = (int32_t*) malloc(display_cells*sizeof(int32_t));
    q3 = (int32_t*) malloc(display_cells*sizeof(int32_t));
    if (!q1 || !q2 || !q3)
      return cerror(ALLOC_ERR, 0);
    for (i = 0; i < nColors; i++) {
      q1[(colors[i].pixel & red_mask) >> red_mask_lower] = i;
      q2[(colors[i].pixel & green_mask) >> green_mask_lower] = i;
      q3[(colors[i].pixel & blue_mask) >> blue_mask_lower] = i;
    }
    while (nelem--) {
      memcpy(&pix, data, step);
      *red++ = colors[q1[(pix & red_mask) >> red_mask_lower]].red >> 8;
      *green++ = colors[q2[(pix & green_mask) >> green_mask_lower]].green >> 8;
      *blue++ = colors[q3[(pix & blue_mask) >> blue_mask_lower]].blue >> 8;
      data += step;
    }
    free(q1);
    free(q2);
    free(q3);
    break;
  case TrueColor:
    while (nelem--) {
      memcpy(&pix, data, step);
      *red++ = ((pix & red_mask) >> red_mask_lower) << (8 - red_mask_bits);
      *green++ = ((pix & green_mask) >> green_mask_lower) << (8 - green_mask_bits);
      *blue++ = ((pix & blue_mask) >> blue_mask_lower) << (8 - blue_mask_bits);
      data += step;
    }
    break;
  }
  return LUX_OK;
}
/*---------------------------------------------------------*/
int32_t lux_pixelsto8bit(int32_t narg, int32_t ps[])
 /* pixelsto8bit,pixels,bits,colormap
    returns 8-bit pixel values in <bits> and a color map in <colormap>
    based on the pixel values in <pixels>.  
  */
{
  int32_t result, ncolors, iq;
  int32_t lux_tolookup(int32_t, int32_t *);
  int32_t lux_byte_inplace(int32_t, int32_t *);

  if (!symbolIsNumericalArray(ps[0]))
    return cerror(ILL_CLASS, ps[0]);
  if (array_num_dims(ps[0]) != 2)
    return cerror(NEED_2D_ARR, ps[0]);
  if (!isIntegerType(array_type(ps[0])))
    return cerror(NEED_INT_ARG, ps[0]);

  /* we determine the number of shades of red, green, and blue */
  internalMode = 1;
  iq = ps[1];
  getFreeTempVariable(ps[1]);
  result = lux_tolookup(narg, ps);
  if (result == LUX_ERROR)
    return result;
  ncolors = array_size(ps[1]);
  if (ncolors <= 256) {
    int32_t dims[2] = { 3, 256 };
    int32_t i, pix;
    uint8_t *p, *q;
    int32_t *q1, step, *q2, *q3;

    /* we can use the indices from tolookup if we convert them to LUX_INT8 */
    if (lux_byte_inplace(1, ps + 2) == LUX_ERROR
	|| redef_array(iq, LUX_INT8, 2, dims) == LUX_ERROR)
      goto error_1;
    q = (uint8_t*) array_data(iq);
    p = (uint8_t*) array_data(ps[1]);
    step = bits_per_pixel/8;
    switch (visualclass(visual)) {
    case PseudoColor: case GrayScale: case StaticColor: case StaticGray:
      q1 = (int32_t*) malloc(display_cells*sizeof(int32_t));
      if (!q1) {
	cerror(ALLOC_ERR, 0);
	goto error_1;
      }
      for (i = 0; i < display_cells; i++)
	if (colors[i].flags & (DoRed | DoGreen | DoBlue))
	  q1[colors[i].pixel] = i;
      for (i = 0; i < ncolors; i++) {
	int32_t pix;
	XColor *color;

	memcpy(&pix, p, step);
	color = &colors[q1[pix]];
	*q++ = color->red >> 8;
	*q++ = color->green >> 8;
	*q++ = color->blue >> 8;
	p += step;
      }
      free(q1);
      for ( ; i < 256; i++) {
	*q++ = 0;
	*q++ = 0;
	*q++ = 0;
      }
      break;
    case DirectColor:
      q1 = (int32_t*) malloc(display_cells*sizeof(int32_t));
      q2 = (int32_t*) malloc(display_cells*sizeof(int32_t));
      q3 = (int32_t*) malloc(display_cells*sizeof(int32_t));
      if (!q1 || !q2 || !q3) {
	free(q1);
	free(q2);
	cerror(ALLOC_ERR, 0);
	goto error_1;
      }
      for (i = 0; i < nColors; i++) {
	q1[(colors[i].pixel & red_mask) >> red_mask_lower] = i;
	q2[(colors[i].pixel & green_mask) >> green_mask_lower] = i;
	q3[(colors[i].pixel & blue_mask) >> blue_mask_lower] = i;
      }
      for (i = 0; i < ncolors; i++) {
	memcpy(&pix, p, step);
	*q++ = colors[q1[(pix & red_mask) >> red_mask_lower]].red >> 8;
	*q++ = colors[q2[(pix & green_mask) >> green_mask_lower]].green >> 8;
	*q++ = colors[q3[(pix & blue_mask) >> blue_mask_lower]].blue >> 8;
	p += step;
      }
      free(q1);
      free(q2);
      free(q3);
      for ( ; i < 256; i++) {
	*q++ = 0;
	*q++ = 0;
	*q++ = 0;
      }
      break;
    case TrueColor:
      for (i = 0; i < ncolors; i++) {
	memcpy(&pix, p, step);
	*q++ = ((pix & red_mask) >> red_mask_lower) << (8 - red_mask_bits);
	*q++ = ((pix & green_mask) >> green_mask_lower) << (8 - green_mask_bits);
	*q++ = ((pix & blue_mask) >> blue_mask_lower) << (8 - blue_mask_bits);
	p += step;
      }
      for ( ; i < 256; i++) {
	*q++ = 0;
	*q++ = 0;
	*q++ = 0;
      }
      break;
    }
  } else {
    return luxerror("More than 256 different colors!", 0);
  }
  zapTemp(ps[1]);	/* no longer needed */
  ps[1] = iq;
  return LUX_OK;

  error_1:
  zapTemp(ps[1]);
  ps[1] = iq;
  return LUX_ERROR;
}
/*---------------------------------------------------------*/
int32_t lux_colorstogrey(int32_t narg, int32_t ps[])
/* colorstogrey,pixels
   takes raw pixel values and replaces them by the corresponding grey
   scale values, which range between 0 and 255.
   LS 2003mar08 */
{
  int32_t *q1, *q2, *q3, i;
  uint8_t *data;
  int32_t nelem, red, green, blue, grey, pix = 0, step;
  XColor *color;

  if (!symbolIsNumericalArray(ps[0]))
    return cerror(ILL_CLASS, ps[0]);
  if (!isIntegerType(array_type(ps[0])))
    return luxerror("Can only decompose integer pixel values", ps[0]);
  if (lux_type_size[array_type(ps[0])]*8 != bits_per_pixel)
    return luxerror("Only arrays with %d bits per pixel are allowed with the current visual", ps[0], bits_per_pixel);
  data = (uint8_t*) array_data(ps[0]);
  nelem = array_size(ps[0]);
  step = bits_per_pixel/8;
  switch (visualclass(visual)) {
  case PseudoColor: case GrayScale: case StaticColor: case StaticGray:
    q1 = (int32_t*) malloc(display_cells*sizeof(int32_t));
    if (!q1)
      return cerror(ALLOC_ERR, 0);
    for (i = 0; i < display_cells; i++)
      if (colors[i].flags & (DoRed | DoGreen | DoBlue))
	q1[colors[i].pixel] = i;
    while (nelem--) {
      memcpy(&pix, data, step);
      color = &colors[q1[pix]];
      red = color->red >> 8;
      green = color->green >> 8;
      blue = color->blue >> 8;
      grey = (red + green + blue)/3;
      memcpy(data, &grey, step);
      data += step;
    }
    free(q1);
    break;
  case DirectColor:
    q1 = (int32_t*) malloc(display_cells*sizeof(int32_t));
    q2 = (int32_t*) malloc(display_cells*sizeof(int32_t));
    q3 = (int32_t*) malloc(display_cells*sizeof(int32_t));
    if (!q1 || !q2 || !q3)
      return cerror(ALLOC_ERR, 0);
    for (i = 0; i < nColors; i++) {
      q1[(colors[i].pixel & red_mask) >> red_mask_lower] = i;
      q2[(colors[i].pixel & green_mask) >> green_mask_lower] = i;
      q3[(colors[i].pixel & blue_mask) >> blue_mask_lower] = i;
    }
    while (nelem--) {
      memcpy(&pix, data, step);
      red = colors[q1[(pix & red_mask) >> red_mask_lower]].red >> 8;
      green = colors[q2[(pix & green_mask) >> green_mask_lower]].green >> 8;
      blue = colors[q3[(pix & blue_mask) >> blue_mask_lower]].blue >> 8;
      grey = (red + green + blue)/3;
      memcpy(data, &grey, step);
      data += step;
    }
    free(q1);
    free(q2);
    free(q3);
    break;
  case TrueColor:
    while (nelem--) {
      memcpy(&pix, data, step);
      red = ((pix & red_mask) >> red_mask_lower) << (8 - red_mask_bits);
      green = ((pix & green_mask) >> green_mask_lower) << (8 - green_mask_bits);
      blue = ((pix & blue_mask) >> blue_mask_lower) << (8 - blue_mask_bits);
      grey = (red + green + blue)/3;
      memcpy(data, &grey, step);
      data += step;
    }
    break;
  }
  return LUX_OK;
}
