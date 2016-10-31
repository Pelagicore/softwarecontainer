
.. _integration-guidelines:

Integration guidelines
**********************

This chapter describes the workflow and major tasks and concepts involved in integrating SoftwareContainer
into a platform.

.. todo::

    This chapter is work in progress and lacks a lot of content.

Introduction
============

The general steps involved in integration are:

* Writing service manifests for capabilities in the platform.
* Integrating a launcher.

Service manifests
=================

For details about format and content of service manifests, see :ref:`Service manifests <service-manifests>`
and :ref:`Gateways <gateways>`.

Service manifests should be installed in ``/etc/softwarecontainer/caps.d/<service-name>``.

Network setup
=============

The network setup of software container is dependent on a network bridge being available on the
host system, if compiled with support for the network gateway. By default, SoftwareContainer will
create such a bridge on the system if it is not already there. This can be changed, so that
SoftwareContainer will simply fail with an error message if the bridge was not available.

The selection of whether or not to create the bridge is a compile-time options given to CMake.

Wayland setup
=============

In order to have applications access wayland, one needs to enable the Wayland gateway, and possibly
give access to graphics hardware. Not all applications require direct access to the graphics
hardware, see :ref:`Wayland example <wayland-example>`. A reasonable capability for a Wayland
application would therefore include both the Wayland gateway and a configuration of the Device Node
gateway for any graphics hardware access needed.
