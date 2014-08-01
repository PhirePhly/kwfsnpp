PREFIX = /usr/local

LDLIBS = -lpthread

SRCS = kwfsnpp.c

OBJS = kwfsnpp.o

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

