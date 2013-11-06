PREFIX ?= /usr

SOURCES = pelagicontain.c  \
	  pulse.c          \
	  iptables.c       \
	  trafficcontrol.c \
	  config.c         \
	  generators.c

HEADERS = pulse.h          \
	  iptables.h       \
	  trafficcontrol.h \
	  config.h         \
	  generators.h

PULSEFLAGS   = `pkg-config --cflags --libs libpulse`
JANSSONFLAGS = `pkg-config --cflags --libs jansson`

all: pelagicontain

pelagicontain: $(SOURCES) $(HEADERS)
	$(CC) $(SOURCES) -o pelagicontain $(CFLAGS) $(PULSEFLAGS)  \
	                                            $(JANSSONFLAGS)

install: pelagicontain
	install -D -m 755 pelagicontain $(DESTDIR)$(PREFIX)/bin/pelagicontain
	install -D lxc-pelagicontain $(DESTDIR)$(PREFIX)/share/lxc/templates/lxc-pelagicontain
	install -D pela_lxc.conf $(DESTDIR)/etc/pelagicontain

clean:
	rm pelagicontain
