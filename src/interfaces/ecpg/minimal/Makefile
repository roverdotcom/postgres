#-------------------------------------------------------------------------
#
# Makefile for src/interfaces/ecpg/preproc
#
# Copyright (c) 1998-2017, PostgreSQL Global Development Group
#
# src/interfaces/ecpg/preproc/Makefile
#
#-------------------------------------------------------------------------

PGFILEDESC = "Redshift syntax checker"
NAME = rscheck

subdir = src/interfaces/ecpg/minimal
top_builddir = ../../../..
include $(top_builddir)/src/Makefile.global

override CPPFLAGS := -I../include -I$(top_srcdir)/src/interfaces/ecpg/include \
	-I. -I$(srcdir) -I/Users/aviheroor/anaconda3/include/python3.6m -DECPG_COMPILE \
	 $(CPPFLAGS)
 
override LDFLAGS := -L/Users/aviheroor/anaconda3/lib/python3.6/config-3.6m-darwin \
		-Wl,-export_dynamic \
		$(LDFLAGS)

override LIBS := -lpython3.6m -ldl -framework CoreFoundation $(LIBS)


override CFLAGS += $(PTHREAD_CFLAGS)

OBJS=preproc.o pgc.o type.o output.o parser.o \
	keywords.o c_keywords.o ecpg_keywords.o ../ecpglib/typename.o descriptor.o variable.o rscheck.o\
	$(WIN32RES)

# Suppress parallel build to avoid a bug in GNU make 3.82
# (see comments in ../Makefile)
ifeq ($(MAKE_VERSION),3.82)
.NOTPARALLEL:
endif

# all: all-lib
all: rscheck

preproc.h: preproc.c ;
preproc.c: BISONFLAGS += -d

rscheck: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(LDFLAGS_EX) $^ $(LIBS) $(PTHREAD_LIBS) -o $@$(X)
# include $(top_srcdir)/src/Makefile.shlib

../ecpglib/typename.o: ../ecpglib/typename.c
	$(MAKE) -C $(dir $@) $(notdir $@)

distprep: preproc.y preproc.c preproc.h pgc.c

clean: 
	rm -f $(OBJS) rscheck$(X)
# garbage from development
	@rm -f core a.out
