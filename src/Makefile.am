# This is file Makefile.am.
#
# Copyright 2013 Louis Strous
#
# This file is part of LUX.
#
# LUX is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your
# option) any later version.
#
# LUX is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License
# along with LUX.  If not, see <http://www.gnu.org/licenses/>.

AM_CPPFLAGS = -I$(top_srcdir)/sofa/src

noinst_LIBRARIES = libana.a

libana_a_SOURCES = anaparser.y anacon.c topology.c contour.c crunch.c decomp.c editor.c ephem.c error.c eval.c execute.c fft.c filemap.c fit.c files.c fun1.c fun2.c fun3.c fun4.c fun5.c fun6.c hersh.c ident.c install.c memck.c plots.c post.c rawio.c sort.c strous.c strous2.c strous3.c subsc.c symbols.c tense.c orientation.c output.c axis.c coord.c cluster.c ephem2.c paerror.c idl.c trace_decoder_ana.c gifread_ana.c gifwrite_ana.c astron.c terminal.c regex.c vsop.c anasofa.c intmath.c calendar.c jpeg.c tape.c dummyterm.c projection.c bindings.c xport.c zoom.c menu.c color.c random.c Bytestack.c poisson.c precession.c printf_extensions.c vsop87adata.c vsop87cdata.c matrix.c ana_func_if.c site.c action.h ana_bitmap.h ana_func_if.h anadefs.h anaparser.h ana_structures.h astrodat2.h astrodat3.h astron.h axis.h bindings.h Bytestack.h calendar.h constellations.h debug.h defs.h dmalloc.h editorcharclass.h editor.h error.h format.h hershey.h install.h intmath.h jconfig.h jinclude.h jpegdata.h once.h output.h symbols.h terminfo.h types.h vsop.h version.h

AM_YFLAGS = -d

libana_a_LIBADD = $(top_builddir)/sofa/src/libsofa_c.a

site.o: version.h

# store "git describe" output in version.h if the output has changed
version.h: FORCE
	@git describe --tags --dirty | sed 's/^\(.*\)/"\1"/' > maybe-GITID
	@diff -q -N GITID maybe-GITID || { mv maybe-GITID GITID; echo -n "#define GIT_VERSION " > version.h; cat GITID >> version.h; }
FORCE:

CLEANFILES = maybe-GITID GITID version.h

bin_PROGRAMS = ana
ana_SOURCES = ana.c
ana_LDADD = libana.a

