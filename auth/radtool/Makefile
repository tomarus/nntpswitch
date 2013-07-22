DISTNAME := radtool
PROG := radtool

PREFIX := /usr/local
LIBDIR := lib/${DISTNAME}
SBINDIR := sbin
ETCDIR := etc/${DISTNAME}
INSTALL := install

INSTUTILS := radtool

SRC := *.c
HDR := *.h

OBJDEPS := 

OBJS := \
		radtool.o \
		tgetopt.o

CC       := gcc
CFLAGS   := -pipe -g -O3 -Wall -DVERSION=\"${VERSION}\"
LIBS     := -lradius
LDFLAGS  := 

all:

.PHONY: all radtool

all: ${PROG}

## Note: these are many more dependencies than technically necessary
${OBJS} ${MODULES} ${CONDMODS} ${UGOBJS}: ${OBJDEPS}

${PROG}: LIBS += -lm -ldl
${PROG}: ${OBJS} ${CONDMODS} ${MODULES}
	${CC} ${LDFLAGS} ${CFLAGS} ${OBJS} ${LIBS} -o $@

clean:
	rm -f *.o ${PROG} > /dev/null 2>&1
