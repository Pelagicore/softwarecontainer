
.. _examples:

SoftwareContainer examples
**************************

SoftwareContainer comes with a few examples that can be built and/or installed
to showcase how SoftwareContainer works. All of these examples comes with a ``launch.sh`` script
that sets up softwarecontainer-agent, configures gateways, and runs the example.

.. _simple-example:

Simple example
==============

The simple example needs no special configuration to run, which is to say that it does not use any
gateways. The example code simply tries to allocate lots of memory forever. The launch script for
this example starts the agent, bind mounts the directory in which it (and the compiled simple
binary) exists into the container, and launches the simple binary. It runs for 30 secs, and then
shuts down the container, and the agent.

.. _temperature-example:

Temperature Example
===================
The temperature example consists of a temperature service, and a temperature client. The service
runs on the host system, and the client is meant to run inside the container. The two communicate
over DBus, so the DBus gateway is set up to allow traffic on DBus between the host and container.

The client tells the service to set the temperature once every second, and when the service emits
a signal that the temperature changed, the client logs this to a file.

.. _wayland-example:

Wayland example
===============

The Wayland example is not an example that is built in SoftwareContainer. Instead, it is only a
launch script that, before it launches the agent, sets up weston (a Wayland compositor). It then
creates a container and configures it for wayland access.  Then, it relies on the weston example
applications being present (weston-simple-egl and weston-flower). It launches these, runs for a
while, and shuts down.
