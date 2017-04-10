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

Also note that the one can use a suffix (k, K, m, M, g or G) to indicate values in kilo,
mega or gigabytes when setting ``memory.limit_in_bytes`` or ``memory.memsw.limit_in_bytes``.

Setting network classes
-----------------------
If you want to mark packets from containers, you can set the ``net_cls.classid`` to a hexadecimal
value of the form 0xAAAABBBB (A denotes major number, B denotes minor number), where leading zeroes
may be omitted. A value 0x10001 means 1:1 for major/minor. The gateway supports and checks the
syntax for this key. More information about this can be found in the `Linux kernel docs
<https://www.kernel.org/doc/Documentation/cgroup-v1/net_cls.txt>`_.

Relative CPU shares
-------------------
It is possible to set the relative CPU shares for containers within SoftwareContainer, using the
``cpu.shares`` setting. The value should be a number of at least 2, as specified in the below
linked RedHat guide. The recommended maximum value is max int (2147483647).

More information can be found in the `RedHat Resource Management Guide
<https://access.redhat.com/documentation/en-US/Red_Hat_Enterprise_Linux/6/html/Resource_Management_Guide/sec-cpu.html>`_.

Whitelisting
------------

This gateway has a whitelisting policy for ``memory.limit_in_bytes``,
``memory.memsw.limit_in_bytes``, and ``cpu.shares``.
If the gateway is configured multiple times with either of these settings the bigger value
will be set. For all other settings the latest read value will be set.

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
        },
        {
            "setting": "net_cls.classid",
            "value": "0x10001"
        },
        {
            "setting": "net_cls.classid",
            "value": "0xFF0001"
        },
        {
            "setting": "cpu.shares",
            "value": "520"
        },
        {
            {"setting": "cpu.shares",
            "value": "800"
        }
    ]

The root object is an array of setting key/value pair objects. Each key/value pair
must have the ``setting`` and ``value`` defined. In the example above the value of
memory.limit_in_bytes will be set to 12000000 due to the whitelisting policy mentioned above.
This example will also set the classid for net_cls to have major number 255 and minor number 1
(because this configuration will be read later than then 1:1 one).
This example will set cpu.shares to 800, due to the whitelisting policy mentioned above.
