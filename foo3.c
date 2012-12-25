#include <printf.h>
#include <stdio.h>

Int main(void)
{
  char *ffmts[] = {
    "%f", "%+f", "% f", "%+ f", "%-f", "%+-f", "%- f", "%+ -f",
    "%20f", "%+20f", "% 20f", "%+ 20f", "%-20f", "%+-20f", "%- 20f", "%+ -20f",
    "%.1f", "%+.1f", "% .1f", "%+ .1f", "%-.1f", "%+-.1f", "%- .1f", "%+ -.1f",
    "%20.1f", "%+20.1f", "% 20.1f", "%+ 20.1f", "%-20.1f", "%+-20.1f", "%- 20.1f", "%+ -20.1f",
    "%020.1f", "%0+20.1f", "%0 20.1f", "%0+ 20.1f", "%0-20.1f", "%0+-20.1f", "%0- 20.1f", "%0+ -20.1f",
  };
  char *tfmts[] = {
    "%T", "%+T", "% T", "%+ T", "%-T", "%+-T", "%- T", "%+ -T",
    "%20T", "%+20T", "% 20T", "%+ 20T", "%-20T", "%+-20T", "%- 20T", "%+ -20T",
    "%.1T", "%+.1T", "% .1T", "%+ .1T", "%-.1T", "%+-.1T", "%- .1T", "%+ -.1T",
    "%20.1T", "%+20.1T", "% 20.1T", "%+ 20.1T", "%-20.1T", "%+-20.1T", "%- 20.1T", "%+ -20.1T",
    "%020.1T", "%0+20.1T", "%0 20.1T", "%0+ 20.1T", "%0-20.1T", "%0+-20.1T", "%0- 20.1T", "%0+ -20.1T",
    "%.2T", "%+.2T", "% .2T", "%+ .2T", "%-.2T", "%+-.2T", "%- .2T", "%+ -.2T",
    "%20.2T", "%+20.2T", "% 20.2T", "%+ 20.2T", "%-20.2T", "%+-20.2T", "%- 20.2T", "%+ -20.2T",
    "%08.2T", "%+09.2T", "% 09.2T", "%+ 09.2T", "%-08.2T", "%+-09.2T", "%- 09.2T", "%+ -09.2T",
    "%.3T", "%+.3T", "% .3T", "%+ .3T", "%-.3T", "%+-.3T", "%- .3T", "%+ -.3T",
    "%20.3T", "%+20.3T", "% 20.3T", "%+ 20.3T", "%-20.3T", "%+-20.3T", "%- 20.3T", "%+ -20.3T",
    "%.4T", "%+.4T", "% .4T", "%+ .4T", "%-.4T", "%+-.4T", "%- .4T", "%+ -.4T",
    "%20.4T", "%+20.4T", "% 20.4T", "%+ 20.4T", "%-20.4T", "%+-20.4T", "%- 20.4T", "%+ -20.4T",
    "%#T", "%+#T", "% #T", "%+ #T", "%-#T", "%+-#T", "%- #T", "%+ -#T",
    "%#20T", "%+#20T", "% #20T", "%+ #20T", "%-#20T", "%+-#20T", "%- #20T", "%+ -#20T",
    "%#.1T", "%+#.1T", "% #.1T", "%+ #.1T", "%-#.1T", "%+-#.1T", "%- #.1T", "%+ -#.1T",
    "%#20.1T", "%+#20.1T", "% #20.1T", "%+ #20.1T", "%-#20.1T", "%+-#20.1T", "%- #20.1T", "%+ -#20.1T",
    "%#.2T", "%+#.2T", "% #.2T", "%+ #.2T", "%-#.2T", "%+-#.2T", "%- #.2T", "%+ -#.2T",
    "%#20.2T", "%+#20.2T", "% #20.2T", "%+ #20.2T", "%-#20.2T", "%+-#20.2T", "%- #20.2T", "%+ -#20.2T",
    "%#.3T", "%+#.3T", "% #.3T", "%+ #.3T", "%-#.3T", "%+-#.3T", "%- #.3T", "%+ -#.3T",
    "%#20.3T", "%+#20.3T", "% #20.3T", "%+ #20.3T", "%-#20.3T", "%+-#20.3T", "%- #20.3T", "%+ -#20.3T",
    "%#.4T", "%+#.4T", "% #.4T", "%+ #.4T", "%-#.4T", "%+-#.4T", "%- #.4T", "%+ -#.4T",
    "%#20.4T", "%+#20.4T", "% #20.4T", "%+ #20.4T", "%-#20.4T", "%+-#20.4T", "%- #20.4T", "%+ -#20.4T",
  };
  char *jfmts[] = {
    "%J", "%+J", "% J", "%+ J", "%-J", "%+-J", "%- J", "%+ -J",
    "%20J", "%+20J", "% 20J", "%+ 20J", "%-20J", "%+-20J", "%- 20J", "%+ -20J",
    "%020J", "%+020J", "% 020J", "%+ 020J", "%-020J", "%+-020J", "%- 020J", "%+ -020J",
    "%.1J", "%+.1J", "% .1J", "%+ .1J", "%-.1J", "%+-.1J", "%- .1J", "%+ -.1J",
    "%20.1J", "%+20.1J", "% 20.1J", "%+ 20.1J", "%-20.1J", "%+-20.1J", "%- 20.1J", "%+ -20.1J",
    "%020.1J", "%+020.1J", "% 020.1J", "%+ 020.1J", "%-020.1J", "%+-020.1J", "%- 020.1J", "%+ -020.1J",
    "%.2J", "%+.2J", "% .2J", "%+ .2J", "%-.2J", "%+-.2J", "%- .2J", "%+ -.2J",
    "%20.2J", "%+20.2J", "% 20.2J", "%+ 20.2J", "%-20.2J", "%+-20.2J", "%- 20.2J", "%+ -20.2J",
    "%020.2J", "%+020.2J", "% 020.2J", "%+ 020.2J", "%-020.2J", "%+-020.2J", "%- 020.2J", "%+ -020.2J",
    "%.3J", "%+.3J", "% .3J", "%+ .3J", "%-.3J", "%+-.3J", "%- .3J", "%+ -.3J",
    "%20.3J", "%+20.3J", "% 20.3J", "%+ 20.3J", "%-20.3J", "%+-20.3J", "%- 20.3J", "%+ -20.3J",
    "%020.3J", "%+020.3J", "% 020.3J", "%+ 020.3J", "%-020.3J", "%+-020.3J", "%- 020.3J", "%+ -020.3J",
    "%.4J", "%+.4J", "% .4J", "%+ .4J", "%-.4J", "%+-.4J", "%- .4J", "%+ -.4J",
    "%20.4J", "%+20.4J", "% 20.4J", "%+ 20.4J", "%-20.4J", "%+-20.4J", "%- 20.4J", "%+ -20.4J",
    "%020.4J", "%+020.4J", "% 020.4J", "%+ 020.4J", "%-020.4J", "%+-020.4J", "%- 020.4J", "%+ -020.4J",
    "%.5J", "%+.5J", "% .5J", "%+ .5J", "%-.5J", "%+-.5J", "%- .5J", "%+ -.5J",
    "%20.5J", "%+20.5J", "% 20.5J", "%+ 20.5J", "%-20.5J", "%+-20.5J", "%- 20.5J", "%+ -20.5J",
    "%020.5J", "%+020.5J", "% 020.5J", "%+ 020.5J", "%-020.5J", "%+-020.5J", "%- 020.5J", "%+ -020.5J",
    "%.6J", "%+.6J", "% .6J", "%+ .6J", "%-.6J", "%+-.6J", "%- .6J", "%+ -.6J",
    "%20.6J", "%+20.6J", "% 20.6J", "%+ 20.6J", "%-20.6J", "%+-20.6J", "%- 20.6J", "%+ -20.6J",
    "%020.6J", "%+020.6J", "% 020.6J", "%+ 020.6J", "%-020.6J", "%+-020.6J", "%- 020.6J", "%+ -020.6J",
    "%.7J", "%+.7J", "% .7J", "%+ .7J", "%-.7J", "%+-.7J", "%- .7J", "%+ -.7J",
    "%20.7J", "%+20.7J", "% 20.7J", "%+ 20.7J", "%-20.7J", "%+-20.7J", "%- 20.7J", "%+ -20.7J",
    "%020.7J", "%+020.7J", "% 020.7J", "%+ 020.7J", "%-020.7J", "%+-020.7J", "%- 020.7J", "%+ -020.7J",
  };
  Int i;

  for (i = 0; i < sizeof(ffmts)/sizeof(*ffmts); i++) {
    printf("%2d %10s |", i, ffmts[i]);
    printf(ffmts[i],0.9999883971177);
    puts("|");
  }
  for (i = 0; i < sizeof(tfmts)/sizeof(*tfmts); i++) {
    printf("%2d %10s |", i, tfmts[i]);
    printf(tfmts[i],0.9999883971177);
    puts("|");
  }
  for (i = 0; i < sizeof(jfmts)/sizeof(*jfmts); i++) {
    printf("%2d %10s |", i, jfmts[i]);
    printf(jfmts[i],2457023.4999883971177);
    puts("|");
  }
  return 0;
}
