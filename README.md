This package includes the Pelagicontain component

Building
--------
To build the project documentation set -DENABLE_DOC=1 when running cmake.

The project Doxygen docs contain info about Pelagicontain in general,
how to run Pelagicontain and how to run the component tests.

To run the unit-tests build the project with -DENABLE_COVERAGE=1 -DBUILD_TESTS=ON

To disable support for various gateways at compile time, set
* -DENABLE_PULSEGATEWAY=OFF (for pulse)
* -DENABLE_NETGATEWAY=OFF (for network)
* -DENABLE_DEVICENODEGATEWAY=OFF (for device node gateway)
* -DENABLE_DBUSGATEWAY=OFF (for dbus)
* -DENABLE_CGROUPSGATEWAY=OFF (for cgroups)

Running
-------
You normally only want to run the pelagicontain-agent, which requires root
privileges. It will register itself onto the system bus, so no dbus magic is
needed. Run it with `--help` to see runtime options.

Note on PulseAudio
------------------

In order to pulseaudio to work inside the container it needs to be running when
the container is started. This is the case because when pelagicontainer sets
up the container it will create a new socket to the pulseaudio server.
