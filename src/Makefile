PREFIX ?= /usr

all: pelagicontain

pelagicontain: pelagicontain.c pulse.h pulse.c iptables.c trafficcontrol.c
	$(CC) pelagicontain.c pulse.c iptables.c trafficcontrol.c -o pelagicontain `pkg-config --cflags --libs libpulse`

install: pelagicontain
	install -D -m 755 pelagicontain $(DESTDIR)$(PREFIX)/bin/pelagicontain
	install -D lxc-pelagicontain $(DESTDIR)$(PREFIX)/share/lxc/templates/lxc-pelagicontain
	install -D pela_lxc.conf $(DESTDIR)/etc/pelagicontain

clean:
	rm pelagicontain
