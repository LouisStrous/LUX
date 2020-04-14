/* This is file regex.cc.

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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#if HAVE_REGEX_H
#include "install.hh"
#include "action.hh"
#include <regex.h>
#include <sys/types.h>
#include <string.h>

static int32_t countMatches(char *text) {
  int32_t n = 1;
  char *p = text;

  while (p = strchr(p, '(')) {
    if (p > text) {
      int32_t n = p - text;
      int32_t i = 1;

      while (i <= n && text[n - i] == '\\')
	i++;
      if ((i % 2) == 0) {
	p++;
	continue;
      }
    }
    n++;
    p++;
  }
  return n;
}

/*

REGEX(<string> [, <regex>] [,/NOCASE]) returns 0 if the <regex> does not match
the <string>, a string array matching the indicated elements otherwise.
If no <regex> is specified, then the last one is used again.

*/

int32_t lux_regex(int32_t narg, int32_t ps[]) {
  char *text, *regex;
  int32_t result, flags, i;
  static regex_t preg;
  static regmatch_t *pmatch = NULL;
  static int32_t nmatch = 0;

  text = string_arg(ps[0]);
  if (!text)
    return cerror(NEED_STR, ps[0]);
  if (narg > 1) {
    regex = string_arg(ps[1]);
    if (!regex)
      return cerror(NEED_STR, ps[1]);
    flags = REG_EXTENDED;
    if ((internalMode & 1) == 0)
      flags |= REG_ICASE;
    regfree(&preg);		// clean out previous one
    result = regcomp(&preg, regex, REG_EXTENDED);
    if (result) {
      int32_t size;
      char errbuf[256];

      size = regerror(result, &preg, errbuf, 256);
      return luxerror("REGEX error: %s", -1, errbuf);
    }
    nmatch = countMatches(regex);
    pmatch = realloc(pmatch, nmatch*sizeof(regmatch_t));
    if (nmatch && !pmatch) {
      nmatch = 1; // flag: non-zero means a preg was defined
      return cerror(ALLOC_ERR, -1);
    }
  } else if (!nmatch)
    return luxerror("No regular expression was specified earlier", -1);
  result = regexec(&preg, text, nmatch, pmatch, 0);
  if (result) 			// no match
    result = LUX_ZERO;
  else
    if (nmatch > 1) {
      char **p;

      result = array_scratch(LUX_STRING_ARRAY, 1, &nmatch);
      p = (char **) array_data(result);
      for (i = 0; i < nmatch; i++) {
	if (pmatch[i].rm_so >= 0) {
	  int32_t len;
	
	  len = pmatch[i].rm_eo - pmatch[i].rm_so;
	  *p = malloc(len + 1);
	  if (!*p) {
	    zap(result);
	    if (!nmatch)
	      nmatch = 1;
	    return cerror(ALLOC_ERR, -1);
	  }
	  memcpy(*p, text + pmatch[i].rm_so, len);
	  (*p)[len] = '\0';
	  p++;
	}
      }
    } else {			// nmatch == 1
      char *p;
      int32_t len;
	
      len = pmatch[0].rm_eo - pmatch[0].rm_so;
      result = string_scratch(len);
      if (result == LUX_ERROR)
	return LUX_ERROR;
      p = string_value(result);
      memcpy(p, text + pmatch[0].rm_so, len);
      p[len] = '\0';
    }
  if (!nmatch)
    nmatch = 1;
  return result;
}
#endif
