.. _configuration:

Configuration
*************

This chapter gives a brief overview of mechanisms that are used to configure SoftwareContainer.

.. _cmd_options:

Command Line Options
====================
It is possible to configure SoftwareContainer Agent with command line options.

**-c or --config <path>** Path to SoftwareContainer configuration file, defaults to |sc-config-file-code|

**-p or --preload <num>** Number of containers to preload, defaults to |preload-count-code|

**-u or --user <uid>** Default user id to be used when starting processes in the container, defaults to ``0``.
This option is deprecated.

**-k or --keep-containers-alive** Containers will not be shut down on exit. Useful for debugging.
Defaults to |keep-containers-alive-code|

**-t or --timeout <seconds>** Timeout in seconds to wait for containers to shutdown,
defaults to |shutdown-timeout-code|

**-m or --manifest-dir <path>** Path to a file or directory where service manifest(s) exist,
defaults to |service-manifest-dir-code|

**-d or --default-manifest-dir <filepath>** Path to a file or directory where default service manifest(s) exist,
defaults to |default-service-manifest-dir-code|

**-b or --session-bus** To use the session bus on D-Bus, instead of the system bus. Defaults to |use-session-bus-code|

**-h or --help** Display usage and exit


.. _json_conf:

JSON
====
JSON is a lightweight data-interchange format. One method to configure SoftwareContainer is using
a JSON string as runtime configuration for specific objects.
For more information about JSON : http://www.json.org

:Container Configure Options:

When creating containers, a configuration is passed as a JSON string. The string is an array with
objects. For more information about supported parameters :ref:`Getting Started <getting-started>`

:Gateway Configure Options:

SoftwareContainer provides a set of gateways. All of these gateways can be configured on
runtime by using SetGatewayConfigs method with a configuration parameter passed as a JSON
string. For more information about gateways and parameters :ref:`Gateways <gateways>`

Files
=====
For some features configuration files are used to pass values to particular operations.

:LXC Configuration File:

There is a ``softwarecontainer.conf`` file to define many options used by The linux containers (lxc).
It is possible to configure back-end container with this file. For more information about LXC
configuration : http://man7.org/linux/man-pages/man5/lxc.container.conf.5.html

:Service Manifests:

As it is indicated in the :ref:`design chapter <design>`, each IPC is managed by a gateway. Service
manifests contain capabilities, which map to gateway configurations. For more information about
service manifests :ref:`Service manifests <service-manifests>`.


CMake Options
=============

CMake options can be used to configure SoftwareContainer. The following is an explanation of some
of them together with use case examples.

Gateways options
----------------
If a gateway is disabled this means that the gateway will not be part of SoftwareContainer at
compile time, hence this will disable the IPC mechanism that the gateway in question handles.
As an example, the following snippet will install SoftwareContainer without any network support::

    cmake -DENABLE_NETWORKGATEWAY=OFF ..
    make
    sudo make install

A complete list of available gateways can be found at :ref:`Gateways <gateways>`.
