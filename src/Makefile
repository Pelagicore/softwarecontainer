PREFIX ?= /usr

SOURCES = \
	config.cpp \
	container.cpp \
	dbusproxy.cpp \
	generators.cpp \
	iptables.cpp \
	pelagicontain.cpp \
	pulse.cpp \
	trafficcontrol.c

HEADERS = \
	config.h \
	container.h \
	dbusproxy.h \
	generators.h \
	iptables.h \
	pelagicontain_common.h \
	pulse.h \
	trafficcontrol.h

CXXFLAGS += -Wall
CXXFLAGS += `pkg-config --cflags --libs libpulse jansson`

all: pelagicontain

pelagicontain: $(SOURCES) $(HEADERS)
	$(CXX) $(SOURCES) -o pelagicontain $(CXXFLAGS)

install: pelagicontain
	install -D -m 755 pelagicontain $(DESTDIR)$(PREFIX)/bin/pelagicontain
	install -D lxc-pelagicontain $(DESTDIR)$(PREFIX)/share/lxc/templates/lxc-pelagicontain
	install -D pela_lxc.conf $(DESTDIR)/etc/pelagicontain

clean:
	rm pelagicontain
