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

SUBDIRS = fonts

AM_CPPFLAGS = -I$(top_srcdir)/sofa/src
AM_YFLAGS = -d
ACLOCAL_AMFLAGS = -I m4

nonmain_sources = luxparser.yy luxcon.cc topology.cc contour.cc crunch.cc decomp.cc editor.cc ephem.cc error.cc eval.cc execute.cc fft.cc filemap.cc fit.cc files.cc fun1.cc fun2.cc fun3.cc fun4.cc fun5.cc fun6.cc hersh.cc ident.cc install.cc memck.cc plots.cc post.cc sort.cc strous.cc strous2.cc strous3.cc subsc.cc symbols.cc tense.cc orientation.cc output.cc axis.cc coord.cc cluster.cc ephem2.cc paerror.cc idl.cc trace_decoder_lux.cc gifread_lux.cc gifwrite_lux.cc astron.cc terminal.cc regex.cc vsop.cc luxsofa.cc intmath.cc calendar.cc jpeg.cc tape.cc terminfo.cc projection.cc bindings.cc xport.cc zoom.cc menu.cc color.cc random.cc Bytestack.cc poisson.cc precession.cc printf_extensions.cc vsop87adata.cc vsop87cdata.cc matrix.cc lux_func_if.cc site.cc action.hh lux_bitmap.xbm lux_func_if.hh luxdefs.hh luxparser.hh lux_structures.hh astrodat2.hh astrodat3.hh astron.hh axis.hh bindings.hh Bytestack.hh calendar.hh constellations.hh debug.hh defs.hh dmalloc.hh editorcharclass.hh editor.hh error.hh format.hh hershey.hh install.hh intmath.hh jconfig.h jinclude.h jpegdata.hh once.hh output.hh symbols.hh terminfo.hh vsop.hh version.hh sincos.cc visualclass.c visualclass.h

bin_PROGRAMS = lux
lux_SOURCES = lux.cc
lux_LDADD = $(top_builddir)/sofa/src/libsofa_c.a liblux.a

# build library, too; makes building test binaries easier
noinst_LIBRARIES = liblux.a
liblux_a_SOURCES = $(nonmain_sources)
liblux_a_LIBADD = $(top_builddir)/sofa/src/libsofa_c.a

site.o: version.hh

# store "git describe" output in version.hh if the output has changed
version.hh: FORCE
	@git describe --tags --dirty | sed 's/^\(.*\)/"\1"/' > maybe-GITID
	@diff -q -N GITID maybe-GITID && rm maybe-GITID || { mv maybe-GITID GITID; echo -n "#define GIT_VERSION " > version.hh; cat GITID >> version.hh; }
FORCE:

CLEANFILES = maybe-GITID GITID version.hh
