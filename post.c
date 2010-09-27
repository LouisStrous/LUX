/* File post.c */
/* ANA routines dealing with PostScript output. */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "action.h"
static char rcsid[] __attribute__ ((unused)) =
 "$Id: post.c,v 4.0 2001/02/07 20:37:04 strous Exp $";

#if DEBUG
#define _Xdebug			/* to get synchronous X error messages */
#endif

int	ipen = 7, ipost = 0;
extern int	landscape;
static int	nline = 0,	/* number of lines since last Stroke */
  icnt = 0;			/* number of chars on current output line */
static float	xq, yq;
extern float	current_gray;
extern int	current_pen;
FILE	*postScriptFp;
void	xyup(void), xydown(void);
/*------------------------------------------------------------------------*/
/*
  x setgray		selects DeviceGray color space
  width height bits/sample matrix datasrc image

  r g b setrgbcolor	selects DeviceRGB color space
  width height bits/comp matrix datasrc false 3 colorimage
 */
int postreset(int landscape)
{
 char	*fname = NULL;
 extern int	psfile, updateBoundingBox;

 if (ipost == 0) {	/* check if already started up */
   if (symbol_type(psfile) == ANA_TEMP_STRING)
     fname = string_value(psfile);
   if (fname)
     fname = expand_name(fname, NULL);
   /* open for update; bit elaborate because different operating systems */
   /* have different ideas about what "update" means.  LS 18jan95 */
   postScriptFp = Fopen(fname, "w+");
   if (!postScriptFp) {
     printf("Can't open postscript file %s\n", fname? fname: "junk.eps");
     return ANA_ERROR;
   }
   ipost = 1;
   /* some preambles */
   fputs("%!PS-Adobe-1.0\n", postScriptFp);
   fputs("%%Creator: ANA generated plot image\n", postScriptFp);
   fputs("%%BoundingBox:  0000  0000  0600  0720\n", postScriptFp);
   fputs("%%EndComments\n", postScriptFp);
   fputs("/M {moveto} bind def /L {lineto} bind def\n", postScriptFp);
   fputs("/S {stroke} bind def\n", postScriptFp);
   fputs("1 setlinecap 1 setlinejoin 720 720 scale\n", postScriptFp);
   if (landscape == 0)
     fputs(".05 .025 translate\n", postScriptFp);
   else
     fputs("90 rotate .05 -.825 translate\n", postScriptFp);
   fprintf(postScriptFp, "%f setlinewidth\n", current_pen*0.00033333);
   /* The next line implicitly selects the DeviceGray color space */
   fprintf(postScriptFp, "%f setgray \n", 1 - current_gray); 
   icnt = nline = 0;
   updateBoundingBox = 1;
 }
 return 1;
}
/*------------------------------------------------------------------------*/
int postpen(int pen, float gray)
{
  if (postreset(landscape) == ANA_ERROR) /* start up if not already */
    return ANA_ERROR;
  if (nline) {
    fputs("\n", postScriptFp);
    fputs("S\n", postScriptFp);
    icnt = nline = 0;
    xyup();
  }
  fprintf(postScriptFp, " %f setlinewidth \n", pen*.00033333);
  fprintf(postScriptFp, " %f setgray \n", 1 - gray);
  xyup();
  return 1;
}
/*------------------------------------------------------------------------*/
int postcolorpen(float red, float green, float blue)
{
  postreset(landscape);		/* start up if not already */
  if (nline) {
    fputs("\n", postScriptFp);
    xyup();
    fputs(" S\n", postScriptFp);
    icnt = 0;
    nline = 0;
  }
  fprintf(postScriptFp, " %f %f %f setrgbcolor \n", red, green, blue);
  /* xyup(); */
  return 1;
}
/*------------------------------------------------------------------------*/
int postvec(float xin, float yin, int mode)
{
  if (postreset(landscape) == ANA_ERROR) /* start up if not already */
    return ANA_ERROR;
  xq = MAX(0, xin);
  xq = MIN(xq, 0.9998);
  yq = MAX(0, yin);
  yq = MIN(yq, 0.9998);
  if (mode == 1)
    xydown();
  else
    xyup();
  nline++;
  if (nline > 500) {	/* time for a stroke */
    nline = icnt = 0;
    fputs("\n", postScriptFp);
    fputs("S\n", postScriptFp);
    xyup();
  }
  if (icnt > 55) {
    fputs("\n", postScriptFp);
    icnt = 0;
  }
  return 1;
}
/*------------------------------------------------------------------------*/
void xyup(void)
{
  extern int	alternateDash;

  if (alternateDash)
    fprintf(postScriptFp, " %.6f setgray %.4f %.4f L %.6f setgray",
	    1 - current_gray, xq, yq, current_gray);
  else
    fprintf(postScriptFp," %5.4f %5.4f M", xq, yq);
  icnt += 14;
}
/*------------------------------------------------------------------------*/
void xydown(void)
{
  fprintf(postScriptFp," %5.4f %5.4f L", xq, yq);
  icnt += 14;
}
/*------------------------------------------------------------------------*/
int postcopy()
{
  if (!ipost) {			/* if the output file is not yet opened, */
	/* then we don't need to generate an empty page with "showpage". */
	/* LS 24oct95 */
    if (postreset(landscape) == ANA_ERROR)/* start up if not already */
      return ANA_ERROR;
  } else {
    fputs("\n", postScriptFp);
    icnt = 0;
    if (nline) {
      fputs(" stroke\n", postScriptFp);
      nline = icnt = 0;
    }
    fputs(" gsave\n", postScriptFp);
    fputs(" showpage\n", postScriptFp);
    fputs(" grestore\n", postScriptFp);
  }
  return 1;
}
/*------------------------------------------------------------------------*/
int postrawout(char *s)
{
  if (postreset(landscape) == ANA_ERROR) /* start up if not already */
    return ANA_ERROR;
  if (nline) {
    fputs(" stroke\n", postScriptFp);
    nline = icnt = 0;
  }
  fputs(s, postScriptFp);
  return 1;
}
/*------------------------------------------------------------------------*/
int postgray(char *barray, int nx, int ny, float xbot, float xtop, float ybot,
	     float ytop, int iorder)
{
  int	matrx[6], i;
  int	c;
  extern float	postXBot, postXTop, postYBot, postYTop;

  for (i = 0; i < 6; i++)
    matrx[i] = 0;
  if (xbot > xtop || ybot > ytop) {
    printf("illegal image window\n");
    return ANA_ERROR;
  }

  /* ought to change !iorder stuff so that for !iorder = 0 the image on */
  /* paper is oriented the same was as on screen  LS 19jan95 */

  if (postreset(landscape) == ANA_ERROR) /* start up if not already */
    return ANA_ERROR;
  if (nline) {
    fputs(" stroke\n", postScriptFp);
    nline = icnt = 0;
  }
  fputs(" gsave\n", postScriptFp);
  fprintf(postScriptFp, " /picstr %d string def ", nx);
  fprintf(postScriptFp, "%7.4f %7.4f translate\n", xbot, ybot);
  fprintf(postScriptFp, "%7.4f %7.4f scale ", xtop-xbot, ytop-ybot);
  fprintf(postScriptFp, "%d %d 8\n", nx, ny);
  switch (iorder) {
    case 0:
      matrx[0] = nx;
      matrx[3] = ny;
      break;
    case 1:
      matrx[1] = nx;
      matrx[2] = ny;
      break;
    case 2:
      matrx[0] = nx;
      matrx[5] = ny;
      matrx[3] = -ny;
      break;
    case 3:
      matrx[1] = nx;
      matrx[4] = ny;
      matrx[2] = -ny;
      break;
    case 4:
      matrx[4] = nx;
      matrx[3] = ny;
      matrx[0] = -nx;
      break;
    case 5:
      matrx[5] = nx;
      matrx[2] = ny;
      matrx[1] = -ny;
      break;
    case 6:
      matrx[4] = nx;
      matrx[5] = ny;
      matrx[3] = -ny;
      matrx[0] = -nx;
      break;
    case 7:
      matrx[5] = nx;
      matrx[1] = -ny;
      matrx[2] = -ny;
      matrx[4] = nx;
    break;
  }
  fprintf(postScriptFp,"[ %d %d %d %d %d %d ]\n", matrx[0], matrx[1], matrx[2],
	  matrx[3], matrx[4], matrx[5]);
  fputs(" {currentfile picstr readhexstring pop} image\n", postScriptFp);
  for (i = 1; i <= nx*ny; i++) {
    c = *barray++;
    c &= 0xff;
    fprintf(postScriptFp, "%02.2x", c);
    if (i % 40 == 0)
      fprintf(postScriptFp, "\n");
  }
  if (i % 40 != 0)
    fprintf(postScriptFp, "\n");
  fputs(" grestore\n", postScriptFp);
  /* always update bounding box */
  if (xbot < postXBot)
    postXBot = xbot;
  if (xtop > postXTop)
    postXTop = xtop;
  if (ybot < postYBot)
    postYBot = ybot;
  if (ytop > postYTop)
    postYTop = ytop;
  return 1;
}
/*------------------------------------------------------------------------*/
int postcolor(char *barray, int nx, int ny, float xbot, float xtop, float ybot,
	      float ytop, int iorder)
/* writes an RGB image to a postscript file.  The image has dimensions
 <nx> by <ny> and has three bytes of color information per pixel, in
 the order red, green, blue.  <xbot>, <xtop>, <ybot>, <ytop> selects
 the coordinate ranges (in PostScript units) where the image is displayed.
 LS 10oct99 */
{
  int	matrx[6], i, s;
  int	c;
  extern float	postXBot, postXTop, postYBot, postYTop;

  for (i = 0; i < 6; i++)
    matrx[i] = 0;
  if (xbot > xtop || ybot > ytop) {
    printf("illegal image window\n");
    return ANA_ERROR;
  }

  /* ought to change !iorder stuff so that for !iorder = 0 the image on */
  /* paper is oriented the same was as on screen  LS 19jan95 */

  if (postreset(landscape) == ANA_ERROR) /* start up if not already */
    return ANA_ERROR;
  if (nline) {
    fputs(" stroke\n", postScriptFp);
    nline = icnt = 0;
  }
  fputs(" gsave\n", postScriptFp);
  fprintf(postScriptFp, " /picstr %d string def ", nx);
  fprintf(postScriptFp, "%7.4f %7.4f translate\n", xbot, ybot);
  fprintf(postScriptFp, "%7.4f %7.4f scale ", xtop-xbot, ytop-ybot);
  fprintf(postScriptFp, "%d %d 8\n", nx, ny);
  switch (iorder) {
    case 0:
      matrx[0] = nx;
      matrx[3] = ny;
      break;
    case 1:
      matrx[1] = nx;
      matrx[2] = ny;
      break;
    case 2:
      matrx[0] = nx;
      matrx[5] = ny;
      matrx[3] = -ny;
      break;
    case 3:
      matrx[1] = nx;
      matrx[4] = ny;
      matrx[2] = -ny;
      break;
    case 4:
      matrx[4] = nx;
      matrx[3] = ny;
      matrx[0] = -nx;
      break;
    case 5:
      matrx[5] = nx;
      matrx[2] = ny;
      matrx[1] = -ny;
      break;
    case 6:
      matrx[4] = nx;
      matrx[5] = ny;
      matrx[3] = -ny;
      matrx[0] = -nx;
      break;
    case 7:
      matrx[5] = nx;
      matrx[1] = -ny;
      matrx[2] = -ny;
      matrx[4] = nx;
    break;
  }
  fprintf(postScriptFp,"[ %d %d %d %d %d %d ]\n", matrx[0], matrx[1], matrx[2],
	  matrx[3], matrx[4], matrx[5]);
  fputs(" {currentfile picstr readhexstring pop} false 3 colorimage\n", postScriptFp);
  s = nx*ny;
  for (i = 1; i <= nx*ny; i++) {
    c = *barray++;
    fprintf(postScriptFp, "%02.2x", c);
    c = barray[s];
    fprintf(postScriptFp, "%02.2x", c);
    c = barray[2*s];
    fprintf(postScriptFp, "%02.2x", c);
    if (i % 40 < 3)
      fprintf(postScriptFp, "\n");
  }
  if (i % 40 >= 3)
    fprintf(postScriptFp, "\n");
  fputs(" grestore\n", postScriptFp);
  /* always update bounding box */
  if (xbot < postXBot)
    postXBot = xbot;
  if (xtop > postXTop)
    postXTop = xtop;
  if (ybot < postYBot)
    postYBot = ybot;
  if (ytop > postYTop)
    postYTop = ytop;
  return ANA_OK;
}
/*------------------------------------------------------------------------*/
int postrelease(int narg, int ps[])
{
  char  ok = 0;
  int	bb1, bb2, bb3, bb4;
  extern float	postXBot, postXTop, postYBot, postYTop;
  extern int	psfile;

  if (ipost) {
    if (narg)
      bb1 = float_arg(*ps++);
    if (narg > 1)
      bb2 = float_arg(*ps++);
    if (narg > 2)
      bb3 = float_arg(*ps++);
    if (narg > 3)
      bb4 = float_arg(*ps++);
    fputs("\n", postScriptFp);
    icnt = 0;
    if (nline) 
      fputs(" stroke\n", postScriptFp);
    fputs(" showpage\n", postScriptFp);
    /* now insert bounding box  LS 18jan95 */
    if (fseek(postScriptFp, 66, SEEK_SET)) {	/* length of preamble */
      printf("Could not reset file pointer to insert bounding box into %s\n",
	     string_value(psfile));
      puts("Using default.");
    } else {
      if (!narg) {
	if (postXTop < postXBot
	    || postYTop < postYBot) { /* no bounding box */
	  bb1 = bb2 = 0;
	  bb3 = 600;
	  bb4 = 720;
	  ok = 1;
	}
      } else {
	postXBot = bb1;
	postXTop = bb2;
	postYBot = bb3;
	postYTop = bb4;
      }
      if (!ok) {
	if (landscape) {
	  bb1 = (int) (594 - 720*postYTop);
	  bb2 = (int) (36 + 720*postXBot);
	  bb3 = (int) (594 - 720*postYBot);
	  bb4 = (int) (36 + 720*postXTop);
	} else {
	  bb1 = (int) (720*(0.05 + postXBot));
	  bb2 = (int) (720*(0.025+ postYBot));
	  bb3 = (int) (720*(0.05 + postXTop));
	  bb4 = (int) (720*(0.025 + postYTop));
	}	  
      }
      fprintf(postScriptFp, "% 4.4d % 4.4d % 4.4d % 4.4d", bb1, bb2, bb3, bb4);
    }
    fclose(postScriptFp);
  }
  ipost = 0;
  return 1;
}
/*------------------------------------------------------------------------*/
