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

