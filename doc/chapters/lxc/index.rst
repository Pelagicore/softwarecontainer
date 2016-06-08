SoftwareContainer using LXC
***************************

SoftwareContainer uses the Linux Containers [#lxc] (LXC) project as its container backend. LXC uses namespaces and cgroups,
and has been supported in the Linux kernel since 2.6.24. 

LXC Template
------------
LXC expects it to be fed a template that sets up a basic file system structure so that the container has something to boot into.
The LXC template SoftwareContainer uses does three things currently:

# Creates a basic rootfs with all directories one would expect
# Copies busybox into the rootfs, and populates /bin with all its aliases
# Adds some conditional options to the lxc configuration file

Create basic rootfs
^^^^^^^^^^^^^^^^^^^
The rootfs created is a basic FHS-like [#fhs] structure, although stripped down, with the added /gateways directory, as well as 
taking two variables when being run by CMake - ``${CMAKE_INSTALL_PREFIX}`` and ``${ADDITIONAL_FOLDER_MOUNTS}``. 
Furthermore, this steps creates a root user and group, and then sets some configuration options in three places:

* ``/etc/pulse/client.conf`` - tell pulse not to use shm
* ``/etc/machine-id`` - populated with a dbus-uuid
* ``/etc/resolv.conf`` - copied from host

/lib64 and /usr/lib64 are added to the rootfs also - they will be empty unless they exist in the host, in which case they
will be bind mounted just like all other file systems, more on that below on the configuration file [#configfile]

Copy and set up busybox
^^^^^^^^^^^^^^^^^^^^^^^
This steps checks for busybox on the host, copies it into the rootfs for the container, then symlinks all its functions to busybox 
in the ``/bin`` directory in the container - so that ``/bin/ls -> /?bin/busybox``.

There is an ongoing discussion on the need for busybox at all - this has implications on startup time as well as for the actual code.

Set up dynamic configuration options
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
This step adds some entries to the config file for LXC. It sets the location of the rootfs, and adds some mount entries,
namely the following:

* The directory containing ``init.lxc`` is bind-mounted to ``/usr/sbin``
* If ``$GATEWAY_DIR`` is set, bind mount its directory to ``/gateways`` and ``chmod 777`` it.

Full example
^^^^^^^^^^^^
This is the full template used.

.. literalinclude:: ../../../libpelagicontain/lxc-pelagicontain.in
    :language: bash

LXC Configuration file
-----------------------

The configuration file contains three things: network setup, device and pty/tty allocation, and mount entries.

Network setup
^^^^^^^^^^^^^
This configuration tells LXC to create a veth interface, connected to lxcbr0 (not set up here!), and for the network to be up.

Device and pty/tty allocation
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
LXC has a directive called "autodev", which, when set, creates all needed devices automatically.
This is used in conjunction with telling LXC to allocate tty and pty devices.

Mount entries
^^^^^^^^^^^^^
The static mount entries tells LXC to bind mount /usr, /lib, /usr/lib and /proc into the container. These are the amended by the template when run

Full example:
^^^^^^^^^^^^^
literalinclude:: ../../../libpelagicontain/pelagicontain.conf
    :language: bash

LXC API
-------
TBD

.. [#lxc] https://linuxcontainers.org/
.. [#fhs] https://en.wikipedia.org/wiki/Filesystem_Hierarchy_Standard
