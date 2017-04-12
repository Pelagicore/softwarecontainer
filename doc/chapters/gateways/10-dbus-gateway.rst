D-Bus gateway
=============

The D-Bus Gateway is used to provide access to host system D-Bus buses, object paths, and
interfaces.

The gateway will create a socket for the D-Bus connection being proxied.
The socket will be placed in a directory accessible from within the
container, and the applications running inside the container are expected
to use this socket when communicating with the outside D-Bus system.
The gateway uses an external program ``dbus-proxy``, which is included
in SoftwareContainer as a submodule, for creation and communication over
the proxied bus connection.

The gateway will also set the ``DBUS_SESSION_BUS_ADDRESS`` or
``DBUS_SYSTEM_BUS_ADDRESS`` for the session and system bus, respectively.
libdbus uses these variables to find the socket to use for D-Bus
communication, and the application running within the container is
expected to use these variables (probably by means of the binding used).

The ``dbus-proxy`` does not modify the messages passed via D-Bus, it only provides a filter
function to allow or disallow on the object path, interface, and method level.

Method arguments are not filtered or inspected in any way. This is good to keep in mind
in certain situations. For example, if an object implements the
``org.freedesktop.DBus.Properties`` interface and the user calls ``GetAll("org.myinterface")``,
the object might return properties for the ``org.myinterface`` interface even if that
specific interface is not allowed by the configuration. This behavior depends on the service
implementation, but the point is that the filtering will not be able to block the call to
``GetAll`` based on the method argument.

Introspection data is not modified to reflect the filtering rules. This means that some filters
may cause unwanted behaviour used in combination with dynamic interpretation of introspection data.
For example, if introspection is configured to be allowed, the introspection data might contain
interfaces and object paths that are not accessible for the application (unless the configuration
also allows everything on the connection).

The ``dbus-proxy`` always allows the ``org.freedesktop.DBus`` interface, even if it is not
specified in the gateway configuration. This means that methods like ``Hello``, ``ListNames``,
etc., are always accessible.

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


