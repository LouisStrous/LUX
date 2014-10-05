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

TESTS = main vsop
check_PROGRAMS = main vsop
main_SOURCES = main.cc check-axis.cc check-binop.cc
AM_CXXFLAGS = -I$(top_srcdir)/src $(CPPUNIT_CFLAGS)
main_LDADD = $(top_builddir)/src/liblux.a -lm -lc -ljpeg -lX11 -lgsl $(CPPUNIT_LIBS)

vsop_SOURCES = vsop87test.cc vsop87test.txt
vsop_LDADD = $(top_builddir)/src/liblux.a -lm -lc
