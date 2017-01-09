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

Kernel Options
--------------
To be able to limit memory usage with cgroups, ``CONFIG_CGROUPS``, ``CONFIG_MEMCG`` and
``CONFIG_MEMCG_SWAP`` should be enabled. Before appying the configuration it is
advised to check if the cgroup option is enabled with swapaccount=1 feauture in the system's
kernel. The Following command will list available cgroup options::

	cat /proc/1/cgroup

Limiting Memory
---------------
To be able to limit memory, ``memory.limit_in_bytes`` should be set to desired value with gateway
configuration options. The operating system also uses swap memory which means if limiting rss and swap
memories is the desired action then ``memory.memsw.limit_in_bytes`` setting should also be specified
in gateway configuration. Note that value of ``memory.memsw.limit_in_bytes`` indicates rss + swap
memory, thus it cannot be less than ``memory.limit_in_bytes``.

Whitelisting
------------

This gateway has a whitelisting policy only when the setting is memory.limit_in_bytes. If the
gateway is configured multiple times whith the memory.limit_in_bytes setting, the bigger value
will be set. For all other settings the last value will be set.

Example configurations
----------------------

Example gateway config::

    [
        {
            "setting": "memory.limit_in_bytes",
            "value": "12000000"
        },
        {
            "setting": "memory.limit_in_bytes",
            "value": "5000"
        },
        {
            "setting": "memory.memsw.limit_in_bytes",
            "value": "12000000"
        }
    ]

The root object is an array of setting key/value pair objects. Each key/value pair
must have the ``setting`` and ``value`` defined. In the example above value of memory.limit_in_bytes
will be set to 12000000 due to whitelisting policy mentioned above.

