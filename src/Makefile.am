bin_PROGRAMS = ana
ana_SOURCES = anaparser.y anacon.c topology.c contour.c crunch.c decomp.c editor.c ephem.c error.c eval.c execute.c fft.c filemap.c fit.c files.c fun1.c fun2.c fun3.c fun4.c fun5.c fun6.c hersh.c ident.c install.c memck.c plots.c post.c rawio.c sort.c strous.c strous2.c strous3.c subsc.c symbols.c tense.c orientation.c rcsversion.c output.c axis.c coord.c cluster.c ephem2.c paerror.c idl.c trace_decoder_ana.c gifread_ana.c gifwrite_ana.c astron.c terminal.c regex.c vsop.c anasofa.c intmath.c calendar.c jpeg.c tape.c dummyterm.c projection.c bindings.c xport.c zoom.c menu.c color.c random.c Bytestack.c poisson.c precession.c printf_extensions.c vsop87adata.c vsop87cdata.c matrix.c ana_func_if.c site.c action.h ana_bitmap.h ana_func_if.h anaparser.h ana_structures.h astrodat2.h astrodat3.h astron.h bindings.h Bytestack.h calendar.h constellations.h debug.h defs.h dmalloc.h editorcharclass.h editor.h format.h hershey.h install.h intmath.h jconfig.h jinclude.h jpegdata.h once.h output.h sofa.h sofam.h symbols.h terminfo.h types.h util.h vsop.h

SOFADIR = sofa/20101201/c/src
ana_LDADD = -lm -lc -ljpeg -lX11 -lgsl $(SOFADIR)/libsofa_c.a

$(SOFADIR)/libsofa_c.a:
	cd $(SOFADIR) && $(MAKE)
