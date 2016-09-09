SoftwareContainer Gateways
**************************

SoftwareContainer provides a set of gateways to enable communication between the host system and the contained system.

Each gateway is dedicated to an IPC mechanism and can then be applied to support multiple services.. This makes the system scalable, as the number of IPC mechanisms is limited while the number of possible services are unlimited.

This chapter contains descriptions of the available gateways and their configuration options.

.. todo:: define phases, e.g. activate

Gateways
========

CGroups Gateway
---------------

The CGroups Gateway is used to limit the contained system's access to CPU and RAM resources.

Configuration
^^^^^^^^^^^^^

The gateway configuration contains settings as key/value pairs and the setting
key and value will be applied using ``lxc-cgroup`` as they are written in the
gateway config (the ``lxc.cgroup`` prefix is added to all keys by the gateway).

No syntax or other checks for correctness is performed on the key/value pairs,
see the ``lxc.container.conf`` man page for more details about legal settings.

JSON format used in gateway config (as passed to ``setConfig()``::

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

The root object is an array of seting key/value pair objects. Each key/value pair
must have the ``setting`` and ``value`` defined. With the above example config the calls
to lxc-cgroup would set the following:

- ``lxc.cgroup.memory.limit_in_bytes`` to ``128M``
- ``lxc.cgroup.cpu.shares to`` ``256``

It is an error to prepend the ``lxc.cgroup`` prefix to settings in the config.



D-Bus Gateway
-------------

The D-Bus Gateway is used to provide access to host system d-bus busses, object paths and interfaces.

The gateway will create a socket for the D-Bus connection being proxied.
The socket will be placed in a directory accessible from within the
container, and the applications running inside the container are expected
to use this socket when communicating with the outside D-Bus system.

The gateway will also set the ``DBUS_SESSION_BUS_ADDRESS`` or
``DBUS_SYSTEM_BUS_ADDRESS`` for the session and system bus, respectively.
libdbus uses these variables to find the socket to use for D-Bus
communication, and the application running withing the container is
expected to use these variables (probably by using libdbus).

The ``dbus-proxy`` does not modify the messages passed via d-bus, it only provides a filter function.
This means that some filters may cause unwanted behaviour used in combination with dynamic
interpretation of introspection data.

The gateway will not do any analysis of the configuration passed
to it through ``setConfig()``, but will pass this configuration along to
``dbus-proxy`` verbatim. This is to support future changes in the configuration
format.

Configuration
^^^^^^^^^^^^^

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
- ``interface`` - A string specifying a D-Bus interface name, e.g.``org.freedesktop.DBus``.
- ``object-path`` - A string specifying a D-Bus object path, e.g. ``/org/freedesktop/UPower/Policy``.
- ``method`` - A string specifying a D-Bus method name or signal name, e.g. ``EnumerateDevices``.

All the values can be substituted with the wildcard character ``*`` with the meaning "all", e.g. a
"direction" set to ``*`` will mean both incoming and outgoing, and a ``method`` set to ``*`` will match
all method and signal names for the interface and object path specified. If a bus configuration
is just an empty array it means all access to that bus will be blocked.

Example Configurations
^^^^^^^^^^^^^^^^^^^^^^

A configuration that provides full access to the system and session buses would look like::

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

A configuration that provides full access to the session bus and no access at all to the system
bus would look like::

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

A configuration that allows introspection on the session bus from within the container and no
access at all to the system bus would look like::

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

Device Node Gateway
-------------------

The Device Node Gateway is used to provide access to host system device nodes.

Configuration
^^^^^^^^^^^^^

The configuration consists of a root list consisting of individual devices. Each device contains the following fields:

- ``name`` The name of the device, with or without path. This is passed verbatim to ``mknod``
- ``major`` The major device number, passed verbatim to ``mknod``
- ``minor`` The minor device number, passed verbatim to ``mknod``
- ``mode`` Permission mode, passed verbatim to ``chmod``


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

Environment Gateway
-------------------

The Environment Gateway is used to export environment variables into the container.

The environment gateway allows users to specify environment variables that
should be known to the container and all commands and functions running
inside the container.

Configuration
^^^^^^^^^^^^^

The configuration consists of a list of environment variable definitions. Each such element must contain the following parameters:

- ``name`` The name of the environment variable in question
- ``value`` The value to attach to the name

It may also, optionally, specify the following parameters:

- ``append`` (bool) If the environment variable is already by the environment gateway defined append the new value to the value already defined. Defaults to false.

Example Configurations
^^^^^^^^^^^^^^^^^^^^^^

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
            "value": "SOME_SUFFIX",
            "append": true
        }
    ]

File Gateway
------------

The File Gateway is used to export individual files from the host file into the container.

Configuration
^^^^^^^^^^^^^

In the container, the files are mapped into a subfolders (currently ``/gateways``), at the location specified by the ``path-container`` field (see below).

An example configuration can look like this::

    {
        "path-host": "/tmp/someIPSocket",   // Path to the file in host's file-system
        "path-container": "someIPSocket",   // Sub-path of the mount point in the container
        "create-symlink": true, // specifies whether a symbolic link should to be created so that the file is available in the container under the same path is in the host.
        "read-only": false,  // if true, the file is accessible in read-only mode in the container
        "env-var-name": "SOMEIP_SOCKET_PATH", // name of a environment variable to be set
        "env-var-prefix": "some-path-prefix", // define a prefix for the path set in the environment variable defined by "env-var-name"
        "env-var-suffix": "some-path-suffix", // define a suffix for the path set in the environment variable defined by "env-var-name"
    }

Network Gateway
---------------

The Network Gateway is used to provide Internet access through a container-specific firewall.

Configuration
^^^^^^^^^^^^^

T.B.D.


PulseAudio Gateway
------------------

The PulseAudio Gateway is used to provide access to the host system PulseAudio server.

This SoftwareContainer gateway is responsible for setting up a connection to the
PulseAudio server running on the host system. The gateway decides whether to
connect to the PulseAudio server or not based on the configuration.

When configured to enable audio, the gateway sets up a mainloop and then connects
to the default PulseAudio server by calling ``pa_context_connect()``. This is done
during the ``activate()`` phase.

Once ``activate`` has been initiated, the gateway listens to changes in the connection
through the ``stateCallback`` function and, once the connection has been successfully
set up, loads the ``module-native-protocol-unix`` PulseAudio module.

Configuration
^^^^^^^^^^^^^

Example configuration enabling audio::

    [
        {"audio": true}
    ]

A malformed configuration or a configuration that sets audio to false will simply
disable audio and in such case, the gateway will not connect to the PulseAudio
server at all.

Wayland Gateway
---------------

The Wayland Gateway is used to provide access to the hsot system Wayland server.

Configuration
^^^^^^^^^^^^^

T.B.D.
