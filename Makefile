PREFIX ?= /usr

all: pelagicontain test_app

pelagicontain: pelagicontain.c pulse.h pulse.c
	$(CC) pelagicontain.c pulse.c -o pelagicontain `pkg-config --cflags --libs libpulse`

test_app: test_app.c
	$(CC) test_app.c -o test_app `pkg-config --cflags --libs dbus-glib-1`

install: pelagicontain test_app
	install -D -m 755 pelagicontain $(DESTDIR)$(PREFIX)/bin/pelagicontain
	install -D -m 755 test_app $(DESTDIR)$(PREFIX)/bin/pelagicontain_test_app
	install -D lxc-pelagicontain $(DESTDIR)$(PREFIX)/share/lxc/templates/lxc-pelagicontain
	install -D pela_lxc.conf $(DESTDIR)/etc/pelagicontain

clean:
	rm pelagicontain test_app
