.. _configuration:

Configuration
*************

This chapter documents brief overview of mechanisms that is used to configure SoftwareContainer.

.. _cmd_options:

Command Line Options
====================
It is possible to configure SoftwareContainer Agent with command line options.
 
**-p or --preload <num>** Number of containers to preload, defaults to 0

**-u or --user <uid>**        Default user id to be used when starting processes in the container, defaults to 0

**-s or --shutdown <bool>**   If false, containers will not be shutdown on exit. Useful for debugging. Defaults to true

**-t or --timeout <seconds>** Timeout in seconds to wait for containers to shutdown, defaults to 2


.. _json_conf:

JSON
====
JSON is a lightweight data-interchange format. One method to configure SoftwareContainer is using JSON string as runtime configuration for specific objects. For more information about JSON : http://www.json.org

:Container Configure Options:

When creating containers, a configuration is passed as a JSON string. The string is an array with objects. For more information about supported parameters :ref:`Getting Started <getting-started>`

:Gateway Configure Options:

SoftwareContainer provides a set of gateways. All of these gateways can be configured on runtime by using SetGatewayConfigs method with a configuration parameter passed as a JSON string. For more information about gateways and parameters :ref:`Gateways <gateways>`

Files
=====
For some feautures configuration files are used to pass values to particular operations.

:LXC Configuration File:

There is ``softwarecontainer.conf`` file to define many options used by The linux containers (lxc). It is possible to configure back-end container with this file. For more information about LXC configuration : http://man7.org/linux/man-pages/man5/lxc.container.conf.5.html

:Service Manifests:

As it is indicated in :ref:`design chapter <design>`, each IPC is managed by a gateway. Service manifests contains capabilities, which maps to gateway configurations. For more information about service manifests :ref:`Service manifests <service-manifests>`  


CMake Options
=============
CMake options can be used to configure softwareContainer. Desired gateways can be enabled/disabled by passing flags to cmake. (Currently following gateways are able for enable/disable option; network, pulse, device node, dbus, cgroup :ref:`Gateways <gateways>`).
