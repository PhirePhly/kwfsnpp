PREFIX = /usr/local

LDLIBS = -lpthread

SRCS = kwfsnppd.c aprsis.c snpp.c util.c
HEADERS = kwfsnppd.h snpp.h util.h usagetext.h

OBJS = $(SRCS:.c=.o)

kwfsnppd: $(OBJS)

$(OBJS): $(HEADERS)

.PHONY: all install clean

all: kwfsnppd

install: kwfsnppd
	-rm -f $(PREFIX)/bin/kwfsnppd
	mkdir -p $(PREFIX)/bin
	cp -f kwfsnppd $(PREFIX)/bin

clean:
	-rm -f kwfsnppd usagetext.h *.o *~

usagetext.h: usagetext.in
	sed -e 's/\\/\\\\/g' \
		-e 's/"/\\"/g' \
		-e 's/.*/"&",/' <usagetext.in >usagetext.h

