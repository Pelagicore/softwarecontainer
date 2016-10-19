
.. _gateways:

Gateways
********

SoftwareContainer provides a set of gateways to enable communication between the host system and the contained system.

Each gateway is dedicated to an IPC mechanism and can then be applied to support multiple services. This makes the
system scalable, as the number of IPC mechanisms is limited while the number of possible services are unlimited.
*Currently, some gateways break this principle, which is subject to correction in the future. The design intention
is to have the gateways IPC centric and construct more abstract concepts like a Wayland "gateway" on top of
multiple IPC gateways.*

This chapter contains descriptions of the available gateways and their configuration options. It is intended to
explain to e.g. a service integrator how to write gateway configurations suitable for the service.

For more information how to integrate and use the configurations in a project, see Integration patterns.

For more information how to develop and integrate gateways in SoftwareContainer, see :ref:`Developer guidelines <developers>`.

.. todo:: The Integration patterns refers to a currently non-existing section.


Note on configurations
======================

All gateway configurations are JSON arrays containing valid JSON elements, and SoftwareContainer requires this.
Beyond that, the structure and content of this JSON is the responsibility of the respective gateway.


CGroups gateway
===============

The CGroups Gateway is used to limit the contained system's access to CPU and RAM resources.

Configuration
-------------

The gateway configuration contains settings as key/value pairs. The ``setting`` key
is a string in the format <cgroup subsystem>.<setting> and the ``value`` is a string
with the value to apply.

No syntax or other checks for correctness is performed on the key/value pairs,
see the `lxc.container.conf` man page for more details about valid settings.

Example configurations
----------------------

Example gateway config::

    [
        {
            "setting": "memory.limit_in_bytes",
            "value": "128M"
        },
        {
            "setting": "cpu.shares",
            "value":  "256"
        }
    ]

The root object is an array of setting key/value pair objects. Each key/value pair
must have the ``setting`` and ``value`` defined.


D-Bus gateway
=============

The D-Bus Gateway is used to provide access to host system D-Bus buses, object paths, and interfaces.

The gateway will create a socket for the D-Bus connection being proxied.
The socket will be placed in a directory accessible from within the
container, and the applications running inside the container are expected
to use this socket when communicating with the outside D-Bus system. The gateway uses an external
program ``dbus-proxy`` for creation and communication over the proxied bus connection.

The gateway will also set the ``DBUS_SESSION_BUS_ADDRESS`` or
``DBUS_SYSTEM_BUS_ADDRESS`` for the session and system bus, respectively.
libdbus uses these variables to find the socket to use for D-Bus
communication, and the application running within the container is
expected to use these variables (probably by means of the binding used).

The ``dbus-proxy`` does not modify the messages passed via D-Bus, it only provides a filter function.
This means that some filters may cause unwanted behaviour used in combination with dynamic
interpretation of introspection data. For example, if introspection is configured to be allowed,
the introspection data might contain interfaces and object paths that are not accessible for the
application (unless the configuration also allowes everything on the connection).

The gateway will not do any analysis of the configuration passed to it, but will pass this configuration
along to ``dbus-proxy`` verbatim. This is to support future changes in the configuration format.

Configuration
-------------

The session and system buses have separate configurations implemented as separate JSON array
objects with the names:

- ``dbus-gateway-config-session``
- ``dbus-gateway-config-system``

The arrays contain JSON objects where each object is an access rule specified as a combination
of:

- Direction of method call or signal, i.e. if the call or signal is outgoing from inside the container or incoming from the outside of the container.
- D-Bus interface of the method or signal.
- Object path where the interface is implemented.
- The method or signal name the rule is for.

The rules are implemented as name/value pairs:

- ``direction`` - A string set to either ``incoming`` if the call or signal is coming from the outside of the container, or ``outgoing`` if the call or signal is coming from inside of the container.
- ``interface`` - A string specifying a D-Bus interface name, e.g. ``org.freedesktop.DBus``.
- ``object-path`` - A string specifying a D-Bus object path, e.g. ``/org/freedesktop/UPower/Policy``.
- ``method`` - A string specifying a D-Bus method name or signal name, e.g. ``EnumerateDevices``.

All the values can be substituted with the wildcard character ``*`` with the meaning "all", e.g. a
"direction" set to ``*`` will mean both incoming and outgoing, and a ``method`` set to ``*`` will match
all method and signal names for the interface and object path specified.

If a bus configuration is just an empty array it means all access to that bus will be blocked.

Example configurations
----------------------

A configuration that provides full access to the system and session buses would look like::

    [
        {
            "dbus-gateway-config-session": [
                {
                    "direction": "*",
                    "interface": "*",
                    "object-path": "*",
                    "method": "*"
                }
            ],
            "dbus-gateway-config-system": [
                {
                    "direction": "*",
                    "interface": "*",
                    "object-path": "*",
                    "method": "*"
                }
            ]
        }
    ]

A configuration that provides full access to the session bus and no access at all to the system
bus would look like::

    [
        {
            "dbus-gateway-config-session": [
                {
                    "direction": "*",
                    "interface": "*",
                    "object-path": "*",
                    "method": "*"
                }
            ],
            "dbus-gateway-config-system": []
        }
    ]

A configuration that allows introspection on the session bus from within the container and no
access at all to the system bus would look like::

    [
        {
            "dbus-gateway-config-session": [
                {
                    "direction": "outgoing",
                    "interface": "org.freedesktop.DBus.Introspectable",
                    "object-path": "/",
                    "method": "Introspect"
                }
            ],
            "dbus-gateway-config-system": []
        }
    ]


Device node gateway
===================

The Device Node Gateway is used to provide access to host system device nodes.

Configuration
-------------

The configuration consists of a root list consisting of individual devices. Each device contains the following fields:

- ``name`` The name of the device, with or without path. This is passed verbatim to ``mknod``
- ``major`` The major device number, passed verbatim to ``mknod``
- ``minor`` The minor device number, passed verbatim to ``mknod``
- ``mode`` Permission mode, passed verbatim to ``chmod``

Example configurations
----------------------

An example configuration can look like this::

    [
        {
            "name":  "/dev/dri/card0"
        },
        {
            "name":  "tty0",
            "major": "4",
            "minor": "0",
            "mode":  "666"
        },
        {
            "name":  "tty1",
            "major": "4",
            "minor": "0",
            "mode":  "400"
        },
        {
            "name":  "/dev/galcore",
            "major": "199",
            "minor": "0",
            "mode":  "666"
        }
    ]


Environment gateway
===================

The Environment Gateway is used to set environment variables in the container.

The environment gateway allows users to specify environment variables that
should be known to the container and all commands and functions running
inside the container.

Configuration
-------------

The configuration consists of a list of environment variable definitions. Each such element must contain the following parameters:

- ``name`` The name of the environment variable in question
- ``value`` The value to attach to the name

It may also, optionally, specify the following parameters:

- ``append`` (bool) If the environment variable is already defined by the gateway, append the new value to the value already defined. Defaults to false.

Example configurations
----------------------

En example configuration would look like this::

    [
        {
            "name": "SOME_ENVIRONMENT_VARIABLE",
            "value": "SOME_VALUE"
        }
    ]

Note that ``value`` will be read as a string.

There are also the possibility to append to an already defined variable::

    [
        {
            "name": "SOME_ENVIRONMENT_VARIABLE",
            "value": "_SOME_SUFFIX",
            "append": true
        }
    ]


File gateway
============

The File Gateway is used to expose individual files from the host filesystem inside the container.

Configuration
-------------

In the container, the files are mapped into a subdirectory (currently ``/gateways``), at the location specified by the ``path-container`` field (see below).

Example configurations
----------------------

An example configuration can look like this::

    [
        {
            "path-host": "/tmp/someIPSocket",   // Path to the file in host's file-system
            "path-container": "someIPSocket",   // Sub-path of the mount point in the container
            "create-symlink": true, // specifies whether a symbolic link should to be created so that the file is available in the container under the same path is in the host.
            "read-only": false,  // if true, the file is accessible in read-only mode in the container
            "env-var-name": "SOMEIP_SOCKET_PATH", // name of a environment variable to be set
            "env-var-prefix": "some-path-prefix", // define a prefix for the path set in the environment variable defined by "env-var-name"
            "env-var-suffix": "some-path-suffix", // define a suffix for the path set in the environment variable defined by "env-var-name"
        }
    ]

Network gateway
===============

The Network Gateway is used to setup network connection and configure which traffic is allowed and not.

Configuration
-------------
The configuration is structured as a list of JSON objects that each describe rules for ``OUTGOING``
or ``INCOMING`` network traffic. During evaluation of each configuration entry, the order in which
the rules are specified in matters. For each entry the rules are read in order starting with the
first specified rule. The validation is done by filtering network traffic on ``host`` and ``port``,
where ``host`` is either hostname or ip address of a destination or source depending on context and
``port`` specifies which ports to filter on. When ``port`` is not specified, the rule applies to all
ports. How to handle matching traffic is specified by ``target``. There are tree valid values for
``target``: ``ACCEPT``, ``DROP`` and ``REJECT``, where ``ACCEPT`` does nothing with the traffic
while ``REJECT`` and ``DROP`` both deny the network traffic to continue. The difference between
the latter two being that ``REJECT`` answers the sender while ``DROP`` does not. If no rule applies
``default`` specifies what to do with the traffic.

As mentioned above, different order of the rules can have profound different meaning. Following are
an example attempting to reject all traffic from example.com on ports 1234 to 5678 and adding an
exception for port 1423.::

    [
        {
            "type": "INCOMING",
            "priority": 1,
            "rules": [
                         { "host": "example.com", "port": "1234-5678", "target": "REJECT"},
                         { "host": "example.com", "port": 1423, "target": "ACCEPT"},
                     ],
            "default": "DROP"
        }
    ]

However, since the rejection of the ports 1234 to 5678 is declared before the exception to the port
1423, which is included in the range of ports rejected, the exception will not have any effect. In
order to achive this the order of the rules has to be changed::

    [
        {
            "type": "INCOMING",
            "priority": 1,
            "rules": [
                         { "host": "example.com", "port": 1423, "target": "ACCEPT"},
                         { "host": "example.com", "port": "1234-5678", "target": "REJECT"},
                     ],
            "default": "DROP"
        }
    ]

Now, any incoming traffic from example.com on port 1423 will be accepted and traffic on the rest
of the range will be rejected.


In order to not make the listing of capabilities, e.g. in application manifests, order dependent,
``priority`` is used when merging network gateway configurations of the same type. This is specified
as an unsigned int > 0 where 1 describes the highest priority.

An example of valid network gateway configuration::

    [
        {
            "type": "OUTGOING",
            "priority": 1,
            "rules": [
                         { "host": "127.0.0.1/16", "port": 80, "target": "ACCEPT"},
                         { "host": "example.com", "port": "80-85", "target": "ACCEPT"},
                         { "host": "127.0.0.1/16", "port": [80, 8080], "target": "ACCEPT"},
                         { "host": "203.0.113.0/24", "target": "DROP"},
                     ],
            "default": "REJECT"
        },
        {
            "type": "INCOMING",
            "priority": 3,
            "rules": [
                         { "host": "127.0.0.1/16", "port": 80, "target": "ACCEPT"},
                         { "host": "example.com", "port": "80-85", "target": "ACCEPT"},
                         { "host": "127.0.0.1/16", "port": [80, 8080], "target": "ACCEPT"},
                         { "host": "203.0.113.0/24", "target": "REJECT"},
                     ],
            "default": "DROP"
        }
    ]


PulseAudio gateway
==================

The PulseAudio Gateway is used to provide access to the host system PulseAudio server.

This gateway is responsible for setting up a connection to the
PulseAudio server running on the host system. The gateway decides whether to
connect to the PulseAudio server or not based on the configuration.

When configured to enable audio, the gateway sets up a mainloop and then connects
to the default PulseAudio server by calling ``pa_context_connect()``. This is done
during the ``activate()`` phase.

Once ``activate`` has been initiated, the gateway listens to changes in the connection
through the ``stateCallback`` function and, once the connection has been successfully
set up, loads the ``module-native-protocol-unix`` PulseAudio module.

Configuration
-------------

Example configuration enabling audio::

    [
        { "audio": true }
    ]

A malformed configuration or a configuration that sets audio to false will simply
disable audio and in such case, the gateway will not connect to the PulseAudio
server at all.


Wayland gateway
===============

The Wayland Gateway is used to provide access to the hsot system Wayland server.

Please note that the Wayland Gateway only provides access to the Wayland
socket and sets the ``XDG_RUNTIME_DIR`` accordingly. For a Wayland
*capability*, it is also necessary to ensure that the contained application
has access to the graphics hardware devices and associated libraries.

Configuration
-------------

Example configuration enabling Wayland::

    [
        { "enabled": true }
    ]
