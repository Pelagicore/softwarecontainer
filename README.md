# SoftwareContainer

This package includes the SoftwareContainer component.

# Building

To build the project documentation set -DENABLE_DOC=1 when running cmake.

The project Doxygen docs contain info about SoftwareContainer in general,
how to run SoftwareContainer and how to run the component tests.

To run the unit-tests build with -DENABLE_COVERAGE=1 -DENABLE_TESTS=ON

To disable support for various gateways at compile time, set
* -DENABLE_PULSEGATEWAY=OFF (for pulse)
* -DENABLE_NETGATEWAY=OFF (for network)
* -DENABLE_DEVICENODEGATEWAY=OFF (for device node gateway)
* -DENABLE_DBUSGATEWAY=OFF (for dbus)
* -DENABLE_CGROUPSGATEWAY=OFF (for cgroups)

Examples are built using -DENABLE_EXAMPLES=1

For a concrete example of building SoftwareContainer and setting up
dependencies, see Vagrantfile in this repository. For an example on how to
build this code, please take a look at the Vagrantfile.

# Building in Vagrant

Vagrant can be used to quickly set up a virtualized environment for building.
On a debian-based system, issue the following commands to build using Vagrant:

```
git submodule init
git submodule update
ssh-keygen -f vagrant_key

# MANUAL STEP: Add vagrant_key.pub to GitLab in order to access sources

sudo apt install virtualbox
sudo apt install vagrant
vagrant up
```

The vagrant machine can then be inspected by running `vagrant ssh`

This will create an environment for building softwarecontainer, download all
the requirements, build the ones necessary to build, build softwarecontainer,
run unit tests and perform a clang code analysis run on the code.

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
- ivi-logging (https://github.com/Pelagicore/ivi-logging)
- glib
- glibmm
- dbus
- dbus-c++
- lxc
- jansson

### Install build dependencies on Debian
```
$ sudo apt-get install lxc lxc-dev libglib-2.0-dev libglibmm-2.4 \
                       libdbus-c++-dev libdbus-c++-1-0v5 libdbus-1-dev \
                       libglibmm-2.4-dev libglibmm-2.4 lxc-dev \
                       libjansson-dev libjansson4
```

### Reasoning behind dependencies
- Being a piece of software aimed towards the automotive industry, ivi-logging
  is a useful logging tool, since it interfaces well with GENIVI DLT.
- In order to be able to provide a simple IPC from clients/launchers, we chose
  DBus as the IPC mechanism, as it is a proven solution that is de-facto
  standard on most GNU/Linux systems.
- Glibmm is used mainly for main loop purposes (dbus-c++ interfaces well with
  it), but also for spawning of binaries (such as dbus-proxy).
- jansson is a simple, reliable c library for parsing json data. We have used
  jansson is other projects, but there is no deeper reasoning behind using
  that specific library. We should probably move to a c++ library some time
  in the future.

## Runtime dependencies
- lxc
- iptables
- brctl, available in bridge-utils on Debian-based systems

# Running

You normally only want to run the softwarecontainer-agent, which requires root
privileges. It will register itself onto the system bus, so no dbus magic is
needed. Run it with `--help` to see runtime options.

## Why does it require root privileges?
First, creating LXC containers require root priveleges. Second, setting up a
network bridge using brctl and ifconfig/iptables typically also does require
that.

# Testing

There are currently two level of tests, the unit tests written in gtest/gmock
available in */unit-test/* and built when building the rest of the system if
-DENABLE_TESTS=ON is defined to cmake. The tests are run using run-tests.sh in
the CI environment so adding them there is a requirement.

Secondly, there are also component tests i the directory component-test/ which
will be run using run-tests.sh.

# TroubleShooting

softwarecontainer is not very complex to begin with but due to a lot of
dependencies there may be some issues with running it. Here are a few common
pitfalls that might be good to be aware of.

## Note on PulseAudio

In order to pulseaudio to work inside the container it needs to be running when
the container is started. This is the case because when softwarecontainer sets
up the container it will look for the socket pointed out by PULSE_SERVER, and
mount it into the container, so that socket has to exist.

## DBus service files

softwarecontainer-agent requires dbus for it to work, this also means that you
need to setup the DBus policies for the softwarecontainer-agent to work. This is
automatically done when you make install the project, but if you install to
somewhere else than the root directory, the DBus policy configuration will wind
up in <prefix>/etc/dbus-1/system.d/ instead of /etc/dbus-1/system.d. If you
don't have the correctly setup configuration, you will get an error along these
lines:

> MAIN [Warning] Can't own the namecom.pelagicore.SoftwareContainerAgent on the system
> bus => use session bus instead         | softwarecontaineragent.cpp:337

# License and Copyright

Copyright (c) 2016 Pelagicore AB

The source code included here is licensed under the LGPL 2.1. Please
see the "LICENSE" file for more information. Text documents are
licensed under creative commons 4.0

SPDX-License-Identifier: CC-BY-4.0
