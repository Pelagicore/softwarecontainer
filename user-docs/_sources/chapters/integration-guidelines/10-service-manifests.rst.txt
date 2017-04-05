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


