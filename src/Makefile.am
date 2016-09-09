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

nonmain_sources = \
	AstronomicalConstants.hh\
	Bytestack.cc\
	Bytestack.hh\
	Ellipsoid.cc\
	Ellipsoid.hh\
	MonotoneInterpolation.cc\
	MonotoneInterpolation.hh\
	Rotate3d.cc\
	Rotate3d.hh\
	SSFC.cc\
	SSFC.hh\
	action.hh\
	astrodat2.hh\
	astrodat3.hh\
	astron.cc\
	astron.hh\
	axis.cc\
	axis.hh\
	bindings.cc\
	bindings.hh\
	calendar.cc\
	calendar.hh\
	cluster.cc\
	color.cc\
	constellations.hh\
	contour.cc\
	coord.cc\
	crunch.cc\
	debug.hh\
	decomp.cc\
	dmalloc.hh\
	editor.cc\
	editor.hh\
	editorcharclass.hh\
	ephem.cc\
	ephem2.cc\
	error.cc\
	error.hh\
	eval.cc\
	execute.cc\
	fft.cc\
	filemap.cc\
	files.cc\
	fit.cc\
	format.hh\
	fun1.cc\
	fun2.cc\
	fun3.cc\
	fun4.cc\
	fun5.cc\
	fun6.cc\
	gifread_lux.cc\
	gifwrite_lux.cc\
	hersh.cc\
	hershey.hh\
	ident.cc\
	idl.cc\
	install.cc\
	install.hh\
	intmath.cc\
	intmath.hh\
	jconfig.h\
	jinclude.h\
	jpeg.cc\
	jpegdata.h\
	lux_bitmap.xbm\
	lux_func_if.cc\
	lux_func_if.hh\
	lux_structures.hh\
	luxcon.cc\
	luxdefs.hh\
	luxparser.hh\
	luxparser.yy\
	luxsofa.cc\
	matrix.cc\
	memck.cc\
	menu.cc\
	once.hh\
	orientation.cc\
	output.cc\
	output.hh\
	paerror.cc\
	plots.cc\
	poisson.cc\
	post.cc\
	precession.cc\
	printf_extensions.cc\
	projection.cc\
	random.cc\
	regex.cc\
	replacements.h\
	sincos.cc\
	site.cc\
	sort.cc\
	strous.cc\
	strous2.cc\
	strous3.cc\
	subsc.cc\
	symbols.cc\
	symbols.hh\
	tape.cc\
	tense.cc\
	terminal.cc\
	terminfo.cc\
	terminfo.hh\
	thebindings.cc\
	topology.cc\
	trace_decoder_lux.cc\
	visualclass.c\
	visualclass.h\
	vsop.cc\
	vsop.hh\
	vsop87adata.cc\
	vsop87cdata.cc\
	xport.cc\
	zoom.cc

bin_PROGRAMS = lux
lux_SOURCES = lux.cc
lux_LDADD = liblux.a $(top_builddir)/sofa/src/libsofa_c.a

thebindings.cc: $(nonmain_sources) bindings.hh bindings.pl
	perl bindings.pl

# build library, too; makes building test binaries easier
noinst_LIBRARIES = liblux.a
liblux_a_SOURCES = $(nonmain_sources)
#liblux_a_LIBADD = $(top_builddir)/sofa/src/libsofa_c.a

debug:
	$(MAKE) CPPFLAGS=-g CFLAGS= CXXFLAGS=

