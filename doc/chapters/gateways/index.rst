
.. _gateways:

Gateways
********

SoftwareContainer provides a set of gateways to enable communication between the host system and the
contained system.

Each gateway is dedicated to an IPC mechanism and can then be applied to support multiple services.
This makes the system scalable, as the number of IPC mechanisms is limited while the number of
possible services are unlimited.  *Currently, some gateways break this principle, which is subject
to correction in the future. The design intention is to have the gateways IPC centric and construct
more abstract concepts like a Wayland "gateway" on top of multiple IPC gateways.*

This chapter contains descriptions of the available gateways, their IDs,  and their configuration
options.  It is intended to explain to e.g. a service integrator how to write gateway configurations
suitable for the service.

For more information how to integrate and use the configurations in a project,
see :ref:`Integration guidelines <integration-guidelines>`.

For more information how to develop and integrate gateways in SoftwareContainer,
see :ref:`Developer guidelines <developers>`.


Note on configurations
======================

All gateway configurations are JSON arrays containing valid JSON elements, and SoftwareContainer
requires this.  Beyond that, the structure and content of this JSON is the responsibility of the
respective gateway.


CGroups gateway
===============

The CGroups Gateway is used to limit the contained system's access to CPU and RAM resources.

ID
--

The ID used for the CGroups gateway is: ``cgroups``

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

The D-Bus Gateway is used to provide access to host system D-Bus buses, object paths, and
interfaces.

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

The ``dbus-proxy`` does not modify the messages passed via D-Bus, it only provides a filter
function.  This means that some filters may cause unwanted behaviour used in combination with
dynamic interpretation of introspection data. For example, if introspection is configured to be
allowed, the introspection data might contain interfaces and object paths that are not accessible
for the application (unless the configuration also allowes everything on the connection).

The gateway will not do any analysis of the configuration passed to it, but will pass this
configuration along to ``dbus-proxy`` verbatim. This is to support future changes in the
configuration format.

ID
--

The ID used for the D-Bus gateway is: ``dbus``

Configuration
-------------

The session and system buses have separate configurations implemented as separate JSON array
objects with the names:

- ``dbus-gateway-config-session``
- ``dbus-gateway-config-system``

The arrays contain JSON objects where each object is an access rule specified as a combination
of:

- Direction of method call or signal, i.e. if the call or signal is outgoing from inside the
  container or incoming from the outside of the container.
- D-Bus interface of the method or signal.
- Object path where the interface is implemented.
- The method or signal name the rule is for.

The rules are implemented as name/value pairs:

- ``direction`` - A string set to either ``incoming`` if the call or signal is coming from the outside of the container, or ``outgoing`` if the call or signal is coming from inside of the container.
- ``interface`` - A string specifying a D-Bus interface name, e.g. ``org.freedesktop.DBus``.
- ``object-path`` - A string specifying a D-Bus object path, e.g. ``/org/freedesktop/UPower/Policy``.
- ``method`` - A string or an array of strings specifying a D-Bus method name or signal name, e.g. ``EnumerateDevices``.

All the values can be substituted with the wildcard character ``*`` with the meaning "all", e.g. a
"direction" set to ``*`` will mean both incoming and outgoing, and a ``method`` set to ``*`` will
match all method and signal names for the interface and object path specified.

If a bus configuration is just an empty array it means all access to that bus will be blocked. When
a D-Bus message is sent, the ``dbus-proxy`` compares message's ``direction``, ``interface``, ``path``
and ``method`` with configuration list. if a matching rule is found, the message is allowed to forward;
otherwise it is dropped.

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

A configuration that allows access to the session bus on only methods "Method1", "Method2","Method3" and "Method4"::

    [
        {
            "dbus-gateway-config-session": [
                {
                    "direction": "*",
                    "interface": "*",
                    "object-path": "*",
                    "method": ["Method1", "Method3", "Method4", "Method2"]
                }
            ],
            "dbus-gateway-config-system": []
        }
    ]


Device node gateway
===================

The Device Node Gateway is used to provide access to host system device nodes.

ID
--

The ID used for the Device node gateway is: ``devicenode``

Configuration
-------------

The configuration consists of a root list consisting of individual devices. Each device contains the
following fields:

- ``name`` The name of the device, with or without path. This is passed verbatim to ``mknod``
- ``major`` The major device number, as an integer, passed verbatim to ``mknod``
- ``minor`` The minor device number, as an integer, passed verbatim to ``mknod``
- ``mode`` Permission mode, passed verbatim to ``chmod``

In the case where the same device node is configured more than once, the most permissive mode
is chosen per user type.

Example configurations
----------------------

An example configuration can look like this::

    [
        {
            "name":  "/dev/dri/card0"
        },
        {
            "name":  "tty0",
            "major": 4,
            "minor": 0,
            "mode":  "644"
        },
        {
            "name":  "tty1",
            "major": 4,
            "minor": 0,
            "mode":  "400"
        },
        {
            "name":  "/dev/galcore",
            "major": 199,
            "minor": 0,
            "mode":  "722"
        },
        {
            "name":  "/dev/galcore",
            "major": 199,
            "minor": 0,
            "mode":  "466"
        }
    ]

In the last example shown above the mode for ``/dev/galcore`` will be set to 766.

Environment gateway
===================

The Environment Gateway is used to set environment variables in the container.

The environment gateway allows users to specify environment variables that
should be known to the container and all commands and functions running
inside the container.

Note that any environment variables set here can be overridden when starting a binary on the
D-Bus interface, see :ref:`D-Bus API <dbus-api>`.

ID
--

The ID used for the Environment gateway is: ``env``

Configuration
-------------

The configuration consists of a list of environment variable definitions. Each
such element must contain the following parameters:

- ``name`` The name of the environment variable in question
- ``value`` The value to attach to the name

Note that ``value`` will be read as a string.

If a variable is added that has previously been added already, the new value is ignored
and the old value is kept intact. This is considered a misconfiguration and will generate
a log warning.

The configuration may also, optionally, specify the following parameters:

- ``mode``: A string that can be either:
    - ``set``: set the variable (this is the default, so one does not usually need to set it)
    - ``append``: append the value given to the previous value for the given variable.
    - ``prepend``: prepend the value given to the previous value for the given variable.
- ``separator``: A string that will be squeezed in between the old and new value if one wants to
                 prepend or append to a variable

Both ``append`` and ``prepend`` will have the same effect as ``set`` if the variable was not already
set.

Example configurations
----------------------

En example configuration would look like this::

    [
        {
            "name": "SOME_ENVIRONMENT_VARIABLE",
            "value": "SOME_VALUE"
        }
    ]

With the above configuration, ``SOME_ENVIRONMENT_VARIABLE`` would be set to ``SOME_VALUE``,
if the variable had not been previously set. In the case where ``SOME_ENVIRONMENT_VARIABLE``
would have been previously set to e.g. ``ORIGINAL_VALUE``, that value would be kept.

There is also the possibility to append to an already defined variable::

    [
        {
            "name": "SOME_ENVIRONMENT_VARIABLE",
            "value": "/some/path",
            "mode": "append"
            "separator": ":"
        }
    ]

With the above configuration, if ``SOME_ENVIRONMENT_VARIABLE`` had previously been set
to e.g. ``/tmp/test``, the varaiable value would now be set to ``/tmp/test:/some/path``.
If ``SOME_ENVIRONMENT_VARIABLE`` had not been previously set, the value would now be
set to ``:/some/path``.

File gateway
============

The File Gateway is used to expose individual files and directories from the host filesystem inside
the container.

ID
--

The ID used for the File gateway is: ``file``

Configuration
-------------

The paths inside the container has to be absolute. There is a check for not mounting over already
existing mount paths.

It is possible to supply the same host path in several configuration snippets to get the same file
mounted onto several locations in the container. However, It is not possible to supply the same
container path several times unless the host path is also the same. In those cases, the gateway will
merge the configurations and set the more permissive of the read-only settings for the file, with
read-write being more permissive than read-only.

Example configurations
----------------------

An example configuration can look like this::

    [
        {
            "path-host": "/tmp/someIPSocket",   // Path to the file in host's file-system
            "path-container": "/tmp/someIPSocket",   // Absolute path to the mount point in the container
            "read-only": false,  // if true, the file is accessible in read-only mode in the container
        }
    ]

Network gateway
===============

The Network Gateway is used to setup network connection and configure which traffic is allowed and
not.

ID
--
The ID used for the Network gateway is: ``network``

Configuration
-------------
The configuration is structured as a list of JSON objects that each describe rules for ``OUTGOING``
or ``INCOMING`` network traffic. The validation is done by filtering network traffic on ``host``,
``ports`` and ``protocols``, where ``host`` is either hostname or ip address of a destination or
source depending on context, ``ports`` specifies which ports to filter on and ``protocols``
specifies which protocols to filter.

``direction`` and ``allow`` list are mandatory for Network Gateway configuration. There are only two
valid values for ``direction``: ``INCOMING`` and ``OUTGOING``. In each entry in ``allow`` list only
``host`` is mandatory. ``host`` can be specific hostname or ip address and also ``*`` indicating all
available ip sources. When ``ports`` is not specified, the rule applies to all ports.  ``ports`` are
valid between 0 and 65536. There are three valid values for ``protocols``: ``"tcp"``, ``"udp"`` and
``"icmp"``. When ``protocols`` is not specified in the ``allow`` list item, the rule applies to all
protocols. When there is an item which has ports without protocols, tcp protocol will be applied to
the rule.  NetworkGateway is programmed to drop all packages that do not match any entry in
``allow`` list.

The following is an example which rejects all ping requests except example.com. Only port 53 on tcp
and udp protocols are allowed for enabling dns lookup. And only icmp protocol from "example.com" is
allowed.::

    [
        {
            "direction": "OUTGOING",
            "allow": [
                        {"host": "example.com", "protocols": "icmp"},
                        {"host": "*", "protocols": ["udp", "tcp"], "ports": 53}
                     ]
        },
        {
            "direction": "INCOMING",
            "allow": [
                        {"host": "example.com", "protocols": "icmp"},
                        {"host": "*", "protocols": ["udp", "tcp"], "ports": 53}
                     ]
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

ID
--

The ID used for the PulseAudio gateway is: ``pulseaudio``

Configuration
-------------

Example configuration enabling audio::

    [
        { "audio": true }
    ]

A malformed configuration or a configuration that sets audio to false will simply
disable audio and in such case, the gateway will not connect to the PulseAudio
server at all.

The configuration can be set several times, but once it has been set to ``true``
it will retain its value.


Wayland gateway
===============

The Wayland Gateway is used to provide access to the hsot system Wayland server.

Please note that the Wayland Gateway only provides access to the Wayland
socket and sets the ``XDG_RUNTIME_DIR`` accordingly. For a Wayland
*capability*, it is also necessary to ensure that the contained application
has access to the graphics hardware devices and associated libraries. Access
to graphics hardware is **not** setup by the Wayland gateway.

ID
--

The ID used for the Wayland gateway is: ``wayland``

Configuration
-------------

Example configuration enabling Wayland::

    [
        { "enabled": true }
    ]

The configuration of the Wayland gateway can be set several times, but once it has been set to ``true``
it will retain its value.
