
AM_CPPFLAGS = -I@top_srcdir@

bin_PROGRAMS = colorize filecompare

COMMON_SRC = main.h bmp.h ui.c helpers.c

colorize_SOURCES = $(COMMON_SRC) colorize.c

filecompare_SOURCES = $(COMMON_SRC) filecompare.c

nice:
	rm -f *~

