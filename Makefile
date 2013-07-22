## Makefile for nntpswitch
## $Id: Makefile,v 1.58 2009-10-07 12:51:54 tommy Exp $

## Variables are ordered first by necessity
## then by a mixture of purpose and likeliness for changes

SHELL := /bin/sh
## used for tracking make's behaviour:
#OLD_SHELL := $(SHELL)
#SHELL = $(warning Making $@ $(if $^,[from $^])$(if $?,[$? newer]))$(OLD_SHELL)

DEFAULT_RULE := all
$(DEFAULT_RULE):
help-var:: ; @echo ' DEFAULT_RULE="$(value DEFAULT_RULE)"'

DISTNAME := nntpswitch
help-var:: ; @echo ' DISTNAME="$(value DISTNAME)"'

VERSION := $(shell ./version.sh)
help-var:: ; @echo ' VERSION="$(value VERSION)"'

TGZ = $(DISTNAME)-$(VERSION).tar.gz
help-var:: ; @echo ' TGZ="$(value TGZ)"'

#SUDO :=
help-var:: ; @echo ' SUDO="$(value SUDO)"'

PREFIX := /usr/local
help-var:: ; @echo ' PREFIX="$(value PREFIX)"'

SBINDIR := ${PREFIX}/sbin
help-var:: ; @echo ' SBINDIR="$(value SBINDIR)"'

ETCDIR := ${PREFIX}/etc/nntpswitch
help-var:: ; @echo ' ETCDIR="$(value ETCDIR)"'

HTMLDIR := html
help-var:: ; @echo ' HTMLDIR="$(value HTMLDIR)"'

CC := gcc
help-var:: ; @echo ' CC="$(value _CC)"'

CFLAGS := -std=gnu99 -pipe -g
CFLAGS += -O2
CFLAGS += -Wall
CFLAGS += -Wtype-limits 
#CFLAGS += -Wconversion
#CFLAGS += -Werror
CFLAGS += -I.

help-var:: ; @echo ' CFLAGS="$(value CFLAGS)"'

LDFLAGS := 
help-var:: ; @echo ' LDFLAGS="$(value LDFLAGS)"'

LDLIBS :=
help-var:: ; @echo ' LDLIBS="$(value LDLIBS)"'

CSCOPEFLAGS := -b -q
help-var:: ; @echo ' CSCOPEFLAGS="$(value CSCOPEFLAGS)"'

## Not user settable
override PROGS := nntpswitchd updategroups nsstats.pl nsstats.sh

override CPPFLAGS = -I. -DVERSION=\"SVN-$(VERSION)\"

override SRCS := \
		access.c aconfig.c active.c auth.c \
		client.c common.c config.c curl.c daemon.c hlist.c\
		log.c memory.c nntpd.c parsedate.c post.c \
		rconfig.c reader.c semaphore.c server.c setproctitle.c \
		socket.c statistics.c tgetopt.c timerproc.c uwildmat.c \
		acct_none.c acct_radius.c acct_syslog.c \
		auth_ignore.c auth_ldap.c auth_mysql.c \
		auth_passwd.c auth_pop3.c auth_radius.c \
		auth_remote.c auth_postgres.c

override UGSRCS := \
		aconfig.c common.c config.c log.c \
		memory.c server.c socket.c tgetopt.c \
		updategroups.c

override HDRS := \
		log.h memory.h nntpd.h semaphore.h \
		setproctitle.h syslognames.h tgetopt.h

override GEN_HDRS := aconfig.h aprotos.h modmap.h

override AUTH_SRCS := $(filter auth_%,$(SRCS))
override ACCT_SRCS := $(filter acct_%,$(SRCS))

override OBJS := $(SRCS:.c=.o)
override UGOBJS := $(UGSRCS:.c=.o)

override _LINK.o := $(LINK.o)
override LINK.o = @echo LINK[$(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS)] $@; $(_LINK.o)
override _CC := $(CC)
#override CC = @echo CC[$(CPPFLAGS) $(CFLAGS)] $@; $(_CC)
override CC = @echo CC $@; $(_CC)

override INSTALL := ${SUDO} install

ifneq ($(MAKE_VERSION),3.80)
override TESTDIRS := testsuite
endif

## RULES ######################################################################
## Rules are ordered first by necessity
## then by a mixture of purpose and likeliness for changes

.PHONY: all
all: ${PROGS}

.PHONY: help
help: help-usr help-dev help-var-hdr help-var help-notes

.PHONY: help-usr
help-usr:: ; @echo; echo '# Please choose one of the following:'
	@echo ' make               # compile and link all PROGS'
	@echo ' make clean         # remove intermediate compilation files'
	@echo ' make distclean     # remove all generated files'

.PHONY: help-dev
help-dev:: ; @echo; echo '# Commands for developers:'
	@echo ' make all           # compile and link all PROGS'

.PHONY: help-var help-var-hdr
help-var-hdr:
	@echo; echo '# Useful variables (use e.g. make CC=: all):'

.PHONY: help-notes
help-notes:: ; @echo; echo '# Notes on using make:'
	@echo ' # Fastest seems to be: "make -j all"'
	@echo ' # Though "make -j help" produces the lines unorderd'

-include auth/Makefile
override ALL_SRCS := $(sort $(SRCS) $(UGSRCS))
override ALL_HDRS := $(sort $(HDRS) $(GEN_HDRS))

nntpswitchd: daemon; ln -f $< $@
help-dev:: ; @echo ' make nntpswitchd   # make nntpswitchd as link to daemon'
distclean:: ; @rm -f nntpswitchd

# FIXME We statically link libjson here which we manually compiled and installed
# to the nntpswitch src dir. Unfortunately jsonc conflics with jsoncpp which is 
# used for nnd already. Both use json/json.h so this was the easiest solution.

daemon: override LDLIBS += -lmysqlclient -lldap -lpq -lradius -ljson

daemon: $(OBJS)
## daemon builds automatically because daemon.c exists
help-dev:: ; @echo ' make daemon        # compile and link all OBJS'
distclean:: ; @rm -f daemon

updategroups: $(UGOBJS)
help-dev:: ; @echo ' make updategroups  # compile and link all UGOBJS'
distclean:: ; @rm -f updategroups
    
$(OBJS) $(UGOBJS): $(ALL_DEPS) version.sh
clean:: ; @rm -f $(OBJS) $(UGOBJS)

#aprotos.h: $(ALL_SRCS); ./mkprotos.pl > $@  # Safer but remakes all
## need mkhdr x.c --> x.h and then forget about aprotos.h
aprotos.h: mkprotos.pl
	./mkprotos.pl > $@ ## Problem: change in a C file is not noticed
clean:: ; @rm -f aprotos.h

parsedate.c:
	yacc -o parsedate.c parsedate.y 2>&1
clean:: ; @rm -f parsedate.c

modmap.h: genmodmap.sh $(AUTH_SRCS) $(ACCT_SRCS)
	./$^ > $@
clean:: ; @rm -f modmap.h

nntpswitch.conf-dist aconfig.h aconfig.c: nntpswitch.conf.in mkconfig.pl
	./mkconfig.pl $< nntpswitch.conf-dist aconfig.h aconfig.c
clean:: ; @rm -f nntpswitch.conf-dist aconfig.h aconfig.c

.PHONY: clean
clean::
	@rm -f *.core
	@find . -name '.#*' -type f -print0 | xargs -0 /bin/rm -f

.PHONY: distclean
distclean:: clean 

ALL_DEPS := $(foreach src,$(ALL_SRCS),$(dir $(src)).$(notdir $(src:.c=.d)))
depend: $(ALL_DEPS)
help-dev:: ; @echo ' make depend        # creates .*.d files from *.c files'
.%.d: %.c $(GEN_HDRS)
	${CC} ${CPPFLAGS} -M -MF $@ -MQ '$(<:.c=.o)' ${CFLAGS} $<
clean:: ; @rm -f ${ALL_DEPS}

NODEP_GOALS := clean distclean test-clean test-distclean
NODEP_GOALS += help help-var help-usr help-dev help-notes
NODEP_GOALS += doc doc-html doc-pdf doc-txt
NODEP_GOALS += cscope tags
ifeq ($(strip $(filter $(NODEP_GOALS),$(MAKECMDGOALS))),)
-include $(ALL_DEPS) ## not making these if cleaning
endif
help-notes::
	@echo ' # targets "${NODEP_GOALS}" should be ran alone'
	@echo ' # e.g. "make clean all" fails'

## Miscellaneous targets ######################################################

.PHONY: dist
dist: $(TGZ)
$(TGZ): $(sort $(shell cat MANIFEST)) # $(ALL_SRCS))
	@tar zcf $@ $^ && ls -lad $@
distclean:: ; @rm -f ${TGZ}
help-usr:: ; @echo ' make dist          # create archive in .tar.gz format'

.PHONY: install
install: install-progs install-init
help-usr:: ; @echo ' make install       # makes all and installs it too'

.PHONY: install-progs
install-progs: $(addprefix $(SBINDIR)/, $(PROGS))

$(SBINDIR)/%: % ; ${INSTALL} -D -m 755 $< $@

install-init: $(ETCDIR)/init.d/nntpswitch
$(ETCDIR)/init.d/nntpswitch: etc/init.d ; ${INSTALL} -D -m 755 $< $@

tags: $(ALL_SRCS) $(ALL_HDRS); ctags ${ALL_SRCS}
help-dev:: ; @echo ' make tags          # create tags file for use in vi'
distclean:: ; @rm -f tags

.PHONY: cscope
cscope: cscope.in.out cscope.out cscope.po.out
cscope.in.out cscope.out cscope.po.out: $(ALL_SRCS) $(ALL_HDRS)
	cscope ${CSCOPEFLAGS}
help-dev:: ; @echo ' make cscope        # create cscope files for use in vi'
distclean:: ; @rm -f cscope.in.out cscope.out cscope.po.out

define run_testsuite
	@$(3) echo start=$(1)
	+@${MAKE} -C $(1) $(2) --no-print-directory
	@$(3) echo stop=$(1)

endef

.PHONY: test test+ test++ test-clean test-distclean
test test+ test++: all
	$(foreach tdir,$(TESTDIRS),$(call run_testsuite,$(tdir),$@,))
test-clean test-distclean:
	$(foreach tdir,$(TESTDIRS),$(call run_testsuite,$(tdir),$@,:))
help-dev:: ; @echo ' make test          # compiles and runs all available tests'
help-dev:: ; @echo ' make test-clean    # removes files from "make test"'
help-dev:: ; @echo ' make test+         # runs "make test-clean test"'
help-dev:: ; @echo ' make test++        # runs "make test-distclean test"'
clean:: test-clean
distclean:: test-distclean

.PHONY: start restart stop
start restart stop: install ; PREFIX=${PREFIX} ${SUDO} ./bin/nntpswitchd-control $@
help-usr:: ; @echo ' make start         # installs and starts nntpswitch'
help-usr:: ; @echo ' make restart       # installs, stops and starts nntpswitch'
help-usr:: ; @echo ' make stop          # installs and stops nntpswitch'

.PHONY: todo
todo:
	@echo replace aprotos.h with .h files
	@echo add install ownership/mode
	@echo generate MANIFEST
	@echo MANUAL\?
	@echo rename daemon.c to nntpswitchd.c so daemon target can go
	@echo hide auxiliary targets in another makefile

#
# Debian Package Targets
#

DARCH = $(shell dpkg --print-architecture)

distclean:: ; @rm -rf package nntpswitch*.deb

dpkg:		nntpswitchd updategroups
		- rm -fr package
		mkdir package
		mkdir -p package/usr/sbin package/etc/init.d package/etc/nntpswitch package/DEBIAN
		cp nntpswitchd package/usr/sbin/
		cp updategroups package/usr/sbin/
		cp nsstats.pl package/usr/sbin/
		cp etc/init.d package/etc/init.d/nntpswitch
		cp etc/overview.fmt package/etc/nntpswitch/
		cp debian/control debian/conffiles debian/postinst debian/postrm package/DEBIAN
		sed 's/#VERSION#/${VERSION}-1/g' debian/control | sed 's/#ARCH#/$(DARCH)/g' > package/DEBIAN/control
		dpkg-deb --build package nntpswitch-${VERSION}-1_${DARCH}.deb

