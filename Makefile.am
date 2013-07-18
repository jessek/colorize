SUBDIRS = src

nice:
	rm -f *~

preflight:
	@grep RBF ${SUBDIRS}/*.c ${SUBDIRS}/*.h ${SUBDIRS}/*.1 \
		  AUTHORS README.md NEWS TODO ChangeLog

windows:
	make distclean
	./configure --host=i686-w64-mingw32
	make
	rm -rf $(distdir)
	mkdir -p $(distdir)
	cp src/*.exe $(distdir)
	i686-w64-mingw32-strip $(distdir)/*.exe
	zip -r9 $(distdir).zip $(distdir)
	rm -rf $(distdir)

