#
# Makefile for current systems (xmkmf/imake are usually not available anymore)
#

# directory of the sound samples, default is the build directory
SOUNDDIR ?= $(CURDIR)

CC ?= cc
CFLAGS ?= -O2
CFLAGS += -std=gnu89 -fcommon -DSOUNDDIR=\"$(SOUNDDIR)\"
LDLIBS = -lX11 -lm

OBJS1=	main.o bfield.o castles.o input.o initx.o oninit.o ongame.o random.o \
        sleepms.o net.o sound.o

all: bb

bb: $(OBJS1)
	$(CC) $(LDFLAGS) -o $@ $(OBJS1) $(LDLIBS)

$(OBJS1): bb.h

clean:
	rm -f bb $(OBJS1)
