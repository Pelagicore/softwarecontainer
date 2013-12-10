PREFIX ?= /usr

SOURCES = pelagicontain.cpp \
	  pulse.cpp \
	  iptables.c       \
	  trafficcontrol.c \
	  config.cpp \
	  generators.cpp

HEADERS = pulse.h          \
	  iptables.h       \
	  trafficcontrol.h \
	  config.h         \
	  generators.h

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
