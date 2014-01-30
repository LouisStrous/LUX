/* This is file hersh.c.

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
/* File hersh.c */
/* LUX routines for dealing with Hershey vector fonts. */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include "action.h"

#define	VSPACE	14.0
#define	SFAC	0.70
Int	tkplot(double x, double y, Int lineStyle, Int symStyle), fontchange(Int),
  draw(Int);
void	swapl(char *, Int);
extern	double	xfac, yfac;
extern	char	*expand_name(char *, char *);
Int	hflag, penDown;
extern Int	ifont;
Byte	*fontbase;
Byte	*fontptr[40];
double	callig_xb, callig_yb, theta = 0.0, size = 1.0, dx, dy, sxb, syb;
double	st, ct, x, y, nct, nst, angle, callig_ratio = 1.0;
double	double_arg(Int);
Int	calligCoordSys, current_font = 1;
/*------------------------------------------------------------------------- */
Int lux_callig(Int narg, Int ps[])
/* draws Hershy character sets */
{
  Int	iq;
  char	*s;
  extern Int	tkCoordSys;
  void	set_cur_pen(void);
  Int	callig2(char *);

  penDown = 1;
					/* first arg must be a string */
  iq = ps[0];
  if (symbol_class(iq) != LUX_STRING)
    return cerror(NEED_STR, *ps);
  s = string_value(iq);
					/* the rest are scalars */
  if (narg > 1 && ps[1])
    callig_xb = double_arg(ps[1]);
  if (narg > 2 && ps[2])
    callig_yb = double_arg(ps[2]);
  if (narg > 3 && ps[3])
    size = double_arg(ps[3]);
  if (narg > 4 && ps[4])
    theta = double_arg(ps[4]);
  if (narg > 5 && ps[5])
    ifont = int_arg(ps[5]);
  if (narg > 6 && ps[6])
    penDown = int_arg(ps[6]);
  calligCoordSys = (internalMode & 7);
  tkCoordSys = LUX_DVI;
  /*  set_cur_pen(); */   /*interferes with plot colors */
  iq = callig2(s);
  return iq;
}
/*------------------------------------------------------------------------- */
Int strpcmp(const void *arg1, const void *arg2)
/* compare strings <arg1> (up to the next non-alphabetic character) and
   <arg2>, and return a number > 0, == 0, or < 0, depending on whether
   <arg2> is lexographically after, equal to, or before <arg1>.  LS 1jun99 */
{
  char	*s1, *s2;
  
  s1 = (char *) arg1;
  s2 = *(char **) arg2;
  while (*s1 == *s2 && *s2) {	/* while equal and not at end of s2 */
    s1++;
    s2++;
  }
  /* when we get here then either the two arguments are no longer equal,
     or we're at the end of arg2. */
  if (*s2) {			/* not at the end of arg2 -> unequal */
    if (isalpha((Int) *s1))
      return *s1 - *s2;
    return -1;			/* s2 is "greater" */
  }
  return isalpha((Int) *s1)? +1: 0;
}
/*------------------------------------------------------------------------- */
void drawlatex(char **text)
/* services a TeX-style calligraphy command.  This routine is called
 with **text == '`', or recursively by drawlatex() itself. */
{
  static char	*latex_codes[] = {
    "Alpha", "Beta", "Chi", "Delta", "Epsilon", "Eta", "Gamma",
    "Iota", "Kappa", "Lambda", "Mu", "Nu", "Omega", "Omicron", "Phi",
    "Pi", "Psi", "Rho", "Sigma", "Tau", "Theta", "Upsilon", "Xi",
    "Zeta", "alpha", "beta", "bf", "box", "cap", "chi", "clubsuit",
    "cup", "dag", "ddag", "delta", "diamondsuit", "downarrow",
    "epsilon", "eta", "exists", "gamma", "ge", "heartsuit", "in",
    "Int", "iota", "it", "kappa", "lambda", "le", "leftarrow", "mu",
    "nabla", "nu", "odot", "oint", "omega", "omicron", "partial",
    "perp", "phi", "pi", "pm", "propto", "psi", "rho", "rightarrow",
    "rm", "sf", "sigma", "sim", "size", "spadesuit", "subset",
    "supset", "surd", "tau", "theta", "times", "uparrow", "upsilon",
    "xi", "zeta"
  };
  enum	codes {
    Alpha, Beta, Chi, Delta, Epsilon, Eta, Gamma,
    Iota, Kappa, Lambda, Mu, Nu, Omega, Omicron, Phi,
    Pi, Psi, Rho, Sigma, Tau, Theta, Upsilon, Xi,
    Zeta, alpha, beta, bf, box, cap, chi, clubsuit,
    cup, dag, ddag, delta, diamondsuit, downarrow,
    epsilon, eta, exists, gamma, ge, heartsuit, in,
    inti, iota, it, kappa, lambda, le, leftarrow, mu,
    nabla, nu, odot, oint, omega, omicron, partial,
    perp, phi, pi, pm, propto, psi, rho, rightarrow,
    rm, sf, sigma, sim, resize, spadesuit, subset,
    supset, surd, tau, theta, times, uparrow, upsilon,
    xi, zeta
  };
  static char *fonts =  "77777777777777777777777777099799997997797999970779979799779977997790079099997799777";
  static char *member = "ABVDEGCIJKLMXOUPWQRSHTNFab0B3vU1OodV5egEcbueii0jkl4lGmnJxodxup+cwq600rA0U02rshX7tnf";
  char	**match, *p1, *p2;
  Int	code, oldfont, level, c;
  double	newsize;

  while (1)
    switch ((Int) **text) {
      case '^':			/* superscript */
	/* figure out the token(s) that this applies to */
	(*text)++;
	p1 = p2 = *text;
	level = 0;
	if (*p1 == '{') {	/* a group */
	  do switch (*p2++) {	/* find the matching closing brace */
	    case '{':
	      level++;
	      break;
	    case '}':
	      level--;
	      break;
	    case '\0':
	      level = 0;
	      break;
	  } while (level);
	  p1++;			/* just after the opening brace */
	  p2--;			/* just before the closing brace */
	  level--;		/* to indicate that we are treating a group */
	} else if (*p1 == '`') { /* a token */
	  p2++;
	  while (isalpha((Int) *p2))
	    p2++;
	} else			/* take a single character */
	  p2++;
	c = *p2;		/* temporarily terminate the text here */
	*p2 = '\0';
	callig_xb = callig_xb - st * VSPACE;
	dx -= st * VSPACE;
	callig_yb = callig_yb + ct * VSPACE * callig_ratio;
	dy += ct * VSPACE * callig_ratio;
	st *= SFAC;
	ct *= SFAC;
	drawlatex(&p1);		/* treat the argument */
	callig_xb = callig_xb - dx;
	callig_yb = callig_yb - dy;
	dx = dy = 0;
	st /= SFAC;
	ct /= SFAC;
	*p2 = c;		/* restore the text */
	if (level)		/* we worked on a brace-enclosed group */
	  p2++;			/* skip the final closing brace */
	*text = p2;
	return;
      case '_':			/* subscript */
	/* figure out the token(s) that this applies to */
	(*text)++;
	p1 = p2 = *text;
	level = 0;
	if (*p1 == '{') {	/* a group */
	  do switch (*p2++) {	/* find the matching closing brace */
	    case '{':
	      level++;
	      break;
	    case '}':
	      level--;
	      break;
	    case '\0':
	      level = 0;
	      break;
	  } while (level);
	  p1++;			/* just after the opening brace */
	  p2--;			/* just before the closing brace */
	  level--;		/* to indicate that we are treating a group */
	} else if (*p1 == '`') { /* a token */
	  p2++;
	  while (isalpha((Int) *p2))
	    p2++;
	} else			/* take a single character */
	  p2++;
	c = *p2;		/* temporarily terminate the text here */
	*p2 = '\0';
	callig_xb = callig_xb + st * VSPACE;
	dx += st * VSPACE;
	callig_yb = callig_yb - ct * VSPACE * callig_ratio;
	dy -= ct * VSPACE * callig_ratio;
	st *= SFAC;
	ct *= SFAC;
	drawlatex(&p1);		/* treat the argument */
	callig_xb = callig_xb - dx;
	callig_yb = callig_yb - dy;
	dx = dy = 0;
	st /= SFAC;
	ct /= SFAC;
	*p2 = c;		/* restore the text */
	if (level)		/* we worked on a brace-enclosed group */
	  p2++;			/* skip the final closing brace */
	*text = p2;
	return;
      case '{':
	p1 = p2 = *text;
	level = 0;
	do switch (*p2++) {	/* find the matching closing brace */
	  case '{':
	    level++;
	    break;
	  case '}':
	    level--;
	    break;
	  case '\0':
	    level = 0;
	    break;
	} while (level);
	p1++;			/* just after the opening brace */
	p2--;			/* just before the closing brace */
	level--;		/* to indicate that we are treating a group */
	c = *p2;
	*p2 = '\0';
	drawlatex(&p1);
	*p2 = c;
	if (level)
	  p2++;
	*text = p2;
	return;
      default:
	draw(**text);
	(*text)++;
	continue;
      case '\0':
	return;
      case '`':			/* TeX-style command */
	(*text)++;
	match = bsearch(*text, latex_codes, sizeof(latex_codes)/sizeof(char *),
			sizeof(char *), strpcmp);
	if (match) {
	  code = match - latex_codes;
	  switch (code) {
	    default:
	      oldfont = current_font;
	      fontchange(fonts[code] - '0');
	      draw(member[code]);
	      fontchange(oldfont);
	      break;
	    case bf:
	      fontchange(5);
	      break;
	    case it:
	      fontchange(8);
	      break;
	    case rm:
	      fontchange(6);
	      break;
	    case sf:
	      fontchange(1);
	      break;
	    case resize:
	      p1 = *text + strlen(*match);
	      p2 = p1;
	      level = 0;
	      if (*p2 == '{') {
		do switch (*p2++) { /* find the matching closing brace */
		  case '{':
		    level++;
		    break;
		  case '}':
		    level--;
		    break;
		  case '\0':
		    level = 0;
		    break;
		} while (level);
		p1++;		/* just after the opening brace */
		p2--;		/* just before the closing brace */
		level--;	/* to indicate that we are treating a group */
	      } else if (isdigit((Int) *p2))
		p2 += 1;
	      if (p2 > p1) {
		c = *p2;
		*p2 = '\0';
		newsize = strtod(p1, &p2);
		st = st * (newsize / size );
		ct = ct * (newsize / size );
		*p2 = c;
		*text += p2 - p1;
	      }
	      break;
	  }
	  *text += strlen(*match);
	} else
	  draw(*(*text++));
	break;
    }
}
/*------------------------------------------------------------------------- */
Int callig2(char *s)
/* called by either lux_callig or callig, finishes the job */
{
  Int	ic;
  double	angle, xq;
  Int	coordTrf(double *, double *, Int, Int), fontchange(Int), hcom(char **),
    draw(Int), empty(void);
					/* setup context */
  coordTrf(&callig_xb, &callig_yb, calligCoordSys, LUX_DVI);
  angle = theta*0.017453293;
  xq= size/1024.;
  nct = ct = cos(angle)*xq;
  nst = st = sin(angle)*xq;
					/* setup font */
  if (fontchange(ifont) < 0)
    return LUX_ERROR;	/* propagate errors */
  dx = dy = 0; 
  while ((ic = *s++) != 0) {		/* decode string */
	/* either a command or a char. to draw */
    switch (ic) {
      case '$':
	if (hcom( &s) != 1)
	  return LUX_ERROR;
	break;
      case '`': case '^': case '_': case '{': case '}':
	s--;
	drawlatex(&s);
	break;
      default:
	if (draw(ic) != 1)
	  return LUX_ERROR;
	break;
    }
  }
  if (penDown)
    empty();
  return 1;
}
/*------------------------------------------------------------------------- */
Int callig(char *s, double xb, double yb, double fsize, double th, Int font,
	   Int nu)
/* for internal calls, sets variables and calls callig2 */
{
  callig_xb = xb;
  callig_yb = yb;
  theta = th;
  ifont = font;
  penDown = nu;
  size = fsize;
  return callig2(s);
}
/*------------------------------------------------------------------------- */
Int hcom(char **s)
					/* an inline callig command */
{
  Int	ic, i1;
  double	newsize;

  ic = *(*s)++;
  switch (ic) {
    case '0':  case '1':  case '2':  case '3':  case '4':  case '5':
    case '6':  case '7':  case '8':  case '9':
						/* font change */
      ic = ic % 128;
      i1 = ic - 48;			/* convert ASCII to binary */
      if ( isdigit ((Byte) **s ) ) i1 = 10 * i1 + (*(*s)++) - 48;
      if (fontchange(i1) < 0) return -1;	break;
    case 'A':  case 'a':
				/* move up, no change in size */
      callig_xb = callig_xb - st * VSPACE;
      dx -= st * VSPACE;
      callig_yb = callig_yb + ct * VSPACE * callig_ratio;
      dy += ct * VSPACE * callig_ratio;
      break;
    case 'E':  case 'e':
				/* move up, change in size */
      callig_xb = callig_xb - st * VSPACE;
      dx -= st * VSPACE;
      callig_yb = callig_yb + ct * VSPACE * callig_ratio;
      dy += ct * VSPACE * callig_ratio;
      st *= SFAC;
      ct *= SFAC;
      break;
    case 'I':  case 'i':
				/* move down, change in size */
      callig_xb = callig_xb + st * VSPACE;
      dx += st * VSPACE;
      callig_yb = callig_yb - ct * VSPACE * callig_ratio;
      dy -= ct * VSPACE * callig_ratio;
      st *= SFAC;
      ct *= SFAC;
      break;
    case 'B':  case 'b':
				/* move down, no change in size */
      callig_xb = callig_xb + st * VSPACE;
      dx += st * VSPACE;
      callig_yb = callig_yb - ct * VSPACE * callig_ratio;
      dy -= ct * VSPACE * callig_ratio;
      break;
    case 'N':  case 'n':
				/* restore normal */
      callig_xb = callig_xb - dx;
      callig_yb = callig_yb - dy;
      dx = dy = 0;
      st = nst;
      ct = nct;
      break;
    case '$':  case '<':  case '>':
      if (draw(ic) != 1) return -1;
      break;
    case 'S':  case 's':
				/* save position */
      sxb = callig_xb;
      syb = callig_yb;
      break;
    case 'R':  case 'r':
				/* restore position */
      callig_xb = sxb;
      callig_yb = syb;
      break;
    case 'X':  case 'x':
				/* restore entry font */
      if (fontchange(ifont) < 0) return -1;
      break;
    case 'C':  case 'c':
				/* size change */
      newsize = strtod(*s, s);	/* modified LS 3mar94 */
      st = st * (newsize / size );
      ct = ct * (newsize / size );
      break;
    case '.':
				/* draw a bullet */
      if (fontchange(20) < 0) return -1;
      if (draw('B') != 1) return -1;
      if (fontchange(ifont) < 0) return -1;
      break;
    case '*':
				/* draw a star */
      if (fontchange(20) < 0) return -1;
      if (draw('E') != 1) return -1;
      if (fontchange(ifont) < 0) return -1;
      break;
    }			/* end of switch */
  return 1;
}
/*------------------------------------------------------------------------- */
Int draw(Int ic)
{
  Int	nvec, offset, ll0 = 0, mode, iq, ix, iy, gap;
  short	is1, is2;
  pointer p;
  extern char	callig_update;	/* added 4oct93 LS (see file plots.c) */

  if (isprint(ic) == 0)
  { printf("illegal char. in callig\n");  return -1; }
  ic -= 32;
  p.b = fontbase;
  p.b = p.b + 4 * ic;	nvec = (Int) *p.b;	gap = (Int) *(p.b + 1);
  if (penDown)
  {
#if WORDS_BIGENDIAN
    is1 = *(p.w + 1);
    swab((char *) &is1, (char *) &is2, 2);
#else
    is2 = *(p.w + 1);
#endif
    offset = (Int) is2;
			/* offset is in I*2 amounts */
    p.b = fontbase;
    p.w += offset;
			/* check if abnormal */
#if WORDS_BIGENDIAN
    is2 = *p.w;	swab((char *) &is2, (char *) &is1, 2);
#else
    is1 = *p.w;
#endif
    if (is1 < 16384)
    { printf("special symbol, sorry - not supported\n");
      return 1; }   /* modified LS 4mar94 to return 1 instead of -1 */
    while (nvec--)
    {
#if WORDS_BIGENDIAN
      is2 = *p.w++;	swab((char *) &is2, (char *) &is1, 2);
      iq = (Int) is1;
#else
      iq = (Int) *p.w++;
#endif
      if (iq & 0x4000) mode = 0; else mode = 1;
      iy = (iq & 0x7f);  ix = (iq/ 0x7f) & 0x7f;
      if (iy > 63) iy = iy - 128;
      if (ix > 63) ix = ix - 128;
      ix = ix - ll0;
      iy = iy - ll0;		/* for abnormals */
      x = (double) ix*ct - (double) iy*st + callig_xb;
      y = ((double) ix*st + (double) iy*ct)*callig_ratio + callig_yb;
      callig_update = 0;
      tkplot(x, y, mode, 0);
      callig_update = 1;
    }
  }
			/* update callig_xb and callig_yb */
  callig_xb += ct*(double) gap;
  callig_yb += st*(double) gap*callig_ratio;
  return 1;
}
/*------------------------------------------------------------------------- */
Int fontchange(Int font)
{
  Int	n, iq;
  Byte	*p;
  char	name1[PATH_MAX], *name2;
  FILE	*fin;
  
  font = MAX(font, 3);
  if (font > 39)
    font = 3;
  if (fontptr[font] != NULL) {
    fontbase = fontptr[font];
    current_font = font;
    return 1;
  }
  /* new font, get font file name */
  {
    char *locations[] = {
      "$LUXFONTSDIR",
      "/usr/local/share/lux",
      "/usr/share/lux",
    };
    int i;
    fin = NULL;
    for (i = 0; i < sizeof(locations)/sizeof(locations[0]); ++i) {
      sprintf(name1, "%s/font%03d.hex0", locations[i], font);
      name2 = expand_name(name1, NULL);
      if (fin = fopen(name2, "r"))
        break;
    }
  }
  if (!fin) {
    if (font == 3)
      return luxerror("can't find font003 file\n", 0);
    printf("font file %s not found, default to font 3\n", name2);
    return fontchange(3);
  }
					/* read in file */
  fscanf(fin, "%d", &n);
  n = n/2;
  if ((p = (Byte *) malloc(n)) == NULL)
    return LUX_ERROR;
  fontptr[font] = p;
  while (n--) {
    fscanf(fin, "%2x", &iq);
    *p++ = (Byte) iq;
  }
  fontbase = fontptr[font];
  fclose(fin);
  current_font = font;
  return 1;
}
/*------------------------------------------------------------------------- */
