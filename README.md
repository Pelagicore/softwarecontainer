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

Examples are built using -DENABLE_EXAMPLES=1

For a concrete example of building Pelagicontain and setting up dependencies,
see Vagrantfile in this repository.

Building in Vagrant
-------------------

Vagrant can be used to quickly set up a virtualized environment for building.
On a debian-based system, issue the following commands to build using Vagrant:

```
ssh-keygen -f vagrant_key

# MANUAL STEP: Add vagrant_key.pub to GitLab in order to access sources

sudo apt install vagrant
vagrant up
```

The vagrant machine can then be inspected by running `vagrant ssh`

Dependencies
------------
- pelagicore-utils - Known good commit: aeb2a9bd52aff15c1a7ecae6a39e1aa271758e06
- ivi-logging - Known good commit: ea313b78b23c2c79bfeee7329131a804b14965c8
- ivi-mainloop - Known good commit: 558fbd49e874eef9a84a7d00a8d1a6dc9dc93cb2
- jsonparser - Known good commit: 7179e326e5f9137faeb11852b0e640b31a3987d5
- git
- cmake
- build-essential (on Debian-based systems)
- pkg-config
- libglib2.0-dev
- libdbus-c++-dev
- libdbus-c++-1-0v5
- libdbus-1-dev
- libglibmm-2.4-dev
- libglibmm-2.4
- lxc-dev
- libpulse-dev
- unzip
- libjansson-dev
- libjansson4


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
