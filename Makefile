PREFIX ?= /usr

all: pelagicontain

pelagicontain: pelagicontain.c
	$(CC) pelagicontain.c -o pelagicontain

install: pelagicontain
	install -D -m 755 pelagicontain $(DESTDIR)$(PREFIX)/bin/pelagicontain
	install -D lxc-pelagicontain $(DESTDIR)$(PREFIX)/share/lxc/templates/lxc-pelagicontain
	install -D pela_lxc.conf $(DESTDIR)/etc/pelagicontain
