/* This is file install.hh.

Copyright 2013 Louis Strous, Richard Shine
Copyright 2014 Louis Strous

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
#define SYMBOLSTACKSIZE         10000 // size of symbolStack[]
#define HASHSIZE                64 // hash table modulus value
#define NSCRAT                  8192 // scratch space size (bytes)
#define NLIST                   2000 // size of listStack[]
#define MAXFILES                10 // number of logical units
#define MAXFMT                        10 // number of active levels in formats
#define MAX_DIMS                8 // max number of array dimensions
#define STACKSIZE                100 // user stack size (#STACK)
#define MAXDEBUG                20 // max number of debugging breakpoints
#define MAXTAPE                        4 // max number of tape drives

#define NBREAKPOINTS                20
#define NWATCHVARS                20

#define MSSIZE                        5000

// symbol stack sizes
#define N_NAMED                 6000 // number of named variables
#define N_TEMPS                 1000 // number of temporary variables
#define N_EXE                   24000 // number of executables
#define N_TEMP_EXE              1000 // number of temp executables
#define NSYM                    (N_NAMED + N_TEMPS + N_EXE + N_TEMP_EXE)
#define NAMED_START             0L
#define TEMPS_START             (NAMED_START + N_NAMED)
#define EXE_START               (TEMPS_START + N_TEMPS)
#define TEMP_EXE_START          (EXE_START + N_EXE)
#define NAMED_END               (NAMED_START + N_NAMED)
#define TEMPS_END               (TEMPS_START + N_TEMPS)
#define EXE_END                 (EXE_START + N_EXE)
#define TEMP_EXE_END            (TEMP_EXE_START + N_TEMP_EXE)

