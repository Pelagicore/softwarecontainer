.. _integration-guidelines:

Integration guidelines
**********************

This chapter describes the workflow, major tasks and concepts involved in integrating SoftwareContainer
into a platform. A brief summary of the general steps involved:

#. Setting up the various IPC mechanisms, that should be reachable from inside containers, on the host system.
#. Writing service manifests for capabilities of the platform.
#. Integrating a launcher that interact with SoftwareContainer.

Note that SoftwareContainer as a project is split into multiple sub-components, and while it is possible to
use e.g. the container library directly, these guidelines assumes the integration point is the complete
component.

Below follows information about what SoftwareContainer assumes about the host system when handling different
IPC mechanisms and examples of configuration and usage.

Service manifests
=================

For details about format and content of service manifests, see :ref:`Service manifests <service-manifests>`
and :ref:`Gateways <gateways>`.

Service manifests should be installed in either |service-manifest-dir-code| or
|default-service-manifest-dir-code| depending on if the capabilities defined in the manifests
should be applied by default or not. Each manifest is expected to be of type JSON, including the
"json" file extension.

Service manifests are read at startup of SoftwareContainer. If two service manifests contain
capabilities with the same name the gateway configurations will be combined (without merging
or removing duplicates), so when the capability's gateway configurations are set all
configurations from the manifests are included. For more information on how gateway
configurations are handled, please refer to :ref:`Gateways <gateways>`.

A service manifest's content is used when one or more capability is set by a call to ``SetCapabilities``.
The respective gateway configurations that are part of any service manifest that relates to the specified
capabilities are then applied to the gateways. If any capability is missing, or if any of the gateway
configurations are erroneous, it is treated as a fatal error by SoftwareContainer as it will mean
the systems capabilities are not correctly defined and the environment of any applications would
be in a bad state.

Example
-------
Here is an example service manifest, with the capabilities for getting and setting temperature::

  {
    "version": "1",
    "capabilities": [{
        "name": "com.pelagicore.temperatureservice.gettemperature",
        "gateways": [{
            "id": "dbus",
            "config": [{
                "dbus-gateway-config-session": []
            }, {
                "dbus-gateway-config-system": [{
                    "direction": "outgoing",
                    "interface": "org.freedesktop.DBus.Introspectable",
                    "object-path": "/com/pelagicore/TemperatureService",
                    "method": "Introspect"
                }, {
                    "direction": "outgoing",
                    "interface": "com.pelagicore.TemperatureService",
                    "object-path": "/com/pelagicore/TemperatureService",
                    "method": "Echo"
                }, {
                    "direction": "outgoing",
                    "interface": "com.pelagicore.TemperatureService",
                    "object-path": "/com/pelagicore/TemperatureService",
                    "method": "GetTemperature"
                }, {
                    "direction": "incoming",
                    "interface": "com.pelagicore.TemperatureService",
                    "object-path": "/com/pelagicore/TemperatureService",
                    "method": "TemperatureChanged"
                }]
            }]
        }]
    }, {
        "name": "com.pelagicore.temperatureservice.settemperature",
        "gateways": [{
            "id": "dbus",
            "config": [{
                "dbus-gateway-config-session": []
            }, {
                "dbus-gateway-config-system": [{
                    "direction": "outgoing",
                    "interface": "org.freedesktop.DBus.Introspectable",
                    "object-path": "/com/pelagicore/TemperatureService",
                    "method": "Introspect"
                }, {
                    "direction": "outgoing",
                    "interface": "com.pelagicore.TemperatureService",
                    "object-path": "/com/pelagicore/TemperatureService",
                    "method": "Echo"
                }, {
                    "direction": "outgoing",
                    "interface": "com.pelagicore.TemperatureService",
                    "object-path": "/com/pelagicore/TemperatureService",
                    "method": "SetTemperature"
                }]
            }]
        }]
    }]
  }


Network setup
=============

The network setup of SoftwareContainer is dependent on a network bridge being available on the
host system, if compiled with support for the NetworkGateway. By default, SoftwareContainer will
create such a bridge on the system if it is not already there. This can be changed, so that
SoftwareContainer will simply fail with an error message if the bridge was not available.

The selection of whether or not to create the bridge is a compile-time option given to CMake.
Please see the README for more information about how to set the various CMake options.

For each container a virtual ethernet device will be set up and be bridged to the above mentioned
network bridge on the host system. The virtual ethernet device is then mapped to an ethernet device
inside of the container (usually eth0).

In order to configure what traffic is allowed the NetworkGateway is used. The NetworkGateway converts
the configuration it receives into iptables rules which are set for the network device inside of the
container. See :ref:`Gateways <gateways>` for more information.

Wayland setup
=============

In order to have applications access Wayland, one needs to enable the Wayland gateway, and possibly
give access to graphics hardware. Not all applications require direct access to the graphics
hardware, see :ref:`Wayland example <wayland-example>`. A reasonable capability for a Wayland
application would therefore include both the Wayland gateway and a configuration of the Device Node
gateway for any graphics hardware access needed.

Example
-------
Here is an example manifest defining Wayland access::

    {
        "version": "1",
        "capabilities": [{
            "name": "com.example.wayland-access",
            "gateways": [{
                "id": "wayland",
                "config": [{
                    "enabled": true
                }]
            }, {
                "id": "devicenode",
                "config": [{
                    "name": "/dev/dri/card0"
                }]
            }]
        }]
    }

The role of a launcher
======================

This section describes what typical integration actions are needed to integrate SoftwareContainer with
a launcher. For an overview of the general architecture involving a launcher and SoftwareContainer, see
:ref:`Design <design>`.

The assumed scenario in this section is that a launcher want to start an application inside the container.

The launcher should do the following:

 * Make the app home directory available inside the container.
 * Set the HOME environment variable in the container point to the above directory.

The above actions are performed by interacting with the SoftwareContainerAgent :ref:`D-Bus API <api>`.

Setting up a home directory and HOME
------------------------------------

By calling ``BindMount`` and passing a path on the host that will be mounted inside
the container at the location specified as the ``pathInContainer`` argument, a directory is
made available to any application started in the container later.

The path inside the container is intended to be set as the ``HOME`` environment variable inside the
container. The variable is set when calling ``Execute`` with an appropriate dictionary passed as
the  ``env`` argument.
