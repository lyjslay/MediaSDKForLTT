# Generated automatically from Makefile.in by configure.
# Makefile.in for thttpd
#
# Copyright � 1995,1998 by Jef Poskanzer <jef@mail.acme.com>.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.

# Various configurable paths (remember to edit Makefile.in, not Makefile)

CUR_DIR      := $(shell pwd)
# Top level hierarchy.
prefix = /usr/local
exec_prefix = ${prefix}
# Pathname of directory to install the binary.
BINDIR = ${exec_prefix}/sbin
# Pathname of directory to install the man page.
MANDIR = ${prefix}/man
# Pathname of directory to install the CGI programs.
WEBDIR = $(prefix)/www

# CONFIGURE: The group that the web directory belongs to.  This is so that
# the makeweb program can be installed set-group-id to that group, and make
# subdirectories.  If you're not going to use makeweb, ignore this.
WEBGROUP =	www

# CONFIGURE: Directory for CGI executables.
CGIBINDIR =	$(WEBDIR)/cgi-bin

# You shouldn't need to edit anything below here.

CC =		$(CROSS_COMPILE_3RD)gcc
CCOPT =		-O2 -g
DEFS =		 -DHAVE__PROGNAME=1 -DHAVE_FCNTL_H=1 -DHAVE_GRP_H=1 -DHAVE_MEMORY_H=1 -DHAVE_PATHS_H=1 -DHAVE_POLL_H=1 -DHAVE_SYS_POLL_H=1 -DTIME_WITH_SYS_TIME=1 -DHAVE_DIRENT_H=1 -DHAVE_LIBCRYPT=1 -DHAVE_STRERROR=1 -DHAVE_WAITPID=1 -DHAVE_VSNPRINTF=1 -DHAVE_DAEMON=1 -DHAVE_SETSID=1 -DHAVE_GETADDRINFO=1 -DHAVE_GETNAMEINFO=1 -DHAVE_GAI_STRERROR=1 -DHAVE_SIGSET=1 -DHAVE_ATOLL=1 -DHAVE_UNISTD_H=1 -DHAVE_GETPAGESIZE=1 -DHAVE_MMAP=1 -DHAVE_SELECT=1 -DHAVE_POLL=1 -DHAVE_TM_GMTOFF=1 -DHAVE_INT64T=1 -DHAVE_SOCKLENT=1
INCLS =		-I.
CFLAGS +=	$(CCOPT) $(DEFS) $(INCLS) -Wno-error=unused-but-set-variable -fPIC
LDFLAGS =
LIBS =		-lcrypt
NETLIBS =
#INSTALL =	/usr/bin/install -c
LIB_DIR     := $(CUR_DIR)/lib

.c.o:
	@rm -f $@
	$(CC) $(CFLAGS) -c $*.c

SRC =		thttpd.c libhttpd.c fdwatch.c mmc.c timers.c match.c tdate_parse.c

OBJ =		$(SRC:.c=.o)

TARGET_A       = $(LIB_DIR)/libthttpd.a
TARGET_SO    = $(LIB_DIR)/libthttpd.so

ALL =		$(TARGET_A) $(TARGET_SO)

GENHDR =	include/mime_encodings.h include/mime_types.h

CLEANFILES =	$(ALL) $(OBJ) $(GENSRC) $(GENHDR) $(TARGET_A) $(TARGET_SO)

SUBDIRS =	cgi-src extras

LDFLAGS_SO   = -shared


all:		$(ALL)

$(TARGET_A): $(OBJ)
	@rm -f $@
	mkdir -p $(CUR_DIR)/lib
#	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJ) $(LIBS) $(NETLIBS)
	$(AR) -rv $(TARGET_A) $(OBJ)

$(TARGET_SO) : $(OBJ)
# 	$(CC) $(LDFLAGS_SO) -o $(OBJ)
	$(CC) $(LDFLAGS_SO) $(LIBS) -o $@ $^

mime_encodings.h:	mime_encodings.txt
	rm -f mime_encodings.h
	sed < mime_encodings.txt > mime_encodings.h \
	  -e 's/#.*//' -e 's/[ 	]*$$//' -e '/^$$/d' \
	  -e 's/[ 	][ 	]*/", 0, "/' -e 's/^/{ "/' -e 's/$$/", 0 },/'

mime_types.h:	mime_types.txt
	rm -f mime_types.h
	sed < mime_types.txt > mime_types.h \
	  -e 's/#.*//' -e 's/[ 	]*$$//' -e '/^$$/d' \
	  -e 's/[ 	][ 	]*/", 0, "/' -e 's/^/{ "/' -e 's/$$/", 0 },/'


subdirs:
	for i in $(SUBDIRS) ; do ( \
	    cd $$i ; \
	    pwd ; \
	    $(MAKE) $(MFLAGS) \
		WEBDIR=$(WEBDIR) \
		CGIBINDIR=$(CGIBINDIR) \
		MANDIR=$(MANDIR) \
		WEBGROUP=$(WEBGROUP) \
	) ; done

clean:
	rm -f $(OBJ) $(TARGET_A) $(TARGET_SO)


install:

thttpd.o:	include/config.h include/version.h include/libhttpd.h include/fdwatch.h include/mmc.h include/timers.h include/match.h
libhttpd.o:	include/config.h include/version.h include/libhttpd.h include/mime_encodings.h include/mime_types.h \
		include/mmc.h include/timers.h include/match.h include/tdate_parse.h
fdwatch.o:	include/fdwatch.h
mmc.o:		include/mmc.h include/libhttpd.h
timers.o:	include/timers.h
match.o:	include/match.h
tdate_parse.o:	include/tdate_parse.h
