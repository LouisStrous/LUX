#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#if HAVE_REGEX_H
#include "install.h"
#include "action.h"
#include <regex.h>
#include <sys/types.h>
#include <string.h>

static int countMatches(char *text) {
  int n = 1;
  char *p = text;

  while (p = strchr(p, '(')) {
    if (p > text) {
      int n = p - text;
      int i = 1;

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

int ana_regex(int narg, int ps[]) {
  char *text, *regex;
  int result, flags, i;
  static regex_t preg;
  static regmatch_t *pmatch = NULL;
  static int nmatch = 0;

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
    regfree(&preg);		/* clean out previous one */
    result = regcomp(&preg, regex, REG_EXTENDED);
    if (result) {
      int size;
      char errbuf[256];
    
      size = regerror(result, &preg, errbuf, 256);
      return anaerror("REGEX error: %s", -1, errbuf);
    }
    nmatch = countMatches(regex);
    pmatch = realloc(pmatch, nmatch*sizeof(regmatch_t));
    if (nmatch && !pmatch) {
      nmatch = 1; /* flag: non-zero means a preg was defined */
      return cerror(ALLOC_ERR, -1);
    }
  } else if (!nmatch)
    return anaerror("No regular expression was specified earlier", -1);
  result = regexec(&preg, text, nmatch, pmatch, 0);
  if (result) 			/* no match */
    result = ANA_ZERO;
  else 
    if (nmatch > 1) {
      char **p;

      result = array_scratch(ANA_STRING_ARRAY, 1, &nmatch);
      p = (char **) array_data(result);
      for (i = 0; i < nmatch; i++) {
	if (pmatch[i].rm_so >= 0) {
	  int len;
	
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
    } else {			/* nmatch == 1 */
      char *p;
      int len;
	
      len = pmatch[0].rm_eo - pmatch[0].rm_so;
      result = string_scratch(len);
      if (result == ANA_ERROR)
	return ANA_ERROR;
      p = string_value(result);
      memcpy(p, text + pmatch[0].rm_so, len);
      p[len] = '\0';
    }
  if (!nmatch)
    nmatch = 1;
  return result;
}
#endif
