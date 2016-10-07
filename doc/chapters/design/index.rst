
Design
******

SoftwareContainer has been designed as a building block enabling sandboxing of
application contents in a larger system.

The SoftwareContainer achieves this by providing a structured way to limit
what inter-process communication (IPC) mechanims are made available to each
application. By targetting the IPC mechanisms rather than the applications or
services directly, the sandboxing mechanism can be deployed in a non-intrusive
way, i.e. completely transparent to both applications and services.

Each IPC is managed by a *gateway*. The IPC needs of each service is described
in *service manifest* files, defining *capabilities* and mapping these to
individual *gateway configurations*. This can be individual objects or
interfaces on d-bus, networking filter rules, access to device nodes or
something completely different.

Each application is then associated with a set of *capabilities*. This could
be done through an *application manifest*, or any other method supported by
the *launcher*. These are used to configure the sandbox in which the
application is then launched.

In a typical deployment the SoftwareContainer is coupled with a *launcher*,
which is responsible for initiating containers and provide an entry point to
start execution from. This means that SoftwareContainer is not intended to be
used as a standalone tool, but rather as a toolbox to build the full
experience from.

The *launcher* is responsible for providing SoftwareContainer with a list of
*capabilities*, a mount point for the application payload and an entrypoint to
start execution from.The SoftwareContainer does not concern itself with
application packaging, deployment of manifest formats. All it does it
associate the given *capabilities* with *gateway configurations* to setup the
sandbox with the appropriate IPC access to the surrounding world.

The Launcher
============

The launcher is responsible for setting up sandboxes for contained
applications and then launch the application entry point.

The SoftwareContainer API allows the launcher great control over the creation
of the container. This makes it possible to implement preloading schemes,
doing much of the sandbox creation and configuration prior to actually loading
an application.

The setup of a container is done in four phases:

1. **Creation**, where the container sandbox is instantiated. This sandbox has
   no access to the host platform.
2. **Activation**, where the sandbox is given a set of capabilities and the
   gateways are configured accordingly.
3. **Mounting**, where the application contents is mounted into the filesystem
   of the sandbox.
4. **Launching**, where the application entry point is executed.

The main launcher for SoftwareContainer is a part of the
`QtApplicationManager <http://code.qt.io/cgit/qt/qtapplicationmanager.git/>`_.

Service Manifests
=================

Service manifests are JSON files mapping capabilities to gateway
configurations. Capabilities is what applications specify when they want
access to a specific feature of the platform. By specifying a list of
capabilities to SoftwareContainer, a sandbox with the correct gateway
configurations is created, providing access to the selected parts of the
platform services.

Capabilities are named according to a reverse DNS scheme, e.g.
``org.genivi.dlt`` or ``com.pelagicore.popups``. In some cases, the same
service provides capabilities of different levels, e.g. generic permission to
show popup compared to permission to show high priority popups. This is
generally added to the end of the name, e.g. ``com.pelagicore.popups`` and
``com.pelagicore.popups.important``.

The service manifests are typically placed in one JSON file per service, but
can be split into one file per capability if needed. The files are placed in
``/share/resource-access-manager/caps.d/``.

For more detailed information about service manifests content and format
see :ref:`Service manifests <service-manifests>`
