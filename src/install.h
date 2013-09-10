#define SYMBOLSTACKSIZE         10000 /* size of symbolStack[] */
#define HASHSIZE                64 /* hash table modulus value */
#define NSCRAT                  8192 /* scratch space size (bytes) */
#define NLIST                   2000 /* size of listStack[] */
#define MAXFILES		10 /* number of logical units */
#define MAXFMT			10 /* number of active levels in formats */
#if HAVE_LIBX11
#define MAXWINDOWS		20 /* number of X ports */
#define MAXPIXMAPS		20 /* number of X pixmaps */
#define MAXCOLORS		200 /* max number of default color cells */
#define MINCOLORS		64 /* minimum number of default color cells */
#define MAXMENU			20 /* max number of X menus */
#ifdef MOTIF
#define MAXWIDGETS		4000
#endif
#endif
#define MAX_DIMS		8 /* max number of array dimensions */
#define STACKSIZE		100 /* user stack size (#STACK) */
#define MAXDEBUG		20 /* max number of debugging breakpoints */
#define MAXTAPE			4 /* max number of tape drives */

#define NBREAKPOINTS		20
#define NWATCHVARS		20

#define MSSIZE			5000

/* symbol stack sizes */
#define N_NAMED                 6000 /* number of named variables */
#define N_TEMPS                 1000 /* number of temporary variables */
#define N_EXE                   24000 /* number of executables */
#define N_TEMP_EXE              1000 /* number of temp executables */
#define NSYM                    (N_NAMED + N_TEMPS + N_EXE + N_TEMP_EXE)
#define NAMED_START             0L
#define TEMPS_START             (NAMED_START + N_NAMED)
#define EXE_START               (TEMPS_START + N_TEMPS)
#define TEMP_EXE_START          (EXE_START + N_EXE)
#define NAMED_END               (NAMED_START + N_NAMED)
#define TEMPS_END               (TEMPS_START + N_TEMPS)
#define EXE_END                 (EXE_START + N_EXE)
#define TEMP_EXE_END            (TEMP_EXE_START + N_TEMP_EXE)

