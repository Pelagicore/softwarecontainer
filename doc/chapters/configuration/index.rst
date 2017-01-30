.. _configuration:

Configuration
*************

This chapter gives a brief overview of mechanisms that are used to configure SoftwareContainer (SC).

There are different categories of configuration sources for SC:

* Build configuration
* Main configuration of SC itself
* Command line options
* LXC configuration
* Gateway configuration
* Dynamic configuration on D-Bus interface

There are different purposes of the configurations:

* SC functionality and behavior
* LXC container configuration
* Environment of the application running inside a container

Configurations are applied at startup by means of installed config files, or at a later stage
through the D-Bus API.


Build configuration
===================

CMake options can be used to configure SC during the configure stage of building. Which gateways
that will be enabled are defined at this stage and are explained below. For a complete list of
options and the details about them, see the top level ``README.md`` and top level ``CMakeLists.txt``
in the SC repo.

Some configuraion options might affect the content of other config files that are created during
the build process, or default values used by SC at runtime.

Gateways build options
----------------------

If a gateway is disabled this means that the gateway will not be part of SoftwareContainer at
compile time, hence this will disable the IPC mechanism that the gateway in question handles.
As an example, the following snippet will configure SC without any network support::

    cmake -DENABLE_NETWORKGATEWAY=OFF


Main SoftwareContainer config
=============================

The main configuration of SC is defined in a configuration file which is configured and installed
by the build system and located (by default) in |sc-config-file-code|.

In the config file, all options available are present and the ones that are optional are set to their
default value and commented out. The values they are set to is what will be used unless otherwise
specified by being uncommented and set to a different value.

Any options in the config file that are not commented out by default are mandatory and needs to
be present. It is an error to comment these out or remove them.

Some of the options in this config can also be set on command line, see :ref:`Command line options <cmd-line-options>`.

In the case of multiple sources of the same config option being available, the order of precedence is:

* Command line options related to the main config options are always considered first
* Config file options are considered secondly
* Defaults are applied if nothing else was specified


.. _cmd-line-options:

Command line options
====================
It is possible to configure SoftwareContainer with command line options.

**-c or --config <path>** Path to SoftwareContainer configuration file, defaults to |sc-config-file-code|

**-u or --user <uid>** Default user id to be used when starting processes in the container, defaults to ``0``.
This option is deprecated.

**-t or --timeout <seconds>** Timeout in seconds to wait for containers to shutdown,
defaults to |shutdown-timeout-code|

**-m or --manifest-dir <path>** Path to a file or directory where service manifest(s) exist,
defaults to |service-manifest-dir-code|

**-d or --default-manifest-dir <path>** Path to a file or directory where default service manifest(s) exist,
defaults to |default-service-manifest-dir-code|

**-b or --session-bus** To use the session bus on D-Bus, instead of the system bus. Defaults to |use-session-bus-code|

**-h or --help** Display usage and exit


LXC config
==========

LXC is configured partly by the LXC template ``libsoftwarecontainer/lxc-softwarecontainer.in`` which is configured
and installed by CMake. This is passed to LXC by SC.

The other source of LXC specific configuration is in ``libsoftwarecontainer/softwarecontainer.conf.in`` which is
configured and installed by CMake to |lxc-config-path-code|.


Gateway configuration
=====================

Gateways (that are configured to be built) are configured by means of gateway configs. For details about
these configs, their content and format, and how they are applied, see :ref:`Design <design>`,
:ref:`Integration guidelines <integration-guidelines>`, and :ref:`Gateways <gateways>`.


Dynamic configuration
=====================

When creating containers using the D-Bus interface, a configuration is passed as a JSON string.
For more details, see :ref:`Getting Started <getting-started>`.

This configuration is used for configs that needs to be set per container at runtime.
