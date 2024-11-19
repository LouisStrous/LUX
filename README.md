**NOTICE**: I've decided to stop development of LUX.  I've worked on
LUX and its predecessor ANA, of which LUX is a fork, since 1990, 34
years ago as I write this.  I intend to switch to using Python and
to port useful pieces of LUX functionality to Python modules.  That
way, more people can benefit from my work, and I can benefit from
many other people's work.  It's been fun.

The LUX website (lux.quae.nl) will eventually disappear.

                                          Louis Strous, 19-11-2024


1 LUX 
======

LUX is a program for (numerical) data processing and image processing.
See http://lux.quae.nl for its packaged source code distribution and
for the on-line LUX manual.  See the README and INSTALL files for
additional information.

2 Third Party Software used in LUX 
===================================

LUX includes the third party software packages mentioned below.

2.1 FFTPACK 
------------

The LUX source code includes some Fast Fourier Transform routines
based on FFTPACK version 4 by Paul N. Swarztrauber (April 1985),
copied from the Netlib CDROM (2nd edition), translated from Fortran to
C using f2c, with manual cleanup by Louis Strous (15 June 1998).

LUX also uses (through linking, not in the form of source code) Fast
Fourier Transform routines from the GNU Scientific Library (GSL),
which routines are themselves also based on FFTPACK.  The direct use
of FFTPACK source code is being phased out, in favor of the routines
from GSL.

2.2 SOFA 
---------

LUX includes source code from the "Standards of Fundamental Astronomy
(SOFA)" Library for ANSI C, obtained from
http://www.iausofa.org/index.html.  LUX uses routines and computations
derived by the author of LUX from software provided by SOFA under
license to the author of LUX.  LUX does not itself constitute software
provided by and/or endorsed by SOFA.

The SOFA library comes complete with its own "makefile", which is not
suitable for including in an autoconfiscated project such as LUX.  The
SOFA sources were adapted as follows for inclusion into the current
project:

1. Change into the "sofa" subdirectory of the current project's main
directory:

      cd sofa

2. Extract the sofa sources from the tarball:

      tar xzf Location_of_Tarball/sofa_c-20120301_a.tar.gz

This yields a subdirectory "sofa" in the current directory, which is
itself also called "sofa" (i.e., the new subdirectory is "sofa/sofa"
relative to the project's main directory).

3. Locate the "c" subdirectory in the extracted sources (it is several
directory levels down) and copy its contents into the current
directory:

      cp -r sofa/20120301_a/c/* .

4. Remove the extracted directory, as it is not needed anymore:

      rm -rf sofa

5. Rename SOFA's original "makefile" so it is not run and causes no
trouble:

      mv src/makefile src/original-makefile

6. Create our own "src/Makefile.am" that builds "libsofa_c.a" from the
SOFA *.c and *.h files in the "src" subdirectory.

Many SOFA routines are bound to LUX subroutines or functions.  The
names of the LUX subroutines or functions bound to SOFA routines are
the same as the names of the SOFA routines, but with the "iau" prefix
removed to comply with the SOFA license.


