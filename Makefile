# Note: anaparser.o must be first in the OBS list.  No prerequisites
# can be specified for suffix rules (such as .c.o).

OBS=anaparser.o anacon.o topology.o contour.o crunch.o decomp.o		\
editor.o ephem.o error.o eval.o execute.o fft.o filemap.o fit.o		\
files.o fun1.o fun2.o fun3.o fun4.o fun5.o fun6.o hersh.o ident.o	\
install.o memck.o plots.o post.o rawio.o sort.o strous.o strous2.o	\
strous3.o subsc.o symbols.o tense.o orientation.o rcsversion.o		\
output.o axis.o coord.o cluster.o ephem2.o paerror.o	\
idl.o trace_decoder_ana.o gifread_ana.o gifwrite_ana.o astron.o		\
terminal.o regex.o vsop.o anasofa.o intmath.o calendar.o \
jpeg.o tape.o dummyterm.o projection.o bindings.o \
xport.o zoom.o menu.o color.o random.o Bytestack.o poisson.o \

CC=gcc
LIBS=-lm -lc -ljpeg -lX11 -lgsl libsofa_c.a
EXEC=ana
MORELIBS=
LDFLAGS= $(MORELIBS)
MOREINCLDIRS=.

ana: sofam.h $(OBS) libsofa_c.a sofa.h
	./updatelevel
	rm -f site.o
	$(CC) $(CPPFLAGS) $(CFLAGS) -DHAVE_CONFIG_H -c site.c
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBS) site.o $(LIBS) -o $(EXEC)
anaparser.c.tab.c anaparser.c.tab.h: anaparser.c
	bison --defines=anaparser.c.tab.h --output-file=anaparser.c.tab.c anaparser.c
anaparser.o: anaparser.h anaparser.c.tab.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -DHAVE_CONFIG_H -c -o anaparser.o anaparser.c.tab.c
calculator.o: anaparser.h calculator.c
	bison -p calc calculator.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -DHAVE_CONFIG_H -c -o calculator.o calculator.c.tab.c
.c.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) -DHAVE_CONFIG_H -c $<

libsofa_c.a sofa.h sofam.h:
	$(MAKE) -C sofa/20101201/c/src && $(MAKE) -C sofa/20101201/c/src test

site:
	rm -f site.o
	$(CC) $(CPPFLAGS) $(CFLAGS) -DHAVE_CONFIG_H -c site.c
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBS) site.o $(LIBS) -o $(EXEC)
opt:
	$(MAKE) CFLAGS="-O $(CFLAGS)" ana
debug:
	$(MAKE) CFLAGS="-g $(CFLAGS)" EXEC=anap ana
keytest: rawio.o
	$(CC) $(CPPFLAGS) $(CFLAGS) -c keytest.c
	$(CC) $(CFLAGS) $(LDFLAGS) keytest.o rawio.o $(LIBS) -o keytest
install:
	./install-sh
uninstall:
	./uninstall-sh
clean:
	rm -f *.o $(EXEC)
distclean:
	rm -f *.o $(EXEC) config.cache config.log Makefile

tarball:
	tar czf ana.tgz *.c *.h config.guess config.sub install-sh *.in \
configure.notes
