# SoftwareContainer

The SoftwareContainer is a framework to manage and contain applications
created and vetted by third party developers in an automotive setting. A
launcher (a UI for example) sends a signal to the SoftwareContainer to start a
new Container. It can then configure the Container using a documented JSON
format and can launch process inside the Container. This can then be used to
contain applications from eachother and make sure they are not interfering with
eachother and to limit/manage resources available to the applications.

Each container is configured using gateways, where each gateway has a scope of
influence (network, dbus, files, pulseaudio, for example). The gateways get a
configuration snippet from the Launcher which they will enforce.

SoftwareContainer is maintained at https://github.com/Pelagicore/softwarecontainer.
The documentation is avaliable at https://pelagicore.github.io/softwarecontainer/.
Pelagicore maintainer: Tobias Olausson <tobias.olausson@pelagicore.com>.

## Sub-components
SoftwareContainer is composed of the following components:
* a `libsoftwarecontainer` library containing the interfacing code to LXC and
  the gateway code. This library handles one container and all the operations
  on it.
* `softwarecontainer-agent`, a D-Bus service that handles multiple instances of
  `libsoftwarecontainer`, so that one can handle several containers from a common entrypoint.
* a `common` library that contains code shared between `libsoftwarecontainer` and the
  `softwarecontainer-agent`, such as utility functions etc.

# Dependencies

## Build tools
- cmake
- pkg-config
- build-essential (on Debian-based systems)
- unzip
- git - optional, if you want to build using vagrant
- vagrant-cookbook - optional, if you want to build using vagrant
- sphinx - optional, if you want to build documentation
- doxygen - optional, if you want to build documentation

## Build dependencies
- ivi-logging (https://github.com/Pelagicore/ivi-logging) (>= 1.3.0)
- glib
- dbus
- glibmm   (>=2.42.0)
- lxc      (>=2.0.0)
- jansson  (>=2.6)

### Install build dependencies on Debian
```
$ sudo apt-get install lxc lxc-dev libglib2.0-dev libglibmm-2.4 \
                       libdbus-1-dev \ libglibmm-2.4-dev libglibmm-2.4 \
                       libjansson-dev libjansson4
```

### Reasoning behind dependencies
- Being a piece of software aimed towards the automotive industry, ivi-logging is a useful logging
  tool, since it interfaces well with GENIVI DLT.
- In order to be able to provide a simple IPC from clients/launchers, we chose DBus as the IPC
  mechanism, as it is a proven solution that is de-facto standard on most GNU/Linux systems.
- Glibmm is used mainly for main loop purposes, for setting up the dbus service, and for spawning
  binaries (such as dbus-proxy)
- jansson is a simple, reliable c library for parsing json data. We have used jansson is other
  projects, but there is no deeper reasoning behind using that specific library. We should probably
  move to a c++ library some time in the future.

## Runtime dependencies
- lxc
- iptables
- brctl, available in bridge-utils on Debian-based systems

# Building
Building and installing is simple:
```
$ mkdir build
$ cd build
$ cmake ../
$ make
$ sudo make install
```

To build the project documentation set ``-DENABLE_ALL_DOC=ON`` when running
cmake. You can also build the docs separately by pointing cmake to the doc
directory. Documentation is separated into doxygen docs (API level docs)
and docs built with sphinx (user documentation and general docs on a higher
level).

To run the unit-tests build with ``-DENABLE_TEST=ON``. To run code coverage tools
build with ``-DENABLE_COVERAGE=ON``. Then run coverage with ``make lcov`` (needs
to be run as root).

To disable support for various gateways at compile time, set
* ``-DENABLE_PULSEGATEWAY=OFF`` (for pulse)
* ``-DENABLE_NETWORKGATEWAY=OFF`` (for network)
* ``-DENABLE_DEVICENODEGATEWAY=OFF`` (for device node gateway)
* ``-DENABLE_DBUSGATEWAY=OFF`` (for dbus)
* ``-DENABLE_CGROUPSGATEWAY=OFF`` (for cgroups)
* ``-DENABLE_WAYLANDGATEWAY=OFF`` (for wayland)
* ``-DENABLE_FILEGATEWAY=OFF`` (for mounting files)
* ``-DENABLE_ENVGATEWAY=OFF`` (for manipulating environment variables)

To build the examples, build with ``-DENABLE_EXAMPLES=ON`` (and see Examples
section).

To prevent SoftwareContainer from creating a network bridge on startup (and
instead only check that one is there), build with ``-DCREATE_BRIDGE=OFF``.

For a concrete example of building SoftwareContainer and setting up
dependencies, see Vagrantfile in this repository. For an example on how to
build this code, please take a look at the Vagrantfile.

**Note**: It is possible to get the complete list of CMake options by running:
``
    mkdir build
    cd build
    cmake -LAH ..
``

## Install without root
In order to install SoftwareContainer without root there are two things that needs to be set.

1. ``-DENABLE_SYSTEM_BUS`` needs to be set to OFF. The dbus xml that is needed for
   the system bus to be used can not be installed without root privileges.
2. ``-DCMAKE_INSTALL_PREFIX`` needs to be set to a path the installing user has
   write access to.

## Building in Vagrant
Vagrant can be used to quickly set up a virtualized environment for building.
On a debian-based system, issue the following commands to build using Vagrant:

```
git submodule init
git submodule update

sudo apt update
sudo apt install virtualbox vagrant

vagrant up
```

The vagrant machine can then be inspected by running `vagrant ssh`

This will create an environment for building softwarecontainer, download all
the requirements, build the ones necessary to build, build softwarecontainer,
run unit tests and perform a clang code analysis run on the code.

# Running
You normally only want to run the softwarecontainer-agent, which requires root
privileges. It will register itself onto the system bus, so no dbus magic is
needed. Run it with `--help` to see runtime options.

## Why does it require root privileges?
First, creating LXC containers requires root privileges. Second, setting up a
network bridge using brctl and ifconfig/iptables typically also does require
that.

# Examples
For examples see the `examples` directory and the README.md there for more
information.

# Testing
There are currently three levels of tests: unit tests, component tests, and
service tests.

The unit tests written in gtest/gmock are available in */unit-test/*. The tests
are run using `run-tests.py` from the build directory. Note that you have
to run them as root, since the actual container creation is not stubbed off
and requires root access.

The service tests are in service-test/ and can be run with run-tests.sh.
See servicetest/README.md for more details about these tests.

All the examples can also be build and tested against the current codebase by
running run-tests.sh from the examples/ directory.

# Versioning
We use [semantic versioning](http://semver.org), but we have yet to release
a first stable 1.0.0 version, so for now, all bets are off.

# License and Copyright
Copyright (C) 2016-2017 Pelagicore AB

The source code included here is licensed under the LGPL 2.1. Please
see the "LICENSE" file for more information. Text documents are
licensed under creative commons 4.0

SPDX-License-Identifier: CC-BY-4.0
