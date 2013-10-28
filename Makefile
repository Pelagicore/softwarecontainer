all:
	cd src ; make

install: all
	install -D -m 755 src/pelagicontain $(DESTDIR)$(PREFIX)/bin/pelagicontain
	install -D src/lxc-pelagicontain $(DESTDIR)$(PREFIX)/share/lxc/templates/lxc-pelagicontain
	install -D src/pela_lxc.conf $(DESTDIR)/etc/pelagicontain
