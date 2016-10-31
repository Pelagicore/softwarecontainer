
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


.. _wayland-example:

Wayland example
===============

The Wayland example is not an example that is built in SoftwareContainer. Instead, it is only a
launch script that, before it launches the agent, sets up weston (a Wayland compositor). It then
creates a container and configures it for wayland access.  Then, it relies on the weston example
applications being present (weston-simple-egl and weston-flower). It launches these, runs for a
while, and shuts down.
