all:
	make -C src
	make -C tests/config
	make -C tests/generators

.PHONY: doc

clean:
	make -C src clean
	make -C tests/config clean
	make -C tests/generators clean

install: all
	install -D -m 755 src/pelagicontain $(DESTDIR)$(PREFIX)/bin/pelagicontain
	install -D src/lxc-pelagicontain $(DESTDIR)$(PREFIX)/share/lxc/templates/lxc-pelagicontain
	install -D src/pela_lxc.conf $(DESTDIR)/etc/pelagicontain

doc:
	doxygen Doxyfile
	cd doc/latex/ ; make

run_tests:
	tests/config/test_config
	tests/generators/test_generators
