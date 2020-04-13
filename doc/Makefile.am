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

info_TEXINFOS = lux.texi

# makeinfo 5.1 has a bug in that it does not accept arguments greater
# than 9 for @enumerate, but lux.texi uses such enumerations.  Until
# that bug has been fixed, we ignore the error messages.
MAKEINFOFLAGS = --force --css-ref=lux.css
EXTRA_DIST = lux.css

if HAVE_DOXYGEN

EXTRA_DIST += Doxyfile

all-local: doxyfile.stamp

doxyfile.stamp: Doxyfile $(top_builddir)/src/liblux.a
	$(DOXYGEN) Doxyfile
	touch doxyfile.stamp

CLEANFILES = doxyfile.stamp

clean-local:
	rm -rf $(top_builddir)/doc/lux.doxy doxyfile.stamp

else

all-local:
	@echo No doxygen, so no output

endif
