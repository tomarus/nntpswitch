## Makefile fragment for nntpswitch for News Service Europe
## $Id: Makefile,v 1.32 2011-06-02 17:50:19 tommy Exp $

override AUTH_AUTH_SRCS := \
		auth_http.c \
		auth_json.c \
		auth_raddaylim.c

override AUTH_ACCT_SRCS := \
		acct_daylimit.c \
		acct_http.c \
		acct_json.c \
		acl_json.c \
		acct_md5syslog.c \
		acct_nssyslog.c \
		acct_raddaylim.c \
		acct_radns.c


override AUTH_MISC_SRCS := \
		config.c \
		daylimits.c \
		timelimit.c \
		log_acct.c \
		crypto.c \
		nnd_http_auth.c \
		prefixmap.c \
		ssl_socket.c

override AUTH_MISC_HDRS := \
		log_acct.h \
		crypto.h

override AUTH_UGSRCS := \
		ssl_socket.c

override AUTHAUTH_SRCS := $(AUTH_AUTH_SRCS) $(AUTH_ACCT_SRCS) $(AUTH_MISC_SRCS)
override AUTHAUTH_HDRS := $(AUTH_MISC_HDRS)

override AUTHAUTH_OBJS := $(AUTHAUTH_SRCS:.c=.o)
override AUTHAUTH_UGOBJS := $(AUTH_UGSRCS:.c=.o)

override SRCS += $(addprefix auth/, $(AUTHAUTH_SRCS))
override HDRS += $(addprefix auth/, $(AUTHAUTH_HDRS))
override OBJS += $(addprefix auth/, $(AUTHAUTH_OBJS))
override UGSRCS += $(addprefix auth/, $(AUTHAUTH_UGSRCS))
override UGOBJS += $(addprefix auth/, $(AUTHAUTH_UGOBJS))

override AUTH_SRCS += $(addprefix auth/, $(AUTH_AUTH_SRCS))
override ACCT_SRCS += $(addprefix auth/, $(AUTH_ACCT_SRCS))

#ifneq ($(MAKE_VERSION),3.80)
#override TESTDIRS += auth/testsuite
#endif

override CPPFLAGS := -I auth $(CPPFLAGS)
CSCOPEFLAGS += -sauth

## RULES ######################################################################
all:

daemon: override LDLIBS += -lcrypto -lmcrypt -lssl -lcurl

updategroups: override LDLIBS += -lssl

ifeq ($(MAKELEVEL),0)
override SUBFLAGS := --no-print-directory -C .. 

TARGETS := all clean cscope depend distclean help tags
TARGETS += test test+ test++ test-clean test-distclean
define TARGET_template
.PHONY: $(1) auth_$(1)
$(1):$(if $(filter clean distclean,$(1)),:) auth_$(1)
auth_$(1): ; +@[ -d auth ] || ${MAKE} ${SUBFLAGS} $(1)
endef

endif
$(foreach target,$(TARGETS),$(eval $(call TARGET_template,$(target))))
