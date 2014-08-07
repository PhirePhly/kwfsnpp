PREFIX = /usr/local

LDLIBS = -lpthread

SRCS = kwfsnppd.c aprsis.c snpp.c util.c

OBJS = kwfsnppd.o aprsis.o snpp.o util.o

kwfsnppd: $(OBJS)
	$(CC) -o kwfsnppd $(OBJS) $(LDLIBS)

.PHONY: all install clean

all: kwfsnppd

install: kwfsnppd
	-rm -f $(PREFIX)/bin/kwfsnppd
	mkdir -p $(PREFIX)/bin
	cp -f kwfsnppd $(PREFIX)/bin

clean:
	-rm -f kwfsnppd *.o *~

