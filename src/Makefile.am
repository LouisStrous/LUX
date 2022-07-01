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

#AM_CPPFLAGS = -Werror -include everywhere.hh \
#  -I$(top_srcdir)/historicephemerides
AM_CPPFLAGS = -Werror

AM_YFLAGS = -d
ACLOCAL_AMFLAGS = -I m4

nonbind_sources = \
	AstronomicalConstants.hh\
	Bytestack.cc\
	Bytestack.hh\
	Ellipsoid.cc\
	Ellipsoid.hh\
	FloatingPointAccumulator.hh\
	GnuPlot.cc\
	GnuPlot.hh\
	InstanceID.hh\
	MonotoneInterpolation.cc\
	MonotoneInterpolation.hh\
	NumericDataDescriptor.cc\
	NumericDataDescriptor.hh\
	Rotate3d.cc\
	Rotate3d.hh\
	SSFC.cc\
	SSFC.hh\
	StandardArguments.cc\
	StandardArguments.hh\
	action.hh\
	astrodat2.hh\
	astrodat3.hh\
	astron.cc\
	astron.hh\
	astron2.cc\
	axis.cc\
	axis.hh\
	bindings.cc\
	bindings.hh\
	calendar.cc\
	calendar.hh\
	cdiv.hh\
	cluster.cc\
	color.cc\
	constellations.hh\
	contour.cc\
	coord.cc\
	crunch.cc\
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
	everywhere.hh\
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
	gnuplot_interface.cc\
	gslpp_sort.cc\
	gslpp_sort.hh\
	gslpp_sort_vector.hh\
	gslpp_vector.hh\
	hersh.cc\
	hershey.hh\
	history.c\
	history.h\
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
	oiio.cc\
	once.hh\
	orientation.cc\
	output.cc\
	output.hh\
	paerror.cc\
	permutations.cc\
	permutations.hh\
	plots.cc\
	poisson.cc\
	post.cc\
	precession.cc\
	printf_extensions.cc\
	projection.cc\
	random.cc\
	readline.c\
	readline.h\
	regex.cc\
	replacements.h\
	sincos.cc\
	site.cc\
	SolarSystemEphemerides.cc\
	SolarSystemEphemerides.hh\
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
	topology.cc\
	trace_decoder_lux.cc\
	types.hh\
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
lux_LDADD = liblux.a

#lux$(EXEEXT): $(top_builddir)/historicephemerides/libhistoricephemerides_c.a

luxparser.cc: luxparser.yy

thebindings.cc: $(nonbind_sources) bindings.hh bindings.pl
	perl bindings.pl

# build library, too; makes building test binaries easier
noinst_LIBRARIES = liblux.a
liblux_a_SOURCES = $(nonbind_sources) thebindings.cc

debug-local:
	$(MAKE) CPPFLAGS=-g CFLAGS= CXXFLAGS= all
